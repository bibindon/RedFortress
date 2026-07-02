#include "CraftMenu.h"

#include <algorithm>

#include "InventoryManager.h"
#include "GameAudio.h"
#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"

namespace
{
const std::wstring kMenuMaskPath = L"res\\2D_Image\\menu_mask.png";
const std::wstring kRecipeCsvPath = L"res\\script\\CraftRecipes.csv";
const std::wstring kItemCsvPath = L"res\\script\\hoshigirl_item_ideas.csv";
const std::wstring kWeaponCsvPath = L"res\\script\\hoshigirl_weapon_ideas.csv";
const int kMaskedGaussianSampleSize = 25;
const std::size_t kVisibleRecipeCount = 11;
const int kRecipeStartY = 260;
const int kRecipeLineHeight = 42;
const UINT kTextColor = D3DCOLOR_RGBA(255, 255, 255, 245);
const UINT kSubTextColor = D3DCOLOR_RGBA(220, 232, 245, 235);
const UINT kSelectedTextColor = D3DCOLOR_RGBA(255, 220, 110, 255);
const UINT kDisabledTextColor = D3DCOLOR_RGBA(115, 125, 140, 210);
const UINT kEnoughTextColor = D3DCOLOR_RGBA(160, 245, 175, 245);
const UINT kMissingTextColor = D3DCOLOR_RGBA(245, 145, 145, 245);
}

void CraftMenu::Initialize(NSRender::Render& render,
                           bool& mouseCursorVisible,
                           InventoryManager& inventory)
{
    m_render = &render;
    m_mouseCursorVisible = &mouseCursorVisible;
    m_inventory = &inventory;
    LoadCatalog(kItemCsvPath);
    LoadCatalog(kWeaponCsvPath);
    LoadRecipes();
}

void CraftMenu::Open()
{
    if (m_render == nullptr || m_inventory == nullptr || m_recipes.empty())
    {
        return;
    }

    m_isOpen = true;
    m_skipInputFrame = true;
    m_selectedIndex = 0;
    m_scrollOffset = 0;
    m_statusMessage.clear();
    if (m_mouseCursorVisible != nullptr)
    {
        m_previousMouseCursorVisible = *m_mouseCursorVisible;
    }
    m_render->SetSceneUpdatePaused(true);
    m_render->SetPostEffectMaskedGaussianFilter(true);
    m_render->SetPostEffectMaskedGaussianMaskPath(kMenuMaskPath);
    m_render->SetPostEffectMaskedGaussianSampleSize(kMaskedGaussianSampleSize);
    SetMouseCursorVisible(true);
}

void CraftMenu::Close()
{
    if (m_render != nullptr)
    {
        m_render->SetPostEffectMaskedGaussianFilter(false);
        m_render->SetSceneUpdatePaused(false);
    }
    m_isOpen = false;
    SetMouseCursorVisible(m_previousMouseCursorVisible);
}

void CraftMenu::Update()
{
    if (!m_isOpen || m_recipes.empty())
    {
        return;
    }

    if (m_skipInputFrame)
    {
        m_skipInputFrame = false;
        return;
    }

    const bool cancelPressed = InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE) ||
                               InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_A);
    if (cancelPressed)
    {
        GameAudio::PlayMenuCancel();
        Close();
        return;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP) ||
        InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_UP))
    {
        MoveSelection(-1);
    }
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN) ||
        InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_DOWN))
    {
        MoveSelection(1);
    }

    const bool confirmPressed = InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN) ||
                                InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE) ||
                                InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_B);
    if (!confirmPressed)
    {
        return;
    }

    const Recipe& recipe = m_recipes.at(m_selectedIndex);
    if (!IsRecipeUnlocked(recipe))
    {
        GameAudio::PlayMenuCancel();
        const int requiredWorld = GetRecipeRequiredWorld(recipe);
        if (requiredWorld <= 2)
        {
            m_statusMessage = L"ワールド2になるまで武器はクラフトできない";
        }
        else
        {
            m_statusMessage = L"ワールド" + std::to_wstring(requiredWorld) + L"からクラフトできます";
        }
        m_statusColor = kMissingTextColor;
        return;
    }

    if (!CanCraft(recipe))
    {
        GameAudio::PlayMenuCancel();
        m_statusMessage = L"素材が不足しています";
        m_statusColor = kMissingTextColor;
        return;
    }

    if (m_inventory->TryCraft(recipe.materials,
                              recipe.resultType,
                              recipe.resultId,
                              recipe.resultCount))
    {
        GameAudio::PlayMenuConfirm();
        m_statusMessage = GetName(recipe.resultId) + L"を作成しました";
        m_statusColor = kEnoughTextColor;
    }
}

void CraftMenu::Render()
{
    if (!m_isOpen || m_render == nullptr || m_inventory == nullptr || m_recipes.empty())
    {
        return;
    }

    if (m_titleFontId < 0)
    {
        m_titleFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 32, kTextColor);
        m_headingFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 25, kTextColor);
        m_textFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 22, kTextColor);
        m_smallFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 19, kSubTextColor);
    }

    m_render->DrawTextExCenter(m_titleFontId, L"クラフト", 0, 110, 1600, 54, kTextColor);
    m_render->DrawTextExCenter(m_headingFontId, L"完成品", 160, 195, 560, 46, kTextColor);
    m_render->DrawTextExCenter(m_headingFontId, L"必要素材", 855, 195, 560, 46, kTextColor);

    const std::size_t endIndex = (std::min)(m_recipes.size(), m_scrollOffset + kVisibleRecipeCount);
    for (std::size_t i = m_scrollOffset; i < endIndex; ++i)
    {
        const Recipe& recipe = m_recipes.at(i);
        UINT color = kTextColor;
        if (!IsRecipeUnlocked(recipe) || !CanCraft(recipe))
        {
            color = kDisabledTextColor;
        }
        if (i == m_selectedIndex && IsRecipeUnlocked(recipe) && CanCraft(recipe))
        {
            color = kSelectedTextColor;
        }

        const int row = static_cast<int>(i - m_scrollOffset);
        std::wstring text = GetName(recipe.resultId) + L"  x" + std::to_wstring(recipe.resultCount);
        if (i == m_selectedIndex)
        {
            text = L"> " + text;
        }
        m_render->DrawTextExCenter(m_textFontId,
                                   text,
                                   155,
                                   kRecipeStartY + row * kRecipeLineHeight,
                                   575,
                                   38,
                                   color);
    }

    const Recipe& selectedRecipe = m_recipes.at(m_selectedIndex);
    for (std::size_t i = 0; i < selectedRecipe.materials.size(); ++i)
    {
        const auto& material = selectedRecipe.materials.at(i);
        const int ownedCount = m_inventory->GetItemCount(material.first);
        UINT color = kEnoughTextColor;
        if (ownedCount < material.second)
        {
            color = kMissingTextColor;
        }
        const std::wstring text = GetName(material.first) + L"  " +
                                  std::to_wstring(ownedCount) + L" / " +
                                  std::to_wstring(material.second);
        m_render->DrawTextExCenter(m_textFontId,
                                   text,
                                   855,
                                   275 + static_cast<int>(i) * 58,
                                   560,
                                   42,
                                   color);
    }

    std::wstring availability = L"素材不足";
    if (!IsRecipeUnlocked(selectedRecipe))
    {
        const int requiredWorld = GetRecipeRequiredWorld(selectedRecipe);
        if (requiredWorld <= 2)
        {
            availability = L"ワールド2になるまで武器はクラフトできない";
        }
        else
        {
            availability = L"ワールド" + std::to_wstring(requiredWorld) + L"からクラフトできます";
        }
    }
    else if (CanCraft(selectedRecipe))
    {
        availability = L"作成できます";
    }
    UINT availabilityColor = kEnoughTextColor;
    if (!IsRecipeUnlocked(selectedRecipe) || !CanCraft(selectedRecipe))
    {
        availabilityColor = kMissingTextColor;
    }
    m_render->DrawTextExCenter(m_headingFontId, availability, 855, 520, 560, 44, availabilityColor);
    m_render->DrawTextExCenter(m_smallFontId,
                               L"↑↓ 選択   Enter / ○ 作成   Esc / × 閉じる",
                               0,
                               800,
                               1600,
                               40,
                               kSubTextColor);
    if (!m_statusMessage.empty())
    {
        m_render->DrawTextExCenter(m_textFontId, m_statusMessage, 760, 660, 760, 45, m_statusColor);
    }
}

bool CraftMenu::IsOpen() const
{
    return m_isOpen;
}

bool CraftMenu::BlocksGameInput() const
{
    return m_isOpen;
}

void CraftMenu::SetCurrentWorld(const int world)
{
    if (world < 1)
    {
        m_currentWorld = 1;
        return;
    }

    m_currentWorld = world;
}

void CraftMenu::LoadCatalog(const std::wstring& csvPath)
{
    std::vector<std::vector<std::wstring>> csvData;
    csvData = csv::Read(NSRender::Util::GetExeDir() + csvPath);

    for (const auto& row : csvData)
    {
        if (row.size() >= 2 && row.at(0) != L"ID")
        {
            m_names[row.at(0)] = row.at(1);
        }
    }
}

void CraftMenu::LoadRecipes()
{
    m_recipes.clear();
    std::vector<std::vector<std::wstring>> csvData;
    csvData = csv::Read(NSRender::Util::GetExeDir() + kRecipeCsvPath);

    for (const auto& row : csvData)
    {
        if (row.size() < 6 || row.at(0) == L"RecipeID")
        {
            continue;
        }

        Recipe recipe;
        recipe.id = row.at(0);
        recipe.resultType = row.at(1);
        recipe.resultId = row.at(2);
        recipe.resultCount = std::stoi(row.at(3));
        for (std::size_t i = 4; i + 1 < row.size(); i += 2)
        {
            if (!row.at(i).empty())
            {
                recipe.materials.push_back({ row.at(i), std::stoi(row.at(i + 1)) });
            }
        }

        if (!recipe.id.empty() && !recipe.resultId.empty() &&
            recipe.resultCount > 0 && !recipe.materials.empty())
        {
            m_recipes.push_back(recipe);
        }
    }
}

bool CraftMenu::CanCraft(const Recipe& recipe) const
{
    return m_inventory != nullptr && m_inventory->HasItems(recipe.materials);
}

int CraftMenu::GetRecipeRequiredWorld(const Recipe& recipe) const
{
    if (recipe.resultType != L"Weapon" && recipe.resultType != L"Item")
    {
        return 1;
    }

    if (recipe.resultId == L"W002")
    {
        return 2;
    }

    if (recipe.resultId == L"W003")
    {
        return 3;
    }

    if (recipe.resultId == L"W004")
    {
        return 4;
    }

    if (recipe.resultId == L"GroundDash")
    {
        return 2;
    }

    if (recipe.resultId == L"AirDash")
    {
        return 3;
    }

    if (recipe.resultId == L"DoubleJump")
    {
        return 4;
    }

    return 1;
}

bool CraftMenu::IsRecipeUnlocked(const Recipe& recipe) const
{
    return m_currentWorld >= GetRecipeRequiredWorld(recipe);
}

std::wstring CraftMenu::GetName(const std::wstring& id) const
{
    const auto found = m_names.find(id);
    if (found == m_names.end())
    {
        return id;
    }
    return found->second;
}

void CraftMenu::MoveSelection(const int direction)
{
    const std::size_t previousIndex = m_selectedIndex;
    if (direction < 0 && m_selectedIndex > 0)
    {
        --m_selectedIndex;
    }
    else if (direction > 0 && m_selectedIndex + 1 < m_recipes.size())
    {
        ++m_selectedIndex;
    }
    if (m_selectedIndex != previousIndex)
    {
        GameAudio::PlayMenuMove();
    }
    m_statusMessage.clear();
    EnsureSelectionVisible();
}

void CraftMenu::EnsureSelectionVisible()
{
    if (m_selectedIndex < m_scrollOffset)
    {
        m_scrollOffset = m_selectedIndex;
    }
    if (m_selectedIndex >= m_scrollOffset + kVisibleRecipeCount)
    {
        m_scrollOffset = m_selectedIndex - kVisibleRecipeCount + 1;
    }
}

void CraftMenu::SetMouseCursorVisible(const bool visible)
{
    if (m_mouseCursorVisible != nullptr)
    {
        *m_mouseCursorVisible = visible;
    }
    InputDevice::Mouse::SetVisible(visible);
}
