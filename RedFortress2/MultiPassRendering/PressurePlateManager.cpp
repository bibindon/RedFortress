#include "PressurePlateManager.h"

#include "SkullManager.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace
{
    const std::wstring kInactivePlateModelPath =
        L"res\\model\\pressure_plate\\pressure_plate_black.x";
    const std::wstring kActivePlateModelPath =
        L"res\\model\\pressure_plate\\pressure_plate_green.x";
    const float kPlateHalfWidth = 1.0f;
    const float kPlateHalfDepth = 1.0f;
    const float kPlayerRadius = 0.3f;
    const float kSkullRadius = 0.38f;
    const float kMinimumContactY = -0.2f;
    const float kMaximumContactY = 0.6f;
    const float kWallTravelDistance = 3.0f;
    const float kWallMoveSpeed = 2.0f;

    std::wstring Trim(const std::wstring& value)
    {
        std::size_t start = 0;
        while (start < value.size() && (value[start] == L' ' || value[start] == L'\t'))
        {
            ++start;
        }

        std::size_t end = value.size();
        while (end > start &&
               (value[end - 1] == L' ' || value[end - 1] == L'\t' || value[end - 1] == L'\r'))
        {
            --end;
        }
        return value.substr(start, end - start);
    }

    std::vector<std::wstring> SplitCsvLine(const std::wstring& line)
    {
        std::vector<std::wstring> cells;
        std::wstringstream stream(line);
        std::wstring cell;
        while (std::getline(stream, cell, L','))
        {
            cells.push_back(Trim(cell));
        }
        return cells;
    }
}

void PressurePlateManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
    m_pairs.clear();
}

void PressurePlateManager::LoadForStage(NSRender::Render& render, const std::wstring& csvPath)
{
    Clear(render);
    m_render = &render;

    if (csvPath.empty())
    {
        return;
    }

    std::wifstream file(NSRender::Util::GetExeDir() + csvPath);
    if (!file.is_open())
    {
        return;
    }

    std::unordered_set<int> loadedPairIds;
    std::unordered_set<int> loadedWallIds;
    std::wstring line;
    bool isFirstLine = true;
    while (std::getline(file, line))
    {
        if (isFirstLine)
        {
            isFirstLine = false;
            continue;
        }

        if (Trim(line).empty())
        {
            continue;
        }

        const std::vector<std::wstring> cells = SplitCsvLine(line);
        if (cells.size() < 9)
        {
            std::abort();
        }

        PressurePlatePair pair;
        pair.id = std::stoi(cells[0]);
        pair.platePosition.x = std::stof(cells[1]);
        pair.platePosition.y = std::stof(cells[2]);
        pair.platePosition.z = std::stof(cells[3]);
        pair.wallCsvId = std::stoi(cells[4]);
        pair.wallRotation.x = D3DXToRadian(std::stof(cells[5]));
        pair.wallRotation.y = D3DXToRadian(std::stof(cells[6]));
        pair.wallRotation.z = D3DXToRadian(std::stof(cells[7]));
        const float wallScale = std::stof(cells[8]);
        pair.wallScale = D3DXVECTOR3(wallScale, wallScale, wallScale);

        if (!loadedPairIds.insert(pair.id).second ||
            !loadedWallIds.insert(pair.wallCsvId).second)
        {
            std::abort();
        }

        if (!render.TryGetCsvMeshPosition(pair.wallCsvId, &pair.wallClosedPosition))
        {
            std::abort();
        }
        pair.wallPosition = pair.wallClosedPosition;
        pair.wallPhysicsId = PhysicsLib::PhysicsLib::GetCsvObjectId(pair.wallCsvId);
        if (pair.wallPhysicsId < 0)
        {
            std::abort();
        }

        pair.inactivePlateMeshId = render.AddMeshMix(kInactivePlateModelPath,
                                                     pair.platePosition,
                                                     D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                     1.0f,
                                                     -1.0f,
                                                     false,
                                                     false,
                                                     false);
        pair.activePlateMeshId = render.AddMeshMix(kActivePlateModelPath,
                                                   pair.platePosition,
                                                   D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                   1.0f,
                                                   -1.0f,
                                                   false,
                                                   false,
                                                   false);
        if (pair.inactivePlateMeshId < 0 || pair.activePlateMeshId < 0)
        {
            std::abort();
        }
        render.SetMeshMixEnabled(pair.activePlateMeshId, false);
        m_pairs.push_back(pair);
    }
}

void PressurePlateManager::Clear(NSRender::Render& render)
{
    for (PressurePlatePair& pair : m_pairs)
    {
        if (pair.inactivePlateMeshId >= 0)
        {
            render.RemoveMeshMix(pair.inactivePlateMeshId);
            pair.inactivePlateMeshId = -1;
        }
        if (pair.activePlateMeshId >= 0)
        {
            render.RemoveMeshMix(pair.activePlateMeshId);
            pair.activePlateMeshId = -1;
        }
    }
    m_pairs.clear();
}

void PressurePlateManager::Update(NSRender::Render& render,
                                  const D3DXVECTOR3& playerPosition,
                                  const SkullManager& skullManager,
                                  const float deltaSeconds)
{
    if (deltaSeconds <= 0.0f)
    {
        std::abort();
    }

    for (PressurePlatePair& pair : m_pairs)
    {
        const bool active =
            IsPlayerOnPlate(pair, playerPosition) || IsSkullOnPlate(pair, skullManager);
        SetPlateActive(render, pair, active);

        float targetY = pair.wallClosedPosition.y;
        if (active)
        {
            targetY += kWallTravelDistance;
        }

        const float previousY = pair.wallPosition.y;
        const float movementDistance = kWallMoveSpeed * deltaSeconds;
        if (pair.wallPosition.y < targetY)
        {
            pair.wallPosition.y = (std::min)(pair.wallPosition.y + movementDistance, targetY);
        }
        else if (pair.wallPosition.y > targetY)
        {
            pair.wallPosition.y = (std::max)(pair.wallPosition.y - movementDistance, targetY);
        }

        if (!render.SetCsvMeshPosition(pair.wallCsvId, pair.wallPosition))
        {
            std::abort();
        }

        PhysicsLib::PhysicsLib::UpdateCsvTransform(pair.wallCsvId,
                                                   pair.wallPosition,
                                                   pair.wallRotation,
                                                   pair.wallScale);
        D3DXVECTOR3 wallVelocity(0.0f, 0.0f, 0.0f);
        wallVelocity.y = (pair.wallPosition.y - previousY) / deltaSeconds;
        PhysicsLib::PhysicsLib::SetVelocity(pair.wallPhysicsId, wallVelocity);
    }
}

std::size_t PressurePlateManager::GetPairCount() const
{
    return m_pairs.size();
}

bool PressurePlateManager::IsPlayerOnPlate(
    const PressurePlatePair& pair,
    const D3DXVECTOR3& playerPosition) const
{
    const float deltaX = std::fabs(playerPosition.x - pair.platePosition.x);
    const float deltaY = playerPosition.y - pair.platePosition.y;
    const float deltaZ = std::fabs(playerPosition.z - pair.platePosition.z);
    return deltaX <= kPlateHalfWidth + kPlayerRadius &&
           deltaZ <= kPlateHalfDepth + kPlayerRadius &&
           deltaY >= kMinimumContactY &&
           deltaY <= kMaximumContactY;
}

bool PressurePlateManager::IsSkullOnPlate(
    const PressurePlatePair& pair,
    const SkullManager& skullManager) const
{
    for (const SkullObject& skull : skullManager.GetSkulls())
    {
        if (skull.state != SkullState::Resting)
        {
            continue;
        }

        const float deltaX = std::fabs(skull.position.x - pair.platePosition.x);
        const float deltaY = skull.position.y - pair.platePosition.y;
        const float deltaZ = std::fabs(skull.position.z - pair.platePosition.z);
        if (deltaX <= kPlateHalfWidth + kSkullRadius &&
            deltaZ <= kPlateHalfDepth + kSkullRadius &&
            deltaY >= kMinimumContactY &&
            deltaY <= kMaximumContactY)
        {
            return true;
        }
    }
    return false;
}

void PressurePlateManager::SetPlateActive(NSRender::Render& render,
                                          PressurePlatePair& pair,
                                          const bool active)
{
    if (pair.active == active)
    {
        return;
    }

    pair.active = active;
    render.SetMeshMixEnabled(pair.inactivePlateMeshId, !active);
    render.SetMeshMixEnabled(pair.activePlateMeshId, active);
}
