#pragma once

#include <d3dx9.h>
#include <functional>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class DestructibleManager;
class InventoryManager;

enum class StarBlinkMode
{
    YellowWhite,
    PinkWhite,
    CyanWhite
};

class PickupManager
{
public:
    void Initialize(NSRender::Render& render, InventoryManager& inventory);
    void Clear();
    void LoadForStage(const std::wstring& starCsvPath, const std::wstring& speedUpCsvPath);
    void ResetTemporaryEffects();
    void ResetPlayerEffects();
    void RespawnStars();
    void UpdateTimers();
    void UpdatePickups(const D3DXVECTOR3& playerPosition,
                       int playerMeshId,
                       DestructibleManager& destructibleManager);

    void ActivateStar(int playerMeshId);
    bool AddSpeedLevel();
    void SetItemCollectedCallback(std::function<void(const std::wstring&, int)> callback);
    void SetAmmoRecoveredCallback(std::function<void()> callback);
    void SetStarActivatedCallback(std::function<void()> callback);
    void SetStarBlinkMode(StarBlinkMode mode);
    void SetSpeedLevel(int speedLevel);
    int GetSpeedLevel() const;
    int GetMaxSpeedLevel() const;
    bool IsStarActive() const;
    float GetRunSpeedMultiplier() const;

private:
    struct StarPickup
    {
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        int meshId = -1;
        float rotationY = 0.0f;
    };

    bool LoadPickupPosition(const std::wstring& csvPath, D3DXVECTOR3* outPosition) const;
    std::vector<D3DXVECTOR3> LoadPickupPositions(const std::wstring& csvPath) const;
    int AddStarMesh(const D3DXVECTOR3& position) const;

    NSRender::Render* m_render = nullptr;
    InventoryManager* m_inventory = nullptr;
    std::function<void(const std::wstring&, int)> m_itemCollectedCallback;
    std::function<void()> m_ammoRecoveredCallback;
    std::function<void()> m_starActivatedCallback;
    int m_starPowerupFrames = 0;
    int m_speedUpMeshId = -1;
    int m_speedLevel = 1;
    int m_baseSpeedLevel = 1;
    StarBlinkMode m_starBlinkMode = StarBlinkMode::YellowWhite;
    std::vector<StarPickup> m_starPickups;
    D3DXVECTOR3 m_speedUpPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
};
