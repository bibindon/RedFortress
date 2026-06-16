#include "EnemyManager.h"

#include "../../RedFortressRender/Render/Render.h"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

namespace
{
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
}

void EnemyManager::Initialize()
{
    m_enemies.clear();
}

void EnemyManager::Clear(NSRender::Render& render)
{
    for (const auto& enemy : m_enemies)
    {
        const int meshId = enemy.GetMeshId();
        if (meshId >= 0)
        {
            render.RemoveMeshMixSkinAnim(meshId);
        }
    }
    m_enemies.clear();
}

void EnemyManager::LoadForStage(NSRender::Render& render, int stageNumber)
{
    Clear(render);

    std::wifstream file(L"res\\EnemyPositions.csv");
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
        if (cells.size() < 4)
        {
            continue;
        }

        const int lineStageNumber = std::stoi(cells[0]);
        if (lineStageNumber != stageNumber)
        {
            continue;
        }

        const float posX = std::stof(cells[1]);
        const float posY = std::stof(cells[2]);
        const float posZ = std::stof(cells[3]);
        Spawn(render, D3DXVECTOR3(posX, posY, posZ));
    }
}

void EnemyManager::Update(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    for (auto& enemy : m_enemies)
    {
        enemy.Update(render, playerPos);
    }
}

void EnemyManager::SyncMeshes(NSRender::Render& render)
{
    for (auto& enemy : m_enemies)
    {
        enemy.SyncMesh(render);
    }
}

std::vector<Enemy>& EnemyManager::GetEnemies()
{
    return m_enemies;
}

const std::vector<Enemy>& EnemyManager::GetEnemies() const
{
    return m_enemies;
}

void EnemyManager::Spawn(NSRender::Render& render, const D3DXVECTOR3& position)
{
    const int meshId = render.AddMeshMixSkinAnim(m_wolfMeshPath,
                                                 m_wolfAnimCsvPath,
                                                 position,
                                                 D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                 1.0f,
                                                 NSRender::AnimSetMap(),
                                                 -1.0f,
                                                 false,
                                                 false,
                                                 NSRender::MeshMixSkinAnimLoadMode::Custom);
    if (meshId < 0)
    {
        return;
    }

    Enemy enemy;
    enemy.Initialize(position, meshId);
    m_enemies.push_back(enemy);
}
