#include "Enemy4.h"

Enemy4::Enemy4(const D3DXVECTOR3& pos, int meshId, float yaw)
    : EnemyBase(pos, meshId, L"enemy4", yaw, 10, 2.5f, 12.0f, 0.98f, 1.12f, -0.56f) {}
