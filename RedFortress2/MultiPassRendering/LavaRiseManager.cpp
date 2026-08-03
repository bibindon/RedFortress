#include "LavaRiseManager.h"

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
    const std::wstring kLavaModelPath =
        L"res\\model\\plateLava.x";
    const D3DXVECTOR3 kDefaultRotation(0.0f, 0.0f, 0.0f);
    const D3DXVECTOR3 kDisabledPosition(0.0f, -10000.0f, 0.0f);
    const float kBasePlaneSize = 8.0f;
    const float kMinimumExtent = 0.001f;

    std::wstring Trim(const std::wstring& value)
    {
        std::size_t start = 0;
        while (start < value.size() &&
               (value[start] == L' ' || value[start] == L'\t'))
        {
            ++start;
        }

        std::size_t end = value.size();
        while (end > start &&
               (value[end - 1] == L' ' ||
                value[end - 1] == L'\t' ||
                value[end - 1] == L'\r'))
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

    D3DXMATRIX BuildLavaWorldMatrix(const D3DXVECTOR3& position,
                                    const D3DXVECTOR3& scale)
    {
        D3DXMATRIX scaleMatrix;
        D3DXMATRIX translationMatrix;
        D3DXMatrixScaling(&scaleMatrix, scale.x, scale.y, scale.z);
        D3DXMatrixTranslation(&translationMatrix,
                              position.x,
                              position.y,
                              position.z);
        return scaleMatrix * translationMatrix;
    }
}

void LavaRiseManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
    m_lavas.clear();
}

void LavaRiseManager::LoadForStage(NSRender::Render& render,
                                   const std::wstring& csvPath)
{
    Clear();
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

    std::unordered_set<std::wstring> loadedIds;
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
        if (cells.size() < 10)
        {
            std::abort();
        }

        Lava lava;
        lava.id = cells[0];
        lava.damage = std::stoi(cells[1]);
        lava.minX = std::stof(cells[2]);
        lava.maxX = std::stof(cells[3]);
        lava.minZ = std::stof(cells[4]);
        lava.maxZ = std::stof(cells[5]);
        lava.startY = std::stof(cells[6]);
        lava.endY = std::stof(cells[7]);
        lava.delay = std::stof(cells[8]);
        lava.duration = std::stof(cells[9]);

        if (lava.id.empty() ||
            !loadedIds.insert(lava.id).second ||
            lava.damage <= 0 ||
            lava.maxX - lava.minX < kMinimumExtent ||
            lava.maxZ - lava.minZ < kMinimumExtent ||
            lava.endY <= lava.startY ||
            lava.delay < 0.0f ||
            lava.duration < 0.0f)
        {
            std::abort();
        }

        lava.meshId = render.AddMeshMix(kLavaModelPath,
                                        D3DXVECTOR3(0.0f, lava.startY, 0.0f),
                                        kDefaultRotation,
                                        1.0f);
        if (lava.meshId < 0)
        {
            std::abort();
        }

        lava.physicsId = PhysicsLib::PhysicsLib::Load(
            kLavaModelPath.c_str(),
            PhysicsLib::PhysicsLib::ObjectType::PassThrough,
            0.0f);
        if (lava.physicsId < 0)
        {
            std::abort();
        }

        m_lavas.push_back(lava);
        ApplyTransform(render, m_lavas.back());
    }
}

void LavaRiseManager::Clear()
{
    for (Lava& lava : m_lavas)
    {
        if (m_render != nullptr && lava.meshId >= 0)
        {
            m_render->RemoveMeshMix(lava.meshId);
        }
        if (lava.physicsId >= 0)
        {
            PhysicsLib::PhysicsLib::SetVelocity(
                lava.physicsId,
                D3DXVECTOR3(0.0f, 0.0f, 0.0f));
            PhysicsLib::PhysicsLib::SetTransform(
                lava.physicsId,
                kDisabledPosition,
                kDefaultRotation,
                D3DXVECTOR3(1.0f, 1.0f, 1.0f));
        }
    }
    m_lavas.clear();
}

void LavaRiseManager::Update(NSRender::Render& render,
                             const float deltaSeconds)
{
    if (deltaSeconds <= 0.0f)
    {
        std::abort();
    }

    for (Lava& lava : m_lavas)
    {
        lava.elapsed += deltaSeconds;
        ApplyTransform(render, lava);
    }
}

int LavaRiseManager::GetContactDamage(
    const D3DXVECTOR3& playerPosition) const
{
    int damage = 0;
    const PhysicsLib::PhysicsLib::ShapeType shapeType =
        PhysicsLib::PhysicsLib::GetShapeType();
    const float radius = PhysicsLib::PhysicsLib::GetCylinderRadius();
    const float height = PhysicsLib::PhysicsLib::GetCylinderHeight();

    for (const Lava& lava : m_lavas)
    {
        if (lava.physicsId < 0)
        {
            continue;
        }

        if (PhysicsLib::PhysicsLib::CheckContactShape(
                lava.physicsId,
                playerPosition,
                shapeType,
                radius,
                height))
        {
            if (lava.damage > damage)
            {
                damage = lava.damage;
            }
        }
    }
    return damage;
}

std::size_t LavaRiseManager::GetLavaCount() const
{
    return m_lavas.size();
}

void LavaRiseManager::ApplyTransform(NSRender::Render& render, Lava& lava)
{
    float progress = 0.0f;
    if (lava.elapsed > lava.delay)
    {
        if (lava.duration <= 0.0f)
        {
            progress = 1.0f;
        }
        else
        {
            progress = (lava.elapsed - lava.delay) / lava.duration;
            progress = (std::max)(0.0f, (std::min)(progress, 1.0f));
        }
    }

    const float currentY = lava.startY +
                           (lava.endY - lava.startY) * progress;
    const D3DXVECTOR3 position(
        (lava.minX + lava.maxX) * 0.5f,
        currentY,
        (lava.minZ + lava.maxZ) * 0.5f);
    const D3DXVECTOR3 scale(
        (lava.maxX - lava.minX) / kBasePlaneSize,
        1.0f,
        (lava.maxZ - lava.minZ) / kBasePlaneSize);
    const D3DXMATRIX worldMatrix = BuildLavaWorldMatrix(position, scale);

    render.SetMeshMixWorldMatrix(lava.meshId, worldMatrix);
    PhysicsLib::PhysicsLib::SetTransform(lava.physicsId,
                                         position,
                                         kDefaultRotation,
                                         scale);
}
