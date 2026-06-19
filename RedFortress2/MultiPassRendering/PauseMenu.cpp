#include "PauseMenu.h"

#include <array>
#include <string>
#include <vector>

#include "InventoryManager.h"
#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"

namespace
{
const std::wstring kMenuMaskPath = L"res\\2D_Image\\menu_mask.png";
const int kMaskedGaussianSampleSize = 25;
const UINT kTextColor = D3DCOLOR_RGBA(255, 255, 255, 245);
const UINT kSubTextColor = D3DCOLOR_RGBA(225, 235, 255, 230);
const UINT kSelectedTextColor = D3DCOLOR_RGBA(255, 220, 110, 255);
const UINT kInactiveTextColor = D3DCOLOR_RGBA(190, 200, 220, 210);
const std::array<const wchar_t*, 5> kTopMenuFirstRow =
{
    L"アイテム",
    L"武器",
    L"装備",
    L"クラフト",
    L"クエスト"
};
const std::array<const wchar_t*, 4> kTopMenuSecondRow =
{
    L"マップ",
    L"図鑑",
    L"セーブ",
    L"設定"
};
const int kTopMenuItemWidth = 240;
const int kTopMenuItemHeight = 44;
const int kTopMenuItemInterval = 250;
const int kTopMenuFirstRowX = 170;
const int kTopMenuSecondRowX = 295;
const int kTopMenuFirstRowY = 180;
const int kTopMenuSecondRowY = 230;
const int kTopMenuCount = 9;
const int kItemMenuIndex = 0;
const int kWeaponMenuIndex = 1;
const std::size_t kVisibleItemCount = 11;
const int kItemListX = 205;
const int kItemListY = 350;
const int kItemListLineHeight = 34;
const std::wstring kItemCsvPath = L"res\\script\\hoshigirl_item_ideas.csv";
const std::wstring kWeaponCsvPath = L"res\\script\\hoshigirl_weapon_ideas.csv";
}

void PauseMenu::Initialize(NSRender::Render& render,
                           bool& mouseCursorVisible,
                           InventoryManager& inventory)
{
    m_render = &render;
    m_mouseCursorVisible = &mouseCursorVisible;
    m_inventory = &inventory;
    LoadItems();
    LoadWeapons();
}

void PauseMenu::Toggle()
{
    if (m_isOpen)
    {
        Close();
        return;
    }

    Open();
}

void PauseMenu::Open()
{
    if (m_render == nullptr)
    {
        return;
    }

    m_isOpen = true;
    m_skipInputFrame = true;
    m_focusArea = FocusArea::TopMenu;
    m_selectedTopMenuIndex = 0;
    m_activeTopMenuIndex = -1;
    m_selectedItemIndex = 0;
    m_itemScrollOffset = 0;
    m_selectedWeaponIndex = 0;
    m_weaponScrollOffset = 0;
    m_render->SetSceneUpdatePaused(true);
    m_render->SetPostEffectMaskedGaussianFilter(true);
    m_render->SetPostEffectMaskedGaussianMaskPath(kMenuMaskPath);
    m_render->SetPostEffectMaskedGaussianSampleSize(kMaskedGaussianSampleSize);
    SetMouseCursorVisible(true);
}

void PauseMenu::Close()
{
    if (m_render != nullptr)
    {
        m_render->SetPostEffectMaskedGaussianFilter(false);
        m_render->SetSceneUpdatePaused(false);
    }

    m_isOpen = false;
}

void PauseMenu::Update()
{
    if (!m_isOpen)
    {
        return;
    }

    if (m_skipInputFrame)
    {
        m_skipInputFrame = false;
        return;
    }

    if (m_focusArea == FocusArea::ItemList)
    {
        UpdateItemList();
        return;
    }

    if (m_focusArea == FocusArea::WeaponList)
    {
        UpdateWeaponList();
        return;
    }

    UpdateTopMenu();
}

void PauseMenu::LoadItems()
{
    m_items.clear();

    std::vector<std::vector<std::wstring>> csvData;
    try
    {
        csvData = csv::Read(NSRender::Util::GetExeDir() + kItemCsvPath);
    }
    catch (...)
    {
        return;
    }

    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.size() < 6 || row.at(0) == L"ID")
        {
            continue;
        }

        ItemData item;
        item.id = row.at(0);
        item.name = row.at(1);
        item.category = row.at(2);
        item.acquisition = row.at(3);
        item.primaryUse = row.at(4);
        item.description = row.at(5);
        m_items.push_back(item);
    }
}

void PauseMenu::LoadWeapons()
{
    m_weapons.clear();

    std::vector<std::vector<std::wstring>> csvData;
    try
    {
        csvData = csv::Read(NSRender::Util::GetExeDir() + kWeaponCsvPath);
    }
    catch (...)
    {
        return;
    }

    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.size() < 6 || row.at(0) == L"ID")
        {
            continue;
        }

        WeaponData weapon;
        weapon.id = row.at(0);
        weapon.name = row.at(1);
        weapon.category = row.at(2);
        weapon.acquisition = row.at(3);
        weapon.feature = row.at(4);
        weapon.description = row.at(5);
        m_weapons.push_back(weapon);
    }
}

void PauseMenu::UpdateTopMenu()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LEFT))
    {
        --m_selectedTopMenuIndex;
        if (m_selectedTopMenuIndex < 0)
        {
            m_selectedTopMenuIndex = kTopMenuCount - 1;
        }
        m_activeTopMenuIndex = -1;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RIGHT))
    {
        ++m_selectedTopMenuIndex;
        if (m_selectedTopMenuIndex >= kTopMenuCount)
        {
            m_selectedTopMenuIndex = 0;
        }
        m_activeTopMenuIndex = -1;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
    {
        m_activeTopMenuIndex = m_selectedTopMenuIndex;
        if (m_activeTopMenuIndex == kItemMenuIndex)
        {
            m_focusArea = FocusArea::ItemList;
        }
        else if (m_activeTopMenuIndex == kWeaponMenuIndex)
        {
            m_focusArea = FocusArea::WeaponList;
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        Close();
    }
}

void PauseMenu::UpdateItemList()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        m_focusArea = FocusArea::TopMenu;
        return;
    }

    const std::vector<std::size_t> ownedItems = GetOwnedItemIndices();
    if (ownedItems.empty())
    {
        return;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP))
    {
        if (m_selectedItemIndex > 0)
        {
            --m_selectedItemIndex;
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN))
    {
        if (m_selectedItemIndex + 1 < ownedItems.size())
        {
            ++m_selectedItemIndex;
        }
    }

    EnsureSelectedItemVisible();
}

void PauseMenu::EnsureSelectedItemVisible()
{
    if (m_selectedItemIndex < m_itemScrollOffset)
    {
        m_itemScrollOffset = m_selectedItemIndex;
    }

    if (m_selectedItemIndex >= m_itemScrollOffset + kVisibleItemCount)
    {
        m_itemScrollOffset = m_selectedItemIndex - kVisibleItemCount + 1;
    }
}

void PauseMenu::UpdateWeaponList()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        m_focusArea = FocusArea::TopMenu;
        return;
    }

    const std::vector<std::size_t> ownedWeapons = GetOwnedWeaponIndices();
    if (ownedWeapons.empty())
    {
        return;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP))
    {
        if (m_selectedWeaponIndex > 0)
        {
            --m_selectedWeaponIndex;
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN))
    {
        if (m_selectedWeaponIndex + 1 < ownedWeapons.size())
        {
            ++m_selectedWeaponIndex;
        }
    }

    EnsureSelectedWeaponVisible();
}

void PauseMenu::EnsureSelectedWeaponVisible()
{
    if (m_selectedWeaponIndex < m_weaponScrollOffset)
    {
        m_weaponScrollOffset = m_selectedWeaponIndex;
    }

    if (m_selectedWeaponIndex >= m_weaponScrollOffset + kVisibleItemCount)
    {
        m_weaponScrollOffset = m_selectedWeaponIndex - kVisibleItemCount + 1;
    }
}

void PauseMenu::Render(const std::wstring& stageName)
{
    if (!m_isOpen || m_render == nullptr)
    {
        return;
    }

    if (m_stageNameFontId < 0)
    {
        m_stageNameFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 30, kTextColor);
    }

    if (m_menuItemFontId < 0)
    {
        m_menuItemFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 26, kTextColor);
    }

    if (m_qualityFontId < 0)
    {
        m_qualityFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 20, kSubTextColor);
    }

    m_render->DrawTextExCenter(m_stageNameFontId,
                               stageName,
                               960,
                               115,
                               560,
                               60,
                               kTextColor);

    RenderTopMenu();

    if (m_activeTopMenuIndex == kItemMenuIndex)
    {
        RenderItemPanel();
        return;
    }

    if (m_activeTopMenuIndex == kWeaponMenuIndex)
    {
        RenderWeaponPanel();
        return;
    }

    m_render->DrawTextExCenter(m_qualityFontId,
                               L"グラフィック設定",
                               1040,
                               525,
                               180,
                               44,
                               kSubTextColor);

    m_render->DrawTextExCenter(m_qualityFontId,
                               L"低",
                               1235,
                               525,
                               70,
                               44,
                               kSubTextColor);

    m_render->DrawTextExCenter(m_qualityFontId,
                               L"中",
                               1320,
                               525,
                               70,
                               44,
                               kSubTextColor);

    m_render->DrawTextExCenter(m_qualityFontId,
                               L"高",
                               1405,
                               525,
                               70,
                               44,
                               kSubTextColor);

    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"拠点に戻る",
                               1180,
                               760,
                               330,
                               56,
                               kTextColor);
}

void PauseMenu::RenderTopMenu()
{
    for (std::size_t i = 0; i < kTopMenuFirstRow.size(); ++i)
    {
        const int menuIndex = static_cast<int>(i);
        const int x = kTopMenuFirstRowX + menuIndex * kTopMenuItemInterval;
        UINT color = kInactiveTextColor;
        if (menuIndex == m_selectedTopMenuIndex)
        {
            color = kSelectedTextColor;
        }

        m_render->DrawTextExCenter(m_qualityFontId,
                                   kTopMenuFirstRow[i],
                                   x,
                                   kTopMenuFirstRowY,
                                   kTopMenuItemWidth,
                                   kTopMenuItemHeight,
                                   color);
    }

    for (std::size_t i = 0; i < kTopMenuSecondRow.size(); ++i)
    {
        const int menuIndex = static_cast<int>(i) + static_cast<int>(kTopMenuFirstRow.size());
        const int x = kTopMenuSecondRowX + static_cast<int>(i) * kTopMenuItemInterval;
        UINT color = kInactiveTextColor;
        if (menuIndex == m_selectedTopMenuIndex)
        {
            color = kSelectedTextColor;
        }

        m_render->DrawTextExCenter(m_qualityFontId,
                                   kTopMenuSecondRow[i],
                                   x,
                                   kTopMenuSecondRowY,
                                   kTopMenuItemWidth,
                                   kTopMenuItemHeight,
                                   color);
    }
}

void PauseMenu::RenderItemPanel()
{
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"アイテム一覧",
                               170,
                               300,
                               560,
                               44,
                               kTextColor);

    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"アイテム詳細",
                               820,
                               300,
                               600,
                               44,
                               kTextColor);

    const std::vector<std::size_t> ownedItems = GetOwnedItemIndices();
    if (ownedItems.empty())
    {
        m_render->DrawTextEx(m_qualityFontId,
                             L"所持しているアイテムはありません。",
                             kItemListX,
                             kItemListY,
                             kSubTextColor);
        return;
    }

    const std::size_t visibleEnd = m_itemScrollOffset + kVisibleItemCount;
    std::size_t itemEnd = visibleEnd;
    if (itemEnd > ownedItems.size())
    {
        itemEnd = ownedItems.size();
    }

    for (std::size_t i = m_itemScrollOffset; i < itemEnd; ++i)
    {
        std::wstring prefix = L"  ";
        UINT color = kSubTextColor;
        if (i == m_selectedItemIndex)
        {
            prefix = L"> ";
            color = kSelectedTextColor;
        }

        const int lineIndex = static_cast<int>(i - m_itemScrollOffset);
        const ItemData& item = m_items.at(ownedItems.at(i));
        m_render->DrawTextEx(m_qualityFontId,
                             prefix + item.name,
                             kItemListX,
                             kItemListY + lineIndex * kItemListLineHeight,
                             color);
    }

    const std::wstring positionText = std::to_wstring(m_selectedItemIndex + 1) +
                                      L" / " +
                                      std::to_wstring(ownedItems.size());
    m_render->DrawTextExCenter(m_qualityFontId,
                               positionText,
                               170,
                               730,
                               560,
                               36,
                               kSubTextColor);

    const ItemData& selectedItem = m_items.at(ownedItems.at(m_selectedItemIndex));
    const int detailX = 850;
    m_render->DrawTextEx(m_menuItemFontId,
                         selectedItem.name,
                         detailX,
                         365,
                         kSelectedTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"分類：" + selectedItem.category,
                         detailX,
                         430,
                         kSubTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"入手方法：" + selectedItem.acquisition,
                         detailX,
                         475,
                         kSubTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"主な用途：" + selectedItem.primaryUse,
                         detailX,
                         520,
                         kSubTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"所持数：" + std::to_wstring(m_inventory->GetItemCount(selectedItem.id)),
                         detailX,
                         565,
                         kSubTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"説明",
                         detailX,
                         620,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         selectedItem.description,
                         detailX,
                         665,
                         kSubTextColor);
}

void PauseMenu::RenderWeaponPanel()
{
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"武器一覧",
                               170,
                               300,
                               560,
                               44,
                               kTextColor);

    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"武器詳細",
                               820,
                               300,
                               600,
                               44,
                               kTextColor);

    const std::vector<std::size_t> ownedWeapons = GetOwnedWeaponIndices();
    if (ownedWeapons.empty())
    {
        m_render->DrawTextEx(m_qualityFontId,
                             L"所持している武器はありません。",
                             kItemListX,
                             kItemListY,
                             kSubTextColor);
        return;
    }

    const std::size_t visibleEnd = m_weaponScrollOffset + kVisibleItemCount;
    std::size_t weaponEnd = visibleEnd;
    if (weaponEnd > ownedWeapons.size())
    {
        weaponEnd = ownedWeapons.size();
    }

    for (std::size_t i = m_weaponScrollOffset; i < weaponEnd; ++i)
    {
        std::wstring prefix = L"  ";
        UINT color = kSubTextColor;
        if (i == m_selectedWeaponIndex)
        {
            prefix = L"> ";
            color = kSelectedTextColor;
        }

        const int lineIndex = static_cast<int>(i - m_weaponScrollOffset);
        const WeaponData& weapon = m_weapons.at(ownedWeapons.at(i));
        m_render->DrawTextEx(m_qualityFontId,
                             prefix + weapon.name,
                             kItemListX,
                             kItemListY + lineIndex * kItemListLineHeight,
                             color);
    }

    const std::wstring positionText = std::to_wstring(m_selectedWeaponIndex + 1) +
                                      L" / " +
                                      std::to_wstring(ownedWeapons.size());
    m_render->DrawTextExCenter(m_qualityFontId,
                               positionText,
                               170,
                               730,
                               560,
                               36,
                               kSubTextColor);

    const WeaponData& selectedWeapon = m_weapons.at(ownedWeapons.at(m_selectedWeaponIndex));
    const int detailX = 850;
    m_render->DrawTextEx(m_menuItemFontId,
                         selectedWeapon.name,
                         detailX,
                         365,
                         kSelectedTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"分類：" + selectedWeapon.category,
                         detailX,
                         430,
                         kSubTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"入手方法：" + selectedWeapon.acquisition,
                         detailX,
                         475,
                         kSubTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"特徴：" + selectedWeapon.feature,
                         detailX,
                         520,
                         kSubTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"所持数：" + std::to_wstring(m_inventory->GetWeaponCount(selectedWeapon.id)),
                         detailX,
                         565,
                         kSubTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"説明",
                         detailX,
                         620,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         selectedWeapon.description,
                         detailX,
                         665,
                         kSubTextColor);
}

std::vector<std::size_t> PauseMenu::GetOwnedItemIndices() const
{
    std::vector<std::size_t> indices;
    if (m_inventory == nullptr)
    {
        return indices;
    }

    for (std::size_t i = 0; i < m_items.size(); ++i)
    {
        if (m_inventory->GetItemCount(m_items.at(i).id) > 0)
        {
            indices.push_back(i);
        }
    }

    return indices;
}

std::vector<std::size_t> PauseMenu::GetOwnedWeaponIndices() const
{
    std::vector<std::size_t> indices;
    if (m_inventory == nullptr)
    {
        return indices;
    }

    for (std::size_t i = 0; i < m_weapons.size(); ++i)
    {
        if (m_inventory->GetWeaponCount(m_weapons.at(i).id) > 0)
        {
            indices.push_back(i);
        }
    }

    return indices;
}

bool PauseMenu::IsOpen() const
{
    return m_isOpen;
}

bool PauseMenu::BlocksGameInput() const
{
    return m_isOpen;
}

void PauseMenu::SetMouseCursorVisible(bool visible)
{
    if (m_mouseCursorVisible != nullptr)
    {
        *m_mouseCursorVisible = visible;
    }

    InputDevice::Mouse::SetVisible(visible);
}
