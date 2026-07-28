#include "EnemySkeleton.h"

#include "../../RedFortressRender/Render/Render.h"

namespace
{
    const int kAttackCooldownFrames = 42;

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

EnemySkeleton::EnemySkeleton(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"skeleton",
                yaw,
                12,
                2.5f,
                13.0f,
                0.45f,
                1.7f,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
}

bool EnemySkeleton::UsesSpecialAttacks() const
{
    return true;
}

bool EnemySkeleton::UpdateSpecialAttack(NSRender::Render& render,
                                        const D3DXVECTOR3& playerPos,
                                        const bool playerInvincible)
{
    UpdateBoneProjectile(render, playerPos, playerInvincible);
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
        if (m_attackType != AttackType::Charge)
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

void EnemySkeleton::SelectAttack(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    const float distance = HorizontalDistance(GetPosition(), playerPos);
    for (int offset = 0; offset < 4; ++offset)
    {
        const int attackIndex = (m_nextAttackIndex + offset) % 4;
        AttackType candidate = AttackType::Slash;
        if (attackIndex == 1)
        {
            candidate = AttackType::Smash;
        }
        else if (attackIndex == 2)
        {
            candidate = AttackType::Charge;
        }
        else if (attackIndex == 3)
        {
            candidate = AttackType::BoneShot;
        }

        if (IsAttackAllowed(candidate, distance))
        {
            m_nextAttackIndex = (attackIndex + 1) % 4;
            BeginAttack(render, candidate, playerPos);
            return;
        }
    }
}

bool EnemySkeleton::IsAttackAllowed(const AttackType attackType, const float distance) const
{
    if (attackType == AttackType::Slash)
    {
        return distance <= 2.0f;
    }
    if (attackType == AttackType::Smash)
    {
        return distance <= 2.8f;
    }
    if (attackType == AttackType::Charge)
    {
        return distance >= 1.5f && distance <= 5.5f;
    }
    return distance >= 2.5f && distance <= 8.0f;
}

void EnemySkeleton::BeginAttack(NSRender::Render& render,
                                const AttackType attackType,
                                const D3DXVECTOR3& playerPos)
{
    m_attackType = attackType;
    m_attackPhase = AttackPhase::Windup;
    m_attackHitApplied = false;
    FaceSpecialAttackTarget(playerPos);
    m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);

    if (attackType == AttackType::Slash)
    {
        m_phaseFrames = 12;
        PlaySpecialAttackAnimation(render, L"attack_slash");
    }
    else if (attackType == AttackType::Smash)
    {
        m_phaseFrames = 28;
        PlaySpecialAttackAnimation(render, L"attack_smash");
    }
    else if (attackType == AttackType::Charge)
    {
        m_phaseFrames = 20;
        PlaySpecialAttackAnimation(render, L"attack_charge");
    }
    else
    {
        m_phaseFrames = 24;
        PlaySpecialAttackAnimation(render, L"attack_bone");
    }
}

void EnemySkeleton::BeginActivePhase(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    m_attackPhase = AttackPhase::Active;
    m_attackHitApplied = false;
    if (m_attackType == AttackType::Slash)
    {
        m_phaseFrames = 4;
    }
    else if (m_attackType == AttackType::Smash)
    {
        m_phaseFrames = 6;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion, GetPosition());
    }
    else if (m_attackType == AttackType::Charge)
    {
        m_phaseFrames = 30;
        m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
    }
    else
    {
        m_phaseFrames = 1;
        m_boneProjectileActive = true;
        m_boneProjectilePosition = GetPosition();
        m_boneProjectilePosition.y += 1.1f;
        m_boneProjectileDirection = HorizontalDirection(m_boneProjectilePosition, playerPos);
        m_boneProjectileFrames = 90;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                   m_boneProjectilePosition);
    }
}

void EnemySkeleton::UpdateActivePhase(const D3DXVECTOR3& playerPos,
                                      const bool playerInvincible)
{
    if (m_attackType == AttackType::Charge)
    {
        MoveForSpecialAttack(m_lockedDirection * 5.5f);
    }
    if (m_attackHitApplied || playerInvincible)
    {
        return;
    }

    const float distance = HorizontalDistance(GetPosition(), playerPos);
    bool hit = false;
    int damage = 0;
    int knockbackFrames = 0;
    if (m_attackType == AttackType::Slash && distance <= 1.9f)
    {
        const D3DXVECTOR3 forward(-sinf(GetYaw()), 0.0f, -cosf(GetYaw()));
        const D3DXVECTOR3 toPlayer = HorizontalDirection(GetPosition(), playerPos);
        hit = D3DXVec3Dot(&forward, &toPlayer) >= 0.35f;
        damage = 10;
        knockbackFrames = 24;
    }
    else if (m_attackType == AttackType::Smash && distance <= 2.4f)
    {
        hit = true;
        damage = 18;
        knockbackFrames = 50;
    }
    else if (m_attackType == AttackType::Charge && distance <= 1.0f)
    {
        hit = true;
        damage = 13;
        knockbackFrames = 40;
    }

    if (hit)
    {
        EmitAttackHit(damage, GetPosition(), knockbackFrames, 0);
        m_attackHitApplied = true;
    }
}

void EnemySkeleton::UpdateBoneProjectile(NSRender::Render& render,
                                         const D3DXVECTOR3& playerPos,
                                         const bool playerInvincible)
{
    if (!m_boneProjectileActive)
    {
        return;
    }

    if (MoveSpecialProjectile(&m_boneProjectilePosition,
                              m_boneProjectileDirection * 6.0f,
                              0.2f))
    {
        m_boneProjectileActive = false;
        return;
    }
    --m_boneProjectileFrames;
    if ((m_boneProjectileFrames % 6) == 0)
    {
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                   m_boneProjectilePosition);
    }

    if (!playerInvincible && HorizontalDistance(m_boneProjectilePosition, playerPos) <= 0.7f &&
        fabsf(m_boneProjectilePosition.y - playerPos.y) <= 1.5f)
    {
        EmitAttackHit(8, m_boneProjectilePosition, 20, 0);
        m_boneProjectileActive = false;
    }
    else if (m_boneProjectileFrames <= 0)
    {
        m_boneProjectileActive = false;
    }
}

void EnemySkeleton::BeginRecovery()
{
    m_attackPhase = AttackPhase::Recovery;
    if (m_attackType == AttackType::Smash)
    {
        m_phaseFrames = 28;
    }
    else
    {
        m_phaseFrames = 18;
    }
}

void EnemySkeleton::EndAttack()
{
    m_attackType = AttackType::None;
    m_attackPhase = AttackPhase::None;
    m_phaseFrames = 0;
    m_attackCooldownFrames = kAttackCooldownFrames;
    FinishSpecialAttack();
}