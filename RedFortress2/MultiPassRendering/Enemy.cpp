#include "Enemy.h"

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
}

Enemy::Enemy()
    : m_position(0.0f, 0.0f, 0.0f)
{
}

void Enemy::Initialize(const D3DXVECTOR3& startPosition,
                       const int meshId,
                       const std::wstring& type,
                       const float yaw,
                       const int maxHp,
                       const float moveSpeed,
                       const float viewDistance,
                       const float contactRadius,
                       const float height)
{
    m_position = startPosition;
    m_homePosition = startPosition;
    m_lastKnownPlayerPosition = startPosition;
    m_meshId = meshId;
    m_type = type;
    m_maxHp = maxHp;
    m_hp = m_maxHp;
    m_moveSpeed = moveSpeed;
    m_viewDistance = viewDistance;
    m_contactRadius = contactRadius;
    m_height = height;
    m_state = State::Idle;
    m_animState = AnimState::Idle;
    m_yaw = yaw;
    m_blinkFrames = 0;
    m_hitStunFrames = 0;
    m_knockbackPerFrame = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    m_knockbackFrames = 0;
    m_removalFrames = 0;
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

void Enemy::Update(NSRender::Render& render, const D3DXVECTOR3& playerPos, bool playerInvincible)
{
    AnimState nextAnim = AnimState::Idle;

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
        }
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
        UpdateRetreatBehavior();
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
            UpdateIdleBehavior();
            if (m_idleMoveFrames > 0)
            {
                nextAnim = AnimState::Walk;
            }
        }
    }

    ApplyAnimation(render, nextAnim);
}

void Enemy::SyncMesh(NSRender::Render& render)
{
    if (m_meshId < 0)
    {
        return;
    }

    render.SetMeshMixSkinAnimPos(m_meshId, m_position);
    render.SetMeshMixSkinAnimRotY(m_meshId, m_yaw);
}

void Enemy::TakeDamage(NSRender::Render& render, int amount, const D3DXVECTOR3& attackerPos)
{
    if (m_state == State::Dead)
    {
        return;
    }

    ApplyDamage(render, amount);
    BeginAlert(attackerPos, true);
}

void Enemy::TakeDamageWithoutFacing(NSRender::Render& render, const int amount)
{
    if (m_state == State::Dead)
    {
        return;
    }

    ApplyDamage(render, amount);
    BeginAlert(m_lastKnownPlayerPosition, false);
}

void Enemy::ApplyDamage(NSRender::Render& render, const int amount)
{
    m_hp -= amount;
    m_blinkFrames = 15;
    if (m_meshId >= 0)
    {
        render.StartMeshMixSkinAnimBlink(m_meshId, m_blinkFrames, 2);
    }

    if (m_hp <= 0)
    {
        m_hp = 0;
        m_state = State::Dead;
        m_hitStunFrames = 0;
        m_facePlayerTurnFrames = 0;
        m_removalFrames = 30;
        if (m_meshId >= 0)
        {
            render.StartMeshMixSkinAnimBlink(m_meshId, m_removalFrames, 2);
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

bool Enemy::IsDead() const
{
    return m_state == State::Dead;
}

bool Enemy::IsReadyToRemove() const
{
    return m_state == State::Dead && m_removalFrames <= 0;
}

void Enemy::MarkAttackedPlayer()
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
        m_retreatDirection = retreatDir;
    }
}

int Enemy::GetHp() const
{
    return m_hp;
}

int Enemy::GetMaxHp() const
{
    return m_maxHp;
}

D3DXVECTOR3 Enemy::GetPosition() const
{
    return m_position;
}

void Enemy::SetPosition(const D3DXVECTOR3& pos)
{
    m_position = pos;
}

void Enemy::StartKnockbackFrom(const D3DXVECTOR3& sourcePosition,
                               const float distance,
                               const int durationFrames)
{
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

void Enemy::StartIdleBehavior()
{
    m_idleWaitFrames = NextRandomInt(20, 70);
    m_idleMoveFrames = 0;
    m_idleMoveYaw = m_yaw;
}

void Enemy::UpdateIdleBehavior()
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
        MoveWithCollision(moveDir * (m_moveSpeed * 0.18f));

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

void Enemy::BeginAlert(const D3DXVECTOR3& playerPos, const bool faceImmediately)
{
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

void Enemy::UpdateChaseBehavior(const D3DXVECTOR3& playerPos, const bool playerInvincible)
{
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

    MoveWithCollision(moveDir * (m_moveSpeed * speedMultiplier));
}

void Enemy::UpdateRetreatBehavior()
{
    UpdateFacing(m_position + m_retreatDirection);
    MoveWithCollision(m_retreatDirection * (m_moveSpeed * 0.42f));

    if (m_retreatFrames > 0)
    {
        --m_retreatFrames;
    }

    if (m_retreatFrames <= 0)
    {
        BeginAlert(m_lastKnownPlayerPosition, false);
    }
}

void Enemy::ApplyAnimation(NSRender::Render& render, const AnimState nextAnim)
{
    if (nextAnim == m_animState)
    {
        return;
    }

    m_animState = nextAnim;
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

float Enemy::GetYaw() const
{
    return m_yaw;
}

void Enemy::SetYaw(float yaw)
{
    m_yaw = yaw;
}

int Enemy::GetMeshId() const
{
    return m_meshId;
}

void Enemy::SetMeshId(const int meshId)
{
    m_meshId = meshId;
}

const std::wstring& Enemy::GetType() const
{
    return m_type;
}

void Enemy::SetType(const std::wstring& type)
{
    m_type = type;
}

bool Enemy::IsTouchingPlayer(const D3DXVECTOR3& playerPos) const
{
    if (m_state == State::Dead)
    {
        return false;
    }

    const D3DXVECTOR3 diff = playerPos - m_position;
    const float horizontalDist = sqrtf(diff.x * diff.x + diff.z * diff.z);
    const float verticalDist = fabsf(diff.y);

    return horizontalDist <= m_contactRadius && verticalDist <= m_height * 0.75f;
}

bool Enemy::IsStompedByPlayer(const D3DXVECTOR3& previousPlayerPos,
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

void Enemy::FaceTargetImmediately(const D3DXVECTOR3& targetPos)
{
    m_yaw = GetYawToTarget(m_position, targetPos);
}

void Enemy::StartFacePlayerTurn()
{
    m_facePlayerTurnFrames = kFacePlayerTurnFrames;
}

void Enemy::UpdateFacePlayerTurn(const D3DXVECTOR3& playerPos)
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

void Enemy::UpdateFacing(const D3DXVECTOR3& targetPos)
{
    const float targetYaw = GetYawToTarget(m_position, targetPos);
    const float kTurnRadiansPerSecond = 10.0f;
    const float kTargetFrameSeconds = 1.0f / 60.0f;
    m_yaw = MoveAngleToward(m_yaw, targetYaw, kTurnRadiansPerSecond * kTargetFrameSeconds);
}

void Enemy::MoveWithCollision(const D3DXVECTOR3& velocity)
{
    if (D3DXVec3LengthSq(&velocity) <= 0.0001f)
    {
        return;
    }

    D3DXVECTOR3 resolvedPosition = m_position;
    D3DXVECTOR3 resolvedVelocity = velocity;
    const float radius = m_contactRadius;
    const float height = m_height;

    PhysicsLib::PhysicsLib::CheckCollide(m_position,
                                         velocity,
                                         PhysicsLib::PhysicsLib::ShapeType::Cylinder,
                                         &resolvedPosition,
                                         &resolvedVelocity,
                                         nullptr,
                                         nullptr,
                                         radius,
                                         height,
                                         nullptr,
                                         nullptr,
                                         nullptr,
                                         nullptr,
                                         nullptr,
                                         nullptr,
                                         nullptr,
                                         nullptr);

    m_position = resolvedPosition;
}

bool Enemy::IsPlayerInView(const D3DXVECTOR3& playerPos) const
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

float Enemy::NextRandom01()
{
    m_behaviorSeed = m_behaviorSeed * 1664525u + 1013904223u;
    const unsigned int value = (m_behaviorSeed >> 8) & 0x00ffffffu;
    return static_cast<float>(value) / static_cast<float>(0x01000000u);
}

int Enemy::NextRandomInt(const int minValueInclusive, const int maxValueInclusive)
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
