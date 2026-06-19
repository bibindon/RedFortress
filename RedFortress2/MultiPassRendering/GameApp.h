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
#include "PauseMenu.h"
#include "SaveDataManager.h"
#include "InventoryManager.h"
#include "CollectibleManager.h"
#include "InteractionManager.h"
#include "CraftMenu.h"
#include "../../QTE_Module/QTE_Module/QTE_Module.h"
#include "StageEditor.h"
#include "HpBar.h"
#include "DamagePopupManager.h"
#include "PlayerAttackController.h"

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
    void UpdateStageClear();
    bool IsStageClearReached();
    bool StartNextStage();
    void LoadCurrentStageObjects();
    void DamagePlayerHp(int amount);
    void HealPlayerHp(int amount);
    void HandlePlayerDeath();
    void CompletePlayerDeath();
    void PopulateStageCombo(HWND hDlg);
    std::wstring BuildStageComboText(const StageManager::StageData& stage);
    bool StartStageByIndex(std::size_t stageIndex);
    void MoveToSelectedStage(HWND hDlg);
    void RefreshTitleContinueCommand();
    void DrawTitleScreen();
    void DrawStageTitle();
    void DrawStageClear();
    void DrawEndingFin();
    POINT ConvertMouseToBaseResolution(int clientX, int clientY);
    D3DXVECTOR3 GetCameraPlanarForward();
    D3DXVECTOR3 GetCameraPlanarRight(const D3DXVECTOR3& forward);
    Enemy* FindEnemyInAttackRange(const PlayerAttackDefinition& attackDefinition);
    void InitializeCameraFromRenderSettings();
    void InitializePlayerPhysics();
    void LoadPhysicsObjectsFromCsv(const std::wstring& csvPath);
    void UpdatePlayerMeshAndCamera(const D3DXVECTOR3& previousRenderPosition);

    enum class PlayerAnimState { Idle, Walk, Run, Jump, Attack };
    enum class GameState { Loading, Title, SlideShow, Playing, StageClear, Ending, EndingFin };

    using PhysicsWorld = PhysicsLib::PhysicsLib;

    bool m_close = false;
    NSRender::Render m_render;
    Player m_player;
    StageManager m_stageManager;
    SaveDataManager m_saveDataManager;
    InventoryManager m_inventoryManager;
    CollectibleManager m_collectibleManager;
    InteractionManager m_interactionManager;
    CraftMenu m_craftMenu;
    NS_QTE_Module::QTE_Module* m_qte = nullptr;
    int m_playerMeshId = -1;
    bool m_playerIsSkinAnim = true;
    PhysicsLib::CharacterMover m_playerMover;
    PhysicsLib::CameraMover m_cameraMover;
    float m_cameraYaw = 0.0f;
    float m_cameraPitch = D3DXToRadian(18.0f);
    float m_cameraDistance = 7.0f;
    float m_playerYaw = 0.0f;
    PlayerAnimState m_playerAnimState = PlayerAnimState::Idle;
    PlayerAttackController m_playerAttackController;
    GameState m_gameState = GameState::Loading;
    SlideShowManager m_slideShowManager;
    PauseMenu m_pauseMenu;
    bool m_mouseCursorVisible = false;
    bool m_remoteDesktopMode = false;
    HWND m_settingsDialog = NULL;
    HWND m_hWnd = NULL;
    HINSTANCE m_hInstance = NULL;
    D3DXVECTOR3 m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    bool m_pendingJump = false;
    std::unordered_map<int, D3DXVECTOR3> m_prevMovingPlatformPositions;
    HpBar m_hpBar;
    DamagePopupManager m_damagePopupManager;
    EnemyManager m_enemyManager;
    int m_playerInvincibleFrames = 0;
    int m_playerKnockbackFrames = 0;
    D3DXVECTOR3 m_playerKnockbackDir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    int m_respawnCameraDelayFrames = 0;
    int m_respawnCameraMoveFrames = 0;
    bool m_playerDeathPending = false;
    bool m_stageClearProcessed = false;
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
    int m_stageTitleFrame = 0;
    int m_goalMarkerMeshId = -1;
};
