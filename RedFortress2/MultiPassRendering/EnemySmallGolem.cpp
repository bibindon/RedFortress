#include "EnemySmallGolem.h"

EnemySmallGolem::EnemySmallGolem(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"small_golem",
                yaw,
                30,
                1.4f,
                11.0f,
                3.90f * 0.5f,
                2.34f * 0.5f,
                -2.08f * 0.5f,
                MovementMode::Ground,
                true)
{
}
