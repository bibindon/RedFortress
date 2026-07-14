#include "GameApp.h"

#include "resource.h"
#include "GameAudio.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Util.h"
#include "../../RedFortressRender/Render/Camera.h"
#include "../../RedFortressRender/Render/Common.h"

namespace
{
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
    std::string NarrowDebugIdentifier(const std::wstring& identifier)
    {
        std::string result;
        result.reserve(identifier.size());
        for (const wchar_t value : identifier)
        {
            if (value < 0 || value > 0x7f)
            {
                throw std::runtime_error("Debug RPC identifiers must contain ASCII characters only.");
            }
            result.push_back(static_cast<char>(value));
        }
        return result;
    }

    int GetDebugKeyCode(std::string keyName)
    {
        std::transform(keyName.begin(), keyName.end(), keyName.begin(), [](const unsigned char value) {
            return static_cast<char>(toupper(value));
        });

        if (keyName == "RETURN" || keyName == "ENTER")
        {
            return DIK_RETURN;
        }
        if (keyName == "SPACE")
        {
            return DIK_SPACE;
        }
        if (keyName == "ESCAPE" || keyName == "ESC")
        {
            return DIK_ESCAPE;
        }
        if (keyName == "LEFT")
        {
            return DIK_LEFT;
        }
        if (keyName == "RIGHT")
        {
            return DIK_RIGHT;
        }
        if (keyName == "UP")
        {
            return DIK_UP;
        }
        if (keyName == "DOWN")
        {
            return DIK_DOWN;
        }
        if (keyName == "W")
        {
            return DIK_W;
        }
        if (keyName == "A")
        {
            return DIK_A;
        }
        if (keyName == "S")
        {
            return DIK_S;
        }
        if (keyName == "D")
        {
            return DIK_D;
        }
        if (keyName == "R")
        {
            return DIK_R;
        }
        if (keyName == "LCONTROL" || keyName == "CTRL")
        {
            return DIK_LCONTROL;
        }
        return -1;
    }

    bool TryParseDebugBoolean(std::string value, bool* result)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
            return static_cast<char>(toupper(character));
        });
        if (value == "TRUE" || value == "1" || value == "ON")
        {
            *result = true;
            return true;
        }
        if (value == "FALSE" || value == "0" || value == "OFF")
        {
            *result = false;
            return true;
        }
        return false;
    }

#endif

    const std::wstring g_arrowSoundPath = L"res\\sound\\arrow.wav";
    const std::wstring g_playerMeshPath = L"res\\model2\\marine_512\\marine.x";
    const std::wstring g_playerAnimCsvPath = L"res\\model2\\marine_512\\marine.csv";
    const std::wstring g_playerIdleAnimName = L"000";
    const std::wstring g_playerWalkAnimName = L"walk";
    const std::wstring g_playerRunAnimName = L"run";
    const std::wstring g_playerJumpAnimName = L"jump";
    const std::wstring g_finImagePath = L"res\\2D_Image\\fin.png";
    const std::wstring g_gameOverImagePath = L"res\\2D_Image\\gameover.png";
    const float kPlayerWalkAnimationSpeed = 1.3f;
    const float kStagePortalClickRadius = 48.0f;
    const float kStageSelectPlayerMoveDuration = 0.5f;
    const float kStageSelectTransitionFadeDuration = 0.3f;
    const float kStageSelectPlayerRightYaw = -D3DX_PI * 0.5f;
    const float kStageSelectPlayerLeftYaw = D3DX_PI * 0.5f;
    const float kStageSelectPlayerVisualOffsetY = 1.0f;
    const float kStageSelectPlayerVisualScale = 1.0f;
    const float kStageSelectPlayerLightHeight = 3.2f;
    const wchar_t* kStageSelectPlayerLightOwnerTag = L"stage-select-player";
    const int kStageSelectStageNameX = 48;
    const int kStageSelectStageNameY = 42;
    const int kStageSelectLivesX = 1190;
    const int kStageSelectLivesY = 42;
    const int kStageSelectLivesWidth = 360;
    const int kStageSelectHintY = 852;
    const int kStageSelectStartButtonX = 650;
    const int kStageSelectStartButtonY = 790;
    const int kStageSelectStartButtonWidth = 300;
    const int kStageSelectStartButtonHeight = 54;
    const std::wstring kStageSelectCubeRedPath = L"res\\model\\cube_red.x";
    const std::wstring kStageSelectCubeGreenPath = L"res\\model\\cubeGreen\\cube_green.x";
    const std::wstring kStageSelectCubeBluePath = L"res\\model\\cubeBlue\\cube_blue.x";
    const float kStageSelectCubeScale = 0.16666667f;
    const std::wstring kAttackClubIconPath = L"res\\2D_Image\\attack_club_icon.png";
    const std::wstring kAttackSlashIconPath = L"res\\2D_Image\\attack_slash_icon.png";
    const std::wstring kAttackBombIconPath = L"res\\2D_Image\\attack_bomb_icon.png";
    const std::wstring kAttackBusterIconPath = L"res\\2D_Image\\attack_buster_icon.png";
    const int kAttackTypeHudX = 42;
    const int kAttackTypeHudY = 86;
    const int kAttackTypeIconSize = 77;
    const std::wstring kAmmoRailImagePath = L"res\\2D_Image\\ammo_rail.png";
    const std::wstring kAmmoBeadFullImagePath = L"res\\2D_Image\\ammo_bead_full.png";
    const std::wstring kAmmoBeadEmptyImagePath = L"res\\2D_Image\\ammo_bead_empty.png";
    const std::wstring kItemNameCsvPath = L"res\\script\\hoshigirl_item_ideas.csv";
    const std::wstring kQteRewardItemId = L"001";
    const std::wstring kStickModelPath = L"res\\model\\stick\\stick.x";
    const std::wstring kSaberModelPath = L"res\\model\\saber\\saber.x";
    const char* kPlayerLeftWristBoneName = "Bone_242";
    const float kStickModelScale = 0.5f;
    const float kSaberModelScale = 0.5f;
    const std::wstring kBombCapacityUpItemId = L"bomb_capacity_up";
    const std::wstring kBusterRapidUpItemId = L"buster_rapid_up";
    const std::wstring kInitialClubWeaponId = L"W001";
    const std::wstring kRedSpaghettiItemId = L"007";
    const std::wstring kPotatoChipsItemId = L"008";
    const std::wstring kChuageJuiceItemId = L"017";
    const int kItemPickupMessageTotalFrames = 180;
    const int kItemPickupMessageFadeFrames = 24;
    const int kItemPickupMessageY = 780;

    const float CAMERA_MOVE_SPEED = 0.08f;
    const float CAMERA_FAST_MOVE_SPEED = 0.25f;
    const float MOUSE_CAMERA_SENSITIVITY_NORMAL = 0.005f;
    const float MOUSE_CAMERA_SENSITIVITY_REMOTE = 0.00025f;
    const int kRemoteDesktopScreenWidth = 1600;
    const int kRemoteDesktopScreenHeight = 900;
    const int kNormalScreenWidth = 1920;
    const int kNormalScreenHeight = 1080;
    const float kPlayerTurnRadiansPerSecond = 10.0f;
    const float kTargetFrameSeconds = 1.0f / 60.0f;
    const float kMinCameraDistance = 1.5f;
    const float kMaxCameraDistance = 20.0f;
    const float kCameraWheelZoomStep = 0.5f;
    const int kPlayerInvincibleDuration = 60;
    const int kRespawnInvincibleFrames = 180;
    const int kKnockbackDurationFrames = 60;
    const float kKnockbackSpeed = 1.0f;
    const int kRespawnCameraDelayFrames = 120;
    const int kRespawnCameraMoveFrames = 30;
    const int kStageTitleFrameMax = 180;
    const int kGameOverFadeFrames = 18;
    const float kFallDeathY = -10.0f;
    const int kFallDeathFrames = 90;
    const float kEnemyAttackKnockbackDistance = 0.2f;
    const int kEnemyAttackKnockbackFrames = 60;
    const std::wstring kGoalArrowModelPath = L"res\\model\\arrow\\arrow.x";
    const float kGoalArrowHeadOffsetY = 2.3f;
    const float kGoalArrowScale = 0.42f;
    const int kStageIntroLetterboxFrames = 25;
    const int kStageIntroHoldFrames = 80;
    const int kStageIntroOutFrames = 25;
    const int kLetterboxBarHeight = 130;
    const std::wstring kLetterboxBarImagePath = L"res\\2D_Image\\black2x2.bmp";
    const std::wstring kStageClearRingImagePath = L"res\\2D_Image\\stage_clear_ring.png";
    const std::wstring kStageClearSparklesImagePath = L"res\\2D_Image\\stage_clear_sparkles.png";
    const std::wstring kStageClearFrameImagePath = L"res\\2D_Image\\stage_clear_frame.png";
    const std::wstring kStageClearFlashImagePath = L"res\\2D_Image\\white_bar.bmp";
    const int kStageClearCameraMoveFrames = 45;
    const int kStageClearIdleFrame = 12;
    const int kStageClearSlashFrame = 28;
    const int kStageClearSlashEndFrame = 82;
    const int kStageClearTitleFrame = 58;
    const int kStageClearInputFrame = 135;
    const int kStageClearFinalAutoFrame = 240;
    const int kStageClearLetterboxHeight = 90;
    const int kStageClearReplayTitleFrame = 8;
    const int kStageClearReplayInputFrame = 28;
    const int kStageClearReplayFinalAutoFrame = 90;
    const int kStageClearReplayLetterboxHeight = 40;
    const float kStageClearTargetFovDegrees = 58.0f;
    const int kStageExitJumpDelayFrames = 30;
    const int kStageExitJumpDurationFrames = 30;
    const int kStageExitFadeStartFrame = kStageExitJumpDelayFrames + 8;
    const int kStageExitBlackHoldFrames = 60;
    const int kStageExitTransitionFrame = kStageExitJumpDelayFrames +
                                          kStageExitJumpDurationFrames +
                                          kStageExitBlackHoldFrames;
    const float kStageExitRiseHeight = 1.2f;
    const float kStageExitAnimationSpeed = 1.2f;
    const float kStageExitFadeDurationSeconds = 0.35f;
    const int kQteVisualRestoreFrames = 24;
    const float kQteVisualMinSaturate = 0.10f;
    const float kQteVisualMaxFovReduction = 18.0f;
    const std::wstring kBombModelPath = L"res\\model\\bomb\\bomb.x";
    const int kBombFrames = 120;
    const float kBombPlaceDistance = 1.5f;
    const float kBombRadius = 0.25f;
    const float kBombCollisionCenterY = 0.25f;
    const float kBombGravity = 9.8f;
    const float kBombExplosionRadius = 3.0f;
    const int kBombExplosionDamage = 10;
    const int kBombKnockbackFrames = 20;
    const int kBombBlinkStartFrames = 60;
    const int kBombBlinkInterval = 4;
    const std::wstring kBusterModelPath = L"res\\model\\Buster\\buster.x";
    const float kBusterSpawnHeight = 1.0f;
    const float kBusterScale = 0.5f;
    const float kBusterSpeed = 20.0f;
    const float kBusterMaxDistance = 10.0f;
    const int kBusterDamage = 3;
    const float kBusterHitRadius = 0.5f;
    const float kDestructibleHitRadius = 0.9f;
    const int kEnemyItemDropPercent = 25;
    const int kEnemyAmmoHeartDropPercent = 25;
    const int kBombAmmoMax = 10;
    const int kBusterAmmoMax = 30;
    const int kBombAmmoRecoverAmount = 1;
    const int kBusterAmmoRecoverAmount = 3;
    const int kBusterRapidLevelMax = 8;
    const int kBusterCooldownByLevel[kBusterRapidLevelMax] = { 24, 20, 16, 12, 9, 6, 4, 3 };
    const float kEnemyAttackTargetHeight = 1.0f;
    const int kAmmoGaugeX = 130;
    const int kAmmoGaugeY = 78;
    const int kAmmoRailHeight = 5;
    const int kAmmoRailOffsetY = 11;
    const int kAmmoBeadSize = 14;
    const int kAmmoBeadStep = 15;
    const int kWeakAttackHitStopFrames = 8;
    const int kStrongAttackHitStopFrames = 8;

    D3DXVECTOR3 GetEnemyAttackTargetPosition(const EnemyBase& enemy)
    {
        return enemy.GetPosition() + D3DXVECTOR3(0.0f, kEnemyAttackTargetHeight, 0.0f);
    }

    bool IsStageSelectId(const std::wstring& stageId)
    {
        return stageId.length() >= 6 && stageId.substr(0, 6) == L"select";
    }

    bool TryParsePositiveNumber(const std::wstring& text, const std::size_t begin, const std::size_t end, int* value)
    {
        if (value == nullptr || begin >= end || end > text.length())
        {
            return false;
        }

        int result = 0;
        for (std::size_t i = begin; i < end; ++i)
        {
            const wchar_t ch = text.at(i);
            if (ch < L'0' || ch > L'9')
            {
                return false;
            }

            result = result * 10 + static_cast<int>(ch - L'0');
        }

        *value = result;
        return true;
    }

    bool TryParseStageDestinationId(const std::wstring& destinationId, int* worldNumber, int* stageNumber)
    {
        const std::size_t separator = destinationId.find(L'-');
        if (separator == std::wstring::npos)
        {
            return false;
        }

        int parsedWorld = 0;
        int parsedStage = 0;
        if (!TryParsePositiveNumber(destinationId, 0, separator, &parsedWorld))
        {
            return false;
        }
        if (!TryParsePositiveNumber(destinationId, separator + 1, destinationId.length(), &parsedStage))
        {
            return false;
        }

        *worldNumber = parsedWorld;
        *stageNumber = parsedStage;
        return true;
    }

    bool TryParseStageSelectDestinationId(const std::wstring& destinationId, int* worldNumber)
    {
        const std::wstring prefix = L"select";
        if (worldNumber == nullptr ||
            destinationId.length() <= prefix.length() ||
            destinationId.substr(0, prefix.length()) != prefix)
        {
            return false;
        }

        return TryParsePositiveNumber(destinationId, prefix.length(), destinationId.length(), worldNumber);
    }

    bool IsStageEndpointConnectedToSelect(const int stageWorldNumber,
                                          const int stageNumber,
                                          const int selectWorldNumber)
    {
        if (selectWorldNumber == stageWorldNumber - 1 && stageNumber == 1)
        {
            return true;
        }
        if (selectWorldNumber == stageWorldNumber + 1 && stageNumber == 8)
        {
            return true;
        }

        return false;
    }

    bool IsStageSelectNavigationPortal(const std::wstring& portalId)
    {
        const std::wstring prefix = L"portal-to-";
        if (portalId.length() <= prefix.length() || portalId.substr(0, prefix.length()) != prefix)
        {
            return false;
        }

        int worldNumber = 0;
        return TryParseStageSelectDestinationId(portalId.substr(prefix.length()), &worldNumber);
    }

    bool AreStagePortalsSequential(const std::wstring& currentPortalId, const std::wstring& candidatePortalId)
    {
        const std::wstring prefix = L"portal-to-";
        if (currentPortalId.length() <= prefix.length() ||
            currentPortalId.substr(0, prefix.length()) != prefix ||
            candidatePortalId.length() <= prefix.length() ||
            candidatePortalId.substr(0, prefix.length()) != prefix)
        {
            return true;
        }

        int currentWorldNumber = 0;
        int currentStageNumber = 0;
        const std::wstring currentDestinationId = currentPortalId.substr(prefix.length());
        const bool currentIsStage =
            TryParseStageDestinationId(currentDestinationId, &currentWorldNumber, &currentStageNumber);

        int candidateWorldNumber = 0;
        int candidateStageNumber = 0;
        const std::wstring candidateDestinationId = candidatePortalId.substr(prefix.length());
        const bool candidateIsStage =
            TryParseStageDestinationId(candidateDestinationId, &candidateWorldNumber, &candidateStageNumber);

        if (currentIsStage && candidateIsStage)
        {
            if (currentWorldNumber != candidateWorldNumber)
            {
                return false;
            }

            return std::abs(currentStageNumber - candidateStageNumber) == 1;
        }

        int currentSelectWorldNumber = 0;
        const bool currentIsStageSelect =
            TryParseStageSelectDestinationId(currentDestinationId, &currentSelectWorldNumber);
        int candidateSelectWorldNumber = 0;
        const bool candidateIsStageSelect =
            TryParseStageSelectDestinationId(candidateDestinationId, &candidateSelectWorldNumber);

        if (currentIsStage && candidateIsStageSelect)
        {
            return IsStageEndpointConnectedToSelect(currentWorldNumber,
                                                     currentStageNumber,
                                                     candidateSelectWorldNumber);
        }
        if (currentIsStageSelect && candidateIsStage)
        {
            return IsStageEndpointConnectedToSelect(candidateWorldNumber,
                                                     candidateStageNumber,
                                                     currentSelectWorldNumber);
        }
        if (currentIsStageSelect || candidateIsStageSelect)
        {
            return false;
        }

        return true;
    }

    bool IsBombAttackType(const PlayerAttackType attackType)
    {
        if (attackType == PlayerAttackType::BombAttack)
        {
            return true;
        }

        return attackType == PlayerAttackType::BombStrongAttack;
    }

    bool IsBusterAttackType(const PlayerAttackType attackType)
    {
        if (attackType == PlayerAttackType::BusterAttack)
        {
            return true;
        }

        return attackType == PlayerAttackType::BusterStrongAttack;
    }

    bool IsWeakMeleeAttackType(const PlayerAttackType attackType)
    {
        if (attackType == PlayerAttackType::WeakAttack)
        {
            return true;
        }

        return attackType == PlayerAttackType::SwordAttack;
    }

    bool IsStrongMeleeAttackType(const PlayerAttackType attackType)
    {
        if (attackType == PlayerAttackType::StrongAttack)
        {
            return true;
        }

        return attackType == PlayerAttackType::SwordStrongAttack;
    }

    const std::wstring& GetAttackIconPath(const PlayerAttackType attackType)
    {
        if (attackType == PlayerAttackType::WeakAttack ||
            attackType == PlayerAttackType::StrongAttack)
        {
            return kAttackClubIconPath;
        }

        if (attackType == PlayerAttackType::SwordAttack ||
            attackType == PlayerAttackType::SwordStrongAttack)
        {
            return kAttackSlashIconPath;
        }

        if (IsBombAttackType(attackType))
        {
            return kAttackBombIconPath;
        }

        if (IsBusterAttackType(attackType))
        {
            return kAttackBusterIconPath;
        }

        return kAttackClubIconPath;
    }

    int GetBusterCooldownFrames(const int rapidLevel)
    {
        if (rapidLevel <= 1)
        {
            return kBusterCooldownByLevel[0];
        }

        if (rapidLevel >= kBusterRapidLevelMax)
        {
            return kBusterCooldownByLevel[kBusterRapidLevelMax - 1];
        }

        return kBusterCooldownByLevel[rapidLevel - 1];
    }

    void PlaceStageWeather(NSRender::Render& render, StageManager::StageWeather weather, const D3DXVECTOR3& origin)
    {
        if (weather == StageManager::StageWeather::Rain)
        {
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Rain, origin);
        }
    }
}

GameApp& GameApp::Instance()
{
    static GameApp s_instance;
    return s_instance;
}

GameApp::GameApp()
    : m_slideShowManager(m_render)
    , m_pendingMove(0.0f, 0.0f, 0.0f)
    , m_playerKnockbackDir(0.0f, 0.0f, 0.0f)
    , m_respawnCameraFromPos(0.0f, 0.0f, 0.0f)
    , m_respawnCameraFromTarget(0.0f, 0.0f, 0.0f)
    , m_respawnCameraToPos(0.0f, 0.0f, 0.0f)
    , m_respawnCameraToTarget(0.0f, 0.0f, 0.0f)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    m_remoteDesktopMode = (st.wDayOfWeek >= 1 && st.wDayOfWeek <= 5 && st.wHour >= 8 && st.wHour < 19);
}

GameApp::~GameApp()
{
}

class GameApp::CommandFont : public NSCommand::IFont
{
public:
    GameApp* app = nullptr;

    void DrawText_(const std::wstring& msg, const int x, const int y, const int transparent) override
    {
        if (app != nullptr && app->m_commandFontId >= 0)
        {
            app->m_render.DrawTextExCenter(app->m_commandFontId,
                                           msg,
                                           x,
                                           y,
                                           100,
                                           100,
                                           D3DCOLOR_RGBA(255, 255, 255, transparent));
        }
    }

    void Init(const bool bEnglish) override
    {
        (void)bEnglish;
        if (app != nullptr)
        {
            app->m_commandFontId = app->m_render.SetUpFontEx(L"BIZ UDGothic", 18, D3DCOLOR_ARGB(255, 255, 255, 255));
        }
    }

    void OnDeviceLost() override {}
    void OnDeviceReset() override {}
};

class GameApp::CommandSprite : public NSCommand::ISprite
{
public:
    GameApp* app = nullptr;

    void DrawImage(const int x, const int y, const int transparency) override
    {
        if (app != nullptr)
        {
            app->m_render.DrawImage(L"res\\2D_Image\\command_cursor.png", x, y, transparency);
        }
    }

    void Load(const std::wstring& filepath) override
    {
        (void)filepath;
    }

    void OnDeviceLost() override {}
    void OnDeviceReset() override {}
};

class GameApp::CommandSE : public NSCommand::ISoundEffect
{
public:
    GameApp* app = nullptr;

    void PlayMove() override
    {
        if (app != nullptr)
        {
            GameAudio::PlayMenuMove();
        }
    }

    void PlayClick() override { GameAudio::PlayMenuConfirm(); }
    void PlayBack() override { GameAudio::PlayMenuCancel(); }

    void Init() override {}
};

class GameApp::QteSprite : public NS_QTE_Module::ISprite
{
public:
    GameApp* app = nullptr;
    std::wstring m_filepath;

    void DrawImage(const int x, const int y, const int transparency) override
    {
        if (app != nullptr && !m_filepath.empty())
        {
            app->m_render.DrawImage(m_filepath, x, y, transparency);
        }
    }

    void DrawImageRect(const int x, const int y, const int srcWidth, const int srcHeight, const int transparency) override
    {
        if (app != nullptr && !m_filepath.empty())
        {
            app->m_render.DrawImageSizedRect(m_filepath, x, y, srcWidth, srcHeight, 0, 0, srcWidth, srcHeight, transparency);
        }
    }

    void DrawImageScaled(const int x, const int y, const int width, const int height, const int transparency) override
    {
        if (app != nullptr && !m_filepath.empty())
        {
            app->m_render.DrawImageSized(m_filepath, x, y, width, height, transparency);
        }
    }

    void Load(const std::wstring& filepath) override
    {
        m_filepath = filepath;
    }

    ISprite* Create() override
    {
        QteSprite* instance = new QteSprite();
        return instance;
    }

    ~QteSprite() {}

    void OnDeviceLost() override {}
    void OnDeviceReset() override {}
};

bool GameApp::Initialize(HINSTANCE hInstance, int nCmdShow)
{
    m_hInstance = hInstance;

    WNDCLASSEX wc { };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = MsgProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_hInstance;
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = _T("Window1");
    wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));

    ATOM atom = RegisterClassEx(&wc);
    assert(atom != 0);

    int screenWidth = kNormalScreenWidth;
    int screenHeight = kNormalScreenHeight;
    if (m_remoteDesktopMode)
    {
        screenWidth = kRemoteDesktopScreenWidth;
        screenHeight = kRemoteDesktopScreenHeight;
    }

    RECT rect;
    SetRect(&rect, 0, 0, screenWidth, screenHeight);
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    rect.right = rect.right - rect.left;
    rect.bottom = rect.bottom - rect.top;
    rect.top = 0;
    rect.left = 0;

    m_hWnd = CreateWindow(_T("Window1"),
                          _T("ホシガール"),
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          rect.right,
                          rect.bottom,
                          NULL,
                          NULL,
                          wc.hInstance,
                          NULL);

    m_render.Initialize(m_hWnd, L"res\\RenderSettings.csv");
    m_render.ChangeResolution(screenWidth, screenHeight);
    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);
    m_render.SetLoadingScreenTitleFontPath(L"res\\font\\BIZUDMincho-Regular.ttf");
    m_render.StartLoadingScreen();
    m_render.SetLoadingScreenProgress(0);
    m_render.Draw();
    SoundLib::SoundLib::Initialize(m_hWnd);
    SoundLib::SoundLib::LoadSoundEffect(g_arrowSoundPath);
    GameAudio::Initialize();
    GameAudio::PlayLoadingEnvironment();

    m_render.SetShowFPS(false);
    m_render.SetLightDir(D3DXVECTOR3(0.6f, 0.7f, -0.9f));
    m_stageManager.Initialize();
    m_stageManager.MoveToStage(m_stageManager.FindStageIndexById(L"select1"));
    const StageManager::StageData& initialStage = m_stageManager.GetCurrentStage();
    m_render.LoadXFileListFromCsv(initialStage.renderCsvPath);
    m_render.SetLoadingScreenProgress(15);
    m_render.Draw();
    m_render.LoadXFileListMoveFromCsv(initialStage.moveCsvPath);
    m_render.SetLoadingScreenProgress(25);
    m_render.Draw();
    m_playerMeshId = m_render.AddMeshMixSkinAnim2(g_playerMeshPath,
                                                  g_playerAnimCsvPath,
                                                  initialStage.playerStartPosition,
                                                  D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                  1.0f,
                                                  NSRender::AnimSetMap(),
                                                  -1.0f,
                                                  false,
                                                  false);
    m_render.SetLoadingScreenProgress(40);
    m_render.Draw();

    InitializePlayerPhysics();
    m_render.SetLoadingScreenProgress(55);
    m_render.Draw();
    PhysicsLib::SettingsState::SetCameraAutoMoveEnabled(true);
    PhysicsLib::SettingsState::SetFocusModeEnabled(false);
    PhysicsLib::SettingsState::SetInfiniteJumpEnabled(false);
    m_useFixedCamera = initialStage.useFixedCamera;
    m_fixedCameraPos = initialStage.fixedCameraPos;
    m_fixedCameraLookAt = initialStage.fixedCameraLookAt;
    InitializeCameraFromRenderSettings();
    UpdatePlayerMeshAndCamera(initialStage.playerStartPosition);
    UpdatePlayerMeshVisibility();
    m_enemyManager.Initialize();
    m_enemyManager.LoadForStage(m_render, initialStage.enemyCsvPath);

    m_destructibleManager.Initialize(m_render);
    m_destructibleManager.SetStarDropCallback([this]() {
        m_pickupManager.ActivateStar(m_playerMeshId);
    });
    m_destructibleManager.SetSpeedUpCallback([this]() {
        if (m_pickupManager.AddSpeedLevel())
        {
            GameAudio::PlayPowerUp();
        }
    });
    m_destructibleManager.LoadForStage(m_render, initialStage.destructibleCsvPath);

    if (!IsStageSelectId(initialStage.id))
    {
        const D3DXVECTOR3 goalPos(initialStage.clearPosition.x,
                                  initialStage.clearPosition.y - 0.5f,
                                  initialStage.clearPosition.z);
        m_goalMarkerMeshId = m_render.AddMeshMix(L"res\\model\\cube_red.x",
                                                 goalPos,
                                                 D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                 1.0f,
                                                 -1.0f,
                                                 false,
                                                 false,
                                                 false);
    }

    InputDevice::Initialize(m_hInstance, m_hWnd);
    InputDevice::SetRemoteDesktopMode(m_remoteDesktopMode);
    m_inventoryManager.Initialize();
    m_inventoryManager.Load();
    if (m_inventoryManager.GetWeaponCount(kInitialClubWeaponId) <= 0)
    {
        m_inventoryManager.AddWeapon(kInitialClubWeaponId, 1);
        m_inventoryManager.Save();
    }
    ApplyUnlockedAbilities();
    LoadItemNameCatalog();
    m_collectibleManager.Initialize(m_render, m_inventoryManager);
    m_collectibleManager.SetItemCollectedCallback([this](const std::wstring& itemId, const int count) {
        HandleItemCollected(itemId, count);
    });
    m_collectibleManager.LoadForStage(initialStage.collectibleCsvPath);
    m_collectibleManager.RefreshVisibility(m_destructibleManager);
    m_interactionManager.Initialize(m_render);
    m_interactionManager.LoadForStage(initialStage.interactableCsvPath);
    m_lavaZoneManager.LoadForStage(initialStage.lavaCsvPath);
    m_pauseMenu.Initialize(m_render, m_mouseCursorVisible, m_inventoryManager);
    m_pauseMenu.SetItemUseCallback([this](const std::wstring& itemId) {
        return HandleInventoryItemUse(itemId);
    });
    m_craftMenu.Initialize(m_render, m_mouseCursorVisible, m_inventoryManager);
    InputDevice::Mouse::SetVisible(m_mouseCursorVisible);
    m_render.SetLoadingScreenProgress(70);
    m_render.Draw();
    m_pickupManager.Initialize(m_render, m_inventoryManager);
    m_pickupManager.SetItemCollectedCallback([this](const std::wstring& itemId, const int count) {
        HandleItemCollected(itemId, count);
    });
    m_pickupManager.SetAmmoRecoveredCallback([this]() {
        RecoverWeaponAmmoFromPickup();
    });
    m_pickupManager.SetStarActivatedCallback([this]() {
        MaximizeTemporaryPowerUps();
    });
    m_pickupManager.LoadForStage(initialStage.starCsvPath, initialStage.speedUpCsvPath);
    m_dashBoosterManager.Initialize(m_render);
    m_dashBoosterManager.LoadForStage(initialStage.dashBoosterCsvPath);

    CommandFont* pFont = new CommandFont();
    pFont->app = this;
    CommandSE* pSE = new CommandSE();
    pSE->app = this;
    CommandSprite* pSpr = new CommandSprite();
    pSpr->app = this;
    m_command.Init(pFont, pSE, pSpr, false, L"res\\commandName_title.csv");

    m_hpBar.Initialize(&m_render, &m_player);
    m_damagePopupManager.Initialize(&m_render);
    m_damagePopupManager.SetEnabled(false);

    m_saveDataManager.Initialize(m_stageManager);
    m_saveDataManager.ResetToDefaults();
    InitializeStageSelectCursor();
    CreateStageSelectCubes();
    UpdatePlayerMeshAndCamera(m_playerMover.GetPosition());
    m_mouseCursorVisible = true;
    InputDevice::Mouse::SetVisible(m_mouseCursorVisible);

    m_command.UpsertCommand(L"start", true);
    m_command.UpsertCommand(L"continue", m_saveDataManager.HasSaveFile());
    m_command.UpsertCommand(L"delete", m_saveDataManager.HasSaveFile());
    m_command.UpsertCommand(L"language", true);
    m_command.UpsertCommand(L"exit", true);
    m_render.SetLoadingScreenProgress(85);
    m_render.Draw();

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
    m_debugFpsSampleTick = GetTickCount64();
    m_debugRpc.Initialize();
#endif

    return true;
}

void GameApp::Run()
{
    MSG msg;

    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            const bool isEscKey = (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE);
            if (m_settingsDialog == NULL || !IsWindowVisible(m_settingsDialog) ||
                isEscKey ||
                !IsDialogMessage(m_settingsDialog, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (m_close)
        {
            break;
        }

        InputDevice::Update();

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
        ProcessDebugRpc();
#endif

        const D3DXVECTOR3 audioPlayerPosition = m_playerMover.GetPosition();
        const D3DXVECTOR3 audioListenerForward = GetCameraPlanarForward();
        SoundLib::Vector3 listenerPosition { audioPlayerPosition.x, audioPlayerPosition.y, audioPlayerPosition.z };
        SoundLib::Vector3 listenerFront { audioListenerForward.x, audioListenerForward.y, audioListenerForward.z };
        SoundLib::Vector3 listenerTop { 0.0f, 1.0f, 0.0f };
        SoundLib::SoundLib::Update(listenerPosition, listenerFront, listenerTop);

        if (m_gameState == GameState::Title)
        {
            GameAudio::PlayTitleMusic();
        }
        else if (m_gameState == GameState::Playing)
        {
            const StageManager::StageData& audioStage = m_stageManager.GetCurrentStage();
            const bool useRainEnvironment = audioStage.weather == StageManager::StageWeather::Rain;
            GameAudio::UpdateStageMusic(audioStage.id, audioStage.number, useRainEnvironment, GetCurrentWorld());
        }
        else if (m_gameState == GameState::Ending || m_gameState == GameState::EndingFin)
        {
            GameAudio::PlayEndingMusic();
        }

        if (m_stageTransitionAction != StageTransitionAction::None)
        {
            UpdateStageTransition();
            continue;
        }

        if (m_gameState == GameState::Playing &&
            !m_pauseMenu.IsOpen() &&
            !m_craftMenu.IsOpen() &&
            !m_playerDeathPending &&
            !IsHitStopActive() &&
            (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE) ||
             InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_START)))
        {
            bool returnToStageSelectEnabled = false;
            if (!IsCurrentStageSelect())
            {
                returnToStageSelectEnabled = m_saveDataManager.IsStageCleared(m_stageManager.GetCurrentStage().id);
            }
            m_pauseMenu.Open(IsCurrentStageSelect(), returnToStageSelectEnabled);
        }

        if (m_gameState != GameState::EndingFin &&
            !IsCurrentStageSelect() &&
            !m_pauseMenu.IsOpen() &&
            !m_craftMenu.IsOpen() &&
            !m_playerDeathPending &&
            !IsHitStopActive() &&
            (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LCONTROL) ||
             InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RCONTROL)))
        {
            m_mouseCursorVisible = !m_mouseCursorVisible;
            InputDevice::Mouse::SetVisible(m_mouseCursorVisible);
        }

        if (m_gameState == GameState::Playing &&
            !IsCurrentStageSelect() &&
            !m_pauseMenu.IsOpen() &&
            !m_craftMenu.IsOpen() &&
            !m_playerDeathPending &&
            !IsHitStopActive() &&
            InputDevice::SKeyBoard::IsDownFirstFrame(DIK_R))
        {
            TryUseRecoveryItemFromKey();
        }

        if (m_gameState == GameState::Loading)
        {
            GameAudio::PlayLoadingEnvironment();
            m_render.Draw();

            if (m_render.IsAllMeshLoaded())
            {
                m_render.EndLoadingScreen();
                m_gameState = GameState::Title;
            }
        }
        else if (m_gameState == GameState::Title)
        {
            if (!m_titleDeleteConfirmMode && !m_titleLanguageSelectionMode)
            {
                RefreshTitleCommands();
            }
            UpdateTitleByInput();
            DrawTitleScreen();

            if (m_titleDeleteConfirmMode)
            {
                if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
                {
                    const std::wstring selectedId = m_command.Into();
                    if (selectedId == L"yes")
                    {
                        ExecuteDeleteSaveData();
                    }
                    else if (selectedId == L"no")
                    {
                        ExitDeleteConfirmation();
                    }
                }

                if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
                {
                    ExitDeleteConfirmation();
                }
            }
            else if (m_titleLanguageSelectionMode)
            {
                if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
                {
                    ExecuteTitleCommand(m_command.Into());
                }

                if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
                {
                    ExitTitleLanguageSelection();
                }

                const InputDevice::MousePosition mousePos = InputDevice::Mouse::GetPosition();
                const POINT baseMousePos = ConvertMouseToBaseResolution(mousePos.x, mousePos.y);
                m_command.MouseMove(baseMousePos.x, baseMousePos.y);

                if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
                {
                    ExecuteTitleCommand(m_command.Click(baseMousePos.x, baseMousePos.y));
                }
            }
            else
            {
                if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
                {
                    ExecuteTitleCommand(m_command.Into());
                }

                const InputDevice::MousePosition mousePos = InputDevice::Mouse::GetPosition();
                const POINT baseMousePos = ConvertMouseToBaseResolution(mousePos.x, mousePos.y);
                m_command.MouseMove(baseMousePos.x, baseMousePos.y);

                if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
                {
                    ExecuteTitleCommand(m_command.Click(baseMousePos.x, baseMousePos.y));
                }
            }
        }
        else if (m_gameState == GameState::SlideShow)
        {
            if (!m_slideShowManager.IsActive())
            {
                if (m_pendingStageIndexAfterSlideShow != static_cast<std::size_t>(-1))
                {
                    const std::size_t stageIndex = m_pendingStageIndexAfterSlideShow;
                    m_pendingStageIndexAfterSlideShow = static_cast<std::size_t>(-1);
                    StartStageByIndexImmediate(stageIndex);
                }
                else if (m_startStageAfterSlideShow)
                {
                    m_startStageAfterSlideShow = false;
                    StartStageAfterClear();
                }
                else
                {
                    if (IsCurrentStageSelect())
                    {
                        m_gameState = GameState::Playing;
                    }
                    else
                    {
                        m_gameState = GameState::StageIntro;
                        BeginStageIntro();
                    }
                    m_prevMovingPlatformPositions.clear();
                }
                m_render.Draw();
            }
            else
            {
                m_slideShowManager.ProcessInput();
                if (m_slideShowManager.Update())
                {
                    if (m_pendingStageIndexAfterSlideShow != static_cast<std::size_t>(-1))
                    {
                        const std::size_t stageIndex = m_pendingStageIndexAfterSlideShow;
                        m_pendingStageIndexAfterSlideShow = static_cast<std::size_t>(-1);
                        StartStageByIndexImmediate(stageIndex);
                    }
                    else if (m_startStageAfterSlideShow)
                    {
                        m_startStageAfterSlideShow = false;
                        StartStageAfterClear();
                    }
                    else
                    {
                        if (IsCurrentStageSelect())
                        {
                            m_gameState = GameState::Playing;
                        }
                        else
                        {
                            m_gameState = GameState::StageIntro;
                            BeginStageIntro();
                        }
                        m_prevMovingPlatformPositions.clear();
                    }
                    m_render.Draw();
                }
                else
                {
                    m_render.Draw();
                    m_slideShowManager.Render();
                    m_slideShowManager.DrawSkipHint();
                }
            }
        }
        else if (m_gameState == GameState::StageClear)
        {
            UpdateStageClear();
        }
        else if (m_gameState == GameState::StageExit)
        {
            UpdateStageExit();
        }
        else if (m_gameState == GameState::GameOver)
        {
            UpdateGameOver();
        }
        else if (m_gameState == GameState::Ending)
        {
            if (m_slideShowManager.IsActive())
            {
                m_slideShowManager.ProcessInput();
                if (m_slideShowManager.Update())
                {
                    m_pauseMenu.Close();
                    m_gameState = GameState::EndingFin;
                    DrawEndingFin();
                }
                else
                {
                    m_render.Draw();
                    m_slideShowManager.Render();
                }
            }
            else
            {
                m_pauseMenu.Close();
                m_gameState = GameState::EndingFin;
                DrawEndingFin();
            }
        }
        else if (m_gameState == GameState::EndingFin)
        {
            DrawEndingFin();
        }
        else if (m_gameState == GameState::StageIntro)
        {
            UpdateStageIntro();
        }
        else
        {
            if (m_craftMenu.BlocksGameInput())
            {
                m_craftMenu.Update();
                ApplyUnlockedAbilities();
                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    DrawAmmoGauge();
                }
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render);
                m_craftMenu.Render();
                m_render.Draw();
                continue;
            }

            if (m_pauseMenu.BlocksGameInput())
            {
                m_pauseMenu.Update();
                if (m_pauseMenu.ConsumeExitRequested())
                {
                    m_close = true;
                    continue;
                }
                if (m_pauseMenu.ConsumeSaveRequested())
                {
                    if (IsCurrentStageSelect())
                    {
                        m_saveDataManager.Save();
                    }
                }
                if (m_pauseMenu.ConsumeReturnToStageSelectRequested())
                {
                    BeginStageExit();
                    continue;
                }
                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    DrawAmmoGauge();
                }
                m_damagePopupManager.Draw();
                m_pauseMenu.Render(m_stageManager.GetCurrentStageDisplayName(), m_player.GetLives());
                m_render.Draw();
                continue;
            }

            if (m_playerDeathPending)
            {
                if (m_respawnCameraDelayFrames > 0)
                {
                    --m_respawnCameraDelayFrames;
                }

                if (m_respawnCameraDelayFrames <= 0)
                {
                    CompletePlayerDeath();
                    continue;
                }

                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    DrawAmmoGauge();
                }
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render);
                m_render.Draw();
                continue;
            }

            if (IsHitStopActive())
            {
                m_destructibleManager.Update(m_render);
                m_enemyManager.SyncMeshes(m_render);
                UpdateGoalArrow();

                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    DrawAmmoGauge();
                    m_render.DrawImageSized(GetAttackIconPath(m_playerAttackController.GetAttackType(false)),
                                            kAttackTypeHudX,
                                            kAttackTypeHudY,
                                            kAttackTypeIconSize,
                                            kAttackTypeIconSize);
                }
                m_damagePopupManager.Update();
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render);
                DrawItemPickupMessage();
                m_render.Draw();
                UpdateHitStop();
                continue;
            }

            // QTE 中はプレイヤー/カメラ/敵/インタラクト入力を止める
            if (m_qte == nullptr)
            {
                // マウスカーソル表示中はUI操作を優先し、カメラ回転を止める。
                // 固定カメラ時もマウスによる回転を無効化する。
                if (!m_mouseCursorVisible && !m_useFixedCamera)
                {
                    UpdateCameraByInput();
                }

                // 入力処理 → メッシュ位置・カメラ設定（衝突判定前）
                UpdatePlayerByInput();

                // 敵の更新
                if (m_debugEnemyUpdateEnabled)
                {
                    m_enemyManager.Update(m_render, m_playerMover.GetPosition(), m_playerInvincibleFrames > 0);
                }

                m_destructibleManager.Update(m_render);

                const bool isStageSelect = IsCurrentStageSelect();
                if (isStageSelect)
                {
                    UpdateStageSelectCursorByInput();
                }
                else
                {
                    // インタラクト通知とQTE起動判定
                    m_interactionManager.Update(m_playerMover.GetPosition());
                    std::wstring interactionId;
                    if (m_interactionManager.ConsumeTriggeredInteraction(&interactionId) && !interactionId.empty())
                    {
                        if (interactionId == L"base-crafting-station-01")
                        {
                            m_craftMenu.SetCurrentWorld(GetCurrentWorld());
                            m_craftMenu.Open();
                        }
                        else
                        {
                            const std::wstring interactionType = m_interactionManager.GetInteractableType(interactionId);
                            if (interactionType == L"Tree")
                            {
                                m_interactionManager.RemoveInteractableById(interactionId);
                            }

                            m_qte = new NS_QTE_Module::QTE_Module();
                            QteSprite* sprGrowingCircle = new QteSprite();
                            sprGrowingCircle->app = this;
                            sprGrowingCircle->Load(L"res\\2D_Image\\qte_growing_circle.png");
                            QteSprite* sprTargetCircle = new QteSprite();
                            sprTargetCircle->app = this;
                            sprTargetCircle->Load(L"res\\2D_Image\\qte_target_circle.png");
                            QteSprite* sprButton = new QteSprite();
                            sprButton->app = this;
                            sprButton->Load(L"res\\2D_Image\\qte_button.png");
                            m_qte->SetCircleSprites(sprGrowingCircle, sprTargetCircle, sprButton, 1600, 900);

                            QteSprite* sprSuccessBurst = new QteSprite();
                            sprSuccessBurst->app = this;
                            sprSuccessBurst->Load(L"res\\2D_Image\\qte_best_burst.png");
                            QteSprite* sprSuccessWave = new QteSprite();
                            sprSuccessWave->app = this;
                            sprSuccessWave->Load(L"res\\2D_Image\\qte_best_wave.png");
                            QteSprite* sprSuccessSparkles = new QteSprite();
                            sprSuccessSparkles->app = this;
                            sprSuccessSparkles->Load(L"res\\2D_Image\\qte_best_sparkles.png");
                            m_qte->SetSuccessEffectSprites(sprSuccessBurst, sprSuccessWave, sprSuccessSparkles);

                            QteSprite* sprNormalWave = new QteSprite();
                            sprNormalWave->app = this;
                            sprNormalWave->Load(L"res\\2D_Image\\qte_normal_wave.png");
                            QteSprite* sprFailureImpact = new QteSprite();
                            sprFailureImpact->app = this;
                            sprFailureImpact->Load(L"res\\2D_Image\\qte_failure_impact.png");
                            m_qte->SetResultEffectSprites(sprNormalWave, sprFailureImpact);

                            GameAudio::PlayQteStart();
                            BeginQteVisualEffect();
                            m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                            m_pendingJump = false;
                        }
                    }
                }

                if (!isStageSelect && m_stagePortalCooldownFrames <= 0)
                {
                    const std::wstring portalId = m_interactionManager.GetNearestOfType(
                        m_playerMover.GetPosition(), L"StagePortal");
                    if (!portalId.empty())
                    {
                        const std::wstring prefix = L"portal-to-";
                        const std::size_t prefixLen = prefix.length();
                        if (portalId.length() > prefixLen && portalId.substr(0, prefixLen) == prefix)
                        {
                            const std::wstring destStageId = portalId.substr(prefixLen);
                            if (destStageId != L"base" && !m_saveDataManager.IsStageUnlocked(destStageId))
                            {
                                // 未解放ステージ：表示はするが移動しない
                            }
                            else
                            {
                                if (destStageId == L"base")
                                {
                                    const std::wstring& currentId = m_stageManager.GetCurrentStage().id;
                                    if (currentId.length() >= 6 && currentId.substr(0, 6) == L"select")
                                    {
                                        m_lastSelectId = currentId;
                }
            }

                                const std::size_t targetIndex = m_stageManager.FindStageIndexById(destStageId);
                                if (targetIndex < m_stageManager.GetStageCount())
                                {
                                    StartStageByIndex(targetIndex);
                                    m_stagePortalCooldownFrames = 60;
                                }
                            }
                        }
                    }

                    const std::wstring returnId = m_interactionManager.GetNearestOfType(
                        m_playerMover.GetPosition(), L"ReturnPortal");
                    if (!returnId.empty())
                    {
                        const std::wstring destId = m_lastSelectId.empty() ? L"select1" : m_lastSelectId;
                        const std::size_t targetIndex = m_stageManager.FindStageIndexById(destId);
                        if (targetIndex < m_stageManager.GetStageCount())
                        {
                            StartStageByIndex(targetIndex);
                            m_stagePortalCooldownFrames = 60;
                        }
                    }
                }
            }
            else
            {
                // QTE 停止入力
                if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE) ||
                    InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_A))
                {
                    if (m_qte->GetBarResult() == NS_QTE_Module::QTE_Module::BarResult::None)
                    {
                        m_qte->StopBarAnimation();
                        GameAudio::PlayQteStop();
                    }
                }

                // QTE 完了判定
                if (m_qte->Update())
                {
                    const NS_QTE_Module::QTE_Module::BarResult result = m_qte->GetBarResult();
                    if (result == NS_QTE_Module::QTE_Module::BarResult::Success ||
                        result == NS_QTE_Module::QTE_Module::BarResult::Normal)
                    {
                        if (result == NS_QTE_Module::QTE_Module::BarResult::Success)
                        {
                            GameAudio::PlayQteSuccess();
                        }
                        else
                        {
                            GameAudio::PlayQteNormal();
                        }
                        m_inventoryManager.AddItem(kQteRewardItemId, 1);
                        m_inventoryManager.Save();
                        ShowItemPickupMessage(kQteRewardItemId, 1);
                    }
                    else
                    {
                        GameAudio::PlayQteFailure();
                    }
                    m_qte->Finalize();
                    delete m_qte;
                    m_qte = nullptr;
                    EndQteVisualEffect();
                }
            }

            UpdateQteVisualEffect();

            m_enemyManager.SyncMeshes(m_render);
            UpdateGoalArrow();

            // 描画（動く床の位置が更新される）
            if (!IsCurrentStageSelect())
            {
                m_hpBar.Draw();
                DrawAmmoGauge();
                m_render.DrawImageSized(GetAttackIconPath(m_playerAttackController.GetAttackType(false)),
                                        kAttackTypeHudX,
                                        kAttackTypeHudY,
                                        kAttackTypeIconSize,
                                        kAttackTypeIconSize);
            }
            if (!IsHitStopActive())
            {
                m_damagePopupManager.Update();
            }
            m_damagePopupManager.Draw();
            m_enemyManager.DrawHpBars(m_render);
            if (m_qte == nullptr && !IsCurrentStageSelect())
            {
                m_interactionManager.DrawPrompt();
            }
            if (m_qte != nullptr)
            {
                m_qte->Render();
            }
            DrawStageSelectCursor();
            DrawItemPickupMessage();
            m_render.Draw();

            if (m_pendingHitStopFrames > 0)
            {
                StartHitStopNow(m_pendingHitStopFrames);
                m_pendingHitStopFrames = 0;
                continue;
            }

            // 動く床の位置を描画エンジンから取得し、物理エンジンに反映する。
            {
                const D3DXVECTOR3 kPlatformRot(0.0f, 0.0f, 0.0f);
                const D3DXVECTOR3 kPlatformScale(1.0f, 1.0f, 1.0f);

                const auto& platforms = m_render.GetMovingPlatforms();
                for (const auto& platform : platforms)
                {
                    const D3DXVECTOR3 platformPos = m_render.GetMeshMixPos(platform.renderId);
                    D3DXVECTOR3& prevPos = m_prevMovingPlatformPositions[platform.csvId];
                    const D3DXVECTOR3 platformVelocity = (platformPos - prevPos) / kTargetFrameSeconds;
                    prevPos = platformPos;

                    PhysicsWorld::UpdateCsvTransform(platform.csvId, platformPos, kPlatformRot, kPlatformScale);
                    const int physicsId = PhysicsWorld::GetCsvObjectId(platform.csvId);
                    if (physicsId >= 0)
                    {
                        PhysicsWorld::SetVelocity(physicsId, platformVelocity);
                    }
                }
            }

            // 衝突判定（動く床の最新位置を反映）
            const bool isStageSelect = IsCurrentStageSelect();
            const D3DXVECTOR3 playerPositionBeforePhysicsUpdate = m_playerMover.GetPosition();
            if (!isStageSelect)
            {
                // 落下死演出中は入力を無効化し自由落下させる
                if (m_playerFallingDead)
                {
                    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                    m_pendingJump = false;
                }
                if (m_debugPlayerPhysicsEnabled)
                {
                    m_playerMover.Update(m_pendingMove, m_pendingJump);
                }
                UpdateDashParticleEffect();
                m_dashBoosterManager.Update(m_playerMover.GetPosition(), m_playerMover);
                m_collectibleManager.Update(m_playerMover.GetPosition(), m_destructibleManager);
                if (m_playerMover.IsCrushed())
                {
                    DamagePlayerHp(m_player.GetHp());
                }

                // 落下死判定: Y座標が閾値以下で落下死。
                // カメラ追従を止め、プレイヤーはそのまま落下させ、指定フレーム後にリスポーンへ接続する。
                if (!m_playerFallingDead)
                {
                    if (m_playerMover.GetPosition().y <= kFallDeathY)
                    {
                        m_playerFallingDead = true;
                        m_fallDeathFrames = 0;
                    }
                }
                else
                {
                    ++m_fallDeathFrames;
                    if (m_fallDeathFrames >= kFallDeathFrames)
                    {
                        HandlePlayerDeath();
                        m_respawnCameraDelayFrames = 0;
                    }
                }

                if (m_qte == nullptr && m_playerAttackController.ConsumeHitRequested())
                {
                    const PlayerAttackDefinition& attackDefinition = m_playerAttackController.GetCurrentDefinition();
                    if (attackDefinition.range > 0.0f)
                    {
                        const int damagedEnemyCount = DamageEnemiesInAttackRange(attackDefinition);
                        if (damagedEnemyCount > 0)
                        {
                            GameAudio::PlaySlashHit();
                            BeginHitStop(GetHitStopFrames(m_playerAttackController.GetCurrentAttackType()));
                        }
                        else
                        {
                            const DestructibleObject* destructible = m_destructibleManager.FindInAttackRange(
                                m_playerMover.GetPosition(), m_playerYaw,
                                attackDefinition.range, attackDefinition.verticalRange,
                                attackDefinition.halfAngleRadians);
                            if (destructible != nullptr)
                            {
                                if (m_destructibleManager.TryDamage(m_render, *destructible, attackDefinition.damage))
                                {
                                    m_damagePopupManager.Add(attackDefinition.damage, destructible->position, false);
                                    GameAudio::PlaySlashHit();
                                    BeginHitStop(GetHitStopFrames(m_playerAttackController.GetCurrentAttackType()));
                                }
                            }
                        }
                    }
                }
            }

            // プレイヤー無敵時間とスター時間を更新
            const bool wasStarActive = m_pickupManager.IsStarActive();
            m_pickupManager.UpdateTimers();
            if (wasStarActive && !m_pickupManager.IsStarActive())
            {
                RestoreTemporaryPowerUps();
            }
            if (!m_pickupManager.IsStarActive() && m_playerInvincibleFrames > 0)
            {
                --m_playerInvincibleFrames;
            }

            // 溶岩床によるダメージ（無敵モード中は歩ける）
            if (m_playerInvincibleFrames <= 0)
            {
                const int lavaDamage = m_lavaZoneManager.GetContactDamage(m_playerMover.GetPosition());
                if (lavaDamage > 0)
                {
                    DamagePlayerHp(lavaDamage);
                    m_playerInvincibleFrames = kPlayerInvincibleDuration;
                    if (m_playerMeshId >= 0)
                    {
                        m_render.StartMeshMixSkinAnimBlink(m_playerMeshId, kPlayerInvincibleDuration, 2);
                    }
                }
            }

            if (m_stagePortalCooldownFrames > 0)
            {
                --m_stagePortalCooldownFrames;
            }

            if (m_itemUseCooldownFrames > 0)
            {
                --m_itemUseCooldownFrames;
            }

            // 敵との接触・踏みつけ判定（QTE 中は無効）
            if (m_qte == nullptr)
            {
                for (auto& enemy : m_enemyManager.GetEnemies())
                {
                    if (enemy->IsDead())
                    {
                        continue;
                    }

                    if (enemy->IsStompedByPlayer(playerPositionBeforePhysicsUpdate,
                                                m_playerMover.GetPosition(),
                                                m_playerMover.IsJumping(),
                                                m_playerMover.GetVelocity().y))
                    {
                        enemy->TakeDamage(m_render, 10, m_playerMover.GetPosition());
                        m_damagePopupManager.Add(10, enemy->GetPosition(), false);
                        TryDropEnemyItem(*enemy);
                        GameAudio::PlayStomp();
                        const float jumpVelocity = m_playerMover.GetSettings().jumpVelocity;
                        m_playerMover.ApplyUpwardVelocity(jumpVelocity);
                        break;
                    }
                    else if (m_pickupManager.IsStarActive() && enemy->IsTouchingPlayer(m_playerMover.GetPosition()))
                    {
                        enemy->TakeDamage(m_render, 10, m_playerMover.GetPosition());
                        m_damagePopupManager.Add(10, enemy->GetPosition(), false);
                        TryDropEnemyItem(*enemy);
                        GameAudio::PlayAttackHit();
                        break;
                    }
                    else if (m_playerInvincibleFrames <= 0 && enemy->IsTouchingPlayer(m_playerMover.GetPosition()))
                    {
                        GameAudio::PlayEnemyAttack();
                        DamagePlayerHp(10);
                        m_playerInvincibleFrames = kPlayerInvincibleDuration;
                        if (m_playerMeshId >= 0)
                        {
                            m_render.StartMeshMixSkinAnimBlink(m_playerMeshId, kPlayerInvincibleDuration, 2);
                        }
                        m_playerKnockbackFrames = kKnockbackDurationFrames;
                        D3DXVECTOR3 knockbackDir = m_playerMover.GetPosition() - enemy->GetPosition();
                        knockbackDir.y = 0.0f;
                        if (D3DXVec3LengthSq(&knockbackDir) > 0.0001f)
                        {
                            D3DXVec3Normalize(&knockbackDir, &knockbackDir);
                        }
                        else
                        {
                            knockbackDir = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
                        }
                        m_playerKnockbackDir = knockbackDir;
                        enemy->MarkAttackedPlayer();
                        break;
                    }
                }
            }

            UpdateBombs();
            UpdateBusters();

            if (m_busterCooldownFrames > 0)
            {
                --m_busterCooldownFrames;
            }

            m_pickupManager.UpdatePickups(m_playerMover.GetPosition(),
                                          m_playerMeshId,
                                          m_destructibleManager);

            if (m_player.IsHpZero())
            {
                HandlePlayerDeath();
                if (m_playerDeathPending)
                {
                    continue;
                }
            }

            if (IsStageClearReached())
            {
                ClearBusters();
                m_gameState = GameState::StageClear;
                m_stageClearProcessed = false;
                m_stageClearFrame = 0;
            }

            if (m_playerMover.JustJumped())
            {
                GameAudio::PlayJump();
                if (m_playerMeshId >= 0)
                {
                    m_playerAnimationSpeed = 0.1f;
                    m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 0.1f);
                    m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerRunAnimName);
                }
            }

            if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F2))
            {
                SoundLib::SoundLib::PlaySoundEffect(g_arrowSoundPath, 100);
            }
        }

        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F1))
        {
            if (m_settingsDialog == NULL)
            {
                m_settingsDialog = CreateDialog(m_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), m_hWnd, SettingsDialogProc);
            }

            if (m_settingsDialog != NULL)
            {
                const bool isVisible = IsWindowVisible(m_settingsDialog);
                if (!isVisible)
                {
                    PopulateStageCombo(m_settingsDialog);
                    PopulateSpeedLevelCombo(m_settingsDialog);
                }
                ShowWindow(m_settingsDialog, isVisible ? SW_HIDE : SW_SHOW);
                if (!isVisible)
                {
                    m_mouseCursorVisible = true;
                    InputDevice::Mouse::SetVisible(true);
                }
            }
        }

        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F8))
        {
            m_render.ShowSettingsDialog();
            m_mouseCursorVisible = true;
            InputDevice::Mouse::SetVisible(true);
        }

        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F9))
        {
            PhysicsWorld::ShowSettingsDialog(m_hWnd);
            m_mouseCursorVisible = true;
            InputDevice::Mouse::SetVisible(true);
        }

        if (m_close)
        {
            break;
        }

    }
}

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
void GameApp::ProcessDebugRpc()
{
    ++m_debugFrameNumber;

    if (m_debugInvincible)
    {
        m_playerInvincibleFrames = 120;
    }

    if (m_debugProfileStartTick != 0)
    {
        const NSRender::RenderFrameProfile& renderProfile = m_render.GetLastFrameProfile();
        ++m_debugProfileRenderSamples;
        m_debugProfileSceneUpdateMilliseconds += renderProfile.sceneUpdateMilliseconds;
        m_debugProfileGBufferMilliseconds += renderProfile.gBufferMilliseconds;
        m_debugProfileMirrorMilliseconds += renderProfile.mirrorMilliseconds;
        m_debugProfileMainPassMilliseconds += renderProfile.mainPassMilliseconds;
        m_debugProfilePostEffectMilliseconds += renderProfile.postEffectMilliseconds;
        m_debugProfileDraw2DMilliseconds += renderProfile.draw2DMilliseconds;
        m_debugProfileFrameWaitMilliseconds += renderProfile.frameWaitMilliseconds;
        m_debugProfilePresentMilliseconds += renderProfile.presentMilliseconds;
        m_debugProfileRenderTotalMilliseconds += renderProfile.totalMilliseconds;
    }

    const ULONGLONG currentTick = GetTickCount64();
    const ULONGLONG elapsedMilliseconds = currentTick - m_debugFpsSampleTick;
    if (elapsedMilliseconds >= 500)
    {
        const ULONGLONG elapsedFrames = m_debugFrameNumber - m_debugFpsSampleFrame;
        m_debugFps = static_cast<float>(elapsedFrames) * 1000.0f /
                     static_cast<float>(elapsedMilliseconds);
        m_debugFpsSampleTick = currentTick;
        m_debugFpsSampleFrame = m_debugFrameNumber;
    }

    m_debugRpc.Poll([this](const std::string& command) {
        return HandleDebugRpcCommand(command);
    });
}

std::string GameApp::HandleDebugRpcCommand(const std::string& command)
{
    std::istringstream commandStream(command);
    std::string commandName;
    commandStream >> commandName;
    std::transform(commandName.begin(), commandName.end(), commandName.begin(), [](const unsigned char value) {
        return static_cast<char>(toupper(value));
    });

    if (commandName == "PING")
    {
        return "{\"ok\":true,\"result\":\"pong\"}";
    }

    if (commandName == "GET_FPS")
    {
        std::ostringstream response;
        response << std::fixed << std::setprecision(2)
                 << "{\"ok\":true,\"fps\":" << m_debugFps
                 << ",\"frame\":" << m_debugFrameNumber << "}";
        return response.str();
    }

    if (commandName == "PROFILE_RESET")
    {
        m_debugProfileStartTick = GetTickCount64();
        m_debugProfileStartFrame = m_debugFrameNumber;
        m_debugProfileRenderSamples = 0;
        m_debugProfileSceneUpdateMilliseconds = 0.0;
        m_debugProfileGBufferMilliseconds = 0.0;
        m_debugProfileMirrorMilliseconds = 0.0;
        m_debugProfileMainPassMilliseconds = 0.0;
        m_debugProfilePostEffectMilliseconds = 0.0;
        m_debugProfileDraw2DMilliseconds = 0.0;
        m_debugProfileFrameWaitMilliseconds = 0.0;
        m_debugProfilePresentMilliseconds = 0.0;
        m_debugProfileRenderTotalMilliseconds = 0.0;
        return "{\"ok\":true}";
    }

    if (commandName == "PROFILE_RESULT")
    {
        if (m_debugProfileStartTick == 0)
        {
            return "{\"ok\":false,\"error\":\"profile_not_started\"}";
        }
        const ULONGLONG elapsedMilliseconds = GetTickCount64() - m_debugProfileStartTick;
        if (elapsedMilliseconds == 0)
        {
            return "{\"ok\":false,\"error\":\"profile_has_no_elapsed_time\"}";
        }
        const ULONGLONG elapsedFrames = m_debugFrameNumber - m_debugProfileStartFrame;
        const float averageFps = static_cast<float>(elapsedFrames) * 1000.0f /
                                 static_cast<float>(elapsedMilliseconds);
        double renderSampleDivisor = 1.0;
        if (m_debugProfileRenderSamples > 0)
        {
            renderSampleDivisor = static_cast<double>(m_debugProfileRenderSamples);
        }
        std::ostringstream response;
        response << std::fixed << std::setprecision(2)
                 << "{\"ok\":true,\"averageFps\":" << averageFps
                 << ",\"elapsedMilliseconds\":" << elapsedMilliseconds
                 << ",\"frames\":" << elapsedFrames
                 << ",\"renderSamples\":" << m_debugProfileRenderSamples
                 << ",\"sceneUpdateMs\":" << m_debugProfileSceneUpdateMilliseconds / renderSampleDivisor
                 << ",\"gBufferMs\":" << m_debugProfileGBufferMilliseconds / renderSampleDivisor
                 << ",\"mirrorMs\":" << m_debugProfileMirrorMilliseconds / renderSampleDivisor
                 << ",\"mainPassMs\":" << m_debugProfileMainPassMilliseconds / renderSampleDivisor
                 << ",\"postEffectMs\":" << m_debugProfilePostEffectMilliseconds / renderSampleDivisor
                 << ",\"draw2DMs\":" << m_debugProfileDraw2DMilliseconds / renderSampleDivisor
                 << ",\"frameWaitMs\":" << m_debugProfileFrameWaitMilliseconds / renderSampleDivisor
                 << ",\"presentMs\":" << m_debugProfilePresentMilliseconds / renderSampleDivisor
                 << ",\"renderTotalMs\":" << m_debugProfileRenderTotalMilliseconds / renderSampleDivisor
                 << "}";
        return response.str();
    }

    if (commandName == "SET_PLAYER_RENDER" ||
        commandName == "SET_PLAYER_PHYSICS" ||
        commandName == "SET_ENEMY_UPDATE" ||
        commandName == "SET_SKIN_ANIMATION" ||
        commandName == "SET_INVINCIBLE")
    {
        std::string value;
        commandStream >> value;
        bool enabled = false;
        if (!TryParseDebugBoolean(value, &enabled))
        {
            return "{\"ok\":false,\"error\":\"invalid_boolean\"}";
        }

        if (commandName == "SET_PLAYER_RENDER")
        {
            m_debugPlayerRenderEnabled = enabled;
        }
        else if (commandName == "SET_PLAYER_PHYSICS")
        {
            m_debugPlayerPhysicsEnabled = enabled;
        }
        else if (commandName == "SET_ENEMY_UPDATE")
        {
            m_debugEnemyUpdateEnabled = enabled;
        }
        else if (commandName == "SET_SKIN_ANIMATION")
        {
            m_render.SetSkinAnimationUpdateEnabled(enabled);
        }
        else if (commandName == "SET_INVINCIBLE")
        {
            m_debugInvincible = enabled;
        }
        return "{\"ok\":true}";
    }

    if (commandName == "SET_RESOLUTION")
    {
        int width = 0;
        int height = 0;
        commandStream >> width >> height;
        if (width <= 0 || height <= 0)
        {
            return "{\"ok\":false,\"error\":\"invalid_resolution\"}";
        }
        m_render.ChangeResolution(width, height);
        return "{\"ok\":true}";
    }

    if (commandName == "GET_STATE")
    {
        const StageManager::StageData& stage = m_stageManager.GetCurrentStage();
        const std::string stageId = NarrowDebugIdentifier(stage.id);
        const std::string selectedPortalId = NarrowDebugIdentifier(m_selectedStagePortalId);
        const D3DXVECTOR3 playerPosition = m_playerMover.GetPosition();
        const EnemyBase* nearestEnemy = nullptr;
        float nearestEnemyDistance = 0.0f;
        std::size_t livingEnemyCount = 0;
        for (const std::unique_ptr<EnemyBase>& enemy : m_enemyManager.GetEnemies())
        {
            if (enemy == nullptr || enemy->GetHp() <= 0)
            {
                continue;
            }

            ++livingEnemyCount;
            const D3DXVECTOR3 difference = enemy->GetPosition() - playerPosition;
            const float distance = sqrtf(difference.x * difference.x + difference.z * difference.z);
            if (nearestEnemy == nullptr || distance < nearestEnemyDistance)
            {
                nearestEnemy = enemy.get();
                nearestEnemyDistance = distance;
            }
        }

        std::ostringstream response;
        response << std::fixed << std::setprecision(2)
                 << "{\"ok\":true"
                 << ",\"frame\":" << m_debugFrameNumber
                 << ",\"fps\":" << m_debugFps
                 << ",\"gameState\":\"" << GetDebugGameStateName() << "\""
                 << ",\"stageId\":\"" << stageId << "\""
                 << ",\"screenWidth\":" << NSRender::Common::ScreenW()
                 << ",\"screenHeight\":" << NSRender::Common::ScreenH()
                 << ",\"pauseOpen\":";
        if (m_pauseMenu.IsOpen())
        {
            response << "true";
        }
        else
        {
            response << "false";
        }
        response << ",\"selectedPortalId\":\"" << selectedPortalId << "\""
                 << ",\"player\":{\"x\":" << playerPosition.x
                 << ",\"y\":" << playerPosition.y
                 << ",\"z\":" << playerPosition.z
                 << ",\"hp\":" << m_player.GetHp() << "}"
                 << ",\"livingEnemyCount\":" << livingEnemyCount
                 << ",\"nearestEnemy\":";
        if (nearestEnemy == nullptr)
        {
            response << "null";
        }
        else
        {
            const D3DXVECTOR3 enemyPosition = nearestEnemy->GetPosition();
            response << "{\"x\":" << enemyPosition.x
                     << ",\"y\":" << enemyPosition.y
                     << ",\"z\":" << enemyPosition.z
                     << ",\"hp\":" << nearestEnemy->GetHp()
                     << ",\"distance\":" << nearestEnemyDistance << "}";
        }
        response << "}";
        return response.str();
    }

    if (commandName == "KEY_DOWN" || commandName == "KEY_UP")
    {
        std::string keyName;
        commandStream >> keyName;
        const int keyCode = GetDebugKeyCode(keyName);
        if (keyCode < 0)
        {
            return "{\"ok\":false,\"error\":\"unknown_key\"}";
        }

        bool isDown = false;
        if (commandName == "KEY_DOWN")
        {
            isDown = true;
        }
        InputDevice::SKeyBoard::SetInjectedKeyDown(keyCode, isDown);
        return "{\"ok\":true}";
    }

    if (commandName == "CLEAR_KEYS")
    {
        InputDevice::SKeyBoard::ClearInjectedKeys();
        return "{\"ok\":true}";
    }

    if (commandName == "MOUSE_DOWN" || commandName == "MOUSE_UP")
    {
        std::string buttonName;
        commandStream >> buttonName;
        std::transform(buttonName.begin(), buttonName.end(), buttonName.begin(), [](const unsigned char value) {
            return static_cast<char>(toupper(value));
        });
        InputDevice::MouseButton button = InputDevice::MOUSE_LEFT;
        if (buttonName == "LEFT")
        {
            button = InputDevice::MOUSE_LEFT;
        }
        else if (buttonName == "RIGHT")
        {
            button = InputDevice::MOUSE_RIGHT;
        }
        else if (buttonName == "MIDDLE")
        {
            button = InputDevice::MOUSE_MIDDLE;
        }
        else
        {
            return "{\"ok\":false,\"error\":\"unknown_mouse_button\"}";
        }

        bool isDown = false;
        if (commandName == "MOUSE_DOWN")
        {
            isDown = true;
        }
        InputDevice::Mouse::SetInjectedButtonDown(button, isDown);
        return "{\"ok\":true}";
    }

    if (commandName == "CLEAR_INPUT")
    {
        InputDevice::SKeyBoard::ClearInjectedKeys();
        InputDevice::Mouse::ClearInjectedButtons();
        return "{\"ok\":true}";
    }

    return "{\"ok\":false,\"error\":\"unknown_command\"}";
}

const char* GameApp::GetDebugGameStateName() const
{
    switch (m_gameState)
    {
    case GameState::Loading:
        return "Loading";
    case GameState::Title:
        return "Title";
    case GameState::SlideShow:
        return "SlideShow";
    case GameState::StageIntro:
        return "StageIntro";
    case GameState::Playing:
        return "Playing";
    case GameState::StageExit:
        return "StageExit";
    case GameState::StageClear:
        return "StageClear";
    case GameState::GameOver:
        return "GameOver";
    case GameState::Ending:
        return "Ending";
    case GameState::EndingFin:
        return "EndingFin";
    }

    throw std::runtime_error("Unknown game state in debug RPC response.");
}
#endif

void GameApp::Finalize()
{
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
    m_debugRpc.Finalize();
    InputDevice::SKeyBoard::ClearInjectedKeys();
    InputDevice::Mouse::ClearInjectedButtons();
#endif

    RestoreQteVisualEffectImmediate();

    if (m_settingsDialog != NULL)
    {
        DestroyWindow(m_settingsDialog);
        m_settingsDialog = NULL;
    }

    if (m_qte != nullptr)
    {
        m_qte->Finalize();
        delete m_qte;
        m_qte = nullptr;
    }

    if (m_craftMenu.IsOpen())
    {
        m_craftMenu.Close();
    }

    m_interactionManager.Clear();
    m_lavaZoneManager.Clear();
    m_collectibleManager.Clear();
    m_render.Finalize();
    PhysicsWorld::Finalize();
    GameAudio::Finalize();
    SoundLib::SoundLib::Finalize();
    InputDevice::Finalize();

    UnregisterClass(_T("Window1"), m_hInstance);
}

static float ClampFloat(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static D3DXVECTOR3 LerpVector3(const D3DXVECTOR3& a, const D3DXVECTOR3& b, float t)
{
    return a + (b - a) * t;
}

static float LerpFloat(const float a, const float b, const float t)
{
    return a + (b - a) * t;
}

static float SmoothStep01(float t)
{
    t = ClampFloat(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

static float MoveAngleToward(float current, float target, float maxDelta)
{
    float diff = target - current;
    while (diff > D3DX_PI)  diff -= 2.0f * D3DX_PI;
    while (diff < -D3DX_PI) diff += 2.0f * D3DX_PI;
    if (fabsf(diff) <= maxDelta) return target;
    return current + (diff > 0.0f ? maxDelta : -maxDelta);
}

void GameApp::BeginQteVisualEffect()
{
    if (m_qteVisualPhase == QteVisualPhase::None)
    {
        m_qteStoredSaturateEnabled = m_render.IsPostEffectSaturateEnabled();
        m_qteStoredSaturate = m_render.GetPostEffectSaturate();
        m_qteStoredFovDegrees = m_render.GetCameraHorizontalFovDegrees();
    }

    m_qteVisualStartSaturate = m_render.GetPostEffectSaturate();
    m_qteVisualStartFovDegrees = m_render.GetCameraHorizontalFovDegrees();
    m_qteVisualFrame = 0;
    m_qteVisualPhase = QteVisualPhase::Active;
    m_render.SetPostEffectSaturateEnable(true);
}

void GameApp::EndQteVisualEffect()
{
    if (m_qteVisualPhase == QteVisualPhase::None)
    {
        return;
    }

    if (m_qteVisualPhase == QteVisualPhase::Restoring)
    {
        return;
    }

    m_qteVisualStartSaturate = m_render.GetPostEffectSaturate();
    m_qteVisualStartFovDegrees = m_render.GetCameraHorizontalFovDegrees();
    m_qteVisualFrame = 0;
    m_qteVisualPhase = QteVisualPhase::Restoring;
}

void GameApp::RestoreQteVisualEffectImmediate()
{
    if (m_qteVisualPhase == QteVisualPhase::None)
    {
        return;
    }

    m_render.SetPostEffectSaturate(m_qteStoredSaturate);
    if (m_qteStoredSaturateEnabled)
    {
        m_render.SetPostEffectSaturateEnable(true);
    }
    else
    {
        m_render.SetPostEffectSaturateEnable(false);
    }
    m_render.SetCameraHorizontalFovDegrees(m_qteStoredFovDegrees);

    m_qteVisualPhase = QteVisualPhase::None;
    m_qteVisualFrame = 0;
}

void GameApp::UpdateQteVisualEffect()
{
    if (m_qteVisualPhase == QteVisualPhase::None)
    {
        return;
    }

    float targetFov = m_qteStoredFovDegrees - kQteVisualMaxFovReduction;
    targetFov = ClampFloat(targetFov, 45.0f, 120.0f);

    if (m_qteVisualPhase == QteVisualPhase::Active)
    {
        float timingCloseness = 0.0f;
        if (m_qte != nullptr)
        {
            timingCloseness = m_qte->GetTimingCloseness();
        }
        timingCloseness = SmoothStep01(timingCloseness);

        const float saturation = LerpFloat(m_qteStoredSaturate, kQteVisualMinSaturate, timingCloseness);
        const float fov = LerpFloat(m_qteStoredFovDegrees, targetFov, timingCloseness);
        ApplyQteVisualEffect(saturation, fov);
        return;
    }

    ++m_qteVisualFrame;
    const float rawT = static_cast<float>(m_qteVisualFrame) / static_cast<float>(kQteVisualRestoreFrames);
    const float t = SmoothStep01(rawT);
    const float saturation = LerpFloat(m_qteVisualStartSaturate, m_qteStoredSaturate, t);
    const float fov = LerpFloat(m_qteVisualStartFovDegrees, m_qteStoredFovDegrees, t);
    ApplyQteVisualEffect(saturation, fov);

    if (m_qteVisualFrame >= kQteVisualRestoreFrames)
    {
        RestoreQteVisualEffectImmediate();
    }
}

void GameApp::ApplyQteVisualEffect(const float saturation, const float fovDegrees)
{
    m_render.SetPostEffectSaturateEnable(true);
    m_render.SetPostEffectSaturate(ClampFloat(saturation, 0.0f, 2.0f));
    m_render.SetCameraHorizontalFovDegrees(fovDegrees);
}

void GameApp::UpdateDashParticleEffect()
{
    if (!m_playerMover.IsDashing())
    {
        m_dashParticleEmitted = false;
        return;
    }

    if (m_dashParticleEmitted)
    {
        return;
    }

    D3DXVECTOR3 direction = m_playerMover.GetVelocity();
    direction.y = 0.0f;
    if (D3DXVec3LengthSq(&direction) <= 0.0001f)
    {
        direction = D3DXVECTOR3(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
    }
    else
    {
        D3DXVec3Normalize(&direction, &direction);
    }

    const D3DXVECTOR3 back = direction * -1.0f;
    D3DXVECTOR3 origin = m_playerMover.GetPosition();
    origin += D3DXVECTOR3(0.0f, 0.92f, 0.0f);
    origin += back * 0.42f;
    m_render.PlaceDashParticleEffect(origin, direction);
    m_dashParticleEmitted = true;
}

void GameApp::SetPlayerAnimationState(const PlayerAnimState nextState, const float animationSpeed)
{
    m_playerAnimState = nextState;
    m_playerAnimationSpeed = animationSpeed;
    if (m_playerMeshId < 0)
    {
        return;
    }

    m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, animationSpeed);
    if (nextState == PlayerAnimState::Run)
    {
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerRunAnimName);
        return;
    }

    if (nextState == PlayerAnimState::Walk)
    {
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerWalkAnimName);
        return;
    }

    if (nextState == PlayerAnimState::Jump)
    {
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerRunAnimName);
        return;
    }

    if (nextState == PlayerAnimState::Attack)
    {
        const PlayerAttackDefinition& attackDefinition = m_playerAttackController.GetCurrentDefinition();
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, attackDefinition.animationName);
        return;
    }

    if (nextState == PlayerAnimState::Dash)
    {
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerRunAnimName);
        return;
    }

    m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerIdleAnimName);
}

void GameApp::InitializeCameraFromRenderSettings()
{
    const D3DXVECTOR3 cameraPos = m_render.GetCameraPos();
    const D3DXVECTOR3 lookAtPos = m_render.GetLookAtPos();
    const D3DXVECTOR3 offset = cameraPos - lookAtPos;
    const float distance = D3DXVec3Length(&offset);
    if (distance <= 0.0001f)
    {
        return;
    }

    m_cameraDistance = ClampFloat(distance, kMinCameraDistance, kMaxCameraDistance);
    m_cameraPitch = asinf(ClampFloat(offset.y / distance, -1.0f, 1.0f));
    m_cameraPitch = ClampFloat(m_cameraPitch, D3DXToRadian(-20.0f), D3DXToRadian(70.0f));
    m_cameraYaw = atan2f(offset.x, -offset.z);
}

void GameApp::UpdateCameraByInput()
{
    const InputDevice::MousePosition mouseDelta = InputDevice::Mouse::GetDelta();
    if (mouseDelta.x != 0 || mouseDelta.y != 0)
    {
        const float sensitivity = m_remoteDesktopMode ? MOUSE_CAMERA_SENSITIVITY_REMOTE : MOUSE_CAMERA_SENSITIVITY_NORMAL;
        m_cameraYaw   -= static_cast<float>(mouseDelta.x) * sensitivity;
        m_cameraPitch  += static_cast<float>(mouseDelta.y) * sensitivity;
        m_cameraPitch  = ClampFloat(m_cameraPitch, D3DXToRadian(-20.0f), D3DXToRadian(70.0f));
    }
}

void GameApp::UpdatePlayerByInput()
{
    const D3DXVECTOR3 previousRenderPosition = m_playerMover.GetPosition();

    if (IsCurrentStageSelect())
    {
        D3DXVECTOR3 nextPosition = m_playerMover.GetPosition();
        if (m_stageSelectPlayerMoveActive)
        {
            m_stageSelectPlayerMoveElapsed += kTargetFrameSeconds;
            float t = m_stageSelectPlayerMoveElapsed / kStageSelectPlayerMoveDuration;
            if (t >= 1.0f)
            {
                t = 1.0f;
                m_stageSelectPlayerMoveActive = false;
            }

            nextPosition = LerpVector3(m_stageSelectPlayerMoveStartPosition,
                                       m_stageSelectPlayerMoveTargetPosition,
                                       t);
            m_playerMover.SetPosition(nextPosition);

            if (!m_stageSelectPlayerMoveActive)
            {
                m_playerYaw = kStageSelectPlayerRightYaw;
                SetPlayerAnimationState(PlayerAnimState::Walk, kPlayerWalkAnimationSpeed);
                GameAudio::PlayStageSelectMove();
            }
        }
        else if (m_hasSelectedStagePortal)
        {
            nextPosition = m_selectedStagePortalPosition;
            m_playerMover.SetPosition(nextPosition);
            m_playerYaw = kStageSelectPlayerRightYaw;
            if (m_playerAnimState != PlayerAnimState::Walk)
            {
                SetPlayerAnimationState(PlayerAnimState::Walk, kPlayerWalkAnimationSpeed);
            }
        }

        UpdatePlayerMeshAndCamera(previousRenderPosition);
        m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_pendingJump = false;
        return;
    }

    m_playerAttackController.Update();

    // ノックバックカウントダウン
    if (m_playerKnockbackFrames > 0)
    {
        --m_playerKnockbackFrames;
    }

    const bool shiftPressed = InputDevice::SKeyBoard::IsDown(DIK_LSHIFT)
        || InputDevice::SKeyBoard::IsDown(DIK_RSHIFT);

    if (!IsCurrentStageSelect())
    {
        const long wheelDelta = InputDevice::Mouse::GetWheelDelta();
        if (wheelDelta != 0)
        {
            m_playerAttackController.CycleAttackCategory();
            GameAudio::PlayWeaponChange();
        }
        else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP))
        {
            m_playerAttackController.CycleAttackCategory(-1);
            GameAudio::PlayWeaponChange();
        }
        else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN))
        {
            m_playerAttackController.CycleAttackCategory(1);
            GameAudio::PlayWeaponChange();
        }
        UpdateHeldWeaponVisibility();
    }

    const PlayerAttackType requestedAttackType = m_playerAttackController.GetAttackType(shiftPressed);

    if (!IsCurrentStageSelect() && InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        const bool isBombCategory = (m_playerAttackController.GetCurrentCategoryName() == std::wstring(L"海賊爆弾"));
        const bool isBusterCategory = (m_playerAttackController.GetCurrentCategoryName() == std::wstring(L"海賊銃"));
        const bool isStarActive = m_pickupManager.IsStarActive();
        if (isBombCategory)
        {
            if (isStarActive || m_bombAmmo > 0)
            {
                const D3DXVECTOR3 forward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
                const D3DXVECTOR3 bombPos = m_playerMover.GetPosition() + forward * kBombPlaceDistance;
                if (PlaceBomb(bombPos))
                {
                    if (!isStarActive)
                    {
                        --m_bombAmmo;
                    }
                }
            }
        }
        else if (isBusterCategory)
        {
            if (m_busterCooldownFrames <= 0 && (isStarActive || m_busterAmmo > 0))
            {
                if (m_playerAttackController.TryStart(requestedAttackType))
                {
                    const D3DXVECTOR3 forward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
                    D3DXVECTOR3 spawnPos = m_playerMover.GetPosition() + forward * 1.0f;
                    spawnPos.y += kBusterSpawnHeight;
                    SpawnBuster(spawnPos, forward);
                    if (!isStarActive)
                    {
                        --m_busterAmmo;
                    }
                    m_busterCooldownFrames = GetBusterCooldownFrames(m_busterRapidLevel);
                    GameAudio::PlayBuster();
                    const PlayerAttackDefinition& attackDefinition = m_playerAttackController.GetCurrentDefinition();
                    SetPlayerAnimationState(PlayerAnimState::Attack, attackDefinition.animationSpeed);
                }
            }
        }
        else if (m_playerAttackController.TryStart(requestedAttackType))
        {
            GameAudio::PlayPlayerAttack();
            const PlayerAttackDefinition& attackDefinition = m_playerAttackController.GetCurrentDefinition();
            SetPlayerAnimationState(PlayerAnimState::Attack, attackDefinition.animationSpeed);
        }
    }

    const D3DXVECTOR3 cameraForward = GetCameraPlanarForward();
    const D3DXVECTOR3 cameraRight   = GetCameraPlanarRight(cameraForward);

    D3DXVECTOR3 localMove(0.0f, 0.0f, 0.0f);
    if (m_playerKnockbackFrames <= 0)
    {
        if (InputDevice::SKeyBoard::IsDown(DIK_W)) localMove.z += 1.0f;
        if (InputDevice::SKeyBoard::IsDown(DIK_S)) localMove.z -= 1.0f;
        if (InputDevice::SKeyBoard::IsDown(DIK_D)) localMove.x += 1.0f;
        if (InputDevice::SKeyBoard::IsDown(DIK_A)) localMove.x -= 1.0f;
    }

    const bool isMoving  = (localMove.x != 0.0f || localMove.z != 0.0f);
    const bool isWalking = isMoving && InputDevice::SKeyBoard::IsDown(DIK_LCONTROL);

    PhysicsLib::CharacterMover::Settings settings = m_playerMover.GetSettings();
    const float walkSpeed = 1.125f;
    const float runSpeed = 3.375f;
    const float runSpeedMultiplier = m_pickupManager.GetRunSpeedMultiplier();
    const float runAnimationSpeed = 1.5f * runSpeedMultiplier;
    if (isWalking)
    {
        settings.moveSpeed = walkSpeed;
    }
    else
    {
        settings.moveSpeed = runSpeed * runSpeedMultiplier;
    }
    m_playerMover.SetSettings(settings);

    D3DXVECTOR3 move(0.0f, 0.0f, 0.0f);
    if (m_playerKnockbackFrames > 0)
    {
        settings.moveSpeed = kKnockbackSpeed;
        m_playerMover.SetSettings(settings);
        move = m_playerKnockbackDir;
    }
    else if (m_playerAttackController.IsMovementActive())
    {
        const PlayerAttackDefinition& attackDefinition = m_playerAttackController.GetCurrentDefinition();
        const D3DXVECTOR3 forward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
        move = forward;
        settings.moveSpeed = attackDefinition.moveSpeed;
        m_playerMover.SetSettings(settings);
    }
    else if (isMoving)
    {
        const D3DXVECTOR3 desiredMove = cameraRight * localMove.x + cameraForward * localMove.z;
        const bool focusModeEnabled = PhysicsWorld::IsFocusModeEnabled();
        if (focusModeEnabled)
        {
            m_playerYaw = atan2f(-cameraForward.x, -cameraForward.z);
        }
        else
        {
            const float targetYaw = atan2f(-desiredMove.x, -desiredMove.z);
            m_playerYaw = MoveAngleToward(m_playerYaw,
                                          targetYaw,
                                          kPlayerTurnRadiansPerSecond * kTargetFrameSeconds);
        }
        move = desiredMove;
        D3DXVec3Normalize(&move, &move);
    }

    if (m_playerMeshId >= 0)
    {
        const bool isJumping = m_playerMover.IsJumping();
        const bool isDashing = m_playerMover.IsDashing();

        PlayerAnimState nextState;
        if (m_playerAttackController.IsAttacking())
        {
            nextState = PlayerAnimState::Attack;
        }
        else if (isDashing)
        {
            nextState = PlayerAnimState::Dash;
        }
        else if (isJumping)
        {
            nextState = PlayerAnimState::Jump;
        }
        else
        {
            if (isWalking)       nextState = PlayerAnimState::Walk;
            else if (isMoving)   nextState = PlayerAnimState::Run;
            else                nextState = PlayerAnimState::Idle;
        }

        if (nextState != m_playerAnimState)
        {
            float animationSpeed = 1.0f;
            if (nextState == PlayerAnimState::Run)
            {
                animationSpeed = runAnimationSpeed;
            }
            else if (nextState == PlayerAnimState::Walk)
            {
                animationSpeed = kPlayerWalkAnimationSpeed;
            }
            else if (nextState == PlayerAnimState::Jump)
            {
                animationSpeed = 0.1f;
            }
            else if (nextState == PlayerAnimState::Attack)
            {
                animationSpeed = m_playerAttackController.GetCurrentDefinition().animationSpeed;
            }
            else if (nextState == PlayerAnimState::Dash)
            {
                animationSpeed = 0.1f;
                GameAudio::PlayDash();
            }

            SetPlayerAnimationState(nextState, animationSpeed);
        }
        else if (nextState == PlayerAnimState::Run)
        {
            m_playerAnimationSpeed = runAnimationSpeed;
            m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, runAnimationSpeed);
        }
    }

    // 衝突判定は後で行う。カメラはここで設定する。
    UpdatePlayerMeshAndCamera(previousRenderPosition);

    m_pendingMove = move;

    const bool dashModifierPressed = InputDevice::SKeyBoard::IsDown(DIK_LSHIFT)
        || InputDevice::SKeyBoard::IsDown(DIK_RSHIFT);
    const bool jumpPressed = InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE);
    if (jumpPressed && dashModifierPressed)
    {
        const D3DXVECTOR3 dashForward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
        m_playerMover.RequestDash(dashForward);
        m_pendingJump = jumpPressed;
    }
    else
    {
        m_pendingJump = jumpPressed;
    }
}

D3DXVECTOR3 GameApp::GetCameraPlanarForward()
{
    return D3DXVECTOR3(-sinf(m_cameraYaw), 0.0f, cosf(m_cameraYaw));
}

D3DXVECTOR3 GameApp::GetCameraPlanarRight(const D3DXVECTOR3& forward)
{
    D3DXVECTOR3 worldUp(0.0f, 1.0f, 0.0f);
    D3DXVECTOR3 right(1.0f, 0.0f, 0.0f);
    D3DXVec3Cross(&right, &worldUp, &forward);
    if (D3DXVec3LengthSq(&right) <= 0.0001f)
    {
        return D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    }

    D3DXVec3Normalize(&right, &right);
    return right;
}

int GameApp::DamageEnemiesInAttackRange(const PlayerAttackDefinition& attackDefinition)
{
    const D3DXVECTOR3 playerPos = m_playerMover.GetPosition();
    const D3DXVECTOR3 forward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
    const float attackCenterY = playerPos.y + kEnemyAttackTargetHeight;
    int damagedCount = 0;

    for (auto& enemy : m_enemyManager.GetEnemies())
    {
        if (enemy->IsDead())
        {
            continue;
        }

        const D3DXVECTOR3 targetPos = GetEnemyAttackTargetPosition(*enemy);
        if (fabsf(targetPos.y - attackCenterY) > attackDefinition.verticalRange)
        {
            continue;
        }

        D3DXVECTOR3 dir = targetPos - playerPos;
        const float dist = D3DXVec3Length(&dir);
        if (dist > attackDefinition.range)
        {
            continue;
        }

        dir.y = 0.0f;
        if (D3DXVec3LengthSq(&dir) > 0.0001f)
        {
            D3DXVec3Normalize(&dir, &dir);
        }
        else
        {
            dir = forward;
        }

        const float dot = D3DXVec3Dot(&forward, &dir);
        if (dot > cosf(attackDefinition.halfAngleRadians))
        {
            enemy->StartKnockbackFrom(playerPos,
                                     kEnemyAttackKnockbackDistance,
                                     kEnemyAttackKnockbackFrames);
            enemy->TakeDamage(m_render, attackDefinition.damage, playerPos);
            m_damagePopupManager.Add(attackDefinition.damage, enemy->GetPosition(), false);
            TryDropEnemyItem(*enemy);
            ++damagedCount;
        }
    }

    return damagedCount;
}

void GameApp::TryDropEnemyItem(const EnemyBase& enemy)
{
    if (!enemy.IsDead())
    {
        return;
    }

    if (m_destructibleManager.TryDropAmmoHeart(m_render, enemy.GetPosition(), kEnemyAmmoHeartDropPercent))
    {
        return;
    }

    m_destructibleManager.TryDropRedCube(m_render, enemy.GetPosition(), kEnemyItemDropPercent);
}

void GameApp::InitializePlayerPhysics()
{
    PhysicsWorld::Initialize();

    LoadPhysicsObjectsFromCsv(m_stageManager.GetCurrentStage().physicsCsvPath);

    PhysicsLib::CharacterMover::Settings settings;
    settings.shapeType = PhysicsWorld::ShapeType::Cylinder;
    settings.radius = 0.3f;
    settings.height = 1.7f;
    settings.collisionCenterY = 0.85f;
    settings.moveSpeed = 9.0f;
    settings.groundAcceleration = 18.0f;
    settings.airAcceleration = 8.0f;
    settings.jumpVelocity = 5.0f;
    settings.airControlEnabled = true;
    settings.doubleJumpEnabled = m_inventoryManager.IsAbilityUnlocked(L"DoubleJump");
    m_playerMover.SetSettings(settings);
    m_player.ResetHp();
    m_hpBar.Reset();
    m_playerMover.Reset(m_stageManager.GetCurrentStage().playerStartPosition);

    PhysicsLib::SettingsState::SetShapeType(PhysicsWorld::ShapeType::Cylinder);
    PhysicsLib::SettingsState::SetCylinderRadius(0.3f);
    PhysicsLib::SettingsState::SetCylinderHeight(1.7f);
    PhysicsLib::SettingsState::SetInertiaMode(PhysicsLib::InertiaMode::Legacy);
    PhysicsLib::SettingsState::SetGroundDashEnabled(m_inventoryManager.IsAbilityUnlocked(L"GroundDash"));
    PhysicsLib::SettingsState::SetAirDashEnabled(m_inventoryManager.IsAbilityUnlocked(L"AirDash"));
    PhysicsLib::SettingsState::SetDashSpeed(18.0f);
    PhysicsLib::SettingsState::SetDashDuration(0.2f);
}

void GameApp::ApplyUnlockedAbilities()
{
    PhysicsLib::CharacterMover::Settings settings = m_playerMover.GetSettings();
    settings.doubleJumpEnabled = m_inventoryManager.IsAbilityUnlocked(L"DoubleJump");
    m_playerMover.SetSettings(settings);

    PhysicsLib::SettingsState::SetGroundDashEnabled(m_inventoryManager.IsAbilityUnlocked(L"GroundDash"));
    PhysicsLib::SettingsState::SetAirDashEnabled(m_inventoryManager.IsAbilityUnlocked(L"AirDash"));
}

void GameApp::LoadPhysicsObjectsFromCsv(const std::wstring& csvPath)
{
    PhysicsWorld::LoadFromCsv(csvPath.c_str());
}

void GameApp::UpdatePlayerMeshAndCamera(const D3DXVECTOR3& previousRenderPosition)
{
    const D3DXVECTOR3 currentRenderPosition = m_playerMover.GetPosition();
    if (m_playerMeshId >= 0)
    {
        bool playerVisible = !m_playerMover.IsDashBoosterCharging();
        if (!m_debugPlayerRenderEnabled)
        {
            playerVisible = false;
        }
        D3DXVECTOR3 displayPosition = currentRenderPosition;
        float displayScale = 1.0f;
        if (IsCurrentStageSelect())
        {
            displayPosition.y += kStageSelectPlayerVisualOffsetY;
            displayScale = kStageSelectPlayerVisualScale;
        }
        else if (m_gameState == GameState::StageExit)
        {
            displayPosition.y += m_stageExitVisualOffsetY;
        }

        if (m_playerIsSkinAnim)
        {
            m_render.SetMeshMixSkinAnimEnabled(m_playerMeshId, playerVisible);
            m_render.SetMeshMixSkinAnimPos(m_playerMeshId, displayPosition);
            m_render.SetMeshMixSkinAnimRotY(m_playerMeshId, m_playerYaw);
            m_render.SetMeshMixSkinAnimScale(m_playerMeshId, displayScale);
        }
        else
        {
            m_render.SetMeshMixEnabled(m_playerMeshId, playerVisible);
            m_render.SetMeshMixPos(m_playerMeshId, displayPosition);
        }
    }

    UpdateStageSelectPlayerLight();

    // 落下死演出中はメッシュ更新のみ行い、カメラ追従を止めてプレイヤーが落ちていく様を見せる
    if (m_playerFallingDead)
    {
        return;
    }

    if (m_useFixedCamera)
    {
        m_render.SetCamera(m_fixedCameraPos, m_fixedCameraLookAt);
        return;
    }

    if (m_respawnCameraMoveFrames > 0)
    {
        const float t = 1.0f - (static_cast<float>(m_respawnCameraMoveFrames) / static_cast<float>(kRespawnCameraMoveFrames));
        const D3DXVECTOR3 cameraPosition = LerpVector3(m_respawnCameraFromPos, m_respawnCameraToPos, t);
        const D3DXVECTOR3 cameraTarget = LerpVector3(m_respawnCameraFromTarget, m_respawnCameraToTarget, t);
        m_render.SetCamera(cameraPosition, cameraTarget);
        --m_respawnCameraMoveFrames;
        return;
    }

    // yaw/pitch/distanceから理想位置を作り、CameraMoverで壁めり込みを補正する。
    const D3DXVECTOR3 cameraTarget = currentRenderPosition + D3DXVECTOR3(0.0f, 1.2f, 0.0f);
    const float horizontalDistance = m_cameraDistance * cosf(m_cameraPitch);
    const D3DXVECTOR3 offset(sinf(m_cameraYaw) * horizontalDistance,
                              sinf(m_cameraPitch) * m_cameraDistance,
                              -cosf(m_cameraYaw) * horizontalDistance);
    const D3DXVECTOR3 desiredCameraPosition = cameraTarget + offset;
    const D3DXVECTOR3 cameraPosition = m_cameraMover.ResolvePosition(cameraTarget, desiredCameraPosition);
    m_render.SetCamera(cameraPosition, cameraTarget);
}

void GameApp::UpdatePlayerMeshVisibility()
{
    if (m_playerMeshId < 0)
    {
        return;
    }

    if (m_playerIsSkinAnim)
    {
        m_render.SetMeshMixSkinAnimEnabled(m_playerMeshId, m_debugPlayerRenderEnabled);
    }
    else
    {
        m_render.SetMeshMixEnabled(m_playerMeshId, m_debugPlayerRenderEnabled);
    }
}

void GameApp::UpdateHeldWeaponVisibility()
{
    bool stickVisible = false;
    bool saberVisible = false;

    if (m_debugPlayerRenderEnabled && !IsCurrentStageSelect())
    {
        const PlayerAttackType attackType = m_playerAttackController.GetAttackType(false);
        if (attackType == PlayerAttackType::WeakAttack)
        {
            stickVisible = true;
        }
        else if (attackType == PlayerAttackType::SwordAttack)
        {
            saberVisible = true;
        }
    }

    if (m_stickMeshId >= 0)
    {
        m_render.SetMeshMixEnabled(m_stickMeshId, stickVisible);
    }

    if (m_saberMeshId >= 0)
    {
        m_render.SetMeshMixEnabled(m_saberMeshId, saberVisible);
    }
}

void GameApp::ConfigureStagePointLights(const std::wstring& stageId)
{
    m_render.ClearPointLights();
    if (stageId == L"select4")
    {
        const wchar_t* portalDestinationIds[] =
        {
            L"select3",
            L"4-1",
            L"4-2",
            L"4-3",
            L"4-4",
            L"4-5",
            L"4-6",
            L"4-7",
            L"4-8",
            L"base"
        };
        const D3DXVECTOR3 portalLightPositions[] =
        {
            D3DXVECTOR3(-18.0f, 2.35f, -12.0f),
            D3DXVECTOR3(-12.0f, 2.40f, -10.0f),
            D3DXVECTOR3(-5.0f, 2.48f, -7.0f),
            D3DXVECTOR3(3.0f, 2.44f, -9.0f),
            D3DXVECTOR3(10.0f, 2.70f, -5.0f),
            D3DXVECTOR3(7.0f, 3.25f, 2.0f),
            D3DXVECTOR3(0.0f, 3.80f, 5.0f),
            D3DXVECTOR3(-7.0f, 4.50f, 10.0f),
            D3DXVECTOR3(0.0f, 5.25f, 15.5f),
            D3DXVECTOR3(18.0f, 2.42f, -10.0f)
        };
        const D3DXCOLOR unclearedColor(1.0f, 0.04f, 0.02f, 1.0f);
        const D3DXCOLOR clearedColor(0.04f, 1.0f, 0.08f, 1.0f);
        const D3DXCOLOR travelColor(0.04f, 0.25f, 1.0f, 1.0f);
        const int portalLightCount = static_cast<int>(sizeof(portalDestinationIds) / sizeof(portalDestinationIds[0]));
        for (int i = 0; i < portalLightCount; ++i)
        {
            const std::wstring destinationId = portalDestinationIds[i];
            D3DXCOLOR lightColor = unclearedColor;
            if (destinationId == L"select3" || destinationId == L"base")
            {
                lightColor = travelColor;
            }
            else if (m_saveDataManager.IsStageCleared(destinationId))
            {
                lightColor = clearedColor;
            }
            m_render.AddPointLight(portalLightPositions[i], 1.8f, lightColor);
        }

        const D3DXCOLOR fireColor(1.0f, 0.14f, 0.02f, 1.0f);
        const D3DXCOLOR spiritColor(0.35f, 0.65f, 1.0f, 1.0f);
        const D3DXCOLOR dawnColor(1.0f, 0.38f, 0.12f, 1.0f);
        m_render.AddPointLight(D3DXVECTOR3(-1.5f, 2.7f, -3.2f), 2.2f, fireColor);
        m_render.AddPointLight(D3DXVECTOR3(0.0f, 6.0f, 15.5f), 2.0f, spiritColor);
        m_render.AddPointLight(D3DXVECTOR3(12.0f, 5.0f, 9.0f), 1.8f, dawnColor);

        const D3DXVECTOR3 playerLightPosition =
            m_playerMover.GetPosition() + D3DXVECTOR3(0.0f, kStageSelectPlayerLightHeight, 0.0f);
        const D3DXCOLOR playerLightColor(1.0f, 0.78f, 0.52f, 1.0f);
        m_render.AddPointLight(playerLightPosition,
                               2.5f,
                               playerLightColor,
                               NSRender::PointLightShape::Point,
                               12.0f,
                               10.0f,
                               10.0f,
                               D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                               8.0f,
                               kStageSelectPlayerLightOwnerTag);
        return;
    }

    if (stageId == L"select3")
    {
        const wchar_t* portalDestinationIds[] =
        {
            L"select2",
            L"3-1",
            L"3-2",
            L"3-3",
            L"3-4",
            L"3-5",
            L"3-6",
            L"3-7",
            L"3-8",
            L"select4",
            L"base"
        };
        const D3DXVECTOR3 portalLightPositions[] =
        {
            D3DXVECTOR3(-16.0f, 2.4f, -10.0f),
            D3DXVECTOR3(-11.0f, 2.6f, -8.0f),
            D3DXVECTOR3(-5.0f, 3.4f, -3.0f),
            D3DXVECTOR3(4.0f, 4.2f, -1.0f),
            D3DXVECTOR3(11.0f, 5.0f, 3.0f),
            D3DXVECTOR3(7.0f, 6.0f, 8.0f),
            D3DXVECTOR3(0.0f, 6.8f, 10.5f),
            D3DXVECTOR3(-7.0f, 7.8f, 14.0f),
            D3DXVECTOR3(0.0f, 9.1f, 17.5f),
            D3DXVECTOR3(15.0f, 2.8f, -8.0f),
            D3DXVECTOR3(-19.0f, 2.8f, -4.0f)
        };
        const D3DXCOLOR unclearedColor(1.0f, 0.04f, 0.02f, 1.0f);
        const D3DXCOLOR clearedColor(0.04f, 1.0f, 0.08f, 1.0f);
        const D3DXCOLOR travelColor(0.04f, 0.25f, 1.0f, 1.0f);
        const int portalLightCount = static_cast<int>(sizeof(portalDestinationIds) / sizeof(portalDestinationIds[0]));
        for (int i = 0; i < portalLightCount; ++i)
        {
            const std::wstring destinationId = portalDestinationIds[i];
            D3DXCOLOR lightColor = unclearedColor;
            if (destinationId == L"select2" || destinationId == L"select4" || destinationId == L"base")
            {
                lightColor = travelColor;
            }
            else if (m_saveDataManager.IsStageCleared(destinationId))
            {
                lightColor = clearedColor;
            }
            m_render.AddPointLight(portalLightPositions[i], 5.0f, lightColor);
        }

        const D3DXCOLOR coldLight(0.10f, 0.30f, 1.0f, 1.0f);
        const D3DXCOLOR spiritLight(0.55f, 0.82f, 1.0f, 1.0f);
        const D3DXCOLOR sealLight(1.0f, 0.34f, 0.04f, 1.0f);
        m_render.AddPointLight(D3DXVECTOR3(-4.0f, 7.5f, 14.0f), 5.5f, spiritLight);
        m_render.AddPointLight(D3DXVECTOR3(0.0f, 10.2f, 21.0f), 7.0f, sealLight);
        m_render.AddPointLight(D3DXVECTOR3(15.0f, 3.0f, -7.0f), 4.5f, coldLight);

        const D3DXVECTOR3 playerLightPosition =
            m_playerMover.GetPosition() + D3DXVECTOR3(0.0f, kStageSelectPlayerLightHeight, 0.0f);
        const D3DXCOLOR playerLightColor(1.0f, 0.78f, 0.52f, 1.0f);
        m_render.AddPointLight(playerLightPosition,
                               7.0f,
                               playerLightColor,
                               NSRender::PointLightShape::Point,
                               12.0f,
                               10.0f,
                               10.0f,
                               D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                               8.0f,
                               kStageSelectPlayerLightOwnerTag);
        return;
    }

    if (stageId != L"select2")
    {
        return;
    }

    const wchar_t* portalDestinationIds[] =
    {
        L"select1",
        L"2-1",
        L"2-2",
        L"2-3",
        L"2-4",
        L"2-5",
        L"2-6",
        L"2-7",
        L"2-8",
        L"select3",
        L"base"
    };
    const D3DXVECTOR3 portalLightPositions[] =
    {
        D3DXVECTOR3(-14.0f, 2.3f, 14.0f),
        D3DXVECTOR3(-3.0f, 2.3f, 15.0f),
        D3DXVECTOR3(8.0f, 2.3f, 12.0f),
        D3DXVECTOR3(13.0f, 2.3f, 7.0f),
        D3DXVECTOR3(8.0f, 2.3f, 3.0f),
        D3DXVECTOR3(-3.0f, 2.3f, 2.0f),
        D3DXVECTOR3(-10.0f, 2.3f, 0.0f),
        D3DXVECTOR3(-9.0f, 2.3f, -5.0f),
        D3DXVECTOR3(-3.0f, 2.3f, -8.0f),
        D3DXVECTOR3(9.0f, 2.3f, -7.0f),
        D3DXVECTOR3(-17.5f, 2.3f, 11.0f)
    };
    const D3DXCOLOR unclearedColor(1.0f, 0.04f, 0.02f, 1.0f);
    const D3DXCOLOR clearedColor(0.04f, 1.0f, 0.08f, 1.0f);
    const D3DXCOLOR travelColor(0.04f, 0.25f, 1.0f, 1.0f);
    const int portalLightCount = static_cast<int>(sizeof(portalDestinationIds) / sizeof(portalDestinationIds[0]));
    for (int i = 0; i < portalLightCount; ++i)
    {
        const std::wstring destinationId = portalDestinationIds[i];
        D3DXCOLOR lightColor = unclearedColor;
        if (destinationId == L"select1" || destinationId == L"select3" || destinationId == L"base")
        {
            lightColor = travelColor;
        }
        else if (m_saveDataManager.IsStageCleared(destinationId))
        {
            lightColor = clearedColor;
        }
        m_render.AddPointLight(portalLightPositions[i], 5.0f, lightColor);
    }

    const D3DXVECTOR3 playerLightPosition =
        m_playerMover.GetPosition() + D3DXVECTOR3(0.0f, kStageSelectPlayerLightHeight, 0.0f);
    const D3DXCOLOR playerLightColor(1.0f, 0.78f, 0.52f, 1.0f);
    m_render.AddPointLight(playerLightPosition,
                           7.0f,
                           playerLightColor,
                           NSRender::PointLightShape::Point,
                           12.0f,
                           10.0f,
                           10.0f,
                           D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                           8.0f,
                           kStageSelectPlayerLightOwnerTag);
}

void GameApp::UpdateStageSelectPlayerLight()
{
    const std::wstring& stageId = m_stageManager.GetCurrentStage().id;
    if (stageId != L"select2" && stageId != L"select3" && stageId != L"select4")
    {
        return;
    }

    const D3DXVECTOR3 lightPosition =
        m_playerMover.GetPosition() + D3DXVECTOR3(0.0f, kStageSelectPlayerLightHeight, 0.0f);
    m_render.SetPointLightPositionByOwnerTag(kStageSelectPlayerLightOwnerTag, lightPosition);
}

bool GameApp::IsCurrentStageSelect() const
{
    const std::wstring& currentId = m_stageManager.GetCurrentStage().id;
    return IsStageSelectId(currentId);
}

bool GameApp::IsStagePortalSelectable(const std::wstring& portalId) const
{
    const std::wstring prefix = L"portal-to-";
    if (portalId.length() <= prefix.length() || portalId.substr(0, prefix.length()) != prefix)
    {
        return false;
    }

    const std::wstring destinationId = portalId.substr(prefix.length());
    if (destinationId == L"base")
    {
        return true;
    }
    return m_saveDataManager.IsStageUnlocked(destinationId);
}

bool GameApp::AreAllStageEnemiesDefeated() const
{
    for (const auto& enemy : m_enemyManager.GetEnemies())
    {
        if (!enemy->IsDead())
        {
            return false;
        }
    }

    return true;
}

bool GameApp::ShouldShowGoalArrow() const
{
    if (m_gameState != GameState::Playing)
    {
        return false;
    }

    if (IsCurrentStageSelect())
    {
        return false;
    }

    const int stageNumber = m_stageManager.GetCurrentStage().number;
    if (stageNumber < 1 || stageNumber > 32)
    {
        return false;
    }

    if (!AreAllStageEnemiesDefeated())
    {
        return false;
    }

    return true;
}

void GameApp::EnsureGoalArrow()
{
    if (m_goalArrowMeshId >= 0)
    {
        m_render.SetMeshMixEnabled(m_goalArrowMeshId, true);
        return;
    }

    m_goalArrowMeshId = m_render.AddMeshMix(kGoalArrowModelPath,
                                            D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                            D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                            1.0f);
}

void GameApp::RemoveGoalArrow()
{
    if (m_goalArrowMeshId < 0)
    {
        return;
    }

    m_render.RemoveMeshMix(m_goalArrowMeshId);
    m_goalArrowMeshId = -1;
}

void GameApp::UpdateGoalArrow()
{
    if (!ShouldShowGoalArrow())
    {
        if (m_goalArrowMeshId >= 0)
        {
            m_render.SetMeshMixEnabled(m_goalArrowMeshId, false);
        }
        return;
    }

    EnsureGoalArrow();
    if (m_goalArrowMeshId < 0)
    {
        return;
    }

    const D3DXVECTOR3 cameraPosition = m_render.GetCameraPos();
    const D3DXVECTOR3 lookAtPosition = m_render.GetLookAtPos();
    D3DXVECTOR3 cameraForward = lookAtPosition - cameraPosition;
    if (D3DXVec3LengthSq(&cameraForward) <= 0.0001f)
    {
        cameraForward = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    }
    D3DXVec3Normalize(&cameraForward, &cameraForward);

    const D3DXVECTOR3 worldUp(0.0f, 1.0f, 0.0f);
    D3DXVECTOR3 cameraRight;
    D3DXVec3Cross(&cameraRight, &worldUp, &cameraForward);
    if (D3DXVec3LengthSq(&cameraRight) <= 0.0001f)
    {
        cameraRight = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    }
    D3DXVec3Normalize(&cameraRight, &cameraRight);

    D3DXVECTOR3 cameraUp;
    D3DXVec3Cross(&cameraUp, &cameraForward, &cameraRight);
    if (D3DXVec3LengthSq(&cameraUp) <= 0.0001f)
    {
        cameraUp = worldUp;
    }
    D3DXVec3Normalize(&cameraUp, &cameraUp);

    D3DXVECTOR3 toGoal = m_stageManager.GetCurrentStage().clearPosition - m_playerMover.GetPosition();
    D3DXVECTOR3 arrowUp;
    if (D3DXVec3LengthSq(&toGoal) <= 0.0001f)
    {
        arrowUp = cameraUp;
    }
    else
    {
        D3DXVec3Normalize(&arrowUp, &toGoal);
    }

    D3DXVECTOR3 arrowRight;
    D3DXVec3Cross(&arrowRight, &worldUp, &arrowUp);
    if (D3DXVec3LengthSq(&arrowRight) <= 0.0001f)
    {
        const D3DXVECTOR3 referenceForward(0.0f, 0.0f, 1.0f);
        D3DXVec3Cross(&arrowRight, &referenceForward, &arrowUp);
        if (D3DXVec3LengthSq(&arrowRight) <= 0.0001f)
        {
            arrowRight = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
        }
    }
    D3DXVec3Normalize(&arrowRight, &arrowRight);

    D3DXVECTOR3 arrowForward;
    D3DXVec3Cross(&arrowForward, &arrowRight, &arrowUp);
    D3DXVec3Normalize(&arrowForward, &arrowForward);

    const D3DXVECTOR3 arrowPosition =
        m_playerMover.GetPosition() + worldUp * kGoalArrowHeadOffsetY;

    D3DXMATRIX arrowWorld;
    D3DXMatrixIdentity(&arrowWorld);
    arrowWorld._11 = arrowRight.x * kGoalArrowScale;
    arrowWorld._12 = arrowRight.y * kGoalArrowScale;
    arrowWorld._13 = arrowRight.z * kGoalArrowScale;
    arrowWorld._21 = arrowUp.x * kGoalArrowScale;
    arrowWorld._22 = arrowUp.y * kGoalArrowScale;
    arrowWorld._23 = arrowUp.z * kGoalArrowScale;
    arrowWorld._31 = arrowForward.x * kGoalArrowScale;
    arrowWorld._32 = arrowForward.y * kGoalArrowScale;
    arrowWorld._33 = arrowForward.z * kGoalArrowScale;
    arrowWorld._41 = arrowPosition.x;
    arrowWorld._42 = arrowPosition.y;
    arrowWorld._43 = arrowPosition.z;

    m_render.SetMeshMixWorldMatrix(m_goalArrowMeshId, arrowWorld);
    m_render.SetMeshMixEnabled(m_goalArrowMeshId, true);
}

void GameApp::InitializeStageSelectCursor()
{
    m_selectedStagePortalId.clear();
    m_selectedStagePortalPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_hasSelectedStagePortal = false;
    m_stageSelectPlayerMoveStartPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_stageSelectPlayerMoveTargetPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_stageSelectPlayerMoveElapsed = 0.0f;
    m_stageSelectPlayerMoveActive = false;
    m_stageSelectStickDirectionActive = false;

    if (!IsCurrentStageSelect())
    {
        m_preferredStageSelectPortalId.clear();
        return;
    }

    const std::vector<InteractionManager::Interactable>& interactables = m_interactionManager.GetInteractables();

    if (!m_preferredStageSelectPortalId.empty())
    {
        for (const InteractionManager::Interactable& interactable : interactables)
        {
            if (interactable.id == m_preferredStageSelectPortalId &&
                IsStagePortalSelectable(interactable.id))
            {
                m_selectedStagePortalId = interactable.id;
                m_selectedStagePortalPosition = interactable.position;
                m_hasSelectedStagePortal = true;
                m_preferredStageSelectPortalId.clear();
                SyncStageSelectPlayerToPortal(true);
                return;
            }
        }

        m_preferredStageSelectPortalId.clear();
    }

    for (const InteractionManager::Interactable& interactable : interactables)
    {
        if (interactable.type != L"StagePortal" || !IsStagePortalSelectable(interactable.id))
        {
            continue;
        }

        const std::wstring destinationId = interactable.id.substr(std::wstring(L"portal-to-").length());
        const bool isNavigationPortal = destinationId == L"base" ||
            (destinationId.length() >= 6 && destinationId.substr(0, 6) == L"select");
        if (isNavigationPortal)
        {
            continue;
        }

        m_selectedStagePortalId = interactable.id;
        m_selectedStagePortalPosition = interactable.position;
        m_hasSelectedStagePortal = true;
        SyncStageSelectPlayerToPortal(true);
        return;
    }

    for (const InteractionManager::Interactable& interactable : interactables)
    {
        if (interactable.type == L"StagePortal" && IsStagePortalSelectable(interactable.id))
        {
            m_selectedStagePortalId = interactable.id;
            m_selectedStagePortalPosition = interactable.position;
            m_hasSelectedStagePortal = true;
            SyncStageSelectPlayerToPortal(true);
            return;
        }
    }
}

void GameApp::SyncStageSelectPlayerToPortal(const bool immediate)
{
    if (!IsCurrentStageSelect() || !m_hasSelectedStagePortal)
    {
        m_stageSelectPlayerMoveActive = false;
        m_stageSelectPlayerMoveElapsed = 0.0f;
        return;
    }

    const D3DXVECTOR3 targetPosition = m_selectedStagePortalPosition;
    if (immediate)
    {
        m_stageSelectPlayerMoveActive = false;
        m_stageSelectPlayerMoveElapsed = 0.0f;
        m_stageSelectPlayerMoveStartPosition = targetPosition;
        m_stageSelectPlayerMoveTargetPosition = targetPosition;
        m_playerMover.SetPosition(targetPosition);
        m_playerYaw = kStageSelectPlayerRightYaw;
        SetPlayerAnimationState(PlayerAnimState::Walk, kPlayerWalkAnimationSpeed);
        return;
    }

    const D3DXVECTOR3 currentPosition = m_playerMover.GetPosition();
    const D3DXVECTOR3 difference = targetPosition - currentPosition;
    if (D3DXVec3LengthSq(&difference) <= 0.0001f)
    {
        m_stageSelectPlayerMoveActive = false;
        m_stageSelectPlayerMoveElapsed = 0.0f;
        m_stageSelectPlayerMoveStartPosition = targetPosition;
        m_stageSelectPlayerMoveTargetPosition = targetPosition;
        m_playerYaw = kStageSelectPlayerRightYaw;
        SetPlayerAnimationState(PlayerAnimState::Walk, kPlayerWalkAnimationSpeed);
        return;
    }

    m_stageSelectPlayerMoveActive = true;
    m_stageSelectPlayerMoveElapsed = 0.0f;
    m_stageSelectPlayerMoveStartPosition = currentPosition;
    m_stageSelectPlayerMoveTargetPosition = targetPosition;
    if (targetPosition.x < currentPosition.x)
    {
        m_playerYaw = kStageSelectPlayerLeftYaw;
    }
    else
    {
        m_playerYaw = kStageSelectPlayerRightYaw;
    }
    SetPlayerAnimationState(PlayerAnimState::Run, 1.2f);
}

void GameApp::MoveStageSelectCursorByDirection(const float directionX, const float directionY)
{
    if (!m_hasSelectedStagePortal)
    {
        return;
    }

    const std::vector<InteractionManager::Interactable>& interactables = m_interactionManager.GetInteractables();

    const POINT currentScreenPosition = NSRender::Camera::GetScreenPos(m_selectedStagePortalPosition);
    if (currentScreenPosition.x < 0 || currentScreenPosition.y < 0)
    {
        return;
    }

    const InteractionManager::Interactable* bestInteractable = nullptr;
    float bestScore = 100000000.0f;

    for (const InteractionManager::Interactable& interactable : interactables)
    {
        if (interactable.type != L"StagePortal" ||
            interactable.id == m_selectedStagePortalId ||
            !AreStagePortalsSequential(m_selectedStagePortalId, interactable.id) ||
            !IsStagePortalSelectable(interactable.id))
        {
            continue;
        }

        const POINT candidateScreenPosition = NSRender::Camera::GetScreenPos(interactable.position);
        if (candidateScreenPosition.x < 0 || candidateScreenPosition.y < 0)
        {
            continue;
        }

        const float differenceX = static_cast<float>(candidateScreenPosition.x - currentScreenPosition.x);
        const float differenceY = static_cast<float>(candidateScreenPosition.y - currentScreenPosition.y);
        const float distance = sqrtf(differenceX * differenceX + differenceY * differenceY);
        if (distance <= 0.0001f)
        {
            continue;
        }

        const float directionDistance = differenceX * directionX + differenceY * directionY;
        const float alignment = directionDistance / distance;
        if (alignment < 0.35f)
        {
            continue;
        }

        const float score = distance + (1.0f - alignment) * 500.0f;
        if (score < bestScore)
        {
            bestScore = score;
            bestInteractable = &interactable;
        }
    }

    if (bestInteractable != nullptr)
    {
        m_selectedStagePortalId = bestInteractable->id;
        m_selectedStagePortalPosition = bestInteractable->position;
        m_hasSelectedStagePortal = true;
        SyncStageSelectPlayerToPortal(false);
    }
}

void GameApp::UpdateStageSelectCursorByInput()
{
    if (!IsCurrentStageSelect())
    {
        return;
    }

    if (!m_mouseCursorVisible || !InputDevice::Mouse::IsVisible())
    {
        m_mouseCursorVisible = true;
        InputDevice::Mouse::SetVisible(true);
    }

    float directionX = 0.0f;
    float directionY = 0.0f;
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LEFT))
    {
        directionX = -1.0f;
    }
    else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RIGHT))
    {
        directionX = 1.0f;
    }
    else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP))
    {
        directionY = -1.0f;
    }
    else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN))
    {
        directionY = 1.0f;
    }
    else if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_LEFT))
    {
        directionX = -1.0f;
    }
    else if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_RIGHT))
    {
        directionX = 1.0f;
    }
    else if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_UP))
    {
        directionY = -1.0f;
    }
    else if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_POV_DOWN))
    {
        directionY = 1.0f;
    }

    const InputDevice::GamePadStick leftStick = InputDevice::GamePad::GetStickL();
    if (leftStick.power <= 0.35f)
    {
        m_stageSelectStickDirectionActive = false;
    }
    else if (directionX == 0.0f && directionY == 0.0f &&
             !m_stageSelectStickDirectionActive && leftStick.power >= 0.60f)
    {
        m_stageSelectStickDirectionActive = true;
        if (fabsf(leftStick.x) >= fabsf(leftStick.y))
        {
            if (leftStick.x < 0.0f)
            {
                directionX = -1.0f;
            }
            else
            {
                directionX = 1.0f;
            }
        }
        else if (leftStick.y > 0.0f)
        {
            directionY = -1.0f;
        }
        else
        {
            directionY = 1.0f;
        }
    }

    if (!m_stageSelectPlayerMoveActive && (directionX != 0.0f || directionY != 0.0f))
    {
        MoveStageSelectCursorByDirection(directionX, directionY);
    }

    const InputDevice::MousePosition mousePosition = InputDevice::Mouse::GetPosition();
    const POINT baseMousePosition = ConvertMouseToBaseResolution(mousePosition.x, mousePosition.y);
    if (baseMousePosition.x >= kStageSelectStartButtonX &&
        baseMousePosition.x < kStageSelectStartButtonX + kStageSelectStartButtonWidth &&
        baseMousePosition.y >= kStageSelectStartButtonY &&
        baseMousePosition.y < kStageSelectStartButtonY + kStageSelectStartButtonHeight)
    {
        m_isMouseOverStartButton = true;
    }
    else
    {
        m_isMouseOverStartButton = false;
    }

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        if (m_isMouseOverStartButton)
        {
            MoveToSelectedStagePortal();
        }
        else
        {
            const std::vector<InteractionManager::Interactable>& interactables = m_interactionManager.GetInteractables();
            float nearestDistanceSquared = kStagePortalClickRadius * kStagePortalClickRadius;
            const InteractionManager::Interactable* selectedInteractable = nullptr;

            for (const InteractionManager::Interactable& interactable : interactables)
            {
                if (interactable.type != L"StagePortal" ||
                    !IsStagePortalSelectable(interactable.id) ||
                    (IsStageSelectNavigationPortal(interactable.id) &&
                     !AreStagePortalsSequential(m_selectedStagePortalId, interactable.id)))
                {
                    continue;
                }

                const POINT screenPosition = NSRender::Camera::GetScreenPos(interactable.position);
                if (screenPosition.x < 0 || screenPosition.y < 0)
                {
                    continue;
                }

                const float scaleX = static_cast<float>(NSRender::Common::BASE_W) /
                    static_cast<float>(NSRender::Common::ScreenW());
                const float scaleY = static_cast<float>(NSRender::Common::BASE_H) /
                    static_cast<float>(NSRender::Common::ScreenH());
                const float portalX = static_cast<float>(screenPosition.x) * scaleX;
                const float portalY = static_cast<float>(screenPosition.y) * scaleY;
                const float differenceX = portalX - static_cast<float>(baseMousePosition.x);
                const float differenceY = portalY - static_cast<float>(baseMousePosition.y);
                const float distanceSquared = differenceX * differenceX + differenceY * differenceY;
                if (distanceSquared <= nearestDistanceSquared)
                {
                    nearestDistanceSquared = distanceSquared;
                    selectedInteractable = &interactable;
                }
            }

            if (selectedInteractable != nullptr)
            {
                m_selectedStagePortalId = selectedInteractable->id;
                m_selectedStagePortalPosition = selectedInteractable->position;
                m_hasSelectedStagePortal = true;
                SyncStageSelectPlayerToPortal(false);
            }
        }
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN) ||
        InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_A))
    {
        MoveToSelectedStagePortal();
    }
}

bool GameApp::MoveToSelectedStagePortal()
{
    if (m_stageSelectPlayerMoveActive)
    {
        return false;
    }

    if (!m_hasSelectedStagePortal || !IsStagePortalSelectable(m_selectedStagePortalId))
    {
        return false;
    }

    const std::wstring prefix = L"portal-to-";
    const std::wstring destinationId = m_selectedStagePortalId.substr(prefix.length());
    const std::size_t targetIndex = m_stageManager.FindStageIndexById(destinationId);
    if (targetIndex >= m_stageManager.GetStageCount())
    {
        return false;
    }

    m_lastSelectId = m_stageManager.GetCurrentStage().id;
    if (!StartStageByIndex(targetIndex))
    {
        return false;
    }

    GameAudio::PlayStageSelectConfirm();
    return true;
}

std::wstring GameApp::GetSelectedStagePortalDisplayName() const
{
    if (!m_hasSelectedStagePortal)
    {
        return L"";
    }

    const std::wstring prefix = L"portal-to-";
    if (m_selectedStagePortalId.length() <= prefix.length() ||
        m_selectedStagePortalId.substr(0, prefix.length()) != prefix)
    {
        return L"";
    }

    const std::wstring destinationId = m_selectedStagePortalId.substr(prefix.length());
    const std::size_t targetIndex = m_stageManager.FindStageIndexById(destinationId);
    if (targetIndex >= m_stageManager.GetStageCount())
    {
        return L"";
    }

    return BuildStageComboText(m_stageManager.GetStage(targetIndex));
}

void GameApp::DrawStageSelectCursor()
{
    if (!IsCurrentStageSelect())
    {
        return;
    }

    if (m_stageSelectFontId < 0)
    {
        m_stageSelectFontId = m_render.SetUpFontEx(L"BIZ UDGothic", 30, D3DCOLOR_RGBA(255, 255, 255, 255));
    }
    if (m_stageSelectHintFontId < 0)
    {
        m_stageSelectHintFontId = m_render.SetUpFontEx(L"BIZ UDGothic", 24, D3DCOLOR_RGBA(255, 255, 255, 220));
    }
    if (m_stageSelectStartButtonFontId < 0)
    {
        m_stageSelectStartButtonFontId = m_render.SetUpFontEx(L"BIZ UDGothic", 40, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    const std::wstring stageName = GetSelectedStagePortalDisplayName();
    if (!stageName.empty())
    {
        m_render.DrawTextEx(m_stageSelectFontId,
                            stageName,
                            kStageSelectStageNameX,
                            kStageSelectStageNameY,
                            D3DCOLOR_RGBA(255, 255, 255, 255));

        const std::wstring livesText = L"残機: " + std::to_wstring(m_player.GetLives());
        m_render.DrawTextExCenter(m_stageSelectFontId,
                                  livesText,
                                  kStageSelectLivesX,
                                  kStageSelectLivesY,
                                  kStageSelectLivesWidth,
                                  32,
                                  D3DCOLOR_RGBA(255, 255, 255, 230));
    }

    m_render.DrawTextEx(m_stageSelectHintFontId,
                        L"方向キー・マウス: ステージ選択",
                        kStageSelectStageNameX,
                        kStageSelectHintY,
                        D3DCOLOR_RGBA(255, 255, 255, 220));

    m_render.DrawTextEx(m_stageSelectHintFontId,
                        L"エンター・クリック: 開始",
                        1180,
                        kStageSelectHintY,
                        D3DCOLOR_RGBA(255, 255, 255, 220));

    UINT startButtonColor = D3DCOLOR_RGBA(255, 255, 255, 255);
    if (m_stageSelectPlayerMoveActive)
    {
        startButtonColor = D3DCOLOR_RGBA(160, 160, 160, 220);
    }
    else if (m_isMouseOverStartButton)
    {
        startButtonColor = D3DCOLOR_RGBA(255, 255, 0, 255);
    }

    m_render.DrawTextExCenter(m_stageSelectStartButtonFontId,
                              L"スタート",
                              kStageSelectStartButtonX,
                              kStageSelectStartButtonY,
                              kStageSelectStartButtonWidth,
                              kStageSelectStartButtonHeight,
                              startButtonColor);
}

void GameApp::LoadItemNameCatalog()
{
    m_itemDisplayNames.clear();

    const std::vector<std::vector<std::wstring>> csvData =
        csv::Read(NSRender::Util::GetExeDir() + kItemNameCsvPath);

    for (const std::vector<std::wstring>& row : csvData)
    {
        if (row.size() >= 2 && row.at(0) != L"ID")
        {
            m_itemDisplayNames[row.at(0)] = row.at(1);
        }
    }
}

std::wstring GameApp::GetItemDisplayName(const std::wstring& itemId) const
{
    const auto found = m_itemDisplayNames.find(itemId);
    if (found != m_itemDisplayNames.end())
    {
        return found->second;
    }

    const std::wstring message = L"Undefined item id: " + itemId + L"\n";
    OutputDebugStringW(message.c_str());
    std::abort();
}

void GameApp::HandleItemCollected(const std::wstring& itemId, const int count)
{
    if (itemId == kBombCapacityUpItemId)
    {
        for (int i = 0; i < count; ++i)
        {
            if (m_baseBombCapacity < kMaxBombs)
            {
                ++m_baseBombCapacity;
            }
        }
    }
    else if (itemId == kBusterRapidUpItemId)
    {
        for (int i = 0; i < count; ++i)
        {
            if (m_baseBusterRapidLevel < kBusterRapidLevelMax)
            {
                ++m_baseBusterRapidLevel;
            }
        }
    }

    if (m_pickupManager.IsStarActive())
    {
        MaximizeTemporaryPowerUps();
    }
    else
    {
        RestoreTemporaryPowerUps();
    }

    ShowItemPickupMessage(itemId, count);
}

int GameApp::GetCurrentAmmo() const
{
    const PlayerAttackType attackType = m_playerAttackController.GetAttackType(false);
    if (IsBusterAttackType(attackType))
    {
        return m_busterAmmo;
    }

    if (IsBombAttackType(attackType))
    {
        return m_bombAmmo;
    }

    return 0;
}

int GameApp::GetCurrentAmmoMax() const
{
    const PlayerAttackType attackType = m_playerAttackController.GetAttackType(false);
    if (IsBusterAttackType(attackType))
    {
        return kBusterAmmoMax;
    }

    if (IsBombAttackType(attackType))
    {
        return kBombAmmoMax;
    }

    return 0;
}

int GameApp::GetCurrentWorld() const
{
    int currentWorld = 1;
    const std::size_t stageCount = m_stageManager.GetStageCount();
    for (std::size_t i = 0; i < stageCount; ++i)
    {
        const StageManager::StageData& stage = m_stageManager.GetStage(i);
        if (!m_saveDataManager.IsStageUnlocked(stage.id))
        {
            continue;
        }

        int stageWorld = 1;
        if (!stage.id.empty())
        {
            const wchar_t firstChar = stage.id.at(0);
            if (firstChar >= L'1' && firstChar <= L'4')
            {
                stageWorld = static_cast<int>(firstChar - L'0');
            }
        }

        if (stageWorld > currentWorld)
        {
            currentWorld = stageWorld;
        }
    }

    return currentWorld;
}

void GameApp::RefillWeaponAmmo()
{
    m_busterAmmo = kBusterAmmoMax;
    m_bombAmmo = kBombAmmoMax;
}

bool GameApp::HandleInventoryItemUse(const std::wstring& itemId)
{
    if (itemId == kRedSpaghettiItemId)
    {
        if (m_player.GetLives() >= m_player.GetMaxLives())
        {
            return false;
        }

        if (!m_inventoryManager.RemoveItem(itemId, 1))
        {
            return false;
        }

        return m_player.AddLife();
    }

    if (itemId == kPotatoChipsItemId || itemId == kChuageJuiceItemId)
    {
        if (m_player.GetHp() >= m_player.GetMaxHp())
        {
            return false;
        }

        if (!m_inventoryManager.RemoveItem(itemId, 1))
        {
            return false;
        }

        HealPlayerHp(m_player.GetMaxHp());
        return true;
    }

    return false;
}

bool GameApp::TryUseRecoveryItemFromKey()
{
    if (m_itemUseCooldownFrames > 0)
    {
        return false;
    }

    const int chipsCount = m_inventoryManager.GetItemCount(kPotatoChipsItemId);
    const int juiceCount = m_inventoryManager.GetItemCount(kChuageJuiceItemId);

    if (chipsCount > 0)
    {
        if (!HandleInventoryItemUse(kPotatoChipsItemId))
        {
            return false;
        }

        m_itemUseCooldownFrames = 60;
        return true;
    }

    if (juiceCount > 0)
    {
        if (!HandleInventoryItemUse(kChuageJuiceItemId))
        {
            return false;
        }

        m_itemUseCooldownFrames = 60;
        return true;
    }

    return false;
}

bool GameApp::RecoverWeaponAmmoFromPickup()
{
    const PlayerAttackType attackType = m_playerAttackController.GetAttackType(false);
    if (IsBusterAttackType(attackType) && m_busterAmmo < kBusterAmmoMax)
    {
        m_busterAmmo += kBusterAmmoRecoverAmount;
        if (m_busterAmmo > kBusterAmmoMax)
        {
            m_busterAmmo = kBusterAmmoMax;
        }
        m_itemPickupMessage = L"バスター弾を回復";
        m_itemPickupMessageFrames = kItemPickupMessageTotalFrames;
        if (m_busterAmmo >= kBusterAmmoMax)
        {
            GameAudio::PlayAmmoMax();
        }
        return true;
    }

    if (IsBombAttackType(attackType) && m_bombAmmo < kBombAmmoMax)
    {
        m_bombAmmo += kBombAmmoRecoverAmount;
        if (m_bombAmmo > kBombAmmoMax)
        {
            m_bombAmmo = kBombAmmoMax;
        }
        m_itemPickupMessage = L"爆弾を回復";
        m_itemPickupMessageFrames = kItemPickupMessageTotalFrames;
        if (m_bombAmmo >= kBombAmmoMax)
        {
            GameAudio::PlayAmmoMax();
        }
        return true;
    }

    if (m_busterAmmo < kBusterAmmoMax)
    {
        m_busterAmmo += kBusterAmmoRecoverAmount;
        if (m_busterAmmo > kBusterAmmoMax)
        {
            m_busterAmmo = kBusterAmmoMax;
        }
        m_itemPickupMessage = L"バスター弾を回復";
        m_itemPickupMessageFrames = kItemPickupMessageTotalFrames;
        if (m_busterAmmo >= kBusterAmmoMax)
        {
            GameAudio::PlayAmmoMax();
        }
        return true;
    }

    if (m_bombAmmo < kBombAmmoMax)
    {
        m_bombAmmo += kBombAmmoRecoverAmount;
        if (m_bombAmmo > kBombAmmoMax)
        {
            m_bombAmmo = kBombAmmoMax;
        }
        m_itemPickupMessage = L"爆弾を回復";
        m_itemPickupMessageFrames = kItemPickupMessageTotalFrames;
        if (m_bombAmmo >= kBombAmmoMax)
        {
            GameAudio::PlayAmmoMax();
        }
        return true;
    }

    return false;
}

void GameApp::DrawAmmoGauge()
{
    const int ammoMax = GetCurrentAmmoMax();
    if (ammoMax <= 0)
    {
        return;
    }

    int ammo = GetCurrentAmmo();
    if (ammo < 0)
    {
        ammo = 0;
    }
    if (ammo > ammoMax)
    {
        ammo = ammoMax;
    }

    const int railTotalWidth = ((ammoMax - 1) * kAmmoBeadStep) + kAmmoBeadSize;
    m_render.DrawImageSized(kAmmoRailImagePath,
                            kAmmoGaugeX,
                            kAmmoGaugeY + kAmmoRailOffsetY,
                            railTotalWidth,
                            kAmmoRailHeight,
                            220);

    for (int i = 0; i < ammoMax; ++i)
    {
        const std::wstring* beadPath = &kAmmoBeadEmptyImagePath;
        if (i < ammo)
        {
            beadPath = &kAmmoBeadFullImagePath;
        }
        m_render.DrawImageSized(*beadPath,
                                kAmmoGaugeX + (i * kAmmoBeadStep),
                                kAmmoGaugeY,
                                kAmmoBeadSize,
                                kAmmoBeadSize,
                                255);
    }
}

void GameApp::MaximizeTemporaryPowerUps()
{
    m_bombCapacity = kMaxBombs;
    m_busterRapidLevel = kBusterRapidLevelMax;
    RefillWeaponAmmo();
}

void GameApp::RestoreTemporaryPowerUps()
{
    m_bombCapacity = m_baseBombCapacity;
    m_busterRapidLevel = m_baseBusterRapidLevel;
}

void GameApp::ShowItemPickupMessage(const std::wstring& itemId, const int count)
{
    std::wstring message = GetItemDisplayName(itemId) + L"を入手";
    if (count > 1)
    {
        message += L" x" + std::to_wstring(count);
    }

    m_itemPickupMessage = message;
    m_itemPickupMessageFrames = kItemPickupMessageTotalFrames;
}

void GameApp::DrawItemPickupMessage()
{
    if (m_itemPickupMessageFrames <= 0 || m_itemPickupMessage.empty())
    {
        return;
    }

    if (m_itemPickupMessageFontId < 0)
    {
        m_itemPickupMessageFontId = m_render.SetUpFontEx(L"BIZ UDGothic",
                                                          28,
                                                          D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    const int elapsedFrames = kItemPickupMessageTotalFrames - m_itemPickupMessageFrames;
    int alpha = 255;
    if (elapsedFrames < kItemPickupMessageFadeFrames)
    {
        alpha = (elapsedFrames * 255) / kItemPickupMessageFadeFrames;
    }
    else if (m_itemPickupMessageFrames < kItemPickupMessageFadeFrames)
    {
        alpha = (m_itemPickupMessageFrames * 255) / kItemPickupMessageFadeFrames;
    }

    if (alpha < 0)
    {
        alpha = 0;
    }
    if (alpha > 255)
    {
        alpha = 255;
    }

    m_render.DrawTextExCenter(m_itemPickupMessageFontId,
                              m_itemPickupMessage,
                              0,
                              kItemPickupMessageY,
                              NSRender::Common::BASE_W,
                              42,
                              D3DCOLOR_RGBA(255, 255, 255, alpha));

    --m_itemPickupMessageFrames;
    if (m_itemPickupMessageFrames <= 0)
    {
        m_itemPickupMessage.clear();
    }
}

void GameApp::RemoveStageSelectCubes()
{
    for (auto it = m_stageSelectCubeMeshIds.rbegin(); it != m_stageSelectCubeMeshIds.rend(); ++it)
    {
        m_render.RemoveMeshMix(*it);
    }
    m_stageSelectCubeMeshIds.clear();
}

void GameApp::CreateStageSelectCubes()
{
    if (!IsCurrentStageSelect())
    {
        return;
    }

    RemoveStageSelectCubes();

    const std::wstring portalPrefix = L"portal-to-";
    const std::vector<InteractionManager::Interactable>& interactables = m_interactionManager.GetInteractables();
    for (const InteractionManager::Interactable& interactable : interactables)
    {
        if (interactable.type != L"StagePortal")
        {
            continue;
        }

        if (interactable.id.length() <= portalPrefix.length() ||
            interactable.id.substr(0, portalPrefix.length()) != portalPrefix)
        {
            continue;
        }

        const std::wstring destinationId = interactable.id.substr(portalPrefix.length());
        std::wstring cubePath;
        if (destinationId == L"base")
        {
            cubePath = kStageSelectCubeBluePath;
        }
        else
        {
            if (!m_saveDataManager.IsStageUnlocked(destinationId))
            {
                continue;
            }

            if (destinationId.length() >= 6 && destinationId.substr(0, 6) == L"select")
            {
                cubePath = kStageSelectCubeBluePath;
            }
            else
            {
                if (m_saveDataManager.IsStageCleared(destinationId))
                {
                    cubePath = kStageSelectCubeGreenPath;
                }
                else
                {
                    cubePath = kStageSelectCubeRedPath;
                }
            }
        }

        const int renderId = m_render.AddMeshMix(cubePath,
                                                  interactable.position,
                                                  D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                  kStageSelectCubeScale);
        if (renderId >= 0)
        {
            m_stageSelectCubeMeshIds.push_back(renderId);
        }
    }
}

void GameApp::PopulateStageCombo(HWND hDlg)
{
    HWND combo = GetDlgItem(hDlg, IDC_COMBO_STAGE);
    if (combo == NULL)
    {
        return;
    }

    SendMessage(combo, CB_RESETCONTENT, 0, 0);

    const std::size_t stageCount = m_stageManager.GetStageCount();
    for (std::size_t i = 0; i < stageCount; ++i)
    {
        const std::wstring text = BuildStageComboText(m_stageManager.GetStage(i));
        const LRESULT itemIndex = SendMessage(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
        if (itemIndex >= 0)
        {
            SendMessage(combo, CB_SETITEMDATA, static_cast<WPARAM>(itemIndex), static_cast<LPARAM>(i));
        }
    }

    SendMessage(combo, CB_SETCURSEL, static_cast<WPARAM>(m_stageManager.GetCurrentStageIndex()), 0);
}

std::wstring GameApp::BuildStageComboText(const StageManager::StageData& stage) const
{
    if (stage.displayName == L"拠点")
    {
        return L"Base";
    }

    const std::wstring prefix = L"Stage ";
    if (stage.displayName.find(prefix) == 0)
    {
        return stage.displayName.substr(prefix.size());
    }

    return stage.displayName;
}

void GameApp::PopulateUnlockStageCombo(HWND hDlg)
{
    HWND combo = GetDlgItem(hDlg, IDC_COMBO_UNLOCK_STAGE);
    if (combo == NULL)
    {
        return;
    }

    SendMessage(combo, CB_RESETCONTENT, 0, 0);

    std::size_t lastUnlockedIndex = 0;
    const std::size_t stageCount = m_stageManager.GetStageCount();
    for (std::size_t i = 0; i < stageCount; ++i)
    {
        const StageManager::StageData& stage = m_stageManager.GetStage(i);
        const std::wstring text = BuildStageComboText(stage);
        const LRESULT itemIndex = SendMessage(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
        if (itemIndex >= 0)
        {
            SendMessage(combo, CB_SETITEMDATA, static_cast<WPARAM>(itemIndex), static_cast<LPARAM>(i));
        }

        if (m_saveDataManager.IsStageUnlocked(stage.id))
        {
            lastUnlockedIndex = i;
        }
    }

    SendMessage(combo, CB_SETCURSEL, static_cast<WPARAM>(lastUnlockedIndex), 0);
}

void GameApp::PopulateSpeedLevelCombo(HWND hDlg)
{
    HWND combo = GetDlgItem(hDlg, IDC_COMBO_SPEED_LEVEL);
    if (combo == NULL)
    {
        return;
    }

    SendMessage(combo, CB_RESETCONTENT, 0, 0);

    const int maxSpeedLevel = m_pickupManager.GetMaxSpeedLevel();
    for (int speedLevel = 1; speedLevel <= maxSpeedLevel; ++speedLevel)
    {
        const std::wstring text = L"Lv " + std::to_wstring(speedLevel);
        const LRESULT itemIndex = SendMessage(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
        if (itemIndex >= 0)
        {
            SendMessage(combo, CB_SETITEMDATA, static_cast<WPARAM>(itemIndex), static_cast<LPARAM>(speedLevel));
        }
    }

    int currentSpeedLevel = m_pickupManager.GetSpeedLevel();
    if (currentSpeedLevel < 1)
    {
        currentSpeedLevel = 1;
    }
    if (currentSpeedLevel > maxSpeedLevel)
    {
        currentSpeedLevel = maxSpeedLevel;
    }

    SendMessage(combo, CB_SETCURSEL, static_cast<WPARAM>(currentSpeedLevel - 1), 0);
}

void GameApp::ApplySelectedSpeedLevel(HWND hDlg)
{
    HWND combo = GetDlgItem(hDlg, IDC_COMBO_SPEED_LEVEL);
    if (combo == NULL)
    {
        return;
    }

    const LRESULT selectedIndex = SendMessage(combo, CB_GETCURSEL, 0, 0);
    if (selectedIndex == CB_ERR)
    {
        return;
    }

    const LRESULT speedLevel = SendMessage(combo, CB_GETITEMDATA, static_cast<WPARAM>(selectedIndex), 0);
    if (speedLevel == CB_ERR)
    {
        return;
    }

    m_pickupManager.SetSpeedLevel(static_cast<int>(speedLevel));
}

void GameApp::UnlockStagesUpToSelected(HWND hDlg)
{
    HWND combo = GetDlgItem(hDlg, IDC_COMBO_UNLOCK_STAGE);
    if (combo == NULL)
    {
        return;
    }

    const LRESULT selectedIndex = SendMessage(combo, CB_GETCURSEL, 0, 0);
    if (selectedIndex == CB_ERR)
    {
        return;
    }

    const LRESULT stageIndex = SendMessage(combo, CB_GETITEMDATA, static_cast<WPARAM>(selectedIndex), 0);
    if (stageIndex == CB_ERR)
    {
        return;
    }

    const std::size_t targetIndex = static_cast<std::size_t>(stageIndex);
    const std::size_t stageCount = m_stageManager.GetStageCount();
    for (std::size_t i = 0; i < stageCount && i <= targetIndex; ++i)
    {
        const StageManager::StageData& stage = m_stageManager.GetStage(i);
        m_saveDataManager.MarkStageUnlocked(stage.id);
    }
}

void GameApp::AllUnlockStages(HWND hDlg)
{
    m_saveDataManager.MarkAllStagesClearedAndUnlocked();
    m_saveDataManager.Save();
    PopulateUnlockStageCombo(hDlg);
    PopulateStageCombo(hDlg);
    RefreshTitleCommands();
}

bool GameApp::StartStageByIndex(std::size_t stageIndex)
{
    if (stageIndex >= m_stageManager.GetStageCount())
    {
        return false;
    }

    const StageManager::StageData& stage = m_stageManager.GetStage(stageIndex);
    if (stage.id == L"base")
    {
        return StartStageByIndexImmediate(stageIndex);
    }

    const std::wstring storyScriptPath = GetStageStoryScriptPath(stage.id, L"Before");
    if (!storyScriptPath.empty())
    {
        if (m_gameState == GameState::Playing && IsCurrentStageSelect())
        {
            return BeginStageTransitionToStory(stageIndex);
        }

        m_pendingStageIndexAfterSlideShow = stageIndex;
        m_slideShowManager.Start(storyScriptPath);
        m_slideShowManager.SetStopOnFinish(false);
        m_gameState = GameState::SlideShow;
        return true;
    }

    return StartStageByIndexImmediate(stageIndex);
}

bool GameApp::StartStageByIndexImmediate(std::size_t stageIndex)
{
    if (stageIndex >= m_stageManager.GetStageCount())
    {
        return false;
    }

    const StageManager::StageData& targetStage = m_stageManager.GetStage(stageIndex);
    const bool leavesStageSelect = m_gameState == GameState::Playing && IsCurrentStageSelect();
    const bool entersStageSelect = IsStageSelectId(targetStage.id);
    if (leavesStageSelect || entersStageSelect)
    {
        return BeginStageTransitionToIndex(stageIndex);
    }

    return CompleteStageMove(stageIndex);
}

bool GameApp::BeginStageTransitionToIndex(const std::size_t stageIndex)
{
    if (stageIndex >= m_stageManager.GetStageCount() ||
        m_stageTransitionAction != StageTransitionAction::None)
    {
        return false;
    }

    m_stageTransitionIndex = stageIndex;
    m_stageTransitionAction = StageTransitionAction::MoveToIndex;
    m_render.StartFadeOut(kStageSelectTransitionFadeDuration);
    return true;
}

bool GameApp::BeginStageTransitionToStory(const std::size_t stageIndex)
{
    if (stageIndex >= m_stageManager.GetStageCount() ||
        m_stageTransitionAction != StageTransitionAction::None)
    {
        return false;
    }

    m_stageTransitionIndex = stageIndex;
    m_stageTransitionAction = StageTransitionAction::StartStory;
    m_render.StartFadeOut(kStageSelectTransitionFadeDuration);
    return true;
}

bool GameApp::BeginStageTransitionAfterClear()
{
    if (m_stageTransitionAction != StageTransitionAction::None)
    {
        return false;
    }

    m_stageTransitionIndex = static_cast<std::size_t>(-1);
    m_stageTransitionAction = StageTransitionAction::MoveAfterClear;
    m_render.StartFadeOut(kStageSelectTransitionFadeDuration);
    return true;
}

void GameApp::UpdateStageTransition()
{
    if (m_stageTransitionAction == StageTransitionAction::FadeIn)
    {
        if (m_render.GetFadeAlpha() <= 0.0f)
        {
            m_stageTransitionAction = StageTransitionAction::None;
            m_stageTransitionIndex = static_cast<std::size_t>(-1);
            return;
        }

        if (m_gameState == GameState::SlideShow && m_slideShowManager.IsActive())
        {
            m_render.Draw();
            m_slideShowManager.Render();
            m_slideShowManager.DrawSkipHint();
            return;
        }

        if (IsCurrentStageSelect())
        {
            DrawStageSelectCursor();
        }
        m_render.Draw();
        return;
    }

    if (m_render.GetFadeAlpha() < 1.0f)
    {
        if (m_gameState == GameState::Title)
        {
            DrawTitleScreen();
            return;
        }

        if (m_gameState == GameState::StageClear)
        {
            DrawStageClear();
        }
        else if (IsCurrentStageSelect())
        {
            DrawStageSelectCursor();
        }

        m_render.Draw();
        return;
    }

    const StageTransitionAction action = m_stageTransitionAction;
    const std::size_t stageIndex = m_stageTransitionIndex;
    m_stageTransitionAction = StageTransitionAction::None;
    m_stageTransitionIndex = static_cast<std::size_t>(-1);

    if (action == StageTransitionAction::MoveToIndex)
    {
        if (!CompleteStageMove(stageIndex))
        {
            throw std::runtime_error("Failed to move to a stage after the stage-select fade-out.");
        }
        return;
    }

    if (action == StageTransitionAction::StartStory)
    {
        const StageManager::StageData& stage = m_stageManager.GetStage(stageIndex);
        const std::wstring storyScriptPath = GetStageStoryScriptPath(stage.id, L"Before");
        if (storyScriptPath.empty())
        {
            throw std::runtime_error("Stage story was not found after the stage-select fade-out.");
        }

        m_pendingStageIndexAfterSlideShow = stageIndex;
        m_slideShowManager.Start(storyScriptPath);
        m_slideShowManager.SetStopOnFinish(false);
        m_render.StartFadeIn(kStageSelectTransitionFadeDuration);
        m_gameState = GameState::SlideShow;
        m_stageTransitionAction = StageTransitionAction::FadeIn;
        return;
    }

    if (action == StageTransitionAction::MoveAfterClear)
    {
        if (!MoveToStageAfterClear())
        {
            throw std::runtime_error("Failed to return to stage select after the fade-out.");
        }
        if (IsCurrentStageSelect())
        {
            m_stageTransitionAction = StageTransitionAction::FadeIn;
        }
        return;
    }

    throw std::runtime_error("Invalid stage transition action.");
}

bool GameApp::CompleteStageMove(const std::size_t stageIndex)
{
    if (!m_stageManager.MoveToStage(stageIndex))
    {
        return false;
    }

    LoadCurrentStageObjects();
    if (IsCurrentStageSelect() || m_stageManager.GetCurrentStage().id == L"base")
    {
        m_render.SetFadeAlpha(1.0f);
        m_render.StartFadeIn(kStageSelectTransitionFadeDuration);
        m_gameState = GameState::Playing;
        m_stageTransitionAction = StageTransitionAction::FadeIn;
    }
    else
    {
        m_render.SetFadeAlpha(1.0f);
        m_gameState = GameState::StageIntro;
        BeginStageIntro();
    }
    return true;
}

void GameApp::StartNewGame()
{
    m_saveDataManager.ResetToDefaults();
    m_inventoryManager.Reset();
    m_inventoryManager.AddWeapon(kInitialClubWeaponId, 1);
    m_inventoryManager.Save();
    m_baseBombCapacity = 1;
    m_baseBusterRapidLevel = 1;
    m_bombCapacity = 1;
    m_busterRapidLevel = 1;
    RefillWeaponAmmo();

    const std::size_t select1Index = m_stageManager.FindStageIndexById(L"select1");
    if (select1Index < m_stageManager.GetStageCount())
    {
        m_stageManager.MoveToStage(select1Index);
        LoadCurrentStageObjects();
    }

    m_slideShowManager.Start(L"res\\script\\hoshigirl_trial_novel.csv");
    m_slideShowManager.SetStopOnFinish(false);
    m_gameState = GameState::SlideShow;
}

void GameApp::RefreshTitleCommands()
{
    const bool canContinue = m_saveDataManager.HasSaveFile();
    m_command.UpsertCommand(L"continue", canContinue);
    m_command.UpsertCommand(L"delete", canContinue);
}

void GameApp::BuildTitleMainCommands()
{
    m_command.RemoveAll();
    const bool canContinue = m_saveDataManager.HasSaveFile();
    m_command.UpsertCommand(L"start", true);
    m_command.UpsertCommand(L"continue", canContinue);
    m_command.UpsertCommand(L"delete", canContinue);
    m_command.UpsertCommand(L"language", true);
    m_command.UpsertCommand(L"exit", true);
}

void GameApp::BuildTitleConfirmCommands()
{
    m_command.RemoveAll();
    m_command.UpsertCommand(L"yes", true);
    m_command.UpsertCommand(L"no", true);
}

void GameApp::BuildTitleLanguageCommands()
{
    m_command.RemoveAll();
    m_command.UpsertCommand(L"english", true);
    m_command.UpsertCommand(L"japanese", true);
}

void GameApp::EnterDeleteConfirmation()
{
    BuildTitleConfirmCommands();
    m_titleDeleteConfirmMode = true;
}

void GameApp::ExitDeleteConfirmation()
{
    BuildTitleMainCommands();
    m_titleDeleteConfirmMode = false;
}

void GameApp::ExitTitleLanguageSelection()
{
    BuildTitleMainCommands();
    m_titleLanguageSelectionMode = false;
}

void GameApp::ExecuteDeleteSaveData()
{
    m_saveDataManager.DeleteSaveData();
    m_inventoryManager.Reset();
    DeleteFileW((NSRender::Util::GetExeDir() + L"res\\savedata\\inventory.csv").c_str());
    ExitDeleteConfirmation();
}

void GameApp::ExecuteTitleCommand(const std::wstring& commandId)
{
    if (commandId.empty())
    {
        return;
    }

    if (commandId == L"start")
    {
        StartNewGame();
    }
    else if (commandId == L"continue")
    {
        m_saveDataManager.Load();
        StartStageByIndex(GetContinueStartStageIndex());
    }
    else if (commandId == L"delete")
    {
        EnterDeleteConfirmation();
    }
    else if (commandId == L"language")
    {
        BuildTitleLanguageCommands();
        m_titleLanguageSelectionMode = true;
    }
    else if (commandId == L"english")
    {
        m_titleLanguage = TitleLanguage::English;
        ExitTitleLanguageSelection();
    }
    else if (commandId == L"japanese")
    {
        m_titleLanguage = TitleLanguage::Japanese;
        ExitTitleLanguageSelection();
    }
    else if (commandId == L"exit")
    {
        m_close = true;
    }
}

std::size_t GameApp::GetContinueStartStageIndex() const
{
    if (m_saveDataManager.IsStageUnlocked(L"select4"))
    {
        return m_stageManager.FindStageIndexById(L"select4");
    }
    if (m_saveDataManager.IsStageUnlocked(L"select3"))
    {
        return m_stageManager.FindStageIndexById(L"select3");
    }
    if (m_saveDataManager.IsStageUnlocked(L"select2"))
    {
        return m_stageManager.FindStageIndexById(L"select2");
    }
    return m_stageManager.FindStageIndexById(L"select1");
}

void GameApp::MoveToSelectedStage(HWND hDlg)
{
    HWND combo = GetDlgItem(hDlg, IDC_COMBO_STAGE);
    if (combo == NULL)
    {
        return;
    }

    const LRESULT selectedIndex = SendMessage(combo, CB_GETCURSEL, 0, 0);
    if (selectedIndex == CB_ERR)
    {
        return;
    }

    const LRESULT stageIndex = SendMessage(combo, CB_GETITEMDATA, static_cast<WPARAM>(selectedIndex), 0);
    if (stageIndex == CB_ERR)
    {
        return;
    }

    StartStageByIndex(static_cast<std::size_t>(stageIndex));
    PopulateStageCombo(hDlg);
}

INT_PTR CALLBACK GameApp::SettingsDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return Instance().OnSettingsDialog(hDlg, msg, wParam, lParam);
}

INT_PTR GameApp::OnSettingsDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK,
                    m_remoteDesktopMode ? BST_CHECKED : BST_UNCHECKED, 0);
        if (m_debugPlayerRenderEnabled)
        {
            SendMessage(GetDlgItem(hDlg, IDC_CHECK_HIDE_PLAYER), BM_SETCHECK, BST_UNCHECKED, 0);
        }
        else
        {
            SendMessage(GetDlgItem(hDlg, IDC_CHECK_HIDE_PLAYER), BM_SETCHECK, BST_CHECKED, 0);
        }
        SetDlgItemText(hDlg, IDC_EDIT_CAMERA_DIST, std::to_wstring(m_cameraDistance).c_str());
        PopulateStageCombo(hDlg);
        PopulateUnlockStageCombo(hDlg);
        PopulateSpeedLevelCombo(hDlg);
        m_stageEditor.Initialize(&m_render, &m_stageManager, &m_enemyManager, &m_playerMover, &m_playerYaw);
        m_stageEditor.OnInitDialog(hDlg);
        return TRUE;

    case WM_NOTIFY:
        m_stageEditor.OnNotify(hDlg, reinterpret_cast<LPNMHDR>(lParam));
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CHECK1:
            m_remoteDesktopMode = (SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, 0, 0) == BST_CHECKED);
            InputDevice::SetRemoteDesktopMode(m_remoteDesktopMode);
            return TRUE;

        case IDC_CHECK_HIDE_PLAYER:
            m_debugPlayerRenderEnabled =
                (SendMessage(GetDlgItem(hDlg, IDC_CHECK_HIDE_PLAYER), BM_GETCHECK, 0, 0) != BST_CHECKED);
            UpdatePlayerMeshVisibility();
            UpdateHeldWeaponVisibility();
            return TRUE;

        case IDC_BUTTON_RESET_MOVING:
            m_render.ResetMovingPlatforms();
            return TRUE;

        case IDC_BUTTON_HP_MINUS:
            DamagePlayerHp(10);
            return TRUE;

        case IDC_BUTTON_HP_PLUS:
            HealPlayerHp(10);
            return TRUE;

        case IDC_BUTTON_KILL_ALL_ENEMIES:
            m_enemyManager.RemoveAll(m_render);
            return TRUE;

        case IDC_EDIT_CAMERA_DIST:
            if (HIWORD(wParam) == EN_CHANGE)
            {
                wchar_t buf[32] = {};
                GetDlgItemText(hDlg, IDC_EDIT_CAMERA_DIST, buf, 32);
                float dist = static_cast<float>(_wtof(buf));
                if (dist >= kMinCameraDistance && dist <= kMaxCameraDistance)
                {
                    m_cameraDistance = dist;
                    D3DXVECTOR3 lookAt = m_render.GetLookAtPos();
                    float hDist = m_cameraDistance * cosf(m_cameraPitch);
                    D3DXVECTOR3 offset(sinf(m_cameraYaw) * hDist,
                                        sinf(m_cameraPitch) * m_cameraDistance,
                                        -cosf(m_cameraYaw) * hDist);
                    m_render.SetCamera(lookAt + offset, lookAt);
                }
            }
            return TRUE;

        case IDC_BUTTON_STAGE_GO:
            MoveToSelectedStage(hDlg);
            return TRUE;

        case IDC_BUTTON_UNLOCK_STAGES:
            UnlockStagesUpToSelected(hDlg);
            return TRUE;

        case IDC_BUTTON_ALL_UNLOCK:
            AllUnlockStages(hDlg);
            return TRUE;

        case IDC_COMBO_SPEED_LEVEL:
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                ApplySelectedSpeedLevel(hDlg);
            }
            return TRUE;

        case IDC_BUTTON_SELECT_X:
        case IDC_BUTTON_PLACE_MESH:
        case IDC_BUTTON_DELETE_MESH:
        case IDC_BUTTON_SAVE_STAGE:
            m_stageEditor.OnCommand(hDlg, LOWORD(wParam));
            return TRUE;

        case IDOK:
        case IDCANCEL:
            ShowWindow(hDlg, SW_HIDE);
            return TRUE;
        }
        break;

    case WM_CLOSE:
        ShowWindow(hDlg, SW_HIDE);
        return TRUE;
    }

    return FALSE;
}

LRESULT WINAPI GameApp::MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CLOSE:
    {
        Instance().m_close = true;
        DestroyWindow(hWnd);
        return 0;
    }

    case WM_DESTROY:
    {
        PostQuitMessage(0);
        Instance().m_close = true;
        return 0;
    }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void GameApp::UpdateTitleByInput()
{
    if (InputDevice::UnifiedInput::IsDownFirstFrame(InputDevice::GAMEPAD_POV_LEFT))
    {
        m_command.Previous();
    }

    if (InputDevice::UnifiedInput::IsDownFirstFrame(InputDevice::GAMEPAD_POV_RIGHT))
    {
        m_command.Next();
    }
}

void GameApp::BeginStageExit()
{
    m_pauseMenu.Close();
    m_mouseCursorVisible = false;
    InputDevice::Mouse::SetVisible(false);
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_stageExitFrame = 0;
    m_stageExitVisualOffsetY = 0.0f;
    m_gameState = GameState::StageExit;

    SetPlayerAnimationState(PlayerAnimState::Idle, 1.0f);
}

void GameApp::UpdateStageExit()
{
    if (m_stageExitFrame == kStageExitJumpDelayFrames)
    {
        if (m_playerMeshId >= 0)
        {
            m_playerAnimState = PlayerAnimState::Jump;
            m_playerAnimationSpeed = kStageExitAnimationSpeed;
            m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, m_playerAnimationSpeed);
            m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerJumpAnimName);
        }

        GameAudio::PlayJump();
    }

    int jumpFrame = m_stageExitFrame - kStageExitJumpDelayFrames;
    if (jumpFrame < 0)
    {
        jumpFrame = 0;
    }
    float riseT = static_cast<float>(jumpFrame) /
                  static_cast<float>(kStageExitJumpDurationFrames);
    if (riseT > 1.0f)
    {
        riseT = 1.0f;
    }
    m_stageExitVisualOffsetY = kStageExitRiseHeight * riseT * riseT;

    UpdatePlayerMeshAndCamera(m_playerMover.GetPosition());

    if (m_stageExitFrame == kStageExitFadeStartFrame)
    {
        m_render.StartFadeOut(kStageExitFadeDurationSeconds);
    }

    if (m_stageExitFrame >= kStageExitTransitionFrame)
    {
        m_stageExitVisualOffsetY = 0.0f;
        if (!MoveToStageAfterClear())
        {
            throw std::runtime_error("Failed to return to stage select after stage exit animation.");
        }
        return;
    }

    m_render.Draw();
    ++m_stageExitFrame;
}

void GameApp::UpdateStageClear()
{
    const std::wstring clearedStageId = m_stageManager.GetCurrentStage().id;
    if (!m_stageClearProcessed)
    {
        m_stageClearWasFirstClear = !m_saveDataManager.IsStageCleared(clearedStageId);
        m_saveDataManager.MarkStageCleared(clearedStageId);
        m_saveDataManager.MarkStageUnlocked(clearedStageId);

        const int stageNumber = m_stageManager.GetCurrentStageNumber();
        const std::vector<std::wstring> unlockIds = m_stageManager.GetUnlockStageIds(stageNumber);
        for (const std::wstring& id : unlockIds)
        {
            m_saveDataManager.MarkStageUnlocked(id);
        }

        BeginStageClearVisual();
        m_stageClearProcessed = true;
    }

    UpdateStageClearVisual();

    int inputFrame = kStageClearInputFrame;
    if (!m_stageClearWasFirstClear)
    {
        inputFrame = kStageClearReplayInputFrame;
    }

    bool proceedToNextScene = false;
    if (m_stageClearFrame >= inputFrame)
    {
        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE) ||
            InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN) ||
            InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_A))
        {
            proceedToNextScene = true;
        }
    }

    const bool isFinalStage = m_stageManager.GetCurrentStage().id == L"4-8";
    int finalAutoFrame = kStageClearFinalAutoFrame;
    if (!m_stageClearWasFirstClear)
    {
        finalAutoFrame = kStageClearReplayFinalAutoFrame;
    }
    if (isFinalStage && m_stageClearFrame >= finalAutoFrame)
    {
        proceedToNextScene = true;
    }

    if (proceedToNextScene)
    {
        RestoreStageClearVisual();
        if (isFinalStage)
        {
            m_slideShowManager.Start(L"res\\script\\ending.csv");
            m_slideShowManager.SetStopOnFinish(false);
            m_gameState = GameState::Ending;
            return;
        }

        const std::wstring storyScriptPath = GetStageStoryScriptPath(clearedStageId, L"After");
        if (!storyScriptPath.empty())
        {
            m_slideShowManager.Start(storyScriptPath);
            m_slideShowManager.SetStopOnFinish(false);
            m_startStageAfterSlideShow = true;
            m_gameState = GameState::SlideShow;
            return;
        }
        if (StartStageAfterClear())
        {
            return;
        }
    }

    DrawStageClear();
    m_render.Draw();
}

void GameApp::BeginStageClearVisual()
{
    m_stageClearFrame = 0;
    m_stageClearCameraStartPos = m_render.GetCameraPos();
    m_stageClearCameraStartTarget = m_render.GetLookAtPos();
    m_stageClearStoredFovDegrees = m_render.GetCameraHorizontalFovDegrees();

    if (!m_stageClearWasFirstClear)
    {
        m_stageClearCameraEndPos = m_stageClearCameraStartPos;
        m_stageClearCameraEndTarget = m_stageClearCameraStartTarget;
        if (m_goalMarkerMeshId >= 0)
        {
            m_render.RemoveMeshMix(m_goalMarkerMeshId);
            m_goalMarkerMeshId = -1;
        }
        RemoveGoalArrow();
        if (m_playerMeshId >= 0)
        {
            SetPlayerAnimationState(PlayerAnimState::Idle, 1.0f);
        }
        m_render.SetCameraShakeDuration(0.08f);
        m_render.SetCameraShakeIntensity(0.012f);
        return;
    }

    const D3DXVECTOR3 playerPosition = m_playerMover.GetPosition();
    m_stageClearCameraEndTarget = playerPosition + D3DXVECTOR3(0.0f, 1.05f, 0.0f);
    D3DXVECTOR3 playerForward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
    if (D3DXVec3LengthSq(&playerForward) <= 0.0001f)
    {
        playerForward = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    }
    D3DXVec3Normalize(&playerForward, &playerForward);
    const D3DXVECTOR3 desiredCameraPosition = m_stageClearCameraEndTarget +
                                               playerForward * 5.0f +
                                               D3DXVECTOR3(0.0f, 1.2f, 0.0f);
    m_stageClearCameraEndPos = m_cameraMover.ResolvePosition(m_stageClearCameraEndTarget,
                                                             desiredCameraPosition);

    if (m_goalMarkerMeshId >= 0)
    {
        m_render.RemoveMeshMix(m_goalMarkerMeshId);
        m_goalMarkerMeshId = -1;
    }
    RemoveGoalArrow();

    if (m_playerMeshId >= 0)
    {
        m_playerAnimState = PlayerAnimState::Run;
        m_playerAnimationSpeed = 0.45f;
        m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, m_playerAnimationSpeed);
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerRunAnimName);
    }

    m_render.SetCameraShakeDuration(0.18f);
    m_render.SetCameraShakeIntensity(0.035f);
}

void GameApp::UpdateStageClearVisual()
{
    if (!m_stageClearWasFirstClear)
    {
        m_render.SetCamera(m_stageClearCameraStartPos, m_stageClearCameraStartTarget);
        m_render.SetCameraHorizontalFovDegrees(m_stageClearStoredFovDegrees);
        if (m_stageClearFrame == kStageClearReplayTitleFrame)
        {
            GameAudio::PlayStageSelectConfirm();
        }
        ++m_stageClearFrame;
        return;
    }

    const float rawCameraT = static_cast<float>(m_stageClearFrame + 1) /
                             static_cast<float>(kStageClearCameraMoveFrames);
    const float cameraT = SmoothStep01(rawCameraT);
    const D3DXVECTOR3 cameraPosition = LerpVector3(m_stageClearCameraStartPos,
                                                   m_stageClearCameraEndPos,
                                                   cameraT);
    const D3DXVECTOR3 cameraTarget = LerpVector3(m_stageClearCameraStartTarget,
                                                 m_stageClearCameraEndTarget,
                                                 cameraT);
    const float fovDegrees = LerpFloat(m_stageClearStoredFovDegrees,
                                       kStageClearTargetFovDegrees,
                                       cameraT);
    m_render.SetCamera(cameraPosition, cameraTarget);
    m_render.SetCameraHorizontalFovDegrees(fovDegrees);

    if (m_stageClearFrame == kStageClearIdleFrame && m_playerMeshId >= 0)
    {
        SetPlayerAnimationState(PlayerAnimState::Idle, 1.0f);
    }

    if (m_stageClearFrame == kStageClearSlashFrame && m_playerMeshId >= 0)
    {
        m_playerAnimState = PlayerAnimState::Attack;
        m_playerAnimationSpeed = 0.85f;
        m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, m_playerAnimationSpeed);
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, L"slash2");
        m_render.SetCameraShakeDuration(0.16f);
        m_render.SetCameraShakeIntensity(0.025f);
    }

    if (m_stageClearFrame == kStageClearTitleFrame)
    {
        GameAudio::PlayStageClear();
    }

    if (m_stageClearFrame == kStageClearSlashEndFrame && m_playerMeshId >= 0)
    {
        SetPlayerAnimationState(PlayerAnimState::Idle, 1.0f);
    }

    ++m_stageClearFrame;
}

void GameApp::RestoreStageClearVisual()
{
    m_render.SetCameraHorizontalFovDegrees(m_stageClearStoredFovDegrees);
    m_render.SetCamera(m_stageClearCameraStartPos, m_stageClearCameraStartTarget);
    m_render.SetCameraShakeDuration(0.0f);
    m_render.SetCameraShakeIntensity(0.0f);
    if (m_playerMeshId >= 0)
    {
        SetPlayerAnimationState(PlayerAnimState::Idle, 1.0f);
    }
    m_stageClearFrame = 0;
}

std::wstring GameApp::GetStageStoryScriptPath(const std::wstring& stageId,
                                               const std::wstring& timing) const
{
    std::vector<std::vector<std::wstring>> csvData;
    try
    {
        csvData = csv::Read(NSRender::Util::GetExeDir() + L"res\\script\\StoryEvents.csv");
    }
    catch (...)
    {
        return std::wstring();
    }

    for (const auto& row : csvData)
    {
        if (row.size() < 4 || row[0] == L"EventId")
        {
            continue;
        }
        if (row[1] == stageId && row[2] == timing)
        {
            return row[3];
        }
    }
    return std::wstring();
}

bool GameApp::IsStageClearReached()
{
    if (!m_stageManager.IsClearReached(m_playerMover.GetPosition()))
    {
        return false;
    }

    for (const auto& enemy : m_enemyManager.GetEnemies())
    {
        if (!enemy->IsDead())
        {
            return false;
        }
    }

    return true;
}

void GameApp::DamagePlayerHp(int amount)
{
    const int oldHp = m_player.GetHp();
    m_player.Damage(amount);
    const int newHp = m_player.GetHp();
    if (newHp < oldHp)
    {
        GameAudio::PlayPlayerDamage();
        D3DXVECTOR3 damageEffectPosition = m_playerMover.GetPosition();
        damageEffectPosition.y += 1.0f;
        m_render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage, damageEffectPosition);
        m_hpBar.OnDamage(oldHp, newHp);
        m_damagePopupManager.Add(oldHp - newHp, m_playerMover.GetPosition(), false);
        m_render.SetCameraShakeDuration(0.2f);
        m_render.SetCameraShakeIntensity(0.08f);
        m_render.TriggerCameraShake();
    }
}

void GameApp::HealPlayerHp(int amount)
{
    const int oldHp = m_player.GetHp();
    m_player.Heal(amount);
    const int newHp = m_player.GetHp();
    if (oldHp < newHp)
    {
        m_hpBar.OnHeal(oldHp, newHp);
        m_damagePopupManager.Add(newHp - oldHp, m_playerMover.GetPosition(), true);
    }
}

void GameApp::BeginHitStop(int frames)
{
    if (frames <= 0)
    {
        return;
    }

    if (m_pauseMenu.IsOpen() || m_craftMenu.IsOpen() || m_playerDeathPending || m_qte != nullptr)
    {
        return;
    }

    if (frames > m_pendingHitStopFrames)
    {
        m_pendingHitStopFrames = frames;
    }
}

void GameApp::StartHitStopNow(int frames)
{
    if (frames <= 0)
    {
        return;
    }

    if (frames > m_hitStopFrames)
    {
        m_hitStopFrames = frames;
    }

    if (!m_hitStopPlayerAnimationPaused && m_playerMeshId >= 0)
    {
        m_hitStopStoredPlayerAnimationSpeed = m_playerAnimationSpeed;
        m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 0.0f);
        m_hitStopPlayerAnimationPaused = true;
    }

    m_render.SetSceneUpdatePaused(true);
}

void GameApp::UpdateHitStop()
{
    if (m_hitStopFrames > 0)
    {
        --m_hitStopFrames;
    }

    if (m_hitStopFrames <= 0)
    {
        m_hitStopFrames = 0;
        if (m_hitStopPlayerAnimationPaused)
        {
            if (m_playerMeshId >= 0)
            {
                m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, m_hitStopStoredPlayerAnimationSpeed);
            }
            m_hitStopPlayerAnimationPaused = false;
        }
        m_render.SetSceneUpdatePaused(false);
    }
}

bool GameApp::IsHitStopActive() const
{
    return m_hitStopFrames > 0;
}

int GameApp::GetHitStopFrames(PlayerAttackType attackType) const
{
    if (IsWeakMeleeAttackType(attackType))
    {
        return kWeakAttackHitStopFrames;
    }

    if (IsStrongMeleeAttackType(attackType))
    {
        return kStrongAttackHitStopFrames;
    }

    return 0;
}

void GameApp::HandlePlayerDeath()
{
    if (m_playerDeathPending)
    {
        return;
    }

    GameAudio::PlayPlayerDeath();

    // カメラ移動開始位置を保存（プレイヤー位置を変更する前に行う）
    m_respawnCameraFromPos = m_render.GetCameraPos();
    m_respawnCameraFromTarget = m_render.GetLookAtPos();
    m_respawnCameraDelayFrames = kRespawnCameraDelayFrames;
    m_playerDeathPending = true;
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerKnockbackFrames = 0;
    m_hitStopFrames = 0;
    m_pendingHitStopFrames = 0;
    m_hitStopPlayerAnimationPaused = false;
    m_pickupManager.ResetPlayerEffects();
    m_playerAttackController.Reset();
    m_baseBombCapacity = 1;
    m_baseBusterRapidLevel = 1;
    m_bombCapacity = 1;
    m_busterRapidLevel = 1;
    m_busterCooldownFrames = 0;
    RefillWeaponAmmo();
    ClearBombs();
    ClearBusters();
    m_render.SetSceneUpdatePaused(true);
}

void GameApp::CompletePlayerDeath()
{
    m_player.Die();
    m_playerDeathPending = false;
    m_playerFallingDead = false;
    m_fallDeathFrames = 0;
    m_respawnCameraDelayFrames = 0;
    m_render.SetSceneUpdatePaused(false);

    if (m_player.IsGameOver())
    {
        StartGameOverSequence();
        m_respawnCameraMoveFrames = 0;
        return;
    }

    // 現在のステージ開始位置からリスポーン
    const D3DXVECTOR3 respawnPos = m_stageManager.GetCurrentStage().playerStartPosition;
    m_playerMover.Reset(respawnPos);
    m_player.ResetHp();
    m_hpBar.Reset();

    // 無敵＋点滅
    m_playerInvincibleFrames = kRespawnInvincibleFrames;
    if (m_playerMeshId >= 0)
    {
        m_render.StartMeshMixSkinAnimBlink(m_playerMeshId, kRespawnInvincibleFrames, 4);
    }

    // 敵を再配置
    m_enemyManager.LoadForStage(m_render, m_stageManager.GetCurrentStage().enemyCsvPath);

    // リスポーン位置を向いたままカメラを高速移動
    const D3DXVECTOR3 cameraTarget = respawnPos + D3DXVECTOR3(0.0f, 1.2f, 0.0f);
    const float horizontalDistance = m_cameraDistance * cosf(m_cameraPitch);
    const D3DXVECTOR3 offset(sinf(m_cameraYaw) * horizontalDistance,
                              sinf(m_cameraPitch) * m_cameraDistance,
                              -cosf(m_cameraYaw) * horizontalDistance);
    m_respawnCameraToPos = cameraTarget + offset;
    m_respawnCameraToTarget = cameraTarget;
    m_respawnCameraMoveFrames = kRespawnCameraMoveFrames;

    // 各種状態リセット
    m_playerKnockbackFrames = 0;
    m_playerAttackController.Reset();
    m_damagePopupManager.Clear();
}

void GameApp::StartGameOverSequence()
{
    if (m_qte != nullptr)
    {
        m_qte->Finalize();
        delete m_qte;
        m_qte = nullptr;
    }
    RestoreQteVisualEffectImmediate();

    m_pauseMenu.Close();
    m_craftMenu.Close();
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerKnockbackFrames = 0;
    m_playerInvincibleFrames = 0;
    m_respawnCameraDelayFrames = 0;
    m_respawnCameraMoveFrames = 0;
    m_playerAttackController.Reset();
    m_damagePopupManager.Clear();
    ClearBombs();
    ClearBusters();
    m_render.SetSceneUpdatePaused(false);
    m_render.StartFadeOut(0.3f);
    m_gameOverPhase = GameOverPhase::FadeOutToScreen;
    m_gameOverFadeFrames = kGameOverFadeFrames;
    m_gameState = GameState::GameOver;
}

void GameApp::UpdateGameOver()
{
    if (m_gameOverPhase == GameOverPhase::FadeOutToScreen)
    {
        m_render.Draw();
        --m_gameOverFadeFrames;
        if (m_gameOverFadeFrames <= 0)
        {
            m_render.StartFadeIn(0.3f);
            m_gameOverPhase = GameOverPhase::FadeInScreen;
            m_gameOverFadeFrames = kGameOverFadeFrames;
        }
        return;
    }

    DrawGameOverScreen();

    if (m_gameOverPhase == GameOverPhase::FadeInScreen)
    {
        --m_gameOverFadeFrames;
        if (m_gameOverFadeFrames <= 0)
        {
            m_gameOverPhase = GameOverPhase::WaitingInput;
        }
        return;
    }

    if (m_gameOverPhase == GameOverPhase::WaitingInput)
    {
        if (IsGameOverActionTriggered())
        {
            m_render.StartFadeOut(0.3f);
            m_gameOverPhase = GameOverPhase::FadeOutToTitle;
            m_gameOverFadeFrames = kGameOverFadeFrames;
        }
        return;
    }

    if (m_gameOverPhase == GameOverPhase::FadeOutToTitle)
    {
        --m_gameOverFadeFrames;
        if (m_gameOverFadeFrames <= 0)
        {
            ReturnToTitleFromGameOver();
        }
    }
}

void GameApp::DrawGameOverScreen()
{
    m_render.DrawImageStretched(g_gameOverImagePath, 255);
    m_render.Draw();
}

bool GameApp::IsGameOverActionTriggered() const
{
    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        return true;
    }
    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_RIGHT))
    {
        return true;
    }
    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_MIDDLE))
    {
        return true;
    }
    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_SIDE1))
    {
        return true;
    }
    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_SIDE2))
    {
        return true;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE))
    {
        return true;
    }
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
    {
        return true;
    }
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
    {
        return true;
    }

    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_A))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_B))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_X))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_Y))
    {
        return true;
    }
    if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_START))
    {
        return true;
    }

    return false;
}

void GameApp::ReturnToTitleFromGameOver()
{
    m_gameOverPhase = GameOverPhase::None;
    m_gameOverFadeFrames = 0;
    m_player.ResetLives();
    m_player.ResetHp();
    m_hpBar.Reset();
    m_enemyManager.Clear(m_render);
    m_damagePopupManager.Clear();
    m_pickupManager.ResetPlayerEffects();
    m_playerAttackController.Reset();
    m_titleDeleteConfirmMode = false;
    m_titleLanguageSelectionMode = false;
    BuildTitleMainCommands();
    RefreshTitleCommands();
    m_render.StartFadeIn(0.3f);
    m_gameState = GameState::Title;
}

bool GameApp::StartNextStage()
{
    if (!m_stageManager.MoveNextStage())
    {
        return false;
    }

    LoadCurrentStageObjects();
    if (IsCurrentStageSelect())
    {
        m_gameState = GameState::Playing;
    }
    else
    {
        m_render.SetFadeAlpha(1.0f);
        m_gameState = GameState::StageIntro;
        BeginStageIntro();
    }
    return true;
}

bool GameApp::StartStageAfterClear()
{
    return BeginStageTransitionAfterClear();
}

bool GameApp::MoveToStageAfterClear()
{
    const std::wstring clearedStageId = m_stageManager.GetCurrentStage().id;
    const int stageNumber = m_stageManager.GetCurrentStageNumber();
    const std::size_t destinationIndex = m_stageManager.GetClearDestinationIndex(stageNumber);

    if (destinationIndex >= m_stageManager.GetStageCount())
    {
        if (!m_stageManager.MoveNextStage())
        {
            return false;
        }
    }
    else
    {
        if (!m_stageManager.MoveToStage(destinationIndex))
        {
            return false;
        }
    }

    m_preferredStageSelectPortalId = L"portal-to-" + clearedStageId;
    LoadCurrentStageObjects();
    if (IsCurrentStageSelect())
    {
        m_render.StartFadeIn(kStageSelectTransitionFadeDuration);
        m_gameState = GameState::Playing;
    }
    else
    {
        m_render.SetFadeAlpha(1.0f);
        m_gameState = GameState::StageIntro;
        BeginStageIntro();
    }
    return true;
}

void GameApp::LoadCurrentStageObjects()
{
    RestoreQteVisualEffectImmediate();

    const StageManager::StageData& stage = m_stageManager.GetCurrentStage();

    std::wstring renderSettingsPath;
    if (stage.renderSettingsCsvPath.empty())
    {
        renderSettingsPath = L"res\\RenderSettings.csv";
    }
    else
    {
        renderSettingsPath = stage.renderSettingsCsvPath;
    }

    const std::wstring currentRenderQuality = m_render.GetRenderQuality();
    m_render.ReloadSettingsCsv(renderSettingsPath);
    m_render.SetRenderQuality(currentRenderQuality);
    ConfigureStagePointLights(stage.id);

    m_useFixedCamera = stage.useFixedCamera;
    m_fixedCameraPos = stage.fixedCameraPos;
    m_fixedCameraLookAt = stage.fixedCameraLookAt;
    RefillWeaponAmmo();

    if (m_qte != nullptr)
    {
        m_qte->Finalize();
        delete m_qte;
        m_qte = nullptr;
    }

    if (m_goalMarkerMeshId >= 0)
    {
        m_render.RemoveMeshMix(m_goalMarkerMeshId);
        m_goalMarkerMeshId = -1;
    }

    m_pickupManager.Clear();
    m_dashBoosterManager.Clear();
    ClearBombs();
    ClearBusters();
    RemoveGoalArrow();

    if (m_stickMeshId >= 0)
    {
        m_render.DetachMeshFromBone(m_stickMeshId);
        m_render.RemoveMeshMix(m_stickMeshId);
        m_stickMeshId = -1;
    }
    if (m_saberMeshId >= 0)
    {
        m_render.DetachMeshFromBone(m_saberMeshId);
        m_render.RemoveMeshMix(m_saberMeshId);
        m_saberMeshId = -1;
    }

    RemoveStageSelectCubes();
    m_render.ClearCsvLoadedMeshes();
    m_render.LoadXFileListFromCsv(stage.renderCsvPath);
    m_render.LoadXFileListMoveFromCsv(stage.moveCsvPath);

    if (!IsStageSelectId(stage.id))
    {
        const D3DXVECTOR3 goalPos(stage.clearPosition.x, stage.clearPosition.y - 0.5f, stage.clearPosition.z);
        m_goalMarkerMeshId = m_render.AddMeshMix(L"res\\model\\cube_red.x",
                                                 goalPos,
                                                 D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                 1.0f,
                                                 -1.0f,
                                                 false,
                                                 false,
                                                 false);
    }

    m_collectibleManager.LoadForStage(stage.collectibleCsvPath);
    m_interactionManager.LoadForStage(stage.interactableCsvPath);
    m_lavaZoneManager.LoadForStage(stage.lavaCsvPath);

    m_pickupManager.LoadForStage(stage.starCsvPath, stage.speedUpCsvPath);
    m_dashBoosterManager.LoadForStage(stage.dashBoosterCsvPath);

    CreateStageSelectCubes();
    m_playerMover.Reset(stage.playerStartPosition);
    InitializeStageSelectCursor();

    m_mouseCursorVisible = IsCurrentStageSelect();
    InputDevice::Mouse::SetVisible(m_mouseCursorVisible);

    PhysicsWorld::ClearObjects();
    LoadPhysicsObjectsFromCsv(stage.physicsCsvPath);

    m_prevMovingPlatformPositions.clear();
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerYaw = 0.0f;
    m_playerAnimState = PlayerAnimState::Idle;
    m_stageExitFrame = 0;
    m_stageExitVisualOffsetY = 0.0f;
    m_damagePopupManager.Clear();
    m_playerInvincibleFrames = 0;
    m_pickupManager.ResetTemporaryEffects();
    RestoreTemporaryPowerUps();
    m_playerKnockbackFrames = 0;
    m_playerAttackController.Reset();
    m_playerAttackController.SelectClubCategory();
    m_respawnCameraDelayFrames = 0;
    m_respawnCameraMoveFrames = 0;
    m_playerDeathPending = false;
    m_playerFallingDead = false;
    m_fallDeathFrames = 0;
    m_render.SetSceneUpdatePaused(false);
    if (m_playerMeshId >= 0)
    {
        m_render.StopMeshMixSkinAnimBlink(m_playerMeshId);
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerIdleAnimName);
    }
    m_render.ClearParticleEffect();
    m_player.ResetHp();
    m_hpBar.Reset();
    m_enemyManager.LoadForStage(m_render, stage.enemyCsvPath);

    m_destructibleManager.LoadForStage(m_render, stage.destructibleCsvPath);
    m_collectibleManager.RefreshVisibility(m_destructibleManager);

    if (m_playerMeshId >= 0)
    {
        const D3DXVECTOR3 kHiddenHeldWeaponPosition(0.0f, -100.0f, 0.0f);
        m_stickMeshId = m_render.AddMeshMix(kStickModelPath,
                                            kHiddenHeldWeaponPosition,
                                            D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                            kStickModelScale);
        if (m_stickMeshId >= 0)
        {
            m_render.SetMeshMixEnabled(m_stickMeshId, false);
            const float kStickLocalRotateZ = D3DX_PI * 0.5f;
            m_render.AttachMeshToBone(m_stickMeshId, m_playerMeshId, kPlayerLeftWristBoneName,
                                      D3DXVECTOR3(0.0f, 0.0f, kStickLocalRotateZ),
                                      D3DXVECTOR3(0.0f, 0.0f, 0.0f));
        }

        m_saberMeshId = m_render.AddMeshMix(kSaberModelPath,
                                            kHiddenHeldWeaponPosition,
                                            D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                            kSaberModelScale);
        if (m_saberMeshId >= 0)
        {
            m_render.SetMeshMixEnabled(m_saberMeshId, false);
            const float kSaberLocalRotateZ = D3DX_PI * 0.5f;
            m_render.AttachMeshToBone(m_saberMeshId, m_playerMeshId, kPlayerLeftWristBoneName,
                                      D3DXVECTOR3(0.0f, 0.0f, kSaberLocalRotateZ),
                                      D3DXVECTOR3(0.0f, 0.0f, 0.0f));
        }
    }

    UpdatePlayerMeshVisibility();
    UpdateHeldWeaponVisibility();
    if (IsCurrentStageSelect() && m_hasSelectedStagePortal)
    {
        SyncStageSelectPlayerToPortal(true);
    }
    UpdatePlayerMeshAndCamera(stage.playerStartPosition);
    PlaceStageWeather(m_render, stage.weather, stage.playerStartPosition);
}

void GameApp::DrawTitleScreen()
{
    if (m_titleFontId < 0)
    {
        m_titleFontId = m_render.SetUpFontEx(L"BIZ UDMincho", 50, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    m_render.DrawTextExCenter(m_titleFontId, L"ホ  シ  ガ  ー  ル", 0, 220, NSRender::Common::BASE_W, 100);

    if (m_titleDeleteConfirmMode)
    {
        m_render.DrawTextExCenter(m_titleFontId, L"セーブデータを削除しますか？", 0, 500, NSRender::Common::BASE_W, 100);
    }
    else if (m_titleLanguageSelectionMode)
    {
        std::wstring languageName = L"Japanese";
        if (m_titleLanguage == TitleLanguage::English)
        {
            languageName = L"English";
        }
        m_render.DrawTextExCenter(m_titleFontId, L"Language", 0, 480, NSRender::Common::BASE_W, 80);
        m_render.DrawTextExCenter(m_titleFontId, L"Current: " + languageName, 0, 560, NSRender::Common::BASE_W, 80);
    }

    m_command.Draw();
    m_render.Draw();
}

void GameApp::BeginStageIntro()
{
    m_stageIntroPhase = StageIntroPhase::LetterboxIn;
    m_stageIntroFrame = 0;
    m_stageIntroStartFadeAlpha = m_render.GetFadeAlpha();
    if (m_stageIntroFontId < 0)
    {
        m_stageIntroFontId = m_render.SetUpFontEx(L"BIZ UDGothic", 56, D3DCOLOR_RGBA(255, 255, 255, 255));
    }
}

void GameApp::UpdateStageIntro()
{
    // 現在フェーズのフレーム数
    int phaseFrames = kStageIntroLetterboxFrames;
    if (m_stageIntroPhase == StageIntroPhase::Hold)
    {
        phaseFrames = kStageIntroHoldFrames;
    }
    else if (m_stageIntroPhase == StageIntroPhase::LetterboxOut)
    {
        phaseFrames = kStageIntroOutFrames;
    }

    float t = 1.0f;
    if (phaseFrames > 0)
    {
        t = static_cast<float>(m_stageIntroFrame) / static_cast<float>(phaseFrames);
        if (t > 1.0f)
        {
            t = 1.0f;
        }
    }

    // フェーズごとのアニメ値
    float barHeight = static_cast<float>(kLetterboxBarHeight);
    float titleAlpha = 1.0f;
    float titleOffsetY = 0.0f;
    float fadeAlpha = 0.0f;
    if (m_stageIntroPhase == StageIntroPhase::LetterboxIn)
    {
        barHeight = static_cast<float>(kLetterboxBarHeight) * t;
        titleAlpha = t;
        titleOffsetY = (1.0f - t) * 20.0f;
        fadeAlpha = m_stageIntroStartFadeAlpha + (0.0f - m_stageIntroStartFadeAlpha) * t;
    }
    else if (m_stageIntroPhase == StageIntroPhase::Hold)
    {
        barHeight = static_cast<float>(kLetterboxBarHeight);
        titleAlpha = 1.0f;
        titleOffsetY = 0.0f;
        fadeAlpha = 0.0f;
    }
    else
    {
        barHeight = static_cast<float>(kLetterboxBarHeight) * (1.0f - t);
        titleAlpha = 1.0f - t;
        titleOffsetY = 0.0f;
        fadeAlpha = 0.0f;
    }

    m_render.SetFadeAlpha(fadeAlpha);

    // シーン描画用のカメラとプレイヤーメッシュを更新
    UpdatePlayerMeshAndCamera(m_playerMover.GetPosition());

    // シネマティック黒帯
    const int barH = static_cast<int>(barHeight + 0.5f);
    if (barH > 0)
    {
        m_render.DrawImageSized(kLetterboxBarImagePath,
                                0, 0,
                                NSRender::Common::BASE_W, barH, 255);
        m_render.DrawImageSized(kLetterboxBarImagePath,
                                0, NSRender::Common::BASE_H - barH,
                                NSRender::Common::BASE_W, barH, 255);
    }

    // ステージ名
    const int alpha = static_cast<int>(255.0f * titleAlpha + 0.5f);
    if (alpha > 0)
    {
        const int titleY = static_cast<int>(260.0f + titleOffsetY);
        m_render.DrawTextExCenter(m_stageIntroFontId,
                                  m_stageManager.GetCurrentStageDisplayName(),
                                  0, titleY,
                                  NSRender::Common::BASE_W, 90,
                                  D3DCOLOR_RGBA(255, 255, 255, alpha));
    }

    m_render.Draw();

    // フレーム進行とフェーズ遷移
    ++m_stageIntroFrame;
    if (m_stageIntroFrame >= phaseFrames)
    {
        m_stageIntroFrame = 0;
        if (m_stageIntroPhase == StageIntroPhase::LetterboxIn)
        {
            m_stageIntroPhase = StageIntroPhase::Hold;
        }
        else if (m_stageIntroPhase == StageIntroPhase::Hold)
        {
            m_stageIntroPhase = StageIntroPhase::LetterboxOut;
        }
        else
        {
            m_render.SetFadeAlpha(0.0f);
            m_gameState = GameState::Playing;
            m_prevMovingPlatformPositions.clear();
        }
    }
}

void GameApp::DrawStageTitle()
{
    if (m_stageTitleFrame <= 0)
    {
        return;
    }

    if (m_stageTitleFontId < 0)
    {
        m_stageTitleFontId = m_render.SetUpFont(L"BIZ UDGothic", 56, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    m_render.DrawTextCenter(m_stageTitleFontId,
                            m_stageManager.GetCurrentStageDisplayName(),
                            0,
                            260,
                            NSRender::Common::BASE_W,
                            90);
    --m_stageTitleFrame;
}

void GameApp::DrawStageClear()
{
    if (m_stageClearFontId < 0)
    {
        m_stageClearFontId = m_render.SetUpFont(L"BIZ UDGothic", 60, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    if (m_stageClearHintFontId < 0)
    {
        m_stageClearHintFontId = m_render.SetUpFont(L"BIZ UDGothic", 24, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    if (!m_stageClearWasFirstClear)
    {
        m_render.DrawImageSized(kLetterboxBarImagePath,
                                0,
                                0,
                                NSRender::Common::BASE_W,
                                NSRender::Common::BASE_H,
                                34);

        if (m_stageClearFrame < 6)
        {
            const float flashT = static_cast<float>(m_stageClearFrame) / 6.0f;
            const int flashAlpha = static_cast<int>((1.0f - flashT) * 105.0f);
            m_render.DrawImageSized(kStageClearFlashImagePath,
                                    0,
                                    0,
                                    NSRender::Common::BASE_W,
                                    NSRender::Common::BASE_H,
                                    flashAlpha);
        }

        if (m_stageClearFrame >= 2)
        {
            const float ringRawT = static_cast<float>(m_stageClearFrame - 2) / 12.0f;
            const float ringT = SmoothStep01(ringRawT);
            const int ringSize = static_cast<int>(330.0f + 150.0f * ringT);
            const int ringAlpha = static_cast<int>(118.0f * ringT);
            m_render.DrawImageSized(kStageClearRingImagePath,
                                    (NSRender::Common::BASE_W - ringSize) / 2,
                                    120 + (480 - ringSize) / 2,
                                    ringSize,
                                    ringSize,
                                    ringAlpha);
        }

        if (m_stageClearFrame >= 4)
        {
            const float sparklesRawT = static_cast<float>(m_stageClearFrame - 4) / 14.0f;
            const float sparklesT = SmoothStep01(sparklesRawT);
            const int sparklesAlpha = static_cast<int>(86.0f * sparklesT);
            m_render.DrawImageSized(kStageClearSparklesImagePath,
                                    430,
                                    105,
                                    740,
                                    520,
                                    sparklesAlpha);
        }

        if (m_stageClearFrame >= kStageClearReplayTitleFrame)
        {
            const float titleRawT = static_cast<float>(m_stageClearFrame - kStageClearReplayTitleFrame) / 10.0f;
            const float titleT = SmoothStep01(titleRawT);
            const int frameWidth = static_cast<int>(860.0f + 80.0f * titleT);
            const int frameHeight = static_cast<int>(300.0f + 30.0f * titleT);
            const int frameAlpha = static_cast<int>(235.0f * titleT);
            m_render.DrawImageSized(kStageClearFrameImagePath,
                                    (NSRender::Common::BASE_W - frameWidth) / 2,
                                    395 + (330 - frameHeight) / 2,
                                    frameWidth,
                                    frameHeight,
                                    frameAlpha);

            std::wstring clearText = L"STAGE CLEAR";
            if (m_stageManager.GetCurrentStage().id == L"4-8")
            {
                clearText = L"ALL CLEAR";
            }
            m_render.DrawTextCenter(m_stageClearFontId,
                                    clearText,
                                    0,
                                    500,
                                    NSRender::Common::BASE_W,
                                    76,
                                    D3DCOLOR_RGBA(255, 245, 205, frameAlpha));
            m_render.DrawTextCenter(m_stageClearHintFontId,
                                    m_stageManager.GetCurrentStageDisplayName(),
                                    0,
                                    570,
                                    NSRender::Common::BASE_W,
                                    38,
                                    D3DCOLOR_RGBA(205, 235, 255, frameAlpha));
        }

        if (m_stageClearFrame >= kStageClearReplayInputFrame)
        {
            std::wstring promptText = L"Space / Enter / A でステージセレクトへ";
            if (m_stageManager.GetCurrentStage().id == L"4-8")
            {
                promptText = L"Space / Enter / A でエンディングへ";
            }
            m_render.DrawTextCenter(m_stageClearHintFontId,
                                    promptText,
                                    0,
                                    735,
                                    NSRender::Common::BASE_W,
                                    42,
                                    D3DCOLOR_RGBA(255, 255, 255, 220));
        }

        const float letterboxRawT = static_cast<float>(m_stageClearFrame) / 10.0f;
        const float letterboxT = SmoothStep01(letterboxRawT);
        const int letterboxHeight = static_cast<int>(static_cast<float>(kStageClearReplayLetterboxHeight) * letterboxT);
        if (letterboxHeight > 0)
        {
            m_render.DrawImageSized(kLetterboxBarImagePath,
                                    0,
                                    0,
                                    NSRender::Common::BASE_W,
                                    letterboxHeight,
                                    255);
            m_render.DrawImageSized(kLetterboxBarImagePath,
                                    0,
                                    NSRender::Common::BASE_H - letterboxHeight,
                                    NSRender::Common::BASE_W,
                                    letterboxHeight,
                                    255);
        }
        return;
    }

    m_render.DrawImageSized(kLetterboxBarImagePath,
                            0,
                            0,
                            NSRender::Common::BASE_W,
                            NSRender::Common::BASE_H,
                            58);

    if (m_stageClearFrame < 14)
    {
        const float flashT = static_cast<float>(m_stageClearFrame) / 14.0f;
        const int flashAlpha = static_cast<int>((1.0f - flashT) * 210.0f);
        m_render.DrawImageSized(kStageClearFlashImagePath,
                                0,
                                0,
                                NSRender::Common::BASE_W,
                                NSRender::Common::BASE_H,
                                flashAlpha);
    }

    if (m_stageClearFrame >= 12)
    {
        const float ringRawT = static_cast<float>(m_stageClearFrame - 12) / 38.0f;
        const float ringT = SmoothStep01(ringRawT);
        const int ringSize = static_cast<int>(300.0f + 340.0f * ringT);
        const int ringAlpha = static_cast<int>(210.0f * ringT);
        m_render.DrawImageSized(kStageClearRingImagePath,
                                (NSRender::Common::BASE_W - ringSize) / 2,
                                72 + (640 - ringSize) / 2,
                                ringSize,
                                ringSize,
                                ringAlpha);
    }

    if (m_stageClearFrame >= 26)
    {
        const float sparklesRawT = static_cast<float>(m_stageClearFrame - 26) / 44.0f;
        const float sparklesT = SmoothStep01(sparklesRawT);
        const int sparklesWidth = static_cast<int>(760.0f + 300.0f * sparklesT);
        const int sparklesHeight = static_cast<int>(570.0f + 170.0f * sparklesT);
        const int sparklesAlpha = static_cast<int>(190.0f * sparklesT);
        m_render.DrawImageSized(kStageClearSparklesImagePath,
                                (NSRender::Common::BASE_W - sparklesWidth) / 2,
                                40,
                                sparklesWidth,
                                sparklesHeight,
                                sparklesAlpha);
    }

    if (m_stageClearFrame >= kStageClearTitleFrame)
    {
        const float titleRawT = static_cast<float>(m_stageClearFrame - kStageClearTitleFrame) / 22.0f;
        const float titleT = SmoothStep01(titleRawT);
        const int frameWidth = static_cast<int>(880.0f + 220.0f * titleT);
        const int frameHeight = static_cast<int>(340.0f + 80.0f * titleT);
        const int frameAlpha = static_cast<int>(255.0f * titleT);
        m_render.DrawImageSized(kStageClearFrameImagePath,
                                (NSRender::Common::BASE_W - frameWidth) / 2,
                                360 + (420 - frameHeight) / 2,
                                frameWidth,
                                frameHeight,
                                frameAlpha);

        std::wstring clearText = L"STAGE CLEAR";
        if (m_stageManager.GetCurrentStage().id == L"4-8")
        {
            clearText = L"ALL CLEAR";
        }
        m_render.DrawTextCenter(m_stageClearFontId,
                                clearText,
                                0,
                                510,
                                NSRender::Common::BASE_W,
                                82,
                                D3DCOLOR_RGBA(255, 245, 205, frameAlpha));
        m_render.DrawTextCenter(m_stageClearHintFontId,
                                m_stageManager.GetCurrentStageDisplayName(),
                                0,
                                592,
                                NSRender::Common::BASE_W,
                                42,
                                D3DCOLOR_RGBA(205, 235, 255, frameAlpha));

        if (m_stageClearWasFirstClear)
        {
            m_render.DrawTextCenter(m_stageClearHintFontId,
                                    L"NEW CLEAR",
                                    0,
                                    458,
                                    NSRender::Common::BASE_W,
                                    40,
                                    D3DCOLOR_RGBA(255, 210, 90, frameAlpha));
        }
    }

    if (m_stageClearFrame >= kStageClearInputFrame)
    {
        std::wstring promptText = L"Space / Enter / A でステージセレクトへ";
        if (m_stageManager.GetCurrentStage().id == L"4-8")
        {
            promptText = L"Space / Enter / A でエンディングへ";
        }
        m_render.DrawTextCenter(m_stageClearHintFontId,
                                promptText,
                                0,
                                735,
                                NSRender::Common::BASE_W,
                                42,
                                D3DCOLOR_RGBA(255, 255, 255, 225));
    }

    const float letterboxRawT = static_cast<float>(m_stageClearFrame) / 28.0f;
    const float letterboxT = SmoothStep01(letterboxRawT);
    const int letterboxHeight = static_cast<int>(static_cast<float>(kStageClearLetterboxHeight) * letterboxT);
    if (letterboxHeight > 0)
    {
        m_render.DrawImageSized(kLetterboxBarImagePath,
                                0,
                                0,
                                NSRender::Common::BASE_W,
                                letterboxHeight,
                                255);
        m_render.DrawImageSized(kLetterboxBarImagePath,
                                0,
                                NSRender::Common::BASE_H - letterboxHeight,
                                NSRender::Common::BASE_W,
                                letterboxHeight,
                                255);
    }
}

void GameApp::DrawEndingFin()
{
    m_render.DrawImageStretched(g_finImagePath, 255);
    m_render.Draw();
}

POINT GameApp::ConvertMouseToBaseResolution(int clientX, int clientY)
{
    RECT clientRect;
    GetClientRect(m_render.GetWindowHandle(), &clientRect);

    const int clientW = clientRect.right - clientRect.left;
    const int clientH = clientRect.bottom - clientRect.top;

    POINT result;
    if (clientW <= 0 || clientH <= 0)
    {
        result.x = clientX;
        result.y = clientY;
        return result;
    }

    result.x = static_cast<int>(static_cast<float>(clientX) * static_cast<float>(NSRender::Common::BASE_W) / static_cast<float>(clientW));
    result.y = static_cast<int>(static_cast<float>(clientY) * static_cast<float>(NSRender::Common::BASE_H) / static_cast<float>(clientH));
    return result;
}

bool GameApp::PlaceBomb(const D3DXVECTOR3& position)
{
    if (static_cast<int>(m_activeBombs.size()) >= m_bombCapacity)
    {
        return false;
    }

    ActiveBomb bomb;
    bomb.position = position;
    bomb.remainingFrames = kBombFrames;
    bomb.meshId = m_render.AddMeshMix(kBombModelPath,
                                      position,
                                      D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                      1.0f,
                                      -1.0f,
                                      false,
                                      false,
                                      false);
    m_activeBombs.push_back(bomb);
    GameAudio::PlayBombPlace();
    return true;
}

void GameApp::UpdateBombPhysics(ActiveBomb& bomb)
{
    if (bomb.isGrounded)
    {
        return;
    }

    bomb.velocity.y -= kBombGravity * kTargetFrameSeconds;

    const D3DXVECTOR3 collisionPosition =
        bomb.position + D3DXVECTOR3(0.0f, kBombCollisionCenterY, 0.0f);
    D3DXVECTOR3 nextCollisionPosition = collisionPosition;
    D3DXVECTOR3 nextVelocity = bomb.velocity;
    D3DXVECTOR3 hitNormal(0.0f, 0.0f, 0.0f);

    const bool collided = PhysicsWorld::CheckCollide(collisionPosition,
                                                     bomb.velocity,
                                                     PhysicsWorld::ShapeType::Sphere,
                                                     &nextCollisionPosition,
                                                     &nextVelocity,
                                                     nullptr,
                                                     nullptr,
                                                     kBombRadius,
                                                     0.0f,
                                                     nullptr,
                                                     &hitNormal);

    bomb.position = nextCollisionPosition - D3DXVECTOR3(0.0f, kBombCollisionCenterY, 0.0f);
    bomb.velocity = nextVelocity;

    if (collided && hitNormal.y > 0.0f)
    {
        bomb.isGrounded = true;
        bomb.velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    }

    if (bomb.meshId >= 0)
    {
        m_render.SetMeshMixPos(bomb.meshId, bomb.position);
    }
}

void GameApp::UpdateBombs()
{
    for (auto it = m_activeBombs.begin(); it != m_activeBombs.end(); )
    {
        UpdateBombPhysics(*it);
        --it->remainingFrames;
        if (it->remainingFrames <= 0)
        {
            const D3DXVECTOR3 bombPos = it->position;

            if (it->meshId >= 0)
            {
                m_render.SetMeshMixDamageFlash(it->meshId, false);
                m_render.RemoveMeshMix(it->meshId);
            }

            m_render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion, bombPos);
            GameAudio::PlayExplosion();

            for (auto& enemy : m_enemyManager.GetEnemies())
            {
                if (enemy->IsDead())
                {
                    continue;
                }

                D3DXVECTOR3 dir = GetEnemyAttackTargetPosition(*enemy) - bombPos;
                const float dist = D3DXVec3Length(&dir);
                if (dist <= kBombExplosionRadius)
                {
                    enemy->TakeDamageWithoutFacing(m_render, kBombExplosionDamage);
                    enemy->StartKnockbackFrom(bombPos, 0.5f, 30);
                    m_damagePopupManager.Add(kBombExplosionDamage, enemy->GetPosition(), false);
                    TryDropEnemyItem(*enemy);
                }
            }

            for (const auto& destructible : m_destructibleManager.GetObjects())
            {
                if (destructible.isDead || destructible.hp <= 0)
                {
                    continue;
                }

                D3DXVECTOR3 dir = destructible.position - bombPos;
                const float dist = D3DXVec3Length(&dir);
                if (dist <= kBombExplosionRadius)
                {
                    if (m_destructibleManager.TryDamage(m_render, destructible, kBombExplosionDamage))
                    {
                        m_damagePopupManager.Add(kBombExplosionDamage, destructible.position, false);
                    }
                }
            }

            for (const auto& destructible : m_destructibleManager.GetObjects())
            {
                if (destructible.isDead || destructible.hp <= 0)
                {
                    continue;
                }

                D3DXVECTOR3 dir = destructible.position - bombPos;
                const float dist = D3DXVec3Length(&dir);
                if (dist <= kBombExplosionRadius)
                {
                    if (m_destructibleManager.TryDamage(m_render, destructible, kBombExplosionDamage))
                    {
                        m_damagePopupManager.Add(kBombExplosionDamage, destructible.position, false);
                    }
                }
            }

            const D3DXVECTOR3 playerDir = m_playerMover.GetPosition() - bombPos;
            const float playerDist = D3DXVec3Length(&playerDir);
            if (playerDist <= kBombExplosionRadius)
            {
                DamagePlayerHp(kBombExplosionDamage);
                D3DXVECTOR3 playerKnockbackDir(playerDir.x, 0.0f, playerDir.z);
                if (D3DXVec3LengthSq(&playerKnockbackDir) > 0.0001f)
                {
                    D3DXVec3Normalize(&playerKnockbackDir, &playerKnockbackDir);
                }
                else
                {
                    playerKnockbackDir = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
                }
                m_playerKnockbackFrames = kBombKnockbackFrames;
                m_playerKnockbackDir = playerKnockbackDir;
            }

            it = m_activeBombs.erase(it);
        }
        else
        {
            if (it->remainingFrames <= kBombBlinkStartFrames)
            {
                ++it->blinkTimer;
                const int phase = it->blinkTimer % (kBombBlinkInterval * 2);
                if (phase < kBombBlinkInterval)
                {
                    m_render.SetMeshMixDamageFlash(it->meshId, true);
                }
                else
                {
                    m_render.SetMeshMixDamageFlash(it->meshId, false);
                }
            }
            ++it;
        }
    }
}

void GameApp::ClearBombs()
{
    for (ActiveBomb& bomb : m_activeBombs)
    {
        if (bomb.meshId >= 0)
        {
            m_render.SetMeshMixDamageFlash(bomb.meshId, false);
            m_render.RemoveMeshMix(bomb.meshId);
        }
    }
    m_activeBombs.clear();
}

void GameApp::SpawnBuster(const D3DXVECTOR3& position, const D3DXVECTOR3& direction)
{
    ActiveBuster buster;
    buster.position = position;
    buster.direction = direction;
    buster.traveledDistance = 0.0f;
    buster.meshId = m_render.AddMeshMix(kBusterModelPath,
                                         position,
                                         D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                         kBusterScale,
                                         -1.f,
                                         false,
                                         false,
                                         false);
    m_activeBusters.push_back(buster);
}

void GameApp::UpdateBusters()
{
    for (auto it = m_activeBusters.begin(); it != m_activeBusters.end(); )
    {
        const float moveStep = kBusterSpeed * kTargetFrameSeconds;
        it->position = it->position + it->direction * moveStep;
        it->traveledDistance += moveStep;

        if (it->meshId >= 0)
        {
            m_render.SetMeshMixPos(it->meshId, it->position);
        }

        bool destroyed = false;

        if (it->traveledDistance >= kBusterMaxDistance)
        {
            destroyed = true;
        }
        else
        {
            for (auto& enemy : m_enemyManager.GetEnemies())
            {
                if (enemy->IsDead())
                {
                    continue;
                }

                D3DXVECTOR3 dir = GetEnemyAttackTargetPosition(*enemy) - it->position;
                const float dist = D3DXVec3Length(&dir);
                if (dist <= kBusterHitRadius)
                {
                    enemy->TakeDamage(m_render, kBusterDamage, it->position);
                    enemy->StartKnockbackFrom(it->position, 0.3f, 20);
                    m_damagePopupManager.Add(kBusterDamage, enemy->GetPosition(), false);
                    TryDropEnemyItem(*enemy);
                    GameAudio::PlayAttackHit();
                    destroyed = true;
                    break;
                }
            }

            if (!destroyed)
            {
                for (const auto& destructible : m_destructibleManager.GetObjects())
                {
                    if (destructible.isDead || destructible.hp <= 0)
                    {
                        continue;
                    }

                    D3DXVECTOR3 dir = destructible.position - it->position;
                    const float dist = D3DXVec3Length(&dir);
                    if (dist <= kBusterHitRadius + kDestructibleHitRadius)
                    {
                        if (m_destructibleManager.TryDamage(m_render, destructible, kBusterDamage))
                        {
                            m_damagePopupManager.Add(kBusterDamage, destructible.position, false);
                            GameAudio::PlayAttackHit();
                            destroyed = true;
                            break;
                        }
                    }
                } 
            }
        }

        if (destroyed)
        {
            if (it->meshId >= 0)
            {
                m_render.RemoveMeshMix(it->meshId);
            }
            it = m_activeBusters.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void GameApp::ClearBusters()
{
    for (ActiveBuster& buster : m_activeBusters)
    {
        if (buster.meshId >= 0)
        {
            m_render.RemoveMeshMix(buster.meshId);
        }
    }
    m_activeBusters.clear();
}



