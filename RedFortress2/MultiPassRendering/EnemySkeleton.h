#pragma once
#include "EnemyBase.h"

class EnemySkeleton : public EnemyBase
{
public:
    EnemySkeleton(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.0f; }
    bool UsesSpecialAttacks() const override;

protected:
    bool UpdateSpecialAttack(NSRender::Render& render,
                             const D3DXVECTOR3& playerPos,
                             bool playerInvincible) override;

private:
    enum class AttackType
    {
        None,
        Slash,
        Smash,
        Charge,
        BoneShot
    };

    enum class AttackPhase
    {
        None,
        Windup,
        Active,
        Recovery
    };

    void SelectAttack(NSRender::Render& render, const D3DXVECTOR3& playerPos);
    void BeginAttack(NSRender::Render& render,
                     AttackType attackType,
                     const D3DXVECTOR3& playerPos);
    void BeginActivePhase(NSRender::Render& render, const D3DXVECTOR3& playerPos);
    void UpdateActivePhase(const D3DXVECTOR3& playerPos, bool playerInvincible);
    void UpdateBoneProjectile(NSRender::Render& render,
                              const D3DXVECTOR3& playerPos,
                              bool playerInvincible);
    void BeginRecovery();
    void EndAttack();
    bool IsAttackAllowed(AttackType attackType, float distance) const;

    AttackType m_attackType = AttackType::None;
    AttackPhase m_attackPhase = AttackPhase::None;
    int m_phaseFrames = 0;
    int m_attackCooldownFrames = 0;
    int m_nextAttackIndex = 0;
    bool m_attackHitApplied = false;
    D3DXVECTOR3 m_lockedDirection = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    bool m_boneProjectileActive = false;
    D3DXVECTOR3 m_boneProjectilePosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_boneProjectileDirection = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    int m_boneProjectileFrames = 0;
};