#include "PlayerAttackController.h"

#include <d3dx9.h>

PlayerAttackController::PlayerAttackController()
    : m_currentDefinition(GetDefinition(PlayerAttackType::WeakAttack))
{
}

bool PlayerAttackController::TryStart(PlayerAttackType attackType)
{
    if (IsAttacking() && (!m_hitResolved || m_hitRequested))
    {
        return false;
    }

    m_currentAttackType = attackType;
    m_currentDefinition = GetDefinition(attackType);
    m_remainingFrames = m_currentDefinition.durationFrames;
    m_hitDelayFrames = m_currentDefinition.hitDelayFrames;
    m_hitRequested = false;
    m_hitResolved = false;
    return true;
}

void PlayerAttackController::Update()
{
    m_hitRequested = false;
    if (!IsAttacking())
    {
        return;
    }

    --m_remainingFrames;
    if (m_hitDelayFrames > 0)
    {
        --m_hitDelayFrames;
    }

    if (m_hitDelayFrames == 0 && !m_hitResolved)
    {
        m_hitRequested = true;
        m_hitResolved = true;
    }
}

void PlayerAttackController::Reset()
{
    m_remainingFrames = 0;
    m_hitDelayFrames = -1;
    m_hitRequested = false;
    m_hitResolved = false;
}

bool PlayerAttackController::IsAttacking() const
{
    return m_remainingFrames > 0;
}

bool PlayerAttackController::IsMovementActive() const
{
    if (!IsAttacking())
    {
        return false;
    }

    return m_remainingFrames >= m_currentDefinition.moveStartRemainingFrames
        && m_remainingFrames <= m_currentDefinition.moveEndRemainingFrames;
}

bool PlayerAttackController::ConsumeHitRequested()
{
    if (!m_hitRequested)
    {
        return false;
    }

    m_hitRequested = false;
    return true;
}

const PlayerAttackDefinition& PlayerAttackController::GetCurrentDefinition() const
{
    return m_currentDefinition;
}

PlayerAttackDefinition PlayerAttackController::GetDefinition(PlayerAttackType attackType) const
{
    PlayerAttackDefinition definition;

    switch (attackType)
    {
    case PlayerAttackType::WeakAttack:
        definition.durationFrames = 24;
        definition.hitDelayFrames = 12;
        definition.damage = 2;
        definition.range = 3.0f;
        definition.halfAngleRadians = D3DXToRadian(90.0f);
        definition.moveSpeed = 5.5f;
        definition.moveStartRemainingFrames = 12;
        definition.moveEndRemainingFrames = 16;
        definition.animationName = L"slash";
        definition.animationSpeed = 4.0f;
        break;
    case PlayerAttackType::StrongAttack:
        definition.durationFrames = 57;
        definition.hitDelayFrames = 28;
        definition.damage = 5;
        definition.range = 3.0f;
        definition.halfAngleRadians = D3DXToRadian(90.0f);
        definition.moveSpeed = 0.0909f;
        definition.moveStartRemainingFrames = 28;
        definition.moveEndRemainingFrames = 38;
        definition.animationName = L"slash";
        definition.animationSpeed = 2.0f;
        break;
    case PlayerAttackType::SwordAttack:
        definition.durationFrames = 24;
        definition.hitDelayFrames = 12;
        definition.damage = 2;
        definition.range = 3.0f;
        definition.halfAngleRadians = D3DXToRadian(90.0f);
        definition.moveSpeed = 5.5f;
        definition.moveStartRemainingFrames = 12;
        definition.moveEndRemainingFrames = 16;
        definition.animationName = L"slash";
        definition.animationSpeed = 4.0f;
        break;
    case PlayerAttackType::SwordStrongAttack:
        definition.durationFrames = 57;
        definition.hitDelayFrames = 28;
        definition.damage = 5;
        definition.range = 3.0f;
        definition.halfAngleRadians = D3DXToRadian(90.0f);
        definition.moveSpeed = 0.0909f;
        definition.moveStartRemainingFrames = 28;
        definition.moveEndRemainingFrames = 38;
        definition.animationName = L"slash";
        definition.animationSpeed = 2.0f;
        break;
    case PlayerAttackType::BombAttack:
        definition.durationFrames = 24;
        definition.hitDelayFrames = 12;
        definition.damage = 2;
        definition.range = 2.0f;
        definition.halfAngleRadians = D3DXToRadian(45.0f);
        definition.moveSpeed = 5.5f;
        definition.moveStartRemainingFrames = 12;
        definition.moveEndRemainingFrames = 16;
        definition.animationName = L"slash";
        definition.animationSpeed = 4.0f;
        break;
    case PlayerAttackType::BombStrongAttack:
        definition.durationFrames = 57;
        definition.hitDelayFrames = 28;
        definition.damage = 5;
        definition.range = 2.0f;
        definition.halfAngleRadians = D3DXToRadian(45.0f);
        definition.moveSpeed = 0.0909f;
        definition.moveStartRemainingFrames = 28;
        definition.moveEndRemainingFrames = 38;
        definition.animationName = L"slash";
        definition.animationSpeed = 2.0f;
        break;
    case PlayerAttackType::BusterAttack:
        definition.durationFrames = 6;
        definition.hitDelayFrames = 3;
        definition.damage = 3;
        definition.range = 0.0f;
        definition.halfAngleRadians = 0.0f;
        definition.moveSpeed = 0.0f;
        definition.moveStartRemainingFrames = 0;
        definition.moveEndRemainingFrames = 0;
        definition.animationName = L"slash";
        definition.animationSpeed = 6.0f;
        break;
    case PlayerAttackType::BusterStrongAttack:
        definition.durationFrames = 6;
        definition.hitDelayFrames = 3;
        definition.damage = 3;
        definition.range = 0.0f;
        definition.halfAngleRadians = 0.0f;
        definition.moveSpeed = 0.0f;
        definition.moveStartRemainingFrames = 0;
        definition.moveEndRemainingFrames = 0;
        definition.animationName = L"slash";
        definition.animationSpeed = 6.0f;
        break;
    }

    return definition;
}

void PlayerAttackController::CycleAttackCategory(int direction)
{
    const int categoryCount = 4;
    int next = m_selectedCategory + direction;
    next %= categoryCount;
    if (next < 0)
    {
        next += categoryCount;
    }
    m_selectedCategory = next;
}

PlayerAttackType PlayerAttackController::GetAttackType(bool isStrong) const
{
    if (m_selectedCategory == 0)
    {
        if (isStrong)
        {
            return PlayerAttackType::StrongAttack;
        }
        return PlayerAttackType::WeakAttack;
    }
    if (m_selectedCategory == 1)
    {
        if (isStrong)
        {
            return PlayerAttackType::SwordStrongAttack;
        }
        return PlayerAttackType::SwordAttack;
    }
    if (m_selectedCategory == 2)
    {
        if (isStrong)
        {
            return PlayerAttackType::BusterStrongAttack;
        }
        return PlayerAttackType::BusterAttack;
    }
    else
    {
        if (isStrong)
        {
            return PlayerAttackType::BombStrongAttack;
        }
        return PlayerAttackType::BombAttack;
    }
}

const wchar_t* PlayerAttackController::GetCurrentCategoryName() const
{
    if (m_selectedCategory == 0)
    {
        return L"薙ぎ払い";
    }
    if (m_selectedCategory == 1)
    {
        return L"海賊剣";
    }
    if (m_selectedCategory == 2)
    {
        return L"バスター";
    }
    else
    {
        return L"爆弾設置";
    }
}
