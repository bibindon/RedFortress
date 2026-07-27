#include "EnemyMushroom.h"

EnemyMushroom::EnemyMushroom(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"mushroom",
                yaw,
                16,
                1.6f,
                10.0f,
                0.6f,
                1.1f,
                MovementMode::Ground,
                true)
{
}
