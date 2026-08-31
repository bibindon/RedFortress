#include "EnemyGiantCrab.h"

namespace
{
    const int kGiantCrabMaxHp = 12;
    const int kBossGiantCrabMaxHp = kGiantCrabMaxHp * 20;
}

EnemyGiantCrab::EnemyGiantCrab(const D3DXVECTOR3& pos, const int meshId, const float yaw)
    : EnemyGiantCrab(pos, meshId, yaw, kGiantCrabMaxHp)
{
}

EnemyGiantCrab::EnemyGiantCrab(const D3DXVECTOR3& pos,
                               const int meshId,
                               const float yaw,
                               const int maxHp)
    : EnemyBase(pos,
                meshId,
                L"giant_crab",
                yaw,
                maxHp,
                1.7f,
                10.0f,
                0.54f * 3.0f,
                0.36f * 3.0f,
                -0.18f * 3.0f,
                MovementMode::Ground,
                true,
                HitReactionMode::SuperArmor)
{
}

EnemyBossGiantCrab::EnemyBossGiantCrab(const D3DXVECTOR3& pos,
                                       const int meshId,
                                       const float yaw)
    : EnemyGiantCrab(pos, meshId, yaw, kBossGiantCrabMaxHp)
{
}
