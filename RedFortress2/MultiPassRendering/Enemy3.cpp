#include "Enemy3.h"

Enemy3::Enemy3(const D3DXVECTOR3& pos, int meshId, float yaw)
    : EnemyBase(pos, meshId, L"enemy3", yaw, 10, 2.5f, 12.0f, 0.5f, 0.5f) {}
