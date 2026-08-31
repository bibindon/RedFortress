#pragma once
#include "EnemyBase.h"

class EnemyGiantCrab : public EnemyBase
{
public:
    EnemyGiantCrab(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.38f * 3.0f; }

protected:
    EnemyGiantCrab(const D3DXVECTOR3& pos, int meshId, float yaw, int maxHp);
};

class EnemyBossGiantCrab : public EnemyGiantCrab
{
public:
    EnemyBossGiantCrab(const D3DXVECTOR3& pos, int meshId, float yaw);
    bool UsesSpecialAttacks() const override;

protected:
    bool UpdateSpecialAttack(NSRender::Render& render,
                             const D3DXVECTOR3& playerPos,
                             bool playerInvincible) override;

private:
    static const int kBubbleProjectileCount = 3;

    enum class AttackType
    {
        None,
        ClawSweep,
        SideCharge,
        GroundSlam,
        BubbleShot,
        JumpSlam,
        RetreatDash
    };

    enum class AttackPhase
    {
        None,
        Windup,
        Active,
        Recovery
    };

    void SelectAttack(NSRender::Render& render, const D3DXVECTOR3& playerPos);
    bool IsAttackAllowed(AttackType attackType, float distance) const;
    void BeginAttack(NSRender::Render& render,
                     AttackType attackType,
                     const D3DXVECTOR3& playerPos);
    void BeginActivePhase(NSRender::Render& render, const D3DXVECTOR3& playerPos);
    void UpdateActivePhase(NSRender::Render& render,
                           const D3DXVECTOR3& playerPos,
                           bool playerInvincible);
    void SpawnBubbleProjectiles(NSRender::Render& render,
                                const D3DXVECTOR3& playerPos);
    void UpdateBubbleProjectiles(NSRender::Render& render,
                                 const D3DXVECTOR3& playerPos,
                                 bool playerInvincible);
    void BeginRecovery();
    void EndAttack();
    bool IsEnraged() const;

    AttackType m_attackType = AttackType::None;
    AttackPhase m_attackPhase = AttackPhase::None;
    int m_phaseFrames = 0;
    int m_attackCooldownFrames = 0;
    int m_nextAttackIndex = 0;
    bool m_attackHitApplied = false;
    bool m_chargeCollided = false;
    int m_attacksUntilRetreat = 2;
    int m_jumpFrame = 0;
    float m_jumpStartY = 0.0f;
    float m_jumpHorizontalSpeed = 0.0f;
    D3DXVECTOR3 m_lockedDirection = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    bool m_bubbleProjectileActive[kBubbleProjectileCount] = {};
    D3DXVECTOR3 m_bubbleProjectilePosition[kBubbleProjectileCount] = {};
    D3DXVECTOR3 m_bubbleProjectileDirection[kBubbleProjectileCount] = {};
    int m_bubbleProjectileFrames[kBubbleProjectileCount] = {};
};
