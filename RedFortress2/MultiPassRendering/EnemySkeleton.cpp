#include "EnemySkeleton.h"

EnemySkeleton::EnemySkeleton(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemySkeleton(pos, meshId, yaw, L"skeleton", 1.0f)
{
}

EnemySkeleton::EnemySkeleton(const D3DXVECTOR3& pos,
                             const int meshId,
                             const float yaw,
                             const std::wstring& type,
                             const float sizeMultiplier)
    : EnemyBase(pos,
                meshId,
                type,
                yaw,
                12,
                2.5f,
                13.0f,
                0.45f * sizeMultiplier,
                1.7f * sizeMultiplier,
                MovementMode::Ground,
                true)
{
}
