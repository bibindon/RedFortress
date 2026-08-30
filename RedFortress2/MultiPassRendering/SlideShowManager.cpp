#include "SlideShowManager.h"
#include <algorithm>

#include "../../InputDevice/InputDevice/InputDevice.h"
#include "GameAudio.h"

const float SlideShowManager::kSkipHoldSeconds = 1.0f;

static const std::wstring kTextBackPath = L"res\\2D_Image\\textBack.png";
static const std::wstring kFadeImagePath = L"res\\2D_Image\\black2x2.bmp";

namespace
{
// マリンの立ち絵は1024px幅のうち右側約410pxが透明なので、
// 画像全体ではなく人物部分が右側に配置されるよう余白を相殺する。
constexpr float kMarinePortraitOffsetX = 300.0f;

bool IsMarinePortrait(const std::wstring& filepath)
{
    return filepath.find(L"novel_chr_marine_") != std::wstring::npos;
}

struct SlideShowCanvasRect
{
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

SlideShowCanvasRect ConvertSlideShowCanvasRect(const float x,
                                               const float y,
                                               const float width,
                                               const float height)
{
    SlideShowCanvasRect result;
    if (NSRender::Common::ScreenW() <= 0 ||
        NSRender::Common::ScreenH() <= 0 ||
        width <= 0.0f ||
        height <= 0.0f)
    {
        return result;
    }

    const float screenScaleX =
        static_cast<float>(NSRender::Common::ScreenW()) /
        static_cast<float>(NSRender::Common::BASE_W);
    const float screenScaleY =
        static_cast<float>(NSRender::Common::ScreenH()) /
        static_cast<float>(NSRender::Common::BASE_H);
    const float uniformScale = (std::min)(screenScaleX, screenScaleY);
    const float canvasScreenWidth =
        static_cast<float>(NSRender::Common::BASE_W) * uniformScale;
    const float canvasScreenHeight =
        static_cast<float>(NSRender::Common::BASE_H) * uniformScale;
    const float offsetScreenX =
        (static_cast<float>(NSRender::Common::ScreenW()) - canvasScreenWidth) * 0.5f;
    const float offsetScreenY =
        (static_cast<float>(NSRender::Common::ScreenH()) - canvasScreenHeight) * 0.5f;

    result.x = static_cast<int>((offsetScreenX + (x * uniformScale)) / screenScaleX);
    result.y = static_cast<int>((offsetScreenY + (y * uniformScale)) / screenScaleY);
    result.width = static_cast<int>((width * uniformScale) / screenScaleX);
    result.height = static_cast<int>((height * uniformScale) / screenScaleY);
    if (result.width <= 0)
    {
        result.width = 1;
    }
    if (result.height <= 0)
    {
        result.height = 1;
    }
    return result;
}

POINT ConvertSlideShowCanvasPoint(const int x, const int y)
{
    const SlideShowCanvasRect converted =
        ConvertSlideShowCanvasRect(static_cast<float>(x),
                                   static_cast<float>(y),
                                   1.0f,
                                   1.0f);
    POINT result;
    result.x = converted.x;
    result.y = converted.y;
    return result;
}

void DrawSlideShowCanvasImage(NSRender::Render& render,
                              const std::wstring& filepath,
                              const float x,
                              const float y,
                              const float width,
                              const float height,
                              const int transparency,
                              const bool flipX)
{
    const SlideShowCanvasRect destination =
        ConvertSlideShowCanvasRect(x, y, width, height);
    render.DrawImageSizedEx(filepath,
                            destination.x,
                            destination.y,
                            destination.width,
                            destination.height,
                            transparency,
                            flipX);
}
}

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

    if (m_filepath.find(L"_chr_") != std::wstring::npos)
    {
        const SIZE imageSize = m_render.GetImageSize(m_filepath);
        if (imageSize.cx <= 0 || imageSize.cy <= 0)
        {
            return;
        }

        const float drawWidth = static_cast<float>(imageSize.cx);
        const float drawHeight = static_cast<float>(imageSize.cy);
        const float centerX = 0.78f * static_cast<float>(NSRender::Common::BASE_W);
        const float centerY = 0.48f * static_cast<float>(NSRender::Common::BASE_H);
        DrawSlideShowCanvasImage(m_render,
                                 m_filepath,
                                 centerX - drawWidth * 0.5f,
                                 centerY - drawHeight * 0.5f,
                                 drawWidth,
                                 drawHeight,
                                 transparency,
                                 false);
    }
    else
    {
        DrawSlideShowCanvasImage(m_render,
                                 m_filepath,
                                 0.0f,
                                 0.0f,
                                 static_cast<float>(NSRender::Common::BASE_W),
                                 static_cast<float>(NSRender::Common::BASE_H),
                                 transparency,
                                 false);
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

    if (m_filepath.find(L"_chr_") != std::wstring::npos)
    {
        const SIZE imageSize = m_render.GetImageSize(m_filepath);
        if (imageSize.cx <= 0 || imageSize.cy <= 0)
        {
            return;
        }

        float centerX = static_cast<float>(x);
        if (IsMarinePortrait(m_filepath))
        {
            centerX += kMarinePortraitOffsetX;
        }

        const float drawWidth = static_cast<float>(imageSize.cx) * scale;
        const float drawHeight = static_cast<float>(imageSize.cy) * scale;
        DrawSlideShowCanvasImage(m_render,
                                 m_filepath,
                                 centerX - drawWidth * 0.5f,
                                 static_cast<float>(y) - drawHeight * 0.5f,
                                 drawWidth,
                                 drawHeight,
                                 transparency,
                                 flipX);
    }
    else
    {
        const SIZE imageSize = m_render.GetImageSize(m_filepath);
        if (imageSize.cx <= 0 || imageSize.cy <= 0)
        {
            return;
        }
        DrawSlideShowCanvasImage(m_render,
                                 m_filepath,
                                 0.0f,
                                 0.0f,
                                 static_cast<float>(imageSize.cx) * scale,
                                 static_cast<float>(imageSize.cy) * scale,
                                 transparency,
                                 false);
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
        const POINT position = ConvertSlideShowCanvasPoint(x, y);
        m_render.DrawTextEx(m_fontIdRef,
                            msg,
                            position.x,
                            position.y,
                            D3DCOLOR_RGBA(255, 255, 255, 255));
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
        m_render.DrawImageStretched(kFadeImagePath, 255);
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

    const SlideShowCanvasRect hintRect =
        ConvertSlideShowCanvasRect(1190.0f, 820.0f, 360.0f, 40.0f);
    m_render.DrawTextCenter(m_skipHintFontId,
                            L"Space長押しでスキップ",
                            hintRect.x,
                            hintRect.y,
                            hintRect.width,
                            hintRect.height,
                            D3DCOLOR_RGBA(255, 255, 255, 190));
}
