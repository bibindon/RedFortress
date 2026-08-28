#include "GameApp.h"

#include "resource.h"
#include "GameAudio.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <random>
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

    std::wstring WidenDebugIdentifier(const std::string& identifier)
    {
        std::wstring result;
        result.reserve(identifier.size());
        for (const unsigned char value : identifier)
        {
            if (value > 0x7f)
            {
                throw std::runtime_error("Debug RPC identifiers must contain ASCII characters only.");
            }
            result.push_back(static_cast<wchar_t>(value));
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

    const std::wstring g_playerMeshPath = L"res\\model2\\marine_512_low\\marine.x";
    const std::wstring g_playerAnimCsvPath = L"res\\model2\\marine_512_low\\marine.csv";
    const std::wstring g_stageSelectPlayerMeshPath =
        L"res\\model2\\marine_512_low\\marine.x";
    const std::wstring g_stageSelectPlayerAnimCsvPath =
        L"res\\model2\\marine_512_low\\marine.csv";
    const std::wstring g_playerIdleAnimName = L"000";
    const std::wstring g_playerWalkAnimName = L"walk";
    const std::wstring g_playerRunAnimName = L"run";
    const std::wstring g_playerJumpAnimName = L"jump";
    const std::wstring g_playerDeathAnimName = L"death";
    const std::wstring g_finImagePath = L"res\\2D_Image\\fin.png";
    const std::wstring g_gameOverImagePath = L"res\\2D_Image\\gameover.png";
    const std::wstring g_gameOverOverlayImagePath = L"res\\2D_Image\\black2x2.bmp";
    const float kPlayerWalkAnimationSpeed = 1.3f;
    const float kTitleSaturationLevel = 0.85f;
    const float kTitleShadowDarkness = 0.75f;
    const std::wstring kPortalStepsModelPath = L"res\\model\\portal\\stone_steps.x";
    const std::wstring kPortalStepsCollisionPath = L"res\\model\\portal\\stone_steps_collision.x";
    const std::wstring kPortalPillarModelPath = L"res\\model\\portal\\light_pillar.x";
    const std::wstring kPortalFlagModelPath = L"res\\model\\portal\\black_flag.x";
    const std::wstring kPortalFlagAnimCsvPath = L"res\\model\\portal\\black_flag.csv";
    const float kPortalStepsScale = 2.0f;
    const float kPortalStepsPositionYOffset = -1.0f;
    const int kPortalClearDelayFrames = 150;
    const float kPortalPillarTouchRadius = 0.9f;
    const float kPortalPillarLightHeight = 1.2f;
    const float kPortalPillarLightBrightness = 0.8f;
    const float kPortalPillarLightRange = 4.0f;
    const std::wstring kPortalPillarLightOwnerTag = L"stage-goal-pillar";
    const float kTitleSunLightIntensity = 0.45f;
    const float kTitleAmbientLightIntensity = 0.14f;
    const float kStagePortalClickRadius = 48.0f;
    const float kStageSelectPlayerMoveDuration = 0.5f;
    const float kStageSelectTransitionFadeDuration = 0.5f;
    const float kStageSelectPlayerRightYaw = -D3DX_PI * 0.5f;
    const float kStageSelectPlayerLeftYaw = D3DX_PI * 0.5f;
    const float kStageSelectPlayerVisualOffsetY = 1.0f;
    const float kStageSelect2PlayerVisualOffsetY = -1.0f;
    const float kStageSelect2PlayerLightOffsetY = -3.0f;
    const float kStageSelectPlayerVisualScale = 1.0f;
    const float kStageSelectPlayerLightHeight = 3.2f;
    const wchar_t* kStageSelectPlayerLightOwnerTag = L"stage-select-player";
    const float kStageSelectCursorLightHeight = 0.4f;
    const float kStageSelectCursorLightBrightness = 0.6f;
    const float kStageSelectCursorLightRange = 2.0f;
    const wchar_t* kStageSelectCursorLightOwnerTag = L"stage-select-cursor";
    const float kPlayerPointLightHeight = 2.2f;
    const float kPlayerPointLightBrightness = 2.5f;
    const float kPlayerPointLightRange = 12.0f;
    const wchar_t* kPlayerPointLightOwnerTag = L"stage-player";
    const int kStageSelectStageNameX = 48;
    const int kStageSelectStageNameY = 42;
    const float kStageSelectStageNameFadeOutDuration = 0.15f;
    const float kStageSelectStageNameFadeInDuration = 0.20f;
    const int kStageSelectHintX = 900;
    const int kStageSelectHintWidth = 650;
    const int kStageSelectHintFirstLineY = 812;
    const int kStageSelectHintSecondLineY = 844;
    const int kStageSelectHintLineHeight = 22;
    const int kStageSelectStartButtonX = 650;
    const int kStageSelectStartButtonY = 790;
    const int kStageSelectStartButtonWidth = 300;
    const int kStageSelectStartButtonHeight = 54;
    const int kStageSelectStartHitX = 620;
    const int kStageSelectStartHitY = 765;
    const int kStageSelectStartHitWidth = 360;
    const int kStageSelectStartHitHeight = 95;
    const std::wstring kStageSelectStartMaskPath = L"res\\2D_Image\\stage_select_start_mask.png";
    const int kStageSelectMaskedGaussianSampleSize = 25;
    const std::wstring kStageSelectCubeRedPath = L"res\\model\\cube_red.x";
    const std::wstring kStageSelectCubeGreenPath = L"res\\model\\cubeGreen\\cube_green.x";
    const std::wstring kStageSelectCubeBluePath = L"res\\model\\cubeBlue\\cube_blue.x";
    const float kStageSelectCubeScale = 0.16666667f;
    const float kStageSelectCubeVisualOffsetY = 1.0f;
    const float kStageSelect2CubeVisualOffsetY = -1.3f;
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

    const std::wstring kCraftMaterialItemIds[] =
    {
        L"001", L"002", L"003", L"004", L"005", L"006", L"009",
        L"010", L"011", L"012", L"013", L"014", L"015", L"016"
    };
    const int kCraftMaterialItemCount =
        static_cast<int>(sizeof(kCraftMaterialItemIds) / sizeof(kCraftMaterialItemIds[0]));

    std::wstring GetRandomCraftMaterialItemId()
    {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> dist(0, kCraftMaterialItemCount - 1);
        return kCraftMaterialItemIds[dist(rng)];
    }

    const std::wstring kStickModelPath = L"res\\model\\stick\\stick.x";
    const std::wstring kSaberModelPath = L"res\\model\\piratekit\\cutlass.x";
    const std::wstring kGunModelPath = L"res\\model\\piratekit\\pistol.x";
    const char* kPlayerRightWristBoneName = "Bone_242";
    const float kStickModelScale = 0.5f;
    const float kSaberModelScale = 0.9f;
    const float kGunModelScale = 0.55f;
    const std::wstring kBombCapacityUpItemId = L"bomb_capacity_up";
    const std::wstring kBusterRapidUpItemId = L"buster_rapid_up";
    const std::wstring kStarPowerUpItemId = L"star_power_up";
    const std::wstring kSpeedUpItemId = L"speed_up";
    const std::wstring kInitialClubWeaponId = L"W001";
    const std::wstring kSwordWeaponId = L"W002";
    const std::wstring kBusterWeaponId = L"W003";
    const std::wstring kBombWeaponId = L"W004";
    const std::wstring kRedSpaghettiItemId = L"007";
    const std::wstring kPotatoChipsItemId = L"008";
    const std::wstring kChuageJuiceItemId = L"017";
    const int kItemPickupMessageTotalFrames = 180;
    const int kItemPickupMessageFadeFrames = 24;
    const int kItemPickupMessageY = 780;
    const int kItemPickupMessageFontSize = 20;
    const int kItemPickupMessageHeight = 30;

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
    const int kRespawnFadeOutFrames = 30;
    const int kPlayerDeathMotionFrames = 72;
    const int kRespawnBlackHoldFrames = 12;
    const int kRespawnFadeInFrames = 24;
    const int kWarpFadeOutFrames = 15;
    const int kWarpBlackHoldFrames = 6;
    const int kWarpFadeInFrames = 15;
    const float kWarpFadeDurationSeconds = 0.25f;
    const int kStageTitleFrameMax = 180;
    const int kGameOverWaitFrames = 180;
    const int kGameOverFadeFrames = 18;
    const int kGameOverOverlayBaseAlpha = 128;
    const int kGameOverOverlayPulseAlpha = 14;
    const int kGameOverOverlayPulseFrames = 120;
    const float kFallDeathY = -10.0f;
    const int kFallDeathFadeDelayFrames = 60;
    const float kEnemyAttackKnockbackDistance = 0.2f;
    const int kEnemyAttackKnockbackFrames = 60;
    const std::wstring kGoalArrowModelPath = L"res\\model\\arrow\\arrow.x";
    const float kGoalArrowHeadOffsetY = 2.3f;
    const float kGoalArrowScale = 0.42f;
    const int kStageIntroLetterboxFrames = 25;
    const int kStageIntroHoldFrames = 80;
    const int kStageIntroOutFrames = 25;
    const int kStageIntroZoomTotalFrames =
        kStageIntroLetterboxFrames + kStageIntroHoldFrames + kStageIntroOutFrames;
    const float kStageIntroZoomStartScale = 2.5f;
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
    const int kStageClearFinalAutoFrame = 240;
    const int kStageClearLetterboxHeight = 90;
    const int kStageClearReplayJumpDelayFrames = 60;
    const int kStageClearReplayAscentFrames = 40;
    const int kStageClearReplayWhiteFrames = 6;
    const int kStageClearReplayVanishedFrames = 120;
    const int kStageClearReplayFinalAutoFrame = kStageClearReplayJumpDelayFrames +
                                                kStageClearReplayAscentFrames +
                                                kStageClearReplayWhiteFrames +
                                                kStageClearReplayVanishedFrames;
    const float kStageClearReplayJumpHeight = 2.0f;
    const float kStageClearReplayJumpAnimationSpeed = 1.0f;
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
    const int kBossDefeatDurationFrames = 120;
    const int kBossDefeatCameraMoveFrames = 42;
    const int kBossDefeatBgmFadeFrames = 36;
    const int kBossDefeatSoundFrame = 42;
    const int kBossDefeatFogRefreshFrame = 28;
    const float kBossDefeatTargetFovDegrees = 64.0f;
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
    const int kBusterAimHoldFrames = 30;
    const int kBusterLowerFrames = 8;
    const int kAmmoGaugeX = 130;
    const int kAmmoGaugeY = 78;
    const int kAmmoRailHeight = 5;
    const int kAmmoRailOffsetY = 11;
    const int kAmmoBeadSize = 14;
    const int kAmmoBeadStep = 15;
    const int kWeakAttackHitStopFrames = 8;
    const int kStrongAttackHitStopFrames = 8;

    bool IsStageSelectId(const std::wstring& stageId)
    {
        return stageId.length() >= 6 && stageId.substr(0, 6) == L"select";
    }

    bool IsBaseId(const std::wstring& stageId)
    {
        if (stageId == L"base")
        {
            return true;
        }
        if (stageId == L"base2" || stageId == L"base3" || stageId == L"base4")
        {
            return true;
        }
        return false;
    }

    int GetWorldFromStageId(const std::wstring& stageId)
    {
        if (stageId.length() >= 2 && stageId.at(1) == L'-')
        {
            if (stageId.at(0) >= L'1' && stageId.at(0) <= L'4')
            {
                return static_cast<int>(stageId.at(0) - L'0');
            }
        }
        if (stageId.length() == 7 && IsStageSelectId(stageId))
        {
            if (stageId.at(6) >= L'1' && stageId.at(6) <= L'4')
            {
                return static_cast<int>(stageId.at(6) - L'0');
            }
        }
        if (stageId == L"base")
        {
            return 1;
        }
        if (stageId.length() == 5 && stageId.substr(0, 4) == L"base")
        {
            if (stageId.at(4) >= L'2' && stageId.at(4) <= L'4')
            {
                return static_cast<int>(stageId.at(4) - L'0');
            }
        }
        return 0;
    }

    float CalculatePlayerStartYaw(const StageManager::StageData& stage)
    {
        if (IsStageSelectId(stage.id))
        {
            return 0.0f;
        }

        const D3DXVECTOR3 direction = stage.clearPosition - stage.playerStartPosition;
        const float horizontalLengthSquared = direction.x * direction.x + direction.z * direction.z;
        if (horizontalLengthSquared <= 0.0001f)
        {
            return 0.0f;
        }

        return atan2f(-direction.x, -direction.z);
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

    bool IsSwordAttackType(const PlayerAttackType attackType)
    {
        if (attackType == PlayerAttackType::SwordAttack)
        {
            return true;
        }

        return attackType == PlayerAttackType::SwordStrongAttack;
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
    const std::wstring cursorDirectory = NSRender::Util::GetExeDir() +
                                         L"res\\2D_Image\\";
    const int cursorSize = 32;
    m_hCursor = static_cast<HCURSOR>(LoadImageW(NULL,
                                                (cursorDirectory + L"marine_cursor.cur").c_str(),
                                                IMAGE_CURSOR,
                                                cursorSize,
                                                cursorSize,
                                                LR_LOADFROMFILE));
    m_hPressedCursor = static_cast<HCURSOR>(LoadImageW(NULL,
                                                       (cursorDirectory + L"marine_cursor_pressed.cur").c_str(),
                                                       IMAGE_CURSOR,
                                                       cursorSize,
                                                       cursorSize,
                                                       LR_LOADFROMFILE));
    m_hLoadingCursor = static_cast<HCURSOR>(LoadImageW(NULL,
                                                       (cursorDirectory + L"marine_cursor_loading.ani").c_str(),
                                                       IMAGE_CURSOR,
                                                       cursorSize,
                                                       cursorSize,
                                                       LR_LOADFROMFILE));
    if (m_hCursor == NULL || m_hPressedCursor == NULL || m_hLoadingCursor == NULL)
    {
        throw std::runtime_error("Failed to load the game mouse cursors.");
    }
    wc.hCursor = m_hLoadingCursor;
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
    GameAudio::Initialize();
    GameAudio::PlayLoadingEnvironment();

#if defined(REDFORTRESS_DISABLE_SETTINGS_DIALOG)
    m_render.SetShowFPS(false);
#else
    m_render.SetShowFPS(true);
#endif
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
    LoadPlayerMeshForStage(IsStageSelectId(initialStage.id), initialStage.playerStartPosition);
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
    m_enemyManager.LoadForStage(m_render, GetEnemyCsvPathForStage(initialStage));

    m_skullManager.Initialize(m_render);
    m_skullManager.LoadForStage(m_render, initialStage.skullCsvPath);
    m_pressurePlateManager.Initialize(m_render);
    m_pressurePlateManager.LoadForStage(m_render, initialStage.pressurePlateCsvPath);
    m_pushableBoxManager.Initialize(m_render);
    m_pushableBoxManager.LoadForStage(m_render, initialStage.pushableBoxCsvPath);
    m_attackTriggerManager.Initialize();
    m_attackTriggerManager.LoadForStage(m_render, initialStage.attackTriggerCsvPath);
    m_warpBearManager.LoadForStage(initialStage.warpBearCsvPath);

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

    if (!IsStageSelectId(initialStage.id) &&
        !IsBaseId(initialStage.id) &&
        ShouldUseGoalPortal())
    {
        InitializePortal(initialStage.clearPosition);
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
    LoadStageSelectNavigation(initialStage.stageSelectNavigationCsvPath);
    m_lavaZoneManager.LoadForStage(initialStage.lavaCsvPath);
    m_lavaFloodManager.Initialize(m_render);
    m_lavaFloodManager.LoadForStage(m_render, initialStage.lavaFloodCsvPath);
    m_lavaRiseManager.Initialize(m_render);
    m_lavaRiseManager.LoadForStage(m_render, initialStage.lavaRiseCsvPath);
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
    m_bossHpBar.Initialize(&m_render);
    m_damagePopupManager.Initialize(&m_render);
    m_damagePopupManager.SetEnabled(false);

    m_saveDataManager.Initialize(m_stageManager);
    m_explanationManager.Initialize(m_render, m_saveDataManager);
    m_explanationManager.LoadForStage(initialStage.id, initialStage.explanationCsvPath);
    m_saveDataManager.ResetToDefaults();
    InitializeStageSelectCursor();
    CreateStageSelectCubes();
    UpdatePlayerMeshAndCamera(m_playerMover.GetPosition());
    m_mouseCursorVisible = true;
    InputDevice::Mouse::SetVisible(m_mouseCursorVisible);
    ApplyMouseCursor();

    m_command.UpsertCommand(L"start", true);
    m_command.UpsertCommand(L"continue", m_saveDataManager.HasSaveFile());
    m_command.UpsertCommand(L"delete", m_saveDataManager.HasSaveFile());
    m_command.UpsertCommand(L"language", true);
    m_command.UpsertCommand(L"exit", true);
    m_render.SetLoadingScreenProgress(85);
    m_render.Draw();

    m_render.PreloadImage(kLetterboxBarImagePath);
    m_render.PreloadImage(kStageClearFlashImagePath);
    m_render.PreloadImage(kStageClearRingImagePath);
    m_render.PreloadImage(kStageClearSparklesImagePath);
    m_render.PreloadImage(kStageClearFrameImagePath);
    m_render.SetLoadingScreenProgress(95);
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

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
        const ULONGLONG inputStartTick = GetTickCount64();
#endif
        InputDevice::Update();
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
        if (m_debugProfileStartTick != 0)
        {
            m_debugInputAccumulatedMilliseconds +=
                static_cast<double>(GetTickCount64() - inputStartTick);
        }
#endif

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
        ProcessDebugRpc();
#endif

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
        const ULONGLONG audioStartTick = GetTickCount64();
#endif
        const D3DXVECTOR3 audioPlayerPosition = m_playerMover.GetPosition();
        const D3DXVECTOR3 audioListenerForward = GetCameraPlanarForward();
        SoundLib::Vector3 listenerPosition { audioPlayerPosition.x, audioPlayerPosition.y, audioPlayerPosition.z };
        SoundLib::Vector3 listenerFront { audioListenerForward.x, audioListenerForward.y, audioListenerForward.z };
        SoundLib::Vector3 listenerTop { 0.0f, 1.0f, 0.0f };
        GameAudio::Update(m_hWnd, listenerPosition, listenerFront, listenerTop);
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
        if (m_debugProfileStartTick != 0)
        {
            m_debugAudioAccumulatedMilliseconds +=
                static_cast<double>(GetTickCount64() - audioStartTick);
        }
#endif

        if (m_gameState == GameState::Title)
        {
            GameAudio::PlayTitleMusic();
        }
        else if (m_gameState == GameState::GameOver)
        {
            GameAudio::PlayGameOverMusic();
        }
        else if (m_gameState == GameState::Playing || m_gameState == GameState::StageIntro)
        {
            const StageManager::StageData& audioStage = m_stageManager.GetCurrentStage();
            const bool useRainEnvironment = audioStage.weather == StageManager::StageWeather::Rain;
            const bool stageCleared = m_saveDataManager.IsStageCleared(audioStage.id);
            GameAudio::UpdateStageMusic(audioStage.id, audioStage.number, useRainEnvironment, GetCurrentWorld(), stageCleared);
        }
        else if (m_gameState == GameState::Ending || m_gameState == GameState::EndingFin)
        {
            GameAudio::PlayEndingMusic();
        }
        else if (m_gameState == GameState::SlideShow)
        {
            GameAudio::PlayStoryMusic();
        }

        UpdateStageSelectMaskedGaussian();

        if (m_stageTransitionAction != StageTransitionAction::None)
        {
            UpdateStageTransition();
            continue;
        }

        if (m_gameState == GameState::Playing &&
            !m_pauseMenu.IsOpen() &&
            !m_craftMenu.IsOpen() &&
            !m_explanationManager.IsActive() &&
            !m_playerDeathPending &&
            !m_stageClearInputLocked &&
            !IsHitStopActive() &&
            (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE) ||
             InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_START)))
        {
            // ステージセレクト画面以外では、攻略済みかどうかにかかわらず
            // 「ステージセレクトに戻る」を常時有効にする。
            const bool returnToStageSelectEnabled = !IsCurrentStageSelect();
            m_pauseMenu.Open(IsCurrentStageSelect(), returnToStageSelectEnabled, true);
        }

        if (m_gameState != GameState::EndingFin &&
            !IsCurrentStageSelect() &&
            !m_pauseMenu.IsOpen() &&
            !m_craftMenu.IsOpen() &&
            !m_explanationManager.IsActive() &&
            !m_playerDeathPending &&
            !m_stageClearInputLocked &&
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
            !m_explanationManager.IsActive() &&
            !m_playerDeathPending &&
            !m_stageClearInputLocked &&
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
                ApplyTitleRenderSettings();
                m_gameState = GameState::Title;
                ApplyMouseCursor();
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
                        m_render.SetFadeAlpha(1.0f);
                        m_gameState = GameState::Playing;
                        m_stageTransitionAction = StageTransitionAction::WaitForStageLoad;
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
                            m_render.SetFadeAlpha(1.0f);
                            m_gameState = GameState::Playing;
                            m_stageTransitionAction = StageTransitionAction::WaitForStageLoad;
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
        else if (m_gameState == GameState::BossDefeat)
        {
            UpdateBossDefeat();
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
                    m_pauseMenu.CloseImmediately();
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
                m_pauseMenu.CloseImmediately();
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
        else if (m_warpPhase != WarpPhase::None)
        {
            GameAudio::StopDoorMovement();
            GameAudio::StopPushableBoxMovement();
            UpdateWarp();
            m_render.Draw();
            continue;
        }
        else
        {
            if (m_craftMenu.BlocksGameInput())
            {
                GameAudio::StopDoorMovement();
                GameAudio::StopPushableBoxMovement();
                m_craftMenu.Update();
                ApplyUnlockedAbilities();
                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    UpdateBossHpBar();
                    DrawBossHpBar();
                    DrawAmmoGauge();
                }
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render, m_playerMover.GetPosition());
                m_craftMenu.Render();
                m_render.Draw();
                continue;
            }

            if (m_pauseMenu.BlocksGameInput())
            {
                GameAudio::StopDoorMovement();
                GameAudio::StopPushableBoxMovement();
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
                        if (m_hasSelectedStagePortal)
                        {
                            m_saveDataManager.SetStageSelectPosition(
                                m_stageManager.GetCurrentStage().id,
                                m_selectedStagePortalId);
                        }
                        m_saveDataManager.Save();
                        m_itemPickupMessage = L"セーブが完了しました";
                        m_itemPickupMessageFrames = kItemPickupMessageTotalFrames;
                        GameAudio::PlaySaveComplete();
                    }
                }
                if (m_pauseMenu.ConsumeReturnToStageSelectRequested())
                {
                    BeginStageExit();
                    continue;
                }
                if (m_pauseMenu.ConsumeReturnToTitleRequested())
                {
                    BeginReturnToTitle();
                    continue;
                }
                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    UpdateBossHpBar();
                    DrawBossHpBar();
                    DrawAmmoGauge();
                }
                m_damagePopupManager.Draw();
                DrawItemPickupMessage();
                m_pauseMenu.Render(m_stageManager.GetCurrentStageDisplayName(), m_player.GetLives());
                m_render.Draw();
                continue;
            }

            if (m_playerDeathPending)
            {
                GameAudio::StopDoorMovement();
                GameAudio::StopPushableBoxMovement();
                // 地上での死亡は崩れ落ちるモーションを見せてから暗転する。
                // 完全暗転（フェードα=1）になったら瞬間移動でリスポーンし、黒保持を経てフェードインする。
                if (m_respawnPhase == RespawnPhase::DeathMotion)
                {
                    --m_respawnFadeFrames;
                    if (m_respawnFadeFrames <= kRespawnFadeOutFrames)
                    {
                        m_respawnPhase = RespawnPhase::FadeOut;
                        m_respawnFadeFrames = kRespawnFadeOutFrames;
                        m_render.StartFadeOut(
                            static_cast<float>(kRespawnFadeOutFrames) / 60.0f);
                    }
                }
                else if (m_respawnPhase == RespawnPhase::GameOverWait)
                {
                    --m_respawnFadeFrames;
                    if (m_respawnFadeFrames <= 0)
                    {
                        m_respawnPhase = RespawnPhase::FadeOut;
                        m_respawnFadeFrames = kRespawnFadeOutFrames;
                        m_render.StartFadeOut(
                            static_cast<float>(kRespawnFadeOutFrames) / 60.0f);
                    }
                }
                else if (m_respawnPhase == RespawnPhase::FadeOut)
                {
                    if (m_render.GetFadeAlpha() >= 1.0f)
                    {
                        CompletePlayerDeath();
                        if (m_gameState == GameState::GameOver)
                        {
                            // GameOver へ移行。以降は UpdateGameOver がフェードを担当する。
                            m_render.Draw();
                            continue;
                        }
                        // 通常リスポーン: m_playerDeathPending は true のままなので、
                        // 次フレーム以降もこのブロックで HoldBlack → FadeIn を進める。
                        m_respawnPhase = RespawnPhase::HoldBlack;
                        m_respawnFadeFrames = kRespawnBlackHoldFrames;
                    }
                }
                else if (m_respawnPhase == RespawnPhase::HoldBlack)
                {
                    --m_respawnFadeFrames;
                    if (m_respawnFadeFrames <= 0)
                    {
                        m_respawnPhase = RespawnPhase::FadeIn;
                        m_respawnFadeFrames = kRespawnFadeInFrames;
                        m_render.StartFadeIn(static_cast<float>(kRespawnFadeInFrames) / 60.0f);
                    }
                }
                else if (m_respawnPhase == RespawnPhase::FadeIn)
                {
                    --m_respawnFadeFrames;
                    if (m_respawnFadeFrames <= 0 && m_render.GetFadeAlpha() <= 0.0f)
                    {
                        // フェードイン完了。死亡シーケンス終了。
                        m_respawnPhase = RespawnPhase::None;
                        m_playerDeathPending = false;
    m_warpPhase = WarpPhase::None;
    m_warpFadeFrames = 0;
                    }
                }

                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    UpdateBossHpBar();
                    DrawBossHpBar();
                    DrawAmmoGauge();
                }
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render, m_playerMover.GetPosition());
                m_render.Draw();
                continue;
            }

            if (IsHitStopActive())
            {
                GameAudio::StopDoorMovement();
                GameAudio::StopPushableBoxMovement();
                m_destructibleManager.Update(m_render);
                m_enemyManager.SyncMeshes(m_render);
                UpdateGoalArrow();

                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    UpdateBossHpBar();
                    DrawBossHpBar();
                    DrawAmmoGauge();
                    m_render.DrawImageSized(GetAttackIconPath(m_playerAttackController.GetAttackType(false)),
                                            kAttackTypeHudX,
                                            kAttackTypeHudY,
                                            kAttackTypeIconSize,
                                            kAttackTypeIconSize);
                }
                m_damagePopupManager.Update();
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render, m_playerMover.GetPosition());
                DrawItemPickupMessage();
                m_render.Draw();
                UpdateHitStop();
                continue;
            }

            m_explanationManager.TryActivate(m_playerMover.GetPosition());
            if (m_explanationManager.IsActive())
            {
                GameAudio::StopDoorMovement();
                GameAudio::StopPushableBoxMovement();
                m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
                m_pendingJump = false;
                m_explanationManager.Update();
                if (!IsCurrentStageSelect())
                {
                    m_hpBar.Draw();
                    UpdateBossHpBar();
                    DrawBossHpBar();
                    DrawAmmoGauge();
                }
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render, m_playerMover.GetPosition());
                m_explanationManager.Render();
                m_render.Draw();
                continue;
            }

            // QTE 中はプレイヤー/カメラ/敵/インタラクト入力を止める
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            ULONGLONG gameLogicStartTick = 0;
#endif
            if (m_qte == nullptr)
            {
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                gameLogicStartTick = GetTickCount64();
#endif
                // マウスカーソル表示中はUI操作を優先し、カメラ回転を止める。
                // 固定カメラ時もマウスによる回転を無効化する。
                if (!m_stageClearInputLocked && !m_mouseCursorVisible && !m_useFixedCamera)
                {
                    UpdateCameraByInput();
                }

                // 入力処理 → メッシュ位置・カメラ設定（衝突判定前）
                if (!m_stageClearInputLocked)
                {
                    UpdatePlayerByInput();
                }

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                if (m_debugProfileStartTick != 0 && gameLogicStartTick != 0)
                {
                    m_debugGameLogicAccumulatedMilliseconds +=
                        static_cast<double>(GetTickCount64() - gameLogicStartTick);
                }
                gameLogicStartTick = 0;
#endif

                // 敵の更新
                if (m_debugEnemyUpdateEnabled)
                {
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                    const ULONGLONG enemyUpdateStartTick = GetTickCount64();
#endif
                    m_enemyManager.Update(m_render, m_playerMover.GetPosition(), m_playerInvincibleFrames > 0);
                    ProcessEnemyAttackHits();
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                    if (m_debugProfileStartTick != 0)
                    {
                        m_debugEnemyUpdateAccumulatedMilliseconds +=
                            static_cast<double>(GetTickCount64() - enemyUpdateStartTick);
                    }
#endif
                }

                m_destructibleManager.Update(m_render);

                const bool isStageSelect = IsCurrentStageSelect();
                if (isStageSelect)
                {
                    UpdateStageSelectCursorByInput();
                }
                else if (!m_stageClearInputLocked)
                {
                    // インタラクト通知とQTE起動判定
                    m_interactionManager.Update(m_playerMover.GetPosition());
                    std::wstring interactionId;
                    if (m_interactionManager.ConsumeTriggeredInteraction(&interactionId) && !interactionId.empty())
                    {
                        const std::wstring interactionType =
                            m_interactionManager.GetInteractableType(interactionId);
                        if (interactionType == L"CraftingStation")
                        {
                            m_craftMenu.SetCurrentWorld(GetCurrentWorld());
                            m_craftMenu.Open();
                        }
                        else
                        {
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

                if (!isStageSelect && !m_stageClearInputLocked && m_stagePortalCooldownFrames <= 0)
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
                            if (!IsBaseId(destStageId) && !m_saveDataManager.IsStageUnlocked(destStageId))
                            {
                                // 未解放ステージ：表示はするが移動しない
                            }
                            else
                            {
                                if (IsBaseId(destStageId))
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
                        int qteRewardItemCount = 1;
                        if (result == NS_QTE_Module::QTE_Module::BarResult::Success)
                        {
                            qteRewardItemCount = 3;
                            GameAudio::PlayQteSuccess();
                        }
                        else
                        {
                            GameAudio::PlayQteNormal();
                        }
                        const std::wstring qteRewardItemId = GetRandomCraftMaterialItemId();
                        m_inventoryManager.AddItem(qteRewardItemId, qteRewardItemCount);
                        m_inventoryManager.Save();
                        ShowItemPickupMessage(qteRewardItemId, qteRewardItemCount);
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
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            const ULONGLONG uiDrawStartTick = GetTickCount64();
#endif
            if (!IsCurrentStageSelect())
            {
                m_hpBar.Draw();
                UpdateBossHpBar();
                DrawBossHpBar();
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
            m_enemyManager.DrawHpBars(m_render, m_playerMover.GetPosition());
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
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            if (m_debugProfileStartTick != 0)
            {
                m_debugUiDrawAccumulatedMilliseconds +=
                    static_cast<double>(GetTickCount64() - uiDrawStartTick);
            }
#endif
            m_render.Draw();

            if (m_pendingHitStopFrames > 0)
            {
                StartHitStopNow(m_pendingHitStopFrames);
                m_pendingHitStopFrames = 0;
                continue;
            }

            // 動く床の位置を描画エンジンから取得し、物理エンジンに反映する。
            {
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                const ULONGLONG platformSyncStartTick = GetTickCount64();
#endif
                const auto& platforms = m_render.GetMovingPlatforms();
                for (const auto& platform : platforms)
                {
                    D3DXVECTOR3 platformPos;
                    if (platform.usesMeshMix2)
                    {
                        platformPos = m_render.GetMeshMix2Pos(platform.renderId);
                    }
                    else
                    {
                        platformPos = m_render.GetMeshMixPos(platform.renderId);
                    }
                    D3DXVECTOR3& prevPos = m_prevMovingPlatformPositions[platform.csvId];
                    const D3DXVECTOR3 platformVelocity = (platformPos - prevPos) / kTargetFrameSeconds;
                    prevPos = platformPos;

                    PhysicsWorld::UpdateCsvTransform(platform.csvId,
                                                     platformPos,
                                                     platform.rotation,
                                                     platform.scale);
                    const int physicsId = PhysicsWorld::GetCsvObjectId(platform.csvId);
                    if (physicsId >= 0)
                    {
                        PhysicsWorld::SetVelocity(physicsId, platformVelocity);
                    }
                }
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                if (m_debugProfileStartTick != 0)
                {
                    m_debugPlatformSyncAccumulatedMilliseconds +=
                        static_cast<double>(GetTickCount64() - platformSyncStartTick);
                }
#endif
            }

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            const ULONGLONG managerUpdateStartTick = GetTickCount64();
#endif
            m_lavaFloodManager.Update(m_render, kTargetFrameSeconds);
            m_lavaRiseManager.Update(m_render, kTargetFrameSeconds);
            m_pushableBoxManager.Update(m_playerMover.GetPosition(),
                                         m_pendingMove,
                                         m_playerMover.GetSettings().moveSpeed,
                                         m_playerMover.IsGrounded(),
                                         kTargetFrameSeconds);
            m_pressurePlateManager.Update(m_render,
                                          m_playerMover.GetPosition(),
                                          m_skullManager,
                                          m_pushableBoxManager,
                                          kTargetFrameSeconds);
            m_attackTriggerManager.Update(m_render, kTargetFrameSeconds);
            GameAudio::SetDoorMovementActive(
                m_pressurePlateManager.IsWallMoving() ||
                m_attackTriggerManager.IsTargetMoving());
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            if (m_debugProfileStartTick != 0)
            {
                m_debugManagerUpdateAccumulatedMilliseconds +=
                    static_cast<double>(GetTickCount64() - managerUpdateStartTick);
            }
#endif

            // 衝突判定（動く床の最新位置を反映）
            const bool isStageSelect = IsCurrentStageSelect();
            const D3DXVECTOR3 playerPositionBeforePhysicsUpdate = m_playerMover.GetPosition();
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            ULONGLONG otherManagersStartTick = 0;
#endif
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
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                    const ULONGLONG physicsStartTick = GetTickCount64();
#endif
                    m_playerMover.Update(m_pendingMove, m_pendingJump);
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                    if (m_debugProfileStartTick != 0)
                    {
                        m_debugPlayerPhysicsAccumulatedMilliseconds +=
                            static_cast<double>(GetTickCount64() - physicsStartTick);
                    }
#endif
                }

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                otherManagersStartTick = GetTickCount64();
#endif
                m_warpBearManager.Update(m_playerMover.GetPosition());
                D3DXVECTOR3 warpTargetPosition;
                float warpTargetRotationY = 0.0f;
                if (m_warpBearManager.TryGetWarpTarget(m_playerMover.GetPosition(),
                                                         &warpTargetPosition,
                                                         &warpTargetRotationY))
                {
                    BeginWarp(warpTargetPosition, warpTargetRotationY);
                    m_render.Draw();
                    continue;
                }

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                const ULONGLONG collectibleStartTick = GetTickCount64();
#endif
                UpdateDashParticleEffect();
                m_dashBoosterManager.Update(m_playerMover.GetPosition(), m_playerMover);
                m_collectibleManager.Update(m_playerMover.GetPosition(), m_destructibleManager);
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
                if (m_debugProfileStartTick != 0)
                {
                    m_debugCollectibleAccumulatedMilliseconds +=
                        static_cast<double>(GetTickCount64() - collectibleStartTick);
                }
#endif
                if (m_playerMover.IsCrushed())
                {
                    DamagePlayerHp(m_player.GetHp());
                }

                // 落下死判定: Y座標が閾値以下でカメラ追従を止める。
                // その後も1秒間自由落下させてから暗転を開始する。
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
                    if (m_fallDeathFrames >= kFallDeathFadeDelayFrames)
                    {
                        HandlePlayerDeath();
                    }
                }

                if (!m_stageClearInputLocked &&
                    m_qte == nullptr &&
                    m_playerAttackController.ConsumeHitRequested())
                {
                    const PlayerAttackDefinition& attackDefinition = m_playerAttackController.GetCurrentDefinition();
                    const PlayerAttackType attackType = m_playerAttackController.GetCurrentAttackType();
                    if (IsBombAttackType(attackType))
                    {
                        const D3DXVECTOR3 forward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
                        const D3DXVECTOR3 bombPosition =
                            m_playerMover.GetPosition() + forward * kBombPlaceDistance;
                        PlaceBomb(bombPosition);
                    }
                    else if (attackDefinition.range > 0.0f)
                    {
                        const int damagedEnemyCount = DamageEnemiesInAttackRange(attackDefinition);
                        if (damagedEnemyCount > 0)
                        {
                            if (IsSwordAttackType(attackType))
                            {
                                GameAudio::PlaySlashHit();
                            }
                            else
                            {
                                GameAudio::PlayAttackHit();
                            }
                            BeginHitStop(GetHitStopFrames(m_playerAttackController.GetCurrentAttackType()));
                        }
                        else
                        {
                            const AttackTriggerActivation triggerActivation =
                                m_attackTriggerManager.TryActivateInAttackRange(
                                    m_render,
                                    m_playerMover.GetPosition(), m_playerYaw,
                                    attackDefinition.range,
                                    attackDefinition.verticalMinOffset,
                                    attackDefinition.verticalMaxOffset,
                                    attackDefinition.halfAngleRadians);
                            if (triggerActivation != AttackTriggerActivation::None)
                            {
                                BeginHitStop(GetHitStopFrames(m_playerAttackController.GetCurrentAttackType()));
                            }
                            else
                            {
                                const DestructibleObject* destructible = m_destructibleManager.FindInAttackRange(
                                    m_playerMover.GetPosition(), m_playerYaw,
                                    attackDefinition.range,
                                    attackDefinition.verticalMinOffset,
                                    attackDefinition.verticalMaxOffset,
                                    attackDefinition.halfAngleRadians);
                                if (destructible != nullptr)
                                {
                                    if (m_destructibleManager.TryDamage(m_render, *destructible, attackDefinition.damage))
                                    {
                                        m_damagePopupManager.Add(attackDefinition.damage, destructible->position, false);
                                        if (IsSwordAttackType(attackType))
                                        {
                                            GameAudio::PlaySlashHit();
                                        }
                                        else
                                        {
                                            GameAudio::PlayAttackHit();
                                        }
                                        BeginHitStop(GetHitStopFrames(m_playerAttackController.GetCurrentAttackType()));
                                    }
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
            if (m_playerInvincibleFrames <= 0 && !m_pickupManager.IsStarActive())
            {
                int lavaDamage = m_lavaZoneManager.GetContactDamage(m_playerMover.GetPosition());
                const int lavaFloodDamage =
                    m_lavaFloodManager.GetContactDamage(m_playerMover.GetPosition());
                if (lavaFloodDamage > lavaDamage)
                {
                    lavaDamage = lavaFloodDamage;
                }
                const int lavaRiseDamage =
                    m_lavaRiseManager.GetContactDamage(m_playerMover.GetPosition());
                if (lavaRiseDamage > lavaDamage)
                {
                    lavaDamage = lavaRiseDamage;
                }
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

            ApplyLavaDamageToEnemies();

            if (m_stagePortalCooldownFrames > 0)
            {
                --m_stagePortalCooldownFrames;
            }

            if (m_itemUseCooldownFrames > 0)
            {
                --m_itemUseCooldownFrames;
            }

            // 敵との接触・踏みつけ判定（QTE 中は無効）
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            const ULONGLONG enemyContactStartTick = GetTickCount64();
#endif
            if (m_qte == nullptr)
            {
                const float playerContactRadius = m_playerMover.GetSettings().radius;
                const float playerContactHeight = m_playerMover.GetSettings().height;
                const bool isStarActive = m_pickupManager.IsStarActive();
                for (auto& enemy : m_enemyManager.GetEnemies())
                {
                    if (enemy->IsDead())
                    {
                        continue;
                    }

                    const bool playerTouchingEnemy =
                        enemy->IsTouchingPlayer(m_playerMover.GetPosition(),
                                                playerContactRadius,
                                                playerContactHeight);
                    const bool enemyCanDamagePlayerOnContact =
                        enemy->CanDamagePlayerOnContact(playerTouchingEnemy);

                    if (isStarActive && playerTouchingEnemy)
                    {
                        enemy->TakeDamage(m_render, 10, m_playerMover.GetPosition());
                        m_damagePopupManager.Add(10, enemy->GetPosition(), false);
                        TryDropEnemyItem(*enemy);
                        GameAudio::PlayAttackHit();
                        break;
                    }
                    else if (!isStarActive &&
                             enemy->IsStompedByPlayer(playerPositionBeforePhysicsUpdate,
                                                      m_playerMover.GetPosition(),
                                                      m_playerMover.IsJumping(),
                                                      m_playerMover.GetVelocity().y,
                                                      playerContactRadius))
                    {
                        enemy->TakeDamage(m_render, 10, m_playerMover.GetPosition());
                        m_damagePopupManager.Add(10, enemy->GetPosition(), false);
                        TryDropEnemyItem(*enemy);
                        GameAudio::PlayStomp();
                        enemy->SuppressContactDamageUntilPlayerSeparates();
                        const float jumpVelocity = m_playerMover.GetSettings().jumpVelocity;
                        m_playerMover.ApplyUpwardVelocity(jumpVelocity);
                        break;
                    }
                    else if (m_playerInvincibleFrames <= 0 &&
                             !enemy->UsesSpecialAttacks() &&
                             enemyCanDamagePlayerOnContact)
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
                        enemy->MarkAttackedPlayer(m_render);
                        break;
                    }
                }
            }
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            if (m_debugProfileStartTick != 0)
            {
                m_debugEnemyContactAccumulatedMilliseconds +=
                    static_cast<double>(GetTickCount64() - enemyContactStartTick);
            }
#endif

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            const ULONGLONG bombSkullStartTick = GetTickCount64();
#endif
            UpdateBombs();
            UpdateBusters();
            m_skullManager.Update(
                m_render,
                m_playerMover.GetPosition(),
                m_playerYaw,
                m_enemyManager.GetEnemies(),
                [this](EnemyBase& enemy, const D3DXVECTOR3& sourcePosition)
                {
                    const int kSkullDamage = 3;
                    enemy.TakeDamageWithoutFacing(m_render, kSkullDamage);
                    enemy.StartKnockbackFrom(sourcePosition, 0.7f, 24);
                    m_damagePopupManager.Add(kSkullDamage, enemy.GetPosition(), false);
                    TryDropEnemyItem(enemy);
                });

            if (m_busterCooldownFrames > 0)
            {
                --m_busterCooldownFrames;
            }

            m_pickupManager.UpdatePickups(m_playerMover.GetPosition(),
                                          m_playerMeshId,
                                          m_destructibleManager);

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            if (m_debugProfileStartTick != 0)
            {
                m_debugBombSkullAccumulatedMilliseconds +=
                    static_cast<double>(GetTickCount64() - bombSkullStartTick);
            }
#endif

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
            if (m_debugProfileStartTick != 0 && otherManagersStartTick != 0)
            {
                m_debugOtherManagersAccumulatedMilliseconds +=
                    static_cast<double>(GetTickCount64() - otherManagersStartTick);
            }
#endif

            if (m_player.IsHpZero())
            {
                HandlePlayerDeath();
                if (m_playerDeathPending)
                {
                    continue;
                }
            }

            const bool isBossStage = IsBossStageNumber(m_stageManager.GetCurrentStageNumber());
            const bool usesGoalPortal = ShouldUseGoalPortal();
            if (usesGoalPortal)
            {
                UpdatePortal();
            }

            const bool stageAlreadyCleared =
                m_saveDataManager.IsStageCleared(m_stageManager.GetCurrentStage().id);
            bool isStageClearReached = false;
            if (usesGoalPortal)
            {
                if (m_portalFlagShown && stageAlreadyCleared)
                {
                    // 再クリア時は固定時間で演出へ移らず、入力を止めたまま自然な着地を待つ。
                    isStageClearReached = m_playerMover.IsGrounded();
                }
                else
                {
                    isStageClearReached = IsStageClearReached();
                }
            }
            else
            {
                isStageClearReached = IsBossStageClearReached();
            }

            if (!IsCurrentStageSelect() &&
                !IsBaseId(m_stageManager.GetCurrentStage().id) &&
                isStageClearReached)
            {
                ClearBusters();
                if (isBossStage &&
                    !stageAlreadyCleared)
                {
                    D3DXVECTOR3 defeatedBossPosition = m_playerMover.GetPosition();
                    for (const auto& enemy : m_enemyManager.GetEnemies())
                    {
                        if (enemy->IsBoss() && enemy->IsDead())
                        {
                            defeatedBossPosition = enemy->GetPosition();
                            break;
                        }
                    }
                    BeginBossDefeat(defeatedBossPosition);
                }
                else
                {
                    m_gameState = GameState::StageClear;
                    m_stageClearProcessed = false;
                    m_stageClearFrame = 0;
                }
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
                GameAudio::PlayArrow();
            }
        }

#if !defined(REDFORTRESS_DISABLE_SETTINGS_DIALOG)
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
#endif

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

    // 前回のフレーム終了時刻から今回までの経過時間をフレーム全体として記録する。
    const ULONGLONG frameStartTick = GetTickCount64();
    if (m_debugProfileStartTick != 0 && m_debugGameLoopEndTick != 0)
    {
        m_debugFrameTotalAccumulatedMilliseconds +=
            static_cast<double>(frameStartTick - m_debugGameLoopEndTick);
    }
    m_debugGameLoopStartTick = frameStartTick;

    if (m_debugInvincible)
    {
        m_playerInvincibleFrames = 120;
    }

    if (m_debugProfileStartTick != 0)
    {
        int rayCastObjectCount = 0;
        int rayCastShapeObjectCount = 0;
        int checkCollideCount = 0;
        double rayCastObjectMilliseconds = 0.0;
        double rayCastShapeObjectMilliseconds = 0.0;
        double checkCollideMilliseconds = 0.0;
        PhysicsWorld::GetProfileCounters(&rayCastObjectCount,
                                         &rayCastShapeObjectCount,
                                         &checkCollideCount,
                                         &rayCastObjectMilliseconds,
                                         &rayCastShapeObjectMilliseconds,
                                         &checkCollideMilliseconds);

        // 前回取得時からの差分を今回のフレーム分として加算する。
        m_debugProfilePhysicsRayCastObjectCount +=
            rayCastObjectCount - m_debugPhysicsRayCastObjectCount;
        m_debugProfilePhysicsRayCastShapeObjectCount +=
            rayCastShapeObjectCount - m_debugPhysicsRayCastShapeObjectCount;
        m_debugProfilePhysicsCheckCollideCount +=
            checkCollideCount - m_debugPhysicsCheckCollideCount;
        m_debugProfilePhysicsRayCastObjectMilliseconds +=
            rayCastObjectMilliseconds - m_debugPhysicsRayCastObjectMilliseconds;
        m_debugProfilePhysicsRayCastShapeObjectMilliseconds +=
            rayCastShapeObjectMilliseconds - m_debugPhysicsRayCastShapeObjectMilliseconds;
        m_debugProfilePhysicsCheckCollideMilliseconds +=
            checkCollideMilliseconds - m_debugPhysicsCheckCollideMilliseconds;
        m_debugPhysicsRayCastObjectCount = rayCastObjectCount;
        m_debugPhysicsRayCastShapeObjectCount = rayCastShapeObjectCount;
        m_debugPhysicsCheckCollideCount = checkCollideCount;
        m_debugPhysicsRayCastObjectMilliseconds = rayCastObjectMilliseconds;
        m_debugPhysicsRayCastShapeObjectMilliseconds = rayCastShapeObjectMilliseconds;
        m_debugPhysicsCheckCollideMilliseconds = checkCollideMilliseconds;

        const NSRender::RenderFrameProfile& renderProfile = m_render.GetLastFrameProfile();
        ++m_debugProfileRenderSamples;
        m_debugProfileSceneUpdateMilliseconds += renderProfile.sceneUpdateMilliseconds;
        m_debugProfileGBufferMilliseconds += renderProfile.gBufferMilliseconds;
        m_debugProfileMirrorMilliseconds += renderProfile.mirrorMilliseconds;
        m_debugProfileMainPassMilliseconds += renderProfile.mainPassMilliseconds;
        m_debugProfileMainPassSkinAnimMilliseconds += renderProfile.mainPassSkinAnimMilliseconds;
        m_debugProfileMainPassMeshMix2Milliseconds += renderProfile.mainPassMeshMix2Milliseconds;
        m_debugProfileMainPassMeshMix2ParameterMilliseconds += renderProfile.mainPassMeshMix2ParameterMilliseconds;
        m_debugProfileMainPassMeshMix2DrawMilliseconds += renderProfile.mainPassMeshMix2DrawMilliseconds;
        m_debugProfileMainPassInstancingMilliseconds += renderProfile.mainPassInstancingMilliseconds;
        m_debugProfileMainPassOtherMeshMilliseconds += renderProfile.mainPassOtherMeshMilliseconds;
        m_debugProfileMainPassSkinAnimDraws += renderProfile.mainPassSkinAnimDraws;
        m_debugProfileMainPassMeshMix2Draws += renderProfile.mainPassMeshMix2Draws;
        m_debugProfileMainPassInstancingDraws += renderProfile.mainPassInstancingDraws;
        m_debugProfilePostEffectMilliseconds += renderProfile.postEffectMilliseconds;
        m_debugProfileDraw2DMilliseconds += renderProfile.draw2DMilliseconds;
        m_debugProfileFrameWaitMilliseconds += renderProfile.frameWaitMilliseconds;
        m_debugProfilePresentMilliseconds += renderProfile.presentMilliseconds;
        m_debugProfileRenderTotalMilliseconds += renderProfile.totalMilliseconds;
        m_debugProfilePhysicsRayCastObjectCount += m_debugPhysicsRayCastObjectCount;
        m_debugProfilePhysicsRayCastShapeObjectCount += m_debugPhysicsRayCastShapeObjectCount;
        m_debugProfilePhysicsCheckCollideCount += m_debugPhysicsCheckCollideCount;
        m_debugProfilePhysicsRayCastObjectMilliseconds += m_debugPhysicsRayCastObjectMilliseconds;
        m_debugProfilePhysicsRayCastShapeObjectMilliseconds += m_debugPhysicsRayCastShapeObjectMilliseconds;
        m_debugProfilePhysicsCheckCollideMilliseconds += m_debugPhysicsCheckCollideMilliseconds;
        m_debugProfilePlayerPhysicsMilliseconds += m_debugPlayerPhysicsAccumulatedMilliseconds;
        m_debugProfileEnemyUpdateMilliseconds += m_debugEnemyUpdateAccumulatedMilliseconds;
        m_debugProfileManagerUpdateMilliseconds += m_debugManagerUpdateAccumulatedMilliseconds;
        m_debugProfilePlatformSyncMilliseconds += m_debugPlatformSyncAccumulatedMilliseconds;
        m_debugProfileGameLogicMilliseconds += m_debugGameLogicAccumulatedMilliseconds;
        m_debugProfileAudioMilliseconds += m_debugAudioAccumulatedMilliseconds;
        m_debugProfileInputMilliseconds += m_debugInputAccumulatedMilliseconds;
        m_debugProfileUiDrawMilliseconds += m_debugUiDrawAccumulatedMilliseconds;
        m_debugProfileOtherManagersMilliseconds += m_debugOtherManagersAccumulatedMilliseconds;
        m_debugProfileEnemyContactMilliseconds += m_debugEnemyContactAccumulatedMilliseconds;
        m_debugProfileBombSkullMilliseconds += m_debugBombSkullAccumulatedMilliseconds;
        m_debugProfileCollectibleMilliseconds += m_debugCollectibleAccumulatedMilliseconds;
        const double sectionTotalMilliseconds =
            m_debugPlayerPhysicsAccumulatedMilliseconds +
            m_debugEnemyUpdateAccumulatedMilliseconds +
            m_debugManagerUpdateAccumulatedMilliseconds +
            m_debugPlatformSyncAccumulatedMilliseconds +
            m_debugGameLogicAccumulatedMilliseconds +
            m_debugAudioAccumulatedMilliseconds +
            m_debugInputAccumulatedMilliseconds +
            m_debugUiDrawAccumulatedMilliseconds +
            m_debugOtherManagersAccumulatedMilliseconds +
            renderProfile.totalMilliseconds;
        m_debugProfileOtherMilliseconds +=
            m_debugFrameTotalAccumulatedMilliseconds - sectionTotalMilliseconds;
        m_debugPlayerPhysicsAccumulatedMilliseconds = 0.0;
        m_debugEnemyUpdateAccumulatedMilliseconds = 0.0;
        m_debugManagerUpdateAccumulatedMilliseconds = 0.0;
        m_debugPlatformSyncAccumulatedMilliseconds = 0.0;
        m_debugGameLogicAccumulatedMilliseconds = 0.0;
        m_debugAudioAccumulatedMilliseconds = 0.0;
        m_debugInputAccumulatedMilliseconds = 0.0;
        m_debugUiDrawAccumulatedMilliseconds = 0.0;
        m_debugOtherManagersAccumulatedMilliseconds = 0.0;
        m_debugEnemyContactAccumulatedMilliseconds = 0.0;
        m_debugBombSkullAccumulatedMilliseconds = 0.0;
        m_debugCollectibleAccumulatedMilliseconds = 0.0;
        m_debugFrameTotalAccumulatedMilliseconds = 0.0;
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

    m_debugGameLoopEndTick = GetTickCount64();
}

bool GameApp::LoadStageForDebug(const std::wstring& stageId)
{
    const std::size_t stageIndex = m_stageManager.FindStageIndexById(stageId);
    if (stageIndex >= m_stageManager.GetStageCount())
    {
        return false;
    }

    return CompleteStageMove(stageIndex);
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
        m_debugProfileMainPassSkinAnimMilliseconds = 0.0;
        m_debugProfileMainPassMeshMix2Milliseconds = 0.0;
        m_debugProfileMainPassMeshMix2ParameterMilliseconds = 0.0;
        m_debugProfileMainPassMeshMix2DrawMilliseconds = 0.0;
        m_debugProfileMainPassInstancingMilliseconds = 0.0;
        m_debugProfileMainPassOtherMeshMilliseconds = 0.0;
        m_debugProfileMainPassSkinAnimDraws = 0;
        m_debugProfileMainPassMeshMix2Draws = 0;
        m_debugProfileMainPassInstancingDraws = 0;
        m_debugProfilePostEffectMilliseconds = 0.0;
        m_debugProfileDraw2DMilliseconds = 0.0;
        m_debugProfileFrameWaitMilliseconds = 0.0;
        m_debugProfilePresentMilliseconds = 0.0;
        m_debugProfileRenderTotalMilliseconds = 0.0;
        m_debugProfilePlayerPhysicsMilliseconds = 0.0;
        m_debugProfileEnemyUpdateMilliseconds = 0.0;
        m_debugProfileManagerUpdateMilliseconds = 0.0;
        m_debugProfilePlatformSyncMilliseconds = 0.0;
        m_debugProfileGameLogicMilliseconds = 0.0;
        m_debugProfileOtherMilliseconds = 0.0;
        m_debugProfilePhysicsRayCastObjectCount = 0;
        m_debugProfilePhysicsRayCastShapeObjectCount = 0;
        m_debugProfilePhysicsCheckCollideCount = 0;
        m_debugProfilePhysicsRayCastObjectMilliseconds = 0.0;
        m_debugProfilePhysicsRayCastShapeObjectMilliseconds = 0.0;
        m_debugProfilePhysicsCheckCollideMilliseconds = 0.0;
        m_debugPlayerPhysicsAccumulatedMilliseconds = 0.0;
        m_debugEnemyUpdateAccumulatedMilliseconds = 0.0;
        m_debugManagerUpdateAccumulatedMilliseconds = 0.0;
        m_debugPlatformSyncAccumulatedMilliseconds = 0.0;
        m_debugGameLogicAccumulatedMilliseconds = 0.0;
        m_debugAudioAccumulatedMilliseconds = 0.0;
        m_debugInputAccumulatedMilliseconds = 0.0;
        m_debugUiDrawAccumulatedMilliseconds = 0.0;
        m_debugOtherManagersAccumulatedMilliseconds = 0.0;
        m_debugEnemyContactAccumulatedMilliseconds = 0.0;
        m_debugBombSkullAccumulatedMilliseconds = 0.0;
        m_debugCollectibleAccumulatedMilliseconds = 0.0;
        m_debugProfileAudioMilliseconds = 0.0;
        m_debugProfileInputMilliseconds = 0.0;
        m_debugProfileUiDrawMilliseconds = 0.0;
        m_debugProfileOtherManagersMilliseconds = 0.0;
        m_debugProfileEnemyContactMilliseconds = 0.0;
        m_debugProfileBombSkullMilliseconds = 0.0;
        m_debugProfileCollectibleMilliseconds = 0.0;
        m_debugFrameTotalAccumulatedMilliseconds = 0.0;
        m_debugPhysicsRayCastObjectCount = 0;
        m_debugPhysicsRayCastShapeObjectCount = 0;
        m_debugPhysicsCheckCollideCount = 0;
        m_debugPhysicsRayCastObjectMilliseconds = 0.0;
        m_debugPhysicsRayCastShapeObjectMilliseconds = 0.0;
        m_debugPhysicsCheckCollideMilliseconds = 0.0;
        PhysicsWorld::ResetProfileAccumulators();
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
                 << ",\"mainPassSkinAnimMs\":" << m_debugProfileMainPassSkinAnimMilliseconds / renderSampleDivisor
                 << ",\"mainPassMeshMix2Ms\":" << m_debugProfileMainPassMeshMix2Milliseconds / renderSampleDivisor
                 << ",\"mainPassMeshMix2ParamMs\":" << m_debugProfileMainPassMeshMix2ParameterMilliseconds / renderSampleDivisor
                 << ",\"mainPassMeshMix2DrawMs\":" << m_debugProfileMainPassMeshMix2DrawMilliseconds / renderSampleDivisor
                 << ",\"mainPassInstancingMs\":" << m_debugProfileMainPassInstancingMilliseconds / renderSampleDivisor
                 << ",\"mainPassOtherMeshMs\":" << m_debugProfileMainPassOtherMeshMilliseconds / renderSampleDivisor
                 << ",\"mainPassSkinAnimDraws\":" << m_debugProfileMainPassSkinAnimDraws
                 << ",\"mainPassMeshMix2Draws\":" << m_debugProfileMainPassMeshMix2Draws
                 << ",\"mainPassInstancingDraws\":" << m_debugProfileMainPassInstancingDraws
                 << ",\"postEffectMs\":" << m_debugProfilePostEffectMilliseconds / renderSampleDivisor
                 << ",\"draw2DMs\":" << m_debugProfileDraw2DMilliseconds / renderSampleDivisor
                 << ",\"frameWaitMs\":" << m_debugProfileFrameWaitMilliseconds / renderSampleDivisor
                 << ",\"presentMs\":" << m_debugProfilePresentMilliseconds / renderSampleDivisor
                 << ",\"renderTotalMs\":" << m_debugProfileRenderTotalMilliseconds / renderSampleDivisor
                 << ",\"playerPhysicsMs\":" << m_debugProfilePlayerPhysicsMilliseconds / renderSampleDivisor
                 << ",\"enemyUpdateMs\":" << m_debugProfileEnemyUpdateMilliseconds / renderSampleDivisor
                 << ",\"managerUpdateMs\":" << m_debugProfileManagerUpdateMilliseconds / renderSampleDivisor
                 << ",\"platformSyncMs\":" << m_debugProfilePlatformSyncMilliseconds / renderSampleDivisor
                 << ",\"gameLogicMs\":" << m_debugProfileGameLogicMilliseconds / renderSampleDivisor
                 << ",\"audioMs\":" << m_debugProfileAudioMilliseconds / renderSampleDivisor
                 << ",\"inputMs\":" << m_debugProfileInputMilliseconds / renderSampleDivisor
                 << ",\"uiDrawMs\":" << m_debugProfileUiDrawMilliseconds / renderSampleDivisor
                 << ",\"otherManagersMs\":" << m_debugProfileOtherManagersMilliseconds / renderSampleDivisor
                 << ",\"enemyContactMs\":" << m_debugProfileEnemyContactMilliseconds / renderSampleDivisor
                 << ",\"bombSkullMs\":" << m_debugProfileBombSkullMilliseconds / renderSampleDivisor
                 << ",\"collectibleMs\":" << m_debugProfileCollectibleMilliseconds / renderSampleDivisor
                 << ",\"otherMs\":" << m_debugProfileOtherMilliseconds / renderSampleDivisor
                 << ",\"physicsRayCastObjectCount\":" << m_debugProfilePhysicsRayCastObjectCount
                 << ",\"physicsRayCastShapeObjectCount\":" << m_debugProfilePhysicsRayCastShapeObjectCount
                 << ",\"physicsCheckCollideCount\":" << m_debugProfilePhysicsCheckCollideCount
                 << ",\"physicsRayCastObjectMs\":" << m_debugProfilePhysicsRayCastObjectMilliseconds / renderSampleDivisor
                 << ",\"physicsRayCastShapeObjectMs\":" << m_debugProfilePhysicsRayCastShapeObjectMilliseconds / renderSampleDivisor
                 << ",\"physicsCheckCollideMs\":" << m_debugProfilePhysicsCheckCollideMilliseconds / renderSampleDivisor
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

    if (commandName == "LOAD_STAGE")
    {
        std::string stageId;
        commandStream >> stageId;
        if (stageId.empty())
        {
            return "{\"ok\":false,\"error\":\"invalid_stage_id\"}";
        }

        const std::wstring wideStageId = WidenDebugIdentifier(stageId);
        if (!LoadStageForDebug(wideStageId))
        {
            return "{\"ok\":false,\"error\":\"stage_not_found\"}";
        }

        return "{\"ok\":true,\"stageId\":\"" + NarrowDebugIdentifier(wideStageId) + "\"}";
    }

    if (commandName == "SET_RENDER_QUALITY")
    {
        std::string qualityName;
        commandStream >> qualityName;
        std::transform(qualityName.begin(), qualityName.end(), qualityName.begin(), [](const unsigned char value) {
            return static_cast<char>(toupper(value));
        });

        std::wstring renderQuality = L"LOW";
        if (qualityName == "MIDDLE")
        {
            renderQuality = L"MIDDLE";
        }
        else if (qualityName == "HIGH")
        {
            renderQuality = L"HIGH";
        }
        else if (qualityName != "LOW")
        {
            return "{\"ok\":false,\"error\":\"invalid_render_quality\"}";
        }

        m_render.SetRenderQuality(renderQuality);
        return "{\"ok\":true,\"quality\":\"" + qualityName + "\"}";
    }

    if (commandName == "QUIT")
    {
        m_close = true;
        if (m_hWnd != NULL)
        {
            DestroyWindow(m_hWnd);
        }
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
    case GameState::BossDefeat:
        return "BossDefeat";
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
        m_craftMenu.CloseImmediately();
    }

    m_explanationManager.CloseImmediately();

    m_interactionManager.Clear();
    m_pressurePlateManager.Clear(m_render);
    m_pushableBoxManager.Clear();
    m_attackTriggerManager.Clear(m_render);
    m_lavaZoneManager.Clear();
    m_lavaFloodManager.Clear();
    m_lavaRiseManager.Clear();
    m_collectibleManager.Clear();
    m_skullManager.Clear(m_render);
    m_render.Finalize();
    PhysicsWorld::Finalize();
    GameAudio::Finalize();
    SoundLib::SoundLib::Finalize();
    InputDevice::Finalize();

    UnregisterClass(_T("Window1"), m_hInstance);

    if (m_hCursor != NULL)
    {
        DestroyCursor(m_hCursor);
        m_hCursor = NULL;
    }

    if (m_hPressedCursor != NULL)
    {
        DestroyCursor(m_hPressedCursor);
        m_hPressedCursor = NULL;
    }

    if (m_hLoadingCursor != NULL)
    {
        DestroyCursor(m_hLoadingCursor);
        m_hLoadingCursor = NULL;
    }
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
    const bool dashStarted = !m_dashParticleEmitted;
    m_render.PlaceDashParticleEffect(origin,
                                     direction,
                                     m_playerMover.IsGrounded(),
                                     dashStarted);
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

    if (nextState == PlayerAnimState::BusterAim)
    {
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, L"shoot_aim");
        return;
    }

    if (nextState == PlayerAnimState::BusterLower)
    {
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, L"shoot_end");
        return;
    }

    if (nextState == PlayerAnimState::Death)
    {
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerDeathAnimName);
        return;
    }

    m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerIdleAnimName);
}

void GameApp::PlayBusterShotAnimation(const bool wasAiming, const float animationSpeed)
{
    m_playerAnimState = PlayerAnimState::Attack;
    m_playerAnimationSpeed = animationSpeed;
    if (m_playerMeshId < 0)
    {
        return;
    }

    m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, animationSpeed);
    if (wasAiming)
    {
        m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, L"shoot_recoil");
        return;
    }

    m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, L"shoot_start");
}

void GameApp::ResetBusterAimState()
{
    m_busterAimHoldFrames = 0;
    m_busterLowerFrames = 0;
}

void GameApp::LoadPlayerMeshForStage(const bool useStageSelectModel, const D3DXVECTOR3& position)
{
    if (m_playerMeshId >= 0)
    {
        if (m_playerUsesStageSelectModel == useStageSelectModel)
        {
            return;
        }
        if (!m_render.RemoveMeshMixSkinAnim(m_playerMeshId))
        {
            throw std::runtime_error("Failed to remove the previous player mesh.");
        }
        m_playerMeshId = -1;
    }

    std::wstring meshPath = g_playerMeshPath;
    std::wstring animationCsvPath = g_playerAnimCsvPath;
    if (useStageSelectModel)
    {
        meshPath = g_stageSelectPlayerMeshPath;
        animationCsvPath = g_stageSelectPlayerAnimCsvPath;
    }

    m_playerMeshId = m_render.AddMeshMixSkinAnim2(meshPath,
                                                  animationCsvPath,
                                                  position,
                                                  D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                  1.0f,
                                                  NSRender::AnimSetMap(),
                                                  -1.0f,
                                                  false,
                                                  false);
    if (m_playerMeshId < 0)
    {
        throw std::runtime_error("Failed to load the player mesh.");
    }
    m_playerUsesStageSelectModel = useStageSelectModel;
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

    // リスポーン時にカメラを初期位置へ戻すための初期値を保存する。
    m_initialCameraDistance = m_cameraDistance;
    m_initialCameraPitch = m_cameraPitch;
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
    if (m_busterAimHoldFrames > 0)
    {
        --m_busterAimHoldFrames;
        if (m_busterAimHoldFrames == 0)
        {
            m_busterLowerFrames = kBusterLowerFrames;
        }
    }
    else if (m_busterLowerFrames > 0)
    {
        --m_busterLowerFrames;
    }

    // ノックバックカウントダウン
    if (m_playerKnockbackFrames > 0)
    {
        --m_playerKnockbackFrames;
    }
    if (m_playerSlowFrames > 0)
    {
        --m_playerSlowFrames;
    }

    const bool shiftPressed = InputDevice::SKeyBoard::IsDown(DIK_LSHIFT)
        || InputDevice::SKeyBoard::IsDown(DIK_RSHIFT);

    if (!IsCurrentStageSelect())
    {
        const long wheelDelta = InputDevice::Mouse::GetWheelDelta();
        if (wheelDelta != 0)
        {
            if (CycleOwnedAttackCategory(1))
            {
                GameAudio::PlayWeaponChange();
            }
        }
        else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP))
        {
            if (CycleOwnedAttackCategory(-1))
            {
                GameAudio::PlayWeaponChange();
            }
        }
        else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN))
        {
            if (CycleOwnedAttackCategory(1))
            {
                GameAudio::PlayWeaponChange();
            }
        }
        else if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_L2))
        {
            // ゲームパッド: L2 で前の攻撃カテゴリへ
            if (CycleOwnedAttackCategory(-1))
            {
                GameAudio::PlayWeaponChange();
            }
        }
        else if (InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_R2))
        {
            // ゲームパッド: R2 で次の攻撃カテゴリへ
            if (CycleOwnedAttackCategory(1))
            {
                GameAudio::PlayWeaponChange();
            }
        }
        UpdateHeldWeaponVisibility();
    }

    bool skullActionTriggered = false;
    const bool padXTriggered = InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_X);
    if (!IsCurrentStageSelect() &&
        (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT) || padXTriggered))
    {
        skullActionTriggered = m_skullManager.HandleLeftClick(m_render,
                                                               m_playerMover.GetPosition(),
                                                               m_playerYaw);
        if (skullActionTriggered)
        {
            UpdateHeldWeaponVisibility();
        }
    }

    const PlayerAttackType requestedAttackType = m_playerAttackController.GetAttackType(shiftPressed);
    if (!IsBusterAttackType(requestedAttackType))
    {
        ResetBusterAimState();
    }

    if (!IsCurrentStageSelect() &&
        !skullActionTriggered &&
        (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT) || padXTriggered) &&
        IsAttackCategoryOwned(requestedAttackType))
    {
        const bool isBombCategory = (m_playerAttackController.GetCurrentCategoryName() == std::wstring(L"海賊爆弾"));
        const bool isBusterCategory = (m_playerAttackController.GetCurrentCategoryName() == std::wstring(L"海賊銃"));
        const bool isStarActive = m_pickupManager.IsStarActive();
        if (isBombCategory)
        {
            const bool canPlaceBomb =
                static_cast<int>(m_activeBombs.size()) < m_bombCapacity;
            if (canPlaceBomb &&
                (isStarActive || m_bombAmmo > 0) &&
                m_playerAttackController.TryStart(requestedAttackType))
            {
                if (!isStarActive)
                {
                    --m_bombAmmo;
                }
                const PlayerAttackDefinition& attackDefinition =
                    m_playerAttackController.GetCurrentDefinition();
                SetPlayerAnimationState(PlayerAnimState::Attack, attackDefinition.animationSpeed);
            }
        }
        else if (isBusterCategory)
        {
            if (m_busterCooldownFrames <= 0 && (isStarActive || m_busterAmmo > 0))
            {
                if (m_playerAttackController.TryStart(requestedAttackType))
                {
                    const bool wasAiming = m_busterAimHoldFrames > 0;
                    const D3DXVECTOR3 forward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
                    D3DXVECTOR3 spawnPos = m_playerMover.GetPosition() + forward * 1.0f;
                    spawnPos.y += kBusterSpawnHeight;
                    SpawnBuster(spawnPos, forward);
                    if (!isStarActive)
                    {
                        --m_busterAmmo;
                    }
                    m_busterCooldownFrames = GetBusterCooldownFrames(m_busterRapidLevel);
                    m_busterAimHoldFrames = kBusterAimHoldFrames;
                    m_busterLowerFrames = 0;
                    GameAudio::PlayBuster();
                    const PlayerAttackDefinition& attackDefinition = m_playerAttackController.GetCurrentDefinition();
                    PlayBusterShotAnimation(wasAiming, attackDefinition.animationSpeed);
                }
            }
        }
        else if (m_playerAttackController.TryStart(requestedAttackType))
        {
            if (IsSwordAttackType(requestedAttackType))
            {
                GameAudio::PlaySwordSwing();
            }
            else
            {
                GameAudio::PlayPlayerAttack();
            }
            const PlayerAttackDefinition& attackDefinition = m_playerAttackController.GetCurrentDefinition();
            SetPlayerAnimationState(PlayerAnimState::Attack, attackDefinition.animationSpeed);
        }
    }

    const D3DXVECTOR3 cameraForward = GetCameraPlanarForward();
    const D3DXVECTOR3 cameraRight   = GetCameraPlanarRight(cameraForward);

    D3DXVECTOR3 localMove(0.0f, 0.0f, 0.0f);
    const InputDevice::GamePadStick padMoveStick = InputDevice::GamePad::GetStickL();
    const float kPadMoveStickThreshold = 0.35f;
    if (m_playerKnockbackFrames <= 0)
    {
        if (InputDevice::SKeyBoard::IsDown(DIK_W)) localMove.z += 1.0f;
        if (InputDevice::SKeyBoard::IsDown(DIK_S)) localMove.z -= 1.0f;
        if (InputDevice::SKeyBoard::IsDown(DIK_D)) localMove.x += 1.0f;
        if (InputDevice::SKeyBoard::IsDown(DIK_A)) localMove.x -= 1.0f;

        // ゲームパッド: 左スティックの倒れ方向で移動
        if (padMoveStick.power >= kPadMoveStickThreshold)
        {
            if (fabsf(padMoveStick.x) >= fabsf(padMoveStick.y))
            {
                if (padMoveStick.x >= 0.0f) localMove.x += 1.0f;
                else localMove.x -= 1.0f;
            }
            else
            {
                if (padMoveStick.y >= 0.0f) localMove.z += 1.0f;
                else localMove.z -= 1.0f;
            }
        }

        // ゲームパッド: POV 十字キーでも移動可能
        if (InputDevice::GamePad::IsDown(InputDevice::GAMEPAD_POV_UP)) localMove.z += 1.0f;
        if (InputDevice::GamePad::IsDown(InputDevice::GAMEPAD_POV_DOWN)) localMove.z -= 1.0f;
        if (InputDevice::GamePad::IsDown(InputDevice::GAMEPAD_POV_LEFT)) localMove.x -= 1.0f;
        if (InputDevice::GamePad::IsDown(InputDevice::GAMEPAD_POV_RIGHT)) localMove.x += 1.0f;
    }

    const bool isMoving  = (localMove.x != 0.0f || localMove.z != 0.0f);
    const bool isWalking = isMoving &&
        (InputDevice::SKeyBoard::IsDown(DIK_LCONTROL) ||
         (padMoveStick.power > 0.05f && padMoveStick.power < 0.5f));

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
    if (m_playerSlowFrames > 0)
    {
        settings.moveSpeed *= 0.5f;
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

    const bool isPushingBox = m_playerKnockbackFrames <= 0 &&
        !m_playerAttackController.IsMovementActive() &&
        isMoving &&
        m_pushableBoxManager.IsPlayerPushingAnyBox(m_playerMover.GetPosition(),
                                                    move,
                                                    m_playerMover.IsGrounded());
    if (isPushingBox)
    {
        settings.moveSpeed *= 0.5f;
        m_playerMover.SetSettings(settings);
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
        else if (m_busterAimHoldFrames > 0)
        {
            nextState = PlayerAnimState::BusterAim;
        }
        else if (m_busterLowerFrames > 0)
        {
            nextState = PlayerAnimState::BusterLower;
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
            else if (nextState == PlayerAnimState::BusterAim)
            {
                animationSpeed = 1.0f;
            }
            else if (nextState == PlayerAnimState::BusterLower)
            {
                animationSpeed = 2.0f;
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
        || InputDevice::SKeyBoard::IsDown(DIK_RSHIFT)
        || InputDevice::GamePad::IsDown(InputDevice::GAMEPAD_L1);
    const bool jumpPressed = InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE)
        || InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_A);
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
    const float attackMinY = playerPos.y + attackDefinition.verticalMinOffset;
    const float attackMaxY = playerPos.y + attackDefinition.verticalMaxOffset;
    int damagedCount = 0;

    for (auto& enemy : m_enemyManager.GetEnemies())
    {
        if (enemy->IsDead())
        {
            continue;
        }

        // 食らい判定は敵の衝突円柱全体。攻撃の垂直帯と体が重なるかで判定する。
        const D3DXVECTOR3 enemyPos = enemy->GetPosition();
        const float enemyHalfHeight = enemy->GetHeight() * 0.5f;
        if (attackMaxY < enemyPos.y - enemyHalfHeight ||
            attackMinY > enemyPos.y + enemyHalfHeight)
        {
            continue;
        }

        D3DXVECTOR3 dir = enemyPos - playerPos;
        dir.y = 0.0f;
        const float dist = D3DXVec3Length(&dir);
        if (dist > attackDefinition.range)
        {
            continue;
        }

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

    const bool busterUnlocked = m_inventoryManager.GetWeaponCount(kBusterWeaponId) > 0;
    const bool bombUnlocked = m_inventoryManager.GetWeaponCount(kBombWeaponId) > 0;
    if ((busterUnlocked || bombUnlocked) &&
        m_destructibleManager.TryDropAmmoHeart(m_render,
                                               enemy.GetPosition(),
                                               kEnemyAmmoHeartDropPercent))
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
    m_bossHpBar.SetBoss(nullptr);
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
    PhysicsLib::SettingsState::SetDoubleJumpEnabled(settings.doubleJumpEnabled);

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
        if (m_gameState == GameState::StageClear &&
            !m_stageClearWasFirstClear &&
            m_stageClearReplayPlayerHidden)
        {
            playerVisible = false;
        }
        D3DXVECTOR3 displayPosition = currentRenderPosition;
        float displayScale = 1.0f;
        if (IsCurrentStageSelect())
        {
            displayPosition.y += kStageSelectPlayerVisualOffsetY;
            if (m_stageManager.GetCurrentStage().id == L"select2")
            {
                displayPosition.y += kStageSelect2PlayerVisualOffsetY;
            }
            displayScale = kStageSelectPlayerVisualScale;
        }
        else if (m_gameState == GameState::StageExit)
        {
            displayPosition.y += m_stageExitVisualOffsetY;
        }
        else if (m_gameState == GameState::StageClear && !m_stageClearWasFirstClear)
        {
            displayPosition.y += m_stageClearVisualOffsetY;
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

    UpdatePlayerPointLight();
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
    bool gunVisible = false;

    if (m_debugPlayerRenderEnabled && !IsCurrentStageSelect() && !m_skullManager.IsHolding())
    {
        const PlayerAttackType attackType = m_playerAttackController.GetAttackType(false);
        if (IsAttackCategoryOwned(attackType))
        {
            if (attackType == PlayerAttackType::WeakAttack)
            {
                stickVisible = true;
            }
            else if (attackType == PlayerAttackType::SwordAttack)
            {
                saberVisible = true;
            }
            else if (IsBusterAttackType(attackType))
            {
                gunVisible = true;
            }
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

    if (m_gunMeshId >= 0)
    {
        m_render.SetMeshMixEnabled(m_gunMeshId, gunVisible);
    }
}

bool GameApp::IsAttackCategoryOwned(const PlayerAttackType attackType) const
{
    if (attackType == PlayerAttackType::WeakAttack ||
        attackType == PlayerAttackType::StrongAttack)
    {
        return m_inventoryManager.GetWeaponCount(kInitialClubWeaponId) > 0;
    }

    if (attackType == PlayerAttackType::SwordAttack ||
        attackType == PlayerAttackType::SwordStrongAttack)
    {
        return m_inventoryManager.GetWeaponCount(kSwordWeaponId) > 0;
    }

    if (IsBusterAttackType(attackType))
    {
        return m_inventoryManager.GetWeaponCount(kBusterWeaponId) > 0;
    }

    if (IsBombAttackType(attackType))
    {
        return m_inventoryManager.GetWeaponCount(kBombWeaponId) > 0;
    }

    return false;
}

bool GameApp::CycleOwnedAttackCategory(const int direction)
{
    const PlayerAttackType previousAttackType = m_playerAttackController.GetAttackType(false);
    const int categoryCount = 4;
    for (int categoryIndex = 0; categoryIndex < categoryCount; ++categoryIndex)
    {
        m_playerAttackController.CycleAttackCategory(direction);
        const PlayerAttackType attackType = m_playerAttackController.GetAttackType(false);
        if (IsAttackCategoryOwned(attackType))
        {
            return attackType != previousAttackType;
        }
    }

    return false;
}

void GameApp::ConfigureStagePointLights(const std::wstring& stageId)
{
    m_render.ClearPointLights();
    UpdatePlayerPointLight();
    if (stageId == L"base2")
    {
        const D3DXCOLOR lanternColor(1.0f, 0.34f, 0.08f, 1.0f);
        const D3DXCOLOR crystalColor(0.10f, 0.55f, 1.0f, 1.0f);
        const D3DXCOLOR portalColor(0.08f, 0.72f, 1.0f, 1.0f);
        m_render.AddPointLight(D3DXVECTOR3(-8.0f, 2.8f, -18.0f), 3.0f, lanternColor);
        m_render.AddPointLight(D3DXVECTOR3(8.0f, 1.8f, 3.0f), 2.6f, crystalColor);
        m_render.AddPointLight(D3DXVECTOR3(0.0f, 3.0f, 26.0f), 3.2f, portalColor);
        return;
    }
    if (stageId == L"base3")
    {
        const D3DXCOLOR sunsetColor(1.0f, 0.38f, 0.10f, 1.0f);
        const D3DXCOLOR relicColor(0.30f, 0.62f, 1.0f, 1.0f);
        const D3DXCOLOR portalColor(0.18f, 0.48f, 1.0f, 1.0f);
        m_render.AddPointLight(D3DXVECTOR3(-8.0f, 2.8f, -18.0f), 2.8f, sunsetColor);
        m_render.AddPointLight(D3DXVECTOR3(9.0f, 3.6f, 7.0f), 2.4f, relicColor);
        m_render.AddPointLight(D3DXVECTOR3(0.0f, 3.0f, 26.0f), 3.0f, portalColor);
        return;
    }
    if (stageId == L"base4")
    {
        const D3DXCOLOR fireColor(1.0f, 0.18f, 0.03f, 1.0f);
        const D3DXCOLOR commandColor(0.26f, 0.42f, 1.0f, 1.0f);
        const D3DXCOLOR portalColor(0.08f, 0.30f, 1.0f, 1.0f);
        m_render.AddPointLight(D3DXVECTOR3(-9.0f, 2.7f, -17.0f), 2.8f, fireColor);
        m_render.AddPointLight(D3DXVECTOR3(9.0f, 2.7f, -17.0f), 2.8f, fireColor);
        m_render.AddPointLight(D3DXVECTOR3(0.0f, 2.4f, 2.0f), 2.5f, commandColor);
        m_render.AddPointLight(D3DXVECTOR3(0.0f, 3.0f, 26.0f), 3.4f, portalColor);
        return;
    }
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
            L"base4"
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
            if (destinationId == L"select3" || IsBaseId(destinationId))
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
                               12.0f,
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
            L"base3"
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
            if (destinationId == L"select2" || destinationId == L"select4" ||
                IsBaseId(destinationId))
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
                               12.0f,
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
        L"base2"
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
        D3DXVECTOR3(-17.0f, 2.3f, 2.0f)
    };
    const D3DXCOLOR unclearedColor(1.0f, 0.04f, 0.02f, 1.0f);
    const D3DXCOLOR clearedColor(0.04f, 1.0f, 0.08f, 1.0f);
    const D3DXCOLOR travelColor(0.04f, 0.25f, 1.0f, 1.0f);
    const int portalLightCount = static_cast<int>(sizeof(portalDestinationIds) / sizeof(portalDestinationIds[0]));
    for (int i = 0; i < portalLightCount; ++i)
    {
        const std::wstring destinationId = portalDestinationIds[i];
        D3DXCOLOR lightColor = unclearedColor;
        if (destinationId == L"select1" || destinationId == L"select3" ||
            IsBaseId(destinationId))
        {
            lightColor = travelColor;
        }
        else if (m_saveDataManager.IsStageCleared(destinationId))
        {
            lightColor = clearedColor;
        }
        m_render.AddPointLight(portalLightPositions[i], 2.0f, lightColor);
    }

    const D3DXCOLOR crystalLightColor(0.08f, 0.55f, 1.0f, 1.0f);
    m_render.AddPointLight(D3DXVECTOR3(-11.5f, 2.4f, 6.5f),
                           2.4f,
                           crystalLightColor,
                           NSRender::PointLightShape::Point,
                           12.0f,
                           10.0f,
                           10.0f,
                           D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                           8.0f);
    m_render.AddPointLight(D3DXVECTOR3(1.5f, 2.4f, 7.5f),
                           2.4f,
                           crystalLightColor,
                           NSRender::PointLightShape::Point,
                           12.0f,
                           10.0f,
                           10.0f,
                           D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                           8.0f);
    m_render.AddPointLight(D3DXVECTOR3(7.0f, 2.4f, 16.5f),
                           2.4f,
                           crystalLightColor,
                           NSRender::PointLightShape::Point,
                           12.0f,
                           10.0f,
                           10.0f,
                           D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                           8.0f);
    m_render.AddPointLight(D3DXVECTOR3(14.5f, 2.4f, -7.5f),
                           2.4f,
                           crystalLightColor,
                           NSRender::PointLightShape::Point,
                           12.0f,
                           10.0f,
                           10.0f,
                           D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                           8.0f);

    D3DXVECTOR3 playerLightPosition =
        m_playerMover.GetPosition() + D3DXVECTOR3(0.0f, kStageSelectPlayerLightHeight, 0.0f);
    playerLightPosition.y += kStageSelect2PlayerLightOffsetY;
    const D3DXCOLOR playerLightColor(1.0f, 0.78f, 0.52f, 1.0f);
    m_render.AddPointLight(playerLightPosition,
                           2.5f,
                           playerLightColor,
                           NSRender::PointLightShape::Point,
                           12.0f,
                           10.0f,
                           10.0f,
                           D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                           12.0f,
                           kStageSelectPlayerLightOwnerTag);
}

void GameApp::LoadPointLightsFromCsv(const std::wstring& csvPath)
{
    if (csvPath.empty())
    {
        return;
    }

    const std::wstring fullCsvPath = NSRender::Util::GetExeDir() + csvPath;
    std::wifstream file(fullCsvPath);
    if (!file.is_open())
    {
        return;
    }
    file.close();

    std::vector<std::vector<std::wstring>> csvData;
    try
    {
        csvData = csv::Read(fullCsvPath);
    }
    catch (...)
    {
        return;
    }

    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.size() < 5 || row.at(0) == L"PosX")
        {
            continue;
        }

        try
        {
            D3DXVECTOR3 pos;
            pos.x = std::stof(row.at(0));
            pos.y = std::stof(row.at(1));
            pos.z = std::stof(row.at(2));

            const float brightness = std::stof(row.at(3));

            D3DXCOLOR color(1.0f, 1.0f, 1.0f, 1.0f);
            if (row.size() > 4)
            {
                color.r = std::stof(row.at(4));
            }
            if (row.size() > 5)
            {
                color.g = std::stof(row.at(5));
            }
            if (row.size() > 6)
            {
                color.b = std::stof(row.at(6));
            }
            if (row.size() > 7)
            {
                color.a = std::stof(row.at(7));
            }

            NSRender::PointLightShape shape = NSRender::PointLightShape::Point;
            if (row.size() > 8)
            {
                const std::wstring shapeStr = row.at(8);
                if (shapeStr == L"Line")
                {
                    shape = NSRender::PointLightShape::Line;
                }
                else if (shapeStr == L"Square")
                {
                    shape = NSRender::PointLightShape::Square;
                }
                else if (shapeStr == L"Cube")
                {
                    shape = NSRender::PointLightShape::Cube;
                }
                else if (shapeStr == L"Sphere")
                {
                    shape = NSRender::PointLightShape::Sphere;
                }
            }

            float lineLength = 12.0f;
            if (row.size() > 9)
            {
                lineLength = std::stof(row.at(9));
            }

            float squareWidth = 10.0f;
            if (row.size() > 10)
            {
                squareWidth = std::stof(row.at(10));
            }

            float squareHeight = 10.0f;
            if (row.size() > 11)
            {
                squareHeight = std::stof(row.at(11));
            }

            D3DXVECTOR3 rotation(0.0f, 0.0f, 0.0f);
            if (row.size() > 14)
            {
                rotation.x = std::stof(row.at(12));
                rotation.y = std::stof(row.at(13));
                rotation.z = std::stof(row.at(14));
            }

            float range = 12.0f;
            if (row.size() > 15)
            {
                range = std::stof(row.at(15));
            }

            std::wstring ownerTag;
            if (row.size() > 16)
            {
                ownerTag = row.at(16);
            }

            m_render.AddPointLight(pos,
                                   brightness,
                                   color,
                                   shape,
                                   lineLength,
                                   squareWidth,
                                   squareHeight,
                                   rotation,
                                   range,
                                   ownerTag);
        }
        catch (...)
        {
            continue;
        }
    }
}

void GameApp::ApplyStageEnvironmentLighting(const std::wstring& stageId)
{
    const int world = GetWorldFromStageId(stageId);
    if (world <= 0)
    {
        return;
    }

    if (world == 2)
    {
        m_render.SetPostEffectSaturate(0.68f);
        m_render.SetMeshMixShadowDarkness(0.75f);
        m_render.SetLightBrightness(0.035f);
        m_render.SetLightColor(D3DXCOLOR(0.16f, 0.20f, 0.36f, 1.0f));
        m_render.SetAmbientLightBrightness(0.055f);
        m_render.SetAmbientLightColor(D3DXCOLOR(0.10f, 0.15f, 0.28f, 1.0f));
        return;
    }

    if (stageId == L"3-1")
    {
        m_render.SetPostEffectSaturate(0.82f);
        m_render.SetMeshMixShadowDarkness(0.92f);
        m_render.SetLightBrightness(0.018f);
        m_render.SetLightColor(D3DXCOLOR(0.10f, 0.12f, 0.20f, 1.0f));
        m_render.SetAmbientLightBrightness(0.022f);
        m_render.SetAmbientLightColor(D3DXCOLOR(0.035f, 0.045f, 0.090f, 1.0f));
        return;
    }

    if (world == 3)
    {
        m_render.SetPostEffectSaturate(1.0f);
        m_render.SetMeshMixShadowDarkness(0.75f);
        m_render.SetLightBrightness(0.65f);
        m_render.SetLightColor(D3DXCOLOR(1.0f, 0.55f, 0.25f, 1.0f));
        m_render.SetAmbientLightBrightness(0.22f);
        m_render.SetAmbientLightColor(D3DXCOLOR(0.55f, 0.25f, 0.10f, 1.0f));
        return;
    }

    if (world == 4)
    {
        m_render.SetPostEffectSaturate(0.8f);
        m_render.SetMeshMixShadowDarkness(0.75f);
        m_render.SetLightBrightness(0.10f);
        m_render.SetLightColor(D3DXCOLOR(0.30f, 0.30f, 0.75f, 1.0f));
        m_render.SetAmbientLightBrightness(0.14f);
        m_render.SetAmbientLightColor(D3DXCOLOR(0.22f, 0.22f, 0.50f, 1.0f));
    }
}

void GameApp::UpdatePlayerPointLight()
{
    const StageManager::StageData& stage = m_stageManager.GetCurrentStage();
    if (!stage.playerPointLightEnabled)
    {
        return;
    }

    const D3DXVECTOR3 lightPosition =
        m_playerMover.GetPosition() + D3DXVECTOR3(0.0f, kPlayerPointLightHeight, 0.0f);
    if (m_render.SetPointLightPositionByOwnerTag(kPlayerPointLightOwnerTag, lightPosition))
    {
        return;
    }

    const D3DXCOLOR lightColor(1.0f, 0.78f, 0.52f, 1.0f);
    m_render.AddPointLight(lightPosition,
                           kPlayerPointLightBrightness,
                           lightColor,
                           NSRender::PointLightShape::Point,
                           12.0f,
                           10.0f,
                           10.0f,
                           D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                           kPlayerPointLightRange,
                           kPlayerPointLightOwnerTag);
}

void GameApp::UpdateStageSelectPlayerLight()
{
    const std::wstring& stageId = m_stageManager.GetCurrentStage().id;
    if (stageId != L"select2" && stageId != L"select3" && stageId != L"select4")
    {
        return;
    }

    D3DXVECTOR3 lightPosition =
        m_playerMover.GetPosition() + D3DXVECTOR3(0.0f, kStageSelectPlayerLightHeight, 0.0f);
    if (stageId == L"select2")
    {
        lightPosition.y += kStageSelect2PlayerLightOffsetY;
    }
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
    if (IsBaseId(destinationId))
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

bool GameApp::ShouldUseGoalPortal() const
{
    const StageManager::StageData& stage = m_stageManager.GetCurrentStage();
    if (!IsBossStageNumber(stage.number))
    {
        return true;
    }

    return m_saveDataManager.IsStageCleared(stage.id);
}

bool GameApp::IsBossStageClearReached() const
{
    const StageManager::StageData& stage = m_stageManager.GetCurrentStage();
    if (!IsBossStageNumber(stage.number))
    {
        return false;
    }

    if (m_saveDataManager.IsStageCleared(stage.id))
    {
        return false;
    }

    bool hasBoss = false;
    for (const auto& enemy : m_enemyManager.GetEnemies())
    {
        if (!enemy->IsBoss())
        {
            continue;
        }

        hasBoss = true;
        if (!enemy->IsDead())
        {
            return false;
        }
    }

    return hasBoss;
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

    if (!ShouldUseGoalPortal())
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
    if (m_goalArrowMeshId >= 0)
    {
        GameAudio::PlayArrow();
    }
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
    m_render.RemovePointLightsByOwnerTag(kStageSelectCursorLightOwnerTag);
    m_selectedStagePortalId.clear();
    m_mouseOverStagePortalId.clear();
    m_stageSelectDisplayedStageName.clear();
    m_stageSelectPendingStageName.clear();
    m_stageSelectStageNameAlpha = 0.0f;
    m_stageSelectStageNameFadingOut = false;
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
        const bool isNavigationPortal = IsBaseId(destinationId) ||
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

void GameApp::LoadStageSelectNavigation(const std::wstring& csvPath)
{
    m_stageSelectNavigation.clear();
    if (!IsCurrentStageSelect())
    {
        return;
    }

    if (csvPath.empty())
    {
        throw std::runtime_error("Stage-select navigation CSV path is empty.");
    }

    const std::wstring fullCsvPath = NSRender::Util::GetExeDir() + csvPath;
    std::wifstream file(fullCsvPath);
    if (!file.is_open())
    {
        throw std::runtime_error("Stage-select navigation CSV could not be opened.");
    }
    file.close();

    const std::vector<std::vector<std::wstring>> csvData = csv::Read(fullCsvPath);
    for (const std::vector<std::wstring>& row : csvData)
    {
        if (!row.empty() && row.at(0) == L"CurrentPortalID")
        {
            continue;
        }
        if (row.size() < 5 || row.at(0).empty())
        {
            throw std::runtime_error("Stage-select navigation CSV contains an invalid row.");
        }

        StageSelectNavigationEntry entry;
        entry.leftPortalId = row.at(1);
        entry.rightPortalId = row.at(2);
        entry.upPortalId = row.at(3);
        entry.downPortalId = row.at(4);
        const std::pair<std::unordered_map<std::wstring, StageSelectNavigationEntry>::iterator, bool> result =
            m_stageSelectNavigation.emplace(row.at(0), entry);
        if (!result.second)
        {
            throw std::runtime_error("Stage-select navigation CSV contains a duplicate portal ID.");
        }
    }

    ValidateStageSelectNavigation();
}

void GameApp::ValidateStageSelectNavigation() const
{
    const std::vector<InteractionManager::Interactable>& interactables = m_interactionManager.GetInteractables();
    std::vector<std::wstring> portalIds;
    for (const InteractionManager::Interactable& interactable : interactables)
    {
        if (interactable.type == L"StagePortal")
        {
            portalIds.push_back(interactable.id);
        }
    }
    if (portalIds.empty())
    {
        throw std::runtime_error("Stage select does not contain any stage portals.");
    }

    for (const std::wstring& portalId : portalIds)
    {
        if (m_stageSelectNavigation.find(portalId) == m_stageSelectNavigation.end())
        {
            throw std::runtime_error("Stage-select navigation CSV is missing a stage portal.");
        }
    }

    for (const std::pair<const std::wstring, StageSelectNavigationEntry>& navigation : m_stageSelectNavigation)
    {
        if (std::find(portalIds.begin(), portalIds.end(), navigation.first) == portalIds.end())
        {
            throw std::runtime_error("Stage-select navigation CSV contains an unknown current portal ID.");
        }

        const std::wstring* destinationIds[] = {
            &navigation.second.leftPortalId,
            &navigation.second.rightPortalId,
            &navigation.second.upPortalId,
            &navigation.second.downPortalId
        };
        for (const std::wstring* destinationId : destinationIds)
        {
            if (destinationId->empty())
            {
                continue;
            }
            if (std::find(portalIds.begin(), portalIds.end(), *destinationId) == portalIds.end())
            {
                throw std::runtime_error("Stage-select navigation CSV contains an unknown destination portal ID.");
            }

            const StageSelectNavigationEntry& reverseEntry = m_stageSelectNavigation.at(*destinationId);
            const bool hasReverseConnection =
                reverseEntry.leftPortalId == navigation.first ||
                reverseEntry.rightPortalId == navigation.first ||
                reverseEntry.upPortalId == navigation.first ||
                reverseEntry.downPortalId == navigation.first;
            if (!hasReverseConnection)
            {
                throw std::runtime_error("Stage-select navigation CSV contains a one-way connection.");
            }
        }
    }

    std::vector<std::wstring> visitedPortalIds;
    std::vector<std::wstring> pendingPortalIds;
    pendingPortalIds.push_back(portalIds.front());
    for (std::size_t pendingIndex = 0; pendingIndex < pendingPortalIds.size(); ++pendingIndex)
    {
        const std::wstring& portalId = pendingPortalIds.at(pendingIndex);
        if (std::find(visitedPortalIds.begin(), visitedPortalIds.end(), portalId) != visitedPortalIds.end())
        {
            continue;
        }
        visitedPortalIds.push_back(portalId);

        const StageSelectNavigationEntry& entry = m_stageSelectNavigation.at(portalId);
        const std::wstring* destinationIds[] = {
            &entry.leftPortalId,
            &entry.rightPortalId,
            &entry.upPortalId,
            &entry.downPortalId
        };
        for (const std::wstring* destinationId : destinationIds)
        {
            if (!destinationId->empty() &&
                std::find(visitedPortalIds.begin(), visitedPortalIds.end(), *destinationId) ==
                    visitedPortalIds.end())
            {
                pendingPortalIds.push_back(*destinationId);
            }
        }
    }

    if (visitedPortalIds.size() != portalIds.size())
    {
        throw std::runtime_error("Stage-select navigation CSV contains an unreachable stage portal.");
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

    const std::unordered_map<std::wstring, StageSelectNavigationEntry>::const_iterator navigation =
        m_stageSelectNavigation.find(m_selectedStagePortalId);
    if (navigation == m_stageSelectNavigation.end())
    {
        throw std::runtime_error("Selected stage portal is missing from the navigation map.");
    }

    const std::wstring* destinationPortalId = nullptr;
    if (directionX < 0.0f)
    {
        destinationPortalId = &navigation->second.leftPortalId;
    }
    else if (directionX > 0.0f)
    {
        destinationPortalId = &navigation->second.rightPortalId;
    }
    else if (directionY < 0.0f)
    {
        destinationPortalId = &navigation->second.upPortalId;
    }
    else if (directionY > 0.0f)
    {
        destinationPortalId = &navigation->second.downPortalId;
    }

    if (destinationPortalId == nullptr || destinationPortalId->empty() ||
        !IsStagePortalSelectable(*destinationPortalId))
    {
        return;
    }

    const std::vector<InteractionManager::Interactable>& interactables = m_interactionManager.GetInteractables();
    for (const InteractionManager::Interactable& interactable : interactables)
    {
        if (interactable.type == L"StagePortal" && interactable.id == *destinationPortalId)
        {
            m_selectedStagePortalId = interactable.id;
            m_selectedStagePortalPosition = interactable.position;
            m_hasSelectedStagePortal = true;
            SyncStageSelectPlayerToPortal(false);
            return;
        }
    }

    throw std::runtime_error("Stage-select navigation destination was not found.");
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

    UpdateStageSelectStageNameAnimation();

    float directionX = 0.0f;
    float directionY = 0.0f;
    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LEFT) ||
        InputDevice::SKeyBoard::IsDownFirstFrame(DIK_A))
    {
        directionX = -1.0f;
    }
    else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RIGHT) ||
             InputDevice::SKeyBoard::IsDownFirstFrame(DIK_D))
    {
        directionX = 1.0f;
    }
    else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_UP) ||
             InputDevice::SKeyBoard::IsDownFirstFrame(DIK_W))
    {
        directionY = -1.0f;
    }
    else if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_DOWN) ||
             InputDevice::SKeyBoard::IsDownFirstFrame(DIK_S))
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
    const bool wasMouseOverStartButton = m_isMouseOverStartButton;
    if (baseMousePosition.x >= kStageSelectStartHitX &&
        baseMousePosition.x < kStageSelectStartHitX + kStageSelectStartHitWidth &&
        baseMousePosition.y >= kStageSelectStartHitY &&
        baseMousePosition.y < kStageSelectStartHitY + kStageSelectStartHitHeight)
    {
        m_isMouseOverStartButton = true;
    }
    else
    {
        m_isMouseOverStartButton = false;
    }
    if (m_isMouseOverStartButton && !wasMouseOverStartButton)
    {
        GameAudio::PlayMenuMove();
    }

    const std::vector<InteractionManager::Interactable>& interactables =
        m_interactionManager.GetInteractables();
    float nearestDistanceSquared = kStagePortalClickRadius * kStagePortalClickRadius;
    const InteractionManager::Interactable* mouseOverInteractable = nullptr;
    if (!m_isMouseOverStartButton)
    {
        for (const InteractionManager::Interactable& interactable : interactables)
        {
            if (interactable.type != L"StagePortal" ||
                !IsStagePortalSelectable(interactable.id))
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
                mouseOverInteractable = &interactable;
            }
        }
    }

    std::wstring mouseOverPortalId;
    if (mouseOverInteractable != nullptr)
    {
        mouseOverPortalId = mouseOverInteractable->id;
    }
    if (mouseOverPortalId != m_mouseOverStagePortalId)
    {
        m_render.RemovePointLightsByOwnerTag(kStageSelectCursorLightOwnerTag);
        if (!mouseOverPortalId.empty())
        {
            GameAudio::PlayStageSelectMove();

            float cubeVisualOffsetY = kStageSelectCubeVisualOffsetY;
            if (m_stageManager.GetCurrentStage().id == L"select2")
            {
                cubeVisualOffsetY += kStageSelect2CubeVisualOffsetY;
            }

            D3DXVECTOR3 lightPosition = mouseOverInteractable->position;
            lightPosition.y += cubeVisualOffsetY + kStageSelectCursorLightHeight;
            const D3DXCOLOR lightColor(1.0f, 0.86f, 0.38f, 1.0f);
            m_render.AddPointLight(lightPosition,
                                   kStageSelectCursorLightBrightness,
                                   lightColor,
                                   NSRender::PointLightShape::Point,
                                   12.0f,
                                   10.0f,
                                   10.0f,
                                   D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                   kStageSelectCursorLightRange,
                                   kStageSelectCursorLightOwnerTag);
        }
    }
    m_mouseOverStagePortalId = mouseOverPortalId;

    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        if (m_isMouseOverStartButton)
        {
            MoveToSelectedStagePortal();
        }
        else if (mouseOverInteractable != nullptr)
        {
            m_selectedStagePortalId = mouseOverInteractable->id;
            m_selectedStagePortalPosition = mouseOverInteractable->position;
            m_hasSelectedStagePortal = true;
            SyncStageSelectPlayerToPortal(false);
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
    m_preferredStageSelectPortalId.clear();
    if (IsStageSelectId(destinationId))
    {
        m_preferredStageSelectPortalId = L"portal-to-" + m_lastSelectId;
    }
    if (!StartStageByIndex(targetIndex))
    {
        m_preferredStageSelectPortalId.clear();
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

void GameApp::UpdateStageSelectStageNameAnimation()
{
    const std::wstring selectedStageName = GetSelectedStagePortalDisplayName();
    if (selectedStageName != m_stageSelectPendingStageName)
    {
        m_stageSelectPendingStageName = selectedStageName;
        if (m_stageSelectDisplayedStageName.empty())
        {
            m_stageSelectDisplayedStageName = m_stageSelectPendingStageName;
            m_stageSelectStageNameAlpha = 0.0f;
            m_stageSelectStageNameFadingOut = false;
        }
        else if (m_stageSelectDisplayedStageName != m_stageSelectPendingStageName)
        {
            m_stageSelectStageNameFadingOut = true;
        }
        else
        {
            m_stageSelectStageNameFadingOut = false;
        }
    }

    if (m_stageSelectStageNameFadingOut)
    {
        m_stageSelectStageNameAlpha -= kTargetFrameSeconds / kStageSelectStageNameFadeOutDuration;
        if (m_stageSelectStageNameAlpha <= 0.0f)
        {
            m_stageSelectStageNameAlpha = 0.0f;
            m_stageSelectDisplayedStageName = m_stageSelectPendingStageName;
            m_stageSelectStageNameFadingOut = false;
        }
        return;
    }

    if (m_stageSelectDisplayedStageName != m_stageSelectPendingStageName)
    {
        m_stageSelectDisplayedStageName = m_stageSelectPendingStageName;
        m_stageSelectStageNameAlpha = 0.0f;
    }

    m_stageSelectStageNameAlpha += kTargetFrameSeconds / kStageSelectStageNameFadeInDuration;
    if (m_stageSelectStageNameAlpha >= 1.0f)
    {
        m_stageSelectStageNameAlpha = 1.0f;
    }
}

void GameApp::UpdateStageSelectMaskedGaussian()
{
    if (m_pauseMenu.IsOpen() || m_craftMenu.IsOpen() || m_explanationManager.IsActive())
    {
        return;
    }

    const bool isEnabled = m_render.IsPostEffectMaskedGaussianFilterEnabled();
    const std::wstring currentMaskPath = m_render.GetPostEffectMaskedGaussianMaskPath();
    const float currentAmount = m_render.GetPostEffectMaskedGaussianAmount();
    const bool shouldEnable = m_gameState == GameState::Playing && IsCurrentStageSelect();
    if (shouldEnable)
    {
        if (!isEnabled || currentMaskPath != kStageSelectStartMaskPath || currentAmount != 1.0f)
        {
            m_render.SetPostEffectMaskedGaussianMaskPath(kStageSelectStartMaskPath);
            m_render.SetPostEffectMaskedGaussianSampleSize(kStageSelectMaskedGaussianSampleSize);
            m_render.SetPostEffectMaskedGaussianAmount(1.0f);
            m_render.SetPostEffectMaskedGaussianFilter(true);
        }
        return;
    }

    if (isEnabled && currentMaskPath == kStageSelectStartMaskPath)
    {
        m_render.SetPostEffectMaskedGaussianFilter(false);
    }
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
        m_stageSelectHintFontId = m_render.SetUpFontEx(L"BIZ UDGothic", 18, D3DCOLOR_RGBA(255, 255, 255, 150));
    }
    if (m_stageSelectStartButtonFontId < 0)
    {
        m_stageSelectStartButtonFontId = m_render.SetUpFontEx(L"BIZ UDGothic", 60, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    if (!m_stageSelectDisplayedStageName.empty())
    {
        const int stageNameAlpha = static_cast<int>(m_stageSelectStageNameAlpha * 255.0f + 0.5f);
        m_render.DrawTextEx(m_stageSelectFontId,
                            m_stageSelectDisplayedStageName,
                            kStageSelectStageNameX,
                            kStageSelectStageNameY,
                            D3DCOLOR_RGBA(255, 255, 255, stageNameAlpha));
    }

    m_render.DrawTextExRight(m_stageSelectHintFontId,
                             L"方向キー・WASD・パッド・マウス: ステージ選択",
                             kStageSelectHintX,
                             kStageSelectHintFirstLineY,
                             kStageSelectHintWidth,
                             kStageSelectHintLineHeight,
                             D3DCOLOR_RGBA(255, 255, 255, 150));

    m_render.DrawTextExRight(m_stageSelectHintFontId,
                             L"エンター・クリック: 開始",
                             kStageSelectHintX,
                             kStageSelectHintSecondLineY,
                             kStageSelectHintWidth,
                             kStageSelectHintLineHeight,
                             D3DCOLOR_RGBA(255, 255, 255, 150));

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
    const int currentStageWorld =
        GetWorldFromStageId(m_stageManager.GetCurrentStage().id);
    if (currentStageWorld > 0)
    {
        return currentStageWorld;
    }

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
    const bool busterUnlocked = m_inventoryManager.GetWeaponCount(kBusterWeaponId) > 0;
    const bool bombUnlocked = m_inventoryManager.GetWeaponCount(kBombWeaponId) > 0;
    const PlayerAttackType attackType = m_playerAttackController.GetAttackType(false);
    if (busterUnlocked &&
        IsBusterAttackType(attackType) &&
        m_busterAmmo < kBusterAmmoMax)
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
        else
        {
            GameAudio::PlayAmmoGet();
        }
        return true;
    }

    if (bombUnlocked &&
        IsBombAttackType(attackType) &&
        m_bombAmmo < kBombAmmoMax)
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
        else
        {
            GameAudio::PlayAmmoGet();
        }
        return true;
    }

    if (busterUnlocked && m_busterAmmo < kBusterAmmoMax)
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
        else
        {
            GameAudio::PlayAmmoGet();
        }
        return true;
    }

    if (bombUnlocked && m_bombAmmo < kBombAmmoMax)
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
        else
        {
            GameAudio::PlayAmmoGet();
        }
        return true;
    }

    if (busterUnlocked || bombUnlocked)
    {
        m_itemPickupMessage = L"残弾数MAX";
        m_itemPickupMessageFrames = kItemPickupMessageTotalFrames;
        GameAudio::PlayAmmoMax();
    }
    return false;
}

void GameApp::UpdateBossHpBar()
{
    // 生存ボスを検索し、ボスバーに設定する。
    // ボスが切り替わったとき（スポーン/死亡/別ステージへ）は
    // BossHpBar 側で表示をリセットする。
    m_bossHpBar.SetBoss(m_enemyManager.GetAliveBoss());
    m_bossHpBar.Update();
}

void GameApp::DrawBossHpBar()
{
    m_bossHpBar.Draw();
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
    if (itemId == kStarPowerUpItemId)
    {
        return;
    }

    std::wstring message;
    if (itemId == kSpeedUpItemId)
    {
        message = L"スピードアップ";
    }
    else
    {
        if (count == 3)
        {
            message = GetItemDisplayName(itemId) + L"を３つ手に入れた";
        }
        else
        {
            message = GetItemDisplayName(itemId) + L"を手に入れた";
        }
    }
    if (count > 1 && count != 3)
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
                                                          kItemPickupMessageFontSize,
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
                              kItemPickupMessageHeight,
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
    float cubeVisualOffsetY = kStageSelectCubeVisualOffsetY;
    if (m_stageManager.GetCurrentStage().id == L"select2")
    {
        cubeVisualOffsetY += kStageSelect2CubeVisualOffsetY;
    }
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
        if (IsBaseId(destinationId))
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

        D3DXVECTOR3 cubePosition = interactable.position;
        cubePosition.y += cubeVisualOffsetY;
        const int renderId = m_render.AddMeshMix(cubePath,
                                                  cubePosition,
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

void GameApp::UnlockAllWeapons()
{
    m_inventoryManager.AddWeapon(kInitialClubWeaponId, 1);
    m_inventoryManager.AddWeapon(kSwordWeaponId, 1);
    m_inventoryManager.AddWeapon(kBusterWeaponId, 1);
    m_inventoryManager.AddWeapon(kBombWeaponId, 1);
    m_inventoryManager.Save();
    RefillWeaponAmmo();
    UpdateHeldWeaponVisibility();
}

void GameApp::UnlockAllSkills()
{
    m_inventoryManager.UnlockAbility(L"GroundDash");
    m_inventoryManager.UnlockAbility(L"AirDash");
    m_inventoryManager.UnlockAbility(L"DoubleJump");
    ApplyUnlockedAbilities();
}

bool GameApp::StartStageByIndex(std::size_t stageIndex)
{
    if (stageIndex >= m_stageManager.GetStageCount())
    {
        return false;
    }

    const StageManager::StageData& stage = m_stageManager.GetStage(stageIndex);
    if (IsBaseId(stage.id))
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

void GameApp::BeginStageLoadingScreen()
{
    if (m_stageLoadingScreenActive)
    {
        return;
    }

    m_render.StartLoadingScreen();
    m_render.SetLoadingScreenShowTitle(false);
    m_render.SetLoadingScreenProgress(0);
    m_stageLoadingScreenActive = true;
}

void GameApp::EndStageLoadingScreen()
{
    if (!m_stageLoadingScreenActive)
    {
        return;
    }

    m_render.SetLoadingScreenProgress(100);
    m_render.EndLoadingScreen();
    m_stageLoadingScreenActive = false;
}

void GameApp::UpdateStageTransition()
{
    if (m_stageTransitionAction == StageTransitionAction::ReturnToTitle)
    {
        if (m_render.GetFadeAlpha() < 1.0f)
        {
            if (IsCurrentStageSelect())
            {
                DrawStageSelectCursor();
            }
            m_render.Draw();
            return;
        }

        BeginStageLoadingScreen();
        const std::size_t titleStageIndex = m_stageManager.FindStageIndexById(L"select1");
        if (titleStageIndex >= m_stageManager.GetStageCount())
        {
            throw std::runtime_error("Title stage was not found.");
        }
        if (!m_stageManager.MoveToStage(titleStageIndex))
        {
            throw std::runtime_error("Failed to move to the title stage.");
        }

        m_render.Draw();
        LoadCurrentStageObjects();
        m_render.SetLoadingScreenProgress(90);
        m_render.SetFadeAlpha(1.0f);
        m_stageTransitionAction = StageTransitionAction::WaitForTitleLoad;
        return;
    }

    if (m_stageTransitionAction == StageTransitionAction::WaitForTitleLoad)
    {
        m_render.SetFadeAlpha(1.0f);
        m_render.Draw();
        if (!m_render.IsAllMeshLoaded())
        {
            return;
        }

        EndStageLoadingScreen();
        CompleteReturnToTitle();
        m_render.StartFadeIn(kStageSelectTransitionFadeDuration);
        return;
    }

    if (m_stageTransitionAction == StageTransitionAction::WaitForStageLoad)
    {
        m_render.SetFadeAlpha(1.0f);
        m_render.Draw();

        if (!m_render.IsAllMeshLoaded())
        {
            return;
        }

        EndStageLoadingScreen();
        if (IsCurrentStageSelect() || IsBaseId(m_stageManager.GetCurrentStage().id))
        {
            m_render.StartFadeIn(kStageSelectTransitionFadeDuration);
            m_stageTransitionAction = StageTransitionAction::FadeIn;
        }
        else
        {
            m_stageTransitionAction = StageTransitionAction::None;
            m_stageTransitionIndex = static_cast<std::size_t>(-1);
            m_gameState = GameState::StageIntro;
            BeginStageIntro();
        }
        return;
    }

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
        return;
    }

    throw std::runtime_error("Invalid stage transition action.");
}

bool GameApp::CompleteStageMove(const std::size_t stageIndex)
{
    if (stageIndex >= m_stageManager.GetStageCount())
    {
        return false;
    }

    if (!m_stageManager.MoveToStage(stageIndex))
    {
        return false;
    }

    BeginStageLoadingScreen();
    m_render.Draw();
    LoadCurrentStageObjects();
    m_render.SetLoadingScreenProgress(90);
    m_render.SetFadeAlpha(1.0f);
    if (IsCurrentStageSelect() || IsBaseId(m_stageManager.GetCurrentStage().id))
    {
        m_gameState = GameState::Playing;
    }
    else
    {
        m_gameState = GameState::StageIntro;
    }
    m_stageTransitionAction = StageTransitionAction::WaitForStageLoad;
    return true;
}

void GameApp::StartNewGame()
{
    m_saveDataManager.ResetToDefaults();
    m_preferredStageSelectPortalId.clear();
    m_inventoryManager.Reset();
    m_inventoryManager.AddWeapon(kInitialClubWeaponId, 1);
    m_inventoryManager.Save();
    ApplyUnlockedAbilities();
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
    ApplyUnlockedAbilities();
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
        const std::size_t continueStageIndex = GetContinueStartStageIndex();
        m_preferredStageSelectPortalId.clear();
        if (m_saveDataManager.HasStageSelectPosition() &&
            m_stageManager.GetStage(continueStageIndex).id == m_saveDataManager.GetStageSelectId())
        {
            m_preferredStageSelectPortalId = m_saveDataManager.GetStageSelectPortalId();
        }
        StartStageByIndex(continueStageIndex);
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
    const std::wstring& savedStageSelectId = m_saveDataManager.GetStageSelectId();
    if (!savedStageSelectId.empty() &&
        m_saveDataManager.IsStageUnlocked(savedStageSelectId))
    {
        const std::size_t savedStageSelectIndex = m_stageManager.FindStageIndexById(savedStageSelectId);
        if (savedStageSelectIndex < m_stageManager.GetStageCount() &&
            IsStageSelectId(m_stageManager.GetStage(savedStageSelectIndex).id))
        {
            return savedStageSelectIndex;
        }
    }

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

        case IDC_BUTTON_UNLOCK_ALL_WEAPONS:
            UnlockAllWeapons();
            return TRUE;

        case IDC_BUTTON_UNLOCK_ALL_SKILLS:
            UnlockAllSkills();
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
    case WM_SETCURSOR:
    {
        if (LOWORD(lParam) == HTCLIENT)
        {
            Instance().ApplyMouseCursor();
            return TRUE;
        }
        break;
    }

    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    {
        Instance().ApplyMouseCursor();
        break;
    }

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

HCURSOR GameApp::GetActiveMouseCursor() const
{
    if (m_gameState == GameState::Loading && m_hLoadingCursor != NULL)
    {
        return m_hLoadingCursor;
    }

    if ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0 && m_hPressedCursor != NULL)
    {
        return m_hPressedCursor;
    }

    return m_hCursor;
}

void GameApp::ApplyMouseCursor()
{
    HCURSOR cursor = GetActiveMouseCursor();
    if (cursor != NULL && GetCursor() != cursor)
    {
        SetCursor(cursor);
    }
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
    m_pauseMenu.CloseImmediately();
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

    if (m_render.GetFadeAlpha() >= 1.0f)
    {
        BeginStageLoadingScreen();
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

void GameApp::BeginBossDefeat(const D3DXVECTOR3& bossPosition)
{
    m_pauseMenu.CloseImmediately();
    m_mouseCursorVisible = false;
    InputDevice::Mouse::SetVisible(false);
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_bossDefeatFrame = 0;
    m_bossDefeatPosition = bossPosition;
    m_bossDefeatCameraStartPos = m_render.GetCameraPos();
    m_bossDefeatCameraStartTarget = m_render.GetLookAtPos();
    m_bossDefeatStoredFovDegrees = m_render.GetCameraHorizontalFovDegrees();
    m_bossDefeatUsesFixedCamera = m_useFixedCamera;
    m_bossDefeatCameraEndTarget = bossPosition + D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    m_bossDefeatCameraEndPos = m_bossDefeatCameraStartPos;

    if (!m_bossDefeatUsesFixedCamera)
    {
        D3DXVECTOR3 cameraOffset = m_bossDefeatCameraStartPos - m_bossDefeatCameraStartTarget;
        if (D3DXVec3LengthSq(&cameraOffset) <= 0.0001f)
        {
            cameraOffset = D3DXVECTOR3(0.0f, 2.0f, 5.0f);
        }
        m_bossDefeatCameraEndPos = m_cameraMover.ResolvePosition(
            m_bossDefeatCameraEndTarget,
            m_bossDefeatCameraEndTarget + cameraOffset);
    }

    m_render.SetCameraShakeDuration(0.24f);
    m_render.SetCameraShakeIntensity(0.045f);
    m_render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Fog,
                                 bossPosition + D3DXVECTOR3(0.0f, 0.8f, 0.0f));
    GameAudio::BeginBgmFadeOut(kBossDefeatBgmFadeFrames);
    SetPlayerAnimationState(PlayerAnimState::Idle, 1.0f);
    m_gameState = GameState::BossDefeat;
}

void GameApp::UpdateBossDefeat()
{
    if (m_bossDefeatFrame == kBossDefeatFogRefreshFrame)
    {
        m_render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Fog,
                                     m_bossDefeatPosition + D3DXVECTOR3(0.0f, 1.2f, 0.0f));
    }

    if (m_bossDefeatFrame == kBossDefeatSoundFrame)
    {
        GameAudio::PlayBossDefeat();
    }

    GameAudio::UpdateBgmFadeOut();
    m_enemyManager.Update(m_render, m_playerMover.GetPosition(), true);
    m_enemyManager.SyncMeshes(m_render);
    UpdatePlayerMeshAndCamera(m_playerMover.GetPosition());

    if (!m_bossDefeatUsesFixedCamera)
    {
        float rawCameraT = static_cast<float>(m_bossDefeatFrame + 1) /
                           static_cast<float>(kBossDefeatCameraMoveFrames);
        if (rawCameraT > 1.0f)
        {
            rawCameraT = 1.0f;
        }
        const float cameraT = SmoothStep01(rawCameraT);
        const D3DXVECTOR3 cameraPosition = LerpVector3(m_bossDefeatCameraStartPos,
                                                       m_bossDefeatCameraEndPos,
                                                       cameraT);
        const D3DXVECTOR3 cameraTarget = LerpVector3(m_bossDefeatCameraStartTarget,
                                                     m_bossDefeatCameraEndTarget,
                                                     cameraT);
        const float fovDegrees = LerpFloat(m_bossDefeatStoredFovDegrees,
                                           kBossDefeatTargetFovDegrees,
                                           cameraT);
        m_render.SetCamera(cameraPosition, cameraTarget);
        m_render.SetCameraHorizontalFovDegrees(fovDegrees);
    }

    if (m_bossDefeatFrame >= kBossDefeatDurationFrames)
    {
        m_render.SetCameraShakeDuration(0.0f);
        m_render.SetCameraShakeIntensity(0.0f);
        m_render.SetCameraHorizontalFovDegrees(m_bossDefeatStoredFovDegrees);
        m_gameState = GameState::StageClear;
        m_stageClearProcessed = false;
        m_stageClearFrame = 0;
        return;
    }

    m_render.Draw();
    ++m_bossDefeatFrame;
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

    const bool isFinalStage = m_stageManager.GetCurrentStage().id == L"4-8";

    bool proceedToNextScene = false;
    int autoFrame = kStageClearFinalAutoFrame;
    if (!m_stageClearWasFirstClear)
    {
        autoFrame = kStageClearReplayFinalAutoFrame;
    }
    if (m_stageClearFrame >= autoFrame)
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

        if (m_stageClearWasFirstClear)
        {
            const std::wstring storyScriptPath = GetStageStoryScriptPath(clearedStageId, L"After");
            if (!storyScriptPath.empty())
            {
                m_slideShowManager.Start(storyScriptPath);
                m_slideShowManager.SetStopOnFinish(false);
                m_startStageAfterSlideShow = true;
                m_gameState = GameState::SlideShow;
                return;
            }
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
    m_stageClearVisualOffsetY = 0.0f;
    m_stageClearCameraStartPos = m_render.GetCameraPos();
    m_stageClearCameraStartTarget = m_render.GetLookAtPos();
    m_stageClearStoredFovDegrees = m_render.GetCameraHorizontalFovDegrees();

    if (!m_stageClearWasFirstClear)
    {
        m_stageClearCameraEndPos = m_stageClearCameraStartPos;
        m_stageClearCameraEndTarget = m_stageClearCameraStartTarget;
        m_stageClearReplayPhase = StageClearReplayPhase::WaitingToJump;
        m_stageClearReplayPhaseFrame = 0;
        m_stageClearReplayPlayerHidden = false;
        RemoveGoalArrow();
        HideStageClearReplayEquipment();
        m_skullManager.ReleaseHeld(m_render, m_playerMover.GetPosition());
        if (m_playerMeshId >= 0)
        {
            m_render.StopMeshMixSkinAnimBlink(m_playerMeshId);
            m_render.SetMeshMixSkinAnimWhiteFlash(m_playerMeshId, false);
            m_render.SetMeshMixSkinAnimEnabled(m_playerMeshId, true);
        }
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
        if (m_stageClearReplayPhase == StageClearReplayPhase::WaitingToJump)
        {
            ++m_stageClearReplayPhaseFrame;
            if (m_stageClearReplayPhaseFrame >= kStageClearReplayJumpDelayFrames)
            {
                m_stageClearReplayPhase = StageClearReplayPhase::Ascending;
                m_stageClearReplayPhaseFrame = 0;
                if (m_playerMeshId >= 0)
                {
                    m_playerAnimState = PlayerAnimState::Jump;
                    m_playerAnimationSpeed = kStageClearReplayJumpAnimationSpeed;
                    m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, m_playerAnimationSpeed);
                    m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerJumpAnimName);
                }
                GameAudio::PlayJump();
                m_render.SetCameraShakeDuration(0.08f);
                m_render.SetCameraShakeIntensity(0.012f);
            }
        }
        else if (m_stageClearReplayPhase == StageClearReplayPhase::Ascending)
        {
            float jumpT = static_cast<float>(m_stageClearReplayPhaseFrame + 1) /
                          static_cast<float>(kStageClearReplayAscentFrames);
            if (jumpT > 1.0f)
            {
                jumpT = 1.0f;
            }
            m_stageClearVisualOffsetY =
                kStageClearReplayJumpHeight * (2.0f * jumpT - jumpT * jumpT);
            UpdatePlayerMeshAndCamera(m_playerMover.GetPosition());

            const StageManager::StageData& stage = m_stageManager.GetCurrentStage();
            if (stage.playerPointLightEnabled)
            {
                D3DXVECTOR3 lightPosition = m_playerMover.GetPosition();
                lightPosition.y += m_stageClearVisualOffsetY + kPlayerPointLightHeight;
                m_render.SetPointLightPositionByOwnerTag(kPlayerPointLightOwnerTag, lightPosition);
            }

            ++m_stageClearReplayPhaseFrame;
            if (m_stageClearReplayPhaseFrame >= kStageClearReplayAscentFrames)
            {
                m_stageClearVisualOffsetY = kStageClearReplayJumpHeight;
                m_stageClearReplayPhase = StageClearReplayPhase::ApexWhite;
                m_stageClearReplayPhaseFrame = 0;
                if (m_playerMeshId >= 0)
                {
                    m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 0.0f);
                    m_render.SetMeshMixSkinAnimWhiteFlash(m_playerMeshId, true);
                }
                GameAudio::PlayStageSelectConfirm();
            }
        }
        else if (m_stageClearReplayPhase == StageClearReplayPhase::ApexWhite)
        {
            ++m_stageClearReplayPhaseFrame;
            if (m_stageClearReplayPhaseFrame >= kStageClearReplayWhiteFrames)
            {
                if (m_playerMeshId >= 0)
                {
                    m_render.SetMeshMixSkinAnimWhiteFlash(m_playerMeshId, false);
                    m_render.SetMeshMixSkinAnimEnabled(m_playerMeshId, false);
                }
                HideStageClearReplayEquipment();
                m_render.RemovePointLightsByOwnerTag(kPlayerPointLightOwnerTag);
                m_stageClearReplayPlayerHidden = true;
                m_stageClearReplayPhase = StageClearReplayPhase::Vanished;
                m_stageClearReplayPhaseFrame = 0;
            }
        }
        else if (m_stageClearReplayPhase == StageClearReplayPhase::Vanished)
        {
            ++m_stageClearReplayPhaseFrame;
        }
        else
        {
            throw std::runtime_error("Invalid replay stage-clear phase.");
        }

        m_render.SetCamera(m_stageClearCameraStartPos, m_stageClearCameraStartTarget);
        m_render.SetCameraHorizontalFovDegrees(m_stageClearStoredFovDegrees);
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
    if (!m_stageClearReplayPlayerHidden)
    {
        m_stageClearVisualOffsetY = 0.0f;
        UpdatePlayerMeshAndCamera(m_playerMover.GetPosition());
    }
    m_render.SetCameraHorizontalFovDegrees(m_stageClearStoredFovDegrees);
    m_render.SetCamera(m_stageClearCameraStartPos, m_stageClearCameraStartTarget);
    m_render.SetCameraShakeDuration(0.0f);
    m_render.SetCameraShakeIntensity(0.0f);
    if (m_playerMeshId >= 0 && !m_stageClearReplayPlayerHidden)
    {
        SetPlayerAnimationState(PlayerAnimState::Idle, 1.0f);
    }
    m_stageClearFrame = 0;
}

void GameApp::HideStageClearReplayEquipment()
{
    if (m_stickMeshId >= 0)
    {
        m_render.SetMeshMixEnabled(m_stickMeshId, false);
    }
    if (m_saberMeshId >= 0)
    {
        m_render.SetMeshMixEnabled(m_saberMeshId, false);
    }
    if (m_gunMeshId >= 0)
    {
        m_render.SetMeshMixEnabled(m_gunMeshId, false);
    }
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

void GameApp::InitializePortal(const D3DXVECTOR3& clearPosition)
{
    m_render.RemovePointLightsByOwnerTag(kPortalPillarLightOwnerTag);
    m_portalBasePosition = D3DXVECTOR3(clearPosition.x, clearPosition.y - 1.0f, clearPosition.z);
    const D3DXVECTOR3 portalStepsPosition =
        m_portalBasePosition + D3DXVECTOR3(0.0f, kPortalStepsPositionYOffset, 0.0f);

    m_portalStepsMeshId = m_render.AddMeshMix(kPortalStepsModelPath,
                                               portalStepsPosition,
                                               D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                               kPortalStepsScale,
                                               -1.0f,
                                               false,
                                               false,
                                               false);

    m_portalCollisionId = PhysicsWorld::Load(kPortalStepsCollisionPath.c_str(),
                                              PhysicsWorld::ObjectType::Slide,
                                              0.5f);
    if (m_portalCollisionId >= 0)
    {
        PhysicsWorld::SetTransform(m_portalCollisionId,
                                   portalStepsPosition,
                                   D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                   D3DXVECTOR3(kPortalStepsScale,
                                               kPortalStepsScale,
                                               kPortalStepsScale));
    }

    m_portalPillarShown = false;
    m_portalFlagShown = false;
    m_portalClearDelayFrames = 0;
    m_stageClearInputLocked = false;
}


void GameApp::RemovePortal()
{
    m_render.RemovePointLightsByOwnerTag(kPortalPillarLightOwnerTag);
    if (m_portalStepsMeshId >= 0)
    {
        m_render.RemoveMeshMix(m_portalStepsMeshId);
        m_portalStepsMeshId = -1;
    }
    if (m_portalPillarMeshId >= 0)
    {
        m_render.RemoveMeshMix(m_portalPillarMeshId);
        m_portalPillarMeshId = -1;
    }
    if (m_portalFlagMeshId >= 0)
    {
        m_render.RemoveMeshMixSkinAnim(m_portalFlagMeshId);
        m_portalFlagMeshId = -1;
    }
    m_portalCollisionId = -1;
    m_portalPillarShown = false;
    m_portalFlagShown = false;
    m_portalClearDelayFrames = 0;
    m_stageClearInputLocked = false;
}


void GameApp::UpdatePortal()
{
    if (m_portalStepsMeshId < 0)
    {
        return;
    }

    // Step 1: Show the light pillar when all enemies are dead.
    if (!m_portalPillarShown && !m_portalFlagShown)
    {
        bool allDead = true;
        for (const auto& enemy : m_enemyManager.GetEnemies())
        {
            if (!enemy->IsDead())
            {
                allDead = false;
                break;
            }
        }
        if (allDead)
        {
            const D3DXVECTOR3 pillarPos = m_portalBasePosition;
            m_portalPillarMeshId = m_render.AddMeshMix(kPortalPillarModelPath,
                                                        pillarPos,
                                                        D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                        1.0f,
                                                        -1.0f,
                                                        false,
                                                        false,
                                                        false);
            const D3DXVECTOR3 pillarLightPosition =
                pillarPos + D3DXVECTOR3(0.0f, kPortalPillarLightHeight, 0.0f);
            const D3DXCOLOR pillarLightColor(0.55f, 0.82f, 1.0f, 1.0f);
            m_render.RemovePointLightsByOwnerTag(kPortalPillarLightOwnerTag);
            m_render.AddPointLight(pillarLightPosition,
                                   kPortalPillarLightBrightness,
                                   pillarLightColor,
                                   NSRender::PointLightShape::Point,
                                   12.0f,
                                   10.0f,
                                   10.0f,
                                   D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                   kPortalPillarLightRange,
                                   kPortalPillarLightOwnerTag);
            m_portalPillarShown = true;
        }
    }

    // Step 2: When the player touches the light pillar, show the flag.
    const D3DXVECTOR3 playerPos = m_playerMover.GetPosition();
    const float dx = playerPos.x - m_portalBasePosition.x;
    const float dz = playerPos.z - m_portalBasePosition.z;
    const bool playerTouchingPillar =
        dx * dx + dz * dz <= kPortalPillarTouchRadius * kPortalPillarTouchRadius;

    if (m_portalPillarShown && !m_portalFlagShown && playerTouchingPillar)
    {
        const float topY = m_portalBasePosition.y + 1.5f;
        const D3DXVECTOR3 flagPos(m_portalBasePosition.x,
                                  topY + 0.02f,
                                  m_portalBasePosition.z);
        m_portalFlagMeshId = m_render.AddMeshMixSkinAnim2(
            kPortalFlagModelPath,
            kPortalFlagAnimCsvPath,
            flagPos,
            D3DXVECTOR3(0.0f, 0.0f, 0.0f),
            1.0f,
            NSRender::AnimSetMap(),
            -1.0f,
            false,
            false);
        if (m_portalFlagMeshId >= 0)
        {
            m_render.PlayMeshMixSkinAnimAnimation(m_portalFlagMeshId, L"wave");
        }
        m_portalFlagShown = true;
        m_portalClearDelayFrames = kPortalClearDelayFrames;
        m_stageClearInputLocked = true;
        m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_pendingJump = false;
    }

    // Step 3: Remove the light pillar automatically after the flag appears.
    if (m_portalFlagShown && m_portalPillarShown && m_portalPillarMeshId >= 0)
    {
        m_render.RemoveMeshMix(m_portalPillarMeshId);
        m_render.RemovePointLightsByOwnerTag(kPortalPillarLightOwnerTag);
        m_portalPillarMeshId = -1;
        m_portalPillarShown = false;
    }
    // Step 4: Count down after the flag has appeared.
    if (m_portalFlagShown && m_portalClearDelayFrames > 0)
    {
        --m_portalClearDelayFrames;
    }
}


bool GameApp::IsStageClearReached()
{
    return m_portalFlagShown && m_portalClearDelayFrames <= 0;
}

void GameApp::ApplyLavaDamageToEnemies()
{
    for (const auto& enemy : m_enemyManager.GetEnemies())
    {
        if (enemy == nullptr || enemy->IsDead())
        {
            continue;
        }

        if (enemy->GetType() == L"bird")
        {
            continue;
        }

        const D3DXVECTOR3 enemyPosition = enemy->GetPosition();
        const float enemyRadius = enemy->GetPhysicsRadius();
        const float enemyHeight = enemy->GetHeight();
        if (enemyRadius <= 0.0f || enemyHeight <= 0.0f)
        {
            continue;
        }

        int lavaDamage =
            m_lavaZoneManager.GetContactDamageForCylinder(
                enemyPosition,
                enemyRadius,
                enemyHeight);
        const int lavaFloodDamage =
            m_lavaFloodManager.GetContactDamageForCylinder(
                enemyPosition,
                enemyRadius,
                enemyHeight);
        if (lavaFloodDamage > lavaDamage)
        {
            lavaDamage = lavaFloodDamage;
        }
        const int lavaRiseDamage =
            m_lavaRiseManager.GetContactDamageForCylinder(
                enemyPosition,
                enemyRadius,
                enemyHeight);
        if (lavaRiseDamage > lavaDamage)
        {
            lavaDamage = lavaRiseDamage;
        }

        if (lavaDamage > 0)
        {
            enemy->TakeDamageWithoutFacing(m_render, enemy->GetHp());
        }
    }
}

void GameApp::ProcessEnemyAttackHits()
{
    for (auto& enemy : m_enemyManager.GetEnemies())
    {
        EnemyBase::AttackHit hit;
        if (!enemy->ConsumeAttackHit(&hit))
        {
            continue;
        }
        if (m_playerInvincibleFrames > 0 ||
            m_pickupManager.IsStarActive() ||
            enemy->IsDead())
        {
            continue;
        }

        GameAudio::PlayEnemyAttack();
        DamagePlayerHp(hit.damage);
        m_playerInvincibleFrames = kPlayerInvincibleDuration;
        if (m_playerMeshId >= 0)
        {
            m_render.StartMeshMixSkinAnimBlink(m_playerMeshId,
                                               kPlayerInvincibleDuration,
                                               2);
        }

        if (hit.knockbackFrames > 0)
        {
            m_playerKnockbackFrames = hit.knockbackFrames;
            D3DXVECTOR3 knockbackDirection = m_playerMover.GetPosition() - hit.sourcePosition;
            knockbackDirection.y = 0.0f;
            if (D3DXVec3LengthSq(&knockbackDirection) > 0.0001f)
            {
                D3DXVec3Normalize(&knockbackDirection, &knockbackDirection);
            }
            else
            {
                knockbackDirection = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
            }
            m_playerKnockbackDir = knockbackDirection;
        }
        if (hit.slowFrames > m_playerSlowFrames)
        {
            m_playerSlowFrames = hit.slowFrames;
        }
        break;
    }
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

    if (m_pauseMenu.IsOpen() || m_craftMenu.IsOpen() || m_explanationManager.IsActive() ||
        m_playerDeathPending || m_qte != nullptr)
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

    m_playerDeathPending = true;
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerKnockbackFrames = 0;
    m_playerSlowFrames = 0;
    m_hitStopFrames = 0;
    m_pendingHitStopFrames = 0;
    m_hitStopPlayerAnimationPaused = false;
    m_pickupManager.ResetPlayerEffects();
    m_playerAttackController.Reset();
    ResetBusterAimState();
    m_baseBombCapacity = 1;
    m_baseBusterRapidLevel = 1;
    m_bombCapacity = 1;
    m_busterRapidLevel = 1;
    m_busterCooldownFrames = 0;
    RefillWeaponAmmo();
    ClearBombs();
    ClearBusters();
    m_skullManager.ReleaseHeld(m_render, m_playerMover.GetPosition());

    // 最後の残機では、ゲームオーバー画面へ移る前に死亡したシーンを約3秒間表示する。
    if (m_player.GetLives() <= 1)
    {
        if (!m_playerFallingDead)
        {
            SetPlayerAnimationState(PlayerAnimState::Death, 1.0f);
        }
        m_respawnPhase = RespawnPhase::GameOverWait;
        m_respawnFadeFrames = kGameOverWaitFrames;
        return;
    }

    // シーン更新は止めず（SetSceneUpdatePaused は使わない）、死亡モーションと暗転を進める。
    // 落下死ではカメラ停止後の1秒待機が完了しているため、ここで暗転を開始する。
    if (m_playerFallingDead)
    {
        m_respawnPhase = RespawnPhase::FadeOut;
        m_respawnFadeFrames = kRespawnFadeOutFrames;
        m_render.StartFadeOut(static_cast<float>(kRespawnFadeOutFrames) / 60.0f);
    }
    else
    {
        SetPlayerAnimationState(PlayerAnimState::Death, 1.0f);
        m_respawnPhase = RespawnPhase::DeathMotion;
        m_respawnFadeFrames = kPlayerDeathMotionFrames;
    }
}

void GameApp::CompletePlayerDeath()
{
    m_player.Die();
    m_playerFallingDead = false;
    m_fallDeathFrames = 0;

    if (m_player.IsGameOver())
    {
        // GameOver へ移行。以降は UpdateGameOver がフェードを担当する。
        m_playerDeathPending = false;
    m_warpPhase = WarpPhase::None;
    m_warpFadeFrames = 0;
        m_respawnPhase = RespawnPhase::None;
        StartGameOverSequence();
        return;
    }

    // 通常リスポーン時は m_playerDeathPending を true のまま残す。
    // メインループの死亡ブロックが HoldBlack → FadeIn を駆動し、
    // フェードイン完了時にフラグを戻す（ここで戻すと暗転が解除されない）。

    // 暗転中にスタート地点へ瞬間移動でリスポーン
    const StageManager::StageData& stage = m_stageManager.GetCurrentStage();
    const D3DXVECTOR3 respawnPos = stage.playerStartPosition;
    m_playerMover.Reset(respawnPos);
    m_player.ResetHp();
    m_hpBar.Reset();
    SetPlayerAnimationState(PlayerAnimState::Idle, 1.0f);

    // 無敵＋点滅
    m_playerInvincibleFrames = kRespawnInvincibleFrames;
    if (m_playerMeshId >= 0)
    {
        m_render.StartMeshMixSkinAnimBlink(m_playerMeshId, kRespawnInvincibleFrames, 4);
    }

    // 敵、破壊可能オブジェクト、取得済みスター、ドクロを再配置
    m_enemyManager.LoadForStage(m_render, GetEnemyCsvPathForStage(m_stageManager.GetCurrentStage()));
    m_destructibleManager.ResetForRespawn(m_render);
    m_collectibleManager.RefreshVisibility(m_destructibleManager);
    m_pickupManager.RespawnStars();
    m_skullManager.ResetForRespawn(m_render);

    // レバー2・3で操作した門を閉じた初期状態へ戻す。
    m_attackTriggerManager.ResetLevers(m_render, std::vector<int>{2, 3});

    // 各種状態リセット
    m_playerKnockbackFrames = 0;
    m_playerSlowFrames = 0;
    m_playerAttackController.Reset();
    ResetBusterAimState();
    m_damagePopupManager.Clear();

    // リスポーン時はカメラもステージ開始時と同じ初期位置（プレイヤー背後の視点）に戻す。
    // 死亡直前にマウスで回転したカメラ角度が残っていると、暗転明けにカメラが
    // 初期位置と異なる位置に見えるため、ステージ開始時と同じ yaw/pitch/距離 にリセットする。
    m_playerYaw = CalculatePlayerStartYaw(stage);
    if (!m_useFixedCamera)
    {
        m_cameraYaw = D3DX_PI - m_playerYaw;
        m_cameraPitch = m_initialCameraPitch;
        m_cameraDistance = m_initialCameraDistance;
    }

    // 真っ暗のうちにメッシュとカメラをリスポーン位置へ即時同期する。
    // 死亡ブロック中は通常更新（UpdatePlayerMeshAndCamera）が走らないため、
    // ここで同期しないと暗転解除後にプレイヤーが死亡位置から移動して見える。
    UpdatePlayerMeshAndCamera(respawnPos);
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

    m_pauseMenu.CloseImmediately();
    m_craftMenu.CloseImmediately();
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerKnockbackFrames = 0;
    m_playerSlowFrames = 0;
    m_playerInvincibleFrames = 0;
    m_playerAttackController.Reset();
    ResetBusterAimState();
    m_damagePopupManager.Clear();
    ClearBombs();
    ClearBusters();
    m_render.SetSceneUpdatePaused(false);
    m_render.StartFadeOut(0.3f);
    m_gameOverPhase = GameOverPhase::FadeOutToScreen;
    m_gameOverFadeFrames = kGameOverFadeFrames;
    m_gameOverPulseFrame = 0;
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
    const float pulseRadians = static_cast<float>(m_gameOverPulseFrame) *
                               D3DX_PI * 2.0f /
                               static_cast<float>(kGameOverOverlayPulseFrames);
    const int overlayAlpha = kGameOverOverlayBaseAlpha +
                             static_cast<int>(sinf(pulseRadians) *
                                              static_cast<float>(kGameOverOverlayPulseAlpha));

    m_render.DrawImageStretched(g_gameOverImagePath, 255);
    m_render.DrawImageStretched(g_gameOverOverlayImagePath, overlayAlpha);
    m_render.Draw();

    ++m_gameOverPulseFrame;
    if (m_gameOverPulseFrame >= kGameOverOverlayPulseFrames)
    {
        m_gameOverPulseFrame = 0;
    }
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

void GameApp::ApplyTitleRenderSettings()
{
    m_render.ClearPointLights();
    m_render.SetPostEffectSaturate(kTitleSaturationLevel);
    m_render.SetMeshMixShadowDarkness(kTitleShadowDarkness);
    m_render.SetLightBrightness(kTitleSunLightIntensity);
    m_render.SetAmbientLightBrightness(kTitleAmbientLightIntensity);
}

void GameApp::BeginReturnToTitle()
{
    if (m_stageTransitionAction != StageTransitionAction::None)
    {
        return;
    }

    m_pauseMenu.CloseImmediately();
    if (m_craftMenu.IsOpen())
    {
        m_craftMenu.CloseImmediately();
    }

    RestoreQteVisualEffectImmediate();
    if (m_qte != nullptr)
    {
        m_qte->Finalize();
        delete m_qte;
        m_qte = nullptr;
    }

    m_render.SetSceneUpdatePaused(false);
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_stageExitFrame = 0;
    m_stageExitVisualOffsetY = 0.0f;
    m_stageClearProcessed = false;
    m_stageClearFrame = 0;
    m_stageClearVisualOffsetY = 0.0f;
    m_playerDeathPending = false;
    m_warpPhase = WarpPhase::None;
    m_warpFadeFrames = 0;
    m_playerFallingDead = false;
    m_fallDeathFrames = 0;
    m_gameOverPhase = GameOverPhase::None;
    m_gameOverFadeFrames = 0;
    m_startStageAfterSlideShow = false;
    m_pendingStageIndexAfterSlideShow = static_cast<std::size_t>(-1);
    m_stageTransitionIndex = static_cast<std::size_t>(-1);
    m_stageTransitionAction = StageTransitionAction::ReturnToTitle;
    m_render.StartFadeOut(kStageSelectTransitionFadeDuration);
}

void GameApp::CompleteReturnToTitle()
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
    ResetBusterAimState();
    m_titleDeleteConfirmMode = false;
    m_titleLanguageSelectionMode = false;
    BuildTitleMainCommands();
    RefreshTitleCommands();
    ApplyTitleRenderSettings();
    m_mouseCursorVisible = true;
    InputDevice::Mouse::SetVisible(true);
    ApplyMouseCursor();
    m_gameState = GameState::Title;
    m_stageTransitionAction = StageTransitionAction::None;
    m_stageTransitionIndex = static_cast<std::size_t>(-1);
}

void GameApp::ReturnToTitleFromGameOver()
{
    BeginReturnToTitle();
}
bool GameApp::StartNextStage()
{
    if (!m_stageManager.MoveNextStage())
    {
        return false;
    }

    BeginStageLoadingScreen();
    m_render.Draw();
    LoadCurrentStageObjects();
    m_render.SetLoadingScreenProgress(90);
    m_render.SetFadeAlpha(1.0f);
    if (IsCurrentStageSelect() || IsBaseId(m_stageManager.GetCurrentStage().id))
    {
        m_gameState = GameState::Playing;
    }
    else
    {
        m_gameState = GameState::StageIntro;
    }
    m_stageTransitionAction = StageTransitionAction::WaitForStageLoad;
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
    std::size_t destinationIndex = m_stageManager.GetClearDestinationIndex(stageNumber);

    if (IsBaseId(clearedStageId))
    {
        const int world = GetWorldFromStageId(clearedStageId);
        if (world <= 0)
        {
            return false;
        }
        const std::wstring stageSelectId = L"select" + std::to_wstring(world);
        destinationIndex = m_stageManager.FindStageIndexById(stageSelectId);
    }

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
    BeginStageLoadingScreen();
    m_render.Draw();
    LoadCurrentStageObjects();
    m_render.SetLoadingScreenProgress(90);
    m_render.SetFadeAlpha(1.0f);
    if (IsCurrentStageSelect() || IsBaseId(m_stageManager.GetCurrentStage().id))
    {
        m_gameState = GameState::Playing;
    }
    else
    {
        m_gameState = GameState::StageIntro;
    }
    m_stageTransitionAction = StageTransitionAction::WaitForStageLoad;
    return true;
}

bool GameApp::IsBossStageNumber(int stageNumber)
{
    return stageNumber >= 8 && stageNumber % 8 == 0;
}

std::wstring GameApp::GetEnemyCsvPathForStage(const StageManager::StageData& stage) const
{
    if (IsBossStageNumber(stage.number) &&
        !IsStageSelectId(stage.id) &&
        !IsBaseId(stage.id) &&
        m_saveDataManager.IsStageCleared(stage.id))
    {
        // クリア済みのボスステージは雑魚のみの配置にする。
        const std::wstring folder = stage.enemyCsvPath.substr(0, stage.enemyCsvPath.rfind(L"\\"));
        return folder + L"\\EnemyPositionsCleared.csv";
    }
    return stage.enemyCsvPath;
}

std::wstring GameApp::GetBossClearedCsvPath(
    const StageManager::StageData& stage,
    const std::wstring& defaultCsvPath) const
{
    if (!IsBossStageNumber(stage.number) ||
        IsStageSelectId(stage.id) ||
        IsBaseId(stage.id) ||
        !m_saveDataManager.IsStageCleared(stage.id))
    {
        return defaultCsvPath;
    }

    const std::wstring extension = L".csv";
    const std::size_t extensionPosition = defaultCsvPath.rfind(extension);
    if (extensionPosition == std::wstring::npos)
    {
        std::abort();
    }

    const std::wstring clearedPath =
        defaultCsvPath.substr(0, extensionPosition) + L"Cleared.csv";
    std::wifstream file(NSRender::Util::GetExeDir() + clearedPath);
    if (file.is_open())
    {
        return clearedPath;
    }
    return defaultCsvPath;
}

StageManager::StageData GameApp::GetStageDataForLoad(
    const StageManager::StageData& stage) const
{
    StageManager::StageData result = stage;
    result.renderCsvPath = GetBossClearedCsvPath(stage, stage.renderCsvPath);
    result.physicsCsvPath = GetBossClearedCsvPath(stage, stage.physicsCsvPath);
    result.moveCsvPath = GetBossClearedCsvPath(stage, stage.moveCsvPath);
    result.collectibleCsvPath = GetBossClearedCsvPath(stage, stage.collectibleCsvPath);
    result.interactableCsvPath = GetBossClearedCsvPath(stage, stage.interactableCsvPath);
    result.starCsvPath = GetBossClearedCsvPath(stage, stage.starCsvPath);
    result.speedUpCsvPath = GetBossClearedCsvPath(stage, stage.speedUpCsvPath);
    result.destructibleCsvPath = GetBossClearedCsvPath(stage, stage.destructibleCsvPath);
    result.dashBoosterCsvPath = GetBossClearedCsvPath(stage, stage.dashBoosterCsvPath);
    result.lavaCsvPath = GetBossClearedCsvPath(stage, stage.lavaCsvPath);
    result.lavaFloodCsvPath = GetBossClearedCsvPath(stage, stage.lavaFloodCsvPath);
    result.lavaRiseCsvPath = GetBossClearedCsvPath(stage, stage.lavaRiseCsvPath);
    result.skullCsvPath = GetBossClearedCsvPath(stage, stage.skullCsvPath);
    result.pressurePlateCsvPath = GetBossClearedCsvPath(stage, stage.pressurePlateCsvPath);
    result.pushableBoxCsvPath = GetBossClearedCsvPath(stage, stage.pushableBoxCsvPath);
    result.attackTriggerCsvPath = GetBossClearedCsvPath(stage, stage.attackTriggerCsvPath);
    result.explanationCsvPath = GetBossClearedCsvPath(stage, stage.explanationCsvPath);
    result.warpBearCsvPath = GetBossClearedCsvPath(stage, stage.warpBearCsvPath);
    result.pointLightCsvPath = GetBossClearedCsvPath(stage, stage.pointLightCsvPath);
    return result;
}

void GameApp::LoadCurrentStageObjects()
{
    RestoreQteVisualEffectImmediate();

    if (m_playerMeshId >= 0)
    {
        m_render.SetMeshMixSkinAnimWhiteFlash(m_playerMeshId, false);
        m_render.SetMeshMixSkinAnimEnabled(m_playerMeshId, true);
        m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 1.0f);
    }
    m_stageClearReplayPhase = StageClearReplayPhase::None;
    m_stageClearReplayPhaseFrame = 0;
    m_stageClearReplayPlayerHidden = false;
    m_stageClearVisualOffsetY = 0.0f;

    const StageManager::StageData& stage = m_stageManager.GetCurrentStage();
    const StageManager::StageData loadStage = GetStageDataForLoad(stage);

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
    if (stage.weather == StageManager::StageWeather::Rain)
    {
        m_render.SetPostEffectGodRay(false);
    }
    ApplyStageEnvironmentLighting(stage.id);
    ConfigureStagePointLights(stage.id);
    LoadPointLightsFromCsv(loadStage.pointLightCsvPath);

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

    m_pickupManager.Clear();
    m_dashBoosterManager.Clear();
    m_pressurePlateManager.Clear(m_render);
    m_pushableBoxManager.Clear();
    m_attackTriggerManager.Clear(m_render);
    ClearBombs();
    ClearBusters();
    m_skullManager.Clear(m_render);
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
    if (m_gunMeshId >= 0)
    {
        m_render.DetachMeshFromBone(m_gunMeshId);
        m_render.RemoveMeshMix(m_gunMeshId);
        m_gunMeshId = -1;
    }

    LoadPlayerMeshForStage(IsStageSelectId(stage.id), stage.playerStartPosition);

    RemovePortal();

    RemoveStageSelectCubes();
    m_render.ClearCsvLoadedMeshes();
    m_render.LoadXFileListFromCsv(loadStage.renderCsvPath);
    m_render.LoadXFileListMoveFromCsv(loadStage.moveCsvPath);

    m_collectibleManager.LoadForStage(loadStage.collectibleCsvPath);
    m_interactionManager.LoadForStage(loadStage.interactableCsvPath);
    LoadStageSelectNavigation(stage.stageSelectNavigationCsvPath);
    m_lavaZoneManager.LoadForStage(loadStage.lavaCsvPath);

    m_pickupManager.LoadForStage(loadStage.starCsvPath, loadStage.speedUpCsvPath);
    m_dashBoosterManager.LoadForStage(loadStage.dashBoosterCsvPath);

    CreateStageSelectCubes();
    m_playerMover.Reset(stage.playerStartPosition);
    InitializeStageSelectCursor();

    m_mouseCursorVisible = IsCurrentStageSelect();
    InputDevice::Mouse::SetVisible(m_mouseCursorVisible);

    m_lavaFloodManager.Clear();
    m_lavaRiseManager.Clear();
    PhysicsWorld::ClearObjects();
    LoadPhysicsObjectsFromCsv(loadStage.physicsCsvPath);
    m_lavaFloodManager.LoadForStage(m_render, loadStage.lavaFloodCsvPath);
    m_lavaRiseManager.LoadForStage(m_render, loadStage.lavaRiseCsvPath);
    m_skullManager.LoadForStage(m_render, loadStage.skullCsvPath);
    m_pressurePlateManager.LoadForStage(m_render, loadStage.pressurePlateCsvPath);
    m_pushableBoxManager.LoadForStage(m_render, loadStage.pushableBoxCsvPath);
    m_attackTriggerManager.LoadForStage(m_render, loadStage.attackTriggerCsvPath);
    m_explanationManager.LoadForStage(stage.id, loadStage.explanationCsvPath);
    m_warpBearManager.LoadForStage(loadStage.warpBearCsvPath);

    if (!IsStageSelectId(stage.id) &&
        !IsBaseId(stage.id) &&
        ShouldUseGoalPortal())
    {
        InitializePortal(stage.clearPosition);
    }

    m_prevMovingPlatformPositions.clear();
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerYaw = CalculatePlayerStartYaw(stage);
    m_playerAnimState = PlayerAnimState::Idle;
    m_stageExitFrame = 0;
    m_stageExitVisualOffsetY = 0.0f;
    m_damagePopupManager.Clear();
    m_playerInvincibleFrames = 0;
    m_pickupManager.ResetTemporaryEffects();
    RestoreTemporaryPowerUps();
    m_playerKnockbackFrames = 0;
    m_playerSlowFrames = 0;
    m_playerAttackController.Reset();
    ResetBusterAimState();
    m_playerAttackController.SelectClubCategory();
    m_playerDeathPending = false;
    m_warpPhase = WarpPhase::None;
    m_warpFadeFrames = 0;
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
    m_enemyManager.LoadForStage(m_render, GetEnemyCsvPathForStage(stage));

    m_destructibleManager.LoadForStage(m_render, loadStage.destructibleCsvPath);
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
            m_render.AttachMeshToBone(m_stickMeshId, m_playerMeshId, kPlayerRightWristBoneName,
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
            m_render.AttachMeshToBone(m_saberMeshId, m_playerMeshId, kPlayerRightWristBoneName,
                                      D3DXVECTOR3(0.0f, 0.0f, kSaberLocalRotateZ),
                                      D3DXVECTOR3(0.0f, 0.0f, 0.0f));
        }

        m_gunMeshId = m_render.AddMeshMix(kGunModelPath,
                                          kHiddenHeldWeaponPosition,
                                          D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                          kGunModelScale);
        if (m_gunMeshId >= 0)
        {
            m_render.SetMeshMixEnabled(m_gunMeshId, false);
            const float kGunLocalRotateZ = D3DX_PI * 0.5f;
            m_render.AttachMeshToBone(m_gunMeshId, m_playerMeshId, kPlayerRightWristBoneName,
                                      D3DXVECTOR3(0.0f, 0.0f, kGunLocalRotateZ),
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
    m_stageIntroZoomElapsed = 0;
    m_stageIntroStartFadeAlpha = m_render.GetFadeAlpha();
    if (!m_useFixedCamera)
    {
        // プレイヤーの進行方向と反対側にカメラを置き、演出後も背後視点を維持する。
        m_cameraYaw = D3DX_PI - m_playerYaw;
    }
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

    if (!m_useFixedCamera)
    {
        const D3DXVECTOR3 zoomCameraTarget = m_playerMover.GetPosition() + D3DXVECTOR3(0.0f, 1.2f, 0.0f);
        const float zoomHorizontalDistance = m_cameraDistance * cosf(m_cameraPitch);
        const D3DXVECTOR3 zoomOffset(sinf(m_cameraYaw) * zoomHorizontalDistance,
                                      sinf(m_cameraPitch) * m_cameraDistance,
                                      -cosf(m_cameraYaw) * zoomHorizontalDistance);
        const D3DXVECTOR3 zoomEndPos = m_cameraMover.ResolvePosition(zoomCameraTarget,
                                                                      zoomCameraTarget + zoomOffset);
        const D3DXVECTOR3 zoomStartPos = m_cameraMover.ResolvePosition(
            zoomCameraTarget,
            zoomCameraTarget + zoomOffset * kStageIntroZoomStartScale);
        const float zoomRawT = static_cast<float>(m_stageIntroZoomElapsed) /
                                static_cast<float>(kStageIntroZoomTotalFrames);
        const float zoomT = SmoothStep01(zoomRawT);
        const D3DXVECTOR3 zoomCameraPosition = LerpVector3(zoomStartPos, zoomEndPos, zoomT);
        m_render.SetCamera(zoomCameraPosition, zoomCameraTarget);
    }

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
    ++m_stageIntroZoomElapsed;
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

void GameApp::BeginWarp(const D3DXVECTOR3& targetPosition, const float targetRotationY)
{
    if (m_warpPhase != WarpPhase::None)
    {
        return;
    }

    m_warpTargetPosition = targetPosition;
    m_warpTargetRotationY = targetRotationY;
    m_warpPhase = WarpPhase::FadeOut;
    GameAudio::PlayWarp();
    m_warpFadeFrames = kWarpFadeOutFrames;
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerAttackController.Reset();
    m_playerKnockbackFrames = 0;
    m_playerSlowFrames = 0;
    m_render.StartFadeOut(kWarpFadeDurationSeconds);
}

void GameApp::UpdateWarp()
{
    if (m_warpPhase == WarpPhase::FadeOut)
    {
        --m_warpFadeFrames;
        if (m_warpFadeFrames <= 0 && m_render.GetFadeAlpha() >= 1.0f)
        {
            m_playerMover.Reset(m_warpTargetPosition);
            m_playerYaw = m_warpTargetRotationY;
            m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            m_pendingJump = false;
            UpdatePlayerMeshAndCamera(m_warpTargetPosition);
            m_warpPhase = WarpPhase::HoldBlack;
            m_warpFadeFrames = kWarpBlackHoldFrames;
        }
        return;
    }

    if (m_warpPhase == WarpPhase::HoldBlack)
    {
        --m_warpFadeFrames;
        if (m_warpFadeFrames <= 0)
        {
            m_warpPhase = WarpPhase::FadeIn;
            m_warpFadeFrames = kWarpFadeInFrames;
            m_render.StartFadeIn(kWarpFadeDurationSeconds);
        }
        return;
    }

    if (m_warpPhase == WarpPhase::FadeIn)
    {
        --m_warpFadeFrames;
        if (m_warpFadeFrames <= 0 && m_render.GetFadeAlpha() <= 0.0f)
        {
            m_warpPhase = WarpPhase::None;
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

                // 食らい判定は敵の衝突円柱全体。爆発位置の高さを体の範囲にクランプして距離判定する。
                const D3DXVECTOR3 enemyPos = enemy->GetPosition();
                const float enemyHalfHeight = enemy->GetHeight() * 0.5f;
                float targetY = bombPos.y;
                if (targetY < enemyPos.y - enemyHalfHeight)
                {
                    targetY = enemyPos.y - enemyHalfHeight;
                }
                else if (targetY > enemyPos.y + enemyHalfHeight)
                {
                    targetY = enemyPos.y + enemyHalfHeight;
                }

                D3DXVECTOR3 dir = D3DXVECTOR3(enemyPos.x, targetY, enemyPos.z) - bombPos;
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

                // 食らい判定は敵の衝突円柱全体。弾の高さを体の範囲にクランプして距離判定する。
                const D3DXVECTOR3 enemyPos = enemy->GetPosition();
                const float enemyHalfHeight = enemy->GetHeight() * 0.5f;
                float targetY = it->position.y;
                if (targetY < enemyPos.y - enemyHalfHeight)
                {
                    targetY = enemyPos.y - enemyHalfHeight;
                }
                else if (targetY > enemyPos.y + enemyHalfHeight)
                {
                    targetY = enemyPos.y + enemyHalfHeight;
                }

                D3DXVECTOR3 dir = D3DXVECTOR3(enemyPos.x, targetY, enemyPos.z) - it->position;
                const float dist = D3DXVec3Length(&dir);
                if (dist <= kBusterHitRadius)
                {
                    enemy->TakeDamage(m_render, kBusterDamage, it->position);
                    enemy->StartKnockbackFrom(it->position, 0.3f, 20);
                    m_damagePopupManager.Add(kBusterDamage, enemy->GetPosition(), false);
                    TryDropEnemyItem(*enemy);
                    GameAudio::PlayBusterHit();
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
                            GameAudio::PlayBusterHit();
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



