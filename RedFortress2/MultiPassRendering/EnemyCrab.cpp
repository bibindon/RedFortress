#include "EnemyCrab.h"

EnemyCrab::EnemyCrab(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyCrab(pos, meshId, yaw, L"crab", 1.0f)
{
}

EnemyCrab::EnemyCrab(const D3DXVECTOR3& pos,
                     const int meshId,
                     const float yaw,
                     const std::wstring& type,
                     const float sizeMultiplier)
    : EnemyBase(pos,
                meshId,
                type,
                yaw,
                12,
                1.7f,
                10.0f,
                0.65f * sizeMultiplier,
                0.55f * sizeMultiplier,
                MovementMode::Ground,
                true)
{
}
