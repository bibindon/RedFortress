#include "StoryManager.h"
#include "KeyBoard.h"
#include "Mouse.h"
#include "GamePad.h"
#include "Common.h"
#include "SharedObj.h"
#include "BGM.h"
#include "SoundEffect.h"
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
        RECT rect = { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };
        D3DXVECTOR3 center { 0, 0, 0 };
        m_D3DSprite->Draw(m_pD3DTexture,
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

        if (FAILED(D3DXCreateTextureFromFile(m_pD3DDevice,
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
        if (m_D3DSprite != nullptr)
        {
            m_D3DSprite->Release();
        }

        if (m_pD3DTexture != nullptr)
        {
            m_pD3DTexture->Release();
        }
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

StoryManager::StoryManager(const std::wstring& csvFile)
{
    {
        NSSlideShow::ISoundEffect* pSE = NEW NSSlideShow::SoundEffect();
        pSE->Init();

        NSSlideShow::ISprite* sprTextBack = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());
        sprTextBack->Load(_T("res\\image\\textBack.png"));

        NSSlideShow::ISprite* sprFade = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());
        sprFade->Load(_T("res\\image\\black.png"));

        NSSlideShow::IFont* pFont = NEW NSSlideShow::Font(SharedObj::GetD3DDevice());

        NSSlideShow::ISprite* sprite = NEW NSSlideShow::Sprite(SharedObj::GetD3DDevice());

        m_storyTelling = NEW SlideShow();
        m_storyTelling->Init(pFont,
                             pSE,
                             sprTextBack,
                             sprFade, 
                             Common::ModExt(csvFile),
                             sprite,
                             Common::EncryptMode(),
                             SharedObj::IsEnglish());

        bFinish = false;
    }
}

StoryManager::~StoryManager()
{
}

void StoryManager::Update()
{
    if (m_storyTelling != nullptr)
    {
        if (GamePad::IsDownFirst(eGamePadButtonType::A))
        {
            m_storyTelling->Next();
            m_firstPage = false;
        }

        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RETURN))
        {
            m_storyTelling->Next();
            m_firstPage = false;
        }

        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_SPACE))
        {
            m_storyTelling->Next();
            m_firstPage = false;
        }

        if (Mouse::IsDownLeft())
        {
            m_storyTelling->Next();
            m_firstPage = false;
        }

        if (m_firstPage == false && m_bPlay == false)
        {
            //BGM::get_ton()->play();
            m_bPlay = true;
        }

        bFinish = m_storyTelling->Update();
        if (bFinish)
        {
            m_storyTelling->Finalize();
            delete m_storyTelling;
            m_storyTelling = nullptr;
        }
    }
}

void StoryManager::Render()
{
    if (m_storyTelling != nullptr)
    {
        m_storyTelling->Render();
    }
}

bool StoryManager::IsFinish() const
{
    return bFinish;
}

// 解像度やウィンドウモードを変更したときのための関数
void StoryManager::OnDeviceLost()
{

}

// 解像度やウィンドウモードを変更したときのための関数
void StoryManager::OnDeviceReset()
{

}

