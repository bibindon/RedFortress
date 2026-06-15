#pragma once

#include <string>
#include "../../RedFortressSlideShow/SlideShow/SlideShow.h"
#include "../../RedFortressRender/Render/Render.h"

class SlideShowManager
{
public:
    explicit SlideShowManager(NSRender::Render& render);

    void Start(const std::wstring& csvPath);
    void Next();
    void Skip();
    bool Update();
    void Render();
    void Finalize();
    bool IsActive() const;
    void SetStopOnFinish(const bool stop);
    void ProcessInput();
    void DrawSkipHint();

private:
    NSRender::Render& m_render;
    NSSlideShow::SlideShow* m_slideShow;
    int m_fontId;
    int m_skipHintFontId;
    bool m_skipRequested;
    bool m_stopOnFinish;
    static const float kSkipHoldSeconds;

    class SpriteAdapter : public NSSlideShow::ISprite
    {
    public:
        explicit SpriteAdapter(NSRender::Render& render);

        void DrawImage(const int x, const int y, const int transparency) override;
        void DrawImageEx(const int x, const int y, const int transparency, const bool flipX, const float scale) override;
        void Load(const std::wstring& filepath) override;
        void GetImageSize(int& width, int& height) const override;
        NSSlideShow::ISprite* Create() override;
        void OnDeviceLost() override;
        void OnDeviceReset() override;

    private:
        NSRender::Render& m_render;
        std::wstring m_filepath;
    };

    class FontAdapter : public NSSlideShow::IFont
    {
    public:
        FontAdapter(NSRender::Render& render, int& fontIdRef);

        void DrawText_(const std::wstring& msg, const int x, const int y) override;
        void Init(const bool bEnglish) override;
        void OnDeviceLost() override;
        void OnDeviceReset() override;

    private:
        NSRender::Render& m_render;
        int& m_fontIdRef;
    };

    class SoundAdapter : public NSSlideShow::ISoundEffect
    {
    public:
        void PlayMove() override;
        void Init() override;
    };
};
