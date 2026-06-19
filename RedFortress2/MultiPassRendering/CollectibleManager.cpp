#include "CollectibleManager.h"

#include "InventoryManager.h"
#include "GameAudio.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"

namespace
{
const std::wstring kCollectibleModelPath = L"res\\model\\smallCube\\small_cube.x";
const float kCollectDistance = 0.9f;
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
    try
    {
        csvData = csv::Read(NSRender::Util::GetExeDir() + csvPath);
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

        collectible.renderId = m_render->AddMesh(NSRender::Util::GetExeDir() + kCollectibleModelPath,
                                                  collectible.position,
                                                  D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                  collectible.scale);
        m_collectibles.push_back(collectible);
    }
}

void CollectibleManager::Update(const D3DXVECTOR3& playerPosition)
{
    for (Collectible& collectible : m_collectibles)
    {
        if (collectible.renderId < 0)
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
        m_inventory->AddItem(collectible.dataId);
    }

    m_inventory->Save();
    GameAudio::PlayItemGet();
    m_render->RemoveMesh(collectible.renderId);
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
                m_render->RemoveMesh(collectible.renderId);
                collectible.renderId = -1;
            }
        }
    }

    m_collectibles.clear();
}
