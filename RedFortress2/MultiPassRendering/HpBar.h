#pragma once

#include <string>

namespace NSRender
{
class Render;
}

class Player;

class HpBar
{
public:
    HpBar();

    void Initialize(NSRender::Render* pRender, Player* pPlayer);
    void Reset();
    void OnDamage(int oldHp, int newHp);
    void OnHeal(int oldHp, int newHp);
    void Update();
    void Draw();

private:
    static float Clamp(float value, float minValue, float maxValue);
    static int CalcBarWidth(float hpValue, int imageWidth, int maxHp);

    NSRender::Render* m_pRender = nullptr;
    Player* m_pPlayer = nullptr;

    float m_frontDisplay = 100.0f;
    float m_damageDisplay = 100.0f;
    float m_frontStart = 100.0f;
    float m_frontTarget = 100.0f;
    float m_frontAnimFrame = 0.0f;
    bool m_frontAnimating = false;
    float m_damageStart = 100.0f;
    float m_damageTarget = 100.0f;
    float m_damageDelayFrame = 0.0f;
    float m_damageAnimFrame = 0.0f;
    bool m_damageWaiting = false;
    bool m_damageAnimating = false;
    bool m_damageFollowFrontAfterHeal = false;

    static const std::wstring kBackImagePath;
    static const std::wstring kFrontImagePath;
    static const std::wstring kDamageImagePath;
    static constexpr float kPosX = 0.03f;
    static constexpr float kPosY = 0.05f;
    static constexpr int kSourceWidth = 1000;
    static constexpr int kSourceHeight = 16;
    static constexpr float kScaleBase = 1920.0f;
    static constexpr float kAnimFrameMax = 30.0f;
    static constexpr float kDamageDelayFrameMax = 30.0f;
};
