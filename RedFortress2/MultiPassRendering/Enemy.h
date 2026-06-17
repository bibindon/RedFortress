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
        Retreat,
        Dead
    };

    enum class AnimState
    {
        Idle,
        Run
    };

    Enemy();

    void Initialize(const D3DXVECTOR3& startPosition, int meshId, const std::wstring& type, float yaw);
    void Update(NSRender::Render& render, const D3DXVECTOR3& playerPos, bool playerInvincible);
    void SyncMesh(NSRender::Render& render);

    void TakeDamage(NSRender::Render& render, int amount);
    bool IsDead() const;
    bool IsReadyToRemove() const;
    void MarkAttackedPlayer();
    int GetHp() const;
    int GetMaxHp() const;
    D3DXVECTOR3 GetPosition() const;
    void SetPosition(const D3DXVECTOR3& pos);
    float GetYaw() const;
    void SetYaw(float yaw);
    int GetMeshId() const;
    const std::wstring& GetType() const;
    void SetType(const std::wstring& type);

    bool IsTouchingPlayer(const D3DXVECTOR3& playerPos) const;
    bool IsStompedByPlayer(const D3DXVECTOR3& playerPos, bool playerIsJumping, float playerYVelocity) const;

private:
    void FaceTargetImmediately(const D3DXVECTOR3& targetPos);
    void StartFacePlayerTurn();
    void UpdateFacePlayerTurn(const D3DXVECTOR3& playerPos);
    void UpdateFacing(const D3DXVECTOR3& targetPos);
    bool IsPlayerInView(const D3DXVECTOR3& playerPos) const;

    D3DXVECTOR3 m_position;
    float m_yaw = 0.0f;
    int m_hp = 10;
    int m_meshId = -1;
    std::wstring m_type;
    State m_state = State::Idle;
    AnimState m_animState = AnimState::Idle;

    float m_viewDistance = 5.0f;
    float m_viewHalfAngle = D3DXToRadian(90.0f);
    float m_moveSpeed = 2.5f;
    float m_retreatDistance = 3.0f;
    float m_contactRadius = 0.5f;
    float m_height = 1.0f;
    int m_blinkFrames = 0;
    int m_removalFrames = 0;
    int m_facePlayerTurnFrames = 0;
};
