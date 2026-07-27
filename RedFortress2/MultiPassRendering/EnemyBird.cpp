#include "EnemyBird.h"

EnemyBird::EnemyBird(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"bird",
                yaw,
                8,
                4.0f,
                14.0f,
                0.45f,
                0.75f,
                MovementMode::Swoop,
                true)
{
}
