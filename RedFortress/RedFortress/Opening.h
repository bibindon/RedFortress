#pragma once

#include "Common.h"
#include "Sprite.h"

#include "../../RedFortressSlideShow/SlideShow/SlideShow.h"

class Opening
{
public:
    Opening();
    ~Opening();
    void Update(eBattleState* eState);
    void Render();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:

    NSSlideShow::SlideShow* m_storyTelling { nullptr };

    bool m_firstPage = true;
    bool m_bPlay = false;

    // カメラ移動前に2秒待つ
    bool m_bStopBeforeCameraFade = false;
    int m_stopBeforeCameraFade = 0;
    const int WAIT_MAX = 120;

    bool m_bCameraFadeOut = false;
    int m_cameraFadeOutCnt = 0;

};

