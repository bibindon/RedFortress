#pragma once

#include "EnemyBase.h"
#include <d3dx9.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace NSRender
{
class Render;
}

struct FactoryEntry
{
    std::wstring meshPath;
    std::wstring animCsvPath;
    std::wstring bossName;
    float scale;
    std::function<std::unique_ptr<EnemyBase>(const D3DXVECTOR3&, int, float)> create;
};

class EnemyManager
{
public:
    void Initialize();
    void Clear(NSRender::Render& render);
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Update(NSRender::Render& render, const D3DXVECTOR3& playerPos, bool playerInvincible);
    void SyncMeshes(NSRender::Render& render);
    void DrawHpBars(NSRender::Render& render);

    std::vector<std::unique_ptr<EnemyBase>>& GetEnemies();
    const std::vector<std::unique_ptr<EnemyBase>>& GetEnemies() const;

    // 生存中のボス敵を1体返す。ボスがいない場合は nullptr。
    EnemyBase* GetAliveBoss();

    void SpawnAt(NSRender::Render& render, const D3DXVECTOR3& position, const std::wstring& type, float yaw);
    void ReplaceEnemyType(NSRender::Render& render, std::size_t index, const std::wstring& type);
    void RemoveEnemy(NSRender::Render& render, std::size_t index);
    void RemoveAll(NSRender::Render& render);
    void SaveToCsv(const std::wstring& csvPath) const;

private:
    void Spawn(NSRender::Render& render, const D3DXVECTOR3& position, const std::wstring& type, float yaw);
    void RegisterEnemyTypes();

    template<typename T>
    void RegisterEnemyType(const std::wstring& type,
                           const std::wstring& folderName,
                           const std::wstring& meshFileName,
                           const std::wstring& animationFileName,
                           const std::wstring& bossName = L"");

    std::vector<std::unique_ptr<EnemyBase>> m_enemies;
    std::unordered_map<std::wstring, FactoryEntry> m_factory;
};
