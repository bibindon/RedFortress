#pragma once

#include <string>
#include "../../RedFortressCraft/Craft/Craft.h"
#include "Common.h"

class CraftManager
{
public:

    void Init();

    void Finalize();
    void Update();
    void Operate(eBattleState* state);
    void Draw();

    // 画面を作り直す。
    void Build();

private:

    NSCraftLib::CraftLib m_gui;

};

