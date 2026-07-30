#include "EnemyCrab.h"

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
}
