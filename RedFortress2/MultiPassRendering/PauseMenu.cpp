#include "PauseMenu.h"

#include <array>
#include <string>
#include <vector>

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
const std::size_t kVisibleItemCount = 11;
const int kItemListX = 205;
const int kItemListY = 350;
const int kItemListLineHeight = 34;
const std::wstring kItemCsvPath = L"res\\script\\hoshigirl_item_ideas.csv";
}

void PauseMenu::Initialize(NSRender::Render& render,
                           bool& mouseCursorVisible)
{
    m_render = &render;
    m_mouseCursorVisible = &mouseCursorVisible;
    LoadItems();
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
    m_focusArea = FocusArea::TopMenu;
    m_selectedTopMenuIndex = 0;
    m_activeTopMenuIndex = -1;
    m_selectedItemIndex = 0;
    m_itemScrollOffset = 0;
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

    if (m_focusArea == FocusArea::ItemList)
    {
        UpdateItemList();
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

    if (m_items.empty())
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
        if (m_selectedItemIndex + 1 < m_items.size())
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

    if (m_items.empty())
    {
        m_render->DrawTextEx(m_qualityFontId,
                             L"アイテムデータを読み込めませんでした。",
                             kItemListX,
                             kItemListY,
                             kSubTextColor);
        return;
    }

    const std::size_t visibleEnd = m_itemScrollOffset + kVisibleItemCount;
    std::size_t itemEnd = visibleEnd;
    if (itemEnd > m_items.size())
    {
        itemEnd = m_items.size();
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
        m_render->DrawTextEx(m_qualityFontId,
                             prefix + m_items.at(i).name,
                             kItemListX,
                             kItemListY + lineIndex * kItemListLineHeight,
                             color);
    }

    const std::wstring positionText = std::to_wstring(m_selectedItemIndex + 1) +
                                      L" / " +
                                      std::to_wstring(m_items.size());
    m_render->DrawTextExCenter(m_qualityFontId,
                               positionText,
                               170,
                               730,
                               560,
                               36,
                               kSubTextColor);

    const ItemData& selectedItem = m_items.at(m_selectedItemIndex);
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
                         L"説明",
                         detailX,
                         585,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         selectedItem.description,
                         detailX,
                         630,
                         kSubTextColor);
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
