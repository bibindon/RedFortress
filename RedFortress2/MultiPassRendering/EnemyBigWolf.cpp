#include "EnemyBigWolf.h"

EnemyBigWolf::EnemyBigWolf(const D3DXVECTOR3& pos, int meshId, float yaw)
    : EnemyBase(pos, meshId, L"enemy2", yaw, 15, 2.5f, 12.0f, 0.75f, 0.75f) {}
