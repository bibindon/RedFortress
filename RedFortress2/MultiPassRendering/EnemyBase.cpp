#include "EnemyBase.h"

#include "../../RedFortressRender/Render/Render.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"

namespace
{
    const float kTargetFrameSeconds = 1.0f / 60.0f;

    float ClampFloat(float v, float lo, float hi)
    {
        if (v < lo) return lo;
        if (v > hi) return hi;
        return v;
    }

    float MoveAngleToward(float current, float target, float maxDelta)
    {
        float diff = target - current;
        while (diff > D3DX_PI)  diff -= 2.0f * D3DX_PI;
        while (diff < -D3DX_PI) diff += 2.0f * D3DX_PI;
        if (fabsf(diff) <= maxDelta) return target;
        if (diff > 0.0f)
        {
            return current + maxDelta;
        }
        return current - maxDelta;
    }

    float GetYawToTarget(const D3DXVECTOR3& fromPos, const D3DXVECTOR3& targetPos)
    {
        const D3DXVECTOR3 diff = targetPos - fromPos;
        return atan2f(-diff.x, -diff.z);
    }

    unsigned int MixSeed(unsigned int seed, unsigned int value)
    {
        seed ^= value + 0x9e3779b9u + (seed << 6) + (seed >> 2);
        return seed;
    }

    const int kFacePlayerTurnFrames = 30;
    const int kAlertFrames = 18;
    const int kHitStunFrames = 60;
    const float kStompMaxDistanceAboveEnemy = 0.3f;
    const int kLastKnownPlayerFrames = 120;
    const float kEnemyGravity = 9.8f;
    const float kEnemyMaxFallSpeed = 30.0f;
    const float kEnemyFallDeathY = -10.0f;
    const float kGroundNormalYThreshold = 0.3f;
}

EnemyBase::EnemyBase(const D3DXVECTOR3& startPosition,
                     const int meshId,
                     const std::wstring& type,
                     const float yaw,
                     const int maxHp,
                     const float moveSpeed,
                     const float viewDistance,
                     const float contactRadius,
                     const float height,
                     const float meshVerticalOffset,
                     const MovementMode movementMode,
                     const bool usesExtendedAnimations,
                     const HitReactionMode hitReactionMode,
                     const float physicsRadius)
    : m_position(startPosition)
{
    // スポーンCSV等のY座標は足元付近の高さを想定した値である。敵座標は
    // 衝突円柱の中心なので、そのまま使うと背の高い地上敵は円柱の下半分が
    // 地面へめり込み、下向きレイが地面上面に当たらず半埋まりのまま固定される。
    // そのため地上敵は円柱の半分の高さを加えて中心座標へ変換する。
    // 飛行敵はY座標を飛行高度としてそのまま使うため補正しない。
    if (movementMode == MovementMode::Ground ||
        movementMode == MovementMode::Frog)
    {
        m_position.y += height * 0.5f;
    }
    m_homePosition = m_position;
    m_lastKnownPlayerPosition = m_position;
    m_meshId = meshId;
    m_type = type;
    m_maxHp = maxHp;
    m_hp = m_maxHp;
    m_moveSpeed = moveSpeed;
    m_viewDistance = viewDistance;
    m_contactRadius = contactRadius;
    // 物理半径が負の場合は接触半径と同じ（後方互換）とする。
    if (physicsRadius < 0.0f)
    {
        m_physicsRadius = contactRadius;
    }
    else
    {
        m_physicsRadius = physicsRadius;
    }
    m_height = height;
    m_meshVerticalOffset = meshVerticalOffset;
    m_movementMode = movementMode;
    m_usesExtendedAnimations = usesExtendedAnimations;
    m_hitReactionMode = hitReactionMode;
    m_state = State::Idle;
    m_animState = AnimState::Idle;
    m_yaw = yaw;
    m_blinkFrames = 0;
    m_hitStunFrames = 0;
    m_knockbackPerFrame = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_knockbackFrames = 0;
    m_removalFrames = 0;
    m_verticalVelocity = 0.0f;
    m_isGrounded = false;
    m_facePlayerTurnFrames = 0;
    m_alertFrames = 0;
    m_idleWaitFrames = 0;
    m_idleMoveFrames = 0;
    m_lastKnownPlayerFrames = 0;
    m_chaseStrafeFrames = 0;
    m_retreatFrames = 0;
    m_idleMoveYaw = yaw;
    m_chaseStrafeDirection = 1.0f;

    unsigned int seed = 2166136261u;
    const int xBits = static_cast<int>((startPosition.x + 1000.0f) * 100.0f);
    const int yBits = static_cast<int>((startPosition.y + 1000.0f) * 100.0f);
    const int zBits = static_cast<int>((startPosition.z + 1000.0f) * 100.0f);
    seed = MixSeed(seed, static_cast<unsigned int>(xBits));
    seed = MixSeed(seed, static_cast<unsigned int>(yBits));
    seed = MixSeed(seed, static_cast<unsigned int>(zBits));
    seed = MixSeed(seed, static_cast<unsigned int>(meshId + 1));
    for (wchar_t ch : type)
    {
        seed = MixSeed(seed, static_cast<unsigned int>(ch));
    }
    m_behaviorSeed = seed;
    m_personalityBias = NextRandom01() * 2.0f - 1.0f;
    StartIdleBehavior();
}

void EnemyBase::Update(NSRender::Render& render, const D3DXVECTOR3& playerPos, bool playerInvincible)
{
    AnimState nextAnim = AnimState::Idle;
    ++m_flightFrame;
    if (m_frogJumpCooldownFrames > 0)
    {
        --m_frogJumpCooldownFrames;
    }

    if (m_knockbackFrames > 0)
    {
        MoveWithCollision(m_knockbackPerFrame / kTargetFrameSeconds);
        --m_knockbackFrames;
        if (m_knockbackFrames <= 0)
        {
            m_knockbackPerFrame = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }
    }

    if (m_state == State::Dead)
    {
        if (m_removalFrames > 0)
        {
            --m_removalFrames;
        }
        return;
    }

    if (m_movementMode == MovementMode::Ground ||
        m_movementMode == MovementMode::Frog)
    {
        ApplyGravity(render);
    }
    if (m_state == State::Dead)
    {
        return;
    }

    if (m_blinkFrames > 0)
    {
        --m_blinkFrames;
        if (m_blinkFrames <= 0 && m_state != State::Dead && m_meshId >= 0)
        {
            render.StopMeshMixSkinAnimBlink(m_meshId);
        }
    }

    if (m_lastKnownPlayerFrames > 0)
    {
        --m_lastKnownPlayerFrames;
    }

    if (m_hitStunFrames > 0)
    {
        --m_hitStunFrames;
        if (m_hitStunFrames <= 0 && m_meshId >= 0)
        {
            render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
            m_animationNeedsRefresh = true;
        }
        return;
    }

    if (m_forcedAnimationFrames > 0)
    {
        --m_forcedAnimationFrames;
        if (m_forcedAnimationFrames <= 0)
        {
            m_animationNeedsRefresh = true;
        }
        return;
    }

    if (UpdateSpecialAttack(render, playerPos, playerInvincible))
    {
        return;
    }

    if (m_state == State::Alert)
    {
        if (m_alertFrames > 0)
        {
            UpdateFacePlayerTurn(m_lastKnownPlayerPosition);
            --m_alertFrames;
        }
        if (m_alertFrames <= 0)
        {
            m_state = State::Chase;
        }
        nextAnim = AnimState::Idle;
    }
    else if (m_state == State::Retreat)
    {
        UpdateRetreatBehavior(render, playerPos);
        nextAnim = AnimState::Walk;
    }
    else if (m_state == State::Chase)
    {
        UpdateChaseBehavior(playerPos, playerInvincible);
        const D3DXVECTOR3 chaseDiff = m_lastKnownPlayerPosition - m_position;
        const float chaseDistance = D3DXVec3Length(&chaseDiff);
        if (m_state == State::Chase)
        {
            if (chaseDistance > 3.0f)
            {
                nextAnim = AnimState::Run;
            }
            else if (chaseDistance > 2.0f)
            {
                nextAnim = AnimState::Walk;
            }
            else
            {
                nextAnim = AnimState::Creep;
            }
        }
    }
    else // Idle
    {
        if (!playerInvincible)
        {
            const D3DXVECTOR3 diff = playerPos - m_position;
            const float distance = D3DXVec3Length(&diff);
            if (distance <= m_viewDistance && IsPlayerInView(playerPos))
            {
                BeginAlert(playerPos, false);
            }
        }

        if (m_state == State::Idle)
        {
            if (m_facePlayerTurnFrames > 0)
            {
                UpdateFacePlayerTurn(playerPos);
            }
            if (m_movementMode == MovementMode::Hover ||
                m_movementMode == MovementMode::Swoop)
            {
                UpdateFlyingIdleBehavior();
            }
            else
            {
                UpdateIdleBehavior();
            }
            if (m_idleMoveFrames > 0)
            {
                nextAnim = AnimState::Walk;
            }
        }
    }

    if (m_movementMode == MovementMode::Frog && m_frogJumpActive)
    {
        nextAnim = AnimState::Run;
    }
    ApplyAnimation(render, nextAnim);
}

void EnemyBase::SyncMesh(NSRender::Render& render)
{
    if (m_meshId < 0)
    {
        return;
    }

    D3DXVECTOR3 meshPosition = m_position;
    meshPosition.y += GetMeshVerticalOffset();
    render.SetMeshMixSkinAnimPos(m_meshId, meshPosition);
    render.SetMeshMixSkinAnimRotY(m_meshId, m_yaw + GetMeshYawOffset());
}

bool EnemyBase::ConsumeAttackHit(AttackHit* outHit)
{
    if (!m_hasPendingAttackHit || outHit == nullptr)
    {
        return false;
    }

    *outHit = m_pendingAttackHit;
    m_pendingAttackHit = AttackHit();
    m_hasPendingAttackHit = false;
    return true;
}

bool EnemyBase::UsesSpecialAttacks() const
{
    return false;
}

bool EnemyBase::UpdateSpecialAttack(NSRender::Render& render,
                                    const D3DXVECTOR3& playerPos,
                                    const bool playerInvincible)
{
    return false;
}

bool EnemyBase::IsSpecialAttackReady() const
{
    return m_state == State::Chase;
}

void EnemyBase::FaceSpecialAttackTarget(const D3DXVECTOR3& targetPos)
{
    FaceTargetImmediately(targetPos);
}

bool EnemyBase::MoveForSpecialAttack(const D3DXVECTOR3& velocity)
{
    return MoveWithCollision(velocity);
}

bool EnemyBase::MoveSpecialProjectile(D3DXVECTOR3* position,
                                      const D3DXVECTOR3& velocity,
                                      const float radius)
{
    if (position == nullptr)
    {
        return true;
    }

    D3DXVECTOR3 resolvedPosition = *position;
    D3DXVECTOR3 resolvedVelocity = velocity;
    const bool collided = PhysicsLib::PhysicsLib::CheckCollide(
        *position,
        velocity,
        PhysicsLib::PhysicsLib::ShapeType::Sphere,
        &resolvedPosition,
        &resolvedVelocity,
        nullptr,
        nullptr,
        radius,
        0.0f);
    *position = resolvedPosition;
    return collided;
}

void EnemyBase::PlaySpecialAttackAnimation(NSRender::Render& render,
                                           const std::wstring& animationName)
{
    if (m_meshId < 0)
    {
        return;
    }

    render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
    render.PlayMeshMixSkinAnimAnimation(m_meshId, animationName);
}

void EnemyBase::FinishSpecialAttack()
{
    m_animationNeedsRefresh = true;
}

void EnemyBase::EmitAttackHit(const int damage,
                              const D3DXVECTOR3& sourcePosition,
                              const int knockbackFrames,
                              const int slowFrames)
{
    if (m_hasPendingAttackHit)
    {
        return;
    }

    m_pendingAttackHit.damage = damage;
    m_pendingAttackHit.sourcePosition = sourcePosition;
    m_pendingAttackHit.knockbackFrames = knockbackFrames;
    m_pendingAttackHit.slowFrames = slowFrames;
    m_hasPendingAttackHit = true;
}

void EnemyBase::TakeDamage(NSRender::Render& render, int amount, const D3DXVECTOR3& attackerPos)
{
    if (m_state == State::Dead)
    {
        return;
    }

    ApplyDamage(render, amount);
    if (m_state == State::Dead)
    {
        return;
    }

    BeginAlert(attackerPos, true);
}

void EnemyBase::TakeDamageWithoutFacing(NSRender::Render& render, const int amount)
{
    if (m_state == State::Dead)
    {
        return;
    }

    ApplyDamage(render, amount);
    if (m_state == State::Dead)
    {
        return;
    }

    BeginAlert(m_lastKnownPlayerPosition, false);
}

void EnemyBase::ApplyDamage(NSRender::Render& render, const int amount)
{
    m_hp -= amount;
    render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage, m_position);
    m_blinkFrames = 15;
    if (m_meshId >= 0)
    {
        render.StartMeshMixSkinAnimBlink(m_meshId, m_blinkFrames, 2);
    }

    if (m_hp <= 0)
    {
        m_hp = 0;
        StartDeath(render);
    }
    else if (m_hitReactionMode == HitReactionMode::Normal)
    {
        if (m_usesExtendedAnimations)
        {
            m_hitStunFrames = 12;
            if (m_meshId >= 0)
            {
                render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
                render.PlayMeshMixSkinAnimAnimation(m_meshId, L"hit");
            }
        }
        else
        {
            m_hitStunFrames = kHitStunFrames;
            if (m_meshId >= 0)
            {
                render.SetMeshMixSkinAnimSpeed(m_meshId, 0.0f);
            }
        }
    }
}

void EnemyBase::StartDeath(NSRender::Render& render)
{
    m_state = State::Dead;
    m_pendingAttackHit = AttackHit();
    m_hasPendingAttackHit = false;
    m_hitStunFrames = 0;
    m_facePlayerTurnFrames = 0;
    m_knockbackFrames = 0;
    m_verticalVelocity = 0.0f;
    m_isGrounded = false;
    m_removalFrames = 30;
    if (m_meshId >= 0)
    {
        if (m_usesExtendedAnimations)
        {
            m_removalFrames = 45;
            render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
            render.PlayMeshMixSkinAnimAnimation(m_meshId, L"death");
        }
        else
        {
            render.StartMeshMixSkinAnimBlink(m_meshId, m_removalFrames, 2);
        }
    }
}

bool EnemyBase::IsDead() const
{
    return m_state == State::Dead;
}

bool EnemyBase::IsReadyToRemove() const
{
    return m_state == State::Dead && m_removalFrames <= 0;
}

void EnemyBase::MarkAttackedPlayer(NSRender::Render& render)
{
    if (m_state != State::Dead)
    {
        m_state = State::Retreat;
        m_facePlayerTurnFrames = 0;
        m_alertFrames = 0;
        m_retreatFrames = NextRandomInt(20, 36);

        D3DXVECTOR3 awayDir = m_position - m_lastKnownPlayerPosition;
        awayDir.y = 0.0f;
        if (D3DXVec3LengthSq(&awayDir) > 0.0001f)
        {
            D3DXVec3Normalize(&awayDir, &awayDir);
        }
        else
        {
            awayDir = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
        }

        D3DXVECTOR3 sideDir(-awayDir.z, 0.0f, awayDir.x);
        float sideSign = 1.0f;
        if (NextRandom01() < 0.5f)
        {
            sideSign = -1.0f;
        }
        D3DXVECTOR3 retreatDir = awayDir + sideDir * (0.35f * sideSign);
        retreatDir.y = 0.0f;
        if (D3DXVec3LengthSq(&retreatDir) > 0.0001f)
        {
            D3DXVec3Normalize(&retreatDir, &retreatDir);
        }
        if (m_movementMode == MovementMode::Hover ||
            m_movementMode == MovementMode::Swoop)
        {
            retreatDir.y = 0.65f;
            D3DXVec3Normalize(&retreatDir, &retreatDir);
        }
        m_retreatDirection = retreatDir;

        if (m_usesExtendedAnimations && m_meshId >= 0)
        {
            render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
            render.PlayMeshMixSkinAnimAnimation(m_meshId, L"attack");
            m_forcedAnimationFrames = 18;
        }
    }
}

int EnemyBase::GetHp() const
{
    return m_hp;
}

int EnemyBase::GetMaxHp() const
{
    return m_maxHp;
}

D3DXVECTOR3 EnemyBase::GetPosition() const
{
    return m_position;
}

D3DXVECTOR3 EnemyBase::GetSpawnPosition() const
{
    D3DXVECTOR3 pos = m_position;
    // 地上敵はコンストラクタで足元基準→円柱中心へ変換しているため、
    // 同じ量を引いて足元基準のスポーン座標へ戻す。
    if (m_movementMode == MovementMode::Ground ||
        m_movementMode == MovementMode::Frog)
    {
        pos.y -= m_height * 0.5f;
    }
    return pos;
}

void EnemyBase::SetPosition(const D3DXVECTOR3& pos)
{
    m_position = pos;
}

void EnemyBase::SetSpawnPosition(const D3DXVECTOR3& pos)
{
    m_position = pos;
    // コンストラクタと同じ変換で、足元基準の座標を円柱中心座標へ変換する。
    if (m_movementMode == MovementMode::Ground ||
        m_movementMode == MovementMode::Frog)
    {
        m_position.y += m_height * 0.5f;
    }
}

void EnemyBase::StartKnockbackFrom(const D3DXVECTOR3& sourcePosition,
                                   const float distance,
                                   const int durationFrames)
{
    if (m_hitReactionMode == HitReactionMode::SuperArmor)
    {
        return;
    }

    if (distance <= 0.0f || durationFrames <= 0)
    {
        return;
    }

    D3DXVECTOR3 direction = m_position - sourcePosition;
    direction.y = 0.0f;
    if (D3DXVec3LengthSq(&direction) <= 0.0001f)
    {
        return;
    }

    D3DXVec3Normalize(&direction, &direction);
    m_knockbackPerFrame = direction * (distance / static_cast<float>(durationFrames));
    m_knockbackFrames = durationFrames;
}

void EnemyBase::StartIdleBehavior()
{
    m_idleWaitFrames = NextRandomInt(20, 70);
    m_idleMoveFrames = 0;
    m_idleMoveYaw = m_yaw;
}

void EnemyBase::UpdateIdleBehavior()
{
    if (m_idleWaitFrames > 0)
    {
        --m_idleWaitFrames;
        if (m_idleWaitFrames <= 0)
        {
            m_idleMoveFrames = NextRandomInt(25, 55);
            m_idleMoveYaw = m_yaw + (NextRandom01() * 1.2f - 0.6f);
        }
        return;
    }

    if (m_idleMoveFrames > 0)
    {
        --m_idleMoveFrames;
        UpdateFacing(m_position + D3DXVECTOR3(-sinf(m_idleMoveYaw), 0.0f, -cosf(m_idleMoveYaw)));
        D3DXVECTOR3 moveDir(-sinf(m_yaw), 0.0f, -cosf(m_yaw));
        if (m_movementMode == MovementMode::Frog)
        {
            UpdateFrogMovement(moveDir, 0.55f);
        }
        else
        {
            MoveWithCollision(moveDir * (m_moveSpeed * 0.18f));
        }

        D3DXVECTOR3 fromHome = m_position - m_homePosition;
        fromHome.y = 0.0f;
        if (D3DXVec3LengthSq(&fromHome) > 16.0f)
        {
            FaceTargetImmediately(m_homePosition);
        }

        if (m_idleMoveFrames <= 0)
        {
            StartIdleBehavior();
        }
        return;
    }

    StartIdleBehavior();
}

void EnemyBase::BeginAlert(const D3DXVECTOR3& playerPos, const bool faceImmediately)
{
    if (m_state == State::Dead)
    {
        return;
    }

    m_lastKnownPlayerPosition = playerPos;
    m_lastKnownPlayerFrames = kLastKnownPlayerFrames;
    m_state = State::Alert;
    m_alertFrames = kAlertFrames;
    m_idleWaitFrames = 0;
    m_idleMoveFrames = 0;
    m_chaseStrafeFrames = 0;
    if (faceImmediately)
    {
        FaceTargetImmediately(playerPos);
        m_facePlayerTurnFrames = 0;
    }
    else
    {
        StartFacePlayerTurn();
    }
}

void EnemyBase::UpdateChaseBehavior(const D3DXVECTOR3& playerPos, const bool playerInvincible)
{
    if (m_movementMode == MovementMode::Hover ||
        m_movementMode == MovementMode::Swoop)
    {
        UpdateFlyingChaseBehavior(playerPos, playerInvincible);
        return;
    }

    bool canSeePlayer = false;
    if (!playerInvincible)
    {
        const D3DXVECTOR3 diff = playerPos - m_position;
        const float distance = D3DXVec3Length(&diff);
        if (distance <= m_viewDistance && IsPlayerInView(playerPos))
        {
            canSeePlayer = true;
        }
    }

    if (canSeePlayer)
    {
        m_lastKnownPlayerPosition = playerPos;
        m_lastKnownPlayerFrames = kLastKnownPlayerFrames;
    }
    else if (m_lastKnownPlayerFrames <= 0)
    {
        m_state = State::Idle;
        StartIdleBehavior();
        return;
    }

    D3DXVECTOR3 moveTarget = m_lastKnownPlayerPosition;
    D3DXVECTOR3 toTarget = moveTarget - m_position;
    toTarget.y = 0.0f;
    const float distance = D3DXVec3Length(&toTarget);
    if (distance <= 0.0001f)
    {
        if (!canSeePlayer)
        {
            m_state = State::Idle;
            StartIdleBehavior();
        }
        return;
    }

    D3DXVECTOR3 forwardDir = toTarget;
    D3DXVec3Normalize(&forwardDir, &forwardDir);
    D3DXVECTOR3 sideDir(-forwardDir.z, 0.0f, forwardDir.x);

    if (m_chaseStrafeFrames > 0)
    {
        --m_chaseStrafeFrames;
    }
    else
    {
        m_chaseStrafeFrames = NextRandomInt(30, 75);
        m_chaseStrafeDirection = 1.0f;
        if (NextRandom01() < 0.5f)
        {
            m_chaseStrafeDirection = -1.0f;
        }
    }

    float strafeWeight = 0.0f;
    if (distance < 5.0f && distance > 1.0f)
    {
        strafeWeight = 0.18f + fabsf(m_personalityBias) * 0.12f;
        if (distance < 2.4f)
        {
            strafeWeight += 0.12f;
        }
        strafeWeight *= m_chaseStrafeDirection;
    }

    D3DXVECTOR3 moveDir = forwardDir + sideDir * strafeWeight;
    moveDir.y = 0.0f;
    if (D3DXVec3LengthSq(&moveDir) > 0.0001f)
    {
        D3DXVec3Normalize(&moveDir, &moveDir);
    }
    else
    {
        moveDir = forwardDir;
    }

    UpdateFacing(m_position + moveDir);

    float speedMultiplier = 1.0f;
    if (distance > 3.0f)
    {
        speedMultiplier = 1.0f;
    }
    else if (distance > 2.0f)
    {
        speedMultiplier = 0.58f;
    }
    else
    {
        speedMultiplier = 0.32f;
    }

    if (m_movementMode == MovementMode::Frog)
    {
        UpdateFrogMovement(moveDir, speedMultiplier);
    }
    else
    {
        MoveWithCollision(moveDir * (m_moveSpeed * speedMultiplier));
    }
}

void EnemyBase::UpdateRetreatBehavior(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    UpdateFacing(playerPos);
    if (m_meshId >= 0)
    {
        render.SetMeshMixSkinAnimSpeed(m_meshId, -1.0f);
    }
    MoveWithCollision(m_retreatDirection * (m_moveSpeed * 0.42f));

    if (m_retreatFrames > 0)
    {
        --m_retreatFrames;
    }

    if (m_retreatFrames <= 0)
    {
        if (m_meshId >= 0)
        {
            render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
        }
        BeginAlert(m_lastKnownPlayerPosition, false);
    }
}

void EnemyBase::UpdateFlyingIdleBehavior()
{
    float hoverAmplitude = 0.25f;
    float hoverSpeed = 0.045f;
    if (m_movementMode == MovementMode::Swoop)
    {
        hoverAmplitude = 0.4f;
        hoverSpeed = 0.065f;
    }

    D3DXVECTOR3 target = m_homePosition;
    target.y += sinf(static_cast<float>(m_flightFrame) * hoverSpeed) * hoverAmplitude;
    D3DXVECTOR3 direction = target - m_position;
    const float distance = D3DXVec3Length(&direction);
    if (distance > 0.01f)
    {
        D3DXVec3Normalize(&direction, &direction);
        float speed = m_moveSpeed * 0.35f;
        if (distance < 0.5f)
        {
            speed *= distance / 0.5f;
        }
        MoveWithCollision(direction * speed);
    }
}

void EnemyBase::UpdateFlyingChaseBehavior(const D3DXVECTOR3& playerPos,
                                          const bool playerInvincible)
{
    bool canSeePlayer = false;
    if (!playerInvincible)
    {
        const D3DXVECTOR3 difference = playerPos - m_position;
        const float distance = D3DXVec3Length(&difference);
        if (distance <= m_viewDistance && IsPlayerInView(playerPos))
        {
            canSeePlayer = true;
        }
    }

    if (canSeePlayer)
    {
        m_lastKnownPlayerPosition = playerPos;
        m_lastKnownPlayerFrames = kLastKnownPlayerFrames;
    }
    else if (m_lastKnownPlayerFrames <= 0)
    {
        m_state = State::Idle;
        return;
    }

    D3DXVECTOR3 moveTarget = m_lastKnownPlayerPosition;
    moveTarget.y += 0.8f;
    float speedMultiplier = 0.75f;
    if (m_movementMode == MovementMode::Swoop)
    {
        moveTarget.y += 0.15f;
        speedMultiplier = 1.0f;
    }

    const float minimumY = m_homePosition.y - 1.5f;
    const float maximumY = m_homePosition.y + 2.0f;
    if (moveTarget.y < minimumY)
    {
        moveTarget.y = minimumY;
    }
    if (moveTarget.y > maximumY)
    {
        moveTarget.y = maximumY;
    }

    D3DXVECTOR3 direction = moveTarget - m_position;
    if (D3DXVec3LengthSq(&direction) <= 0.0001f)
    {
        return;
    }
    D3DXVec3Normalize(&direction, &direction);
    UpdateFacing(moveTarget);
    MoveWithCollision(direction * (m_moveSpeed * speedMultiplier));
}

void EnemyBase::UpdateFrogMovement(const D3DXVECTOR3& moveDirection,
                                   const float speedMultiplier)
{
    if (m_isGrounded && m_frogJumpCooldownFrames <= 0 && !m_frogJumpActive)
    {
        m_frogJumpDirection = moveDirection;
        m_frogJumpDirection.y = 0.0f;
        if (D3DXVec3LengthSq(&m_frogJumpDirection) > 0.0001f)
        {
            D3DXVec3Normalize(&m_frogJumpDirection, &m_frogJumpDirection);
        }
        m_verticalVelocity = 4.5f;
        m_isGrounded = false;
        m_frogJumpActive = true;
    }

    if (m_frogJumpActive)
    {
        MoveWithCollision(m_frogJumpDirection * (m_moveSpeed * speedMultiplier));
    }
}

void EnemyBase::ApplyAnimation(NSRender::Render& render, const AnimState nextAnim)
{
    if (nextAnim == m_animState && !m_animationNeedsRefresh)
    {
        return;
    }

    m_animState = nextAnim;
    m_animationNeedsRefresh = false;
    if (m_meshId < 0)
    {
        return;
    }

    render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
    if (m_animState == AnimState::Run)
    {
        render.PlayMeshMixSkinAnimAnimation(m_meshId, L"run");
        return;
    }

    if (m_animState == AnimState::Walk)
    {
        render.PlayMeshMixSkinAnimAnimation(m_meshId, L"walk");
        return;
    }

    if (m_animState == AnimState::Creep)
    {
        render.PlayMeshMixSkinAnimAnimation(m_meshId, L"creep");
        return;
    }

    render.PlayMeshMixSkinAnimAnimation(m_meshId, L"idle");
}

float EnemyBase::GetYaw() const
{
    return m_yaw;
}

void EnemyBase::SetYaw(float yaw)
{
    m_yaw = yaw;
}

int EnemyBase::GetMeshId() const
{
    return m_meshId;
}

void EnemyBase::SetMeshId(const int meshId)
{
    m_meshId = meshId;
}

const std::wstring& EnemyBase::GetType() const
{
    return m_type;
}

void EnemyBase::SetType(const std::wstring& type)
{
    m_type = type;
}

void EnemyBase::SetBossName(const std::wstring& bossName)
{
    m_bossName = bossName;
}

bool EnemyBase::IsTouchingPlayer(const D3DXVECTOR3& playerPos) const
{
    if (m_state == State::Dead)
    {
        return false;
    }

    const D3DXVECTOR3 diff = playerPos - m_position;
    const float horizontalDist = sqrtf(diff.x * diff.x + diff.z * diff.z);
    const float verticalDist = fabsf(diff.y);

    float verticalTolerance = m_height * 0.75f;
    if (m_movementMode == MovementMode::Hover ||
        m_movementMode == MovementMode::Swoop)
    {
        // 飛行敵は空中から急降下してくるため、上下方向の接触許容を広く取る。
        // さもないと Swoop の高度差（約 0.8〜0.95m）が小さな m_height の許容
        // を常に上回り、接触攻撃が一切発動しなくなる。
        verticalTolerance = 1.5f;
    }

    return horizontalDist <= m_contactRadius && verticalDist <= verticalTolerance;
}

bool EnemyBase::IsStompedByPlayer(const D3DXVECTOR3& previousPlayerPos,
                                  const D3DXVECTOR3& playerPos,
                                  const bool playerIsJumping,
                                  const float playerYVelocity) const
{
    if (m_state == State::Dead)
    {
        return false;
    }

    if (!playerIsJumping)
    {
        return false;
    }

    const D3DXVECTOR3 diff = playerPos - m_position;
    const float horizontalDist = sqrtf(diff.x * diff.x + diff.z * diff.z);
    if (horizontalDist > m_contactRadius || playerYVelocity > 0.0f)
    {
        return false;
    }

    const float enemyTopY = m_position.y + m_height * 0.5f;
    const float verticalOffset = playerPos.y - enemyTopY;
    const bool isNearEnemyTop = verticalOffset >= 0.0f &&
        verticalOffset <= kStompMaxDistanceAboveEnemy;
    const bool crossedEnemyTop = previousPlayerPos.y >= enemyTopY && playerPos.y < enemyTopY;

    return isNearEnemyTop || crossedEnemyTop;
}

void EnemyBase::FaceTargetImmediately(const D3DXVECTOR3& targetPos)
{
    m_yaw = GetYawToTarget(m_position, targetPos);
}

void EnemyBase::StartFacePlayerTurn()
{
    m_facePlayerTurnFrames = kFacePlayerTurnFrames;
}

void EnemyBase::UpdateFacePlayerTurn(const D3DXVECTOR3& playerPos)
{
    const float targetYaw = GetYawToTarget(m_position, playerPos);
    float diff = targetYaw - m_yaw;
    while (diff > D3DX_PI)
    {
        diff -= 2.0f * D3DX_PI;
    }
    while (diff < -D3DX_PI)
    {
        diff += 2.0f * D3DX_PI;
    }

    if (m_facePlayerTurnFrames <= 1)
    {
        FaceTargetImmediately(playerPos);
        m_facePlayerTurnFrames = 0;
        return;
    }

    m_yaw += diff / static_cast<float>(m_facePlayerTurnFrames);
    --m_facePlayerTurnFrames;
}

void EnemyBase::UpdateFacing(const D3DXVECTOR3& targetPos)
{
    const float targetYaw = GetYawToTarget(m_position, targetPos);
    const float kTurnRadiansPerSecond = 10.0f;
    const float kTargetFrameSeconds = 1.0f / 60.0f;
    m_yaw = MoveAngleToward(m_yaw, targetYaw, kTurnRadiansPerSecond * kTargetFrameSeconds);
}

void EnemyBase::ApplyGravity(NSRender::Render& render)
{
    if (m_position.y < kEnemyFallDeathY)
    {
        StartDeath(render);
        return;
    }

    m_verticalVelocity -= kEnemyGravity * kTargetFrameSeconds;
    if (m_verticalVelocity < -kEnemyMaxFallSpeed)
    {
        m_verticalVelocity = -kEnemyMaxFallSpeed;
    }

    D3DXVECTOR3 hitNormal(0.0f, 0.0f, 0.0f);
    const bool collided = MoveWithCollision(D3DXVECTOR3(0.0f, m_verticalVelocity, 0.0f), &hitNormal);
    m_isGrounded = false;
    if (collided && hitNormal.y > kGroundNormalYThreshold && m_verticalVelocity <= 0.0f)
    {
        m_isGrounded = true;
        m_verticalVelocity = 0.0f;
        if (m_movementMode == MovementMode::Frog && m_frogJumpActive)
        {
            m_frogJumpActive = false;
            m_frogJumpCooldownFrames = 24;
            m_animationNeedsRefresh = true;
        }
    }

    if (m_position.y < kEnemyFallDeathY)
    {
        StartDeath(render);
    }
}

bool EnemyBase::MoveWithCollision(const D3DXVECTOR3& velocity, D3DXVECTOR3* outHitNormal)
{
    if (D3DXVec3LengthSq(&velocity) <= 0.0001f)
    {
        if (outHitNormal != nullptr)
        {
            *outHitNormal = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }
        return false;
    }

    D3DXVECTOR3 resolvedPosition = m_position;
    D3DXVECTOR3 resolvedVelocity = velocity;
    D3DXVECTOR3 hitNormal(0.0f, 0.0f, 0.0f);
    const float radius = m_physicsRadius;
    const float height = m_height;

    const bool collided = PhysicsLib::PhysicsLib::CheckCollide(m_position,
                                                               velocity,
                                                               PhysicsLib::PhysicsLib::ShapeType::Cylinder,
                                                               &resolvedPosition,
                                                               &resolvedVelocity,
                                                               nullptr,
                                                               nullptr,
                                                               radius,
                                                               height,
                                                               nullptr,
                                                               &hitNormal,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr,
                                                               nullptr);

    m_position = resolvedPosition;
    if (outHitNormal != nullptr)
    {
        *outHitNormal = hitNormal;
    }
    return collided;
}

bool EnemyBase::IsPlayerInView(const D3DXVECTOR3& playerPos) const
{
    const D3DXVECTOR3 forward(-sinf(m_yaw), 0.0f, -cosf(m_yaw));
    D3DXVECTOR3 toPlayer = playerPos - m_position;
    toPlayer.y = 0.0f;

    if (D3DXVec3LengthSq(&toPlayer) < 0.0001f)
    {
        return true;
    }

    D3DXVec3Normalize(&toPlayer, &toPlayer);
    const float dot = D3DXVec3Dot(&forward, &toPlayer);
    const float angle = acosf(ClampFloat(dot, -1.0f, 1.0f));
    return angle <= m_viewHalfAngle;
}

float EnemyBase::NextRandom01()
{
    m_behaviorSeed = m_behaviorSeed * 1664525u + 1013904223u;
    const unsigned int value = (m_behaviorSeed >> 8) & 0x00ffffffu;
    return static_cast<float>(value) / static_cast<float>(0x01000000u);
}

int EnemyBase::NextRandomInt(const int minValueInclusive, const int maxValueInclusive)
{
    if (maxValueInclusive <= minValueInclusive)
    {
        return minValueInclusive;
    }

    const int range = maxValueInclusive - minValueInclusive + 1;
    const float scaled = NextRandom01() * static_cast<float>(range);
    int value = minValueInclusive + static_cast<int>(scaled);
    if (value > maxValueInclusive)
    {
        value = maxValueInclusive;
    }
    return value;
}
