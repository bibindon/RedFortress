#include "EnemyKanata.h"

#include "../../RedFortressRender/Render/Render.h"

namespace
{
    const int kAttackCooldownFrames = 54;

    const int kHammerWindupFrames = 34;
    const int kHammerActiveFrames = 10;
    const int kHammerRecoveryFrames = 18;
    const float kHammerRange = 2.0f;
    const int kHammerDamage = 18;

    const int kSweepWindupFrames = 33;
    const int kSweepActiveFrames = 9;
    const int kSweepRecoveryFrames = 16;
    const float kSweepRange = 2.7f;
    const int kSweepDamage = 14;

    const int kBarrageWindupFrames = 34;
    const int kBarrageActiveFrames = 33;
    const int kBarrageRecoveryFrames = 17;
    const int kBarrageShotIntervalFrames = 8;
    const int kBarrageProjectileLifetimeFrames = 90;
    const float kBarrageProjectileSpeed = 8.0f;
    const float kBarrageProjectileRadius = 0.25f;
    const int kBarrageDamage = 6;

    const int kDiveWindupFrames = 55;
    const int kDiveActiveFrames = 15;
    const int kDiveRecoveryFrames = 22;
    const float kDiveSpeed = 9.0f;
    const float kDiveLandingRange = 3.0f;
    const int kDiveDamage = 20;

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

    D3DXVECTOR3 ForwardDirection(const float yaw)
    {
        return D3DXVECTOR3(-sinf(yaw), 0.0f, -cosf(yaw));
    }
}

EnemyKanata::EnemyKanata(const D3DXVECTOR3& pos,
                         const int meshId,
                         const float yaw)
    : EnemyBase(pos,
                meshId,
                L"kanata",
                yaw,
                80,
                1.8f,
                20.0f,
                0.70f,
                GetCollisionHeight(),
                -GetCollisionHeight() * 0.5f,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
    for (int i = 0; i < kBarrageProjectileCount; ++i)
    {
        m_barrageProjectilePosition[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_barrageProjectileDirection[i] = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    }
}

bool EnemyKanata::UsesSpecialAttacks() const
{
    return true;
}

bool EnemyKanata::UpdateSpecialAttack(NSRender::Render& render,
                                      const D3DXVECTOR3& playerPos,
                                      const bool playerInvincible)
{
    UpdateBarrageProjectiles(render, playerPos, playerInvincible);
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
        FaceSpecialAttackTarget(playerPos);
        if (m_attackType == AttackType::Dive)
        {
            m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
            if (D3DXVec3LengthSq(&m_lockedDirection) <= 0.0001f)
            {
                m_lockedDirection = ForwardDirection(GetYaw());
            }
        }

        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            BeginActivePhase(render, playerPos);
        }
    }
    else if (m_attackPhase == AttackPhase::Active)
    {
        UpdateActivePhase(render, playerPos, playerInvincible);
        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            BeginRecovery(render, playerPos, playerInvincible);
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

void EnemyKanata::SelectAttack(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    const float distance = HorizontalDistance(GetPosition(), playerPos);
    for (int offset = 0; offset < 4; ++offset)
    {
        const int attackIndex = (m_nextAttackIndex + offset) % 4;
        AttackType candidate = AttackType::Hammer;
        if (attackIndex == 1)
        {
            candidate = AttackType::Sweep;
        }
        else if (attackIndex == 2)
        {
            candidate = AttackType::Barrage;
        }
        else if (attackIndex == 3)
        {
            candidate = AttackType::Dive;
        }

        if (IsAttackAllowed(candidate, distance))
        {
            m_nextAttackIndex = (attackIndex + 1) % 4;
            BeginAttack(render, candidate, playerPos);
            return;
        }
    }
}

bool EnemyKanata::IsAttackAllowed(const AttackType attackType, const float distance) const
{
    if (attackType == AttackType::Hammer)
    {
        return distance <= 3.0f;
    }
    if (attackType == AttackType::Sweep)
    {
        return distance <= 2.5f;
    }
    if (attackType == AttackType::Barrage)
    {
        return distance >= 3.0f && distance <= 10.0f;
    }
    return distance >= 2.5f && distance <= 9.0f;
}

void EnemyKanata::BeginAttack(NSRender::Render& render,
                              const AttackType attackType,
                              const D3DXVECTOR3& playerPos)
{
    m_attackType = attackType;
    m_attackPhase = AttackPhase::Windup;
    m_attackHitApplied = false;
    m_diveCollided = false;
    m_barrageActiveFrames = 0;
    m_barrageShotsFired = 0;
    FaceSpecialAttackTarget(playerPos);
    m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
    if (D3DXVec3LengthSq(&m_lockedDirection) <= 0.0001f)
    {
        m_lockedDirection = ForwardDirection(GetYaw());
    }

    D3DXVECTOR3 telegraphPosition = GetPosition();
    telegraphPosition.y += 0.8f;
    render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                               telegraphPosition);

    if (attackType == AttackType::Hammer)
    {
        m_phaseFrames = kHammerWindupFrames;
        PlaySpecialAttackAnimation(render, L"attack_hammer");
    }
    else if (attackType == AttackType::Sweep)
    {
        m_phaseFrames = kSweepWindupFrames;
        PlaySpecialAttackAnimation(render, L"attack_sweep");
    }
    else if (attackType == AttackType::Barrage)
    {
        m_phaseFrames = kBarrageWindupFrames;
        PlaySpecialAttackAnimation(render, L"attack_barrage");
    }
    else
    {
        m_phaseFrames = kDiveWindupFrames;
        PlaySpecialAttackAnimation(render, L"attack_dive");
    }
}

void EnemyKanata::BeginActivePhase(NSRender::Render& render,
                                   const D3DXVECTOR3& playerPos)
{
    m_attackPhase = AttackPhase::Active;
    m_attackHitApplied = false;
    m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
    if (D3DXVec3LengthSq(&m_lockedDirection) <= 0.0001f)
    {
        m_lockedDirection = ForwardDirection(GetYaw());
    }

    if (m_attackType == AttackType::Hammer)
    {
        m_phaseFrames = kHammerActiveFrames;
        D3DXVECTOR3 impactPosition = GetPosition() + m_lockedDirection * 1.2f;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion,
                                   impactPosition);
    }
    else if (m_attackType == AttackType::Sweep)
    {
        m_phaseFrames = kSweepActiveFrames;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Dash,
                                   GetPosition());
    }
    else if (m_attackType == AttackType::Barrage)
    {
        m_phaseFrames = kBarrageActiveFrames;
        m_barrageActiveFrames = 0;
        m_barrageShotsFired = 0;
        SpawnBarrageProjectile(render, playerPos);
    }
    else
    {
        m_phaseFrames = kDiveActiveFrames;
        m_diveCollided = false;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Dash,
                                   GetPosition());
    }
}

void EnemyKanata::UpdateActivePhase(NSRender::Render& render,
                                    const D3DXVECTOR3& playerPos,
                                    const bool playerInvincible)
{
    if (m_attackType == AttackType::Barrage)
    {
        FaceSpecialAttackTarget(playerPos);
        ++m_barrageActiveFrames;
        if (m_barrageShotsFired < kBarrageProjectileCount)
        {
            const int nextShotFrame = m_barrageShotsFired * kBarrageShotIntervalFrames;
            if (m_barrageActiveFrames >= nextShotFrame)
            {
                SpawnBarrageProjectile(render, playerPos);
            }
        }
        return;
    }

    if (m_attackType == AttackType::Dive && !m_diveCollided)
    {
        if (MoveForSpecialAttack(m_lockedDirection * kDiveSpeed))
        {
            m_diveCollided = true;
            m_phaseFrames = 1;
        }
    }

    if (m_attackHitApplied || playerInvincible)
    {
        return;
    }

    if (m_attackType == AttackType::Hammer)
    {
        const D3DXVECTOR3 impactPosition = GetPosition() + m_lockedDirection * 1.2f;
        if (HorizontalDistance(impactPosition, playerPos) <= kHammerRange &&
            fabsf(impactPosition.y - playerPos.y) <= 2.2f)
        {
            EmitAttackHit(kHammerDamage, impactPosition, 52, 0);
            m_attackHitApplied = true;
        }
    }
    else if (m_attackType == AttackType::Sweep)
    {
        if (HorizontalDistance(GetPosition(), playerPos) <= kSweepRange &&
            fabsf(GetPosition().y - playerPos.y) <= 2.2f)
        {
            EmitAttackHit(kSweepDamage, GetPosition(), 34, 0);
            m_attackHitApplied = true;
        }
    }
}

void EnemyKanata::SpawnBarrageProjectile(NSRender::Render& render,
                                         const D3DXVECTOR3& playerPos)
{
    if (m_barrageShotsFired >= kBarrageProjectileCount)
    {
        return;
    }

    const int projectileIndex = m_barrageShotsFired;
    D3DXVECTOR3 origin = GetPosition();
    origin.y += 0.8f;
    const D3DXVECTOR3 side(m_lockedDirection.z, 0.0f, -m_lockedDirection.x);
    float sideOffset = -0.45f;
    if ((projectileIndex % 2) != 0)
    {
        sideOffset = 0.45f;
    }
    origin += side * sideOffset;

    D3DXVECTOR3 direction = HorizontalDirection(origin, playerPos);
    if (D3DXVec3LengthSq(&direction) <= 0.0001f)
    {
        direction = m_lockedDirection;
    }

    m_barrageProjectileActive[projectileIndex] = true;
    m_barrageProjectilePosition[projectileIndex] = origin;
    m_barrageProjectileDirection[projectileIndex] = direction;
    m_barrageProjectileFrames[projectileIndex] = kBarrageProjectileLifetimeFrames;
    ++m_barrageShotsFired;

    render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage, origin);
}

void EnemyKanata::UpdateBarrageProjectiles(NSRender::Render& render,
                                           const D3DXVECTOR3& playerPos,
                                           const bool playerInvincible)
{
    for (int i = 0; i < kBarrageProjectileCount; ++i)
    {
        if (!m_barrageProjectileActive[i])
        {
            continue;
        }

        if (MoveSpecialProjectile(&m_barrageProjectilePosition[i],
                                  m_barrageProjectileDirection[i] * kBarrageProjectileSpeed,
                                  kBarrageProjectileRadius))
        {
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                       m_barrageProjectilePosition[i]);
            m_barrageProjectileActive[i] = false;
            continue;
        }

        --m_barrageProjectileFrames[i];
        if ((m_barrageProjectileFrames[i] % 4) == 0)
        {
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                       m_barrageProjectilePosition[i]);
        }

        if (!playerInvincible &&
            HorizontalDistance(m_barrageProjectilePosition[i], playerPos) <= 0.75f &&
            fabsf(m_barrageProjectilePosition[i].y - playerPos.y) <= 2.0f)
        {
            EmitAttackHit(kBarrageDamage, m_barrageProjectilePosition[i], 14, 0);
            m_barrageProjectileActive[i] = false;
        }
        else if (m_barrageProjectileFrames[i] <= 0)
        {
            m_barrageProjectileActive[i] = false;
        }
    }
}

void EnemyKanata::BeginRecovery(NSRender::Render& render,
                                const D3DXVECTOR3& playerPos,
                                const bool playerInvincible)
{
    m_attackPhase = AttackPhase::Recovery;
    if (m_attackType == AttackType::Hammer)
    {
        m_phaseFrames = kHammerRecoveryFrames;
    }
    else if (m_attackType == AttackType::Sweep)
    {
        m_phaseFrames = kSweepRecoveryFrames;
    }
    else if (m_attackType == AttackType::Barrage)
    {
        m_phaseFrames = kBarrageRecoveryFrames;
    }
    else
    {
        m_phaseFrames = kDiveRecoveryFrames;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion,
                                   GetPosition());
        if (!m_attackHitApplied && !playerInvincible &&
            HorizontalDistance(GetPosition(), playerPos) <= kDiveLandingRange &&
            fabsf(GetPosition().y - playerPos.y) <= 2.2f)
        {
            EmitAttackHit(kDiveDamage, GetPosition(), 60, 0);
            m_attackHitApplied = true;
        }
    }
}

void EnemyKanata::EndAttack()
{
    m_attackType = AttackType::None;
    m_attackPhase = AttackPhase::None;
    m_phaseFrames = 0;
    m_attackCooldownFrames = kAttackCooldownFrames;
    m_barrageActiveFrames = 0;
    m_barrageShotsFired = 0;
    m_diveCollided = false;
    FinishSpecialAttack();
}
