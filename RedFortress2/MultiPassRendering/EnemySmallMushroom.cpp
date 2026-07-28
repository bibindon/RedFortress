#include "EnemySmallMushroom.h"

EnemySmallMushroom::EnemySmallMushroom(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"small_mushroom",
                yaw,
                16,
                1.6f,
                10.0f,
                0.6f * 0.5f,
                1.1f * 0.5f,
                MovementMode::Ground,
                true)
{
}
