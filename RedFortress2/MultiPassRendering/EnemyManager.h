#pragma once

#include "Enemy.h"
#include <d3dx9.h>
#include <string>
#include <unordered_map>
#include <vector>

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
    void RegisterEnemyTypes();
    void RegisterEnemyType(const std::wstring& type,
                           const std::wstring& folderName);
    void AdjustMeshIdsAfterRemoval(int removedMeshId);

    std::vector<Enemy> m_enemies;

    struct EnemyTypeInfo
    {
        std::wstring meshPath;
        std::wstring animCsvPath;
        float scale = 1.0f;
        int maxHp = 10;
        float moveSpeed = 2.5f;
        float viewDistance = 5.0f;
        float contactRadius = 0.5f;
        float height = 0.5f;
    };

    bool TryGetTypeInfo(const std::wstring& type, EnemyTypeInfo* typeInfo) const;
    std::unordered_map<std::wstring, EnemyTypeInfo> m_typeInfoMap;
};
