#include "EnemyWolf.h"

EnemyWolf::EnemyWolf(const D3DXVECTOR3& pos, int meshId, float yaw)
    : EnemyBase(pos, meshId, L"wolf", yaw, 10, 2.5f, 12.0f, 0.5f, 0.5f) {}
