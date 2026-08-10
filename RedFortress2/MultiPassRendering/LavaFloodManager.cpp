#include "LavaFloodManager.h"

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

    D3DXMATRIX BuildFloodWorldMatrix(const D3DXVECTOR3& position,
                                     const D3DXVECTOR3& rotation,
                                     const D3DXVECTOR3& scale)
    {
        D3DXMATRIX scaleMatrix;
        D3DXMATRIX rotationMatrix;
        D3DXMATRIX translationMatrix;
        D3DXMatrixScaling(&scaleMatrix, scale.x, scale.y, scale.z);
        D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
                                       rotation.y,
                                       rotation.x,
                                       rotation.z);
        D3DXMatrixTranslation(&translationMatrix,
                              position.x,
                              position.y,
                              position.z);
        return scaleMatrix * rotationMatrix * translationMatrix;
    }
}

void LavaFloodManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
    m_floods.clear();
}

void LavaFloodManager::LoadForStage(NSRender::Render& render,
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
        if (cells.size() != 11 && cells.size() != 13)
        {
            std::abort();
        }

        Flood flood;
        flood.id = cells[0];
        flood.damage = std::stoi(cells[1]);
        flood.anchor.x = std::stof(cells[2]);
        flood.anchor.y = std::stof(cells[3]);
        flood.anchor.z = std::stof(cells[4]);
        if (cells.size() == 13)
        {
            flood.direction.x = std::stof(cells[5]);
            flood.direction.z = std::stof(cells[6]);
            flood.startWidth = std::stof(cells[7]);
            flood.startLength = std::stof(cells[8]);
            flood.endWidth = std::stof(cells[9]);
            flood.endLength = std::stof(cells[10]);
            flood.delay = std::stof(cells[11]);
            flood.duration = std::stof(cells[12]);
        }
        else
        {
            flood.direction.x = 0.0f;
            flood.direction.z = std::stof(cells[5]);
            flood.startWidth = std::stof(cells[6]);
            flood.startLength = std::stof(cells[7]);
            flood.endWidth = std::stof(cells[8]);
            flood.endLength = std::stof(cells[9]);
            flood.delay = 0.0f;
            flood.duration = std::stof(cells[10]);
        }

        const float directionLength = std::sqrt(
            flood.direction.x * flood.direction.x +
            flood.direction.z * flood.direction.z);

        if (flood.id.empty() ||
            !loadedIds.insert(flood.id).second ||
            flood.damage <= 0 ||
            directionLength <= 0.0001f ||
            flood.startWidth < kMinimumExtent ||
            flood.startLength < kMinimumExtent ||
            flood.endWidth < kMinimumExtent ||
            flood.endLength < kMinimumExtent ||
            flood.delay < 0.0f ||
            flood.duration < 0.0f)
        {
            std::abort();
        }

        flood.direction.x /= directionLength;
        flood.direction.z /= directionLength;

        flood.meshId = render.AddMeshMix(kLavaModelPath,
                                         flood.anchor,
                                         kDefaultRotation,
                                         1.0f);
        if (flood.meshId < 0)
        {
            std::abort();
        }

        flood.physicsId = PhysicsLib::PhysicsLib::Load(
            kLavaModelPath.c_str(),
            PhysicsLib::PhysicsLib::ObjectType::PassThrough,
            0.0f);
        if (flood.physicsId < 0)
        {
            std::abort();
        }

        m_floods.push_back(flood);
        ApplyFloodTransform(render, m_floods.back());
    }
}

void LavaFloodManager::Clear()
{
    for (Flood& flood : m_floods)
    {
        if (flood.physicsId >= 0)
        {
            PhysicsLib::PhysicsLib::SetVelocity(
                flood.physicsId,
                D3DXVECTOR3(0.0f, 0.0f, 0.0f));
            PhysicsLib::PhysicsLib::SetTransform(
                flood.physicsId,
                kDisabledPosition,
                kDefaultRotation,
                D3DXVECTOR3(1.0f, 1.0f, 1.0f));
        }
    }
    for (std::vector<Flood>::reverse_iterator iterator = m_floods.rbegin();
         iterator != m_floods.rend();
         ++iterator)
    {
        if (m_render != nullptr && iterator->meshId >= 0)
        {
            m_render->RemoveMeshMix(iterator->meshId);
        }
    }
    m_floods.clear();
}

void LavaFloodManager::Update(NSRender::Render& render,
                              const float deltaSeconds)
{
    if (deltaSeconds <= 0.0f)
    {
        std::abort();
    }

    for (Flood& flood : m_floods)
    {
        flood.elapsed += deltaSeconds;
        const float endTime = flood.delay + flood.duration;
        if (flood.elapsed > endTime)
        {
            flood.elapsed = endTime;
        }
        ApplyFloodTransform(render, flood);
    }
}

int LavaFloodManager::GetContactDamage(
    const D3DXVECTOR3& playerPosition) const
{
    int damage = 0;
    const PhysicsLib::PhysicsLib::ShapeType shapeType =
        PhysicsLib::PhysicsLib::GetShapeType();
    const float radius = PhysicsLib::PhysicsLib::GetCylinderRadius();
    const float height = PhysicsLib::PhysicsLib::GetCylinderHeight();

    for (const Flood& flood : m_floods)
    {
        if (!flood.active || flood.physicsId < 0)
        {
            continue;
        }

        if (PhysicsLib::PhysicsLib::CheckContactShape(
                flood.physicsId,
                playerPosition,
                shapeType,
                radius,
                height))
        {
            if (flood.damage > damage)
            {
                damage = flood.damage;
            }
        }
    }
    return damage;
}

int LavaFloodManager::GetContactDamageForCylinder(
    const D3DXVECTOR3& position,
    const float radius,
    const float height) const
{
    int damage = 0;

    for (const Flood& flood : m_floods)
    {
        if (!flood.active || flood.physicsId < 0)
        {
            continue;
        }

        if (PhysicsLib::PhysicsLib::CheckContactShape(
                flood.physicsId,
                position,
                PhysicsLib::PhysicsLib::ShapeType::Cylinder,
                radius,
                height))
        {
            if (flood.damage > damage)
            {
                damage = flood.damage;
            }
        }
    }

    return damage;
}

std::size_t LavaFloodManager::GetFloodCount() const
{
    return m_floods.size();
}

void LavaFloodManager::ApplyFloodTransform(NSRender::Render& render,
                                           Flood& flood)
{
    if (flood.elapsed < flood.delay)
    {
        flood.active = false;
        render.SetMeshMixEnabled(flood.meshId, false);
        PhysicsLib::PhysicsLib::SetTransform(flood.physicsId,
                                             kDisabledPosition,
                                             kDefaultRotation,
                                             D3DXVECTOR3(1.0f, 1.0f, 1.0f));
        return;
    }

    flood.active = true;
    render.SetMeshMixEnabled(flood.meshId, true);
    const float activeElapsed = flood.elapsed - flood.delay;
    float progress = 1.0f;
    if (flood.duration > 0.0f)
    {
        progress = activeElapsed / flood.duration;
        progress = (std::max)(0.0f, (std::min)(progress, 1.0f));
    }

    const float width = flood.startWidth +
                        (flood.endWidth - flood.startWidth) * progress;
    const float length = flood.startLength +
                         (flood.endLength - flood.startLength) * progress;
    const D3DXVECTOR3 position(
        flood.anchor.x + flood.direction.x * length * 0.5f,
        flood.anchor.y,
        flood.anchor.z + flood.direction.z * length * 0.5f);
    const D3DXVECTOR3 rotation(
        0.0f,
        std::atan2(flood.direction.x, flood.direction.z),
        0.0f);
    const D3DXVECTOR3 scale(width / kBasePlaneSize,
                            1.0f,
                            length / kBasePlaneSize);
    const D3DXMATRIX worldMatrix = BuildFloodWorldMatrix(position,
                                                         rotation,
                                                         scale);

    render.SetMeshMixWorldMatrix(flood.meshId, worldMatrix);
    PhysicsLib::PhysicsLib::SetTransform(flood.physicsId,
                                         position,
                                         rotation,
                                         scale);
}
