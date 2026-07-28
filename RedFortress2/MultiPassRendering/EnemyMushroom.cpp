#include "EnemyMushroom.h"

EnemyMushroom::EnemyMushroom(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyMushroom(pos, meshId, yaw, L"mushroom", 1.0f)
{
}

EnemyMushroom::EnemyMushroom(const D3DXVECTOR3& pos,
                             const int meshId,
                             const float yaw,
                             const std::wstring& type,
                             const float sizeMultiplier)
    : EnemyBase(pos,
                meshId,
                type,
                yaw,
                16,
                1.6f,
                10.0f,
                0.6f * sizeMultiplier,
                1.1f * sizeMultiplier,
                MovementMode::Ground,
                true)
{
}
