#pragma once

#include "../../RedFortressHUD/HUD/HUD.h"

class HudManager
{
public:
    void Init();
    void Finalize();
    void Update();
    void Draw();
private:
    NSHUD::HUD* m_hud = nullptr;
};

