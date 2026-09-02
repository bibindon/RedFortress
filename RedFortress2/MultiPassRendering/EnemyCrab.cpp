#include "EnemyCrab.h"

namespace
{
    const int kSideDashFrames = 9;
    const int kSideDashInitialCooldownMinFrames = 60;
    const int kSideDashInitialCooldownMaxFrames = 120;
    const int kSideDashCooldownMinFrames = 100;
    const int kSideDashCooldownMaxFrames = 190;
    const float kSideDashSpeed = 7.0f;
    const float kSideDashMinPlayerDistance = 1.2f;
    const float kSideDashMaxPlayerDistance = 6.0f;
}

EnemyCrab::EnemyCrab(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"crab",
                yaw,
                12,
                1.7f,
                10.0f,
                0.54f,
                0.36f,
                -0.18f,
                MovementMode::Ground,
                true)
{
    m_sideDashCooldownFrames = NextRandomInt(kSideDashInitialCooldownMinFrames,
                                             kSideDashInitialCooldownMaxFrames);
}

void EnemyCrab::UpdateChaseLocomotion(const D3DXVECTOR3& targetPosition,
                                      const D3DXVECTOR3& moveDirection,
                                      const D3DXVECTOR3& sideDirection,
                                      const float speedMultiplier,
                                      const float distance)
{
    if (m_sideDashFrames > 0)
    {
        FaceSpecialAttackTarget(targetPosition);
        MoveForSpecialAttack(m_sideDashDirection * kSideDashSpeed);
        --m_sideDashFrames;
        if (m_sideDashFrames <= 0)
        {
            m_sideDashCooldownFrames = NextRandomInt(kSideDashCooldownMinFrames,
                                                     kSideDashCooldownMaxFrames);
        }
        return;
    }

    if (m_sideDashCooldownFrames > 0)
    {
        --m_sideDashCooldownFrames;
    }

    const bool isPlayerInDashRange = distance >= kSideDashMinPlayerDistance &&
        distance <= kSideDashMaxPlayerDistance;
    if (m_sideDashCooldownFrames <= 0 && isPlayerInDashRange)
    {
        float dashDirectionSign = 1.0f;
        if (NextRandom01() < 0.5f)
        {
            dashDirectionSign = -1.0f;
        }
        m_sideDashDirection = sideDirection * dashDirectionSign;
        m_sideDashFrames = kSideDashFrames;

        FaceSpecialAttackTarget(targetPosition);
        MoveForSpecialAttack(m_sideDashDirection * kSideDashSpeed);
        --m_sideDashFrames;
        return;
    }

    EnemyBase::UpdateChaseLocomotion(targetPosition,
                                     moveDirection,
                                     sideDirection,
                                     speedMultiplier,
                                     distance);
}
