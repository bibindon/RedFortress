#include "EnemySmallMushroom.h"

EnemySmallMushroom::EnemySmallMushroom(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"small_mushroom",
                yaw,
                16,
                1.6f,
                10.0f,
                1.51f * 0.5f,
                3.31f * 0.5f,
                -1.61f * 0.5f,
                MovementMode::Ground,
                true)
{
}
