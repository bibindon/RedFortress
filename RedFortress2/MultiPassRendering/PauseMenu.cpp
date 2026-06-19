#include "PauseMenu.h"

#include <array>
#include <string>

#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../RedFortressRender/Render/Render.h"

namespace
{
const std::wstring kMenuMaskPath = L"res\\2D_Image\\menu_mask.png";
const int kMaskedGaussianSampleSize = 25;
const UINT kTextColor = D3DCOLOR_RGBA(255, 255, 255, 245);
const UINT kSubTextColor = D3DCOLOR_RGBA(225, 235, 255, 230);
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
}

void PauseMenu::Initialize(NSRender::Render& render,
                           bool& mouseCursorVisible)
{
    m_render = &render;
    m_mouseCursorVisible = &mouseCursorVisible;
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
}

void PauseMenu::Render(const std::wstring& stageName)
{
    if (!m_isOpen || m_render == nullptr)
    {
        return;
    }

    if (m_stageNameFontId < 0)
    {
        m_stageNameFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 34, kTextColor);
    }

    if (m_menuItemFontId < 0)
    {
        m_menuItemFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 30, kTextColor);
    }

    if (m_qualityFontId < 0)
    {
        m_qualityFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 24, kSubTextColor);
    }

    m_render->DrawTextExCenter(m_stageNameFontId,
                               stageName,
                               960,
                               115,
                               560,
                               60,
                               kTextColor);

    for (size_t i = 0; i < kTopMenuFirstRow.size(); ++i)
    {
        const int x = kTopMenuFirstRowX + static_cast<int>(i) * kTopMenuItemInterval;
        m_render->DrawTextExCenter(m_qualityFontId,
                                   kTopMenuFirstRow[i],
                                   x,
                                   kTopMenuFirstRowY,
                                   kTopMenuItemWidth,
                                   kTopMenuItemHeight,
                                   kTextColor);
    }

    for (size_t i = 0; i < kTopMenuSecondRow.size(); ++i)
    {
        const int x = kTopMenuSecondRowX + static_cast<int>(i) * kTopMenuItemInterval;
        m_render->DrawTextExCenter(m_qualityFontId,
                                   kTopMenuSecondRow[i],
                                   x,
                                   kTopMenuSecondRowY,
                                   kTopMenuItemWidth,
                                   kTopMenuItemHeight,
                                   kTextColor);
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
