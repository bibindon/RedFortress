#include "Enemy6.h"

Enemy6::Enemy6(const D3DXVECTOR3& pos, int meshId, float yaw)
    : EnemyBase(pos, meshId, L"enemy6", yaw, 10, 2.5f, 12.0f, 0.5f, 0.5f) {}
