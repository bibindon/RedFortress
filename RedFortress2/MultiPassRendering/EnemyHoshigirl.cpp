#include "EnemyHoshigirl.h"


#include "../../RedFortressRender/Render/Render.h"

namespace
{
    const int kAttackCooldownFrames = 42;

    // ①霊弾連射（SoulBolt）
    const int kSoulBoltWindupFrames = 24;
    const int kSoulBoltActiveFrames = 1;
    const float kSoulBoltSpeed = 6.0f;
    const float kSoulBoltRadius = 0.3f;
    const int kSoulBoltLifetimeFrames = 90;

    // ②幽体突進（WraithCharge）
    const int kWraithChargeWindupFrames = 20;
    const int kWraithChargeActiveFrames = 30;
    const float kWraithChargeSpeed = 6.0f;

    // ③呪いの沼（CurseMire）
    const int kCurseMireWindupFrames = 30;
    const int kCurseMireActiveFrames = 1;
    const int kCurseMireLifetimeFrames = 180;
    const int kCurseMireDamageCooldownFrames = 45;

    // ④引き裂き（SoulReap）
    const int kSoulReapWindupFrames = 14;
    const int kSoulReapActiveFrames = 4;

    float HorizontalDistance(const D3DXVECTOR3& a, const D3DXVECTOR3& b)
    {
        const float x = a.x - b.x;
        const float z = a.z - b.z;
        return sqrtf(x * x + z * z);
    }

    D3DXVECTOR3 HorizontalDirection(const D3DXVECTOR3& from, const D3DXVECTOR3& to)
    {
        D3DXVECTOR3 direction = to - from;
        direction.y = 0.0f;
        if (D3DXVec3LengthSq(&direction) > 0.0001f)
        {
            D3DXVec3Normalize(&direction, &direction);
        }
        return direction;
    }
}

EnemyHoshigirl::EnemyHoshigirl(const D3DXVECTOR3& pos,
                               const int meshId,
                               const float yaw)
    : EnemyBase(pos,
                meshId,
                L"hoshigirl",
                yaw,
                120,
                1.8f,
                20.0f,
                1.2f,
                4.0f,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
}

bool EnemyHoshigirl::UsesSpecialAttacks() const
{
    return true;
}

bool EnemyHoshigirl::UpdateSpecialAttack(NSRender::Render& render,
                                         const D3DXVECTOR3& playerPos,
                                         const bool playerInvincible)
{
    UpdateSoulBoltProjectile(render, playerPos, playerInvincible);
    UpdateCurseMire(render, playerPos, playerInvincible);
    if (m_attackCooldownFrames > 0)
    {
        --m_attackCooldownFrames;
    }

    if (m_attackPhase == AttackPhase::None)
    {
        if (IsSpecialAttackReady() && !playerInvincible && m_attackCooldownFrames <= 0)
        {
            SelectAttack(render, playerPos);
        }
        return m_attackPhase != AttackPhase::None;
    }

    if (m_attackPhase == AttackPhase::Windup)
    {
        // 突進は予備動作中に方向をロック済みなので旋回しない。それ以外は追従。
        if (m_attackType != AttackType::WraithCharge)
        {
            FaceSpecialAttackTarget(playerPos);
        }
        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            BeginActivePhase(render, playerPos);
        }
    }
    else if (m_attackPhase == AttackPhase::Active)
    {
        UpdateActivePhase(render, playerPos, playerInvincible);
        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            BeginRecovery();
        }
    }
    else if (m_attackPhase == AttackPhase::Recovery)
    {
        --m_phaseFrames;
        if (m_phaseFrames <= 0)
        {
            EndAttack();
        }
    }

    return m_attackPhase != AttackPhase::None;
}

void EnemyHoshigirl::SelectAttack(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    const float distance = HorizontalDistance(GetPosition(), playerPos);
    for (int offset = 0; offset < 4; ++offset)
    {
        const int attackIndex = (m_nextAttackIndex + offset) % 4;
        AttackType candidate = AttackType::SoulBolt;
        if (attackIndex == 1)
        {
            candidate = AttackType::WraithCharge;
        }
        else if (attackIndex == 2)
        {
            candidate = AttackType::CurseMire;
        }
        else if (attackIndex == 3)
        {
            candidate = AttackType::SoulReap;
        }

        if (IsAttackAllowed(candidate, distance))
        {
            m_nextAttackIndex = (attackIndex + 1) % 4;
            BeginAttack(render, candidate, playerPos);
            return;
        }
    }
}

bool EnemyHoshigirl::IsAttackAllowed(const AttackType attackType, const float distance) const
{
    if (attackType == AttackType::SoulBolt)
    {
        // 遠距離専用の飛翔体。
        return distance >= 2.5f && distance <= 9.0f;
    }
    if (attackType == AttackType::WraithCharge)
    {
        // 中距離の突進。
        return distance >= 1.5f && distance <= 6.0f;
    }
    if (attackType == AttackType::CurseMire)
    {
        // プレイヤー足元に設置する持続AoE。
        return distance <= 5.0f;
    }
    // SoulReap: 近接の前方円錐攻撃。
    return distance <= 2.2f;
}

void EnemyHoshigirl::BeginAttack(NSRender::Render& render,
                                 const AttackType attackType,
                                 const D3DXVECTOR3& playerPos)
{
    m_attackType = attackType;
    m_attackPhase = AttackPhase::Windup;
    m_attackHitApplied = false;
    FaceSpecialAttackTarget(playerPos);
    m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);

    // 全攻撃で idle アニメを再生（アニメは idle のみ用意されているため）。
    // 攻撃の判別は予備動作フレーム長とパーティクルで行う。
    if (attackType == AttackType::SoulBolt)
    {
        m_phaseFrames = kSoulBoltWindupFrames;
    }
    else if (attackType == AttackType::WraithCharge)
    {
        m_phaseFrames = kWraithChargeWindupFrames;
    }
    else if (attackType == AttackType::CurseMire)
    {
        m_phaseFrames = kCurseMireWindupFrames;
    }
    else
    {
        m_phaseFrames = kSoulReapWindupFrames;
    }
    PlaySpecialAttackAnimation(render, L"idle");
}

void EnemyHoshigirl::BeginActivePhase(NSRender::Render& render, const D3DXVECTOR3& playerPos)
{
    m_attackPhase = AttackPhase::Active;
    m_attackHitApplied = false;

    if (m_attackType == AttackType::SoulBolt)
    {
        // 霊弾を生成。発射位置は体の中心よりやや上。
        m_phaseFrames = kSoulBoltActiveFrames;
        m_soulBoltActive = true;
        m_soulBoltPosition = GetPosition();
        m_soulBoltPosition.y += 1.2f;
        m_soulBoltDirection = HorizontalDirection(m_soulBoltPosition, playerPos);
        m_soulBoltFrames = kSoulBoltLifetimeFrames;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                   m_soulBoltPosition);
    }
    else if (m_attackType == AttackType::WraithCharge)
    {
        // 突進開始時に方向を再ロック。
        m_phaseFrames = kWraithChargeActiveFrames;
        m_lockedDirection = HorizontalDirection(GetPosition(), playerPos);
    }
    else if (m_attackType == AttackType::CurseMire)
    {
        // プレイヤーの足元に沼を設置。
        m_phaseFrames = kCurseMireActiveFrames;
        m_curseMireActive = true;
        m_curseMirePosition = playerPos;
        m_curseMirePosition.y = GetPosition().y;
        m_curseMireFrames = kCurseMireLifetimeFrames;
        m_curseMireDamageCooldownFrames = 0;
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Explosion,
                                   m_curseMirePosition);
    }
    else
    {
        // SoulReap: 発動フレームのみ設定。命中判定は UpdateActivePhase で行う。
        m_phaseFrames = kSoulReapActiveFrames;
    }
}

void EnemyHoshigirl::UpdateActivePhase(NSRender::Render& render,
                                       const D3DXVECTOR3& playerPos,
                                       const bool playerInvincible)
{
    // 突進中はロック方向へ直線移動。
    if (m_attackType == AttackType::WraithCharge)
    {
        MoveForSpecialAttack(m_lockedDirection * kWraithChargeSpeed);
    }
    if (m_attackHitApplied || playerInvincible)
    {
        return;
    }

    const float distance = HorizontalDistance(GetPosition(), playerPos);
    bool hit = false;
    int damage = 0;
    int knockbackFrames = 0;
    if (m_attackType == AttackType::WraithCharge && distance <= 1.1f)
    {
        hit = true;
        damage = 15;
        knockbackFrames = 45;
    }
    else if (m_attackType == AttackType::SoulReap && distance <= 2.0f)
    {
        // 前方円錐判定。ボスの正面付近にいるプレイヤーのみ命中。
        const D3DXVECTOR3 forward(-sinf(GetYaw()), 0.0f, -cosf(GetYaw()));
        const D3DXVECTOR3 toPlayer = HorizontalDirection(GetPosition(), playerPos);
        if (D3DXVec3Dot(&forward, &toPlayer) >= 0.35f)
        {
            hit = true;
            damage = 14;
            knockbackFrames = 28;
        }
    }

    if (hit)
    {
        EmitAttackHit(damage, GetPosition(), knockbackFrames, 0);
        m_attackHitApplied = true;
    }
}

void EnemyHoshigirl::UpdateSoulBoltProjectile(NSRender::Render& render,
                                              const D3DXVECTOR3& playerPos,
                                              const bool playerInvincible)
{
    if (!m_soulBoltActive)
    {
        return;
    }

    if (MoveSpecialProjectile(&m_soulBoltPosition,
                              m_soulBoltDirection * kSoulBoltSpeed,
                              kSoulBoltRadius))
    {
        m_soulBoltActive = false;
        return;
    }
    --m_soulBoltFrames;
    if ((m_soulBoltFrames % 6) == 0)
    {
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                   m_soulBoltPosition);
    }

    if (!playerInvincible &&
        HorizontalDistance(m_soulBoltPosition, playerPos) <= 0.7f &&
        fabsf(m_soulBoltPosition.y - playerPos.y) <= 1.5f)
    {
        EmitAttackHit(12, m_soulBoltPosition, 30, 0);
        m_soulBoltActive = false;
    }
    else if (m_soulBoltFrames <= 0)
    {
        m_soulBoltActive = false;
    }
}

void EnemyHoshigirl::UpdateCurseMire(NSRender::Render& render,
                                     const D3DXVECTOR3& playerPos,
                                     const bool playerInvincible)
{
    if (!m_curseMireActive)
    {
        return;
    }

    --m_curseMireFrames;
    if (m_curseMireDamageCooldownFrames > 0)
    {
        --m_curseMireDamageCooldownFrames;
    }
    if ((m_curseMireFrames % 30) == 0)
    {
        render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage,
                                   m_curseMirePosition);
    }

    if (!playerInvincible && m_curseMireDamageCooldownFrames <= 0 &&
        HorizontalDistance(m_curseMirePosition, playerPos) <= 1.8f)
    {
        EmitAttackHit(5, m_curseMirePosition, 0, 60);
        m_curseMireDamageCooldownFrames = kCurseMireDamageCooldownFrames;
    }
    if (m_curseMireFrames <= 0)
    {
        m_curseMireActive = false;
    }
}

void EnemyHoshigirl::BeginRecovery()
{
    m_attackPhase = AttackPhase::Recovery;
    m_phaseFrames = 18;
}

void EnemyHoshigirl::EndAttack()
{
    m_attackType = AttackType::None;
    m_attackPhase = AttackPhase::None;
    m_phaseFrames = 0;
    m_attackCooldownFrames = kAttackCooldownFrames;
    FinishSpecialAttack();
}
