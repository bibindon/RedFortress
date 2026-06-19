#include "SlideShowManager.h"
#include "../../InputDevice/InputDevice/InputDevice.h"
#include "GameAudio.h"

const float SlideShowManager::kSkipHoldSeconds = 1.0f;

static const std::wstring kTextBackPath = L"res\\2D_Image\\textBack.png";
static const std::wstring kFadeImagePath = L"res\\2D_Image\\black2x2.bmp";

//-----------------------------------------------------------------------------
// SpriteAdapter
//-----------------------------------------------------------------------------

SlideShowManager::SpriteAdapter::SpriteAdapter(NSRender::Render& render)
    : m_render(render)
{
}

void SlideShowManager::SpriteAdapter::DrawImage(const int x, const int y, const int transparency)
{
    (void)x;
    (void)y;
    if (m_filepath.empty())
    {
        return;
    }

    if (m_filepath.find(L"novel_chr_") != std::wstring::npos)
    {
        m_render.DrawImageAutoResize(m_filepath, 0.78f, 0.48f, transparency);
    }
    else
    {
        m_render.DrawImageStretched(m_filepath, transparency);
    }
}

void SlideShowManager::SpriteAdapter::DrawImageEx(const int x,
                                                   const int y,
                                                   const int transparency,
                                                   const bool flipX,
                                                   const float scale)
{
    if (m_filepath.empty())
    {
        return;
    }

    if (m_filepath.find(L"novel_chr_") != std::wstring::npos)
    {
        m_render.DrawImageAutoResizeEx(m_filepath,
                                       static_cast<float>(x) / static_cast<float>(NSRender::Common::BASE_W),
                                       static_cast<float>(y) / static_cast<float>(NSRender::Common::BASE_H),
                                       scale,
                                       flipX,
                                       transparency);
    }
    else
    {
        m_render.DrawImageStretchedScaled(m_filepath, scale, transparency);
    }
}

void SlideShowManager::SpriteAdapter::Load(const std::wstring& filepath)
{
    m_filepath = filepath;
    m_render.LoadImage(m_filepath);
}

void SlideShowManager::SpriteAdapter::GetImageSize(int& width, int& height) const
{
    if (m_filepath.empty())
    {
        width = 0;
        height = 0;
        return;
    }

    const SIZE size = m_render.GetImageSize(m_filepath);
    width = size.cx;
    height = size.cy;
}

NSSlideShow::ISprite* SlideShowManager::SpriteAdapter::Create()
{
    SpriteAdapter* sprite = new SpriteAdapter(m_render);
    sprite->m_filepath = m_filepath;
    return sprite;
}

void SlideShowManager::SpriteAdapter::OnDeviceLost() {}
void SlideShowManager::SpriteAdapter::OnDeviceReset() {}

//-----------------------------------------------------------------------------
// FontAdapter
//-----------------------------------------------------------------------------

SlideShowManager::FontAdapter::FontAdapter(NSRender::Render& render, int& fontIdRef)
    : m_render(render)
    , m_fontIdRef(fontIdRef)
{
}

void SlideShowManager::FontAdapter::DrawText_(const std::wstring& msg, const int x, const int y)
{
    if (m_fontIdRef >= 0)
    {
        m_render.DrawTextEx(m_fontIdRef, msg, x, y, D3DCOLOR_RGBA(255, 255, 255, 255));
    }
}

void SlideShowManager::FontAdapter::Init(const bool bEnglish)
{
    (void)bEnglish;
    m_fontIdRef = m_render.SetUpFontEx(L"BIZ UDGothic", 22, D3DCOLOR_RGBA(255, 255, 255, 255));
}

void SlideShowManager::FontAdapter::OnDeviceLost() {}
void SlideShowManager::FontAdapter::OnDeviceReset() {}

//-----------------------------------------------------------------------------
// SoundAdapter
//-----------------------------------------------------------------------------

void SlideShowManager::SoundAdapter::PlayMove()
{
    GameAudio::PlayMenuMove();
}

void SlideShowManager::SoundAdapter::Init() {}

//-----------------------------------------------------------------------------
// SlideShowManager
//-----------------------------------------------------------------------------

SlideShowManager::SlideShowManager(NSRender::Render& render)
    : m_render(render)
    , m_slideShow(nullptr)
    , m_fontId(-1)
    , m_skipHintFontId(-1)
    , m_skipRequested(false)
    , m_stopOnFinish(false)
{
}

void SlideShowManager::Start(const std::wstring& csvPath)
{
    Finalize();
    m_skipRequested = false;

    FontAdapter* font = new FontAdapter(m_render, m_fontId);
    SoundAdapter* se = new SoundAdapter();
    SpriteAdapter* sprTextBack = new SpriteAdapter(m_render);
    sprTextBack->Load(kTextBackPath);
    SpriteAdapter* sprFade = new SpriteAdapter(m_render);
    sprFade->Load(kFadeImagePath);
    SpriteAdapter* sprImage = new SpriteAdapter(m_render);

    m_slideShow = new NSSlideShow::SlideShow();
    m_slideShow->SetScreenSize(NSRender::Common::BASE_W, NSRender::Common::BASE_H);
    m_slideShow->Init(font, se, sprTextBack, sprFade, csvPath, sprImage, false, false);
}

void SlideShowManager::Next()
{
    if (m_slideShow != nullptr)
    {
        m_slideShow->Next();
    }
}

void SlideShowManager::Skip()
{
    if (m_slideShow != nullptr)
    {
        m_slideShow->Skip();
    }
}

bool SlideShowManager::Update()
{
    if (m_slideShow == nullptr)
    {
        return false;
    }

    if (m_slideShow->Update())
    {
        if (m_stopOnFinish)
        {
            return false;
        }

        Finalize();
        return true;
    }

    return false;
}

void SlideShowManager::Render()
{
    if (m_slideShow != nullptr)
    {
        m_slideShow->Render();
    }
}

void SlideShowManager::Finalize()
{
    if (m_slideShow != nullptr)
    {
        m_slideShow->Finalize();
        delete m_slideShow;
        m_slideShow = nullptr;
        m_fontId = -1;
        m_skipRequested = false;
    }
}

bool SlideShowManager::IsActive() const
{
    return m_slideShow != nullptr;
}

void SlideShowManager::SetStopOnFinish(const bool stop)
{
    m_stopOnFinish = stop;
}

void SlideShowManager::ProcessInput()
{
    if (m_slideShow == nullptr)
    {
        return;
    }

    if (InputDevice::SKeyBoard::IsDownFirstFrame(DIK_RETURN) ||
        InputDevice::SKeyBoard::IsDownFirstFrame(DIK_SPACE) ||
        InputDevice::Mouse::IsDownFirstFrame(InputDevice::MOUSE_LEFT))
    {
        m_slideShow->Next();
    }

    if (InputDevice::SKeyBoard::IsHoldDuration(DIK_SPACE, kSkipHoldSeconds))
    {
        if (!m_skipRequested)
        {
            m_slideShow->Skip();
            m_skipRequested = true;
        }
    }
    else
    {
        m_skipRequested = false;
    }
}

void SlideShowManager::DrawSkipHint()
{
    if (m_slideShow == nullptr)
    {
        return;
    }

    if (m_skipHintFontId < 0)
    {
        m_skipHintFontId = m_render.SetUpFont(L"BIZ UDGothic", 18, D3DCOLOR_RGBA(255, 255, 255, 255));
    }

    m_render.DrawTextCenter(m_skipHintFontId,
                            L"Space長押しでスキップ",
                            1190,
                            820,
                            360,
                            40,
                            D3DCOLOR_RGBA(255, 255, 255, 190));
}
