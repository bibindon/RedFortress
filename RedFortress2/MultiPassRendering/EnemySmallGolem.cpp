#include "EnemySmallGolem.h"

EnemySmallGolem::EnemySmallGolem(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"small_golem",
                yaw,
                30,
                1.4f,
                11.0f,
                0.8f * 0.5f,
                2.4f * 0.5f,
                MovementMode::Ground,
                true)
{
}
