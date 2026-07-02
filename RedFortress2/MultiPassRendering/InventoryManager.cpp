#include "InventoryManager.h"

#include <Windows.h>
#include <algorithm>
#include <vector>

#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Util.h"

namespace
{
const std::wstring kItemType = L"Item";
const std::wstring kWeaponType = L"Weapon";
const std::wstring kCollectedWeaponType = L"CollectedWeapon";
}

void InventoryManager::Initialize()
{
    BuildFilePath();
}

void InventoryManager::BuildFilePath()
{
    m_filePath = NSRender::Util::GetExeDir() + L"res\\savedata\\inventory.csv";
}

bool InventoryManager::EnsureDirectoryExists() const
{
    const std::size_t lastSlash = m_filePath.find_last_of(L"\\/");
    if (lastSlash == std::wstring::npos)
    {
        return false;
    }

    const std::wstring directory = m_filePath.substr(0, lastSlash);
    const BOOL result = CreateDirectoryW(directory.c_str(), nullptr);
    if (result != 0)
    {
        return true;
    }

    return GetLastError() == ERROR_ALREADY_EXISTS;
}

bool InventoryManager::Load()
{
    m_itemCounts.clear();
    m_weaponCounts.clear();
    m_collectedWeaponCollectibleIds.clear();
    m_unlockedAbilityIds.clear();

    std::vector<std::vector<std::wstring>> csvData;
    try
    {
        csvData = csv::Read(m_filePath);
    }
    catch (...)
    {
        return false;
    }

    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.size() < 3 || row.at(0) == L"Type")
        {
            continue;
        }

        int count = 0;
        try
        {
            count = std::stoi(row.at(2));
        }
        catch (...)
        {
            continue;
        }

        if (row.at(0) == kItemType && count > 0)
        {
            m_itemCounts[row.at(1)] = count;
        }
        else if (row.at(0) == kWeaponType && count > 0)
        {
            m_weaponCounts[row.at(1)] = count;
        }
        else if (row.at(0) == kCollectedWeaponType && count > 0)
        {
            m_collectedWeaponCollectibleIds.insert(row.at(1));
        }
        else if (row.at(0) == L"Ability" && !row.at(1).empty())
        {
            m_unlockedAbilityIds.insert(row.at(1));
        }
    }

    return true;
}

void InventoryManager::Save() const
{
    if (!EnsureDirectoryExists())
    {
        return;
    }

    std::vector<std::vector<std::wstring>> csvData;
    csvData.push_back({ L"Type", L"ID", L"Count" });

    std::vector<std::wstring> ids;
    for (const auto& entry : m_itemCounts)
    {
        ids.push_back(entry.first);
    }
    std::sort(ids.begin(), ids.end());
    for (const std::wstring& id : ids)
    {
        csvData.push_back({ kItemType, id, std::to_wstring(m_itemCounts.at(id)) });
    }

    ids.clear();
    for (const auto& entry : m_weaponCounts)
    {
        ids.push_back(entry.first);
    }
    std::sort(ids.begin(), ids.end());
    for (const std::wstring& id : ids)
    {
        csvData.push_back({ kWeaponType, id, std::to_wstring(m_weaponCounts.at(id)) });
    }

    ids.assign(m_collectedWeaponCollectibleIds.begin(), m_collectedWeaponCollectibleIds.end());
    std::sort(ids.begin(), ids.end());
    for (const std::wstring& id : ids)
    {
        csvData.push_back({ kCollectedWeaponType, id, L"1" });
    }

    csv::Write(m_filePath, csvData);
}

void InventoryManager::UnlockAbility(const std::wstring& abilityId)
{
    if (!abilityId.empty())
    {
        if (m_unlockedAbilityIds.insert(abilityId).second)
        {
            Save();
        }
    }
}

bool InventoryManager::IsAbilityUnlocked(const std::wstring& abilityId) const
{
    return m_unlockedAbilityIds.find(abilityId) != m_unlockedAbilityIds.end();
}

void InventoryManager::AddItem(const std::wstring& itemId, const int count)
{
    if (!itemId.empty() && count > 0)
    {
        m_itemCounts[itemId] += count;
    }
}

void InventoryManager::AddWeapon(const std::wstring& weaponId, const int count)
{
    if (!weaponId.empty() && count > 0)
    {
        m_weaponCounts[weaponId] += count;
    }
}

bool InventoryManager::RemoveItem(const std::wstring& itemId, const int count)
{
    if (itemId.empty() || count <= 0)
    {
        return false;
    }

    auto found = m_itemCounts.find(itemId);
    if (found == m_itemCounts.end() || found->second < count)
    {
        return false;
    }

    found->second -= count;
    if (found->second <= 0)
    {
        m_itemCounts.erase(found);
    }

    Save();
    return true;
}

int InventoryManager::GetItemCount(const std::wstring& itemId) const
{
    return GetCount(m_itemCounts, itemId);
}

int InventoryManager::GetWeaponCount(const std::wstring& weaponId) const
{
    return GetCount(m_weaponCounts, weaponId);
}

bool InventoryManager::HasItems(const std::vector<std::pair<std::wstring, int>>& materials) const
{
    std::unordered_map<std::wstring, int> requiredCounts;
    for (const auto& material : materials)
    {
        if (material.first.empty() || material.second <= 0)
        {
            return false;
        }
        requiredCounts[material.first] += material.second;
    }

    for (const auto& required : requiredCounts)
    {
        if (GetItemCount(required.first) < required.second)
        {
            return false;
        }
    }

    return !materials.empty();
}

bool InventoryManager::TryCraft(const std::vector<std::pair<std::wstring, int>>& materials,
                                const std::wstring& resultType,
                                const std::wstring& resultId,
                                const int resultCount)
{
    if (resultId.empty() || resultCount <= 0 || !HasItems(materials))
    {
        return false;
    }

    if (resultType != kItemType && resultType != kWeaponType)
    {
        return false;
    }

    std::unordered_map<std::wstring, int> requiredCounts;
    for (const auto& material : materials)
    {
        requiredCounts[material.first] += material.second;
    }

    for (const auto& required : requiredCounts)
    {
        auto found = m_itemCounts.find(required.first);
        if (found == m_itemCounts.end())
        {
            return false;
        }

        found->second -= required.second;
        if (found->second <= 0)
        {
            m_itemCounts.erase(found);
        }
    }

    if (resultType == kWeaponType)
    {
        AddWeapon(resultId, resultCount);
    }
    else
    {
        AddItem(resultId, resultCount);
    }

    Save();
    return true;
}

int InventoryManager::GetCount(const std::unordered_map<std::wstring, int>& counts,
                               const std::wstring& id) const
{
    const auto found = counts.find(id);
    if (found == counts.end())
    {
        return 0;
    }

    return found->second;
}

void InventoryManager::MarkWeaponCollectibleCollected(const std::wstring& collectibleId)
{
    if (!collectibleId.empty())
    {
        m_collectedWeaponCollectibleIds.insert(collectibleId);
    }
}

bool InventoryManager::IsWeaponCollectibleCollected(const std::wstring& collectibleId) const
{
    return m_collectedWeaponCollectibleIds.find(collectibleId) !=
           m_collectedWeaponCollectibleIds.end();
}

void InventoryManager::Reset()
{
    m_itemCounts.clear();
    m_weaponCounts.clear();
    m_collectedWeaponCollectibleIds.clear();
}
