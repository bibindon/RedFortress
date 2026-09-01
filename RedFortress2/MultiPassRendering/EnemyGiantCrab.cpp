#include "EnemyGiantCrab.h"

#include "../../RedFortressRender/Render/Render.h"

namespace
{
    const int kGiantCrabMaxHp = 12;
    const int kBossGiantCrabMaxHp = kGiantCrabMaxHp * 20;
    const float kBossGiantCrabBodyScale = 2.0f;

    const int kNormalAttackCooldownFrames = 62;
    const int kEnragedAttackCooldownFrames = 36;

    const int kClawSweepWindupFrames = 30;
    const int kClawSweepActiveFrames = 10;
    const int kClawSweepRecoveryFrames = 20;
    const float kClawSweepRange = 3.4f;
    const int kClawSweepDamage = 16;

    const int kSideChargeWindupFrames = 38;
    const int kSideChargeActiveFrames = 34;
    const int kSideChargeRecoveryFrames = 30;
    const float kSideChargeSpeed = 8.0f;
    const float kSideChargeHitRange = 2.1f;
    const int kSideChargeDamage = 20;

    const int kGroundSlamWindupFrames = 46;
    const int kGroundSlamActiveFrames = 8;
    const int kGroundSlamRecoveryFrames = 28;
    const float kGroundSlamRange = 4.4f;
    const float kGroundSlamMaxVerticalDistance = 1.2f;
    const int kGroundSlamDamage = 18;

    const int kBubbleShotWindupFrames = 32;
    const int kBubbleShotActiveFrames = 1;
    const int kBubbleShotRecoveryFrames = 18;
    const int kBubbleProjectileLifetimeFrames = 120;
    const float kBubbleProjectileSpeed = 5.5f;
    const float kBubbleProjectileRadius = 0.3f;
    const float kBubbleHitRange = 0.85f;
    const int kBubbleDamage = 8;
    const int kBubbleSlowFrames = 90;

    const int kJumpSlamWindupFrames = 34;
    const int kJumpSlamActiveFrames = 54;
    const int kJumpSlamRecoveryFrames = 26;
    const float kJumpSlamHeight = 4.0f;
    const float kJumpSlamMaxTravelDistance = 8.0f;
    const float kJumpSlamRange = 3.2f;
    const float kJumpSlamMaxVerticalDistance = 1.2f;
    const int kJumpSlamDamage = 22;

    const int kBurrowDiveFrames = 54;
    const int kBurrowWaitFrames = 300;
    const int kBurrowEmergeFrames = 42;
    const float kBurrowDepth = 4.0f;
    const float kBurrowPitch = -D3DX_PI * 0.5f;
    const float kBurrowEmergeMaxVerticalDistance = 1.4f;
    const int kBurrowEmergeDamage = 26;

    const int kRetreatDashWindupFrames = 10;
    const int kRetreatDashActiveFrames = 28;
    const int kRetreatDashRecoveryFrames = 10;
    const float kRetreatDashSpeed = 9.0f;

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

EnemyGiantCrab::EnemyGiantCrab(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyGiantCrab(pos, meshId, yaw, kGiantCrabMaxHp)
{
}

EnemyGiantCrab::EnemyGiantCrab(const D3DXVECTOR3& pos,
                               const int meshId,
                               const float yaw,
                               const int maxHp,
                               const float bodyScale)
    : EnemyBase(pos,
                meshId,
                L"giant_crab",
                yaw,
                maxHp,
                1.7f,
                10.0f,
                0.54f * 3.0f * bodyScale,
                0.36f * 3.0f * bodyScale,
                -0.18f * 3.0f * bodyScale,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
}

EnemyBossGiantCrab::EnemyBossGiantCrab(const D3DXVECTOR3& pos,
                                       const int meshId,
                                       const float yaw)
    : EnemyGiantCrab(pos,
                     meshId,
                     yaw,
                     kBossGiantCrabMaxHp,
                     kBossGiantCrabBodyScale)
{
    m_arenaSurfaceY = GetPosition().y;
    for (int i = 0; i < kBubbleProjectileCount; ++i)
    {
        m_bubbleProjectilePosition[i] = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        m_bubbleProjectileDirection[i] = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    }
}

bool EnemyBossGiantCrab::UsesSpecialAttacks() const
{
    return true;
}

bool EnemyBossGiantCrab::CanBeStomped() const
{
    return m_attackType != AttackType::BurrowAmbush;
}

float EnemyBossGiantCrab::GetMeshVerticalOffset() const
{
    return EnemyBase::GetMeshVerticalOffset() + m_burrowMeshOffsetY;
}

D3DXVECTOR3 EnemyBossGiantCrab::GetMeshRotationOffset() const
{
    return D3DXVECTOR3(m_burrowPitch, 0.0f, 0.0f);
}

bool EnemyBossGiantCrab::UpdateSpecialAttack(NSRender::Render& render,
                                              const D3DXVECTOR3& playerPos,
                                              const bool playerInvincible)
{
    UpdateBubbleProjectiles(render, playerPos, playerInvincible);
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
        if (m_attackType == AttackType::RetreatDash)
        {
            FaceSpecialAttackTarget(GetPosition() + m_lockedDirection);
        }
        else
        {
            FaceSpecialAttackTarget(playerPos);
        }
        if (m_attackType == AttackType::BurrowAmbush)
        {
            const float progress = 1.0f - static_cast<float>(m_phaseFrames) /
                                              static_cast<float>(kBurrowDiveFrames);
            m_burrowPitch = kBurrowPitch * progress;
            m_burrowMeshOffsetY = -kBurrowDepth * progress;
        }
        if (m_attackType == AttackType::SideCharge)
        {
            m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
            if (D3DXVec3LengthSq(&m_lockedDirection) <= 0.0001f)
            {
                m_lockedDirection = ForwardDirection(GetYaw());
            }
        }

        if (m_attackType == AttackType::GroundSlam && (m_phaseFrames % 12) == 0)
        {
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                       GetPosition());
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
            BeginRecovery();
        }
    }
    else if (m_attackPhase == AttackPhase::Recovery)
    {
        if (m_attackType == AttackType::BurrowAmbush)
        {
            const float progress = 1.0f - static_cast<float>(m_phaseFrames) /
                                              static_cast<float>(kBurrowEmergeFrames);
            m_burrowPitch = kBurrowPitch * (1.0f - progress);
            m_burrowMeshOffsetY = -kBurrowDepth * (1.0f - progress);
        }
        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            EndAttack();
        }
    }

    return m_attackPhase != AttackPhase::None;
}

void EnemyBossGiantCrab::SelectAttack(NSRender::Render& render,
                                      const D3DXVECTOR3& playerPos)
{
    const float distance = HorizontalDistance(GetPosition(), playerPos);
    if (m_attacksUntilRetreat <= 0 && distance <= 5.5f)
    {
        BeginAttack(render, AttackType::RetreatDash, playerPos);
        return;
    }

    for (int offset = 0; offset < 6; ++offset)
    {
        const int attackIndex = (m_nextAttackIndex + offset) % 6;
        AttackType candidate = AttackType::ClawSweep;
        if (attackIndex == 1)
        {
            candidate = AttackType::SideCharge;
        }
        else if (attackIndex == 2)
        {
            candidate = AttackType::GroundSlam;
        }
        else if (attackIndex == 3)
        {
            candidate = AttackType::BubbleShot;
        }
        else if (attackIndex == 4)
        {
            candidate = AttackType::JumpSlam;
        }
        else if (attackIndex == 5)
        {
            candidate = AttackType::BurrowAmbush;
        }

        if (IsAttackAllowed(candidate, distance))
        {
            m_nextAttackIndex = (attackIndex + 1) % 6;
            BeginAttack(render, candidate, playerPos);
            return;
        }
    }
}

bool EnemyBossGiantCrab::IsAttackAllowed(const AttackType attackType,
                                         const float distance) const
{
    if (attackType == AttackType::ClawSweep)
    {
        return distance <= 3.8f;
    }
    if (attackType == AttackType::SideCharge)
    {
        return distance >= 2.4f && distance <= 10.0f;
    }
    if (attackType == AttackType::GroundSlam)
    {
        return distance <= 6.0f;
    }
    if (attackType == AttackType::BubbleShot)
    {
        return distance >= 3.0f && distance <= 11.0f;
    }
    if (attackType == AttackType::BurrowAmbush)
    {
        return true;
    }
    return distance >= 2.5f && distance <= kJumpSlamMaxTravelDistance;
}

void EnemyBossGiantCrab::BeginAttack(NSRender::Render& render,
                                     const AttackType attackType,
                                     const D3DXVECTOR3& playerPos)
{
    m_attackType = attackType;
    m_attackPhase = AttackPhase::Windup;
    m_attackHitApplied = false;
    m_chargeCollided = false;
    if (attackType == AttackType::BurrowAmbush)
    {
        m_burrowMeshOffsetY = 0.0f;
        m_burrowPitch = 0.0f;
    }
    FaceSpecialAttackTarget(playerPos);
    m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
    if (D3DXVec3LengthSq(&m_lockedDirection) <= 0.0001f)
    {
        m_lockedDirection = ForwardDirection(GetYaw());
    }
    if (attackType == AttackType::RetreatDash)
    {
        m_lockedDirection = HorizontalDirection(playerPos, GetPosition());
        if (D3DXVec3LengthSq(&m_lockedDirection) <= 0.0001f)
        {
            m_lockedDirection = ForwardDirection(GetYaw()) * -1.0f;
        }
    }

    D3DXVECTOR3 telegraphPosition = GetPosition();
    telegraphPosition.y += 0.6f;
    render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                               telegraphPosition);

    if (attackType == AttackType::ClawSweep)
    {
        m_phaseFrames = kClawSweepWindupFrames;
    }
    else if (attackType == AttackType::SideCharge)
    {
        m_phaseFrames = kSideChargeWindupFrames;
    }
    else if (attackType == AttackType::GroundSlam)
    {
        m_phaseFrames = kGroundSlamWindupFrames;
    }
    else if (attackType == AttackType::BubbleShot)
    {
        m_phaseFrames = kBubbleShotWindupFrames;
    }
    else if (attackType == AttackType::JumpSlam)
    {
        m_phaseFrames = kJumpSlamWindupFrames;
    }
    else if (attackType == AttackType::BurrowAmbush)
    {
        m_phaseFrames = kBurrowDiveFrames;
    }
    else
    {
        m_phaseFrames = kRetreatDashWindupFrames;
    }

    if (attackType == AttackType::RetreatDash)
    {
        PlaySpecialAttackAnimation(render, L"run");
    }
    else
    {
        PlaySpecialAttackAnimation(render, L"attack");
    }
}

void EnemyBossGiantCrab::BeginActivePhase(NSRender::Render& render,
                                          const D3DXVECTOR3& playerPos)
{
    m_attackPhase = AttackPhase::Active;
    m_attackHitApplied = false;
    if (m_attackType != AttackType::RetreatDash)
    {
        m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
        if (D3DXVec3LengthSq(&m_lockedDirection) <= 0.0001f)
        {
            m_lockedDirection = ForwardDirection(GetYaw());
        }
    }

    if (m_attackType == AttackType::ClawSweep)
    {
        m_phaseFrames = kClawSweepActiveFrames;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Dash,
                                   GetPosition());
    }
    else if (m_attackType == AttackType::SideCharge)
    {
        m_phaseFrames = kSideChargeActiveFrames;
        PlaySpecialAttackAnimation(render, L"run");
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Dash,
                                   GetPosition());
    }
    else if (m_attackType == AttackType::GroundSlam)
    {
        m_phaseFrames = kGroundSlamActiveFrames;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion,
                                   GetPosition());
    }
    else if (m_attackType == AttackType::BubbleShot)
    {
        m_phaseFrames = kBubbleShotActiveFrames;
        SpawnBubbleProjectiles(render, playerPos);
    }
    else if (m_attackType == AttackType::JumpSlam)
    {
        m_phaseFrames = kJumpSlamActiveFrames;
        m_jumpFrame = 0;
        m_jumpStartY = GetPosition().y;
        float travelDistance = HorizontalDistance(GetPosition(), playerPos);
        if (travelDistance > kJumpSlamMaxTravelDistance)
        {
            travelDistance = kJumpSlamMaxTravelDistance;
        }
        const float activeSeconds = static_cast<float>(kJumpSlamActiveFrames) / 60.0f;
        m_jumpHorizontalSpeed = travelDistance / activeSeconds;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Dash,
                                   GetPosition());
    }
    else if (m_attackType == AttackType::BurrowAmbush)
    {
        m_phaseFrames = kBurrowWaitFrames;
        m_burrowPitch = kBurrowPitch;
        m_burrowMeshOffsetY = -kBurrowDepth;
    }
    else
    {
        m_phaseFrames = kRetreatDashActiveFrames;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Dash,
                                   GetPosition());
    }
}

void EnemyBossGiantCrab::UpdateActivePhase(NSRender::Render& render,
                                           const D3DXVECTOR3& playerPos,
                                           const bool playerInvincible)
{
    if (m_attackType == AttackType::BurrowAmbush)
    {
        if (m_phaseFrames <= 1)
        {
            D3DXVECTOR3 emergePosition = GetPosition();
            emergePosition.x = playerPos.x;
            emergePosition.y = m_arenaSurfaceY;
            emergePosition.z = playerPos.z;
            SetPosition(emergePosition);

            D3DXVECTOR3 effectPosition = emergePosition;
            effectPosition.y -= GetHeight() * 0.5f;
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion,
                                       effectPosition);
            if (!playerInvincible &&
                fabsf(playerPos.y - emergePosition.y) <= kBurrowEmergeMaxVerticalDistance)
            {
                EmitAttackHit(kBurrowEmergeDamage, emergePosition, 64, 0);
                m_attackHitApplied = true;
            }
        }
        return;
    }

    if (m_attackType == AttackType::SideCharge && !m_chargeCollided)
    {
        if (MoveForSpecialAttack(m_lockedDirection * kSideChargeSpeed))
        {
            m_chargeCollided = true;
            m_phaseFrames = 1;
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion,
                                       GetPosition());
        }
        else if ((m_phaseFrames % 5) == 0)
        {
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Dash,
                                       GetPosition());
        }
    }

    if (m_attackType == AttackType::RetreatDash)
    {
        if (MoveForSpecialAttack(m_lockedDirection * kRetreatDashSpeed))
        {
            m_phaseFrames = 1;
        }
        else if ((m_phaseFrames % 5) == 0)
        {
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Dash,
                                       GetPosition());
        }
        return;
    }

    if (m_attackType == AttackType::JumpSlam)
    {
        ++m_jumpFrame;
        MoveForSpecialAttack(m_lockedDirection * m_jumpHorizontalSpeed);
        float jumpProgress = static_cast<float>(m_jumpFrame) /
                             static_cast<float>(kJumpSlamActiveFrames);
        if (jumpProgress > 1.0f)
        {
            jumpProgress = 1.0f;
        }
        D3DXVECTOR3 jumpPosition = GetPosition();
        jumpPosition.y = m_jumpStartY + sinf(D3DX_PI * jumpProgress) * kJumpSlamHeight;
        SetPosition(jumpPosition);

        if (m_phaseFrames <= 1)
        {
            jumpPosition.y = m_jumpStartY;
            SetPosition(jumpPosition);
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion,
                                       jumpPosition);
            if (!playerInvincible &&
                HorizontalDistance(jumpPosition, playerPos) <= kJumpSlamRange &&
                fabsf(jumpPosition.y - playerPos.y) <= kJumpSlamMaxVerticalDistance)
            {
                EmitAttackHit(kJumpSlamDamage, jumpPosition, 58, 0);
                m_attackHitApplied = true;
            }
        }
        return;
    }

    if (m_attackHitApplied || playerInvincible)
    {
        return;
    }

    const float distance = HorizontalDistance(GetPosition(), playerPos);
    const float verticalDistance = fabsf(GetPosition().y - playerPos.y);
    if (m_attackType == AttackType::ClawSweep && distance <= kClawSweepRange &&
        verticalDistance <= 1.8f)
    {
        D3DXVECTOR3 playerDirection = HorizontalDirection(GetPosition(), playerPos);
        if (D3DXVec3Dot(&playerDirection, &m_lockedDirection) >= -0.2f)
        {
            EmitAttackHit(kClawSweepDamage, GetPosition(), 38, 0);
            m_attackHitApplied = true;
        }
    }
    else if (m_attackType == AttackType::SideCharge && distance <= kSideChargeHitRange &&
             verticalDistance <= 1.8f)
    {
        EmitAttackHit(kSideChargeDamage, GetPosition(), 54, 0);
        m_attackHitApplied = true;
    }
    else if (m_attackType == AttackType::GroundSlam && distance <= kGroundSlamRange &&
             verticalDistance <= kGroundSlamMaxVerticalDistance)
    {
        EmitAttackHit(kGroundSlamDamage, GetPosition(), 48, 0);
        m_attackHitApplied = true;
    }
}

void EnemyBossGiantCrab::SpawnBubbleProjectiles(NSRender::Render& render,
                                                const D3DXVECTOR3& playerPos)
{
    int projectileCount = 1;
    if (IsEnraged())
    {
        projectileCount = kBubbleProjectileCount;
    }

    D3DXVECTOR3 origin = GetPosition();
    origin.y += 0.55f;
    D3DXVECTOR3 centerDirection = HorizontalDirection(origin, playerPos);
    if (D3DXVec3LengthSq(&centerDirection) <= 0.0001f)
    {
        centerDirection = m_lockedDirection;
    }
    const D3DXVECTOR3 sideDirection(centerDirection.z, 0.0f, -centerDirection.x);

    for (int i = 0; i < projectileCount; ++i)
    {
        float spread = 0.0f;
        if (projectileCount > 1)
        {
            spread = static_cast<float>(i - 1) * 0.24f;
        }
        D3DXVECTOR3 direction = centerDirection + sideDirection * spread;
        D3DXVec3Normalize(&direction, &direction);

        m_bubbleProjectileActive[i] = true;
        m_bubbleProjectilePosition[i] = origin;
        m_bubbleProjectileDirection[i] = direction;
        m_bubbleProjectileFrames[i] = kBubbleProjectileLifetimeFrames;
    }

    render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage, origin);
}

void EnemyBossGiantCrab::UpdateBubbleProjectiles(NSRender::Render& render,
                                                 const D3DXVECTOR3& playerPos,
                                                 const bool playerInvincible)
{
    for (int i = 0; i < kBubbleProjectileCount; ++i)
    {
        if (!m_bubbleProjectileActive[i])
        {
            continue;
        }

        if (MoveSpecialProjectile(&m_bubbleProjectilePosition[i],
                                  m_bubbleProjectileDirection[i] * kBubbleProjectileSpeed,
                                  kBubbleProjectileRadius))
        {
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                       m_bubbleProjectilePosition[i]);
            m_bubbleProjectileActive[i] = false;
            continue;
        }

        --m_bubbleProjectileFrames[i];
        if ((m_bubbleProjectileFrames[i] % 5) == 0)
        {
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                       m_bubbleProjectilePosition[i]);
        }

        if (!playerInvincible &&
            HorizontalDistance(m_bubbleProjectilePosition[i], playerPos) <= kBubbleHitRange &&
            fabsf(m_bubbleProjectilePosition[i].y - playerPos.y) <= 1.5f)
        {
            EmitAttackHit(kBubbleDamage, m_bubbleProjectilePosition[i], 12, kBubbleSlowFrames);
            m_bubbleProjectileActive[i] = false;
        }
        else if (m_bubbleProjectileFrames[i] <= 0)
        {
            m_bubbleProjectileActive[i] = false;
        }
    }
}

void EnemyBossGiantCrab::BeginRecovery()
{
    m_attackPhase = AttackPhase::Recovery;
    if (m_attackType == AttackType::ClawSweep)
    {
        m_phaseFrames = kClawSweepRecoveryFrames;
    }
    else if (m_attackType == AttackType::SideCharge)
    {
        m_phaseFrames = kSideChargeRecoveryFrames;
    }
    else if (m_attackType == AttackType::GroundSlam)
    {
        m_phaseFrames = kGroundSlamRecoveryFrames;
    }
    else if (m_attackType == AttackType::BubbleShot)
    {
        m_phaseFrames = kBubbleShotRecoveryFrames;
    }
    else if (m_attackType == AttackType::JumpSlam)
    {
        m_phaseFrames = kJumpSlamRecoveryFrames;
    }
    else if (m_attackType == AttackType::BurrowAmbush)
    {
        m_phaseFrames = kBurrowEmergeFrames;
    }
    else
    {
        m_phaseFrames = kRetreatDashRecoveryFrames;
    }
}

void EnemyBossGiantCrab::EndAttack()
{
    const AttackType finishedAttack = m_attackType;
    m_attackType = AttackType::None;
    m_attackPhase = AttackPhase::None;
    m_phaseFrames = 0;
    m_chargeCollided = false;
    m_jumpFrame = 0;
    m_jumpHorizontalSpeed = 0.0f;
    m_burrowMeshOffsetY = 0.0f;
    m_burrowPitch = 0.0f;

    if (finishedAttack == AttackType::RetreatDash)
    {
        if (IsEnraged())
        {
            m_attacksUntilRetreat = 1;
        }
        else
        {
            m_attacksUntilRetreat = 2;
        }
        m_attackCooldownFrames = 14;
        FinishSpecialAttack();
        return;
    }

    if (m_attacksUntilRetreat > 0)
    {
        --m_attacksUntilRetreat;
    }

    if (IsEnraged())
    {
        m_attackCooldownFrames = kEnragedAttackCooldownFrames;
        if (finishedAttack == AttackType::SideCharge)
        {
            m_nextAttackIndex = 0;
            m_attackCooldownFrames = 8;
        }
    }
    else
    {
        m_attackCooldownFrames = kNormalAttackCooldownFrames;
    }
    FinishSpecialAttack();
}

bool EnemyBossGiantCrab::IsEnraged() const
{
    return GetHp() * 2 <= GetMaxHp();
}
