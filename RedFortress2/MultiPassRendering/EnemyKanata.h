#pragma once

#include "EnemyBase.h"

class EnemyKanata : public EnemyBase
{
public:
    EnemyKanata(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.0f; }
    bool IsBoss() const override { return true; }
    std::wstring GetBossName() const override { return L"天音かなた"; }
    float GetAttackTargetHeightOffset() const override { return 0.0f; }
    bool UsesSpecialAttacks() const override;

protected:
    bool UpdateSpecialAttack(NSRender::Render& render,
                             const D3DXVECTOR3& playerPos,
                             bool playerInvincible) override;
    static float GetCollisionHeight() { return 3.0f; }

    // The prepared model follows the standard Blender -Y facing convention. Do not rotate it by 180 degrees.
    float GetMeshYawOffset() const override { return 0.0f; }

private:
    static const int kBarrageProjectileCount = 5;

    enum class AttackType
    {
        None,
        Hammer,
        Sweep,
        Barrage,
        Dive
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
    void UpdateActivePhase(NSRender::Render& render,
                           const D3DXVECTOR3& playerPos,
                           bool playerInvincible);
    void UpdateBarrageProjectiles(NSRender::Render& render,
                                  const D3DXVECTOR3& playerPos,
                                  bool playerInvincible);
    void SpawnBarrageProjectile(NSRender::Render& render,
                                const D3DXVECTOR3& playerPos);
    void BeginRecovery(NSRender::Render& render,
                       const D3DXVECTOR3& playerPos,
                       bool playerInvincible);
    void EndAttack();
    bool IsAttackAllowed(AttackType attackType, float distance) const;

    AttackType m_attackType = AttackType::None;
    AttackPhase m_attackPhase = AttackPhase::None;
    int m_phaseFrames = 0;
    int m_attackCooldownFrames = 0;
    int m_nextAttackIndex = 0;
    int m_barrageActiveFrames = 0;
    int m_barrageShotsFired = 0;
    bool m_attackHitApplied = false;
    bool m_diveCollided = false;
    D3DXVECTOR3 m_lockedDirection = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    bool m_barrageProjectileActive[kBarrageProjectileCount] = {};
    D3DXVECTOR3 m_barrageProjectilePosition[kBarrageProjectileCount] = {};
    D3DXVECTOR3 m_barrageProjectileDirection[kBarrageProjectileCount] = {};
    int m_barrageProjectileFrames[kBarrageProjectileCount] = {};
};
