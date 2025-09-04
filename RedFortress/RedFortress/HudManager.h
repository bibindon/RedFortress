#pragma once

#include "../../RedFortressHUD/HUD/HUD.h"

class HudManager
{
public:
    void Init();
    void Finalize();
    void Update();
    void Draw();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:
    NSHUD::HUD* m_hud = nullptr;
};

