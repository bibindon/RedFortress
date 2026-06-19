#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class PauseMenu
{
public:
    void Initialize(NSRender::Render& render,
                    bool& mouseCursorVisible);
    void Toggle();
    void Open();
    void Close();
    void Update();
    void Render(const std::wstring& stageName);
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

    enum class FocusArea
    {
        TopMenu,
        ItemList
    };

    void LoadItems();
    void UpdateTopMenu();
    void UpdateItemList();
    void EnsureSelectedItemVisible();
    void RenderTopMenu();
    void RenderItemPanel();
    void SetMouseCursorVisible(bool visible);

    NSRender::Render* m_render = nullptr;
    bool* m_mouseCursorVisible = nullptr;
    bool m_isOpen = false;
    FocusArea m_focusArea = FocusArea::TopMenu;
    int m_selectedTopMenuIndex = 0;
    int m_activeTopMenuIndex = -1;
    std::size_t m_selectedItemIndex = 0;
    std::size_t m_itemScrollOffset = 0;
    std::vector<ItemData> m_items;
    int m_stageNameFontId = -1;
    int m_menuItemFontId = -1;
    int m_qualityFontId = -1;
};
