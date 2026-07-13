#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <tchar.h>
#include <unordered_map>
#include <vector>

#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLibInternal.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../SoundLib/SoundLib/SoundLib.h"
#include "../../RedFortressCommand/Command/Command.h"
#include "SlideShowManager.h"
#include "Player.h"
#include "StageManager.h"
#include "EnemyManager.h"
#include "DestructibleManager.h"
#include "PauseMenu.h"
#include "SaveDataManager.h"
#include "InventoryManager.h"
#include "CollectibleManager.h"
#include "InteractionManager.h"
#include "LavaZoneManager.h"
#include "CraftMenu.h"
#include "../../QTE_Module/QTE_Module/QTE_Module.h"
#include "StageEditor.h"
#include "HpBar.h"
#include "DamagePopupManager.h"
#include "PlayerAttackController.h"
#include "PickupManager.h"
#include "DashBoosterManager.h"

struct ActiveBomb
{
    int meshId = -1;
    D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    bool isGrounded = false;
    int remainingFrames = 0;
    int blinkTimer = 0;
};

struct ActiveBuster
{
    int meshId = -1;
    D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 direction = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float traveledDistance = 0.0f;
};

class GameApp
{
public:
    static GameApp& Instance();

    bool Initialize(HINSTANCE hInstance, int nCmdShow);
    void Run();
    void Finalize();

private:
    GameApp();
    ~GameApp();
    GameApp(const GameApp&) = delete;
    GameApp& operator=(const GameApp&) = delete;

    // ウィンドウ・メッセージ処理
    static LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool InitializeWindow(HINSTANCE hInstance, int nCmdShow);
    void ProcessMessages();

    // ゲームループ
    void Update();
    void Draw();

    // ダイアログ
    static INT_PTR CALLBACK SettingsDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
    INT_PTR OnSettingsDialog(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

    // コマンド UI 実装
    class CommandFont;
    class CommandSprite;
    class CommandSE;
    class QteSprite;

    // ヘルパー
    void UpdateCameraByInput();
    void UpdatePlayerByInput();
    void UpdateTitleByInput();
    void BeginStageExit();
    void UpdateStageExit();
    void UpdateStageClear();
    bool IsStageClearReached();
    void BeginStageClearVisual();
    void UpdateStageClearVisual();
    void RestoreStageClearVisual();
    bool StartNextStage();
    std::wstring GetStageStoryScriptPath(const std::wstring& stageId,
                                         const std::wstring& timing) const;
    bool StartStageAfterClear();
    bool MoveToStageAfterClear();
    void LoadCurrentStageObjects();
    void BeginStageIntro();
    void UpdateStageIntro();
    void DamagePlayerHp(int amount);
    void HealPlayerHp(int amount);
    void HandlePlayerDeath();
    void CompletePlayerDeath();
    void StartGameOverSequence();
    void UpdateGameOver();
    void DrawGameOverScreen();
    bool IsGameOverActionTriggered() const;
    void ReturnToTitleFromGameOver();
    void PopulateStageCombo(HWND hDlg);
    std::wstring BuildStageComboText(const StageManager::StageData& stage) const;
    void PopulateUnlockStageCombo(HWND hDlg);
    void PopulateSpeedLevelCombo(HWND hDlg);
    void ApplySelectedSpeedLevel(HWND hDlg);
    void UnlockStagesUpToSelected(HWND hDlg);
    void AllUnlockStages(HWND hDlg);
    bool StartStageByIndex(std::size_t stageIndex);
    bool StartStageByIndexImmediate(std::size_t stageIndex);
    void StartNewGame();
    std::size_t GetContinueStartStageIndex() const;
    void MoveToSelectedStage(HWND hDlg);
    void RefreshTitleCommands();
    void DrawTitleScreen();
    void DrawStageTitle();
    void DrawStageClear();
    void DrawEndingFin();
    void BuildTitleMainCommands();
    void BuildTitleConfirmCommands();
    void BuildTitleLanguageCommands();
    void EnterDeleteConfirmation();
    void ExitDeleteConfirmation();
    void ExitTitleLanguageSelection();
    void ExecuteDeleteSaveData();
    void ExecuteTitleCommand(const std::wstring& commandId);
    POINT ConvertMouseToBaseResolution(int clientX, int clientY);
    D3DXVECTOR3 GetCameraPlanarForward();
    D3DXVECTOR3 GetCameraPlanarRight(const D3DXVECTOR3& forward);
    int DamageEnemiesInAttackRange(const PlayerAttackDefinition& attackDefinition);
    void InitializeCameraFromRenderSettings();
    void InitializePlayerPhysics();
    void ApplyUnlockedAbilities();
    void LoadPhysicsObjectsFromCsv(const std::wstring& csvPath);
    void UpdatePlayerMeshAndCamera(const D3DXVECTOR3& previousRenderPosition);
    void UpdatePlayerMeshVisibility();
    void UpdateHeldWeaponVisibility();
    bool IsCurrentStageSelect() const;
    bool IsStagePortalSelectable(const std::wstring& portalId) const;
    bool AreAllStageEnemiesDefeated() const;
    bool ShouldShowGoalArrow() const;
    void EnsureGoalArrow();
    void RemoveGoalArrow();
    void UpdateGoalArrow();
    void TryDropEnemyItem(const EnemyBase& enemy);
    void InitializeStageSelectCursor();
    void SyncStageSelectPlayerToPortal(bool immediate);
    void MoveStageSelectCursorByDirection(float directionX, float directionY);
    void UpdateStageSelectCursorByInput();
    bool MoveToSelectedStagePortal();
    std::wstring GetSelectedStagePortalDisplayName() const;
    void DrawStageSelectCursor();
    void CreateStageSelectCubes();
    void RemoveStageSelectCubes();
    bool PlaceBomb(const D3DXVECTOR3& position);
    void UpdateBombPhysics(ActiveBomb& bomb);
    void UpdateBombs();
    void ClearBombs();
    void SpawnBuster(const D3DXVECTOR3& position, const D3DXVECTOR3& direction);
    void UpdateBusters();
    void ClearBusters();
    void DrawAmmoGauge();
    int GetCurrentAmmo() const;
    int GetCurrentAmmoMax() const;
    int GetCurrentWorld() const;
    void RefillWeaponAmmo();
    bool RecoverWeaponAmmoFromPickup();
    void LoadItemNameCatalog();
    std::wstring GetItemDisplayName(const std::wstring& itemId) const;
    void HandleItemCollected(const std::wstring& itemId, int count);
    bool HandleInventoryItemUse(const std::wstring& itemId);
    bool TryUseRecoveryItemFromKey();
    void MaximizeTemporaryPowerUps();
    void RestoreTemporaryPowerUps();
    void ShowItemPickupMessage(const std::wstring& itemId, int count);
    void DrawItemPickupMessage();
    void BeginQteVisualEffect();
    void EndQteVisualEffect();
    void RestoreQteVisualEffectImmediate();
    void UpdateQteVisualEffect();
    void ApplyQteVisualEffect(float saturation, float fovDegrees);
    void UpdateDashParticleEffect();
    void BeginHitStop(int frames);
    void StartHitStopNow(int frames);
    void UpdateHitStop();
    bool IsHitStopActive() const;
    int GetHitStopFrames(PlayerAttackType attackType) const;

    enum class PlayerAnimState { Idle, Walk, Run, Jump, Attack, Dash };
    enum class GameState { Loading, Title, SlideShow, StageIntro, Playing, StageExit, StageClear, GameOver, Ending, EndingFin };
    enum class GameOverPhase { None, FadeOutToScreen, FadeInScreen, WaitingInput, FadeOutToTitle };
    enum class StageIntroPhase { LetterboxIn, Hold, LetterboxOut };
    enum class TitleLanguage { English, Japanese };
    enum class QteVisualPhase { None, Active, Restoring };

    void SetPlayerAnimationState(PlayerAnimState nextState, float animationSpeed);

    using PhysicsWorld = PhysicsLib::PhysicsLib;

    bool m_close = false;
    NSRender::Render m_render;
    Player m_player;
    StageManager m_stageManager;
    SaveDataManager m_saveDataManager;
    InventoryManager m_inventoryManager;
    CollectibleManager m_collectibleManager;
    InteractionManager m_interactionManager;
    LavaZoneManager m_lavaZoneManager;
    CraftMenu m_craftMenu;
    NS_QTE_Module::QTE_Module* m_qte = nullptr;
    QteVisualPhase m_qteVisualPhase = QteVisualPhase::None;
    int m_qteVisualFrame = 0;
    bool m_qteStoredSaturateEnabled = false;
    float m_qteStoredSaturate = 1.0f;
    float m_qteStoredFovDegrees = 90.0f;
    float m_qteVisualStartSaturate = 1.0f;
    float m_qteVisualStartFovDegrees = 90.0f;
    int m_playerMeshId = -1;
    bool m_playerIsSkinAnim = true;
    PhysicsLib::CharacterMover m_playerMover;
    PhysicsLib::CameraMover m_cameraMover;
    float m_cameraYaw = 0.0f;
    float m_cameraPitch = D3DXToRadian(18.0f);
    float m_cameraDistance = 10.0f;
    bool m_useFixedCamera = false;
    D3DXVECTOR3 m_fixedCameraPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_fixedCameraLookAt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float m_playerYaw = 0.0f;
    PlayerAnimState m_playerAnimState = PlayerAnimState::Idle;
    PlayerAttackController m_playerAttackController;
    GameState m_gameState = GameState::Loading;
    GameOverPhase m_gameOverPhase = GameOverPhase::None;
    int m_gameOverFadeFrames = 0;
    int m_itemUseCooldownFrames = 0;
    SlideShowManager m_slideShowManager;
    PauseMenu m_pauseMenu;
    bool m_mouseCursorVisible = false;
    bool m_remoteDesktopMode = false;
    HWND m_settingsDialog = NULL;
    HWND m_hWnd = NULL;
    HINSTANCE m_hInstance = NULL;
    D3DXVECTOR3 m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    bool m_pendingJump = false;
    bool m_dashParticleEmitted = false;
    int m_hitStopFrames = 0;
    int m_pendingHitStopFrames = 0;
    float m_playerAnimationSpeed = 1.0f;
    float m_hitStopStoredPlayerAnimationSpeed = 1.0f;
    bool m_hitStopPlayerAnimationPaused = false;
    std::unordered_map<int, D3DXVECTOR3> m_prevMovingPlatformPositions;
    HpBar m_hpBar;
    DamagePopupManager m_damagePopupManager;
    EnemyManager m_enemyManager;
    DestructibleManager m_destructibleManager;
    PickupManager m_pickupManager;
    DashBoosterManager m_dashBoosterManager;
    int m_playerInvincibleFrames = 0;
    int m_stickMeshId = -1;
    int m_saberMeshId = -1;
    int m_playerKnockbackFrames = 0;
    D3DXVECTOR3 m_playerKnockbackDir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    int m_respawnCameraDelayFrames = 0;
    int m_respawnCameraMoveFrames = 0;
    bool m_playerDeathPending = false;
    bool m_playerFallingDead = false;
    int m_fallDeathFrames = 0;
    int m_stageExitFrame = 0;
    float m_stageExitVisualOffsetY = 0.0f;
    bool m_stageClearProcessed = false;
    bool m_stageClearWasFirstClear = false;
    int m_stageClearFrame = 0;
    D3DXVECTOR3 m_stageClearCameraStartPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_stageClearCameraStartTarget = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_stageClearCameraEndPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_stageClearCameraEndTarget = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float m_stageClearStoredFovDegrees = 90.0f;
    bool m_startStageAfterSlideShow = false;
    std::size_t m_pendingStageIndexAfterSlideShow = static_cast<std::size_t>(-1);
    StageEditor m_stageEditor;
    D3DXVECTOR3 m_respawnCameraFromPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_respawnCameraFromTarget = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_respawnCameraToPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_respawnCameraToTarget = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    NSCommand::Command m_command;

    int m_commandFontId = -1;
    int m_titleFontId = -1;
    int m_stageTitleFontId = -1;
    int m_stageClearFontId = -1;
    int m_stageClearHintFontId = -1;
    int m_stageSelectFontId = -1;
    int m_stageSelectStartButtonFontId = -1;
    int m_stageSelectHintFontId = -1;
    int m_itemPickupMessageFontId = -1;
    std::unordered_map<std::wstring, std::wstring> m_itemDisplayNames;
    std::wstring m_itemPickupMessage;
    int m_itemPickupMessageFrames = 0;
    bool m_isMouseOverStartButton = false;
    int m_stageTitleFrame = 0;
    int m_stageIntroFontId = -1;
    StageIntroPhase m_stageIntroPhase = StageIntroPhase::LetterboxIn;
    int m_stageIntroFrame = 0;
    float m_stageIntroStartFadeAlpha = 0.0f;
    int m_goalMarkerMeshId = -1;
    int m_goalArrowMeshId = -1;
    int m_stagePortalCooldownFrames = 0;
    std::wstring m_lastSelectId;
    std::wstring m_selectedStagePortalId;
    std::wstring m_preferredStageSelectPortalId;
    D3DXVECTOR3 m_selectedStagePortalPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    bool m_hasSelectedStagePortal = false;
    D3DXVECTOR3 m_stageSelectPlayerMoveStartPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_stageSelectPlayerMoveTargetPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float m_stageSelectPlayerMoveElapsed = 0.0f;
    bool m_stageSelectPlayerMoveActive = false;
    std::vector<int> m_stageSelectCubeMeshIds;
    bool m_titleDeleteConfirmMode = false;
    bool m_titleLanguageSelectionMode = false;
    TitleLanguage m_titleLanguage = TitleLanguage::Japanese;
    std::vector<ActiveBomb> m_activeBombs;
    static const int kMaxBombs = 8;
    int m_bombCapacity = 1;
    int m_baseBombCapacity = 1;
    int m_bombAmmo = 10;
    std::vector<ActiveBuster> m_activeBusters;
    int m_busterCooldownFrames = 0;
    int m_busterRapidLevel = 1;
    int m_baseBusterRapidLevel = 1;
    int m_busterAmmo = 30;
};

