#include "PauseMenu.h"

#include <algorithm>
#include <array>
#include <string>
#include <unordered_map>
#include <utility>
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
const std::wstring kMenuTopBackgroundPath = L"res\\2D_Image\\menu_top_bg.png";
const std::wstring kMenuItemListBackgroundPath = L"res\\2D_Image\\item_list_bg.png";
const std::wstring kCommandCursorPath = L"res\\2D_Image\\command_cursor.png";
const std::wstring kHeartImagePath = L"res\\2D_Image\\heart.png";
const std::wstring kWeaponClubIconPath = L"res\\2D_Image\\attack_club_icon.png";
const std::wstring kWeaponSlashIconPath = L"res\\2D_Image\\attack_slash_icon.png";
const std::wstring kWeaponBusterIconPath = L"res\\2D_Image\\attack_buster_icon.png";
const std::wstring kWeaponBombIconPath = L"res\\2D_Image\\attack_bomb_icon.png";
const std::wstring kSettingsArrowButtonImagePath = L"res\\2D_Image\\settings_button_arrow.png";
const std::wstring kSettingsLeftArrowImagePath = L"res\\2D_Image\\settings_arrow_left.png";
const std::wstring kSettingsRightArrowImagePath = L"res\\2D_Image\\settings_arrow_right.png";
const std::wstring kSettingsApplyButtonImagePath = L"res\\2D_Image\\settings_button_apply.png";
const std::wstring kSettingsCancelButtonImagePath = L"res\\2D_Image\\settings_button_cancel.png";
const std::wstring kItemScrollUpImagePath = L"res\\2D_Image\\item_scroll_up.png";
const std::wstring kItemScrollDownImagePath = L"res\\2D_Image\\item_scroll_down.png";
const int kHeartImageSize = 36;
const int kLivesDisplayX = 1320;
const int kLivesDisplayY = 130;
const int kLivesTextX = kLivesDisplayX + kHeartImageSize + 10;
const int kLivesTextY = 133;
// command_cursor.png の白い点は画像左上(0,0)〜(12,12)にあるため、点の中心は画像内 (6,6)。
const int kCommandCursorDotCenterX = 6;
const int kCommandCursorDotCenterY = 6;
// 点の中心をずらす場合の縦位置。文字列(DT_VCENTERで箱の中央)より上に浮かせる。
const int kCommandCursorVerticalOffset = -2;
const int kMaskedGaussianSampleSize = 25;
const float kMaskedGaussianAnimationDurationSeconds = 0.5f;
const UINT kTextColor = D3DCOLOR_RGBA(255, 255, 255, 245);
const UINT kSubTextColor = D3DCOLOR_RGBA(225, 235, 255, 230);
const UINT kItemListHeaderTextColor = D3DCOLOR_RGBA(180, 180, 180, 230);
const UINT kSuccessTextColor = D3DCOLOR_RGBA(160, 245, 175, 245);
const UINT kErrorTextColor = D3DCOLOR_RGBA(245, 145, 145, 245);
const std::array<const wchar_t*, 5> kTopMenuItems =
{
    L"アイテム",
    L"武器",
    L"設定",
    L"セーブ",
    L"終了"
};
const int kTopMenuItemWidth = 200;
const int kTopMenuItemHeight = 44;
const int kTopMenuItemInterval = 260;
const int kTopMenuX = 190;
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
const int kExitConfirmYesX = 250;
const int kExitConfirmNoX = 430;
const int kExitConfirmY = 700;
const int kConfirmPromptX = 210;
const int kConfirmPromptY = 650;
const int kConfirmPromptWidth = 400;
const int kConfirmPromptHeight = 36;
const int kSaveConfirmYesX = 220;
const int kSaveConfirmNoX = 400;
const int kSaveConfirmY = 410;
const int kSaveConfirmPromptX = 220;
const int kSaveConfirmPromptY = 350;
const int kExitPanelStageSelectIndex = 0;
const int kExitPanelTitleIndex = 1;
const int kExitPanelGameIndex = 2;
const int kExitPanelButtonX = 220;
const int kExitPanelButtonWidth = 460;
const int kExitPanelButtonHeight = 56;
const int kExitPanelFirstY = 300;
const int kExitPanelButtonInterval = 75;
const int kSettingsRowX = 200;
const int kSettingsRowTextX = 220;
const int kSettingsRowWidth = 260;
const int kSettingsFirstRowY = 342;
const int kSettingsRowInterval = 74;
const int kSettingsOptionListX = 480;
const int kSettingsOptionListWidth = 300;
const int kSettingsValueHeight = 62;
const int kSettingsArrowWidth = 80;
const int kSettingsArrowHeight = 62;
const int kSettingsArrowIconSize = 24;
const int kSettingsArrowIconOffsetX = (kSettingsArrowWidth - kSettingsArrowIconSize) / 2;
const int kSettingsArrowIconOffsetY = (kSettingsArrowHeight - kSettingsArrowIconSize) / 2;
const int kSettingsLeftArrowX = 390;
const int kSettingsRightArrowX = 790;
const int kSettingsApplyX = 460;
const int kSettingsApplyY = 590;
const int kSettingsApplyWidth = 160;
const int kSettingsApplyHeight = 52;
const int kSettingsCancelX = 650;
const int kSettingsCancelY = kSettingsApplyY;
const int kSettingsCancelWidth = 190;
const int kSettingsCancelHeight = kSettingsApplyHeight;
const int kSettingsButtonTransparency = 64;
const int kDisabledSettingsButtonTransparency = 32;
const int kSettingsArrowIconTransparency = 245;
const int kDisabledSettingsArrowIconTransparency = 90;
const UINT kDisabledSettingsTextColor = D3DCOLOR_RGBA(120, 125, 135, 190);
const std::size_t kVisibleItemCount = 7;
const std::size_t kVisibleWeaponCount = 4;
const int kItemListX = 255;
const int kItemListHeaderY = 338;
const int kItemListY = 385;
const int kItemPanelListY = 412;
const int kItemListLineHeight = 40;
const int kItemCountColumnX = 500;
const int kItemCountColumnWidth = 100;
// フォント(20px)の行高。所持数の描画はこの高さで上揃えにして、行内で縦の位置を揃える。
const int kMenuTextLineHeight = 26;
// アイテム一覧(ヘッダー行+リスト)全体を覆う白い角丸矩形。PNG と 1:1 で描画する。
const int kItemListBackgroundX = kItemListX - 50;
const int kItemListBackgroundY = kItemListHeaderY - 26;
const int kItemListBackgroundWidth =
    kItemCountColumnX + kItemCountColumnWidth + 30 - (kItemListX - 50);
const int kItemListBackgroundHeight =
    kItemPanelListY + static_cast<int>(kVisibleItemCount) * kItemListLineHeight + 62 -
    (kItemListHeaderY - 26);
const int kItemScrollArrowWidth = 48;
const int kItemScrollArrowHeight = 32;
const int kItemScrollArrowX =
    kItemListBackgroundX + (kItemListBackgroundWidth - kItemScrollArrowWidth) / 2;
const int kItemScrollUpArrowY = 364;
const int kItemScrollDownArrowY =
    kItemPanelListY + static_cast<int>(kVisibleItemCount) * kItemListLineHeight + 12;
const int kItemScrollButtonHeight = 52;
const int kItemScrollUpButtonY = kItemScrollUpArrowY - 10;
const int kItemScrollDownButtonY = kItemScrollDownArrowY - 10;
const int kWeaponListIconX = kItemListX;
const int kWeaponListIconSize = 56;
const int kWeaponListLineHeight = 72;
const int kWeaponListTextX = kWeaponListIconX + kWeaponListIconSize + 18;
const int kWeaponListTextYOffset = 10;
const int kWeaponListCursorCenterYOffset = kWeaponListIconSize / 2;
const std::wstring kItemCsvPath = L"res\\script\\hoshigirl_item_ideas.csv";
const std::wstring kWeaponCsvPath = L"res\\script\\hoshigirl_weapon_ideas.csv";
const std::wstring kItemIllustrationDir = L"res\\2D_Image\\item_illustrations\\";
const int kItemIllustrationX = 800;
const int kItemIllustrationY = 365;
const int kItemIllustrationSize = 300;

bool IsMenuLeftPressed()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LEFT))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_LEFT))
    {
        return true;
    }
    return false;
}

bool IsMenuRightPressed()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RIGHT))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_RIGHT))
    {
        return true;
    }
    return false;
}

bool IsMenuUpPressed()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_UP))
    {
        return true;
    }
    return false;
}

bool IsMenuDownPressed()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_DOWN))
    {
        return true;
    }
    return false;
}

bool IsMenuConfirmPressed()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
    {
        return true;
    }
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_B))
    {
        return true;
    }
    return false;
}

bool IsMenuCancelPressed()
{
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_A))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_START))
    {
        return true;
    }
    return false;
}

std::wstring GetWeaponIconPath(const std::wstring& weaponId)
{
    if (weaponId == L"W001")
    {
        return kWeaponClubIconPath;
    }
    if (weaponId == L"W002")
    {
        return kWeaponSlashIconPath;
    }
    if (weaponId == L"W003")
    {
        return kWeaponBusterIconPath;
    }
    if (weaponId == L"W004")
    {
        return kWeaponBombIconPath;
    }
    return L"";
}
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

    Open(m_saveEnabled,
         m_returnToStageSelectEnabled,
         m_returnToTitleEnabled,
         m_hasUnsavedChanges);
}

void PauseMenu::Open(const bool saveEnabled,
                     const bool returnToStageSelectEnabled,
                     const bool returnToTitleEnabled,
                     const bool hasUnsavedChanges)
{
    if (m_render == nullptr)
    {
        return;
    }

    m_isOpen = true;
    m_exitRequested = false;
    GameAudio::PlayMenuOpen();
    m_showExitConfirm = false;
    m_showSaveConfirm = false;
    m_saveRequested = false;
    m_saveEnabled = saveEnabled;
    m_returnToStageSelectRequested = false;
    m_returnToTitleRequested = false;
    m_returnToStageSelectEnabled = returnToStageSelectEnabled;
    m_returnToTitleEnabled = returnToTitleEnabled;
    m_hasUnsavedChanges = hasUnsavedChanges;
    m_skipInputFrame = true;
    // メニューを開いたら最初からアイテム一覧を表示する。
    m_focusArea = FocusArea::ItemList;
    m_selectedSettingsRow = SettingsRow::Resolution;
    m_selectedTopMenuIndex = kItemMenuIndex;
    m_activeTopMenuIndex = kItemMenuIndex;
    m_selectedExitConfirmIndex = kExitConfirmNoIndex;
    m_exitConfirmAction = ExitConfirmAction::Game;
    m_selectedExitPanelIndex = kExitPanelGameIndex;
    if (m_returnToTitleEnabled)
    {
        m_selectedExitPanelIndex = kExitPanelTitleIndex;
    }
    if (m_returnToStageSelectEnabled)
    {
        m_selectedExitPanelIndex = kExitPanelStageSelectIndex;
    }
    m_selectedResolutionIndex = 0;
    m_selectedWindowModeIndex = 0;
    m_selectedQualityIndex = 0;
    m_selectedItemIndex = 0;
    m_itemScrollOffset = 0;
    m_itemStatusMessage.clear();
    m_selectedWeaponIndex = 0;
    m_weaponScrollOffset = 0;
    m_maskedGaussianAmount = 0.0f;
    m_transitionStartAmount = 0.0f;
    m_transitionStartTime = std::chrono::steady_clock::now();
    m_transitionState = TransitionState::Opening;
    m_render->SetSceneUpdatePaused(true);
    m_render->SetPostEffectMaskedGaussianMaskPath(kMenuMaskPath);
    m_render->SetPostEffectMaskedGaussianSampleSize(kMaskedGaussianSampleSize);
    m_render->SetPostEffectMaskedGaussianAmount(m_maskedGaussianAmount);
    m_render->SetPostEffectMaskedGaussianFilter(true);
    RefreshSettingsOptions();
    SetMouseCursorVisible(true);
}

void PauseMenu::Close()
{
    if (!m_isOpen || m_transitionState == TransitionState::Closing)
    {
        return;
    }

    m_transitionStartAmount = m_maskedGaussianAmount;
    m_transitionStartTime = std::chrono::steady_clock::now();
    m_transitionState = TransitionState::Closing;
}

void PauseMenu::CloseImmediately()
{
    m_maskedGaussianAmount = 0.0f;
    if (m_render != nullptr)
    {
        m_render->SetPostEffectMaskedGaussianAmount(m_maskedGaussianAmount);
    }
    CompleteClose();
}

void PauseMenu::CompleteClose()
{
    if (m_render != nullptr)
    {
        m_render->SetPostEffectMaskedGaussianFilter(false);
        m_render->SetSceneUpdatePaused(false);
    }

    m_isOpen = false;
    m_transitionState = TransitionState::Closed;
    m_showExitConfirm = false;
    m_showSaveConfirm = false;
}

void PauseMenu::UpdateMaskedGaussianAnimation()
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

void PauseMenu::Update()
{
    if (!m_isOpen)
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

    if (TryActivateTopMenuFromMouseClick())
    {
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

    if (m_focusArea == FocusArea::ExitPanel)
    {
        UpdateExitPanel();
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
    if (IsMenuLeftPressed())
    {
        MoveTopMenuSelection(-1);
        m_activeTopMenuIndex = -1;
    }

    if (IsMenuRightPressed())
    {
        MoveTopMenuSelection(1);
        m_activeTopMenuIndex = -1;
    }

    if (m_selectedTopMenuIndex != previousIndex)
    {
        GameAudio::PlayMenuMove();
    }

    if (TryActivateTopMenuFromMouseClick())
    {
        return;
    }

    if (IsMenuConfirmPressed())
    {
        if (!IsTopMenuItemEnabled(m_selectedTopMenuIndex))
        {
            return;
        }

        ActivateTopMenu(m_selectedTopMenuIndex);
    }

    if (IsMenuCancelPressed())
    {
        GameAudio::PlayMenuCancel();
        Close();
    }
}

void PauseMenu::UpdateExitPanel()
{
    int firstIndex = kExitPanelTitleIndex;
    if (m_returnToStageSelectEnabled)
    {
        firstIndex = kExitPanelStageSelectIndex;
    }

    if (IsMenuUpPressed())
    {
        if (m_selectedExitPanelIndex > firstIndex)
        {
            --m_selectedExitPanelIndex;
            GameAudio::PlayMenuMove();
        }
    }

    if (IsMenuDownPressed())
    {
        if (m_selectedExitPanelIndex < kExitPanelGameIndex)
        {
            ++m_selectedExitPanelIndex;
            GameAudio::PlayMenuMove();
        }
    }

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    const InputDevice::MousePosition mouseDelta = InputDevice::Mouse::GetDelta();
    const bool mouseMoved = mouseDelta.x != 0 || mouseDelta.y != 0;
    if (mouseMoved)
    {
        int hoveredIndex = -1;
        if (m_returnToStageSelectEnabled &&
            IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitPanelButtonX,
                          GetExitPanelButtonY(kExitPanelStageSelectIndex),
                          kExitPanelButtonWidth,
                          kExitPanelButtonHeight))
        {
            hoveredIndex = kExitPanelStageSelectIndex;
        }
        else if (m_returnToTitleEnabled &&
                 IsPointInRect(baseMouseX,
                               baseMouseY,
                               kExitPanelButtonX,
                               GetExitPanelButtonY(kExitPanelTitleIndex),
                               kExitPanelButtonWidth,
                               kExitPanelButtonHeight))
        {
            hoveredIndex = kExitPanelTitleIndex;
        }
        else if (IsPointInRect(baseMouseX,
                               baseMouseY,
                               kExitPanelButtonX,
                               GetExitPanelButtonY(kExitPanelGameIndex),
                               kExitPanelButtonWidth,
                               kExitPanelButtonHeight))
        {
            hoveredIndex = kExitPanelGameIndex;
        }

        if (hoveredIndex >= 0 && hoveredIndex != m_selectedExitPanelIndex)
        {
            m_selectedExitPanelIndex = hoveredIndex;
            GameAudio::PlayMenuMove();
        }
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        if (m_returnToStageSelectEnabled &&
            IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitPanelButtonX,
                          GetExitPanelButtonY(kExitPanelStageSelectIndex),
                          kExitPanelButtonWidth,
                          kExitPanelButtonHeight))
        {
            GameAudio::PlayMenuConfirm();
            m_returnToStageSelectRequested = true;
            Close();
            return;
        }

        if (m_returnToTitleEnabled &&
            IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitPanelButtonX,
                          GetExitPanelButtonY(kExitPanelTitleIndex),
                          kExitPanelButtonWidth,
                          kExitPanelButtonHeight))
        {
            GameAudio::PlayMenuConfirm();
            m_selectedExitPanelIndex = kExitPanelTitleIndex;
            m_exitConfirmAction = ExitConfirmAction::Title;
            m_showExitConfirm = true;
            m_selectedExitConfirmIndex = kExitConfirmNoIndex;
            return;
        }

        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitPanelButtonX,
                          GetExitPanelButtonY(kExitPanelGameIndex),
                          kExitPanelButtonWidth,
                          kExitPanelButtonHeight))
        {
            GameAudio::PlayMenuConfirm();
            m_selectedExitPanelIndex = kExitPanelGameIndex;
            m_exitConfirmAction = ExitConfirmAction::Game;
            m_showExitConfirm = true;
            m_selectedExitConfirmIndex = kExitConfirmNoIndex;
            return;
        }
    }

    if (IsMenuConfirmPressed())
    {
        if (m_selectedExitPanelIndex == kExitPanelStageSelectIndex &&
            m_returnToStageSelectEnabled)
        {
            GameAudio::PlayMenuConfirm();
            m_returnToStageSelectRequested = true;
            Close();
            return;
        }

        if (m_selectedExitPanelIndex == kExitPanelTitleIndex &&
            m_returnToTitleEnabled)
        {
            GameAudio::PlayMenuConfirm();
            m_exitConfirmAction = ExitConfirmAction::Title;
            m_showExitConfirm = true;
            m_selectedExitConfirmIndex = kExitConfirmNoIndex;
            return;
        }

        GameAudio::PlayMenuConfirm();
        m_exitConfirmAction = ExitConfirmAction::Game;
        m_showExitConfirm = true;
        m_selectedExitConfirmIndex = kExitConfirmNoIndex;
        return;
    }

    if (IsMenuCancelPressed())
    {
        GameAudio::PlayMenuCancel();
        m_focusArea = FocusArea::TopMenu;
        m_activeTopMenuIndex = -1;
    }
}
void PauseMenu::UpdateItemList()
{
    if (IsMenuCancelPressed())
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
    bool navigationInputDetected = false;
    const long wheelDelta = InputDevice::Mouse::GetWheelDelta();
    if (wheelDelta > 0)
    {
        navigationInputDetected = true;
        if (m_selectedItemIndex > 0)
        {
            --m_selectedItemIndex;
        }
    }
    else if (wheelDelta < 0)
    {
        navigationInputDetected = true;
        if (m_selectedItemIndex + 1 < ownedItems.size())
        {
            ++m_selectedItemIndex;
        }
    }
    else if (IsMenuUpPressed())
    {
        navigationInputDetected = true;
        if (m_selectedItemIndex > 0)
        {
            --m_selectedItemIndex;
        }
    }
    else if (IsMenuDownPressed())
    {
        navigationInputDetected = true;
        if (m_selectedItemIndex + 1 < ownedItems.size())
        {
            ++m_selectedItemIndex;
        }
    }

    if (m_selectedItemIndex != previousIndex)
    {
        GameAudio::PlayMenuMove();
        m_itemStatusMessage.clear();
    }

    EnsureSelectedItemVisible();

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    const InputDevice::MousePosition mouseDelta = InputDevice::Mouse::GetDelta();
    const bool mouseMoved = mouseDelta.x != 0 || mouseDelta.y != 0;
    if (!navigationInputDetected &&
        mouseMoved &&
        IsPointInRect(baseMouseX,
                      baseMouseY,
                      kItemListX,
                      kItemPanelListY,
                      520,
                      static_cast<int>(kVisibleItemCount) * kItemListLineHeight))
    {
        const std::size_t hoveredIndex = m_itemScrollOffset +
            static_cast<std::size_t>((baseMouseY - kItemPanelListY) / kItemListLineHeight);
        if (hoveredIndex < ownedItems.size() && hoveredIndex != m_selectedItemIndex)
        {
            m_selectedItemIndex = hoveredIndex;
            m_itemStatusMessage.clear();
            GameAudio::PlayMenuMove();
        }
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        if (m_itemScrollOffset > 0 &&
            IsPointInRect(baseMouseX,
                          baseMouseY,
                          kItemListBackgroundX,
                          kItemScrollUpButtonY,
                          kItemListBackgroundWidth,
                          kItemScrollButtonHeight))
        {
            --m_itemScrollOffset;
            if (m_selectedItemIndex > 0)
            {
                --m_selectedItemIndex;
            }
            m_itemStatusMessage.clear();
            GameAudio::PlayMenuMove();
            return;
        }

        if (m_itemScrollOffset + kVisibleItemCount < ownedItems.size() &&
            IsPointInRect(baseMouseX,
                          baseMouseY,
                          kItemListBackgroundX,
                          kItemScrollDownButtonY,
                          kItemListBackgroundWidth,
                          kItemScrollButtonHeight))
        {
            ++m_itemScrollOffset;
            if (m_selectedItemIndex + 1 < ownedItems.size())
            {
                ++m_selectedItemIndex;
            }
            m_itemStatusMessage.clear();
            GameAudio::PlayMenuMove();
            return;
        }

        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kItemListX,
                          kItemPanelListY,
                          520,
                          static_cast<int>(kVisibleItemCount) * kItemListLineHeight))
        {
            const std::size_t clickedIndex = m_itemScrollOffset +
                static_cast<std::size_t>((baseMouseY - kItemPanelListY) / kItemListLineHeight);
            if (clickedIndex < ownedItems.size())
            {
                if (clickedIndex != m_selectedItemIndex)
                {
                    m_selectedItemIndex = clickedIndex;
                    m_itemStatusMessage.clear();
                    EnsureSelectedItemVisible();
                    GameAudio::PlayMenuMove();
                }
            }
            return;
        }
    }

    if (IsMenuConfirmPressed())
    {
        const ItemData& selectedItem = m_items.at(ownedItems.at(m_selectedItemIndex));
        if (!IsUsableItem(selectedItem.id) || !m_itemUseCallback)
        {
            GameAudio::PlayMenuCancel();
            m_itemStatusMessage = L"このアイテムは使用できません";
            m_itemStatusColor = kErrorTextColor;
            return;
        }

        if (m_itemUseCallback(selectedItem.id))
        {
            m_hasUnsavedChanges = true;
            GameAudio::PlayMenuConfirm();
            GameAudio::PlayDrink();
            m_itemStatusMessage = selectedItem.name + L"を使用しました";
            m_itemStatusColor = kSuccessTextColor;
            const std::vector<std::size_t> refreshedItems = GetOwnedItemIndices();
            if (m_selectedItemIndex >= refreshedItems.size() && m_selectedItemIndex > 0)
            {
                --m_selectedItemIndex;
            }
            EnsureSelectedItemVisible();
        }
        else
        {
            GameAudio::PlayMenuCancel();
            m_itemStatusMessage = L"今は使用できません";
            m_itemStatusColor = kErrorTextColor;
        }
    }
}

bool PauseMenu::IsUsableItem(const std::wstring& itemId) const
{
    return itemId == L"007" || itemId == L"008" || itemId == L"017";
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

std::wstring PauseMenu::GetItemIllustrationPath(const std::wstring& itemId) const
{
    static const std::unordered_map<std::wstring, std::wstring> kItemIllustrationFiles =
    {
        { L"001", L"item_001_branch.png" },
        { L"002", L"item_002_vine.png" },
        { L"003", L"item_003_scrap_iron.png" },
        { L"004", L"item_004_suspicious_book.png" },
        { L"005", L"item_005_surprise_mushroom.png" },
        { L"006", L"item_006_lighter.png" },
        { L"007", L"item_007_red_spaghetti.png" },
        { L"008", L"item_008_potato_chips.png" },
        { L"009", L"item_009_canned_tomato.png" },
        { L"010", L"item_010_dried_noodles.png" },
        { L"011", L"item_011_potato.png" },
        { L"012", L"item_012_salt.png" },
        { L"013", L"item_013_sturdy_cord.png" },
        { L"014", L"item_014_wind_crystal.png" },
        { L"015", L"item_015_jump_seed.png" },
        { L"016", L"item_016_mysterious_spring.png" },
        { L"017", L"item_017_launch_juice.png" },
        { L"bomb_capacity_up", L"item_bomb_capacity_up.png" },
        { L"buster_rapid_up", L"item_buster_rapid_up.png" },
        { L"star_power_up", L"item_star_power_up.png" },
        { L"speed_up", L"item_speed_up.png" }
    };

    const auto found = kItemIllustrationFiles.find(itemId);
    if (found == kItemIllustrationFiles.end())
    {
        return L"";
    }

    return kItemIllustrationDir + found->second;
}

void PauseMenu::UpdateWeaponList()
{
    if (IsMenuCancelPressed())
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
    if (IsMenuUpPressed())
    {
        if (m_selectedWeaponIndex > 0)
        {
            --m_selectedWeaponIndex;
        }
    }

    if (IsMenuDownPressed())
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

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    if (IsPointInRect(baseMouseX,
                      baseMouseY,
                      kItemListX,
                      kItemListY,
                      520,
                      static_cast<int>(kVisibleWeaponCount) * kWeaponListLineHeight))
    {
        const std::size_t hoveredIndex = m_weaponScrollOffset +
            static_cast<std::size_t>((baseMouseY - kItemListY) / kWeaponListLineHeight);
        if (hoveredIndex < ownedWeapons.size() && hoveredIndex != m_selectedWeaponIndex)
        {
            m_selectedWeaponIndex = hoveredIndex;
            GameAudio::PlayMenuMove();
        }
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kItemListX,
                          kItemListY,
                          520,
                          static_cast<int>(kVisibleWeaponCount) * kWeaponListLineHeight))
        {
            const std::size_t clickedIndex = m_weaponScrollOffset +
                static_cast<std::size_t>((baseMouseY - kItemListY) / kWeaponListLineHeight);
            if (clickedIndex < ownedWeapons.size())
            {
                if (clickedIndex != m_selectedWeaponIndex)
                {
                    m_selectedWeaponIndex = clickedIndex;
                    EnsureSelectedWeaponVisible();
                    GameAudio::PlayMenuMove();
                }
            }
            return;
        }
    }
}

void PauseMenu::UpdateSettingsPanel()
{
    if (IsMenuUpPressed())
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

    if (IsMenuDownPressed())
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

    if (IsMenuLeftPressed())
    {
        MoveSelectedSettingsOption(-1);
    }
    if (IsMenuRightPressed())
    {
        MoveSelectedSettingsOption(1);
    }
    if (IsMenuConfirmPressed())
    {
        ApplySelectedSettings();
    }

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    const InputDevice::MousePosition mouseDelta = InputDevice::Mouse::GetDelta();
    const bool mouseMoved = mouseDelta.x != 0 || mouseDelta.y != 0;
    if (mouseMoved)
    {
        SettingsRow hoveredRow;
        if (TryGetSettingsRowFromPoint(baseMouseX, baseMouseY, &hoveredRow))
        {
            if (hoveredRow != m_selectedSettingsRow)
            {
                m_selectedSettingsRow = hoveredRow;
                GameAudio::PlayMenuMove();
            }
        }
    }

    int selectedRowY = kSettingsFirstRowY;
    if (m_selectedSettingsRow == SettingsRow::WindowMode)
    {
        selectedRowY += kSettingsRowInterval;
    }
    else if (m_selectedSettingsRow == SettingsRow::Quality)
    {
        selectedRowY += kSettingsRowInterval * 2;
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kSettingsLeftArrowX,
                          selectedRowY,
                          kSettingsArrowWidth,
                          kSettingsArrowHeight))
        {
            MoveSelectedSettingsOption(-1);
            return;
        }

        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kSettingsRightArrowX,
                          selectedRowY,
                          kSettingsArrowWidth,
                          kSettingsArrowHeight))
        {
            MoveSelectedSettingsOption(1);
            return;
        }

        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kSettingsApplyX,
                          kSettingsApplyY,
                          kSettingsApplyWidth,
                          kSettingsApplyHeight))
        {
            ApplySelectedSettings();
            return;
        }

        SettingsRow clickedRow;
        if (TryGetSettingsRowFromPoint(baseMouseX, baseMouseY, &clickedRow))
        {
            if (clickedRow != m_selectedSettingsRow)
            {
                m_selectedSettingsRow = clickedRow;
                GameAudio::PlayMenuMove();
            }
            return;
        }

        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kSettingsCancelX,
                          kSettingsCancelY,
                          kSettingsCancelWidth,
                          kSettingsCancelHeight))
        {
            CancelSelectedSettings();
            return;
        }
    }

    if (IsMenuCancelPressed())
    {
        GameAudio::PlayMenuCancel();
        m_focusArea = FocusArea::TopMenu;
    }
}

void PauseMenu::UpdateExitConfirm()
{
    if (IsMenuLeftPressed())
    {
        m_selectedExitConfirmIndex = kExitConfirmYesIndex;
        GameAudio::PlayMenuMove();
    }

    if (IsMenuRightPressed())
    {
        m_selectedExitConfirmIndex = kExitConfirmNoIndex;
        GameAudio::PlayMenuMove();
    }

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    const InputDevice::MousePosition mouseDelta = InputDevice::Mouse::GetDelta();
    const bool mouseMoved = mouseDelta.x != 0 || mouseDelta.y != 0;
    if (mouseMoved)
    {
        int hoveredIndex = -1;
        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitConfirmYesX,
                          kExitConfirmY,
                          kExitConfirmButtonWidth,
                          kExitConfirmButtonHeight))
        {
            hoveredIndex = kExitConfirmYesIndex;
        }
        else if (IsPointInRect(baseMouseX,
                               baseMouseY,
                               kExitConfirmNoX,
                               kExitConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight))
        {
            hoveredIndex = kExitConfirmNoIndex;
        }

        if (hoveredIndex >= 0 && hoveredIndex != m_selectedExitConfirmIndex)
        {
            m_selectedExitConfirmIndex = hoveredIndex;
            GameAudio::PlayMenuMove();
        }
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kExitConfirmYesX,
                          kExitConfirmY,
                          kExitConfirmButtonWidth,
                          kExitConfirmButtonHeight))
        {
            GameAudio::PlayMenuConfirm();
            if (m_exitConfirmAction == ExitConfirmAction::Title)
            {
                m_returnToTitleRequested = true;
            }
            else
            {
                m_exitRequested = true;
            }
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

    if (IsMenuConfirmPressed())
    {
        if (m_selectedExitConfirmIndex == kExitConfirmYesIndex)
        {
            GameAudio::PlayMenuConfirm();
            if (m_exitConfirmAction == ExitConfirmAction::Title)
            {
                m_returnToTitleRequested = true;
            }
            else
            {
                m_exitRequested = true;
            }
            Close();
            return;
        }

        GameAudio::PlayMenuCancel();
        m_showExitConfirm = false;
        return;
    }

    if (IsMenuCancelPressed())
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

    if (m_selectedWeaponIndex >= m_weaponScrollOffset + kVisibleWeaponCount)
    {
        m_weaponScrollOffset = m_selectedWeaponIndex - kVisibleWeaponCount + 1;
    }
}

void PauseMenu::Render(const std::wstring& stageName, const int lives)
{
    if (!m_isOpen || m_render == nullptr)
    {
        return;
    }

    // 開くときはアニメーション完了後に、閉じるときはアニメーション開始と同時にテキストを出し入れする。
    if (m_transitionState != TransitionState::Open)
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

    m_render->DrawImageSized(kHeartImagePath,
                             kLivesDisplayX,
                             kLivesDisplayY,
                             kHeartImageSize,
                             kHeartImageSize,
                             255);
    m_render->DrawTextEx(m_menuItemFontId,
                         L"× " + std::to_wstring(lives),
                         kLivesTextX,
                         kLivesTextY,
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

    if (m_activeTopMenuIndex == kSettingsMenuIndex)
    {
        RenderSettingsPanel();
        return;
    }

    if (m_activeTopMenuIndex == kSaveMenuIndex)
    {
        if (m_showSaveConfirm)
        {
            RenderSaveConfirm();
        }
    }

    if (m_activeTopMenuIndex == kExitMenuIndex)
    {
        RenderExitPanel();
        if (m_showExitConfirm)
        {
            RenderExitConfirm();
        }
    }
}

void PauseMenu::RenderExitPanel()
{
    const int textX = kExitPanelButtonX + 30;
    const int cursorCenterX = kExitPanelButtonX + 15;
    if (m_returnToStageSelectEnabled)
    {
        m_render->DrawTextEx(m_menuItemFontId,
                             L"ステージセレクトに戻る",
                             textX,
                             GetExitPanelButtonY(kExitPanelStageSelectIndex) + 15,
                             kTextColor);
        if (m_selectedExitPanelIndex == kExitPanelStageSelectIndex)
        {
            m_render->DrawImage(kCommandCursorPath,
                                cursorCenterX - kCommandCursorDotCenterX,
                                GetExitPanelButtonY(kExitPanelStageSelectIndex) +
                                    (kExitPanelButtonHeight / 2) -
                                    kCommandCursorDotCenterY,
                                255);
        }
    }

    if (m_returnToTitleEnabled)
    {
        m_render->DrawTextEx(m_menuItemFontId,
                             L"タイトルに戻る",
                             textX,
                             GetExitPanelButtonY(kExitPanelTitleIndex) + 15,
                             kTextColor);
        if (m_selectedExitPanelIndex == kExitPanelTitleIndex)
        {
            m_render->DrawImage(kCommandCursorPath,
                                cursorCenterX - kCommandCursorDotCenterX,
                                GetExitPanelButtonY(kExitPanelTitleIndex) +
                                    (kExitPanelButtonHeight / 2) -
                                    kCommandCursorDotCenterY,
                                255);
        }
    }

    m_render->DrawTextEx(m_menuItemFontId,
                         L"ゲームを終了",
                         textX,
                         GetExitPanelButtonY(kExitPanelGameIndex) + 15,
                         kTextColor);
    if (m_selectedExitPanelIndex == kExitPanelGameIndex)
    {
        m_render->DrawImage(kCommandCursorPath,
                            cursorCenterX - kCommandCursorDotCenterX,
                            GetExitPanelButtonY(kExitPanelGameIndex) +
                                (kExitPanelButtonHeight / 2) -
                                kCommandCursorDotCenterY,
                            255);
    }
}
void PauseMenu::RenderTopMenu()
{
    int hoveredMenuIndex = -1;
    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    int pointedMenuIndex = -1;
    if (TryGetTopMenuIndexFromPoint(baseMouseX, baseMouseY, &pointedMenuIndex) &&
        IsTopMenuItemEnabled(pointedMenuIndex))
    {
        hoveredMenuIndex = pointedMenuIndex;
    }

    const int topMenuBgHeight = kTopMenuItemHeight * 2 - 10;
    const int topMenuBgOffsetY = 12;
    const int topMenuBgWidth = (kTopMenuCount - 1) * kTopMenuItemInterval +
                               kTopMenuItemWidth;
    m_render->DrawImageSized(kMenuTopBackgroundPath,
                             kTopMenuX,
                             kTopMenuY - (topMenuBgHeight / 2) + topMenuBgOffsetY,
                             topMenuBgWidth,
                             topMenuBgHeight,
                             255);

    for (std::size_t i = 0; i < kTopMenuItems.size(); ++i)
    {
        const int menuIndex = static_cast<int>(i);
        const int x = kTopMenuX + menuIndex * kTopMenuItemInterval;
        UINT color = kTextColor;
        if (!IsTopMenuItemEnabled(menuIndex))
        {
            color = D3DCOLOR_RGBA(110, 115, 125, 180);
        }

        m_render->DrawTextExCenter(m_qualityFontId,
                                   kTopMenuItems[i],
                                   x,
                                   kTopMenuY,
                                   kTopMenuItemWidth,
                                   kTopMenuItemHeight,
                                   color);

        bool isCursorTarget = false;
        if (hoveredMenuIndex >= 0)
        {
            isCursorTarget = menuIndex == hoveredMenuIndex;
        }
        else if (m_activeTopMenuIndex < 0)
        {
            isCursorTarget = menuIndex == m_selectedTopMenuIndex;
        }
        if (isCursorTarget)
        {
            // 文字列は DrawTextExCenter で 200x44 の箱の中央に描かれる。
            // 文字数が違っても文字列の中心は箱の中心( x + 幅/2 )で一定になる。
            // 白い点の中心(画像内 +6,+6)が文字列の中心に一致するように画像位置を決める。
            const int cursorCenterX = x + (kTopMenuItemWidth / 2);
            const int cursorCenterY = kTopMenuY + kCommandCursorVerticalOffset;
            m_render->DrawImage(kCommandCursorPath,
                                cursorCenterX - kCommandCursorDotCenterX,
                                cursorCenterY - kCommandCursorDotCenterY,
                                255);
        }
    }
}

bool PauseMenu::IsTopMenuItemEnabled(const int menuIndex) const
{
    if (menuIndex == kSaveMenuIndex)
    {
        return m_saveEnabled;
    }

    return true;
}

void PauseMenu::MoveTopMenuSelection(const int direction)
{
    int nextIndex = m_selectedTopMenuIndex;
    for (int i = 0; i < kTopMenuCount; ++i)
    {
        nextIndex += direction;
        if (nextIndex < 0)
        {
            nextIndex = kTopMenuCount - 1;
        }
        else if (nextIndex >= kTopMenuCount)
        {
            nextIndex = 0;
        }

        if (IsTopMenuItemEnabled(nextIndex))
        {
            m_selectedTopMenuIndex = nextIndex;
            return;
        }
    }
}

void PauseMenu::ActivateTopMenu(const int menuIndex)
{
    if (menuIndex != m_selectedTopMenuIndex)
    {
        m_selectedTopMenuIndex = menuIndex;
        GameAudio::PlayMenuMove();
    }

    GameAudio::PlayMenuConfirm();
    m_showSaveConfirm = false;
    m_showExitConfirm = false;
    m_activeTopMenuIndex = menuIndex;
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
        m_focusArea = FocusArea::ExitPanel;
        m_selectedExitPanelIndex = kExitPanelGameIndex;
        if (m_returnToTitleEnabled)
        {
            m_selectedExitPanelIndex = kExitPanelTitleIndex;
        }
        if (m_returnToStageSelectEnabled)
        {
            m_selectedExitPanelIndex = kExitPanelStageSelectIndex;
        }
    }
}

bool PauseMenu::TryActivateTopMenuFromMouseClick()
{
    if (!InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        return false;
    }

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    int clickedMenuIndex = -1;
    if (!TryGetTopMenuIndexFromPoint(baseMouseX, baseMouseY, &clickedMenuIndex))
    {
        return false;
    }

    if (!IsTopMenuItemEnabled(clickedMenuIndex))
    {
        return false;
    }

    ActivateTopMenu(clickedMenuIndex);
    return true;
}

void PauseMenu::RenderItemPanel()
{
    m_render->DrawImageSized(kMenuItemListBackgroundPath,
                             kItemListBackgroundX,
                             kItemListBackgroundY,
                             kItemListBackgroundWidth,
                             kItemListBackgroundHeight,
                             255);

    m_render->DrawTextEx(m_qualityFontId,
                         L"アイテム名",
                         kItemListX,
                         kItemListHeaderY,
                         kItemListHeaderTextColor);
    m_render->DrawTextExCenter(m_qualityFontId,
                               L"所持数",
                               kItemCountColumnX,
                               kItemListHeaderY,
                               kItemCountColumnWidth,
                               kMenuTextLineHeight,
                               kItemListHeaderTextColor);

    const std::vector<std::size_t> ownedItems = GetOwnedItemIndices();
    if (ownedItems.empty())
    {
        m_render->DrawTextEx(m_qualityFontId,
                             L"所持しているアイテムはありません。",
                             kItemListX,
                             kItemPanelListY,
                             kTextColor);
        return;
    }

    const std::size_t visibleEnd = m_itemScrollOffset + kVisibleItemCount;
    std::size_t itemEnd = visibleEnd;
    if (itemEnd > ownedItems.size())
    {
        itemEnd = ownedItems.size();
    }

    // 上側にスクロールし戻せるアイテムがあれば、リストの上に画像ボタンを表示する。
    if (m_itemScrollOffset > 0)
    {
        m_render->DrawImageSized(kItemScrollUpImagePath,
                                 kItemScrollArrowX,
                                 kItemScrollUpArrowY,
                                 kItemScrollArrowWidth,
                                 kItemScrollArrowHeight,
                                 245);
    }

    for (std::size_t i = m_itemScrollOffset; i < itemEnd; ++i)
    {
        const int lineIndex = static_cast<int>(i - m_itemScrollOffset);
        const ItemData& item = m_items.at(ownedItems.at(i));
        const int rowY = kItemPanelListY + lineIndex * kItemListLineHeight;
        m_render->DrawTextEx(m_qualityFontId,
                             item.name,
                             kItemListX,
                             rowY,
                             kTextColor);
        m_render->DrawTextExCenter(m_qualityFontId,
                                   std::to_wstring(m_inventory->GetItemCount(item.id)),
                                   kItemCountColumnX,
                                   rowY,
                                   kItemCountColumnWidth,
                                   kMenuTextLineHeight,
                                   kTextColor);
        if (i == m_selectedItemIndex)
        {
            m_render->DrawImage(kCommandCursorPath,
                                kItemListX - 17 - kCommandCursorDotCenterX,
                                rowY + 10 - kCommandCursorDotCenterY,
                                255);
        }
    }

    // スクロール下側にまだアイテムがあれば、最終行の下に画像ボタンを表示する。
    if (m_itemScrollOffset + kVisibleItemCount < ownedItems.size())
    {
        m_render->DrawImageSized(kItemScrollDownImagePath,
                                 kItemScrollArrowX,
                                 kItemScrollDownArrowY,
                                 kItemScrollArrowWidth,
                                 kItemScrollArrowHeight,
                                 245);
    }

    const ItemData& selectedItem = m_items.at(ownedItems.at(m_selectedItemIndex));
    const std::wstring illustrationPath = GetItemIllustrationPath(selectedItem.id);
    if (!illustrationPath.empty())
    {
        m_render->DrawImageSized(illustrationPath,
                                 kItemIllustrationX,
                                 kItemIllustrationY,
                                 kItemIllustrationSize,
                                 kItemIllustrationSize,
                                 245);
    }

    const int detailX = 1160;
    m_render->DrawTextEx(m_menuItemFontId,
                         selectedItem.name,
                         detailX,
                         365,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"分類：" + selectedItem.category,
                         detailX,
                         430,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"入手方法：" + selectedItem.acquisition,
                         detailX,
                         475,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"主な用途：" + selectedItem.primaryUse,
                         detailX,
                         520,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"説明",
                         detailX,
                         565,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         selectedItem.description,
                         detailX,
                         600,
                         kTextColor);
    m_render->DrawTextExCenter(m_qualityFontId,
                               L"Enter / Space  使用   Esc  戻る",
                               820,
                               720,
                               640,
                               28,
                               kTextColor);
    if (!m_itemStatusMessage.empty())
    {
        m_render->DrawTextExCenter(m_qualityFontId,
                                   m_itemStatusMessage,
                                   820,
                                   750,
                                   640,
                                   28,
                                   m_itemStatusColor);
    }
}

void PauseMenu::RenderWeaponPanel()
{
    m_render->DrawImageSized(kMenuItemListBackgroundPath,
                             kItemListBackgroundX,
                             kItemListBackgroundY,
                             kItemListBackgroundWidth,
                             kItemListBackgroundHeight,
                             255);

    const std::vector<std::size_t> ownedWeapons = GetOwnedWeaponIndices();
    if (ownedWeapons.empty())
    {
        m_render->DrawTextEx(m_qualityFontId,
                             L"所持している武器はありません。",
                             kItemListX,
                             kItemListY,
                             kTextColor);
        return;
    }

    const std::size_t visibleEnd = m_weaponScrollOffset + kVisibleWeaponCount;
    std::size_t weaponEnd = visibleEnd;
    if (weaponEnd > ownedWeapons.size())
    {
        weaponEnd = ownedWeapons.size();
    }

    for (std::size_t i = m_weaponScrollOffset; i < weaponEnd; ++i)
    {
        const int lineIndex = static_cast<int>(i - m_weaponScrollOffset);
        const WeaponData& weapon = m_weapons.at(ownedWeapons.at(i));
        const int rowY = kItemListY + lineIndex * kWeaponListLineHeight;
        const std::wstring iconPath = GetWeaponIconPath(weapon.id);
        if (!iconPath.empty())
        {
            m_render->DrawImageSized(iconPath,
                                     kWeaponListIconX,
                                     rowY,
                                     kWeaponListIconSize,
                                     kWeaponListIconSize,
                                     255);
        }
        m_render->DrawTextEx(m_stageNameFontId,
                             weapon.name,
                             kWeaponListTextX,
                             rowY + kWeaponListTextYOffset,
                             kTextColor);
        if (i == m_selectedWeaponIndex)
        {
            m_render->DrawImage(kCommandCursorPath,
                                kWeaponListIconX - 17 - kCommandCursorDotCenterX,
                                rowY + kWeaponListCursorCenterYOffset -
                                    kCommandCursorDotCenterY,
                                255);
        }
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
                               kTextColor);

    const WeaponData& selectedWeapon = m_weapons.at(ownedWeapons.at(m_selectedWeaponIndex));
    const int detailX = 800;
    m_render->DrawTextEx(m_menuItemFontId,
                         selectedWeapon.name,
                         detailX,
                         365,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"分類：" + selectedWeapon.category,
                         detailX,
                         430,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"入手方法：" + selectedWeapon.acquisition,
                         detailX,
                         475,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"特徴：" + selectedWeapon.feature,
                         detailX,
                         520,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"状態：使用可能",
                         detailX,
                         565,
                         kTextColor);
    m_render->DrawTextEx(m_qualityFontId,
                         L"説明",
                         detailX,
                         620,
                         kTextColor);
    const std::size_t kWeaponDescriptionLineLength = 28;
    std::wstring descriptionLine1 = selectedWeapon.description;
    std::wstring descriptionLine2;
    if (descriptionLine1.length() > kWeaponDescriptionLineLength)
    {
        descriptionLine2 = descriptionLine1.substr(kWeaponDescriptionLineLength);
        descriptionLine1 = descriptionLine1.substr(0, kWeaponDescriptionLineLength);
    }
    m_render->DrawTextEx(m_qualityFontId,
                         descriptionLine1,
                         detailX,
                         665,
                         kTextColor);
    if (!descriptionLine2.empty())
    {
        m_render->DrawTextEx(m_qualityFontId,
                             descriptionLine2,
                             detailX,
                             695,
                             kTextColor);
    }
}

void PauseMenu::RenderExitConfirm()
{
    const wchar_t* confirmMessage = L"ゲームを終了しますか？";
    if (m_exitConfirmAction == ExitConfirmAction::Title)
    {
        if (m_hasUnsavedChanges)
        {
            confirmMessage = L"未保存のままタイトルに戻りますか？";
        }
        else
        {
            confirmMessage = L"タイトルに戻りますか？";
        }
    }
    else if (m_hasUnsavedChanges)
    {
        confirmMessage = L"未保存のまま終了しますか？";
    }

    m_render->DrawTextExCenter(m_qualityFontId,
                               confirmMessage,
                               kConfirmPromptX,
                               kConfirmPromptY,
                               kConfirmPromptWidth,
                               kConfirmPromptHeight,
                               kSubTextColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"はい",
                               kExitConfirmYesX,
                               kExitConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight,
                               kTextColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"いいえ",
                               kExitConfirmNoX,
                               kExitConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight,
                               kTextColor);
    const int cursorCenterY = kExitConfirmY + (kExitConfirmButtonHeight / 2);
    if (m_selectedExitConfirmIndex == kExitConfirmYesIndex)
    {
        m_render->DrawImage(kCommandCursorPath,
                            kExitConfirmYesX + 20 - kCommandCursorDotCenterX,
                            cursorCenterY - kCommandCursorDotCenterY,
                            255);
    }
    else if (m_selectedExitConfirmIndex == kExitConfirmNoIndex)
    {
        m_render->DrawImage(kCommandCursorPath,
                            kExitConfirmNoX + 20 - kCommandCursorDotCenterX,
                            cursorCenterY - kCommandCursorDotCenterY,
                            255);
    }
}

void PauseMenu::RenderSettingsPanel()
{
    const int leftX = kSettingsRowTextX;
    const int leftY = kSettingsFirstRowY + 9;
    m_render->DrawTextEx(m_qualityFontId,
                         L"解像度",
                         leftX,
                         leftY,
                         kTextColor);

    m_render->DrawTextEx(m_qualityFontId,
                         L"表示モード",
                         leftX,
                         leftY + kSettingsRowInterval,
                         kTextColor);

    m_render->DrawTextEx(m_qualityFontId,
                         L"描画品質",
                         leftX,
                         leftY + kSettingsRowInterval * 2,
                         kTextColor);

    int selectedRowY = leftY;
    if (m_selectedSettingsRow == SettingsRow::WindowMode)
    {
        selectedRowY = leftY + kSettingsRowInterval;
    }
    else if (m_selectedSettingsRow == SettingsRow::Quality)
    {
        selectedRowY = leftY + kSettingsRowInterval * 2;
    }
    m_render->DrawImage(kCommandCursorPath,
                        leftX - 13 - kCommandCursorDotCenterX,
                        selectedRowY + 13 - kCommandCursorDotCenterY,
                        255);

    RenderSettingsOptionList(m_selectedSettingsRow);

    m_render->DrawTextExCenter(m_qualityFontId,
                               L"↑↓ 項目選択   ←→ 値変更   Enter 適用   クリック 操作   Esc 戻る",
                               170,
                               730,
                               900,
                               36,
                               kSubTextColor);
}

void PauseMenu::RenderSettingsOptionList(const SettingsRow row)
{
    const int optionCount = GetSettingsOptionCount(row);
    const int selectedIndex = GetSelectedSettingsOptionIndex(row);
    m_render->DrawTextExCenter(m_qualityFontId,
                               BuildResolutionComboText(),
                               kSettingsOptionListX,
                               kSettingsFirstRowY,
                               kSettingsOptionListWidth,
                               kSettingsValueHeight,
                               kTextColor);
    m_render->DrawTextExCenter(m_qualityFontId,
                               BuildWindowModeComboText(),
                               kSettingsOptionListX,
                               kSettingsFirstRowY + kSettingsRowInterval,
                               kSettingsOptionListWidth,
                               kSettingsValueHeight,
                               kTextColor);
    m_render->DrawTextExCenter(m_qualityFontId,
                               BuildQualityComboText(),
                               kSettingsOptionListX,
                               kSettingsFirstRowY + kSettingsRowInterval * 2,
                               kSettingsOptionListWidth,
                               kSettingsValueHeight,
                               kTextColor);

    int selectedRowY = kSettingsFirstRowY;
    if (row == SettingsRow::WindowMode)
    {
        selectedRowY += kSettingsRowInterval;
    }
    else if (row == SettingsRow::Quality)
    {
        selectedRowY += kSettingsRowInterval * 2;
    }

    int leftArrowButtonTransparency = kSettingsButtonTransparency;
    int leftArrowIconTransparency = kSettingsArrowIconTransparency;
    if (selectedIndex <= 0)
    {
        leftArrowButtonTransparency = kDisabledSettingsButtonTransparency;
        leftArrowIconTransparency = kDisabledSettingsArrowIconTransparency;
    }
    int rightArrowButtonTransparency = kSettingsButtonTransparency;
    int rightArrowIconTransparency = kSettingsArrowIconTransparency;
    if (selectedIndex >= optionCount - 1)
    {
        rightArrowButtonTransparency = kDisabledSettingsButtonTransparency;
        rightArrowIconTransparency = kDisabledSettingsArrowIconTransparency;
    }
    m_render->DrawImageSized(kSettingsArrowButtonImagePath,
                             kSettingsLeftArrowX,
                             selectedRowY,
                             kSettingsArrowWidth,
                             kSettingsArrowHeight,
                             leftArrowButtonTransparency);
    m_render->DrawImageSized(kSettingsArrowButtonImagePath,
                             kSettingsRightArrowX,
                             selectedRowY,
                             kSettingsArrowWidth,
                             kSettingsArrowHeight,
                             rightArrowButtonTransparency);
    m_render->DrawImageSized(kSettingsLeftArrowImagePath,
                             kSettingsLeftArrowX + kSettingsArrowIconOffsetX,
                             selectedRowY + kSettingsArrowIconOffsetY,
                             kSettingsArrowIconSize,
                             kSettingsArrowIconSize,
                             leftArrowIconTransparency);
    m_render->DrawImageSized(kSettingsRightArrowImagePath,
                             kSettingsRightArrowX + kSettingsArrowIconOffsetX,
                             selectedRowY + kSettingsArrowIconOffsetY,
                             kSettingsArrowIconSize,
                             kSettingsArrowIconSize,
                             rightArrowIconTransparency);

    UINT applyColor = kDisabledSettingsTextColor;
    if (IsSettingsDirty())
    {
        applyColor = kTextColor;
    }

    int applyTransparency = kDisabledSettingsButtonTransparency;
    if (IsSettingsDirty())
    {
        applyTransparency = kSettingsButtonTransparency;
    }
    m_render->DrawImageSized(kSettingsApplyButtonImagePath,
                             kSettingsApplyX,
                             kSettingsApplyY,
                             kSettingsApplyWidth,
                             kSettingsApplyHeight,
                             applyTransparency);
    m_render->DrawImageSized(kSettingsCancelButtonImagePath,
                             kSettingsCancelX,
                             kSettingsCancelY,
                             kSettingsCancelWidth,
                             kSettingsCancelHeight,
                             kSettingsButtonTransparency);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"適用",
                               kSettingsApplyX,
                               kSettingsApplyY,
                               kSettingsApplyWidth,
                               kSettingsApplyHeight,
                               applyColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"キャンセル",
                               kSettingsCancelX,
                               kSettingsCancelY,
                               kSettingsCancelWidth,
                               kSettingsCancelHeight,
                               kTextColor);

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    int cursorX = -1;
    int cursorY = -1;
    if (selectedIndex > 0 &&
        IsPointInRect(baseMouseX,
                      baseMouseY,
                      kSettingsLeftArrowX,
                      selectedRowY,
                      kSettingsArrowWidth,
                      kSettingsArrowHeight))
    {
        cursorX = kSettingsLeftArrowX + 12;
        cursorY = selectedRowY + kSettingsArrowHeight / 2;
    }
    else if (selectedIndex < optionCount - 1 &&
             IsPointInRect(baseMouseX,
                           baseMouseY,
                           kSettingsRightArrowX,
                           selectedRowY,
                           kSettingsArrowWidth,
                           kSettingsArrowHeight))
    {
        cursorX = kSettingsRightArrowX + 12;
        cursorY = selectedRowY + kSettingsArrowHeight / 2;
    }
    else if (IsSettingsDirty() &&
             IsPointInRect(baseMouseX,
                           baseMouseY,
                           kSettingsApplyX,
                           kSettingsApplyY,
                           kSettingsApplyWidth,
                           kSettingsApplyHeight))
    {
        cursorX = kSettingsApplyX + 12;
        cursorY = kSettingsApplyY + kSettingsApplyHeight / 2;
    }
    else if (IsPointInRect(baseMouseX,
                           baseMouseY,
                           kSettingsCancelX,
                           kSettingsCancelY,
                           kSettingsCancelWidth,
                           kSettingsCancelHeight))
    {
        cursorX = kSettingsCancelX + 12;
        cursorY = kSettingsCancelY + kSettingsCancelHeight / 2;
    }

    if (cursorX >= 0 && cursorY >= 0)
    {
        m_render->DrawImage(kCommandCursorPath,
                            cursorX - kCommandCursorDotCenterX,
                            cursorY - kCommandCursorDotCenterY,
                            255);
    }
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

    m_resolutionOptions.push_back(std::make_pair(NSRender::Common::ScreenW(),
                                                 NSRender::Common::ScreenH()));
    std::sort(m_resolutionOptions.begin(),
              m_resolutionOptions.end(),
              [](const std::pair<int, int>& left, const std::pair<int, int>& right)
              {
                  const long long leftPixels = static_cast<long long>(left.first) * left.second;
                  const long long rightPixels = static_cast<long long>(right.first) * right.second;
                  if (leftPixels != rightPixels)
                  {
                      return leftPixels < rightPixels;
                  }
                  if (left.first != right.first)
                  {
                      return left.first < right.first;
                  }
                  return left.second < right.second;
              });
    m_resolutionOptions.erase(std::unique(m_resolutionOptions.begin(),
                                          m_resolutionOptions.end()),
                              m_resolutionOptions.end());

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

    std::wstring quality = L"LOW";
    if (m_render != nullptr)
    {
        quality = m_render->GetRenderQuality();
    }
    m_selectedQualityIndex = 0;
    if (quality == L"MIDDLE" || quality == L"HIGH")
    {
        m_selectedQualityIndex = 1;
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
        quality = L"HIGH";
    }
    m_render->SetRenderQuality(quality);
}

void PauseMenu::ApplySelectedSettings()
{
    const bool resolutionDirty = IsSettingsRowDirty(SettingsRow::Resolution);
    const bool windowModeDirty = IsSettingsRowDirty(SettingsRow::WindowMode);
    const bool qualityDirty = IsSettingsRowDirty(SettingsRow::Quality);
    if (!resolutionDirty && !windowModeDirty && !qualityDirty)
    {
        return;
    }

    if (qualityDirty)
    {
        ApplySelectedQuality();
    }
    if (windowModeDirty)
    {
        ApplySelectedWindowMode();
    }
    if (resolutionDirty)
    {
        ApplySelectedResolution();
    }

    RefreshSettingsOptions();
    GameAudio::PlayMenuConfirm();
}

void PauseMenu::CancelSelectedSettings()
{
    RefreshSettingsOptions();
    GameAudio::PlayMenuCancel();
}

bool PauseMenu::IsSettingsRowDirty(const SettingsRow row) const
{
    if (row == SettingsRow::Resolution)
    {
        if (m_selectedResolutionIndex < 0 ||
            m_selectedResolutionIndex >= static_cast<int>(m_resolutionOptions.size()))
        {
            return false;
        }

        const std::pair<int, int>& resolution = m_resolutionOptions.at(m_selectedResolutionIndex);
        return resolution.first != NSRender::Common::ScreenW() ||
               resolution.second != NSRender::Common::ScreenH();
    }

    if (row == SettingsRow::WindowMode)
    {
        if (m_render == nullptr)
        {
            return false;
        }

        NSRender::eWindowMode selectedMode = NSRender::eWindowMode::WINDOW;
        if (m_selectedWindowModeIndex == 1)
        {
            selectedMode = NSRender::eWindowMode::BORDERLESS;
        }
        return selectedMode != m_render->GetWindowMode();
    }

    if (row == SettingsRow::Quality)
    {
        if (m_render == nullptr)
        {
            return false;
        }

        const std::wstring quality = m_render->GetRenderQuality();
        if (m_selectedQualityIndex == 1)
        {
            return quality != L"MIDDLE" && quality != L"HIGH";
        }
        return quality == L"MIDDLE" || quality == L"HIGH";
    }

    return false;
}

bool PauseMenu::IsSettingsDirty() const
{
    return IsSettingsRowDirty(SettingsRow::Resolution) ||
           IsSettingsRowDirty(SettingsRow::WindowMode) ||
           IsSettingsRowDirty(SettingsRow::Quality);
}

int PauseMenu::GetSettingsOptionCount(const SettingsRow row) const
{
    if (row == SettingsRow::Resolution)
    {
        return static_cast<int>(m_resolutionOptions.size());
    }

    if (row == SettingsRow::WindowMode)
    {
        return 2;
    }

    return 2;
}

int PauseMenu::GetSelectedSettingsOptionIndex(const SettingsRow row) const
{
    if (row == SettingsRow::Resolution)
    {
        return m_selectedResolutionIndex;
    }

    if (row == SettingsRow::WindowMode)
    {
        return m_selectedWindowModeIndex;
    }

    return m_selectedQualityIndex;
}

std::wstring PauseMenu::GetSettingsOptionLabel(const SettingsRow row, const int index) const
{
    if (row == SettingsRow::Resolution)
    {
        if (index < 0 || index >= static_cast<int>(m_resolutionOptions.size()))
        {
            return L"-";
        }

        const std::pair<int, int>& resolution = m_resolutionOptions.at(index);
        return FormatResolutionLabel(resolution.first, resolution.second);
    }

    if (row == SettingsRow::WindowMode)
    {
        if (index == 1)
        {
            return WindowModeToLabel(NSRender::eWindowMode::BORDERLESS);
        }

        return WindowModeToLabel(NSRender::eWindowMode::WINDOW);
    }

    if (index == 1)
    {
        return QualityToLabel(L"HIGH");
    }

    return QualityToLabel(L"LOW");
}

void PauseMenu::SetSelectedSettingsOptionIndex(const SettingsRow row, const int index)
{
    const int optionCount = GetSettingsOptionCount(row);
    if (index < 0 || index >= optionCount)
    {
        return;
    }

    if (row == SettingsRow::Resolution)
    {
        m_selectedResolutionIndex = index;
    }
    else if (row == SettingsRow::WindowMode)
    {
        m_selectedWindowModeIndex = index;
    }
    else
    {
        m_selectedQualityIndex = index;
    }
}

void PauseMenu::MoveSelectedSettingsOption(const int direction)
{
    const int optionCount = GetSettingsOptionCount(m_selectedSettingsRow);
    if (optionCount <= 0)
    {
        return;
    }

    int nextIndex = GetSelectedSettingsOptionIndex(m_selectedSettingsRow) + direction;
    if (nextIndex < 0)
    {
        nextIndex = 0;
    }
    else if (nextIndex >= optionCount)
    {
        nextIndex = optionCount - 1;
    }

    if (nextIndex == GetSelectedSettingsOptionIndex(m_selectedSettingsRow))
    {
        return;
    }

    SetSelectedSettingsOptionIndex(m_selectedSettingsRow, nextIndex);
    GameAudio::PlayMenuMove();
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
    if (quality == L"MIDDLE" || quality == L"HIGH")
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

    if (IsPointInRect(x, y, kSettingsRowX, 342, kSettingsRowWidth, 62))
    {
        *outRow = SettingsRow::Resolution;
        return true;
    }

    if (IsPointInRect(x, y, kSettingsRowX, 416, kSettingsRowWidth, 62))
    {
        *outRow = SettingsRow::WindowMode;
        return true;
    }

    if (IsPointInRect(x, y, kSettingsRowX, 490, kSettingsRowWidth, 62))
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
    for (std::size_t i = 0; i < m_weapons.size(); ++i)
    {
        if (m_inventory != nullptr && m_inventory->GetWeaponCount(m_weapons.at(i).id) > 0)
        {
            indices.push_back(i);
        }
    }

    return indices;
}

bool PauseMenu::ConsumeExitRequested()
{
    if (m_isOpen)
    {
        return false;
    }
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

bool PauseMenu::ConsumeReturnToStageSelectRequested()
{
    if (m_isOpen)
    {
        return false;
    }
    const bool requested = m_returnToStageSelectRequested;
    m_returnToStageSelectRequested = false;
    return requested;
}

bool PauseMenu::ConsumeReturnToTitleRequested()
{
    if (m_isOpen)
    {
        return false;
    }
    const bool requested = m_returnToTitleRequested;
    m_returnToTitleRequested = false;
    return requested;
}

void PauseMenu::UpdateSaveConfirm()
{
    if (IsMenuLeftPressed())
    {
        m_selectedSaveConfirmIndex = kSaveConfirmYesIndex;
        GameAudio::PlayMenuMove();
    }

    if (IsMenuRightPressed())
    {
        m_selectedSaveConfirmIndex = kSaveConfirmNoIndex;
        GameAudio::PlayMenuMove();
    }

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                         static_cast<float>(NSRender::Common::ScreenW());
    const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                         static_cast<float>(NSRender::Common::ScreenH());
    const long baseMouseX = static_cast<long>(static_cast<float>(mousePosition.x) * scaleX);
    const long baseMouseY = static_cast<long>(static_cast<float>(mousePosition.y) * scaleY);
    const InputDevice::MousePosition mouseDelta = InputDevice::Mouse::GetDelta();
    const bool mouseMoved = mouseDelta.x != 0 || mouseDelta.y != 0;
    if (mouseMoved)
    {
        int hoveredIndex = -1;
        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kSaveConfirmYesX,
                          kSaveConfirmY,
                          kExitConfirmButtonWidth,
                          kExitConfirmButtonHeight))
        {
            hoveredIndex = kSaveConfirmYesIndex;
        }
        else if (IsPointInRect(baseMouseX,
                               baseMouseY,
                               kSaveConfirmNoX,
                               kSaveConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight))
        {
            hoveredIndex = kSaveConfirmNoIndex;
        }

        if (hoveredIndex >= 0 && hoveredIndex != m_selectedSaveConfirmIndex)
        {
            m_selectedSaveConfirmIndex = hoveredIndex;
            GameAudio::PlayMenuMove();
        }
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        if (IsPointInRect(baseMouseX,
                          baseMouseY,
                          kSaveConfirmYesX,
                          kSaveConfirmY,
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
                          kSaveConfirmNoX,
                          kSaveConfirmY,
                          kExitConfirmButtonWidth,
                          kExitConfirmButtonHeight))
        {
            GameAudio::PlayMenuCancel();
            m_showSaveConfirm = false;
            m_activeTopMenuIndex = -1;
            return;
        }
    }

    if (IsMenuConfirmPressed())
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

    if (IsMenuCancelPressed())
    {
        GameAudio::PlayMenuCancel();
        m_showSaveConfirm = false;
        m_activeTopMenuIndex = -1;
    }
}

void PauseMenu::RenderSaveConfirm()
{
    m_render->DrawTextEx(m_menuItemFontId,
                         L"セーブする",
                         kSaveConfirmPromptX,
                         kSaveConfirmPromptY,
                         kTextColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"はい",
                               kSaveConfirmYesX,
                               kSaveConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight,
                               kTextColor);
    m_render->DrawTextExCenter(m_menuItemFontId,
                               L"いいえ",
                               kSaveConfirmNoX,
                               kSaveConfirmY,
                               kExitConfirmButtonWidth,
                               kExitConfirmButtonHeight,
                               kTextColor);
    const int cursorCenterY = kSaveConfirmY + (kExitConfirmButtonHeight / 2);
    if (m_selectedSaveConfirmIndex == kSaveConfirmYesIndex)
    {
        m_render->DrawImage(kCommandCursorPath,
                            kSaveConfirmYesX + 20 - kCommandCursorDotCenterX,
                            cursorCenterY - kCommandCursorDotCenterY,
                            255);
    }
    else if (m_selectedSaveConfirmIndex == kSaveConfirmNoIndex)
    {
        m_render->DrawImage(kCommandCursorPath,
                            kSaveConfirmNoX + 20 - kCommandCursorDotCenterX,
                            cursorCenterY - kCommandCursorDotCenterY,
                            255);
    }
}

bool PauseMenu::IsOpen() const
{
    return m_isOpen;
}

bool PauseMenu::BlocksGameInput() const
{
    return m_isOpen;
}

void PauseMenu::SetHasUnsavedChanges(const bool hasUnsavedChanges)
{
    m_hasUnsavedChanges = hasUnsavedChanges;
}

void PauseMenu::SetItemUseCallback(std::function<bool(const std::wstring&)> callback)
{
    m_itemUseCallback = std::move(callback);
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

int PauseMenu::GetExitPanelButtonY(const int index) const
{
    int buttonY = kExitPanelFirstY;
    if (index == kExitPanelStageSelectIndex)
    {
        return buttonY;
    }

    if (m_returnToStageSelectEnabled)
    {
        buttonY += kExitPanelButtonInterval;
    }
    if (index == kExitPanelTitleIndex)
    {
        return buttonY;
    }

    if (m_returnToTitleEnabled)
    {
        buttonY += kExitPanelButtonInterval;
    }
    return buttonY;
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
