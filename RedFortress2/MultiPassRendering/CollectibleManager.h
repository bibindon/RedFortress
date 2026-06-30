#pragma once

#include <d3dx9.h>
#include <functional>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class InventoryManager;
class DestructibleManager;

class CollectibleManager
{
public:
    void Initialize(NSRender::Render& render, InventoryManager& inventory);
    void LoadForStage(const std::wstring& csvPath);
    void Update(const D3DXVECTOR3& playerPosition, const DestructibleManager& destructibleManager);
    void Clear();
    void SetItemCollectedCallback(std::function<void(const std::wstring&, int)> callback);

private:
    enum class CollectibleType
    {
        Item,
        Weapon
    };

    struct Collectible
    {
        std::wstring collectibleId;
        CollectibleType type = CollectibleType::Item;
        std::wstring dataId;
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        float scale = 1.0f;
        int renderId = -1;
    };

    bool IsBlockedByAliveDestructible(const D3DXVECTOR3& position,
                                      const DestructibleManager& destructibleManager) const;
    void Collect(Collectible& collectible);

    NSRender::Render* m_render = nullptr;
    InventoryManager* m_inventory = nullptr;
    std::function<void(const std::wstring&, int)> m_itemCollectedCallback;
    std::vector<Collectible> m_collectibles;
};
