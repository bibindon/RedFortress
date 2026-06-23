#include "PauseMenu.h"

#include <array>
#include <string>
#include <vector>

#include "InventoryManager.h"
#include "GameAudio.h"
#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Common.h"
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
const std::array<const wchar_t*, 5> kTopMenuItems =
{
    L"アイテム",
    L"武器",
    L"設定",
    L"セーブ",
    L"終了"
};
const int kTopMenuItemWidth = 220;
const int kTopMenuItemHeight = 44;
const int kTopMenuItemInterval = 270;
const int kTopMenuX = 250;
const int kTopMenuY = 200;
const int kTopMenuCount = 5;
const int kItemMenuIndex = 0;
const int kWeaponMenuIndex = 1;
const int kSettingsMenuIndex = 2;
const int kSaveMenuIndex = 3;
const int kExitMenuIndex = 4;
const int kExitConfirmYesIndex = 0;
const int kExitConfirmNoIndex = 1;
const int kSaveConfirmYesIndex = 0;
const int kSaveConfirmNoIndex = 1;
const int kExitConfirmButtonWidth = 150;
const int kExitConfirmButtonHeight = 44;
const int kExitConfirmYesX = 70;
const int kExitConfirmNoX = 240;
const int kExitConfirmY = 790;
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
    m_exitRequested = false;
    m_showExitConfirm = false;
    m_showSaveConfirm = false;
    m_saveRequested = false;
    m_skipInputFrame = true;
    m_focusArea = FocusArea::TopMenu;
    m_selectedSettingsRow = SettingsRow::Resolution;
    m_selectedTopMenuIndex = 0;
    m_activeTopMenuIndex = -1;
    m_selectedExitConfirmIndex = kExitConfirmNoIndex;
    m_selectedResolutionIndex = 0;
    m_selectedWindowModeIndex = 0;
    m_selectedQualityIndex = 0;
    m_selectedItemIndex = 0;
    m_itemScrollOffset = 0;
    m_selectedWeaponIndex = 0;
    m_weaponScrollOffset = 0;
    m_render->SetSceneUpdatePaused(true);
    m_render->SetPostEffectMaskedGaussianFilter(true);
    m_render->SetPostEffectMaskedGaussianMaskPath(kMenuMaskPath);
    m_render->SetPostEffectMaskedGaussianSampleSize(kMaskedGaussianSampleSize);
    RefreshSettingsOptions();
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
    m_showExitConfirm = false;
    m_showSaveConfirm = false;
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

    if (m_focusArea == FocusArea::SettingsPanel)
    {
        UpdateSettingsPanel();
        return;
    }

    if (m_showExitConfirm)
    {
        UpdateExitConfirm();
        return;
    }

    if (m_showSaveConfirm)
    {
        UpdateSaveConfirm();
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
    const int previousIndex = m_selectedTopMenuIndex;
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

    if (m_selectedTopMenuIndex != previousIndex)
    {
        GameAudio::PlayMenuMove();
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
        const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                             static_cast<float>(NSRender::Common::ScreenW());
        const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                             static_cast<float>(NSRender::Common::ScreenH());
        const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
        const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
        int clickedMenuIndex = -1;
        if (TryGetTopMenuIndexFromPoint(baseMouseX, baseMouseY, &clickedMenuIndex))
        {
            if (clickedMenuIndex != m_selectedTopMenuIndex)
            {
                m_selectedTopMenuIndex = clickedMenuIndex;
                m_activeTopMenuIndex = -1;
                GameAudio::PlayMenuMove();
            }
            else
            {
                GameAudio::PlayMenuConfirm();
                m_activeTopMenuIndex = clickedMenuIndex;
                if (m_activeTopMenuIndex == kItemMenuIndex)
                {
                    m_focusArea = FocusArea::ItemList;
                }
                else if (m_activeTopMenuIndex == kWeaponMenuIndex)
                {
                    m_focusArea = FocusArea::WeaponList;
                }
                else if (m_activeTopMenuIndex == kSettingsMenuIndex)
                {
                    RefreshSettingsOptions();
                    m_focusArea = FocusArea::SettingsPanel;
                    m_selectedSettingsRow = SettingsRow::Resolution;
                }
                else if (m_activeTopMenuIndex == kSaveMenuIndex)
                {
                    m_showSaveConfirm = true;
                    m_selectedSaveConfirmIndex = kSaveConfirmNoIndex;
                }
                else if (m_activeTopMenuIndex == kExitMenuIndex)
                {
                    m_showExitConfirm = true;
                    m_selectedExitConfirmIndex = kExitConfirmNoIndex;
                }
            }
            return;
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
    {
        GameAudio::PlayMenuConfirm();
        m_activeTopMenuIndex = m_selectedTopMenuIndex;
        if (m_activeTopMenuIndex == kItemMenuIndex)
        {
            m_focusArea = FocusArea::ItemList;
        }
        else if (m_activeTopMenuIndex == kWeaponMenuIndex)
        {
            m_focusArea = FocusArea::WeaponList;
        }
        else if (m_activeTopMenuIndex == kSettingsMenuIndex)
        {
            RefreshSettingsOptions();
            m_focusArea = FocusArea::SettingsPanel;
            m_selectedSettingsRow = SettingsRow::Resolution;
        }
        else if (m_activeTopMenuIndex == kSaveMenuIndex)
        {
            m_showSaveConfirm = true;
            m_selectedSaveConfirmIndex = kSaveConfirmNoIndex;
        }
        else if (m_activeTopMenuIndex == kExitMenuIndex)
        {
            m_showExitConfirm = true;
            m_selectedExitConfirmIndex = kExitConfirmNoIndex;
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        GameAudio::PlayMenuCancel();
        Close();
    }
}

void PauseMenu::UpdateItemList()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        GameAudio::PlayMenuCancel();
        m_focusArea = FocusArea::TopMenu;
        return;
    }

    const std::vector<std::size_t> ownedItems = GetOwnedItemIndices();
    if (ownedItems.empty())
    {
        return;
    }

    const std::size_t previousIndex = m_selectedItemIndex;
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

    if (m_selectedItemIndex != previousIndex)
    {
        GameAudio::PlayMenuMove();
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
        GameAudio::PlayMenuCancel();
        m_focusArea = FocusArea::TopMenu;
        return;
    }

    const std::vector<std::size_t> ownedWeapons = GetOwnedWeaponIndices();
    if (ownedWeapons.empty())
    {
        return;
    }

    const std::size_t previousIndex = m_selectedWeaponIndex;
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

    if (m_selectedWeaponIndex != previousIndex)
    {
        GameAudio::PlayMenuMove();
    }

    EnsureSelectedWeaponVisible();
}

void PauseMenu::UpdateSettingsPanel()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP))
    {
        if (m_selectedSettingsRow == SettingsRow::Resolution)
        {
            m_selectedSettingsRow = SettingsRow::Quality;
        }
        else if (m_selectedSettingsRow == SettingsRow::WindowMode)
        {
            m_selectedSettingsRow = SettingsRow::Resolution;
        }
        else
        {
            m_selectedSettingsRow = SettingsRow::WindowMode;
        }
        GameAudio::PlayMenuMove();
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN))
    {
        if (m_selectedSettingsRow == SettingsRow::Resolution)
        {
            m_selectedSettingsRow = SettingsRow::WindowMode;
        }
        else if (m_selectedSettingsRow == SettingsRow::WindowMode)
        {
            m_selectedSettingsRow = SettingsRow::Quality;
        }
        else
        {
            m_selectedSettingsRow = SettingsRow::Resolution;
        }
        GameAudio::PlayMenuMove();
    }

    bool applyPrevious = false;
    bool applyNext = false;
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LEFT))
    {
        applyPrevious = true;
    }
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RIGHT) ||
        InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
    {
        applyNext = true;
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
        const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                             static_cast<float>(NSRender::Common::ScreenW());
        const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                             static_cast<float>(NSRender::Common::ScreenH());
        const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
        const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
        SettingsRow clickedRow;
        if (TryGetSettingsRowFromPoint(baseMouseX, baseMouseY, &clickedRow))
        {
            if (clickedRow != m_selectedSettingsRow)
            {
                m_selectedSettingsRow = clickedRow;
                GameAudio::PlayMenuMove();
            }
            else
            {
                applyNext = true;
            }
        }
    }

    if (applyPrevious || applyNext)
    {
        bool changed = false;
        if (m_selectedSettingsRow == SettingsRow::Resolution && !m_resolutionOptions.empty())
        {
            if (applyPrevious)
            {
                --m_selectedResolutionIndex;
                if (m_selectedResolutionIndex < 0)
                {
                    m_selectedResolutionIndex = static_cast<int>(m_resolutionOptions.size()) - 1;
                }
            }
            else
            {
                ++m_selectedResolutionIndex;
                if (m_selectedResolutionIndex >= static_cast<int>(m_resolutionOptions.size()))
                {
                    m_selectedResolutionIndex = 0;
                }
            }
            ApplySelectedResolution();
            changed = true;
        }
        else if (m_selectedSettingsRow == SettingsRow::WindowMode)
        {
            if (applyPrevious)
            {
                --m_selectedWindowModeIndex;
                if (m_selectedWindowModeIndex < 0)
                {
                    m_selectedWindowModeIndex = 1;
                }
            }
            else
            {
                ++m_selectedWindowModeIndex;
                if (m_selectedWindowModeIndex > 1)
                {
                    m_selectedWindowModeIndex = 0;
                }
            }
            ApplySelectedWindowMode();
            changed = true;
        }
        else if (m_selectedSettingsRow == SettingsRow::Quality)
        {
            if (applyPrevious)
            {
                --m_selectedQualityIndex;
                if (m_selectedQualityIndex < 0)
                {
                    m_selectedQualityIndex = 2;
                }
            }
            else
            {
                ++m_selectedQualityIndex;
                if (m_selectedQualityIndex > 2)
                {
                    m_selectedQualityIndex = 0;
                }
            }
            ApplySelectedQuality();
            changed = true;
        }

        if (changed)
        {
            GameAudio::PlayMenuConfirm();
            RefreshSettingsOptions();
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        GameAudio::PlayMenuCancel();
        m_focusArea = FocusArea::TopMenu;
    }
}

void PauseMenu::UpdateExitConfirm()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LEFT))
    {
        m_selectedExitConfirmIndex = kExitConfirmYesIndex;
        GameAudio::PlayMenuMove();
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RIGHT))
    {
        m_selectedExitConfirmIndex = kExitConfirmNoIndex;
        GameAudio::PlayMenuMove();
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
        const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                             static_cast<float>(NSRender::Common::ScreenW());
        const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                             static_cast<float>(NSRender::Common::ScreenH());
        const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
        const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitConfirmYesX,
                          kExitConfirmY,
                          kExitConfirmButtonWidth,
                          kExitConfirmButtonHeight))
        {
            GameAudio::PlayMenuConfirm();
            m_exitRequested = true;
            Close();
            return;
        }

        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitConfirmNoX,
                          kExitConfirmY,
                          kExitConfirmButtonWidth,
                          kExitConfirmButtonHeight))
        {
            GameAudio::PlayMenuCancel();
            m_showExitConfirm = false;
            m_selectedExitConfirmIndex = kExitConfirmNoIndex;
            return;
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
    {
        if (m_selectedExitConfirmIndex == kExitConfirmYesIndex)
        {
            GameAudio::PlayMenuConfirm();
            m_exitRequested = true;
            Close();
            return;
        }

        GameAudio::PlayMenuCancel();
        m_showExitConfirm = false;
        return;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        GameAudio::PlayMenuCancel();
        m_showExitConfirm = false;
        m_selectedExitConfirmIndex = kExitConfirmNoIndex;
    }
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

void PauseMenu::Render(const std::wstring& stageName, const int lives)
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

    const std::wstring livesText = L"残機: " + std::to_wstring(lives);
    m_render->DrawTextExCenter(m_qualityFontId,
                               livesText,
                               960,
                               150,
                               320,
                               40,
                               kSubTextColor);

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

    if (m_activeTopMenuIndex == kSettingsMenuIndex)
    {
        RenderSettingsPanel();
        return;
    }

    if (m_activeTopMenuIndex == kSaveMenuIndex)
    {
        m_render->DrawTextExCenter(m_menuItemFontId,
                                   L"セーブしますか？",
                                   1180,
                                   760,
                                   330,
                                   56,
                                   kTextColor);
        if (m_showSaveConfirm)
        {
            RenderSaveConfirm();
        }
    }

    if (m_activeTopMenuIndex == kExitMenuIndex)
    {
        m_render->DrawTextExCenter(m_menuItemFontId,
                                   L"拠点に戻る",
                                   1180,
                                   760,
                                   330,
                                   56,
                                   kTextColor);
        if (m_showExitConfirm)
        {
            RenderExitConfirm();
        }
    }
}

void PauseMenu::RenderTopMenu()
{
    for (std::size_t i = 0; i < kTopMenuItems.size(); ++i)
    {
        const int menuIndex = static_cast<int>(i);
        const int x = kTopMenuX + menuIndex * kTopMenuItemInterval;
        UINT color = kInactiveTextColor;
        if (menuIndex == m_selectedTopMenuIndex)
        {
            color = kSelectedTextColor;
        }

        m_render->DrawTextExCenter(m_qualityFontId,
                                   kTopMenuItems[i],
                                   x,
                                   kTopMenuY,
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

void PauseMenu::RenderExitConfirm()
{
    UINT yesColor = kInactiveTextColor;
    if (m_selectedExitConfirmIndex == kExitConfirmYesIndex)
    {
        yesColor = kSelectedTextColor;
    }

    UINT noColor = kInactiveTextColor;
    if (m_selectedExitConfirmIndex == kExitConfirmNoIndex)
    {
        noColor = kSelectedTextColor;
    }

    m_render->DrawTextExCenter(m_qualityFontId,
                               L"ゲームを終了しますか？",
                               40,
                               740,
                               380,
                               36,
                               kSubTextColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"はい",
                               kExitConfirmYesX,
                               kExitConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight,
                               yesColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"いいえ",
                               kExitConfirmNoX,
                               kExitConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight,
                               noColor);
}

void PauseMenu::RenderSettingsPanel()
{
    const int labelX = 860;
    const int valueX = 1040;
    const int comboWidth = 420;
    const int comboHeight = 42;
    UINT resolutionColor = kSubTextColor;
    if (m_selectedSettingsRow == SettingsRow::Resolution)
    {
        resolutionColor = kSelectedTextColor;
    }
    UINT windowModeColor = kSubTextColor;
    if (m_selectedSettingsRow == SettingsRow::WindowMode)
    {
        windowModeColor = kSelectedTextColor;
    }
    UINT qualityColor = kSubTextColor;
    if (m_selectedSettingsRow == SettingsRow::Quality)
    {
        qualityColor = kSelectedTextColor;
    }

    m_render->DrawTextEx(m_menuItemFontId,
                         L"解像度",
                         labelX,
                         360,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"16:9限定",
                         labelX,
                         396,
                         kSubTextColor);
    m_render->DrawTextExCenter(m_qualityFontId,
                               L"[ " + BuildResolutionComboText() + L" v ]",
                               valueX,
                               360,
                               comboWidth,
                               comboHeight,
                               resolutionColor);

    m_render->DrawTextEx(m_menuItemFontId,
                         L"表示モード",
                         labelX,
                         455,
                         kTextColor);
    m_render->DrawTextExCenter(m_qualityFontId,
                               L"[ " + BuildWindowModeComboText() + L" v ]",
                               valueX,
                               455,
                               comboWidth,
                               comboHeight,
                               windowModeColor);

    m_render->DrawTextEx(m_menuItemFontId,
                         L"描画品質",
                         labelX,
                         550,
                         kTextColor);
    m_render->DrawTextExCenter(m_qualityFontId,
                               L"[ " + BuildQualityComboText() + L" v ]",
                               valueX,
                               550,
                               comboWidth,
                               comboHeight,
                               qualityColor);

    m_render->DrawTextExCenter(m_qualityFontId,
                               L"↑↓で項目選択  ←→/Enter/クリックで切替  Escで戻る",
                               820,
                               650,
                               640,
                               36,
                               kSubTextColor);
}

std::wstring PauseMenu::BuildResolutionComboText() const
{
    if (m_resolutionOptions.empty() ||
        m_selectedResolutionIndex < 0 ||
        m_selectedResolutionIndex >= static_cast<int>(m_resolutionOptions.size()))
    {
        return L"-";
    }

    const std::pair<int, int>& resolution = m_resolutionOptions.at(m_selectedResolutionIndex);
    return FormatResolutionLabel(resolution.first, resolution.second);
}

std::wstring PauseMenu::BuildWindowModeComboText() const
{
    NSRender::eWindowMode mode = NSRender::eWindowMode::WINDOW;
    if (m_selectedWindowModeIndex == 1)
    {
        mode = NSRender::eWindowMode::BORDERLESS;
    }
    return WindowModeToLabel(mode);
}

std::wstring PauseMenu::BuildQualityComboText() const
{
    std::wstring quality = L"LOW";
    if (m_selectedQualityIndex == 1)
    {
        quality = L"MIDDLE";
    }
    else if (m_selectedQualityIndex == 2)
    {
        quality = L"HIGH";
    }
    return QualityToLabel(quality);
}

void PauseMenu::RefreshSettingsOptions()
{
    m_resolutionOptions.clear();
    if (m_render != nullptr)
    {
        const std::vector<std::pair<int, int>> resolutionList = m_render->GetResolutionList();
        for (const auto& resolution : resolutionList)
        {
            if (IsSixteenByNine(resolution.first, resolution.second))
            {
                m_resolutionOptions.push_back(resolution);
            }
        }
    }

    if (m_resolutionOptions.empty())
    {
        m_resolutionOptions.push_back(std::make_pair(NSRender::Common::ScreenW(),
                                                     NSRender::Common::ScreenH()));
    }

    m_selectedResolutionIndex = 0;
    for (std::size_t i = 0; i < m_resolutionOptions.size(); ++i)
    {
        if (m_resolutionOptions[i].first == NSRender::Common::ScreenW() &&
            m_resolutionOptions[i].second == NSRender::Common::ScreenH())
        {
            m_selectedResolutionIndex = static_cast<int>(i);
            break;
        }
    }

    m_selectedWindowModeIndex = 0;
    if (m_render != nullptr && m_render->GetWindowMode() == NSRender::eWindowMode::BORDERLESS)
    {
        m_selectedWindowModeIndex = 1;
    }

    const std::wstring quality = m_render != nullptr ? m_render->GetRenderQuality() : L"LOW";
    m_selectedQualityIndex = 0;
    if (quality == L"MIDDLE")
    {
        m_selectedQualityIndex = 1;
    }
    else if (quality == L"HIGH")
    {
        m_selectedQualityIndex = 2;
    }
}

void PauseMenu::ApplySelectedResolution()
{
    if (m_render == nullptr ||
        m_selectedResolutionIndex < 0 ||
        m_selectedResolutionIndex >= static_cast<int>(m_resolutionOptions.size()))
    {
        return;
    }

    const std::pair<int, int>& resolution = m_resolutionOptions.at(m_selectedResolutionIndex);
    m_render->ChangeResolution(resolution.first, resolution.second);
}

void PauseMenu::ApplySelectedWindowMode()
{
    if (m_render == nullptr)
    {
        return;
    }

    NSRender::eWindowMode mode = NSRender::eWindowMode::WINDOW;
    if (m_selectedWindowModeIndex == 1)
    {
        mode = NSRender::eWindowMode::BORDERLESS;
    }
    m_render->ChangeWindowMode(mode);
}

void PauseMenu::ApplySelectedQuality()
{
    if (m_render == nullptr)
    {
        return;
    }

    std::wstring quality = L"LOW";
    if (m_selectedQualityIndex == 1)
    {
        quality = L"MIDDLE";
    }
    else if (m_selectedQualityIndex == 2)
    {
        quality = L"HIGH";
    }
    m_render->SetRenderQuality(quality);
}

std::wstring PauseMenu::WindowModeToLabel(const NSRender::eWindowMode mode)
{
    if (mode == NSRender::eWindowMode::BORDERLESS)
    {
        return L"ボーダーレスウィンドウ";
    }

    return L"ウィンドウモード";
}

std::wstring PauseMenu::QualityToLabel(const std::wstring& quality)
{
    if (quality == L"MIDDLE")
    {
        return L"中";
    }

    if (quality == L"HIGH")
    {
        return L"高";
    }

    return L"低";
}

bool PauseMenu::TryGetSettingsRowFromPoint(const long x, const long y, SettingsRow* outRow)
{
    if (outRow == nullptr)
    {
        return false;
    }

    if (IsPointInRect(x, y, 1040, 360, 420, 42))
    {
        *outRow = SettingsRow::Resolution;
        return true;
    }

    if (IsPointInRect(x, y, 1040, 455, 420, 42))
    {
        *outRow = SettingsRow::WindowMode;
        return true;
    }

    if (IsPointInRect(x, y, 1040, 550, 420, 42))
    {
        *outRow = SettingsRow::Quality;
        return true;
    }

    return false;
}

std::wstring PauseMenu::FormatResolutionLabel(const int width, const int height)
{
    return std::to_wstring(width) + L" x " + std::to_wstring(height);
}

bool PauseMenu::IsSixteenByNine(const int width, const int height)
{
    return width > 0 &&
           height > 0 &&
           width * 9 == height * 16;
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

bool PauseMenu::ConsumeExitRequested()
{
    const bool requested = m_exitRequested;
    m_exitRequested = false;
    return requested;
}

bool PauseMenu::ConsumeSaveRequested()
{
    const bool requested = m_saveRequested;
    m_saveRequested = false;
    return requested;
}

void PauseMenu::UpdateSaveConfirm()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LEFT))
    {
        m_selectedSaveConfirmIndex = kSaveConfirmYesIndex;
        GameAudio::PlayMenuMove();
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RIGHT))
    {
        m_selectedSaveConfirmIndex = kSaveConfirmNoIndex;
        GameAudio::PlayMenuMove();
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
        const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                             static_cast<float>(NSRender::Common::ScreenW());
        const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                             static_cast<float>(NSRender::Common::ScreenH());
        const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
        const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitConfirmYesX,
                          kExitConfirmY,
                          kExitConfirmButtonWidth,
                          kExitConfirmButtonHeight))
        {
            GameAudio::PlayMenuConfirm();
            m_saveRequested = true;
            m_showSaveConfirm = false;
            m_activeTopMenuIndex = -1;
            return;
        }

        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitConfirmNoX,
                          kExitConfirmY,
                          kExitConfirmButtonWidth,
                          kExitConfirmButtonHeight))
        {
            GameAudio::PlayMenuCancel();
            m_showSaveConfirm = false;
            m_activeTopMenuIndex = -1;
            return;
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
    {
        if (m_selectedSaveConfirmIndex == kSaveConfirmYesIndex)
        {
            GameAudio::PlayMenuConfirm();
            m_saveRequested = true;
            m_showSaveConfirm = false;
            m_activeTopMenuIndex = -1;
            return;
        }

        GameAudio::PlayMenuCancel();
        m_showSaveConfirm = false;
        m_activeTopMenuIndex = -1;
        return;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        GameAudio::PlayMenuCancel();
        m_showSaveConfirm = false;
        m_activeTopMenuIndex = -1;
    }
}

void PauseMenu::RenderSaveConfirm()
{
    UINT yesColor = kInactiveTextColor;
    if (m_selectedSaveConfirmIndex == kSaveConfirmYesIndex)
    {
        yesColor = kSelectedTextColor;
    }

    UINT noColor = kInactiveTextColor;
    if (m_selectedSaveConfirmIndex == kSaveConfirmNoIndex)
    {
        noColor = kSelectedTextColor;
    }

    m_render->DrawTextExCenter(m_qualityFontId,
                               L"セーブしますか？",
                               40,
                               740,
                               380,
                               36,
                               kSubTextColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"はい",
                               kExitConfirmYesX,
                               kExitConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight,
                               yesColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"いいえ",
                               kExitConfirmNoX,
                               kExitConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight,
                               noColor);
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

bool PauseMenu::TryGetTopMenuIndexFromPoint(const long x, const long y, int* outMenuIndex) const
{
    if (outMenuIndex == nullptr)
    {
        return false;
    }

    for (std::size_t i = 0; i < kTopMenuItems.size(); ++i)
    {
        const int menuIndex = static_cast<int>(i);
        const int itemX = kTopMenuX + menuIndex * kTopMenuItemInterval;
        if (IsPointInRect(x, y, itemX, kTopMenuY, kTopMenuItemWidth, kTopMenuItemHeight))
        {
            *outMenuIndex = menuIndex;
            return true;
        }
    }

    return false;
}

bool PauseMenu::IsPointInRect(const long x,
                              const long y,
                              const int left,
                              const int top,
                              const int width,
                              const int height)
{
    return left <= x &&
           x <= left + width &&
           top <= y &&
           y <= top + height;
}
