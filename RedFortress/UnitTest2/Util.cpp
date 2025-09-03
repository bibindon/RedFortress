#include "Util.h"
#include "../starman/SharedObj.h"
#include "../starman/GamePad.h"
#include "../starman/SoundEffect.h"
#include "../starman/BGM.h"
#include "../starman/Mouse.h"
#include "../starman/KeyBoard.h"
#include "../../RedFortressModel/Model/Rynen.h"
#include "../../RedFortressModel/Model/HumanInfoManager.h"
#include "../../RedFortressModel/Model/MapInfoManager.h"
#include "../../RedFortressModel/Model/Storehouse.h"
#include "../../RedFortressModel/Model/WeaponManager.h"
#include "../../RedFortressModel/Model/EnemyInfoManager.h"
#include "../../RedFortressModel/Model/SkillManager.h"
#include "../../RedFortressModel/Model/PowereggDateTime.h"
#include "../../RedFortressModel/Model/Guide.h"
#include "../starman/SaveManager.h"
#include <cassert>
#include "../../RedFortressModel/Model/CraftInfoManager.h"
#include "../../RedFortressModel/Model/CraftSystem.h"

LPDIRECT3D9 Util::m_D3D = nullptr;
LPDIRECTINPUT8 Util::m_directInput = nullptr;

void Util::InitWin_DX9_DI8(const bool bShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = _T("MyWindowClass");

    RegisterClass(&wc);

    auto hInstance = (HINSTANCE)GetModuleHandle(0);
    HWND hWnd = CreateWindow(_T("MyWindowClass"),
                             _T("dummy"),
                             WS_OVERLAPPEDWINDOW,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             NULL,
                             NULL,
                             hInstance,
                             NULL);

    SharedObj::SetWindowHandle(hWnd);

    if (bShow)
    {
        ShowWindow(SharedObj::GetWindowHandle(), SW_SHOW);
    }

    // Direct3Dの初期化
    m_D3D = nullptr;
    if (!(m_D3D = Direct3DCreate9(D3D_SDK_VERSION)))
    {
        throw std::exception((""));
    }

    LPDIRECT3DDEVICE9 D3DDevice;

    D3DPRESENT_PARAMETERS d3dpp = {
        0,
        0,
        D3DFMT_UNKNOWN,
        0,
        D3DMULTISAMPLE_NONE,
        0,
        D3DSWAPEFFECT_DISCARD,
        NULL,
        TRUE,
        TRUE,
        D3DFMT_D24S8,
        0,
        D3DPRESENT_RATE_DEFAULT,
        D3DPRESENT_INTERVAL_DEFAULT
    };

    auto result = m_D3D->CreateDevice(0,
                                    D3DDEVTYPE_HAL,
                                    hWnd,
                                    D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                                    &d3dpp,
                                    &D3DDevice);

    if (FAILED(result))
    {
        throw std::exception((""));
    }

    SharedObj::SetD3DDevice(D3DDevice);

    m_directInput = nullptr;

    HRESULT ret = DirectInput8Create(hInstance,
                                     DIRECTINPUT_VERSION,
                                     IID_IDirectInput8,
                                     (LPVOID*)&m_directInput,
                                     NULL);

    Mouse::Init(m_directInput, hWnd);
    GamePad::Init(m_directInput, hWnd);
    BGMManager::Get()->Init(hWnd);
    SoundEffect::initialize(hWnd);
}

void Util::ReleaseWin_DX9_DI8()
{
    Mouse::Finalize();
    GamePad::Finalize();
    BGMManager::Get()->Finalize();
    SoundEffect::finalize();

    if (SharedObj::KeyBoard() != nullptr)
    {
        delete SharedObj::KeyBoard();
    }
    SharedObj::SetKeyBoard(nullptr);

    if (SharedObj::GetPlayer() != nullptr)
    {
        delete SharedObj::GetPlayer();
    }
    SharedObj::SetPlayer(nullptr);

    if (SharedObj::GetMap() != nullptr)
    {
        delete SharedObj::GetMap();
    }
    SharedObj::SetMap(nullptr);

    SharedObj::Finalize();
    SAFE_RELEASE(m_D3D);
    SAFE_RELEASE(m_directInput);

    DestroyWindow(SharedObj::GetWindowHandle());

    // DestroyWindowを行った後、メッセージ処理が完了するのを待つ
    MSG msg = { 0 };
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void MockPopUpFont::Draw(const std::wstring& text, const int transparent)
{
    m_text = text;
}

std::wstring MockPopUpFont::GetShowText()
{
    return m_text;
}

void MockKeyBoard::Init(LPDIRECTINPUT8, HWND)
{
}

void MockKeyBoard::Update()
{
    if (m_bAltF4)
    {
        PostMessage(SharedObj::GetWindowHandle(), WM_CLOSE, 0, 0);
        m_bAltF4 = false;
    }

    std::copy(&m_bDownFirstReserve[0], &m_bDownFirstReserve[256], &m_bDownFirst[0]);
    std::fill(&m_bDownFirstReserve[0], &m_bDownFirstReserve[256], false);
}

void MockKeyBoard::Finalize()
{
}

bool MockKeyBoard::IsDown(int keyCode)
{
    return m_bDown[keyCode];
}

bool MockKeyBoard::IsDownFirstFrame(int keyCode)
{
    return m_bDownFirst[keyCode];
}

bool MockKeyBoard::IsHold(int keyCode)
{
    return m_bHold[keyCode];
}

void MockKeyBoard::SetKeyDown(int keyCode)
{
    m_bDown[keyCode] = true;
}

void MockKeyBoard::SetKeyDownFirst(int keyCode)
{
    m_bDownFirstReserve[keyCode] = true;
}

void MockKeyBoard::SetKeyHold(int keyCode)
{
    m_bHold[keyCode] = true;
}

void MockKeyBoard::ClearAll()
{
    std::fill(&m_bDown[0], &m_bDown[256], false);
    std::fill(&m_bDownFirst[0], &m_bDownFirst[256], false);
    std::fill(&m_bDownFirstReserve[0], &m_bDownFirstReserve[256], false);
    std::fill(&m_bHold[0], &m_bHold[256], false);
}

void MockKeyBoard::SetAltF4()
{
    m_bAltF4 = true;
}

void Util::DestroyLibData()
{
    NSStarmanLib::Rynen::Destroy();
    NSStarmanLib::HumanInfoManager::Destroy();
    NSStarmanLib::MapInfoManager::Destroy();
    NSStarmanLib::ItemManager::Destroy();
    NSStarmanLib::Inventory::Destroy();
    NSStarmanLib::StorehouseManager::Destroy();
    NSStarmanLib::WeaponManager::Destroy();
    NSStarmanLib::EnemyInfoManager::Destroy();
    NSStarmanLib::SkillManager::Destroy();
    NSStarmanLib::StatusManager::Destroy();
    NSStarmanLib::Guide::Destroy();
    NSStarmanLib::PowereggDateTime::Destroy();
    NSStarmanLib::MapObjManager::Destroy();
    NSStarmanLib::Voyage::Destroy();
    NSStarmanLib::CraftInfoManager::Destroy();
    NSStarmanLib::CraftSystem::Destroy();
    SaveManager::Destroy();
}

BOOL Util::DeleteDirectory(LPCTSTR dirPath)
{
    TCHAR searchPath[MAX_PATH];
    WIN32_FIND_DATA findData;
    HANDLE hFind;

    // パス + _T("\*") を作成
    lstrcpy(searchPath, dirPath);
    lstrcat(searchPath, _T("\\*"));

    hFind = FindFirstFile(searchPath, &findData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    do
    {
        // _T("." と "..") はスキップ
        if (lstrcmp(findData.cFileName, _T(".")) == 0 ||
            lstrcmp(findData.cFileName, _T("..")) == 0)
        {
            continue;
        }

        // フルパスを作成
        TCHAR fullPath[MAX_PATH];
        lstrcpy(fullPath, dirPath);
        lstrcat(fullPath, _T("\\"));
        lstrcat(fullPath, findData.cFileName);

        // ディレクトリなら再帰
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            DeleteDirectory(fullPath);
        }
        else
        {
            // ファイル属性を標準に戻してから削除
            SetFileAttributes(fullPath, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(fullPath);
        }

    } while (FindNextFile(hFind, &findData));

    FindClose(hFind);

    // 最後にこのフォルダ自身を削除
    BOOL result = SetFileAttributes(dirPath, FILE_ATTRIBUTE_NORMAL);

    assert(result == TRUE);

    return RemoveDirectory(dirPath);
}

bool Util::IsDirectory(const std::wstring& path)
{
    DWORD attribs = GetFileAttributes(path.c_str());
    return (attribs != INVALID_FILE_ATTRIBUTES && (attribs & FILE_ATTRIBUTE_DIRECTORY));
}

