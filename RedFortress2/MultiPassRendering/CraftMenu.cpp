#include "CraftMenu.h"

#include <algorithm>

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
const std::wstring kPanelBackgroundPath = L"res\\2D_Image\\item_list_bg.png";
const std::wstring kRowHighlightPath = L"res\\2D_Image\\solid_white.png";
const std::wstring kIconDir = L"res\\2D_Image\\";
const std::wstring kItemIllustrationDir = L"res\\2D_Image\\item_illustrations\\";
const int kRightColumnX = 830;
const int kRightColumnWidth = 300;
const int kPanelY = 195;
const int kPanelHeight = 560;
const int kListPanelX = 155;
const int kListPanelWidth = 625;
const int kDetailPanelX = 800;
const int kDetailPanelWidth = 340;
const int kHeadingY = 215;
const int kResultImageSize = 150;
const int kResultImageX = kRightColumnX + (kRightColumnWidth - kResultImageSize) / 2;
const int kResultImageY = 265;
const int kMaterialStartY = 455;
const int kMaterialLineHeight = 40;
const std::wstring kRecipeCsvPath = L"res\\script\\CraftRecipes.csv";
const std::wstring kItemCsvPath = L"res\\script\\hoshigirl_item_ideas.csv";
const std::wstring kWeaponCsvPath = L"res\\script\\hoshigirl_weapon_ideas.csv";
const int kMaskedGaussianSampleSize = 25;
const float kMaskedGaussianAnimationDurationSeconds = 0.5f;
const std::size_t kVisibleRecipeCount = 11;
const int kRecipeStartY = 280;
const int kRecipeLineHeight = 42;
const int kRecipeListX = 185;
const int kRecipeListWidth = 575;
const UINT kTextColor = D3DCOLOR_RGBA(255, 255, 255, 245);
const UINT kSelectedTextColor = D3DCOLOR_RGBA(255, 220, 110, 255);
const UINT kDisabledTextColor = D3DCOLOR_RGBA(255, 120, 200, 255);
const UINT kEnoughTextColor = D3DCOLOR_RGBA(160, 245, 175, 245);
const UINT kMissingTextColor = D3DCOLOR_RGBA(245, 145, 145, 245);
const int kRowHighlightOffsetX = 18;
const int kRowHighlightOffsetY = 4;
const int kRowHighlightWidth = kRecipeListWidth + 26;
const int kRowHighlightHeight = kRecipeLineHeight - 4;
const int kRowHighlightTransparency = 45;
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
    GameAudio::PlayCraftOpen();
    if (m_mouseCursorVisible != nullptr)
    {
        m_previousMouseCursorVisible = *m_mouseCursorVisible;
    }
    m_maskedGaussianAmount = 0.0f;
    m_transitionStartAmount = 0.0f;
    m_transitionStartTime = std::chrono::steady_clock::now();
    m_transitionState = TransitionState::Opening;
    m_render->SetSceneUpdatePaused(true);
    m_render->SetPostEffectMaskedGaussianMaskPath(kMenuMaskPath);
    m_render->SetPostEffectMaskedGaussianSampleSize(kMaskedGaussianSampleSize);
    m_render->SetPostEffectMaskedGaussianAmount(m_maskedGaussianAmount);
    m_render->SetPostEffectMaskedGaussianFilter(true);
    SetMouseCursorVisible(true);
}

void CraftMenu::Close()
{
    if (!m_isOpen || m_transitionState == TransitionState::Closing)
    {
        return;
    }

    m_transitionStartAmount = m_maskedGaussianAmount;
    m_transitionStartTime = std::chrono::steady_clock::now();
    m_transitionState = TransitionState::Closing;
}

void CraftMenu::CloseImmediately()
{
    m_maskedGaussianAmount = 0.0f;
    if (m_render != nullptr)
    {
        m_render->SetPostEffectMaskedGaussianAmount(m_maskedGaussianAmount);
    }
    CompleteClose();
}

void CraftMenu::CompleteClose()
{
    if (m_render != nullptr)
    {
        m_render->SetPostEffectMaskedGaussianFilter(false);
        m_render->SetSceneUpdatePaused(false);
    }
    m_isOpen = false;
    m_transitionState = TransitionState::Closed;
    SetMouseCursorVisible(m_previousMouseCursorVisible);
}

void CraftMenu::UpdateMaskedGaussianAnimation()
{
    if (m_render == nullptr ||
        (m_transitionState != TransitionState::Opening &&
         m_transitionState != TransitionState::Closing))
    {
        return;
    }

    float targetAmount = 1.0f;
    if (m_transitionState == TransitionState::Closing)
    {
        targetAmount = 0.0f;
    }

    float transitionDistance = targetAmount - m_transitionStartAmount;
    if (transitionDistance < 0.0f)
    {
        transitionDistance = -transitionDistance;
    }
    const float transitionDuration = kMaskedGaussianAnimationDurationSeconds * transitionDistance;
    float progress = 1.0f;
    if (transitionDuration > 0.0f)
    {
        const float elapsedSeconds = std::chrono::duration<float>(
            std::chrono::steady_clock::now() - m_transitionStartTime).count();
        progress = elapsedSeconds / transitionDuration;
        progress = (std::max)(0.0f, (std::min)(progress, 1.0f));
    }

    m_maskedGaussianAmount = m_transitionStartAmount +
                             (targetAmount - m_transitionStartAmount) * progress;
    m_render->SetPostEffectMaskedGaussianAmount(m_maskedGaussianAmount);

    if (progress < 1.0f)
    {
        return;
    }

    if (m_transitionState == TransitionState::Opening)
    {
        m_transitionState = TransitionState::Open;
        return;
    }

    CompleteClose();
}

void CraftMenu::Update()
{
    if (!m_isOpen || m_recipes.empty())
    {
        return;
    }

    UpdateMaskedGaussianAnimation();
    if (!m_isOpen || m_transitionState != TransitionState::Open)
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

    // マウス操作。解像度に依存しないよう、マウス位置をベース解像度(1600x900)座標に変換して判定する。
    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);

    std::size_t hoveredIndex = m_recipes.size();
    if (TryGetRecipeIndexFromPoint(baseMouseX, baseMouseY, &hoveredIndex) &&
        hoveredIndex != m_selectedIndex)
    {
        MoveSelectionTo(hoveredIndex);
    }

    const bool confirmPressed = InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN) ||
                                InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE) ||
                                InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_B) ||
                                (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT) &&
                                 hoveredIndex < m_recipes.size());
    if (!confirmPressed)
    {
        return;
    }

    const Recipe& recipe = m_recipes.at(m_selectedIndex);
    if (IsRecipeAlreadyCrafted(recipe))
    {
        GameAudio::PlayMenuCancel();
        m_statusMessage = L"すでに作成済みです";
        m_statusColor = kMissingTextColor;
        return;
    }

    if (!IsRecipeUnlocked(recipe))
    {
        GameAudio::PlayMenuCancel();
        const int requiredWorld = GetRecipeRequiredWorld(recipe);
        m_statusMessage = L"ワールド" + std::to_wstring(requiredWorld) + L"からクラフト可能";
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

    // 開くときはアニメーション完了後に、閉じるときはアニメーション開始と同時にテキストを出し入れする。
    if (m_transitionState != TransitionState::Open)
    {
        return;
    }

    if (m_titleFontId < 0)
    {
        m_titleFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 32, kTextColor);
        m_headingFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 25, kTextColor);
        m_textFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 22, kTextColor);
    }

    m_render->DrawImageSized(kPanelBackgroundPath,
                             kListPanelX,
                             kPanelY,
                             kListPanelWidth,
                             kPanelHeight,
                             255);
    m_render->DrawImageSized(kPanelBackgroundPath,
                             kDetailPanelX,
                             kPanelY,
                             kDetailPanelWidth,
                             kPanelHeight,
                             255);
    m_render->DrawTextExCenter(m_titleFontId,
                               L"クラフト",
                               kListPanelX,
                               130,
                               kDetailPanelX + kDetailPanelWidth - kListPanelX,
                               54,
                               kTextColor);
    m_render->DrawTextEx(m_headingFontId, L"生成物", kRecipeListX, kHeadingY, kTextColor);
    m_render->DrawTextEx(m_headingFontId, L"クラフト素材", kRightColumnX, kHeadingY, kTextColor);

    const std::wstring resultImagePath = GetResultImagePath(m_recipes.at(m_selectedIndex));
    if (!resultImagePath.empty())
    {
        m_render->DrawImageSized(resultImagePath,
                                 kResultImageX,
                                 kResultImageY,
                                 kResultImageSize,
                                 kResultImageSize,
                                 245);
    }

    const std::size_t endIndex = (std::min)(m_recipes.size(), m_scrollOffset + kVisibleRecipeCount);
    for (std::size_t i = m_scrollOffset; i < endIndex; ++i)
    {
        const Recipe& recipe = m_recipes.at(i);
        const bool alreadyCrafted = IsRecipeAlreadyCrafted(recipe);
        UINT color = kTextColor;
        if (alreadyCrafted || !IsRecipeUnlocked(recipe) || !CanCraft(recipe))
        {
            color = kDisabledTextColor;
        }
        if (i == m_selectedIndex && !alreadyCrafted && IsRecipeUnlocked(recipe) && CanCraft(recipe))
        {
            color = kSelectedTextColor;
        }

        const int row = static_cast<int>(i - m_scrollOffset);
        std::wstring text = GetName(recipe.resultId);
        if (alreadyCrafted)
        {
            text += L"  作成済み";
        }
        const int rowY = kRecipeStartY + row * kRecipeLineHeight;
        if (i == m_selectedIndex)
        {
            m_render->DrawImageSized(kRowHighlightPath,
                                     kRecipeListX - kRowHighlightOffsetX,
                                     rowY - kRowHighlightOffsetY,
                                     kRowHighlightWidth,
                                     kRowHighlightHeight,
                                     kRowHighlightTransparency);
        }
        m_render->DrawTextEx(m_textFontId, text, kRecipeListX, rowY, color);
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
        m_render->DrawTextEx(m_textFontId,
                             text,
                             kRightColumnX,
                             kMaterialStartY + static_cast<int>(i) * kMaterialLineHeight,
                             color);
    }

    std::wstring availability = L"";
    if (IsRecipeAlreadyCrafted(selectedRecipe))
    {
        availability = L"作成済み";
    }
    else if (!IsRecipeUnlocked(selectedRecipe))
    {
        const int requiredWorld = GetRecipeRequiredWorld(selectedRecipe);
        availability = L"ワールド" + std::to_wstring(requiredWorld) + L"からクラフト可能";
    }
    else if (CanCraft(selectedRecipe))
    {
        availability = L"作成できます";
    }
    if (!availability.empty())
    {
        UINT availabilityColor = kEnoughTextColor;
        if (IsRecipeAlreadyCrafted(selectedRecipe) || !IsRecipeUnlocked(selectedRecipe) || !CanCraft(selectedRecipe))
        {
            availabilityColor = kMissingTextColor;
        }
        m_render->DrawTextEx(m_headingFontId, availability, kRightColumnX, 600, availabilityColor);
    }
    if (!m_statusMessage.empty())
    {
        m_render->DrawTextEx(m_textFontId, m_statusMessage, kRightColumnX, 680, m_statusColor);
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
    return m_inventory != nullptr &&
           !IsRecipeAlreadyCrafted(recipe) &&
           m_inventory->HasItems(recipe.materials);
}

bool CraftMenu::IsRecipeAlreadyCrafted(const Recipe& recipe) const
{
    if (m_inventory == nullptr)
    {
        return false;
    }

    if (recipe.resultType == L"Weapon")
    {
        return m_inventory->GetWeaponCount(recipe.resultId) > 0;
    }

    if (recipe.resultType == L"Ability")
    {
        return m_inventory->IsAbilityUnlocked(recipe.resultId);
    }

    return false;
}

int CraftMenu::GetRecipeRequiredWorld(const Recipe& recipe) const
{

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
        return 1;
    }

    if (recipe.resultId == L"AirDash")
    {
        return 2;
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
    static const std::unordered_map<std::wstring, std::wstring> kAbilityNames =
    {
        { L"GroundDash", L"ダッシュ" },
        { L"AirDash", L"空中ダッシュ" },
        { L"DoubleJump", L"二段ジャンプ" }
    };
    const auto abilityFound = kAbilityNames.find(id);
    if (abilityFound != kAbilityNames.end())
    {
        return abilityFound->second;
    }

    const auto found = m_names.find(id);
    if (found == m_names.end())
    {
        return id;
    }
    return found->second;
}

std::wstring CraftMenu::GetResultImagePath(const Recipe& recipe) const
{
    static const std::unordered_map<std::wstring, std::wstring> kWeaponIconFiles =
    {
        { L"W001", L"attack_club_icon.png" },
        { L"W002", L"attack_slash_icon.png" },
        { L"W003", L"attack_buster_icon.png" },
        { L"W004", L"attack_bomb_icon.png" }
    };
    static const std::unordered_map<std::wstring, std::wstring> kAbilityIconFiles =
    {
        { L"GroundDash", L"ability_dash_icon.png" },
        { L"AirDash", L"ability_air_dash_icon.png" },
        { L"DoubleJump", L"ability_double_jump_icon.png" }
    };
    static const std::unordered_map<std::wstring, std::wstring> kItemIllustrationFiles =
    {
        { L"007", L"item_007_red_spaghetti.png" },
        { L"008", L"item_008_potato_chips.png" },
        { L"017", L"item_017_launch_juice.png" }
    };

    if (recipe.resultType == L"Weapon")
    {
        const auto found = kWeaponIconFiles.find(recipe.resultId);
        if (found != kWeaponIconFiles.end())
        {
            return kIconDir + found->second;
        }
    }
    else if (recipe.resultType == L"Ability")
    {
        const auto found = kAbilityIconFiles.find(recipe.resultId);
        if (found != kAbilityIconFiles.end())
        {
            return kIconDir + found->second;
        }
    }
    else if (recipe.resultType == L"Item")
    {
        const auto found = kItemIllustrationFiles.find(recipe.resultId);
        if (found != kItemIllustrationFiles.end())
        {
            return kItemIllustrationDir + found->second;
        }
    }
    return L"";
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

void CraftMenu::MoveSelectionTo(const std::size_t index)
{
    if (index >= m_recipes.size())
    {
        return;
    }

    if (m_selectedIndex != index)
    {
        m_selectedIndex = index;
        GameAudio::PlayMenuMove();
    }

    m_statusMessage.clear();
    EnsureSelectionVisible();
}

bool CraftMenu::TryGetRecipeIndexFromPoint(const long x,
                                           const long y,
                                           std::size_t* outIndex) const
{
    if (outIndex == nullptr)
    {
        return false;
    }

    const std::size_t endIndex = (std::min)(m_recipes.size(), m_scrollOffset + kVisibleRecipeCount);
    for (std::size_t i = m_scrollOffset; i < endIndex; ++i)
    {
        const int row = static_cast<int>(i - m_scrollOffset);
        if (IsPointInRect(x,
                          y,
                          kRecipeListX,
                          kRecipeStartY + row * kRecipeLineHeight,
                          kRecipeListWidth,
                          kRecipeLineHeight))
        {
            *outIndex = i;
            return true;
        }
    }

    return false;
}

bool CraftMenu::IsPointInRect(const long x,
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

void CraftMenu::SetMouseCursorVisible(const bool visible)
{
    if (m_mouseCursorVisible != nullptr)
    {
        *m_mouseCursorVisible = visible;
    }
    InputDevice::Mouse::SetVisible(visible);
}
