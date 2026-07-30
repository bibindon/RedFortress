#include "EnemySpider.h"

#include "../../RedFortressRender/Render/Render.h"

namespace
{
    const int kAttackCooldownFrames = 38;

    float HorizontalDistance(const D3DXVECTOR3& a, const D3DXVECTOR3& b)
    {
        const float x = a.x - b.x;
        const float z = a.z - b.z;
        return sqrtf(x * x + z * z);
    }

    D3DXVECTOR3 HorizontalDirection(const D3DXVECTOR3& from, const D3DXVECTOR3& to)
    {
        D3DXVECTOR3 direction = to - from;
        direction.y = 0.0f;
        if (D3DXVec3LengthSq(&direction) > 0.0001f)
        {
            D3DXVec3Normalize(&direction, &direction);
        }
        return direction;
    }
}

EnemySpider::EnemySpider(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"spider",
                yaw,
                8,
                3.5f,
                12.0f,
                1.94f,
                1.13f,
                -0.56f,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
}

bool EnemySpider::UsesSpecialAttacks() const
{
    return true;
}

bool EnemySpider::UpdateSpecialAttack(NSRender::Render& render,
                                      const D3DXVECTOR3& playerPos,
                                      const bool playerInvincible)
{
    UpdateWebProjectile(render, playerPos, playerInvincible);
    UpdatePoisonPool(render, playerPos, playerInvincible);
    if (m_attackCooldownFrames > 0)
    {
        --m_attackCooldownFrames;
    }

    if (m_attackPhase == AttackPhase::None)
    {
        if (IsSpecialAttackReady() && !playerInvincible && m_attackCooldownFrames <= 0)
        {
            SelectAttack(render, playerPos);
        }
        return m_attackPhase != AttackPhase::None;
    }

    if (m_attackPhase == AttackPhase::Windup)
    {
        if (m_attackType != AttackType::Pounce)
        {
            FaceSpecialAttackTarget(playerPos);
        }
        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            BeginActivePhase(render, playerPos);
        }
    }
    else if (m_attackPhase == AttackPhase::Active)
    {
        UpdateActivePhase(playerPos, playerInvincible);
        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            BeginRecovery();
        }
    }
    else if (m_attackPhase == AttackPhase::Recovery)
    {
        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            EndAttack();
        }
    }

    return m_attackPhase != AttackPhase::None;
}

void EnemySpider::SelectAttack(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    const float distance = HorizontalDistance(GetPosition(), playerPos);
    for (int offset = 0; offset < 4; ++offset)
    {
        const int attackIndex = (m_nextAttackIndex + offset) % 4;
        AttackType candidate = AttackType::Bite;
        if (attackIndex == 1)
        {
            candidate = AttackType::Pounce;
        }
        else if (attackIndex == 2)
        {
            candidate = AttackType::WebShot;
        }
        else if (attackIndex == 3)
        {
            candidate = AttackType::PoisonPool;
        }

        if (IsAttackAllowed(candidate, distance))
        {
            m_nextAttackIndex = (attackIndex + 1) % 4;
            BeginAttack(render, candidate, playerPos);
            return;
        }
    }
}

bool EnemySpider::IsAttackAllowed(const AttackType attackType, const float distance) const
{
    if (attackType == AttackType::Bite)
    {
        return distance <= 1.5f;
    }
    if (attackType == AttackType::Pounce)
    {
        return distance >= 1.2f && distance <= 5.0f;
    }
    if (attackType == AttackType::WebShot)
    {
        return distance >= 2.5f && distance <= 8.0f;
    }
    return distance <= 4.5f;
}

void EnemySpider::BeginAttack(NSRender::Render& render,
                              const AttackType attackType,
                              const D3DXVECTOR3& playerPos)
{
    m_attackType = attackType;
    m_attackPhase = AttackPhase::Windup;
    m_attackHitApplied = false;
    FaceSpecialAttackTarget(playerPos);
    m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);

    if (attackType == AttackType::Bite)
    {
        m_phaseFrames = 10;
        PlaySpecialAttackAnimation(render, L"attack_bite");
    }
    else if (attackType == AttackType::Pounce)
    {
        m_phaseFrames = 24;
        PlaySpecialAttackAnimation(render, L"attack_pounce");
    }
    else if (attackType == AttackType::WebShot)
    {
        m_phaseFrames = 20;
        PlaySpecialAttackAnimation(render, L"attack_web");
    }
    else
    {
        m_phaseFrames = 30;
        PlaySpecialAttackAnimation(render, L"attack_poison");
    }
}

void EnemySpider::BeginActivePhase(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    m_attackPhase = AttackPhase::Active;
    m_attackHitApplied = false;
    if (m_attackType == AttackType::Bite)
    {
        m_phaseFrames = 4;
    }
    else if (m_attackType == AttackType::Pounce)
    {
        m_phaseFrames = 26;
        m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
    }
    else if (m_attackType == AttackType::WebShot)
    {
        m_phaseFrames = 1;
        m_webProjectileActive = true;
        m_webProjectilePosition = GetPosition();
        m_webProjectilePosition.y += 0.6f;
        m_webProjectileDirection = HorizontalDirection(m_webProjectilePosition, playerPos);
        m_webProjectileFrames = 105;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                   m_webProjectilePosition);
    }
    else
    {
        m_phaseFrames = 1;
        m_poisonPoolActive = true;
        m_poisonPoolPosition = playerPos;
        m_poisonPoolPosition.y = GetPosition().y;
        m_poisonPoolFrames = 180;
        m_poisonDamageCooldownFrames = 0;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion,
                                   m_poisonPoolPosition);
    }
}

void EnemySpider::UpdateActivePhase(const D3DXVECTOR3& playerPos,
                                    const bool playerInvincible)
{
    if (m_attackType == AttackType::Pounce)
    {
        MoveForSpecialAttack(m_lockedDirection * 6.5f);
    }
    if (m_attackHitApplied || playerInvincible)
    {
        return;
    }

    const float distance = HorizontalDistance(GetPosition(), playerPos);
    if (m_attackType == AttackType::Bite && distance <= 1.35f)
    {
        EmitAttackHit(10, GetPosition(), 22, 0);
        m_attackHitApplied = true;
    }
    else if (m_attackType == AttackType::Pounce && distance <= 1.0f)
    {
        EmitAttackHit(15, GetPosition(), 42, 0);
        m_attackHitApplied = true;
    }
}

void EnemySpider::UpdateWebProjectile(NSRender::Render& render,
                                      const D3DXVECTOR3& playerPos,
                                      const bool playerInvincible)
{
    if (!m_webProjectileActive)
    {
        return;
    }

    if (MoveSpecialProjectile(&m_webProjectilePosition,
                              m_webProjectileDirection * 5.0f,
                              0.25f))
    {
        m_webProjectileActive = false;
        return;
    }
    --m_webProjectileFrames;
    if ((m_webProjectileFrames % 6) == 0)
    {
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                   m_webProjectilePosition);
    }

    if (!playerInvincible && HorizontalDistance(m_webProjectilePosition, playerPos) <= 0.75f &&
        fabsf(m_webProjectilePosition.y - playerPos.y) <= 1.5f)
    {
        EmitAttackHit(4, m_webProjectilePosition, 10, 120);
        m_webProjectileActive = false;
    }
    else if (m_webProjectileFrames <= 0)
    {
        m_webProjectileActive = false;
    }
}

void EnemySpider::UpdatePoisonPool(NSRender::Render& render,
                                   const D3DXVECTOR3& playerPos,
                                   const bool playerInvincible)
{
    if (!m_poisonPoolActive)
    {
        return;
    }

    --m_poisonPoolFrames;
    if (m_poisonDamageCooldownFrames > 0)
    {
        --m_poisonDamageCooldownFrames;
    }
    if ((m_poisonPoolFrames % 30) == 0)
    {
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                   m_poisonPoolPosition);
    }

    if (!playerInvincible && m_poisonDamageCooldownFrames <= 0 &&
        HorizontalDistance(m_poisonPoolPosition, playerPos) <= 1.8f)
    {
        EmitAttackHit(5, m_poisonPoolPosition, 0, 0);
        m_poisonDamageCooldownFrames = 45;
    }
    if (m_poisonPoolFrames <= 0)
    {
        m_poisonPoolActive = false;
    }
}

void EnemySpider::BeginRecovery()
{
    m_attackPhase = AttackPhase::Recovery;
    if (m_attackType == AttackType::Pounce)
    {
        m_phaseFrames = 24;
    }
    else
    {
        m_phaseFrames = 18;
    }
}

void EnemySpider::EndAttack()
{
    m_attackType = AttackType::None;
    m_attackPhase = AttackPhase::None;
    m_phaseFrames = 0;
    m_attackCooldownFrames = kAttackCooldownFrames;
    FinishSpecialAttack();
}