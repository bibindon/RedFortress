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

private:
    void Spawn(NSRender::Render& render, const D3DXVECTOR3& position);

    std::vector<Enemy> m_enemies;
    std::wstring m_wolfMeshPath = L"res\\model2\\separatedAnim\\wolfAnim.x";
    std::wstring m_wolfAnimCsvPath = L"res\\model2\\separatedAnim\\wolfAnim.csv";
};
