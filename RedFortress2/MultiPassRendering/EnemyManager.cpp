#include "EnemyManager.h"

#include "EnemyWolf.h"
#include "EnemyBigWolf.h"
#include "Enemy3.h"
#include "Enemy4.h"
#include "Enemy5.h"
#include "Enemy6.h"

#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Camera.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

namespace
{
    const std::wstring kHpBarBlackImagePath = L"res\\2D_Image\\hp_black_p.png";
    const std::wstring kHpBarGreenImagePath = L"res\\2D_Image\\hp_green_p.png";
    const int kHpBarWidth = 96;
    const int kHpBarHeight = 3;

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

template<typename T>
void EnemyManager::RegisterEnemyType(const std::wstring& type, const std::wstring& folderName)
{
    const std::wstring basePath = L"res\\model2\\" + folderName + L"\\";
    m_factory[type] = {
        basePath + L"wolfAnim.x",
        basePath + L"wolfAnim.csv",
        T::GetScale(),
        [](const D3DXVECTOR3& pos, int meshId, float yaw) {
            return std::make_unique<T>(pos, meshId, yaw);
        }
    };
}

void EnemyManager::Initialize()
{
    m_enemies.clear();
    RegisterEnemyTypes();
}

void EnemyManager::Clear(NSRender::Render& render)
{
    for (auto it = m_enemies.rbegin(); it != m_enemies.rend(); ++it)
    {
        const int meshId = (*it)->GetMeshId();
        if (meshId >= 0)
        {
            render.RemoveMeshMixSkinAnim(meshId);
        }
    }
    m_enemies.clear();
}

void EnemyManager::LoadForStage(NSRender::Render& render, const std::wstring& csvPath)
{
    Clear(render);

    std::wifstream file(csvPath);
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
        if (cells.size() == 3)
        {
            const float posX = std::stof(cells[0]);
            const float posY = std::stof(cells[1]);
            const float posZ = std::stof(cells[2]);
            Spawn(render, D3DXVECTOR3(posX, posY, posZ), L"wolf", 0.0f);
        }
        else if (cells.size() >= 5)
        {
            const std::wstring type = cells[0];
            const float posX = std::stof(cells[1]);
            const float posY = std::stof(cells[2]);
            const float posZ = std::stof(cells[3]);
            const float rotYDeg = std::stof(cells[4]);
            const float rotY = D3DXToRadian(rotYDeg);
            Spawn(render, D3DXVECTOR3(posX, posY, posZ), type, rotY);
        }
    }
}

void EnemyManager::Update(NSRender::Render& render, const D3DXVECTOR3& playerPos, bool playerInvincible)
{
    for (auto& enemy : m_enemies)
    {
        enemy->Update(render, playerPos, playerInvincible);
    }

    for (auto it = m_enemies.begin(); it != m_enemies.end();)
    {
        if ((*it)->IsReadyToRemove())
        {
            const int meshId = (*it)->GetMeshId();
            if (meshId >= 0)
            {
                render.StopMeshMixSkinAnimBlink(meshId);
                render.RemoveMeshMixSkinAnim(meshId);
            }
            it = m_enemies.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void EnemyManager::SyncMeshes(NSRender::Render& render)
{
    for (auto& enemy : m_enemies)
    {
        enemy->SyncMesh(render);
    }
}

void EnemyManager::DrawHpBars(NSRender::Render& render)
{
    const float scale = static_cast<float>(NSRender::Common::ScreenW()) / static_cast<float>(NSRender::Common::BASE_W);

    for (const auto& enemy : m_enemies)
    {
        if (enemy->IsDead())
        {
            continue;
        }

        const int hp = enemy->GetHp();
        const int maxHp = enemy->GetMaxHp();
        if (maxHp <= 0)
        {
            continue;
        }

        const D3DXVECTOR3 barPos = enemy->GetPosition() + D3DXVECTOR3(0.0f, 1.5f, 0.0f);
        const POINT screenPos = NSRender::Camera::GetScreenPos(barPos);
        if (screenPos.x < 0 || screenPos.y < 0)
        {
            continue;
        }

        const int barWidthScaled = static_cast<int>(kHpBarWidth * scale);
        const float normalizedX = (static_cast<float>(screenPos.x) - barWidthScaled / 2.0f) / static_cast<float>(NSRender::Common::ScreenW());
        const float normalizedY = static_cast<float>(screenPos.y) / static_cast<float>(NSRender::Common::ScreenH());

        const int greenWidth = (kHpBarWidth * hp) / maxHp;

        render.DrawImageAutoResizeSizedRect(kHpBarBlackImagePath,
                                            normalizedX,
                                            normalizedY,
                                            0,
                                            0,
                                            kHpBarWidth,
                                            kHpBarHeight,
                                            scale,
                                            255);

        if (greenWidth > 0)
        {
            render.DrawImageAutoResizeSizedRect(kHpBarGreenImagePath,
                                                normalizedX,
                                                normalizedY,
                                                0,
                                                0,
                                                greenWidth,
                                                kHpBarHeight,
                                                scale,
                                                255);
        }
    }
}

std::vector<std::unique_ptr<EnemyBase>>& EnemyManager::GetEnemies()
{
    return m_enemies;
}

const std::vector<std::unique_ptr<EnemyBase>>& EnemyManager::GetEnemies() const
{
    return m_enemies;
}

void EnemyManager::SpawnAt(NSRender::Render& render, const D3DXVECTOR3& position, const std::wstring& type, float yaw)
{
    Spawn(render, position, type, yaw);
}

void EnemyManager::RemoveEnemy(NSRender::Render& render, std::size_t index)
{
    if (index >= m_enemies.size())
    {
        return;
    }

    const int meshId = m_enemies[index]->GetMeshId();
    if (meshId >= 0)
    {
        render.StopMeshMixSkinAnimBlink(meshId);
        render.RemoveMeshMixSkinAnim(meshId);
    }

    m_enemies.erase(m_enemies.begin() + index);
}

void EnemyManager::RemoveAll(NSRender::Render& render)
{
    for (auto it = m_enemies.rbegin(); it != m_enemies.rend(); ++it)
    {
        const int meshId = (*it)->GetMeshId();
        if (meshId >= 0)
        {
            render.StopMeshMixSkinAnimBlink(meshId);
            render.RemoveMeshMixSkinAnim(meshId);
        }
    }

    m_enemies.clear();
}

void EnemyManager::SaveToCsv(const std::wstring& csvPath) const
{
    std::vector<std::vector<std::wstring>> csvData;
    std::vector<std::wstring> header;
    header.push_back(L"Type");
    header.push_back(L"PosX");
    header.push_back(L"PosY");
    header.push_back(L"PosZ");
    header.push_back(L"RotY");
    csvData.push_back(header);

    for (const auto& enemy : m_enemies)
    {
        if (enemy->IsReadyToRemove())
        {
            continue;
        }

        std::vector<std::wstring> row;
        row.push_back(enemy->GetType());
        row.push_back(std::to_wstring(enemy->GetPosition().x));
        row.push_back(std::to_wstring(enemy->GetPosition().y));
        row.push_back(std::to_wstring(enemy->GetPosition().z));

        const float rotYDeg = enemy->GetYaw() * 180.0f / D3DX_PI;
        row.push_back(std::to_wstring(rotYDeg));

        csvData.push_back(row);
    }

    csv::Write(csvPath, csvData);
}

void EnemyManager::RegisterEnemyTypes()
{
    m_factory.clear();
    RegisterEnemyType<EnemyWolf>(L"wolf", L"separatedAnim");
    RegisterEnemyType<EnemyBigWolf>(L"enemy2", L"Enemy2");
    RegisterEnemyType<Enemy3>(L"enemy3", L"Enemy3");
    RegisterEnemyType<Enemy4>(L"enemy4", L"Enemy4");
    RegisterEnemyType<Enemy5>(L"enemy5", L"Enemy5");
    RegisterEnemyType<Enemy6>(L"enemy6", L"Enemy6");
}

void EnemyManager::Spawn(NSRender::Render& render, const D3DXVECTOR3& position, const std::wstring& type, float yaw)
{
    const auto found = m_factory.find(type);
    if (found == m_factory.end())
    {
        return;
    }

    const FactoryEntry& entry = found->second;
    const int meshId = render.AddMeshMixSkinAnim(entry.meshPath,
                                                  entry.animCsvPath,
                                                  position,
                                                  D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                  entry.scale,
                                                  NSRender::AnimSetMap(),
                                                  -1.0f,
                                                  false,
                                                  false,
                                                  NSRender::MeshMixSkinAnimLoadMode::Custom);
    if (meshId < 0)
    {
        return;
    }

    m_enemies.push_back(entry.create(position, meshId, yaw));
}
