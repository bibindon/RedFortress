#pragma once
#include "EnemyBase.h"

class EnemyCrab : public EnemyBase
{
public:
    EnemyCrab(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.38f; }

protected:
    void UpdateChaseLocomotion(const D3DXVECTOR3& targetPosition,
                               const D3DXVECTOR3& moveDirection,
                               const D3DXVECTOR3& sideDirection,
                               float speedMultiplier,
                               float distance) override;

private:
    int m_sideDashCooldownFrames = 0;
    int m_sideDashFrames = 0;
    D3DXVECTOR3 m_sideDashDirection = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
};
