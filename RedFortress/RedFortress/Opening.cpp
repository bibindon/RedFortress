#include "Opening.h"
#include "KeyBoard.h"
#include "Mouse.h"
#include "GamePad.h"
#include "Common.h"
#include "SharedObj.h"
#include "BGM.h"
#include "SoundEffect.h"
#include "Camera.h"
#include <cassert>

using namespace NSSlideShow;

namespace NSSlideShow
{
class Sprite : public ISprite
{
public:

    Sprite(LPDIRECT3DDEVICE9 dev)
        : m_pD3DDevice(dev)
    {
    }

    void DrawImage(const int x, const int y, const int transparency) override
    {
        D3DXVECTOR3 pos { (float)x, (float)y, 0.f };
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect = {
            0,
            0,
            static_cast<LONG>(m_width),
            static_cast<LONG>(m_height) };
        D3DXVECTOR3 center { 0, 0, 0 };
        m_D3DSprite->Draw(
            m_pD3DTexture,
            &rect,
            &center,
            &pos,
            D3DCOLOR_ARGB(transparency, 255, 255, 255));
        m_D3DSprite->End();

    }

    void Load(const std::wstring& filepath) override
    {
        LPD3DXSPRITE tempSprite { nullptr };
        if (FAILED(D3DXCreateSprite(m_pD3DDevice, &m_D3DSprite)))
        {
            throw std::exception("Failed to create a sprite.");
        }

        if (FAILED(D3DXCreateTextureFromFile(
            m_pD3DDevice,
            filepath.c_str(),
            &m_pD3DTexture)))
        {
            throw std::exception("Failed to create a texture.");
        }

        D3DSURFACE_DESC desc { };
        if (FAILED(m_pD3DTexture->GetLevelDesc(0, &desc)))
        {
            throw std::exception("Failed to create a texture.");
        }
        m_width = desc.Width;
        m_height = desc.Height;
    }

    ISprite* Create()
    {
        return new Sprite(m_pD3DDevice);
    }

    ~Sprite()
    {
        SAFE_RELEASE(m_D3DSprite);
        SAFE_RELEASE(m_pD3DTexture);
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXSPRITE m_D3DSprite = NULL;
    LPDIRECT3DTEXTURE9 m_pD3DTexture = NULL;
    UINT m_width { 0 };
    UINT m_height { 0 };
};

class Font : public IFont
{
public:

    Font(LPDIRECT3DDEVICE9 pD3DDevice)
        : m_pD3DDevice(pD3DDevice)
    {
    }

    void Init(const bool bEnglish)
    {
        HRESULT hr = S_OK;
        if (!bEnglish)
        {
            hr = D3DXCreateFont(m_pD3DDevice,
                                24,
                                0,
                                FW_NORMAL,
                                1,
                                false,
                                SHIFTJIS_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                CLEARTYPE_NATURAL_QUALITY,
                                FF_DONTCARE,
                                _T("BIZ UDMincho"),
                                &m_pFont);
        }
        else
        {
            hr = D3DXCreateFont(m_pD3DDevice,
                                Common::FontSizeEng(),
                                0,
                                Common::FontThicknessEng(),
                                1,
                                false,
                                DEFAULT_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                CLEARTYPE_NATURAL_QUALITY,
                                FF_DONTCARE,
                                Common::FontNameEng(),
                                &m_pFont);
        }
        assert(hr == S_OK);
    }

    virtual void DrawText_(const std::wstring& msg, const int x, const int y)
    {
        RECT rect = { x, y, 0, 0 };
        m_pFont->DrawText(NULL, msg.c_str(), -1, &rect, DT_LEFT | DT_NOCLIP,
                          D3DCOLOR_ARGB(255, 255, 255, 255));
    }

    ~Font()
    {
        m_pFont->Release();
        m_pFont = nullptr;
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXFONT m_pFont = NULL;
};

class SoundEffect : public ISoundEffect
{
    virtual void PlayMove() override
    {
        ::SoundEffect::get_ton()->play(_T("res\\sound\\cursor_move.wav"));
    }
    virtual void Init() override
    {
        ::SoundEffect::get_ton()->load(_T("res\\sound\\cursor_move.wav"));
    }
};
}

Opening::Opening()
{
    {
        ISoundEffect* pSE = NEW NSSlideShow::SoundEffect();
        pSE->Init();

        ISprite* sprTextBack = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());
        sprTextBack->Load(_T("res\\image\\textBack.png"));

        ISprite* sprFade = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());
        sprFade->Load(_T("res\\image\\black.png"));

        NSSlideShow::IFont* pFont = NEW NSSlideShow::Font(SharedObj::GetD3DDevice());

        std::vector<Page> pageList;
        {
            Page page;
            ISprite* sprite = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());
            if (!SharedObj::IsEnglish())
            {
                sprite->Load(_T("res\\image\\opening01.png"));
            }
            else
            {
                sprite->Load(_T("res\\image\\opening01.en.png"));
            }
            page.SetSprite(sprite);
            std::vector<std::vector<std::wstring> > vvs;
            std::vector<std::wstring> vs;
            vs.push_back(_T(""));
            vvs.push_back(vs);
            page.SetTextList(vvs);
            pageList.push_back(page);
        }
        {
            Page page;
            ISprite* sprite = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());
            sprite->Load(_T("res\\image\\opening02.png"));
            page.SetSprite(sprite);
            std::vector<std::vector<std::wstring> > vvs;
            std::vector<std::wstring> vs;

            if (!SharedObj::IsEnglish())
            {
                vs.push_back(_T("あるところにパワーエッグ星という惑星があった。"));
                vs.push_back(_T("パワーエッグ星には海といくつかの大陸、そして無数の島があった。"));
                vs.push_back(_T("その島の中にプロリタン島という島があった。"));
            }
            else
            {
                vs.push_back(_T("Once upon a time, there was a planet called Poweregg Planet."));
                vs.push_back(_T("Poweregg Planet had oceans, several continents, and countless islands."));
                vs.push_back(_T("Among those islands was one called Prolitan Island."));
            }

            vvs.push_back(vs);
            page.SetTextList(vvs);
            pageList.push_back(page);
        }
        {
            Page page;
            ISprite* sprite = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());
            sprite->Load(_T("res\\image\\opening03.png"));
            page.SetSprite(sprite);
            std::vector<std::vector<std::wstring> > vvs;
            std::vector<std::wstring> vs;

            if (!SharedObj::IsEnglish())
            {
                vs.push_back(_T("プロリタン島の、入り江の木の下で一人の男性が寝ていた。寝ていた男性、ホシマンは浜辺で目を覚ました。"));
                vs.push_back(_T("ホシマンは頭部が星の形をしていた。星の形をしているうえに全身黄色でまさに星という感じであった。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("「し、死ぬ・・・」"));
                vs.push_back(_T("かすれ声だった。"));
                vvs.push_back(vs);
            }
            else
            {
                vs.push_back(_T("Under a tree by the inlet on Prolitan Island, a man lay sleeping. That man, Star Man, woke up on the beach."));
                vs.push_back(_T("His head was shaped like a star. With his entire body yellow as well, he truly looked like a star."));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("\"I... I'm dying...\""));
                vs.push_back(_T("His voice was barely a whisper."));
                vvs.push_back(vs);
            }
            page.SetTextList(vvs);
            pageList.push_back(page);
        }
        {
            Page page;
            ISprite* sprite = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());
            sprite->Load(_T("res\\image\\opening04.png"));
            page.SetSprite(sprite);
            std::vector<std::vector<std::wstring> > vvs;
            std::vector<std::wstring> vs;

            if (!SharedObj::IsEnglish())
            {
                vs.clear();
                vs.push_back(_T("ホシマンはバイクでツーリングを楽しむため船で東のオプティプル島に移動中だった。"));
                vs.push_back(_T("しかし、格安ツアーだったため台風でも強行された。"));
                vs.push_back(_T("そして、元からボロボロだった船は亀裂から浸水が起こり、船が傾きやがて転覆してしまった。"));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("船が転覆したとき、船体は砕け、ホシマンは船の瓦礫に掴まった。そのあと寝ずに３０時間過ごした。"));
                vs.push_back(_T("漂流して３０時間後、島の姿が近くに現れた。"));
                vs.push_back(_T("ホシマンは無我夢中になり、瓦礫をオールにして島まで漕ぎ、島にたどり着くと浜辺に倒れこんだ。そして、そのまま意識を失った。"));
                vvs.push_back(vs);
            }
            else
            {
                vs.clear();
                vs.push_back(_T("Star Man was on a ferry heading east to Optiple Island for a motorcycle tour."));
                vs.push_back(_T("However, because it was a budget tour, the trip went ahead despite a typhoon."));
                vs.push_back(_T("The ship, already in poor condition, began taking on water through cracks, tilted, and eventually capsized."));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("When the ship capsized, the hull broke apart, and Star Man clung to the debris."));
                vs.push_back(_T("He then spent 30 hours without sleep."));
                vs.push_back(_T("After drifting for 30 hours, an island came into view nearby."));
                vvs.push_back(vs);
                vs.clear();
                vs.push_back(_T("Driven by sheer desperation, Star Man used the wreckage as a paddle and rowed to the island."));
                vs.push_back(_T("Upon reaching shore, he collapsed and lost consciousness."));
                vvs.push_back(vs);
            }

            page.SetTextList(vvs);
            pageList.push_back(page);
        }

        m_storyTelling = NEW SlideShow();
        m_storyTelling->Init(pFont, pSE, sprTextBack, sprFade, pageList, SharedObj::IsEnglish());
    }
}

Opening::~Opening()
{
}

void Opening::Update(eBattleState* eState)
{
    if (m_storyTelling != nullptr)
    {
        if (GamePad::IsDownFirst(eGamePadButtonType::A))
        {
            m_storyTelling->Next();
            m_firstPage = false;
        }

        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_SPACE))
        {
            m_storyTelling->Next();
            m_firstPage = false;
        }

        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RETURN))
        {
            m_storyTelling->Next();
            m_firstPage = false;
        }

        if (Mouse::IsDownLeft())
        {
            m_storyTelling->Next();
            m_firstPage = false;
        }

        bool bStoryFinish = m_storyTelling->Update();
        if (bStoryFinish)
        {
            m_storyTelling->Finalize();
            delete m_storyTelling;
            m_storyTelling = nullptr;

            m_bStopBeforeCameraFade = true;
            m_stopBeforeCameraFade = 0;
        }
    }
    else if (m_bStopBeforeCameraFade)
    {
        ++m_stopBeforeCameraFade;
        if (m_stopBeforeCameraFade >= WAIT_MAX)
        {
            m_bStopBeforeCameraFade = false;
            m_bCameraFadeOut = true;
            m_cameraFadeOutCnt = 0;
            Camera::SetCameraMode(eCameraMode::TITLE_TO_BATTLE);
        }
    }
    else
    {
        ++m_cameraFadeOutCnt;
        if (m_cameraFadeOutCnt >= Camera::MOVE_COUNT_MAX)
        {
            *eState = eBattleState::NORMAL;
        }
    }
}

void Opening::Render()
{
    if (m_storyTelling != nullptr)
    {
        m_storyTelling->Render();
    }
}
