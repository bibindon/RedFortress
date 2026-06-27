#pragma once

#include <d3dx9.h>
#include <string>

namespace NSRender
{
class Render;
}

class DestructibleManager;
class InventoryManager;

class PickupManager
{
public:
    void Initialize(NSRender::Render& render, InventoryManager& inventory);
    void Clear();
    void LoadForStage(const std::wstring& starCsvPath, const std::wstring& speedUpCsvPath);
    void ResetTemporaryEffects();
    void ResetPlayerEffects();
    void UpdateTimers();
    void UpdatePickups(const D3DXVECTOR3& playerPosition,
                       int playerMeshId,
                       DestructibleManager& destructibleManager);

    void ActivateStar(int playerMeshId);
    void AddSpeedLevel();
    void SetSpeedLevel(int speedLevel);
    int GetSpeedLevel() const;
    int GetMaxSpeedLevel() const;
    bool IsStarActive() const;
    float GetRunSpeedMultiplier() const;

private:
    bool LoadPickupPosition(const std::wstring& csvPath, D3DXVECTOR3* outPosition) const;

    NSRender::Render* m_render = nullptr;
    InventoryManager* m_inventory = nullptr;
    int m_starPowerupFrames = 0;
    int m_starMeshId = -1;
    int m_speedUpMeshId = -1;
    int m_speedLevel = 1;
    D3DXVECTOR3 m_starPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_speedUpPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
};
