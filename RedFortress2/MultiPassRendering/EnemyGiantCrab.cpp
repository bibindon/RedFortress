#include "EnemyGiantCrab.h"

EnemyGiantCrab::EnemyGiantCrab(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"giant_crab",
                yaw,
                12,
                1.7f,
                10.0f,
                0.65f * 3.0f,
                0.55f * 3.0f,
                MovementMode::Ground,
                true)
{
}
