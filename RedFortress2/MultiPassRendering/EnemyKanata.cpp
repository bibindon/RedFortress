#include "EnemyKanata.h"

EnemyKanata::EnemyKanata(const D3DXVECTOR3& pos,
                         const int meshId,
                         const float yaw)
    : EnemyBase(pos,
                meshId,
                L"kanata",
                yaw,
                80,
                1.8f,
                20.0f,
                0.55f,
                3.0f,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
}