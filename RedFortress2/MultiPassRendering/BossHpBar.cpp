#include "BossHpBar.h"

#include "EnemyBase.h"
#include "../../RedFortressRender/Render/Render.h"

const std::wstring BossHpBar::kBackGaussImagePath = L"res\\2D_Image\\hp_back_gauss.png";
const std::wstring BossHpBar::kBackImagePath = L"res\\2D_Image\\hp_back.png";
const std::wstring BossHpBar::kFrontImagePath = L"res\\2D_Image\\hp_front.png";
const std::wstring BossHpBar::kDamageImagePath = L"res\\2D_Image\\hp_damage.png";

BossHpBar::BossHpBar()
{
}

void BossHpBar::Initialize(NSRender::Render* pRender)
{
    m_pRender = pRender;
    if (m_pRender != nullptr && m_fontId < 0)
    {
        m_fontId = m_pRender->SetUpFontEx(L"BIZ UDGothic",
                                          kNameFontSize,
                                          D3DCOLOR_RGBA(255, 255, 255, 255));
    }
}

void BossHpBar::SetBoss(EnemyBase* pBoss)
{
    if (m_pBoss == pBoss)
    {
        return;
    }

    m_pBoss = pBoss;
    if (m_pBoss == nullptr)
    {
        m_lastObservedHp = -1;
        return;
    }

    ResetDisplay(m_pBoss->GetHp(), m_pBoss->GetMaxHp());
    m_lastObservedHp = m_pBoss->GetHp();
}

void BossHpBar::ResetDisplay(int hp, int maxHp)
{
    const float hpValue = static_cast<float>(hp);
    m_frontDisplay = hpValue;
    m_damageDisplay = hpValue;
    m_frontStart = hpValue;
    m_frontTarget = hpValue;
    m_frontAnimFrame = 0.0f;
    m_frontAnimating = false;
    m_damageStart = hpValue;
    m_damageTarget = hpValue;
    m_damageDelayFrame = 0.0f;
    m_damageAnimFrame = 0.0f;
    m_damageWaiting = false;
    m_damageAnimating = false;
    m_damageFollowFrontAfterHeal = false;
}

void BossHpBar::OnDamage(int oldHp, int newHp)
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

void BossHpBar::Update()
{
    if (m_pBoss == nullptr)
    {
        return;
    }

    // 毎フレーム HP を監視し、減少を検出したらダメージアニメーションを駆動。
    // これにより被弾処理側の修正なしでバーが反応する。
    const int currentHp = m_pBoss->GetHp();
    if (m_lastObservedHp >= 0 && currentHp < m_lastObservedHp)
    {
        OnDamage(m_lastObservedHp, currentHp);
    }
    m_lastObservedHp = currentHp;

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

void BossHpBar::Draw()
{
    if (m_pRender == nullptr || m_pBoss == nullptr)
    {
        return;
    }

    const int maxHp = m_pBoss->GetMaxHp();
    if (maxHp <= 0)
    {
        return;
    }

    // 描画仕様: DrawImageAutoResizeSizedRect は (sourceWidth * scale) が
    // 描画ピクセル幅になる。バー全体の長さは kBossBarScale で、
    // HP 割合による短縮は sourceWidth 側で制御する（左詰めで短くなる）。
    const float scale = kBossBarScale;
    const int damageSourceW = CalcBarWidth(m_damageDisplay, kSourceWidth, maxHp);
    const int frontSourceW = CalcBarWidth(m_frontDisplay, kSourceWidth, maxHp);

    // バー全体の正規化描画幅から、中央寄せの左上 X を求める。
    const float normalizedFullWidth =
        static_cast<float>(kSourceWidth) * scale / static_cast<float>(NSRender::Common::BASE_W);
    const float posX = (1.0f - normalizedFullWidth) * 0.5f;
    const float posY = kPosY;

    // グロー（背景）は本体より一回り大きいので位置を少し上にずらす。
    const float gaussNormalizedFullWidth =
        static_cast<float>(kBackGaussSourceWidth) * scale / static_cast<float>(NSRender::Common::BASE_W);
    const float gaussPosX = (1.0f - gaussNormalizedFullWidth) * 0.5f;
    const float gaussPosY = posY - 0.012f;

    m_pRender->DrawImageAutoResizeSizedRect(kBackGaussImagePath,
                                            gaussPosX,
                                            gaussPosY,
                                            0,
                                            0,
                                            kBackGaussSourceWidth,
                                            kBackGaussSourceHeight,
                                            scale,
                                            255);
    m_pRender->DrawImageAutoResizeSizedRect(kBackImagePath, posX, posY, 0, 0, kSourceWidth, kSourceHeight, scale, 255);
    m_pRender->DrawImageAutoResizeSizedRect(kDamageImagePath, posX, posY, 0, 0, damageSourceW, kSourceHeight, scale, 255);
    m_pRender->DrawImageAutoResizeSizedRect(kFrontImagePath, posX, posY, 0, 0, frontSourceW, kSourceHeight, scale, 255);

    // ボス名をバー上部に中央寄せで描画。
    if (m_fontId >= 0)
    {
        const std::wstring name = m_pBoss->GetBossName();
        if (!name.empty())
        {
            const int textHeightBase = static_cast<int>(kNameFontSize * 1.6f);
            const int textY = static_cast<int>(posY * NSRender::Common::BASE_H) - textHeightBase - 2;
            m_pRender->DrawTextExCenter(m_fontId,
                                        name,
                                        0,
                                        textY,
                                        NSRender::Common::BASE_W,
                                        textHeightBase,
                                        D3DCOLOR_RGBA(255, 255, 255, 255));
        }
    }
}

float BossHpBar::Clamp(float value, float minValue, float maxValue)
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

int BossHpBar::CalcBarWidth(float hpValue, int imageWidth, int maxHp)
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
