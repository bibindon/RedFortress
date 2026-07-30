#include "EnemyFrog.h"

EnemyFrog::EnemyFrog(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"frog",
                yaw,
                8,
                2.4f,
                11.0f,
                0.46f,
                0.75f,
                -0.36f,
                MovementMode::Frog,
                true)
{
}
