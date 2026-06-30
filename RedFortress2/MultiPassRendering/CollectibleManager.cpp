#include "CollectibleManager.h"

#include "DestructibleManager.h"
#include "InventoryManager.h"
#include "GameAudio.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"
#include <cmath>
#include <fstream>
#include <utility>

namespace
{
const std::wstring kCollectibleModelPath = L"res\\model\\itemIconMaterial\\itemIconMaterial.x";
const std::wstring kBombCapacityUpItemId = L"bomb_capacity_up";
const std::wstring kBusterRapidUpItemId = L"buster_rapid_up";
const float kCollectDistance = 0.55f;
const float kBlockedByDestructibleHorizontalDistance = 0.75f;
const float kBlockedByDestructibleVerticalDistance = 1.2f;

bool IsTemporaryPowerUpItem(const std::wstring& itemId)
{
    if (itemId == kBombCapacityUpItemId)
    {
        return true;
    }

    if (itemId == kBusterRapidUpItemId)
    {
        return true;
    }

    return false;
}
}

void CollectibleManager::Initialize(NSRender::Render& render,
                                    InventoryManager& inventory)
{
    m_render = &render;
    m_inventory = &inventory;
}

void CollectibleManager::LoadForStage(const std::wstring& csvPath)
{
    Clear();
    if (m_render == nullptr || m_inventory == nullptr || csvPath.empty())
    {
        return;
    }

    std::vector<std::vector<std::wstring>> csvData;
    const std::wstring fullCsvPath = NSRender::Util::GetExeDir() + csvPath;
    std::wifstream file(fullCsvPath);
    if (!file.is_open())
    {
        return;
    }
    file.close();

    try
    {
        csvData = csv::Read(fullCsvPath);
    }
    catch (...)
    {
        return;
    }

    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.size() < 7 || row.at(0) == L"CollectibleID")
        {
            continue;
        }

        Collectible collectible;
        collectible.collectibleId = row.at(0);
        if (row.at(1) == L"Weapon")
        {
            collectible.type = CollectibleType::Weapon;
            if (m_inventory->IsWeaponCollectibleCollected(collectible.collectibleId))
            {
                continue;
            }
        }
        else if (row.at(1) == L"Item")
        {
            collectible.type = CollectibleType::Item;
        }
        else
        {
            continue;
        }

        try
        {
            collectible.dataId = row.at(2);
            collectible.position.x = std::stof(row.at(3));
            collectible.position.y = std::stof(row.at(4));
            collectible.position.z = std::stof(row.at(5));
            collectible.scale = std::stof(row.at(6));
        }
        catch (...)
        {
            continue;
        }

        collectible.renderId = m_render->AddMeshMix(NSRender::Util::GetExeDir() + kCollectibleModelPath,
                                                     collectible.position,
                                                     D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                     collectible.scale,
                                                     -1.0f,
                                                     false,
                                                     false,
                                                     false);
        m_collectibles.push_back(collectible);
    }
}

void CollectibleManager::Update(const D3DXVECTOR3& playerPosition,
                                const DestructibleManager& destructibleManager)
{
    for (Collectible& collectible : m_collectibles)
    {
        if (collectible.renderId < 0)
        {
            continue;
        }

        if (IsBlockedByAliveDestructible(collectible.position, destructibleManager))
        {
            continue;
        }

        const D3DXVECTOR3 difference = playerPosition - collectible.position;
        if (D3DXVec3Length(&difference) <= kCollectDistance)
        {
            Collect(collectible);
        }
    }
}

bool CollectibleManager::IsBlockedByAliveDestructible(const D3DXVECTOR3& position,
                                                      const DestructibleManager& destructibleManager) const
{
    const std::vector<DestructibleObject>& objects = destructibleManager.GetObjects();
    for (const DestructibleObject& object : objects)
    {
        if (object.isDead || object.hp <= 0)
        {
            continue;
        }

        const float dx = position.x - object.position.x;
        const float dz = position.z - object.position.z;
        const float horizontalDistanceSq = (dx * dx) + (dz * dz);
        const float blockedDistanceSq = kBlockedByDestructibleHorizontalDistance *
                                        kBlockedByDestructibleHorizontalDistance;
        if (horizontalDistanceSq > blockedDistanceSq)
        {
            continue;
        }

        const float dy = position.y - object.position.y;
        if (fabsf(dy) <= kBlockedByDestructibleVerticalDistance)
        {
            return true;
        }
    }

    return false;
}

void CollectibleManager::Collect(Collectible& collectible)
{
    if (m_render == nullptr || m_inventory == nullptr)
    {
        return;
    }

    if (collectible.type == CollectibleType::Weapon)
    {
        m_inventory->AddWeapon(collectible.dataId);
        m_inventory->MarkWeaponCollectibleCollected(collectible.collectibleId);
    }
    else
    {
        if (!IsTemporaryPowerUpItem(collectible.dataId))
        {
            m_inventory->AddItem(collectible.dataId);
            m_inventory->Save();
        }

        if (m_itemCollectedCallback)
        {
            m_itemCollectedCallback(collectible.dataId, 1);
        }
    }

    if (collectible.type == CollectibleType::Weapon)
    {
        m_inventory->Save();
    }

    GameAudio::PlayItemGet();
    m_render->RemoveMeshMix(collectible.renderId);
    collectible.renderId = -1;
}

void CollectibleManager::Clear()
{
    if (m_render != nullptr)
    {
        for (Collectible& collectible : m_collectibles)
        {
            if (collectible.renderId >= 0)
            {
                m_render->RemoveMeshMix(collectible.renderId);
                collectible.renderId = -1;
            }
        }
    }

    m_collectibles.clear();
}

void CollectibleManager::SetItemCollectedCallback(std::function<void(const std::wstring&, int)> callback)
{
    m_itemCollectedCallback = std::move(callback);
}
