#pragma once

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

class InventoryManager
{
public:
    void Initialize();
    bool Load();
    void Save() const;

    void AddItem(const std::wstring& itemId, int count = 1);
    void AddWeapon(const std::wstring& weaponId, int count = 1);
    int GetItemCount(const std::wstring& itemId) const;
    int GetWeaponCount(const std::wstring& weaponId) const;
    bool HasItems(const std::vector<std::pair<std::wstring, int>>& materials) const;
    bool TryCraft(const std::vector<std::pair<std::wstring, int>>& materials,
                  const std::wstring& resultType,
                  const std::wstring& resultId,
                  int resultCount);

    void MarkWeaponCollectibleCollected(const std::wstring& collectibleId);
    bool IsWeaponCollectibleCollected(const std::wstring& collectibleId) const;

    void Reset();

private:
    void BuildFilePath();
    bool EnsureDirectoryExists() const;
    int GetCount(const std::unordered_map<std::wstring, int>& counts,
                 const std::wstring& id) const;

    std::unordered_map<std::wstring, int> m_itemCounts;
    std::unordered_map<std::wstring, int> m_weaponCounts;
    std::unordered_set<std::wstring> m_collectedWeaponCollectibleIds;
    std::wstring m_filePath;
};
