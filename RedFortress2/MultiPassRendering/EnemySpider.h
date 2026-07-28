#pragma once
#include "EnemyBase.h"

class EnemySpider : public EnemyBase
{
public:
    EnemySpider(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.6f; }
    bool UsesSpecialAttacks() const override;

protected:
    bool UpdateSpecialAttack(NSRender::Render& render,
                             const D3DXVECTOR3& playerPos,
                             bool playerInvincible) override;

private:
    enum class AttackType
    {
        None,
        Bite,
        Pounce,
        WebShot,
        PoisonPool
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
    void UpdateWebProjectile(NSRender::Render& render,
                             const D3DXVECTOR3& playerPos,
                             bool playerInvincible);
    void UpdatePoisonPool(NSRender::Render& render,
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
    bool m_webProjectileActive = false;
    D3DXVECTOR3 m_webProjectilePosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_webProjectileDirection = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    int m_webProjectileFrames = 0;
    bool m_poisonPoolActive = false;
    D3DXVECTOR3 m_poisonPoolPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    int m_poisonPoolFrames = 0;
    int m_poisonDamageCooldownFrames = 0;
};