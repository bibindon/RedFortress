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
#include <fstream>
#include <sstream>
#include <vector>

#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../SoundLib/SoundLib/SoundLib.h"
#include "resource.h"

bool g_bClose = false;
const std::wstring g_arrowSoundPath = L"res\\sound\\arrow.wav";
NSRender::Render g_Render;
using PhysicsWorld = PhysicsLib::PhysicsLib;
const float CAMERA_MOVE_SPEED = 0.08f;
const float CAMERA_FAST_MOVE_SPEED = 0.25f;
const float MOUSE_CAMERA_SENSITIVITY = 0.0001f;
const D3DXVECTOR3 PLAYER_START_POSITION(0.0f, 0.5f, 0.0f);
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
enum class PlayerAnimState { Idle, Walk, Run };
PlayerAnimState g_playerAnimState = PlayerAnimState::Idle;
bool g_mouseCursorVisible = false;

static void UpdateCameraByInput();
static void UpdatePlayerByInput();
static D3DXVECTOR3 GetCameraPlanarForward();
static D3DXVECTOR3 GetCameraPlanarRight(const D3DXVECTOR3& forward);
static void InitializeCameraFromRenderSettings();
static void InitializePlayerPhysics();
static void LoadPhysicsObjectsFromCsv(const std::wstring& csvPath);
static void UpdatePlayerMeshAndCamera(const D3DXVECTOR3& previousRenderPosition);
static INT_PTR CALLBACK SettingsDialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
std::wstring TrimCsvField(const std::wstring& text)
{
    const std::wstring whitespace = L" \t\r\n";
    const std::size_t first = text.find_first_not_of(whitespace);
    if (first == std::wstring::npos)
    {
        return L"";
    }

    const std::size_t last = text.find_last_not_of(whitespace);
    return text.substr(first, last - first + 1);
}

std::vector<std::wstring> SplitCsvLine(const std::wstring& line)
{
    std::vector<std::wstring> fields;
    std::wstring current;
    bool insideQuotes = false;

    for (wchar_t ch : line)
    {
        if (ch == L'"')
        {
            insideQuotes = !insideQuotes;
            continue;
        }

        if (ch == L',' && !insideQuotes)
        {
            fields.push_back(TrimCsvField(current));
            current.clear();
            continue;
        }

        current.push_back(ch);
    }

    fields.push_back(TrimCsvField(current));
    return fields;
}

std::wstring GetParentDirectoryPath(const std::wstring& path)
{
    const std::size_t pos = path.find_last_of(L"\\/");
    if (pos == std::wstring::npos)
    {
        return L"";
    }

    return path.substr(0, pos);
}

std::wstring ResolveRelativePath(const std::wstring& baseDirectory, const std::wstring& relativePath)
{
    if (relativePath.empty())
    {
        return L"";
    }

    if (relativePath.size() > 1 && relativePath[1] == L':')
    {
        return relativePath;
    }

    if (!relativePath.empty() && (relativePath[0] == L'\\' || relativePath[0] == L'/'))
    {
        return relativePath;
    }

    if (baseDirectory.empty())
    {
        return relativePath;
    }

    return baseDirectory + L"\\" + relativePath;
}
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

    ShowWindow(hWnd, SW_SHOWDEFAULT);
    UpdateWindow(hWnd);

    g_Render.Initialize(hWnd, L"res\\RenderSettings.csv");
    g_Render.ChangeResolution(1600, 900);
    g_Render.SetShowFPS(false);
    g_Render.SetLightDir(D3DXVECTOR3(-0.4f, 1.0f, 0.6f));
    g_Render.LoadXFileListFromCsv(L"res\\model\\XFileList_simple.csv");
    g_playerMeshId = g_Render.AddMeshMixSkinAnim(L"res\\model2\\separatedAnim\\wolfAnim.x",
                                                 L"res\\model2\\separatedAnim\\wolfAnim.csv",
                                                 PLAYER_START_POSITION,
                                                 D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                 1.0f,
                                                 NSRender::AnimSetMap());
    InitializePlayerPhysics();
    InitializeCameraFromRenderSettings();
    UpdatePlayerMeshAndCamera(PLAYER_START_POSITION);

    InputDevice::Initialize(hInstance, hWnd);
    InputDevice::Mouse::SetVisible(g_mouseCursorVisible);
    SoundLib::SoundLib::Initialize(hWnd);
    SoundLib::SoundLib::LoadSoundEffect(g_arrowSoundPath);

    MSG msg;

    while (true)
    {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            DispatchMessage(&msg);
        }

        InputDevice::Update();
        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_ESCAPE))
        {
            g_mouseCursorVisible = !g_mouseCursorVisible;
            InputDevice::Mouse::SetVisible(g_mouseCursorVisible);
        }

        // マウスカーソル表示中はUI操作を優先し、カメラ回転を止める。
        if (!g_mouseCursorVisible)
        {
            UpdateCameraByInput();
        }
        UpdatePlayerByInput();

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

        if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F3))
        {
            DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, SettingsDialogProc);
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

        g_Render.Draw();
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
        g_cameraYaw   -= static_cast<float>(mouseDelta.x) * 0.00025f;
        g_cameraPitch  += static_cast<float>(mouseDelta.y) * 0.00025f;
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
    const float baseSpeed = 6.0f;
    settings.moveSpeed = isRunning ? baseSpeed * 1.5f : baseSpeed;
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
        PlayerAnimState nextState = PlayerAnimState::Idle;
        if (isRunning)     nextState = PlayerAnimState::Run;
        else if (isMoving) nextState = PlayerAnimState::Walk;

        if (nextState != g_playerAnimState)
        {
            g_playerAnimState = nextState;
            if (nextState == PlayerAnimState::Run)
                g_Render.PlayMeshMixSkinAnimAnimation(g_playerMeshId, L"run");
            else if (nextState == PlayerAnimState::Walk)
                g_Render.PlayMeshMixSkinAnimAnimation(g_playerMeshId, L"walk");
            else
                g_Render.PlayMeshMixSkinAnimAnimation(g_playerMeshId, L"idle");
        }
    }

    g_playerMover.Update(move, InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE));
    UpdatePlayerMeshAndCamera(previousRenderPosition);
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

    LoadPhysicsObjectsFromCsv(L"res\\model\\XFileList_simple.csv");

    PhysicsLib::CharacterMover::Settings settings;
    settings.shapeType = PhysicsWorld::ShapeType::Cylinder;
    settings.shapeOffset = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    settings.radius = 0.45f;
    settings.height = 1.0f;
    settings.moveSpeed = 6.0f;
    settings.groundAcceleration = 18.0f;
    settings.airAcceleration = 8.0f;
    settings.jumpVelocity = 5.0f;
    settings.airControlEnabled = true;
    g_playerMover.SetSettings(settings);
    g_playerMover.Reset(PLAYER_START_POSITION);
}

void LoadPhysicsObjectsFromCsv(const std::wstring& csvPath)
{
    std::wifstream file(csvPath);
    if (!file)
    {
        return;
    }

    const std::wstring baseDirectory = GetParentDirectoryPath(csvPath);
    std::wstring line;
    while (std::getline(file, line))
    {
        const std::wstring trimmedLine = TrimCsvField(line);
        if (trimmedLine.empty() || trimmedLine.front() == L'#')
        {
            continue;
        }

        const std::vector<std::wstring> fields = SplitCsvLine(trimmedLine);
        if (fields.size() < 5)
        {
            continue;
        }

        const std::wstring modelPath = ResolveRelativePath(baseDirectory, fields[0]);
        const int objectId = PhysicsWorld::Load(modelPath.c_str(), PhysicsWorld::ObjectType::Slide, 1.0f);
        if (objectId < 0)
        {
            continue;
        }

        try
        {
            const D3DXVECTOR3 position(static_cast<float>(std::stof(fields[1])),
                                       static_cast<float>(std::stof(fields[2])),
                                       static_cast<float>(std::stof(fields[3])));
            const D3DXVECTOR3 rotation(0.0f,
                                       D3DXToRadian(static_cast<float>(std::stof(fields[4]))),
                                       0.0f);
            PhysicsWorld::SetTransform(objectId, position, rotation);
        }
        catch (...)
        {
        }
    }
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
        return TRUE;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            EndDialog(hDlg, LOWORD(wParam));
            return TRUE;
        }
        break;

    case WM_CLOSE:
        EndDialog(hDlg, IDCANCEL);
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


