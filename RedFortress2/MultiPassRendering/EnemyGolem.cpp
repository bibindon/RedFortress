#include "EnemyGolem.h"

EnemyGolem::EnemyGolem(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"golem",
                yaw,
                30,
                1.4f,
                11.0f,
                3.90f,
                2.34f,
                -2.08f,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
}
