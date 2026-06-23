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
        Alert,
        Chase,
        Retreat,
        Dead
    };

    enum class AnimState
    {
        Idle,
        Walk,
        Creep,
        Run
    };

    Enemy();

    void Initialize(const D3DXVECTOR3& startPosition,
                    int meshId,
                    const std::wstring& type,
                    float yaw,
                    int maxHp,
                    float moveSpeed,
                    float viewDistance,
                    float contactRadius,
                    float height);
    void Update(NSRender::Render& render, const D3DXVECTOR3& playerPos, bool playerInvincible);
    void SyncMesh(NSRender::Render& render);

    void TakeDamage(NSRender::Render& render, int amount, const D3DXVECTOR3& attackerPos);
    bool IsDead() const;
    bool IsReadyToRemove() const;
    void MarkAttackedPlayer();
    int GetHp() const;
    int GetMaxHp() const;
    D3DXVECTOR3 GetPosition() const;
    void SetPosition(const D3DXVECTOR3& pos);
    void StartKnockbackFrom(const D3DXVECTOR3& sourcePosition, float distance, int durationFrames);
    float GetYaw() const;
    void SetYaw(float yaw);
    int GetMeshId() const;
    void SetMeshId(int meshId);
    const std::wstring& GetType() const;
    void SetType(const std::wstring& type);

    bool IsTouchingPlayer(const D3DXVECTOR3& playerPos) const;
    bool IsStompedByPlayer(const D3DXVECTOR3& previousPlayerPos,
                           const D3DXVECTOR3& playerPos,
                           bool playerIsJumping,
                           float playerYVelocity) const;

private:
    void StartIdleBehavior();
    void UpdateIdleBehavior();
    void BeginAlert(const D3DXVECTOR3& playerPos, bool faceImmediately);
    void UpdateChaseBehavior(const D3DXVECTOR3& playerPos, bool playerInvincible);
    void UpdateRetreatBehavior();
    void ApplyAnimation(NSRender::Render& render, AnimState nextAnim);
    void FaceTargetImmediately(const D3DXVECTOR3& targetPos);
    void StartFacePlayerTurn();
    void UpdateFacePlayerTurn(const D3DXVECTOR3& playerPos);
    void UpdateFacing(const D3DXVECTOR3& targetPos);
    bool IsPlayerInView(const D3DXVECTOR3& playerPos) const;
    float NextRandom01();
    int NextRandomInt(int minValueInclusive, int maxValueInclusive);

    D3DXVECTOR3 m_position;
    D3DXVECTOR3 m_homePosition;
    D3DXVECTOR3 m_lastKnownPlayerPosition;
    D3DXVECTOR3 m_retreatDirection = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    float m_yaw = 0.0f;
    int m_hp = 10;
    int m_maxHp = 10;
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
    int m_hitStunFrames = 0;
    D3DXVECTOR3 m_knockbackPerFrame = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    int m_knockbackFrames = 0;
    int m_removalFrames = 0;
    int m_facePlayerTurnFrames = 0;
    int m_alertFrames = 0;
    int m_idleWaitFrames = 0;
    int m_idleMoveFrames = 0;
    int m_lastKnownPlayerFrames = 0;
    int m_chaseStrafeFrames = 0;
    int m_retreatFrames = 0;
    float m_idleMoveYaw = 0.0f;
    float m_chaseStrafeDirection = 1.0f;
    float m_personalityBias = 0.0f;
    unsigned int m_behaviorSeed = 1;
};
