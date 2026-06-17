#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class DamagePopupManager
{
public:
    DamagePopupManager();

    void Initialize(NSRender::Render* pRender);
    void Add(int amount, const D3DXVECTOR3& pos, bool isHeal);
    void Update();
    void Draw();
    void Clear();

private:
    struct Popup
    {
        std::wstring text;
        D3DXVECTOR3 basePos;
        float riseOffset = 0.0f;
        int remainingFrames = 0;
        int fontSize = 24;
        D3DXCOLOR color;
    };

    NSRender::Render* m_pRender = nullptr;
    std::vector<Popup> m_popups;

    static constexpr int kDuration = 60;
    static constexpr float kRiseSpeed = 0.01f;
    static const D3DXCOLOR kDamageColor;
    static const D3DXCOLOR kHealColor;
};
