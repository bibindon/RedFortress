#pragma once

#include "Sprite.h"
#include <vector>
#include <string>

class PopUp
{
public:
    static void Init(const bool bEnglish);
    static PopUp* Get();
    static void Finalize();
    void SetText(const std::vector<std::vector<std::wstring>>& arg);
    bool IsShow();
    void Next();
    void Cancel();
    void Update();
    void Render();
    ~PopUp();

private:
    static PopUp* m_singleTonObj;

    Sprite* m_sprBack { nullptr };
    LPD3DXFONT m_D3DFont { nullptr };
    std::vector<std::vector<std::wstring>> m_textList;
    int m_counter { 0 };
    bool m_isShow { false };
    int m_currentIndex { 0 };
    const int WAIT_NEXT_FRAME { 60 };
};

