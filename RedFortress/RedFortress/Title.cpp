
#include "Title.h"
#include "KeyBoard.h"
#include "Mouse.h"
#include "GamePad.h"
#include "Common.h"
#include "SharedObj.h"
#include "SoundEffect.h"
#include "SaveManager.h"
#include <Shlwapi.h>
#include "Camera.h"
#include "Rain.h"
#include "PopUp2.h"
#include <assert.h>

Title::Title(const bool blackFadeIn, const bool bFirst)
{
    m_bFirst = bFirst;

    m_titleCommand = NEW CommandManager();
    m_titleCommand->Init(CommandManager::eType::Title);

    m_sprBack = NEW Sprite(_T("res\\image\\black.png"));

    if (SharedObj::IsEnglish())
    {
        m_sprLogo = NEW Sprite(_T("res\\image\\title01.en.png"));
    }
    else
    {
        m_sprLogo = NEW Sprite(_T("res\\image\\title01.png"));
    }
    
    m_sprClock = NEW Sprite(_T("res\\image\\load_clock.png"));
    m_sprLoading = NEW Sprite(_T("res\\image\\loading.png"));

    Common::SetCursorVisibility(true);

    HRESULT ret = D3DXCreateFont(SharedObj::GetD3DDevice(),
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
                                 &m_font);

    if (blackFadeIn)
    {
        m_bFadeIn = true;
        m_bCameraFadeIn = false;

        m_fadeInAlpha = 255;
    }
    else
    {
        m_bFadeIn = false;
        m_bCameraFadeIn = true;

        m_fadeInAlpha = 0;
    }

    {
        auto map = SharedObj::GetMap();
        map->Update();
    }

    Rain::Get()->SetShow(false);
}

Title::~Title()
{
    Rain::Get()->SetShow(true);

    m_titleCommand->Finalize();
    SAFE_DELETE(m_titleCommand);
    SAFE_DELETE(m_sprBack);
    SAFE_DELETE(m_sprLogo);
    SAFE_DELETE(m_sprClock);
    SAFE_DELETE(m_sprLoading);
    SAFE_RELEASE(m_font);

    if (m_thread != nullptr)
    {
        m_thread->join();
        SAFE_DELETE(m_thread);
    }
}

void Title::Update(eSequence* sequence, eBattleState* eState)
{
    if (m_bFadeIn)
    {
        ++m_fadeInCount;
        m_fadeInAlpha = 255 - (m_fadeInCount * 255 / 60);
        if (m_fadeInCount >= FADE_IN)
        {
            m_bFadeIn = false;
            m_fadeInAlpha = 0;
        }
    }
    else if (m_bFadeOut)
    {
        ++m_fadeOutCount;
        m_fadeOutAlpha = m_fadeOutCount * 255 / 60;
        if (m_fadeOutCount >= FADE_OUT)
        {
            m_bFadeOut = false;
            m_fadeOutAlpha = 255;
            if (m_bLoading == false)
            {
                *eState = eBattleState::OPENING;
            }
        }
    }
    else if (m_bCameraFadeIn)
    {
        ++m_cameraFadeInCount;
        if (m_cameraFadeInCount >= Camera::MOVE_COUNT_MAX)
        {
            m_bCameraFadeIn = false;
            Camera::SetCameraMode(eCameraMode::TITLE);
            m_bFirst = false;
        }
    }
    else if (m_bCameraFadeOut)
    {
        ++m_cameraFadeOutCount;
        if (m_cameraFadeOutCount >= Camera::MOVE_COUNT_MAX)
        {
            m_bCameraFadeOut = false;
            Camera::SetCameraMode(eCameraMode::TITLE_TO_BATTLE);
            *eState = eBattleState::NORMAL;
        }
    }
    else if (m_bLoading == true)
    {
        if (m_loaded.load() == true)
        {
            m_bLoading = false;
            *eState = eBattleState::OPENING;
        }
    }
    else
    {
        std::wstring result = m_titleCommand->Operate();

        if (result == _T("start"))
        {
            // 即座にオープニングが始まるのではなく、
            // フェードアウトを描画し、
            // フェードアウトが完了したらオープニングが始まるようにする。
            m_eMenu = eTitleMenu::START;
            m_bFadeOut = true;
            m_fadeOutCount = 0;
            Common::SetCursorVisibility(false);

            bool saveExist = SaveManager::Get()->SaveFolderExists();

            // セーブデータがあったら初期データを読む。
            // セーブデータがなくても、一度ゲームを開始してから
            // タイトル画面に戻ってきたなら再読み込みをする。
            if (saveExist || !m_bFirst)
            {
                m_bLoading = true;
                SAFE_DELETE(m_thread);
                m_loaded.store(false);
                m_thread = NEW std::thread([&]
                                           {
                                               SaveManager::Get()->LoadOrigin();
                                               m_loaded.store(true);
                                           });
            }

            Rain::Get()->SetShow(true);
        }
        else if (result == _T("continue"))
        {
            m_eMenu = eTitleMenu::CONTINUE;
            m_bCameraFadeOut = true;
            m_cameraFadeOutCount = 0;
            Common::SetCursorVisibility(false);

            Camera::SetCameraMode(eCameraMode::TITLE_TO_BATTLE);
            auto ppos = SharedObj::GetPlayer()->GetPos();
            ppos.y += 1.0f;
            Camera::SetLookAtPos(ppos);

            Rain::Get()->SetShow(true);
        }
        else if (result == _T("demo"))
        {

            // 即座にオープニングが始まるのではなく、
            // フェードアウトを描画し、
            // フェードアウトが完了したらオープニングが始まるようにする。
            m_eMenu = eTitleMenu::DEMO;
            m_bFadeOut = true;
            m_fadeOutCount = 0;
            Common::SetCursorVisibility(false);

            bool demoExist = SaveManager::Get()->DemoFolderExists();
            assert(demoExist);

            if (demoExist)
            {
                m_bLoading = true;
                SAFE_DELETE(m_thread);
                m_loaded.store(false);
                m_thread = NEW std::thread([&]
                                           {
                                               SaveManager::Get()->LoadDemoData();
                                               m_loaded.store(true);
                                           });
            }

            Rain::Get()->SetShow(true);
        }
        else if (result == _T("language"))
        {
            m_titleCommand->Finalize();
            SAFE_DELETE(m_titleCommand);

            m_titleCommand = NEW CommandManager();
            m_titleCommand->Init(CommandManager::eType::Title_Language);
        }
        else if (result == _T("exit"))
        {
            m_eMenu = eTitleMenu::EXIT;
            *sequence = eSequence::EXIT;
        }
        else if (result == _T("japanese"))
        {
            PopUp2::Get()->SetText(_T("再起動時に有効になります。"));

            SaveManager::Get()->SetLangFile(_T("Japanese"));
        }
        else if (result == _T("english"))
        {
            PopUp2::Get()->SetText(_T("This will take effect after a restart."));

            SaveManager::Get()->SetLangFile(_T("English"));
        }
        else if (result == _T("back"))
        {
            m_titleCommand->Finalize();
            SAFE_DELETE(m_titleCommand);

            m_titleCommand = NEW CommandManager();
            m_titleCommand->Init(CommandManager::eType::Title);
        }
    }
}

void Title::Render()
{
    // 1フレーム目は、Update関数が呼ばれる前に呼ばれることに注意する

    D3DXVECTOR3 pos(0.f, 0.f, 0.f);

    if (m_bFadeIn)
    {
        m_sprBack->Render(pos, m_fadeInAlpha);
    }
    else if (m_bFadeOut)
    {
        m_sprBack->Render(pos, m_fadeOutAlpha);
    }
    else if (m_bLoading)
    {
        m_sprBack->Render(pos, m_fadeOutAlpha);
    }

    if (SharedObj::IsEnglish())
    {
        pos.x = 600.f;
    }
    else
    {
        pos.x = 630.f;
    }
    pos.y = 200.f;

    if (m_bLoading == false && m_bCameraFadeOut == false)
    {
        m_sprLogo->Render(pos);
    }

    if (m_bFadeIn == false &&
        m_bFadeOut == false &&
        m_bCameraFadeIn == false &&
        m_bCameraFadeOut == false &&
        m_bLoading == false)
    {
        m_titleCommand->Draw();
    }

    // ロード中ならくるくるを表示
    if (m_bLoading && m_loaded.load() == false)
    {

        static float counter = 0;
        counter += 0.05f;
        int temp1 = 0;

        D3DXVECTOR3 pos { 0.0f, 0.0f, 0.0f };
        pos.x = std::sin(counter) * 100 + 800;
        pos.y = std::cos(counter) * 60 + 450;
        temp1 = (int)((std::sin(counter * 0.2) + 1) * 64);
        m_sprClock->Render(pos, temp1);

        static float counter2 = 0;
        counter2 += 0.04f;

        pos.x = std::sin(counter2) * 60 + 800;
        pos.y = std::cos(counter2) * 90 + 450;
        temp1 = (int)((std::sin(counter2 * 0.2) + 1) * 128);
        m_sprClock->Render(pos, temp1);

        static int counter3 = 0;

        counter3 += 2;
        if (counter3 >= 256)
        {
            counter3 = 0;
        }

        int temp = 0;
        if (counter3 <= 127)
        {
            temp = counter3;
        }
        else
        {
            temp = 256 - counter3;
        }

        // 右方向に8ピクセル、下方向に31ピクセル移動すればクライアント領域？
        // 本当はちゃんとやらないといけない。

        pos = D3DXVECTOR3(800 - 40, 450 - 10, 0.0f);
        m_sprLoading->Render(pos, temp);
    }

    // GPU名を表示
    {
        std::wstring GPUName = Common::GetGPUName();
        RECT rect { };
        SetRect(&rect, 10, 850, 500, 890);
        m_font->DrawText(NULL,
                         GPUName.c_str(),
                         -1,
                         &rect,
                         DT_LEFT | DT_BOTTOM,
                         D3DCOLOR_ARGB(32, 255, 255, 255));
    }
}

bool Title::GetLoading() const
{
    return m_bLoading;
}

// 解像度やウィンドウモードを変更したときのための関数
void Title::OnDeviceLost()
{
    m_sprBack->OnDeviceLost();
    m_sprClock->OnDeviceLost();
    m_sprLoading->OnDeviceLost();
    m_sprLogo->OnDeviceLost();
    m_font->OnLostDevice();
}

// 解像度やウィンドウモードを変更したときのための関数
void Title::OnDeviceReset()
{
    m_sprBack->OnDeviceReset();
    m_sprClock->OnDeviceReset();
    m_sprLoading->OnDeviceReset();
    m_sprLogo->OnDeviceReset();
    m_font->OnResetDevice();
}

