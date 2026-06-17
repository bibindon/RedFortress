#include "GameApp.h"

#include "resource.h"

namespace
{
    const std::wstring g_arrowSoundPath = L"res\\sound\\arrow.wav";
    const std::wstring g_cursorMoveSoundPath = L"res\\sound\\cursor_move.wav";
    const std::wstring g_playerMeshPath = L"res\\model2\\marine\\marine.x";
    const std::wstring g_playerAnimCsvPath = L"res\\model2\\marine\\marine.csv";
    const std::wstring g_playerIdleAnimName = L"000";
    const std::wstring g_playerWalkAnimName = L"walk";
    const std::wstring g_playerRunAnimName = L"run";
    const std::wstring g_playerJumpAnimName = L"jump";
    const std::wstring g_playerSlashAnimName = L"slash";
    const std::wstring g_finImagePath = L"res\\2D_Image\\fin.png";

    const float CAMERA_MOVE_SPEED = 0.08f;
    const float CAMERA_FAST_MOVE_SPEED = 0.25f;
    const float MOUSE_CAMERA_SENSITIVITY_NORMAL = 0.005f;
    const float MOUSE_CAMERA_SENSITIVITY_REMOTE = 0.00025f;
    const float kPlayerTurnRadiansPerSecond = 10.0f;
    const float kTargetFrameSeconds = 1.0f / 60.0f;
    const float kMinCameraDistance = 1.5f;
    const float kMaxCameraDistance = 20.0f;
    const float kCameraWheelZoomStep = 0.5f;
    const int kSlashDurationFrames = 120;
    const int kSlashAttackDelayFrames = 60;
    const int kPlayerInvincibleDuration = 180;
    const int kRespawnInvincibleFrames = 180;
    const float kStompBounceVelocity = 3.0f;
    const int kKnockbackDurationFrames = 60;
    const float kKnockbackSpeed = 3.0f;
    const int kRespawnCameraDelayFrames = 120;
    const int kRespawnCameraMoveFrames = 30;
    const int kStageTitleFrameMax = 180;
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
            SoundLib::SoundLib::PlaySoundEffect(g_cursorMoveSoundPath, 100);
        }
    }

    void PlayClick() override {}
    void PlayBack() override {}

    void Init() override {}
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

    RECT rect;
    SetRect(&rect, 0, 0, 640, 480);
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    rect.right = rect.right - rect.left;
    rect.bottom = rect.bottom - rect.top;
    rect.top = 0;
    rect.left = 0;

    m_hWnd = CreateWindow(_T("Window1"),
                          _T("Hello DirectX9 World !!"),
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
    m_render.ChangeResolution(1600, 900);
    ShowWindow(m_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(m_hWnd);
    m_render.SetLoadingScreenTitleFontPath(L"res\\font\\BIZUDMincho-Regular.ttf");
    m_render.StartLoadingScreen();
    m_render.SetLoadingScreenProgress(0);
    m_render.Draw();

    m_render.SetShowFPS(false);
    m_render.SetLightDir(D3DXVECTOR3(-0.4f, 1.0f, 0.6f));
    m_stageManager.Initialize();
    const StageManager::StageData& initialStage = m_stageManager.GetCurrentStage();
    m_render.LoadXFileListFromCsv(initialStage.renderCsvPath);
    m_render.SetLoadingScreenProgress(15);
    m_render.Draw();
    m_render.LoadXFileListMoveFromCsv(initialStage.moveCsvPath);
    m_render.SetLoadingScreenProgress(25);
    m_render.Draw();
    m_playerMeshId = m_render.AddMeshMixSkinAnim(g_playerMeshPath,
                                                 g_playerAnimCsvPath,
                                                 initialStage.playerStartPosition,
                                                 D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                 1.0f,
                                                 NSRender::AnimSetMap(),
                                                 -1.0f,
                                                 false,
                                                 false,
                                                 NSRender::MeshMixSkinAnimLoadMode::Custom);
    m_render.SetLoadingScreenProgress(40);
    m_render.Draw();
    InitializePlayerPhysics();
    m_render.SetLoadingScreenProgress(55);
    m_render.Draw();
    PhysicsLib::SettingsState::SetCameraAutoMoveEnabled(true);
    PhysicsLib::SettingsState::SetFocusModeEnabled(false);
    PhysicsLib::SettingsState::SetInfiniteJumpEnabled(false);
    InitializeCameraFromRenderSettings();
    UpdatePlayerMeshAndCamera(initialStage.playerStartPosition);
    m_enemyManager.LoadForStage(m_render, initialStage.enemyCsvPath);

    InputDevice::Initialize(m_hInstance, m_hWnd);
    m_pauseMenu.Initialize(m_render, m_mouseCursorVisible);
    InputDevice::Mouse::SetVisible(m_mouseCursorVisible);
    m_render.SetLoadingScreenProgress(70);
    m_render.Draw();
    SoundLib::SoundLib::Initialize(m_hWnd);
    SoundLib::SoundLib::LoadSoundEffect(g_arrowSoundPath);
    SoundLib::SoundLib::LoadSoundEffect(g_cursorMoveSoundPath);

    CommandFont* pFont = new CommandFont();
    pFont->app = this;
    CommandSE* pSE = new CommandSE();
    pSE->app = this;
    CommandSprite* pSpr = new CommandSprite();
    pSpr->app = this;
    m_command.Init(pFont, pSE, pSpr, false, L"res\\commandName_title.csv");

    m_hpBar.Initialize(&m_render, &m_player);
    m_damagePopupManager.Initialize(&m_render);

    m_saveDataManager.Initialize(m_stageManager);
    m_saveDataManager.Load();

    m_command.UpsertCommand(L"start", true);
    m_command.UpsertCommand(L"continue", m_saveDataManager.IsStageCleared(L"1-4"));
    m_command.UpsertCommand(L"exit", true);
    m_render.SetLoadingScreenProgress(85);
    m_render.Draw();

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

        InputDevice::Update();
        if (m_gameState == GameState::Playing &&
            !m_playerDeathPending &&
            InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
        {
            m_pauseMenu.Toggle();
        }

        if (m_gameState != GameState::EndingFin &&
            !m_pauseMenu.IsOpen() &&
            !m_playerDeathPending &&
            (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_LCONTROL) ||
             InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RCONTROL)))
        {
            m_mouseCursorVisible = !m_mouseCursorVisible;
            InputDevice::Mouse::SetVisible(m_mouseCursorVisible);
        }

        if (m_gameState == GameState::Loading)
        {
            m_render.Draw();

            if (m_render.IsAllMeshLoaded())
            {
                m_render.EndLoadingScreen();
                m_gameState = GameState::Title;
            }
        }
        else if (m_gameState == GameState::Title)
        {
            RefreshTitleContinueCommand();
            UpdateTitleByInput();
            DrawTitleScreen();

            if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
            {
                const std::wstring selectedId = m_command.Into();
                if (selectedId == L"start")
                {
                    m_slideShowManager.Start(L"res\\script\\hoshigirl_trial_novel.csv");
                    m_slideShowManager.SetStopOnFinish(false);
                    m_gameState = GameState::SlideShow;
                }
                else if (selectedId == L"continue")
                {
                    const std::size_t baseIndex = m_stageManager.FindStageIndexById(L"base");
                    StartStageByIndex(baseIndex);
                }
                else if (selectedId == L"exit")
                {
                    m_close = true;
                }
            }

            const InputDevice::MousePosition mousePos = InputDevice::Mouse::GetPosition();
            const POINT baseMousePos = ConvertMouseToBaseResolution(mousePos.x, mousePos.y);
            m_command.MouseMove(baseMousePos.x, baseMousePos.y);

            if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
            {
                const std::wstring clickedId = m_command.Click(baseMousePos.x, baseMousePos.y);
                if (clickedId == L"start")
                {
                    m_slideShowManager.Start(L"res\\script\\hoshigirl_trial_novel.csv");
                    m_slideShowManager.SetStopOnFinish(false);
                    m_gameState = GameState::SlideShow;
                }
                else if (clickedId == L"continue")
                {
                    const std::size_t baseIndex = m_stageManager.FindStageIndexById(L"base");
                    StartStageByIndex(baseIndex);
                }
                else if (clickedId == L"exit")
                {
                    m_close = true;
                }
            }
        }
        else if (m_gameState == GameState::SlideShow)
        {
            if (!m_slideShowManager.IsActive())
            {
                m_gameState = GameState::Playing;
                m_stageTitleFrame = kStageTitleFrameMax;
                m_prevMovingPlatformPositions.clear();
                m_render.Draw();
            }
            else
            {
                m_slideShowManager.ProcessInput();
                if (m_slideShowManager.Update())
                {
                    m_gameState = GameState::Playing;
                    m_stageTitleFrame = kStageTitleFrameMax;
                    m_prevMovingPlatformPositions.clear();
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
        else
        {
            if (m_pauseMenu.BlocksGameInput())
            {
                m_hpBar.Draw();
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render);
                m_pauseMenu.Render(m_stageManager.GetCurrentStageDisplayName());
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

                m_hpBar.Draw();
                m_damagePopupManager.Draw();
                m_enemyManager.DrawHpBars(m_render);
                m_render.Draw();
                continue;
            }

            // マウスカーソル表示中はUI操作を優先し、カメラ回転を止める。
            if (!m_mouseCursorVisible)
            {
                UpdateCameraByInput();
            }

            // 入力処理 → メッシュ位置・カメラ設定（衝突判定前）
            UpdatePlayerByInput();

            // 敵の更新
            m_enemyManager.Update(m_render, m_playerMover.GetPosition(), m_playerInvincibleFrames > 0);
            m_enemyManager.SyncMeshes(m_render);

            // 描画（動く床の位置が更新される）
            DrawStageTitle();
            m_hpBar.Draw();
            m_damagePopupManager.Update();
            m_damagePopupManager.Draw();
            m_enemyManager.DrawHpBars(m_render);
            m_render.Draw();

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
            m_playerMover.Update(m_pendingMove, m_pendingJump);
            if (m_playerMover.IsCrushed())
            {
                DamagePlayerHp(m_player.GetHp());
            }

            // slash 攻撃判定（左クリックから 1 秒後）
            if (m_playerSlashAttackFrames == 0)
            {
                m_playerSlashAttackFrames = -1;
                Enemy* slashTarget = FindEnemyInSlashRange();
                if (slashTarget != nullptr)
                {
                    slashTarget->TakeDamage(m_render, 5);
                    m_damagePopupManager.Add(5, slashTarget->GetPosition(), false);
                }
            }

            // プレイヤー無敵時間を更新
            if (m_playerInvincibleFrames > 0)
            {
                --m_playerInvincibleFrames;
            }

            // 敵との接触・踏みつけ判定
            for (auto& enemy : m_enemyManager.GetEnemies())
            {
                if (enemy.IsDead())
                {
                    continue;
                }

                if (enemy.IsStompedByPlayer(m_playerMover.GetPosition(), m_playerMover.IsJumping(), m_playerMover.GetVelocity().y))
                {
                    enemy.TakeDamage(m_render, 10);
                    m_damagePopupManager.Add(10, enemy.GetPosition(), false);
                    D3DXVECTOR3 playerPos = m_playerMover.GetPosition();
                    playerPos.y += 0.3f;
                    m_playerMover.SetPosition(playerPos);
                    break;
                }
                else if (m_playerInvincibleFrames <= 0 && enemy.IsTouchingPlayer(m_playerMover.GetPosition()))
                {
                    DamagePlayerHp(10);
                    m_playerInvincibleFrames = kPlayerInvincibleDuration;
                    if (m_playerMeshId >= 0)
                    {
                        m_render.StartMeshMixSkinAnimBlink(m_playerMeshId, kPlayerInvincibleDuration, 4);
                    }
                    m_playerKnockbackFrames = kKnockbackDurationFrames;
                    D3DXVECTOR3 knockbackDir = m_playerMover.GetPosition() - enemy.GetPosition();
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
                    enemy.MarkAttackedPlayer();
                    break;
                }
            }

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
                m_gameState = GameState::StageClear;
                m_stageClearProcessed = false;
            }

            if (m_playerMover.JustJumped() && m_playerAnimState == PlayerAnimState::Jump)
            {
                if (m_playerMeshId >= 0)
                {
                    m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 0.1f);
                    m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerRunAnimName);
                }
            }

            const D3DXVECTOR3 playerRenderPosition = m_playerMover.GetPosition();
            const D3DXVECTOR3 listenerForward = GetCameraPlanarForward();
            SoundLib::Vector3 listenerPosition { playerRenderPosition.x, playerRenderPosition.y, playerRenderPosition.z };
            SoundLib::Vector3 listenerFront { listenerForward.x, listenerForward.y, listenerForward.z };
            SoundLib::Vector3 listenerTop { 0.0f, 1.0f, 0.0f };
            SoundLib::SoundLib::Update(listenerPosition, listenerFront, listenerTop);

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
                }
                ShowWindow(m_settingsDialog, isVisible ? SW_HIDE : SW_SHOW);
            }
        }

        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F8))
        {
            m_render.ShowSettingsDialog();
        }

        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F9))
        {
            PhysicsWorld::ShowSettingsDialog(m_hWnd);
        }

        if (m_close)
        {
            break;
        }

    }
}

void GameApp::Finalize()
{
    if (m_settingsDialog != NULL)
    {
        DestroyWindow(m_settingsDialog);
        m_settingsDialog = NULL;
    }

    m_render.Finalize();
    PhysicsWorld::Finalize();
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

static float MoveAngleToward(float current, float target, float maxDelta)
{
    float diff = target - current;
    while (diff > D3DX_PI)  diff -= 2.0f * D3DX_PI;
    while (diff < -D3DX_PI) diff += 2.0f * D3DX_PI;
    if (fabsf(diff) <= maxDelta) return target;
    return current + (diff > 0.0f ? maxDelta : -maxDelta);
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

    // slash カウントダウン
    if (m_playerSlashFrames > 0)
    {
        --m_playerSlashFrames;
    }

    // slash 攻撃判定カウントダウン
    if (m_playerSlashAttackFrames > 0)
    {
        --m_playerSlashAttackFrames;
    }

    // ノックバックカウントダウン
    if (m_playerKnockbackFrames > 0)
    {
        --m_playerKnockbackFrames;
    }

    // 左クリックで slash
    if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT) && m_playerSlashFrames <= 0)
    {
        m_playerSlashFrames = kSlashDurationFrames;
        m_playerSlashAttackFrames = kSlashAttackDelayFrames;
        m_playerAnimState = PlayerAnimState::Slash;
        if (m_playerMeshId >= 0)
        {
            m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 1.0f);
            m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerSlashAnimName);
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
    const bool isRunning = isMoving && InputDevice::SKeyBoard::IsDown(DIK_LSHIFT);

    PhysicsLib::CharacterMover::Settings settings = m_playerMover.GetSettings();
    const float walkSpeed = 1.5f;
    const float runSpeed = 4.5f;
    settings.moveSpeed = isRunning ? runSpeed : walkSpeed;
    m_playerMover.SetSettings(settings);

    D3DXVECTOR3 move(0.0f, 0.0f, 0.0f);
    if (m_playerKnockbackFrames > 0)
    {
        move = m_playerKnockbackDir * kKnockbackSpeed;
        const float targetYaw = atan2f(-move.x, -move.z);
        m_playerYaw = MoveAngleToward(m_playerYaw,
                                      targetYaw,
                                      kPlayerTurnRadiansPerSecond * kTargetFrameSeconds);
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

        PlayerAnimState nextState;
        if (m_playerSlashFrames > 0)
        {
            nextState = PlayerAnimState::Slash;
        }
        else if (isJumping)
        {
            nextState = PlayerAnimState::Jump;
        }
        else
        {
            if (isRunning)      nextState = PlayerAnimState::Run;
            else if (isMoving)  nextState = PlayerAnimState::Walk;
            else                nextState = PlayerAnimState::Idle;
        }

        if (nextState != m_playerAnimState)
        {
            m_playerAnimState = nextState;
            if (nextState == PlayerAnimState::Run)
            {
                m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 1.0f);
                m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerRunAnimName);
            }
            else if (nextState == PlayerAnimState::Walk)
            {
                m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 1.0f);
                m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerWalkAnimName);
            }
            else if (nextState == PlayerAnimState::Jump)
            {
                m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 0.1f);
                m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerRunAnimName);
            }
            else
            {
                m_render.SetMeshMixSkinAnimSpeed(m_playerMeshId, 1.0f);
                m_render.PlayMeshMixSkinAnimAnimation(m_playerMeshId, g_playerIdleAnimName);
            }
        }
    }

    // 衝突判定は後で行う。カメラはここで設定する。
    UpdatePlayerMeshAndCamera(previousRenderPosition);

    m_pendingMove = move;
    m_pendingJump = InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE);
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

Enemy* GameApp::FindEnemyInSlashRange()
{
    const D3DXVECTOR3 playerPos = m_playerMover.GetPosition();
    const D3DXVECTOR3 forward(-sinf(m_playerYaw), 0.0f, -cosf(m_playerYaw));
    const float kSlashRange = 2.0f;
    const float kSlashHalfAngle = D3DXToRadian(45.0f);

    Enemy* bestEnemy = nullptr;
    float bestDot = -1.0f;

    for (auto& enemy : m_enemyManager.GetEnemies())
    {
        if (enemy.IsDead())
        {
            continue;
        }

        D3DXVECTOR3 dir = enemy.GetPosition() - playerPos;
        dir.y = 0.0f;
        const float dist = D3DXVec3Length(&dir);
        if (dist > kSlashRange)
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
        if (dot > cosf(kSlashHalfAngle) && dot > bestDot)
        {
            bestDot = dot;
            bestEnemy = &enemy;
        }
    }

    return bestEnemy;
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
    settings.moveSpeed = 6.0f;
    settings.groundAcceleration = 18.0f;
    settings.airAcceleration = 8.0f;
    settings.jumpVelocity = 5.0f;
    settings.airControlEnabled = true;
    settings.doubleJumpEnabled = true;
    m_playerMover.SetSettings(settings);
    m_player.ResetHp();
    m_hpBar.Reset();
    m_playerMover.Reset(m_stageManager.GetCurrentStage().playerStartPosition);

    PhysicsLib::SettingsState::SetShapeType(PhysicsWorld::ShapeType::Cylinder);
    PhysicsLib::SettingsState::SetCylinderRadius(0.3f);
    PhysicsLib::SettingsState::SetCylinderHeight(1.7f);
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
        if (m_playerIsSkinAnim)
        {
            m_render.SetMeshMixSkinAnimPos(m_playerMeshId, currentRenderPosition);
            m_render.SetMeshMixSkinAnimRotY(m_playerMeshId, m_playerYaw);
        }
        else
        {
            m_render.SetMeshMixPos(m_playerMeshId, currentRenderPosition);
        }
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

std::wstring GameApp::BuildStageComboText(const StageManager::StageData& stage)
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

bool GameApp::StartStageByIndex(std::size_t stageIndex)
{
    if (!m_stageManager.MoveToStage(stageIndex))
    {
        return false;
    }

    LoadCurrentStageObjects();
    m_gameState = GameState::Playing;
    m_stageTitleFrame = kStageTitleFrameMax;
    return true;
}

void GameApp::RefreshTitleContinueCommand()
{
    const bool canContinue = m_saveDataManager.IsStageCleared(L"1-4");
    m_command.UpsertCommand(L"continue", canContinue);
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
        PopulateStageCombo(hDlg);
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

        case IDC_BUTTON_STAGE_GO:
            MoveToSelectedStage(hDlg);
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

void GameApp::UpdateStageClear()
{
    if (!m_stageClearProcessed)
    {
        const std::wstring clearedStageId = m_stageManager.GetCurrentStage().id;
        m_saveDataManager.MarkStageCleared(clearedStageId);
        m_saveDataManager.Save();
        m_stageClearProcessed = true;
    }

    if (m_stageManager.IsLastStage())
    {
        m_slideShowManager.Start(L"res\\script\\ending.csv");
        m_slideShowManager.SetStopOnFinish(false);
        m_gameState = GameState::Ending;
        return;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE) ||
        InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
    {
        if (StartNextStage())
        {
            return;
        }
    }

    DrawStageClear();
    m_render.Draw();
}

bool GameApp::IsStageClearReached()
{
    return m_stageManager.IsClearReached(m_playerMover.GetPosition());
}

void GameApp::DamagePlayerHp(int amount)
{
    const int oldHp = m_player.GetHp();
    m_player.Damage(amount);
    const int newHp = m_player.GetHp();
    if (newHp < oldHp)
    {
        m_hpBar.OnDamage(oldHp, newHp);
        m_damagePopupManager.Add(oldHp - newHp, m_playerMover.GetPosition(), false);
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

void GameApp::HandlePlayerDeath()
{
    if (m_playerDeathPending)
    {
        return;
    }

    // カメラ移動開始位置を保存（プレイヤー位置を変更する前に行う）
    m_respawnCameraFromPos = m_render.GetCameraPos();
    m_respawnCameraFromTarget = m_render.GetLookAtPos();
    m_respawnCameraDelayFrames = kRespawnCameraDelayFrames;
    m_playerDeathPending = true;
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerKnockbackFrames = 0;
    m_playerSlashFrames = 0;
    m_playerSlashAttackFrames = -1;
    m_render.SetSceneUpdatePaused(true);
}

void GameApp::CompletePlayerDeath()
{
    m_player.Die();
    m_playerDeathPending = false;
    m_respawnCameraDelayFrames = 0;
    m_render.SetSceneUpdatePaused(false);

    if (m_player.IsGameOver())
    {
        const int stageNumber = m_stageManager.GetCurrentStage().number;
        if (stageNumber >= 1 && stageNumber <= 4)
        {
            m_gameState = GameState::Title;
            m_player.ResetLives();
            m_player.ResetHp();
            m_enemyManager.Clear(m_render);
        }
        else
        {
            StartStageByIndex(4);
            m_player.ResetLives();
        }
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
    m_playerSlashFrames = 0;
    m_playerSlashAttackFrames = -1;
    m_damagePopupManager.Clear();
}

bool GameApp::StartNextStage()
{
    if (!m_stageManager.MoveNextStage())
    {
        return false;
    }

    LoadCurrentStageObjects();
    m_gameState = GameState::Playing;
    m_stageTitleFrame = kStageTitleFrameMax;
    return true;
}

void GameApp::LoadCurrentStageObjects()
{
    const StageManager::StageData& stage = m_stageManager.GetCurrentStage();

    if (m_goalMarkerMeshId >= 0)
    {
        m_render.RemoveMesh(m_goalMarkerMeshId);
        m_goalMarkerMeshId = -1;
    }

    m_render.ClearCsvLoadedMeshes();
    m_render.LoadXFileListFromCsv(stage.renderCsvPath);
    m_render.LoadXFileListMoveFromCsv(stage.moveCsvPath);

    const D3DXVECTOR3 goalPos(stage.clearPosition.x, stage.clearPosition.y - 1.0f, stage.clearPosition.z);
    m_goalMarkerMeshId = m_render.AddMesh(L"res\\model\\cube_red.x",
                                          goalPos,
                                          D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                          1.0f);

    PhysicsWorld::ClearObjects();
    LoadPhysicsObjectsFromCsv(stage.physicsCsvPath);

    m_prevMovingPlatformPositions.clear();
    m_pendingMove = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_pendingJump = false;
    m_playerYaw = 0.0f;
    m_playerAnimState = PlayerAnimState::Idle;
    m_damagePopupManager.Clear();
    m_playerInvincibleFrames = 0;
    m_playerKnockbackFrames = 0;
    m_playerSlashFrames = 0;
    m_playerSlashAttackFrames = -1;
    m_respawnCameraDelayFrames = 0;
    m_respawnCameraMoveFrames = 0;
    m_playerDeathPending = false;
    m_render.SetSceneUpdatePaused(false);
    if (m_playerMeshId >= 0)
    {
        m_render.StopMeshMixSkinAnimBlink(m_playerMeshId);
    }
    m_player.ResetHp();
    m_hpBar.Reset();
    m_playerMover.Reset(stage.playerStartPosition);
    m_enemyManager.LoadForStage(m_render, stage.enemyCsvPath);
    UpdatePlayerMeshAndCamera(stage.playerStartPosition);
}

void GameApp::DrawTitleScreen()
{
    if (m_titleFontId < 0)
    {
        m_titleFontId = m_render.SetUpFont(L"BIZ UDMincho", 50, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    m_render.DrawTextCenter(m_titleFontId, L"ホ  シ  ガ  ー  ル", 0, 220, NSRender::Common::BASE_W, 100);
    m_command.Draw();
    m_render.Draw();
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
        m_stageClearFontId = m_render.SetUpFont(L"BIZ UDGothic", 64, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    if (m_stageClearHintFontId < 0)
    {
        m_stageClearHintFontId = m_render.SetUpFont(L"BIZ UDGothic", 24, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    if (m_stageManager.IsLastStage())
    {
        m_render.DrawTextCenter(m_stageClearFontId, L"All Clear", 0, 330, NSRender::Common::BASE_W, 100);
        return;
    }

    const std::wstring clearText = m_stageManager.GetCurrentStageDisplayName() + L" Clear";
    m_render.DrawTextCenter(m_stageClearFontId, clearText, 0, 300, NSRender::Common::BASE_W, 100);
    m_render.DrawTextCenter(m_stageClearHintFontId,
                            L"Space / Enter で次のステージへ",
                            0,
                            405,
                            NSRender::Common::BASE_W,
                            50,
                            D3DCOLOR_RGBA(255, 255, 255, 220));
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


