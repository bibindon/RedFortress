#include "EnemySpider.h"

EnemySpider::EnemySpider(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemySpider(pos, meshId, yaw, L"spider", 1.0f)
{
}

EnemySpider::EnemySpider(const D3DXVECTOR3& pos,
                         const int meshId,
                         const float yaw,
                         const std::wstring& type,
                         const float sizeMultiplier)
    : EnemyBase(pos,
                meshId,
                type,
                yaw,
                8,
                3.5f,
                12.0f,
                0.55f * sizeMultiplier,
                0.4f * sizeMultiplier,
                MovementMode::Ground,
                true)
{
}
