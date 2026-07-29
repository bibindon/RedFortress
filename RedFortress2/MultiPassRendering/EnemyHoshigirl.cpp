#include "EnemyHoshigirl.h"

EnemyHoshigirl::EnemyHoshigirl(const D3DXVECTOR3& pos,
                               const int meshId,
                               const float yaw)
    : EnemyBase(pos,
                meshId,
                L"hoshigirl",
                yaw,
                120,
                1.8f,
                20.0f,
                1.2f,
                4.0f,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
}
