#include "PlayerAttackController.h"

#include <d3dx9.h>

PlayerAttackController::PlayerAttackController()
    : m_currentDefinition(GetDefinition(PlayerAttackType::WeakAttack))
{
}

bool PlayerAttackController::TryStart(PlayerAttackType attackType)
{
    if (IsAttacking())
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
        definition.durationFrames = 57;
        definition.hitDelayFrames = 28;
        definition.damage = 5;
        definition.range = 2.0f;
        definition.halfAngleRadians = D3DXToRadian(45.0f);
        definition.moveSpeed = 108.0f;
        definition.moveStartRemainingFrames = 28;
        definition.moveEndRemainingFrames = 38;
        definition.animationName = L"slash";
        definition.animationSpeed = 2.0f;
        break;
    case PlayerAttackType::StrongAttack:
        definition.durationFrames = 57;
        definition.hitDelayFrames = 28;
        definition.damage = 5;
        definition.range = 2.0f;
        definition.halfAngleRadians = D3DXToRadian(45.0f);
        definition.moveSpeed = 108.0f;
        definition.moveStartRemainingFrames = 28;
        definition.moveEndRemainingFrames = 38;
        definition.animationName = L"slash";
        definition.animationSpeed = 2.0f;
        break;
    }

    return definition;
}
