#include "EnemySpider.h"

EnemySpider::EnemySpider(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"spider",
                yaw,
                8,
                3.5f,
                12.0f,
                0.55f,
                0.4f,
                MovementMode::Ground,
                true)
{
}
