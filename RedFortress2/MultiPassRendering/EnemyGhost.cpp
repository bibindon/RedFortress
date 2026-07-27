#include "EnemyGhost.h"

EnemyGhost::EnemyGhost(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"ghost",
                yaw,
                12,
                1.8f,
                11.0f,
                0.55f,
                1.0f,
                MovementMode::Hover,
                true)
{
}
