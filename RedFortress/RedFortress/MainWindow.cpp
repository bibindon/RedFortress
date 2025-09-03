
#include "MainWindow.h"

#include <exception>
#include <chrono>
#include <random>
#include <cassert>

#include "KeyBoard.h"
#include "Mouse.h"
#include "GamePad.h"
#include "BGM.h"
#include "SoundEffect.h"
#include "Common.h"
#include "Camera.h"
#include "SharedObj.h"
#include "PopUp.h"
#include "PopUp2.h"
#include "SaveManager.h"
#include "resource.h"
#include "QuestManager.h"

#include "../../RedFortressModel/Model/HumanInfoManager.h"
#include "../../RedFortressModel/Model/MapInfoManager.h"
#include "../../RedFortressModel/Model/ItemManager.h"
#include "../../RedFortressModel/Model/Inventory.h"
#include "../../RedFortressModel/Model/Storehouse.h"
#include "../../RedFortressModel/Model/WeaponManager.h"
#include "../../RedFortressModel/Model/EnemyInfoManager.h"
#include "../../RedFortressModel/Model/SkillManager.h"
#include "../../RedFortressModel/Model/StatusManager.h"
#include "../../RedFortressModel/Model/Guide.h"
#include "../../RedFortressModel/Model/PowereggDateTime.h"
#include "../../RedFortressModel/Model/MapObjManager.h"
#include "../../RedFortressModel/Model/Rynen.h"
#include "../../RedFortressModel/Model/RainModel.h"
#include "../../RedFortressModel/Model/CraftInfoManager.h"
#include "../../RedFortressModel/Model/CraftSystem.h"
#include "../../RedFortressModel/Model/Help.h"
#include "../../RedFortressModel/Model/ActivityBase.h"
#include "NpcManager.h"
#include "Rain.h"

SeqBattle* MainWindow::m_seqBattle = nullptr;

using std::chrono::system_clock;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT mes, WPARAM wParam, LPARAM lParam)
{
    if (mes == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }
    // ウィンドウがアクティブor非アクティブになったときの処理
    // カメラとカーソルのモードを変える
    else if (mes == WM_ACTIVATE)
    {
        int lower = wParam & 0xFFFF;

        // アクティブになった時の処理
        if (lower == WA_ACTIVE || lower == WA_CLICKACTIVE)
        {
            auto seq = MainWindow::GetBattleSequence();
            if (seq != nullptr)
            {
                if (seq->GetState() == eBattleState::NORMAL)
                {
                    Camera::SetCameraMode(eCameraMode::BATTLE);
                    Common::SetCursorVisibility(false);
                }
                if (seq->GetState() == eBattleState::LOAD)
                {
                    Common::SetCursorVisibility(false);
                }
            }
        }
        // 非アクティブになった時の処理
        else if (lower == WA_INACTIVE)
        {
            Common::SetCursorVisibility(true);
        }
    }
    // ウィンドウを閉じる操作を行ったらセーブを自動実行する
    else if (mes == WM_CLOSE)
    {
        auto seq = MainWindow::GetBattleSequence();
        if (seq != nullptr)
        {
            if (seq->GetState() == eBattleState::NORMAL ||
                seq->GetState() == eBattleState::MENU)
            {
                if (!Common::DebugMode())
                {
                    SaveManager::Get()->Save();
                }
            }
        }
        // DestroyWindow(hWnd);
    }

    return DefWindowProc(hWnd, mes, wParam, lParam);
}

MainWindow::MainWindow(const HINSTANCE& hInstance, IKeyBoard* keyboard)
{
    //-------------------------------------------------
    // 解像度が1680x1050以下だったら警告を出す
    //-------------------------------------------------
    {
        int dispx = GetSystemMetrics(SM_CXSCREEN);
        int dispy = GetSystemMetrics(SM_CYSCREEN);
        if (dispx < 1680 || dispy < 1050)
        {
            MessageBox(NULL, _T("モニターの解像度が1680x1050以下の場合、正常動作しません。"), _T("警告"), MB_OK);
        }
    }

    //-------------------------------------------------
    // 言語設定
    // 英語と日本語のみ
    // システムが日本語だったら日本語、それ以外だったら英語
    // 設定ファイルで指定されていたらそちらが優先される
    //-------------------------------------------------
    {
        // システムのUI言語
        bool bJapan = false;

        std::wstring lang = SaveManager::Get()->GetLangFile();

        if (!lang.empty())
        {
            if (lang == _T("Japanese"))
            {
                bJapan = true;
            }
        }
        else
        {
            LANGID langId = GetUserDefaultUILanguage();
            if (langId == MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN))
            {
                bJapan = true;
            }
        }

//        bJapan = false;

        SharedObj::SetEnglish(!bJapan);

        if (bJapan)
        {
            SetThreadLocale(MAKELCID(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), SORT_DEFAULT));
            SetThreadUILanguage(MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN));

            m_title = L"ホシマン";
        }
        else
        {
            SetThreadLocale(MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT));
            SetThreadUILanguage(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));

            m_title = L"starman";
        }

    }

    SharedObj::Init();

    //-------------------------------------------------
    // ウィンドウクラスの登録
    //-------------------------------------------------
    HICON hIcon = (HICON)LoadImage(hInstance,
                                   MAKEINTRESOURCE(IDI_ICON1),
                                   IMAGE_ICON,
                                   GetSystemMetrics(SM_CXICON),
                                   GetSystemMetrics(SM_CYICON),
                                   0);

    WNDCLASSEX wcex { };

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = hIcon;
    wcex.hCursor = nullptr;
    wcex.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = m_title.c_str();
    wcex.hIconSm = nullptr;

    auto result = RegisterClassEx(&wcex);
    if (!result)
    {
        DWORD err = GetLastError();
        throw std::exception(Common::WstringToUtf8(L"RegisterClassEx関数の実行が失敗しました。").c_str());
    } 

    //-------------------------------------------------
    // ウィンドウの作成
    //-------------------------------------------------
    {
        //-------------------------------------------------
        // ウィンドウが画面中央に表示されるように設定
        //-------------------------------------------------
        RECT rect { };
        SetRect(&rect, 0, 0, 1600, 900);

        // タイトルバー、ウィンドウフレームの幅を考慮したサイズを取得
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        rect.right = rect.right - rect.left;
        rect.bottom = rect.bottom - rect.top;

        int monitorWidth = GetSystemMetrics(SM_CXSCREEN);
        int monitorHeight = GetSystemMetrics(SM_CYSCREEN);

        int startX = (monitorWidth / 2) - (1600 / 2);
        int startY = (monitorHeight / 2) - (900 / 2);

        m_hWnd = CreateWindow(m_title.c_str(),
                              m_title.c_str(),
                              /* ウィンドウサイズの変更をさせない。最小化はOK */
                              WS_OVERLAPPEDWINDOW ^ WS_MAXIMIZEBOX ^ WS_THICKFRAME | WS_VISIBLE,
                              startX,
                              startY,
                              rect.right,
                              rect.bottom,
                              NULL,
                              NULL,
                              hInstance,
                              NULL);

        if (!m_hWnd)
        {
            throw std::exception(Common::WstringToUtf8(L"ウィンドウの作成ができませんでした").c_str());
        }

        SharedObj::SetWindowHandle(m_hWnd);
    }

    //-------------------------------------------------
    // Direct3Dの初期化
    //-------------------------------------------------
    {
        m_D3D = Direct3DCreate9(D3D_SDK_VERSION);

        if (!m_D3D)
        {
            throw std::exception(Common::WstringToUtf8(L"DirectXの初期化に失敗しました").c_str());
        }
    }

    HRESULT hResult = E_FAIL;

    //-------------------------------------------------
    // Direct3DDeviceの初期化
    //-------------------------------------------------
    {
        //-------------------------------------------------
        // DirectX9ではプログラムからGPUの切り替えができない。
        // 使用するGPUを変えたい場合は、ユーザーがWindowsの設定画面で設定するしかない。
        //-------------------------------------------------

        // グラフィックボードの名称を取得
        D3DADAPTER_IDENTIFIER9 adapterInfo { };
        m_D3D->GetAdapterIdentifier(D3DADAPTER_DEFAULT, 0, &adapterInfo);

        std::wstring GPUName = Common::Utf8ToWstring(adapterInfo.Description);
        Common::SetGPUName(GPUName);

        // TODO フルスクリーン対応

        D3DPRESENT_PARAMETERS d3dpp { };
        d3dpp.BackBufferWidth = 0;
        d3dpp.BackBufferHeight = 0;
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
        d3dpp.BackBufferCount = 0;

        // マルチサンプリングをオフにする
        // VirtualBoxだとサポートしていない
#ifdef MULTI_SAMPLE_NONE
        d3dpp.MultiSampleType = D3DMULTISAMPLE_NONE;
#else
        d3dpp.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;
#endif
        d3dpp.MultiSampleQuality = 0;

        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.hDeviceWindow = nullptr;
        d3dpp.Windowed = TRUE;
        d3dpp.EnableAutoDepthStencil = TRUE;
        d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
        d3dpp.Flags = 0;

        d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
        d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;

        LPDIRECT3DDEVICE9 D3DDevice = nullptr;

        hResult = m_D3D->CreateDevice(D3DADAPTER_DEFAULT,
                                      D3DDEVTYPE_HAL,
                                      m_hWnd,
                                      D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                                      &d3dpp,
                                      &D3DDevice);

        if (FAILED(hResult))
        {
            hResult = m_D3D->CreateDevice(D3DADAPTER_DEFAULT,
                                          D3DDEVTYPE_HAL,
                                          m_hWnd,
                                          D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                                          &d3dpp,
                                          &D3DDevice);
            if (FAILED(hResult))
            {
                throw std::exception(Common::WstringToUtf8(L"Direct3D::CreateDevice関数が失敗しました").c_str());
            }
        }

        SharedObj::SetD3DDevice(D3DDevice);

//        // 試しに異方性フィルタを使う
//        {
//            D3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
//            D3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
//            D3DDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
//            D3DDevice->SetSamplerState(0, D3DSAMP_MAXANISOTROPY, 8);
//        }
    }

    //-------------------------------------------------
    // DirectInputの初期化
    //-------------------------------------------------
    {
        // directinput
        hResult = DirectInput8Create(hInstance,
                                     DIRECTINPUT_VERSION,
                                     IID_IDirectInput8,
                                     (LPVOID*)&m_directInput,
                                     NULL);

        if (FAILED(hResult))
        {
            throw std::exception(Common::WstringToUtf8(L"DirectInput8Create関数が失敗しました").c_str());
        }
    }

    //-------------------------------------------------
    // フォントの初期化
    //-------------------------------------------------
    {
        hResult = D3DXCreateFont(SharedObj::GetD3DDevice(),
                                 Common::FontSizeEng() - 4,
                                 0,
                                 Common::FontThicknessEng(),
                                 1,
                                 false,
                                 DEFAULT_CHARSET,
                                 OUT_TT_ONLY_PRECIS,
                                 CLEARTYPE_NATURAL_QUALITY,
                                 FF_DONTCARE,
                                 Common::FontNameEng(),
                                 &m_D3DFont);

        assert(hResult == S_OK);
    }


    {
        // 一時的にシステムにフォントを登録
        AddFontResourceEx(_T("res\\font\\BIZUDMincho-Regular.ttf"), FR_PRIVATE, nullptr);
    }

    //-------------------------------------------------
    // キーボード、マウス、ゲームパッド、サウンドの初期化
    //-------------------------------------------------
    {
        keyboard->Init(m_directInput, m_hWnd);
        SharedObj::SetKeyBoard(keyboard);

        Mouse::Init(m_directInput, m_hWnd);
        GamePad::Init(m_directInput, m_hWnd);
        BGMManager::Get()->Init(m_hWnd);
        SoundEffect::initialize(m_hWnd);
    }

    // PopUpクラスの初期化
    PopUp::Init(SharedObj::IsEnglish());

    // PopUp2クラスの初期化
    IPopUpFont* popUpFont = NEW PopUpFont(SharedObj::GetD3DDevice(), SharedObj::IsEnglish());
    PopUp2::Init(popUpFont);

    //-------------------------------------------------
    // m_seqBattleの初期化
    //
    // SeqBattleクラスのコンストラクタはDirectXを使用するので
    // MainWindowクラスのコンストラクタの最後のほうで生成する必要がある
    //-------------------------------------------------
    m_seqBattle = NEW SeqBattle();

    // Sleep関数の精度を上げる
    timeBeginPeriod(1);

    // ウィンドウを表示
    ShowWindow(m_hWnd, SW_SHOW);
}

MainWindow::~MainWindow()
{
    // 逆の順番で解放する

    SAFE_DELETE(m_seqEnding);
    SAFE_DELETE(m_seqBattle);

    SaveManager::Destroy();

    NSModel::Voyage::Destroy();
    NSModel::PatchTestManager::Destroy();
    NSModel::MapObjManager::Destroy();
    NSModel::PowereggDateTime::Destroy();
    NSModel::Guide::Destroy();
    NSModel::StatusManager::Destroy();
    NSModel::SkillManager::Destroy();
    NSModel::EnemyInfoManager::Destroy();
    NSModel::WeaponManager::Destroy();
    NSModel::StorehouseManager::Destroy();
    NSModel::Inventory::Destroy();
    NSModel::ItemManager::Destroy();
    NSModel::MapInfoManager::Destroy();
    NSModel::HumanInfoManager::Destroy();
    NSModel::Rynen::Destroy();
    NSModel::RainModel::Get()->Destroy();
    NSModel::CraftInfoManager::Destroy();
    NSModel::CraftSystem::Destroy();
    NSModel::NpcStatusManager::Destroy();
    NSModel::Help::Destroy();
    NSModel::ActivityBase::Get()->Finalize();

    QuestManager::Finalize();

    PopUp2::Finalize();
    PopUp::Finalize();

    SoundEffect::finalize();
    BGMManager::Get()->Finalize();
    GamePad::Finalize();
    Mouse::Finalize();
    SharedObj::KeyBoard()->Finalize();
    SharedObj::SetKeyBoard(nullptr);

    NpcManager::Finalize();
    Rain::Finalize();
    VoyageManager::Destroy();

    SAFE_RELEASE(m_D3DFont);
    SAFE_RELEASE(m_directInput);

    SharedObj::Finalize();

    SAFE_RELEASE(m_D3D);

    timeEndPeriod(1);
}

int MainWindow::MainLoop()
{
    do
    {
        if (PeekMessage(&m_msg, NULL, 0, 0, PM_REMOVE))
        {
            DispatchMessage(&m_msg);
        }

        // 60FPSになるようにスリープ時間を調節
        {
            static system_clock::time_point tpStart = system_clock::now();
            static system_clock::time_point tpEnd = system_clock::now();
            tpEnd = system_clock::now();

            std::chrono::milliseconds dura =
                std::chrono::duration_cast<std::chrono::milliseconds>(tpEnd - tpStart);
            int dura_i = (int)dura.count();

            if (dura_i >= 16 || dura_i <= -1)
            {
                // Do nothing
            }
            else
            {
                Sleep(16 - dura_i -1);
            }

            tpStart = system_clock::now();
        }

        // 実際のFPSを求める
        int fps = 0;
        if (Common::DebugMode())
        {
            fps = CalcFPS();
        }

        SharedObj::KeyBoard()->Update();
        Mouse::Update();
        GamePad::Update();

        static int counter = 0;
        counter++;

        if (counter % 60 == 59)
        {
            BGMManager::Get()->Update();
        }

        if (m_sequence == eSequence::BATTLE)
        {
            m_seqBattle->Update(&m_sequence);
            if (m_sequence == eSequence::ENDING)
            {
                SAFE_DELETE(m_seqBattle);
                m_seqEnding = NEW SeqEnding();
            }
        }
        else if (m_sequence == eSequence::ENDING)
        {
            m_seqEnding->Update(&m_sequence);
            if (m_sequence == eSequence::BATTLE)
            {
                SAFE_DELETE(m_seqEnding);
                m_seqBattle = NEW SeqBattle();
            }
        }
        else if (m_sequence == eSequence::EXIT)
        {
            PostMessage(m_hWnd, WM_CLOSE, 0, 0);
        }

        //------------------------------------------------------
        // 描画処理
        //------------------------------------------------------
        LPDIRECT3DDEVICE9 D3DDevice = SharedObj::GetD3DDevice();

        D3DDevice->Clear(0,
                         NULL,
                         D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                         D3DCOLOR_XRGB(40, 40, 80),
                         1.0f, 0);

        D3DDevice->BeginScene();

        if (m_sequence == eSequence::BATTLE)
        {
            m_seqBattle->Render();
        }
        else if (m_sequence == eSequence::ENDING)
        {
            m_seqEnding->Render();
        }

        if (Common::DebugMode())
        {
            ShowDebugInfo(fps);
        }

        D3DDevice->EndScene();
        D3DDevice->Present(NULL, NULL, NULL, NULL);
    }
    while (m_msg.message != WM_QUIT);

    return 0;
}

SeqBattle* MainWindow::GetBattleSequence()
{
    return m_seqBattle;
}

void MainWindow::ShowDebugInfo(const int fps)
{
    std::wstring work;

    RECT rect { };

    //-------------------------------------------------------
    // FPSの表示
    //-------------------------------------------------------
    {
        SetRect(&rect, 0, 0, 50, 50);

        work = _T(" fps : ") + std::to_wstring(fps);
        m_D3DFont->DrawText(NULL,
                            work.c_str(),
                            -1,
                            &rect,
                            DT_LEFT | DT_NOCLIP,
                            D3DCOLOR_ARGB(128, 255, 255, 255));
    }

    //-------------------------------------------------------
    // プレイヤーの座標表示
    //-------------------------------------------------------
    {
        D3DXVECTOR3 pos(0.f, 0.f, 0.f);

        if (SharedObj::GetPlayer() != nullptr)
        {
            pos = SharedObj::GetPlayer()->GetPos();
        }

        // X座標の表示
        SetRect(&rect, 0, 25, 100, 100);
        work = _T(" x : ") + std::to_wstring(pos.x);

        m_D3DFont->DrawText(NULL,
                            work.c_str(),
                            -1,
                            &rect,
                            DT_LEFT | DT_NOCLIP,
                            D3DCOLOR_ARGB(128, 255, 255, 255));

        // Y座標の表示
        SetRect(&rect, 0, 50, 100, 100);
        work = _T(" y : ") + std::to_wstring(pos.y);

        m_D3DFont->DrawText(NULL,
                            work.c_str(),
                            -1,
                            &rect,
                            DT_LEFT | DT_NOCLIP,
                            D3DCOLOR_ARGB(128, 255, 255, 255));

        // Z座標の表示
        SetRect(&rect, 0, 75, 100, 100);
        work = _T(" z : ") + std::to_wstring(pos.z);

        m_D3DFont->DrawText(NULL,
                            work.c_str(),
                            -1,
                            &rect,
                            DT_LEFT | DT_NOCLIP,
                            D3DCOLOR_ARGB(128, 255, 255, 255));
    }
}

int MainWindow::CalcFPS()
{
    //--------------------------------------------------------
    // 毎フレーム、現在時刻を記録し、リストの末尾に追加する。
    // 先頭ほど古い時刻が記録される。
    // リストの先頭から、記録された時刻と現在時刻の差を比較していくと、
    // 最初は1秒以上の差があるが、やがて1秒以下の要素が見つかる。（Aとする）
    // そのときの、A以降の要素の総数がFPSである
    //--------------------------------------------------------

    int fps = 0;

    const int TIME_RECORD_LENGTH = 150;

    system_clock::time_point tempTime = system_clock::now();

    m_vecTime.push_back(tempTime);
    const int removeCnt = (int)m_vecTime.size() - TIME_RECORD_LENGTH;
    if (removeCnt >= 1)
    {
        m_vecTime.erase(m_vecTime.begin(), m_vecTime.begin() + removeCnt);
    }

    tempTime = tempTime - std::chrono::seconds(1);

    for (int i = 0; i < TIME_RECORD_LENGTH; ++i)
    {
        if (m_vecTime.size() == TIME_RECORD_LENGTH)
        {
            if (tempTime < m_vecTime.at(i))
            {
                fps = TIME_RECORD_LENGTH - i;
                break;
            }
        }
    }

    return fps;
}

