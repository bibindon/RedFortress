#include "DestructibleManager.h"

#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"
#include <fstream>
#include <sstream>
#include <string>
#include <random>

namespace
{
    const std::wstring kModelPath = L"res\\model\\cubeWoodSmall\\cube_wood_small.x";
    const std::wstring kRedCubeModelPath = L"res\\model\\cube_red.x";

    std::wstring Trim(const std::wstring& str)
    {
        size_t start = 0;
        while (start < str.size() && (str[start] == L' ' || str[start] == L'\t'))
        {
            ++start;
        }
        size_t end = str.size();
        while (end > start && (str[end - 1] == L' ' || str[end - 1] == L'\t' || str[end - 1] == L'\r'))
        {
            --end;
        }
        return str.substr(start, end - start);
    }

    std::vector<std::wstring> SplitCsvLine(const std::wstring& line)
    {
        std::vector<std::wstring> result;
        std::wstringstream stream(line);
        std::wstring cell;
        while (std::getline(stream, cell, L','))
        {
            result.push_back(Trim(cell));
        }
        return result;
    }

    int GetRandomPercent()
    {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> dist(0, 99);
        return dist(rng);
    }
}

void DestructibleManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
    m_objects.clear();
    m_droppedRedCubes.clear();
}

void DestructibleManager::LoadForStage(NSRender::Render& render, const std::wstring& csvPath)
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

    std::wstring line;
    bool isFirstLine = true;
    while (std::getline(file, line))
    {
        if (isFirstLine)
        {
            isFirstLine = false;
            continue;
        }

        const std::vector<std::wstring> cells = SplitCsvLine(line);
        if (cells.size() < 3)
        {
            continue;
        }

        DestructibleObject obj;
        obj.position.x = std::stof(cells.at(0));
        obj.position.y = std::stof(cells.at(1));
        obj.position.z = std::stof(cells.at(2));

        if (cells.size() >= 4)
        {
            const int hp = std::stoi(cells.at(3));
            obj.maxHp = hp;
            obj.hp = hp;
        }

        obj.meshId = m_render->AddMeshMix(kModelPath,
                                           obj.position,
                                           D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                           1.0f);
        if (obj.meshId >= 0)
        {
            m_objects.push_back(obj);
        }
    }
}

void DestructibleManager::Clear()
{
    if (m_render != nullptr)
    {
        for (auto& obj : m_objects)
        {
            if (obj.meshId >= 0)
            {
                m_render->SetMeshMixDamageFlash(obj.meshId, false);
                m_render->SetMeshMixEnabled(obj.meshId, true);
                m_render->RemoveMeshMix(obj.meshId);
            }
        }
        for (auto& cube : m_droppedRedCubes)
        {
            if (cube.meshId >= 0)
            {
                m_render->RemoveMeshMix(cube.meshId);
            }
        }
    }

    m_objects.clear();
    m_droppedRedCubes.clear();
}

void DestructibleManager::Update(NSRender::Render& render)
{
    for (auto& obj : m_objects)
    {
        if (obj.isDead || obj.meshId < 0)
        {
            continue;
        }

        if (obj.blinkFrames > 0)
        {
            --obj.blinkFrames;
            const bool flash = ((obj.blinkFrames / kDestructibleBlinkInterval) % 2) == 0;
            render.SetMeshMixEnabled(obj.meshId, true);
            render.SetMeshMixDamageFlash(obj.meshId, flash);

            if (obj.blinkFrames == 0)
            {
                render.SetMeshMixDamageFlash(obj.meshId, false);
                if (obj.hp <= 0)
                {
                    obj.isDead = true;
                    render.RemoveMeshMix(obj.meshId);
                    TryDropItem(render, obj.position);
                }
                else
                {
                    render.SetMeshMixEnabled(obj.meshId, true);
                }
            }
        }
    }

    for (auto& cube : m_droppedRedCubes)
    {
        if (cube.meshId < 0)
        {
            continue;
        }

        if (cube.pickupWaitFrames > 0)
        {
            --cube.pickupWaitFrames;
        }
    }
}

const DestructibleObject* DestructibleManager::FindInAttackRange(
    const D3DXVECTOR3& playerPos,
    const float playerYaw,
    const float range,
    const float halfAngleRadians) const
{
    const D3DXVECTOR3 forward(-sinf(playerYaw), 0.0f, -cosf(playerYaw));
    const DestructibleObject* bestObj = nullptr;
    float bestDot = -1.0f;

    for (const auto& obj : m_objects)
    {
        if (obj.isDead)
        {
            continue;
        }

        D3DXVECTOR3 dir = obj.position - playerPos;
        const float dist = D3DXVec3Length(&dir);
        if (dist > range)
        {
            continue;
        }

        if (D3DXVec3LengthSq(&dir) > 0.0001f)
        {
            D3DXVec3Normalize(&dir, &dir);
        }
        else
        {
            dir = forward;
        }

        const float dot = D3DXVec3Dot(&forward, &dir);
        if (dot > cosf(halfAngleRadians) && dot > bestDot)
        {
            bestDot = dot;
            bestObj = &obj;
        }
    }

    return bestObj;
}

bool DestructibleManager::TryDamage(NSRender::Render& render,
                                     const DestructibleObject& obj,
                                     const int damage)
{
    for (auto& o : m_objects)
    {
        if (&o == &obj)
        {
            o.hp -= damage;
            o.blinkFrames = kDestructibleBlinkFrames;
            render.SetMeshMixEnabled(o.meshId, true);
            render.SetMeshMixDamageFlash(o.meshId, true);
            return true;
        }
    }

    return false;
}

const std::vector<DestructibleObject>& DestructibleManager::GetObjects() const
{
    return m_objects;
}

const std::vector<DroppedRedCube>& DestructibleManager::GetDroppedRedCubes() const
{
    return m_droppedRedCubes;
}

void DestructibleManager::SetStarDropCallback(std::function<void()> callback)
{
    m_starDropCallback = std::move(callback);
}

void DestructibleManager::SetSpeedUpCallback(std::function<void()> callback)
{
    m_speedUpCallback = std::move(callback);
}

void DestructibleManager::RemoveDroppedRedCube(NSRender::Render& render, const std::size_t index)
{
    if (index >= m_droppedRedCubes.size())
    {
        return;
    }

    DroppedRedCube& cube = m_droppedRedCubes.at(index);
    if (cube.meshId >= 0)
    {
        render.RemoveMeshMix(cube.meshId);
        cube.meshId = -1;
    }
}

void DestructibleManager::TryDropItem(NSRender::Render& render, const D3DXVECTOR3& pos)
{
    const int r = GetRandomPercent();

    if (r < 80)
    {
        return;
    }

    if (r < 87)
    {
        if (m_starDropCallback)
        {
            m_starDropCallback();
        }
        return;
    }

    if (r < 94)
    {
        if (m_speedUpCallback)
        {
            m_speedUpCallback();
        }
        return;
    }

    DroppedRedCube cube;
    cube.position = D3DXVECTOR3(pos.x, pos.y + 1.0f, pos.z);
    cube.pickupWaitFrames = kDroppedRedCubePickupDelayFrames;
    cube.meshId = render.AddMeshMix(kRedCubeModelPath,
                                     cube.position,
                                     D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                     0.5f,
                                     -1.0f,
                                     false,
                                     false,
                                     false);
    if (cube.meshId >= 0)
    {
        m_droppedRedCubes.push_back(cube);
    }
}
