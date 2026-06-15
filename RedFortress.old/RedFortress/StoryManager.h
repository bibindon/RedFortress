#pragma once


#include "Common.h"
#include "Sprite.h"

#include "../../RedFortressSlideShow/SlideShow/SlideShow.h"

class StoryManager
{
public:
    StoryManager(const std::wstring& csvFile);
    ~StoryManager();
    void Update();
    void Render();
    bool IsFinish() const;

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();
private:

    NSSlideShow::SlideShow* m_storyTelling { nullptr };

    bool m_firstPage { true };
    bool m_bPlay { false };
    bool bFinish { false };
};

