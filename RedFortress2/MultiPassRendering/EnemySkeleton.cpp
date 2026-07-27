#include "EnemySkeleton.h"

EnemySkeleton::EnemySkeleton(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"skeleton",
                yaw,
                12,
                2.5f,
                13.0f,
                0.45f,
                1.7f,
                MovementMode::Ground,
                true)
{
}
