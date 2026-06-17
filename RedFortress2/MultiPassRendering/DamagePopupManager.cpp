#include "DamagePopupManager.h"

#include "../../RedFortressRender/Render/Render.h"

const D3DXCOLOR DamagePopupManager::kDamageColor = D3DXCOLOR(1.0f, 0.2f, 0.2f, 1.0f);
const D3DXCOLOR DamagePopupManager::kHealColor = D3DXCOLOR(0.2f, 1.0f, 0.2f, 1.0f);

DamagePopupManager::DamagePopupManager()
{
}

void DamagePopupManager::Initialize(NSRender::Render* pRender)
{
    m_pRender = pRender;
}

void DamagePopupManager::Add(int amount, const D3DXVECTOR3& pos, bool isHeal)
{
    Popup popup;
    popup.text = std::to_wstring(amount);
    popup.basePos = pos + D3DXVECTOR3(0.0f, 1.5f, 0.0f);
    popup.riseOffset = 0.0f;
    popup.remainingFrames = kDuration;
    popup.fontSize = 24;
    if (isHeal)
    {
        popup.color = kHealColor;
    }
    else
    {
        popup.color = kDamageColor;
    }
    m_popups.push_back(popup);
}

void DamagePopupManager::Update()
{
    for (auto& popup : m_popups)
    {
        popup.riseOffset += kRiseSpeed;
        popup.remainingFrames -= 1;
    }

    m_popups.erase(
        std::remove_if(m_popups.begin(),
                       m_popups.end(),
                       [](const Popup& popup) { return popup.remainingFrames <= 0; }),
        m_popups.end());
}

void DamagePopupManager::Draw()
{
    if (m_pRender == nullptr)
    {
        return;
    }

    for (const auto& popup : m_popups)
    {
        const float alphaRate = static_cast<float>(popup.remainingFrames) / static_cast<float>(kDuration);
        D3DXCOLOR color = popup.color;
        color.a *= alphaRate;
        const D3DXVECTOR3 drawPos = popup.basePos + D3DXVECTOR3(0.0f, popup.riseOffset, 0.0f);
        m_pRender->DrawWorldText(popup.text, drawPos, popup.fontSize, color, true);
    }
}

void DamagePopupManager::Clear()
{
    m_popups.clear();
}
