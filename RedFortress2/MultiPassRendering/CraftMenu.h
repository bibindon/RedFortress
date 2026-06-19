#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace NSRender
{
class Render;
}

class InventoryManager;

class CraftMenu
{
public:
    void Initialize(NSRender::Render& render,
                    bool& mouseCursorVisible,
                    InventoryManager& inventory);
    void Open();
    void Close();
    void Update();
    void Render();
    bool IsOpen() const;
    bool BlocksGameInput() const;

private:
    struct Recipe
    {
        std::wstring id;
        std::wstring resultType;
        std::wstring resultId;
        int resultCount = 1;
        std::vector<std::pair<std::wstring, int>> materials;
    };

    void LoadCatalog(const std::wstring& csvPath);
    void LoadRecipes();
    bool CanCraft(const Recipe& recipe) const;
    std::wstring GetName(const std::wstring& id) const;
    void MoveSelection(int direction);
    void EnsureSelectionVisible();
    void SetMouseCursorVisible(bool visible);

    NSRender::Render* m_render = nullptr;
    InventoryManager* m_inventory = nullptr;
    bool* m_mouseCursorVisible = nullptr;
    bool m_previousMouseCursorVisible = false;
    bool m_isOpen = false;
    bool m_skipInputFrame = false;
    std::size_t m_selectedIndex = 0;
    std::size_t m_scrollOffset = 0;
    std::vector<Recipe> m_recipes;
    std::unordered_map<std::wstring, std::wstring> m_names;
    std::wstring m_statusMessage;
    unsigned int m_statusColor = 0;
    int m_titleFontId = -1;
    int m_headingFontId = -1;
    int m_textFontId = -1;
    int m_smallFontId = -1;
};
