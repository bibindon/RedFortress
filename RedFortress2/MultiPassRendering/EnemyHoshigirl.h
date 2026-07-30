#pragma once

#include "EnemyBase.h"

// ホシガール: ステージ3-8のボス。
// 黒い布ゴースト型の敵。待機(idle)モーションのみ用意されており、
// すべてのアニメーション状態が idle にマッピングされる。
// 着地して歩行で追尾し、スーパーアーマーで被弾時にノックバックしない。
// 4種の攻撃パターン(霊弾連射/幽体突進/呪いの沼/引き裂き)を持つ。
// 詳細は HOSHIGIRL_PLAN.md を参照。
class EnemyHoshigirl : public EnemyBase
{
public:
    EnemyHoshigirl(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 2.5f; }

    // 4種の特殊攻撃を使用する。true を返すと接触ダメージが無効化され、
    // 全ダメージは UpdateSpecialAttack 内の EmitAttackHit 経由で処理される。
    bool UsesSpecialAttacks() const override;

protected:
    bool UpdateSpecialAttack(NSRender::Render& render,
                             const D3DXVECTOR3& playerPos,
                             bool playerInvincible) override;

private:
    enum class AttackType
    {
        None,
        SoulBolt,       // ①霊弾連射（遠距離・飛翔体）
        WraithCharge,   // ②幽体突進（中距離・突進）
        CurseMire,      // ③呪いの沼（設置・持続AoE）
        SoulReap        // ④引き裂き（近接・前方円錐）
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
    void UpdateSoulBoltProjectile(NSRender::Render& render,
                                  const D3DXVECTOR3& playerPos,
                                  bool playerInvincible);
    void UpdateCurseMire(NSRender::Render& render,
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

    // ①霊弾連射（SoulBolt）用
    bool m_soulBoltActive = false;
    D3DXVECTOR3 m_soulBoltPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_soulBoltDirection = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
    int m_soulBoltFrames = 0;

    // ③呪いの沼（CurseMire）用
    bool m_curseMireActive = false;
    D3DXVECTOR3 m_curseMirePosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    int m_curseMireFrames = 0;
    int m_curseMireDamageCooldownFrames = 0;
};
