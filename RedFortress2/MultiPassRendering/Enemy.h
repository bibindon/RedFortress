#pragma once

#include <d3dx9.h>
#include <string>

namespace NSRender
{
class Render;
}

class Enemy
{
public:
    enum class State
    {
        Idle,
        Chase,
        Dead
    };

    enum class AnimState
    {
        Idle,
        Run
    };

    Enemy();

    void Initialize(const D3DXVECTOR3& startPosition, int meshId);
    void Update(NSRender::Render& render, const D3DXVECTOR3& playerPos);
    void SyncMesh(NSRender::Render& render);

    void TakeDamage(NSRender::Render& render, int amount);
    bool IsDead() const;
    D3DXVECTOR3 GetPosition() const;
    int GetMeshId() const;

    bool IsTouchingPlayer(const D3DXVECTOR3& playerPos) const;
    bool IsStompedByPlayer(const D3DXVECTOR3& playerPos, bool playerIsJumping, float playerYVelocity) const;

private:
    void UpdateFacing(const D3DXVECTOR3& targetPos);
    bool IsPlayerInView(const D3DXVECTOR3& playerPos) const;

    D3DXVECTOR3 m_position;
    float m_yaw = 0.0f;
    int m_hp = 10;
    int m_meshId = -1;
    State m_state = State::Idle;
    AnimState m_animState = AnimState::Idle;

    float m_viewDistance = 5.0f;
    float m_viewHalfAngle = D3DXToRadian(90.0f);
    float m_moveSpeed = 2.5f;
    float m_contactRadius = 0.5f;
    float m_height = 1.0f;
    int m_invincibleFrames = 0;
};
