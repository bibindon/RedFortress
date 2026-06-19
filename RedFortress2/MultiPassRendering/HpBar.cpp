#include "HpBar.h"

#include "Player.h"
#include "../../RedFortressRender/Render/Render.h"

const std::wstring HpBar::kBackGaussImagePath = L"res\\2D_Image\\hp_back_gauss.png";
const std::wstring HpBar::kBackImagePath = L"res\\2D_Image\\hp_back.png";
const std::wstring HpBar::kFrontImagePath = L"res\\2D_Image\\hp_front.png";
const std::wstring HpBar::kDamageImagePath = L"res\\2D_Image\\hp_damage.png";

HpBar::HpBar()
{
}

void HpBar::Initialize(NSRender::Render* pRender, Player* pPlayer)
{
    m_pRender = pRender;
    m_pPlayer = pPlayer;
    Reset();
}

void HpBar::Reset()
{
    if (m_pPlayer == nullptr)
    {
        return;
    }

    const float hp = static_cast<float>(m_pPlayer->GetHp());
    m_frontDisplay = hp;
    m_damageDisplay = hp;
    m_frontStart = hp;
    m_frontTarget = hp;
    m_frontAnimFrame = 0.0f;
    m_frontAnimating = false;
    m_damageStart = hp;
    m_damageTarget = hp;
    m_damageDelayFrame = 0.0f;
    m_damageAnimFrame = 0.0f;
    m_damageWaiting = false;
    m_damageAnimating = false;
    m_damageFollowFrontAfterHeal = false;
}

void HpBar::OnDamage(int oldHp, int newHp)
{
    const float oldHpValue = static_cast<float>(oldHp);
    const float newHpValue = static_cast<float>(newHp);

    m_frontDisplay = newHpValue;
    m_frontStart = newHpValue;
    m_frontTarget = newHpValue;
    m_frontAnimFrame = 0.0f;
    m_frontAnimating = false;

    m_damageStart = m_damageDisplay;
    if (m_damageStart < oldHpValue)
    {
        m_damageStart = oldHpValue;
    }
    m_damageTarget = newHpValue;
    m_damageDelayFrame = 0.0f;
    m_damageAnimFrame = 0.0f;
    m_damageWaiting = true;
    m_damageAnimating = false;
    m_damageFollowFrontAfterHeal = false;
}

void HpBar::OnHeal(int oldHp, int newHp)
{
    const float newHpValue = static_cast<float>(newHp);

    m_frontStart = m_frontDisplay;
    m_frontTarget = newHpValue;
    m_frontAnimFrame = 0.0f;
    m_frontAnimating = true;

    m_damageWaiting = false;
    m_damageAnimating = false;
    m_damageFollowFrontAfterHeal = true;
    m_damageDelayFrame = 0.0f;
    m_damageAnimFrame = 0.0f;
}

void HpBar::Update()
{
    if (m_frontAnimating)
    {
        m_frontAnimFrame += 1.0f;
        const float rate = Clamp(m_frontAnimFrame / kAnimFrameMax, 0.0f, 1.0f);
        m_frontDisplay = m_frontStart + ((m_frontTarget - m_frontStart) * rate);
        if (rate >= 1.0f)
        {
            m_frontDisplay = m_frontTarget;
            m_frontAnimating = false;
            if (m_damageFollowFrontAfterHeal)
            {
                if (m_damageDisplay < m_frontTarget)
                {
                    m_damageDisplay = m_frontTarget;
                }
                m_damageStart = m_damageDisplay;
                m_damageTarget = m_damageDisplay;
                m_damageFollowFrontAfterHeal = false;
            }
        }
    }

    if (m_damageWaiting)
    {
        m_damageDelayFrame += 1.0f;
        if (m_damageDelayFrame >= kDamageDelayFrameMax)
        {
            m_damageWaiting = false;
            m_damageAnimating = true;
            m_damageAnimFrame = 0.0f;
            m_damageStart = m_damageDisplay;
        }
    }

    if (m_damageAnimating)
    {
        m_damageAnimFrame += 1.0f;
        const float rate = Clamp(m_damageAnimFrame / kAnimFrameMax, 0.0f, 1.0f);
        m_damageDisplay = m_damageStart + ((m_damageTarget - m_damageStart) * rate);
        if (rate >= 1.0f)
        {
            m_damageDisplay = m_damageTarget;
            m_damageAnimating = false;
        }
    }
}

void HpBar::Draw()
{
    Update();

    if (m_pRender == nullptr || m_pPlayer == nullptr)
    {
        return;
    }

    const int maxHp = m_pPlayer->GetMaxHp();
    if (maxHp <= 0)
    {
        return;
    }

    const float scale = static_cast<float>(NSRender::Common::BASE_W) / kScaleBase;
    const int fullBarWidth = static_cast<int>(static_cast<float>(kSourceWidth) * scale + 0.5f);
    const int damageWidth = CalcBarWidth(m_damageDisplay, fullBarWidth, maxHp);
    const int frontWidth = CalcBarWidth(m_frontDisplay, fullBarWidth, maxHp);
    const int damageSourceW = static_cast<int>(static_cast<float>(damageWidth) / scale + 0.5f);
    const int frontSourceW = static_cast<int>(static_cast<float>(frontWidth) / scale + 0.5f);

    m_pRender->DrawImageAutoResizeSizedRect(kBackGaussImagePath,
                                            kBackGaussPosX,
                                            kBackGaussPosY,
                                            0,
                                            0,
                                            kBackGaussSourceWidth,
                                            kBackGaussSourceHeight,
                                            scale,
                                            255);
    m_pRender->DrawImageAutoResizeSizedRect(kBackImagePath, kPosX, kPosY, 0, 0, kSourceWidth, kSourceHeight, scale, 255);
    m_pRender->DrawImageAutoResizeSizedRect(kDamageImagePath, kPosX, kPosY, 0, 0, damageSourceW, kSourceHeight, scale, 255);
    m_pRender->DrawImageAutoResizeSizedRect(kFrontImagePath, kPosX, kPosY, 0, 0, frontSourceW, kSourceHeight, scale, 255);
}

float HpBar::Clamp(float value, float minValue, float maxValue)
{
    if (value < minValue)
    {
        return minValue;
    }

    if (maxValue < value)
    {
        return maxValue;
    }

    return value;
}

int HpBar::CalcBarWidth(float hpValue, int imageWidth, int maxHp)
{
    if (maxHp <= 0)
    {
        return 0;
    }

    const float rate = Clamp(hpValue / static_cast<float>(maxHp), 0.0f, 1.0f);
    int width = static_cast<int>(static_cast<float>(imageWidth) * rate);
    if (width < 1 && hpValue > 0.0f)
    {
        width = 1;
    }

    return width;
}
