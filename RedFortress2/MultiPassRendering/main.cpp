#pragma comment( lib, "d3d9.lib" )
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "d3dx9d.lib" )
#else
#pragma comment( lib, "d3dx9.lib" )
#endif

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <tchar.h>
#include <cassert>
#include <crtdbg.h>
#include <unordered_map>

#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLibInternal.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../SoundLib/SoundLib/SoundLib.h"
#include "../../RedFortressCommand/Command/Command.h"
#include "../../RedFortressSlideShow/SlideShow/SlideShow.h"
#include "resource.h"

bool g_bClose = false;
const std::wstring g_arrowSoundPath = L"res\\sound\\arrow.wav";
const std::wstring g_cursorMoveSoundPath = L"res\\sound\\cursor_move.wav";
const std::wstring g_slideShowCsvPath = L"res\\script\\hoshigirl_trial_novel.csv";
const std::wstring g_playerMeshPath = L"res\\model2\\marine\\marine.x";
const std::wstring g_playerAnimCsvPath = L"res\\model2\\marine\\marine.csv";
const std::wstring g_playerIdleAnimName = L"000";
const std::wstring g_playerWalkAnimName = L"walk";
const std::wstring g_playerRunAnimName = L"run";
const std::wstring g_playerJumpAnimName = L"jump";
NSRender::Render g_Render;
using PhysicsWorld = PhysicsLib::PhysicsLib;
const float CAMERA_MOVE_SPEED = 0.08f;
const float CAMERA_FAST_MOVE_SPEED = 0.25f;
const float MOUSE_CAMERA_SENSITIVITY_NORMAL = 0.005f;
const float MOUSE_CAMERA_SENSITIVITY_REMOTE = 0.00025f;
bool g_remoteDesktopMode = []() {
    SYSTEMTIME st;
    GetLocalTime(&st);
    return st.wDayOfWeek >= 1 && st.wDayOfWeek <= 5 && st.wHour >= 8 && st.wHour < 19;
}();
const D3DXVECTOR3 PLAYER_START_POSITION(0.0f, 1.0f, 0.0f);
int g_playerMeshId = -1;
bool g_playerIsSkinAnim = true;
PhysicsLib::CharacterMover g_playerMover;
PhysicsLib::CameraMover g_cameraMover;
float g_cameraYaw = 0.0f;
float g_cameraPitch = D3DXToRadian(18.0f);
float g_cameraDistance = 6.0f;
float g_playerYaw = 0.0f;
const float kPlayerTurnRadiansPerSecond = 10.0f;
const float kTargetFrameSeconds = 1.0f / 60.0f;
const float kMinCameraDistance = 1.5f;
const float kMaxCameraDistance = 20.0f;
const float kCameraWheelZoomStep = 0.5f;
enum class PlayerAnimState { Idle, Walk, Run, Jump };
PlayerAnimState g_playerAnimState = PlayerAnimState::Idle;
enum class GameState { Loading, Title, SlideShow, Playing };
GameState g_gameState = GameState::Loading;
NSSlideShow::SlideShow* g_slideShow = nullptr;
int g_slideShowFontId = -1;
int g_slideShowSkipHintFontId = -1;
bool g_slideShowSkipRequested = false;
const float kSlideShowSkipHoldSeconds = 1.0f;
bool g_mouseCursorVisible = false;
HWND g_settingsDialog = NULL;
D3DXVECTOR3 g_pendingMove(0.0f, 0.0f, 0.0f);
bool g_pendingJump = false;

static void UpdateCameraByInput();
static void UpdatePlayerByInput();
static void UpdateSlideShow();
static void UpdateTitleByInput();
static void DrawSlideShowSkipHint();
static void DrawTitleScreen();
static void DrawStageTitle();
static POINT ConvertMouseToBaseResolution(int clientX, int clientY);
static D3DXVECTOR3 GetCameraPlanarForward();
static D3DXVECTOR3 GetCameraPlanarRight(const D3DXVECTOR3& forward);
static void InitializeCameraFromRenderSettings();
static void InitializePlayerPhysics();
static void LoadPhysicsObjectsFromCsv(const std::wstring& csvPath);
static void UpdatePlayerMeshAndCamera(const D3DXVECTOR3& previousRenderPosition);
static INT_PTR CALLBACK SettingsDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

int g_commandFontId = -1;
int g_titleFontId = -1;
int g_stageTitleFontId = -1;
int g_stageTitleFrame = 0;
const int kStageTitleFrameMax = 180;

class CommandFont : public NSCommand::IFont
{
public:
    void DrawText_(const std::wstring& msg, const int x, const int y, const int transparent) override
    {
        if (g_commandFontId >= 0)
        {
            g_Render.DrawTextExCenter(g_commandFontId,
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
        g_commandFontId = g_Render.SetUpFontEx(L"BIZ UDGothic", 18, D3DCOLOR_ARGB(255, 255, 255, 255));
    }

    void OnDeviceLost() override {}
    void OnDeviceReset() override {}
};

class CommandSprite : public NSCommand::ISprite
{
public:
    void DrawImage(const int x, const int y, const int transparency) override
    {
        g_Render.DrawImage(L"res\\2D_Image\\command_cursor.png", x, y, transparency);
    }

    void Load(const std::wstring& filepath) override
    {
        (void)filepath;
    }

    void OnDeviceLost() override {}
    void OnDeviceReset() override {}
};

class CommandSE : public NSCommand::ISoundEffect
{
public:
    void PlayMove() override
    {
        SoundLib::SoundLib::PlaySoundEffect(g_cursorMoveSoundPath, 100);
    }

    void PlayClick() override {}
    void PlayBack() override {}

    void Init() override {}
};

class SlideShowSprite : public NSSlideShow::ISprite
{
public:
    void DrawImage(const int x, const int y, const int transparency) override
    {
        (void)x;
        (void)y;
        if (m_filepath.empty())
        {
            return;
        }

        if (m_filepath.find(L"novel_chr_") != std::wstring::npos)
        {
            g_Render.DrawImageAutoResize(m_filepath, 0.78f, 0.48f, transparency);
        }
        else
        {
            g_Render.DrawImageStretched(m_filepath, transparency);
        }
    }

    void DrawImageEx(const int x,
                     const int y,
                     const int transparency,
                     const bool flipX,
                     const float scale) override
    {
        if (m_filepath.empty())
        {
            return;
        }

        if (m_filepath.find(L"novel_chr_") != std::wstring::npos)
        {
            g_Render.DrawImageAutoResizeEx(m_filepath,
                                           static_cast<float>(x) / static_cast<float>(NSRender::Common::BASE_W),
                                           static_cast<float>(y) / static_cast<float>(NSRender::Common::BASE_H),
                                           scale,
                                           flipX,
                                           transparency);
        }
        else
        {
            g_Render.DrawImageStretchedScaled(m_filepath, scale, transparency);
        }
    }

    void Load(const std::wstring& filepath) override
    {
        m_filepath = filepath;
        g_Render.LoadImage(m_filepath);
    }

    void GetImageSize(int& width, int& height) const override
    {
        if (m_filepath.empty())
        {
            width = 0;
            height = 0;
            return;
        }

        const SIZE size = g_Render.GetImageSize(m_filepath);
        width = size.cx;
        height = size.cy;
    }

    NSSlideShow::ISprite* Create() override
    {
        SlideShowSprite* sprite = new SlideShowSprite();
        sprite->m_filepath = m_filepath;
        return sprite;
    }

    void OnDeviceLost() override {}
    void OnDeviceReset() override {}

private:
    std::wstring m_filepath;
};

class SlideShowFont : public NSSlideShow::IFont
{
public:
    void DrawText_(const std::wstring& msg, const int x, const int y) override
    {
        if (g_slideShowFontId >= 0)
        {
            g_Render.DrawText_(g_slideShowFontId, msg, x, y, D3DCOLOR_RGBA(255, 255, 255, 255));
        }
    }

    void Init(const bool bEnglish) override
    {
        (void)bEnglish;
        g_slideShowFontId = g_Render.SetUpFont(L"BIZ UDGothic", 22, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    void OnDeviceLost() override {}
    void OnDeviceReset() override {}
};

class SlideShowSE : public NSSlideShow::ISoundEffect
{
public:
    void PlayMove() override
    {
        SoundLib::SoundLib::PlaySoundEffect(g_cursorMoveSoundPath, 100);
    }

    void Init() override {}
};

static void InitializeSlideShow()
{
    g_slideShowSkipRequested = false;

    NSSlideShow::IFont* font = new SlideShowFont();
    NSSlideShow::ISoundEffect* se = new SlideShowSE();
    NSSlideShow::ISprite* sprTextBack = new SlideShowSprite();
    sprTextBack->Load(L"res\\2D_Image\\textBack.png");
    NSSlideShow::ISprite* sprFade = new SlideShowSprite();
    sprFade->Load(L"res\\2D_Image\\black2x2.bmp");
    NSSlideShow::ISprite* sprImage = new SlideShowSprite();

    g_slideShow = new NSSlideShow::SlideShow();
    g_slideShow->SetScreenSize(NSRender::Common::BASE_W, NSRender::Common::BASE_H);
    g_slideShow->Init(font, se, sprTextBack, sprFade, g_slideShowCsvPath, sprImage, false, false);
}

namespace
{
    NSCommand::Command g_command;
}

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

extern int WINAPI _tWinMain(_In_ HINSTANCE hInstance,
                            _In_opt_ HINSTANCE hPrevInstance,
                            _In_ LPTSTR lpCmdLine,
                            _In_ int nCmdShow);

int WINAPI _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR lpCmdLine,
                     _In_ int nCmdShow)
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    WNDCLASSEX wc { };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = MsgProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = NULL;
    wc.hCursor = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = _T("Window1");
    wc.hIconSm = NULL;

    ATOM atom = RegisterClassEx(&wc);
    assert(atom != 0);

    RECT rect;
    SetRect(&rect, 0, 0, 640, 480);
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    rect.right = rect.right - rect.left;
    rect.bottom = rect.bottom - rect.top;
    rect.top = 0;
    rect.left = 0;

    HWND hWnd = CreateWindow(_T("Window1"),
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

    g_Render.Initialize(hWnd, L"res\\RenderSettings.csv");
    g_Render.ChangeResolution(1600, 900);
    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);
    g_Render.SetLoadingScreenTitleFontPath(L"res\\font\\BIZUDMincho-Regular.ttf");
    g_Render.StartLoadingScreen();
    g_Render.SetLoadingScreenProgress(0);
    g_Render.Draw();

    g_Render.SetShowFPS(false);
    g_Render.SetLightDir(D3DXVECTOR3(-0.4f, 1.0f, 0.6f));
    g_Render.LoadXFileListFromCsv(L"res\\model\\XFileList_simple.csv");
    g_Render.SetLoadingScreenProgress(15);
    g_Render.Draw();
    g_Render.LoadXFileListMoveFromCsv(L"res\\model\\XFileListMove.csv");
    g_Render.SetLoadingScreenProgress(25);
    g_Render.Draw();
    g_playerMeshId = g_Render.AddMeshMixSkinAnim(g_playerMeshPath,
                                                 g_playerAnimCsvPath,
                                                 PLAYER_START_POSITION,
                                                 D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                 1.0f,
                                                 NSRender::AnimSetMap(),
                                                 -1.0f,
                                                 false,
                                                 false,
                                                 NSRender::MeshMixSkinAnimLoadMode::Custom);
    g_Render.SetLoadingScreenProgress(40);
    g_Render.Draw();
    InitializePlayerPhysics();
    g_Render.SetLoadingScreenProgress(55);
    g_Render.Draw();
    PhysicsLib::SettingsState::SetCameraAutoMoveEnabled(true);
    PhysicsLib::SettingsState::SetFocusModeEnabled(false);
    PhysicsLib::SettingsState::SetInfiniteJumpEnabled(false);
    InitializeCameraFromRenderSettings();
    UpdatePlayerMeshAndCamera(PLAYER_START_POSITION);

    InputDevice::Initialize(hInstance, hWnd);
    InputDevice::Mouse::SetVisible(g_mouseCursorVisible);
    g_Render.SetLoadingScreenProgress(70);
    g_Render.Draw();
    SoundLib::SoundLib::Initialize(hWnd);
    SoundLib::SoundLib::LoadSoundEffect(g_arrowSoundPath);
    SoundLib::SoundLib::LoadSoundEffect(g_cursorMoveSoundPath);

    CommandFont* pFont = new CommandFont();
    CommandSE* pSE = new CommandSE();
    CommandSprite* pSpr = new CommandSprite();
    g_command.Init(pFont, pSE, pSpr, false, L"res\\commandName_title.csv");
    g_command.UpsertCommand(L"start", true);
    g_command.UpsertCommand(L"continue", true);
    g_command.UpsertCommand(L"exit", true);
    g_Render.SetLoadingScreenProgress(85);
    g_Render.Draw();

    MSG msg;

    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            const bool isEscKey = (msg.message == WM_KEYDOWN && msg.wParam == VK_ESCAPE);
            if (g_settingsDialog == NULL || !IsWindowVisible(g_settingsDialog) ||
                isEscKey ||
                !IsDialogMessage(g_settingsDialog, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        InputDevice::Update();
        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
        {
            g_mouseCursorVisible = !g_mouseCursorVisible;
            InputDevice::Mouse::SetVisible(g_mouseCursorVisible);
        }

        if (g_gameState == GameState::Loading)
        {
            g_Render.Draw();

            if (g_Render.IsAllMeshLoaded())
            {
                g_Render.EndLoadingScreen();
                g_gameState = GameState::Title;
            }
        }
        else if (g_gameState == GameState::Title)
        {
            UpdateTitleByInput();
            DrawTitleScreen();

            if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN))
            {
                const std::wstring selectedId = g_command.Into();
                if (selectedId == L"start")
                {
                    InitializeSlideShow();
                    g_gameState = GameState::SlideShow;
                }
                else if (selectedId == L"continue")
                {
                    g_gameState = GameState::Playing;
                }
                else if (selectedId == L"exit")
                {
                    g_bClose = true;
                }
            }

            const InputDevice::MousePosition mousePos = InputDevice::Mouse::GetPosition();
            const POINT baseMousePos = ConvertMouseToBaseResolution(mousePos.x, mousePos.y);
            g_command.MouseMove(baseMousePos.x, baseMousePos.y);

            if (InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
            {
                const std::wstring clickedId = g_command.Click(baseMousePos.x, baseMousePos.y);
                if (clickedId == L"start")
                {
                    InitializeSlideShow();
                    g_gameState = GameState::SlideShow;
                }
                else if (clickedId == L"continue")
                {
                    g_gameState = GameState::Playing;
                }
                else if (clickedId == L"exit")
                {
                    g_bClose = true;
                }
            }
        }
        else if (g_gameState == GameState::SlideShow)
        {
            UpdateSlideShow();
        }
        else
        {
            // マウスカーソル表示中はUI操作を優先し、カメラ回転を止める。
            if (!g_mouseCursorVisible)
            {
                UpdateCameraByInput();
            }

            // 入力処理 → メッシュ位置・カメラ設定（衝突判定前）
            UpdatePlayerByInput();

            // 描画（動く床の位置が更新される）
            DrawStageTitle();
            g_Render.Draw();

            // 動く床の位置を描画エンジンから取得し、物理エンジンに反映する。
            {
                const D3DXVECTOR3 kPlatformRot(0.0f, 0.0f, 0.0f);
                const D3DXVECTOR3 kPlatformScale(1.0f, 1.0f, 1.0f);

                static std::unordered_map<int, D3DXVECTOR3> s_prevPositions;

                const auto& platforms = g_Render.GetMovingPlatforms();
                for (const auto& platform : platforms)
                {
                    const D3DXVECTOR3 platformPos = g_Render.GetMeshMixPos(platform.renderId);
                    D3DXVECTOR3& prevPos = s_prevPositions[platform.csvId];
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
            g_playerMover.Update(g_pendingMove, g_pendingJump);

            if (g_playerMover.JustJumped() && g_playerAnimState == PlayerAnimState::Jump)
            {
                if (g_playerMeshId >= 0)
                {
                    g_Render.SetMeshMixSkinAnimSpeed(g_playerMeshId, 0.1f);
                    g_Render.PlayMeshMixSkinAnimAnimation(g_playerMeshId, g_playerRunAnimName);
                }
            }

            const D3DXVECTOR3 playerRenderPosition = g_playerMover.GetPosition();
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
            if (g_settingsDialog == NULL)
            {
                g_settingsDialog = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, SettingsDialogProc);
            }

            if (g_settingsDialog != NULL)
            {
                const bool isVisible = IsWindowVisible(g_settingsDialog);
                ShowWindow(g_settingsDialog, isVisible ? SW_HIDE : SW_SHOW);
            }
        }

        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F8))
        {
            g_Render.ShowSettingsDialog();
        }

        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F9))
        {
            PhysicsWorld::ShowSettingsDialog(hWnd);
        }

        if (g_bClose)
        {
            break;
        }

    }

    if (g_settingsDialog != NULL)
    {
        DestroyWindow(g_settingsDialog);
        g_settingsDialog = NULL;
    }

    g_Render.Finalize();
    PhysicsWorld::Finalize();
    SoundLib::SoundLib::Finalize();
    InputDevice::Finalize();

    UnregisterClass(_T("Window1"), wc.hInstance);
    return 0;
}

static float ClampFloat(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float MoveAngleToward(float current, float target, float maxDelta)
{
    float diff = target - current;
    while (diff > D3DX_PI)  diff -= 2.0f * D3DX_PI;
    while (diff < -D3DX_PI) diff += 2.0f * D3DX_PI;
    if (fabsf(diff) <= maxDelta) return target;
    return current + (diff > 0.0f ? maxDelta : -maxDelta);
}

void InitializeCameraFromRenderSettings()
{
    const D3DXVECTOR3 cameraPos = g_Render.GetCameraPos();
    const D3DXVECTOR3 lookAtPos = g_Render.GetLookAtPos();
    const D3DXVECTOR3 offset = cameraPos - lookAtPos;
    const float distance = D3DXVec3Length(&offset);
    if (distance <= 0.0001f)
    {
        return;
    }

    g_cameraDistance = ClampFloat(distance, kMinCameraDistance, kMaxCameraDistance);
    g_cameraPitch = asinf(ClampFloat(offset.y / distance, -1.0f, 1.0f));
    g_cameraPitch = ClampFloat(g_cameraPitch, D3DXToRadian(-20.0f), D3DXToRadian(70.0f));
    g_cameraYaw = atan2f(offset.x, -offset.z);
}

void UpdateCameraByInput()
{
    const InputDevice::MousePosition mouseDelta = InputDevice::Mouse::GetDelta();
    if (mouseDelta.x != 0 || mouseDelta.y != 0)
    {
        const float sensitivity = g_remoteDesktopMode ? MOUSE_CAMERA_SENSITIVITY_REMOTE : MOUSE_CAMERA_SENSITIVITY_NORMAL;
        g_cameraYaw   -= static_cast<float>(mouseDelta.x) * sensitivity;
        g_cameraPitch  += static_cast<float>(mouseDelta.y) * sensitivity;
        g_cameraPitch  = ClampFloat(g_cameraPitch, D3DXToRadian(-20.0f), D3DXToRadian(70.0f));
    }
}

void UpdatePlayerByInput()
{
    const D3DXVECTOR3 previousRenderPosition = g_playerMover.GetPosition();
    const D3DXVECTOR3 cameraForward = GetCameraPlanarForward();
    const D3DXVECTOR3 cameraRight   = GetCameraPlanarRight(cameraForward);

    D3DXVECTOR3 localMove(0.0f, 0.0f, 0.0f);
    if (InputDevice::SKeyBoard::IsDown(DIK_W)) localMove.z += 1.0f;
    if (InputDevice::SKeyBoard::IsDown(DIK_S)) localMove.z -= 1.0f;
    if (InputDevice::SKeyBoard::IsDown(DIK_D)) localMove.x += 1.0f;
    if (InputDevice::SKeyBoard::IsDown(DIK_A)) localMove.x -= 1.0f;

    const bool isMoving  = (localMove.x != 0.0f || localMove.z != 0.0f);
    const bool isRunning = isMoving && InputDevice::SKeyBoard::IsDown(DIK_LSHIFT);

    PhysicsLib::CharacterMover::Settings settings = g_playerMover.GetSettings();
    const float walkSpeed = 1.5f;
    const float runSpeed = 4.5f;
    settings.moveSpeed = isRunning ? runSpeed : walkSpeed;
    g_playerMover.SetSettings(settings);

    D3DXVECTOR3 move(0.0f, 0.0f, 0.0f);
    if (isMoving)
    {
        const D3DXVECTOR3 desiredMove = cameraRight * localMove.x + cameraForward * localMove.z;
        const bool focusModeEnabled = PhysicsWorld::IsFocusModeEnabled();
        if (focusModeEnabled)
        {
            g_playerYaw = atan2f(-cameraForward.x, -cameraForward.z);
        }
        else
        {
            const float targetYaw = atan2f(-desiredMove.x, -desiredMove.z);
            g_playerYaw = MoveAngleToward(g_playerYaw,
                                          targetYaw,
                                          kPlayerTurnRadiansPerSecond * kTargetFrameSeconds);
        }
        move = desiredMove;
        D3DXVec3Normalize(&move, &move);
    }

    if (g_playerMeshId >= 0)
    {
        const bool isJumping = g_playerMover.IsJumping();

        PlayerAnimState nextState;
        if (isJumping)
        {
            nextState = PlayerAnimState::Jump;
        }
        else
        {
            if (isRunning)      nextState = PlayerAnimState::Run;
            else if (isMoving)  nextState = PlayerAnimState::Walk;
            else                nextState = PlayerAnimState::Idle;
        }

        if (nextState != g_playerAnimState)
        {
            g_playerAnimState = nextState;
            if (nextState == PlayerAnimState::Run)
            {
                g_Render.SetMeshMixSkinAnimSpeed(g_playerMeshId, 1.0f);
                g_Render.PlayMeshMixSkinAnimAnimation(g_playerMeshId, g_playerRunAnimName);
            }
            else if (nextState == PlayerAnimState::Walk)
            {
                g_Render.SetMeshMixSkinAnimSpeed(g_playerMeshId, 1.0f);
                g_Render.PlayMeshMixSkinAnimAnimation(g_playerMeshId, g_playerWalkAnimName);
            }
            else if (nextState == PlayerAnimState::Jump)
            {
                g_Render.SetMeshMixSkinAnimSpeed(g_playerMeshId, 0.1f);
                g_Render.PlayMeshMixSkinAnimAnimation(g_playerMeshId, g_playerRunAnimName);
            }
            else
            {
                g_Render.SetMeshMixSkinAnimSpeed(g_playerMeshId, 1.0f);
                g_Render.PlayMeshMixSkinAnimAnimation(g_playerMeshId, g_playerIdleAnimName);
            }
        }
    }

    // 衝突判定は後で行う。カメラはここで設定する。
    UpdatePlayerMeshAndCamera(previousRenderPosition);

    g_pendingMove = move;
    g_pendingJump = InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE);
}

D3DXVECTOR3 GetCameraPlanarForward()
{
    return D3DXVECTOR3(-sinf(g_cameraYaw), 0.0f, cosf(g_cameraYaw));
}

D3DXVECTOR3 GetCameraPlanarRight(const D3DXVECTOR3& forward)
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

void InitializePlayerPhysics()
{
    PhysicsWorld::Initialize();

    LoadPhysicsObjectsFromCsv(L"res\\model\\XFileListPhysics.csv");

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
    g_playerMover.SetSettings(settings);
    g_playerMover.Reset(PLAYER_START_POSITION);

    PhysicsLib::SettingsState::SetShapeType(PhysicsWorld::ShapeType::Cylinder);
    PhysicsLib::SettingsState::SetCylinderRadius(0.3f);
    PhysicsLib::SettingsState::SetCylinderHeight(1.7f);
}

void LoadPhysicsObjectsFromCsv(const std::wstring& csvPath)
{
    PhysicsWorld::LoadFromCsv(csvPath.c_str());
}

void UpdatePlayerMeshAndCamera(const D3DXVECTOR3& previousRenderPosition)
{
    const D3DXVECTOR3 currentRenderPosition = g_playerMover.GetPosition();
    if (g_playerMeshId >= 0)
    {
        if (g_playerIsSkinAnim)
        {
            g_Render.SetMeshMixSkinAnimPos(g_playerMeshId, currentRenderPosition);
            g_Render.SetMeshMixSkinAnimRotY(g_playerMeshId, g_playerYaw);
        }
        else
        {
            g_Render.SetMeshMixPos(g_playerMeshId, currentRenderPosition);
        }
    }

    // yaw/pitch/distanceから理想位置を作り、CameraMoverで壁めり込みを補正する。
    const D3DXVECTOR3 cameraTarget = currentRenderPosition + D3DXVECTOR3(0.0f, 1.2f, 0.0f);
    const float horizontalDistance = g_cameraDistance * cosf(g_cameraPitch);
    const D3DXVECTOR3 offset(sinf(g_cameraYaw) * horizontalDistance,
                              sinf(g_cameraPitch) * g_cameraDistance,
                              -cosf(g_cameraYaw) * horizontalDistance);
    const D3DXVECTOR3 desiredCameraPosition = cameraTarget + offset;
    const D3DXVECTOR3 cameraPosition = g_cameraMover.ResolvePosition(cameraTarget, desiredCameraPosition);
    g_Render.SetCamera(cameraPosition, cameraTarget);
}

INT_PTR CALLBACK SettingsDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_SETCHECK,
                    g_remoteDesktopMode ? BST_CHECKED : BST_UNCHECKED, 0);
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_CHECK1:
            g_remoteDesktopMode = (SendMessage(GetDlgItem(hDlg, IDC_CHECK1), BM_GETCHECK, 0, 0) == BST_CHECKED);
            return TRUE;

        case IDC_BUTTON_RESET_MOVING:
            g_Render.ResetMovingPlatforms();
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

LRESULT WINAPI MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        g_bClose = true;
        return 0;
    }
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void UpdateTitleByInput()
{
    if (InputDevice::UnifiedInput::IsDownFirstFrame(InputDevice::GAMEPAD_POV_LEFT))
    {
        g_command.Previous();
    }

    if (InputDevice::UnifiedInput::IsDownFirstFrame(InputDevice::GAMEPAD_POV_RIGHT))
    {
        g_command.Next();
    }
}

void UpdateSlideShow()
{
    if (g_slideShow == nullptr)
    {
        g_gameState = GameState::Playing;
        g_Render.Draw();
        return;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN) ||
        InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE) ||
        InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        g_slideShow->Next();
    }

    if (InputDevice::SKeyBoard::IsHoldDuration(DIK_SPACE, kSlideShowSkipHoldSeconds))
    {
        if (!g_slideShowSkipRequested)
        {
            g_slideShow->Skip();
            g_slideShowSkipRequested = true;
        }
    }
    else
    {
        g_slideShowSkipRequested = false;
    }

    if (g_slideShow->Update())
    {
        g_slideShow->Finalize();
        delete g_slideShow;
        g_slideShow = nullptr;
        g_gameState = GameState::Playing;
        g_slideShowSkipRequested = false;
        g_stageTitleFrame = kStageTitleFrameMax;
        g_Render.Draw();
        return;
    }

    g_slideShow->Render();
    DrawSlideShowSkipHint();
    g_Render.Draw();
}

void DrawSlideShowSkipHint()
{
    if (g_slideShowSkipHintFontId < 0)
    {
        g_slideShowSkipHintFontId = g_Render.SetUpFont(L"BIZ UDGothic", 18, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    g_Render.DrawTextCenter(g_slideShowSkipHintFontId,
                            L"Space長押しでスキップ",
                            1190,
                            820,
                            360,
                            40,
                            D3DCOLOR_RGBA(255, 255, 255, 190));
}

void DrawTitleScreen()
{
    if (g_titleFontId < 0)
    {
        g_titleFontId = g_Render.SetUpFont(L"BIZ UDGothic", 50, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    g_Render.DrawTextCenter(g_titleFontId, L"ホ  シ  ガ  ー  ル", 0, 220, NSRender::Common::BASE_W, 100);
    g_command.Draw();
    g_Render.Draw();
}

void DrawStageTitle()
{
    if (g_stageTitleFrame <= 0)
    {
        return;
    }

    if (g_stageTitleFontId < 0)
    {
        g_stageTitleFontId = g_Render.SetUpFont(L"BIZ UDGothic", 56, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    g_Render.DrawTextCenter(g_stageTitleFontId, L"Stage 1", 0, 260, NSRender::Common::BASE_W, 90);
    --g_stageTitleFrame;
}

POINT ConvertMouseToBaseResolution(int clientX, int clientY)
{
    RECT clientRect;
    GetClientRect(g_Render.GetWindowHandle(), &clientRect);

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


