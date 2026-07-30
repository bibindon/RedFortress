#include "EnemySmallSpider.h"

EnemySmallSpider::EnemySmallSpider(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"small_spider",
                yaw,
                8,
                3.5f,
                12.0f,
                1.94f * 0.5f,
                1.13f * 0.5f,
                -0.56f * 0.5f,
                MovementMode::Ground,
                true)
{
}
