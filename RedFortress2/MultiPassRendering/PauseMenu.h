#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../../RedFortressRender/Render/WindowManager.h"

namespace NSRender
{
class Render;
}

class InventoryManager;

class PauseMenu
{
public:
    void Initialize(NSRender::Render& render,
                    bool& mouseCursorVisible,
                    InventoryManager& inventory);
    void Toggle();
    void Open();
    void Close();
    void Update();
    void Render(const std::wstring& stageName, int lives);
    bool ConsumeExitRequested();
    bool ConsumeSaveRequested();
    bool IsOpen() const;
    bool BlocksGameInput() const;

private:
    struct ItemData
    {
        std::wstring id;
        std::wstring name;
        std::wstring category;
        std::wstring acquisition;
        std::wstring primaryUse;
        std::wstring description;
    };

    struct WeaponData
    {
        std::wstring id;
        std::wstring name;
        std::wstring category;
        std::wstring acquisition;
        std::wstring feature;
        std::wstring description;
    };

    enum class FocusArea
    {
        TopMenu,
        ItemList,
        WeaponList,
        SettingsPanel
    };

    enum class SettingsRow
    {
        Resolution,
        WindowMode,
        Quality,
    };

    void LoadItems();
    void LoadWeapons();
    void UpdateTopMenu();
    void UpdateItemList();
    void UpdateWeaponList();
    void UpdateSettingsPanel();
    void UpdateExitConfirm();
    void UpdateSaveConfirm();
    void EnsureSelectedItemVisible();
    void EnsureSelectedWeaponVisible();
    void RenderTopMenu();
    void RenderItemPanel();
    void RenderWeaponPanel();
    void RenderExitConfirm();
    void RenderSaveConfirm();
    void RenderSettingsPanel();
    std::wstring BuildResolutionComboText() const;
    std::wstring BuildWindowModeComboText() const;
    std::wstring BuildQualityComboText() const;
    void RefreshSettingsOptions();
    void ApplySelectedResolution();
    void ApplySelectedWindowMode();
    void ApplySelectedQuality();
    static std::wstring WindowModeToLabel(NSRender::eWindowMode mode);
    static std::wstring QualityToLabel(const std::wstring& quality);
    static bool TryGetSettingsRowFromPoint(long x, long y, SettingsRow* outRow);
    static std::wstring FormatResolutionLabel(int width, int height);
    static bool IsSixteenByNine(int width, int height);
    std::vector<std::size_t> GetOwnedItemIndices() const;
    std::vector<std::size_t> GetOwnedWeaponIndices() const;
    void SetMouseCursorVisible(bool visible);
    bool TryGetTopMenuIndexFromPoint(long x, long y, int* outMenuIndex) const;
    static bool IsPointInRect(long x, long y, int left, int top, int width, int height);

    NSRender::Render* m_render = nullptr;
    InventoryManager* m_inventory = nullptr;
    bool* m_mouseCursorVisible = nullptr;
    bool m_isOpen = false;
    bool m_exitRequested = false;
    bool m_showExitConfirm = false;
    bool m_skipInputFrame = false;
    bool m_saveRequested = false;
    bool m_showSaveConfirm = false;
    int m_selectedSaveConfirmIndex = 0;
    FocusArea m_focusArea = FocusArea::TopMenu;
    SettingsRow m_selectedSettingsRow = SettingsRow::Resolution;
    int m_selectedTopMenuIndex = 0;
    int m_activeTopMenuIndex = -1;
    int m_selectedExitConfirmIndex = 0;
    int m_selectedResolutionIndex = 0;
    int m_selectedWindowModeIndex = 0;
    int m_selectedQualityIndex = 0;
    std::size_t m_selectedItemIndex = 0;
    std::size_t m_itemScrollOffset = 0;
    std::vector<ItemData> m_items;
    std::size_t m_selectedWeaponIndex = 0;
    std::size_t m_weaponScrollOffset = 0;
    std::vector<WeaponData> m_weapons;
    std::vector<std::pair<int, int>> m_resolutionOptions;
    int m_stageNameFontId = -1;
    int m_menuItemFontId = -1;
    int m_qualityFontId = -1;
};
