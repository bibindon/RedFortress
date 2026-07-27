#include "EnemyGolem.h"

EnemyGolem::EnemyGolem(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"golem",
                yaw,
                30,
                1.4f,
                11.0f,
                0.8f,
                2.4f,
                MovementMode::Ground,
                true)
{
}
