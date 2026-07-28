#include "EnemySmallSkeleton.h"

EnemySmallSkeleton::EnemySmallSkeleton(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"small_skeleton",
                yaw,
                12,
                2.5f,
                13.0f,
                0.45f * 0.5f,
                1.7f * 0.5f,
                MovementMode::Ground,
                true)
{
}
