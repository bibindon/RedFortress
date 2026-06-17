#pragma once

#include "Enemy.h"
#include <vector>
#include <d3dx9.h>

namespace NSRender
{
class Render;
}

class EnemyManager
{
public:
    void Initialize();
    void Clear(NSRender::Render& render);
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Update(NSRender::Render& render, const D3DXVECTOR3& playerPos, bool playerInvincible);
    void SyncMeshes(NSRender::Render& render);
    void DrawHpBars(NSRender::Render& render);

    std::vector<Enemy>& GetEnemies();
    const std::vector<Enemy>& GetEnemies() const;

    void SpawnAt(NSRender::Render& render, const D3DXVECTOR3& position, const std::wstring& type, float yaw);
    void RemoveEnemy(NSRender::Render& render, std::size_t index);
    void SaveToCsv(const std::wstring& csvPath) const;

private:
    void Spawn(NSRender::Render& render, const D3DXVECTOR3& position, const std::wstring& type, float yaw);

    std::vector<Enemy> m_enemies;

    struct EnemyTypeInfo
    {
        std::wstring meshPath;
        std::wstring animCsvPath;
    };

    EnemyTypeInfo GetTypeInfo(const std::wstring& type) const;

    std::wstring m_wolfMeshPath = L"res\\model2\\separatedAnim\\wolfAnim.x";
    std::wstring m_wolfAnimCsvPath = L"res\\model2\\separatedAnim\\wolfAnim.csv";
};
