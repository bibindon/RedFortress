#include "EnemyGolem.h"

EnemyGolem::EnemyGolem(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyGolem(pos, meshId, yaw, L"golem", 1.0f)
{
}

EnemyGolem::EnemyGolem(const D3DXVECTOR3& pos,
                       const int meshId,
                       const float yaw,
                       const std::wstring& type,
                       const float sizeMultiplier)
    : EnemyBase(pos,
                meshId,
                type,
                yaw,
                30,
                1.4f,
                11.0f,
                0.8f * sizeMultiplier,
                2.4f * sizeMultiplier,
                MovementMode::Ground,
                true)
{
}
