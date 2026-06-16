#include "Enemy.h"

#include "../../RedFortressRender/Render/Render.h"

namespace
{
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
        return current + (diff > 0.0f ? maxDelta : -maxDelta);
    }
}

Enemy::Enemy()
    : m_position(0.0f, 0.0f, 0.0f)
{
}

void Enemy::Initialize(const D3DXVECTOR3& startPosition, int meshId)
{
    m_position = startPosition;
    m_meshId = meshId;
    m_hp = 10;
    m_state = State::Idle;
    m_animState = AnimState::Idle;
    m_yaw = 0.0f;
    m_invincibleFrames = 0;
}

void Enemy::Update(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    if (m_state == State::Dead)
    {
        return;
    }

    if (m_invincibleFrames > 0)
    {
        --m_invincibleFrames;
    }

    const D3DXVECTOR3 diff = playerPos - m_position;
    const float distance = D3DXVec3Length(&diff);

    if (distance <= m_viewDistance && IsPlayerInView(playerPos))
    {
        m_state = State::Chase;
    }
    else
    {
        m_state = State::Idle;
    }

    if (m_state == State::Chase)
    {
        UpdateFacing(playerPos);

        D3DXVECTOR3 moveDir = diff;
        moveDir.y = 0.0f;
        if (D3DXVec3LengthSq(&moveDir) > 0.0001f)
        {
            D3DXVec3Normalize(&moveDir, &moveDir);
        }
        else
        {
            moveDir = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }

        const float kTargetFrameSeconds = 1.0f / 60.0f;
        m_position += moveDir * m_moveSpeed * kTargetFrameSeconds;
    }

    const AnimState nextAnim = (m_state == State::Chase) ? AnimState::Run : AnimState::Idle;
    if (nextAnim != m_animState)
    {
        m_animState = nextAnim;
        if (m_meshId >= 0)
        {
            if (m_animState == AnimState::Run)
            {
                render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
                render.PlayMeshMixSkinAnimAnimation(m_meshId, L"run");
            }
            else
            {
                render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
                render.PlayMeshMixSkinAnimAnimation(m_meshId, L"idle");
            }
        }
    }
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

void Enemy::TakeDamage(NSRender::Render& render, int amount)
{
    if (m_state == State::Dead)
    {
        return;
    }

    m_hp -= amount;
    m_invincibleFrames = 10;

    if (m_hp <= 0)
    {
        m_hp = 0;
        m_state = State::Dead;
        if (m_meshId >= 0)
        {
            render.SetMeshMixSkinAnimSpeed(m_meshId, 1.0f);
            render.PlayMeshMixSkinAnimAnimation(m_meshId, L"seat");
        }
    }
}

bool Enemy::IsDead() const
{
    return m_state == State::Dead;
}

D3DXVECTOR3 Enemy::GetPosition() const
{
    return m_position;
}

int Enemy::GetMeshId() const
{
    return m_meshId;
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

bool Enemy::IsStompedByPlayer(const D3DXVECTOR3& playerPos, bool playerIsJumping, float playerYVelocity) const
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
    const float verticalOffset = playerPos.y - (m_position.y + m_height * 0.5f);

    return horizontalDist <= m_contactRadius && verticalOffset >= 0.0f && playerYVelocity <= 0.0f;
}

void Enemy::UpdateFacing(const D3DXVECTOR3& targetPos)
{
    const D3DXVECTOR3 diff = targetPos - m_position;
    const float targetYaw = atan2f(-diff.x, -diff.z);
    const float kTurnRadiansPerSecond = 10.0f;
    const float kTargetFrameSeconds = 1.0f / 60.0f;
    m_yaw = MoveAngleToward(m_yaw, targetYaw, kTurnRadiansPerSecond * kTargetFrameSeconds);
}

bool Enemy::IsPlayerInView(const D3DXVECTOR3& playerPos) const
{
    const D3DXVECTOR3 forward(-sinf(m_yaw), 0.0f, cosf(m_yaw));
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
