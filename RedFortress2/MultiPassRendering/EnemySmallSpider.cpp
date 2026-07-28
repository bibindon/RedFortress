#include "EnemySmallSpider.h"

EnemySmallSpider::EnemySmallSpider(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyBase(pos,
                meshId,
                L"small_spider",
                yaw,
                8,
                3.5f,
                12.0f,
                0.55f * 0.5f,
                0.4f * 0.5f,
                MovementMode::Ground,
                true)
{
}
