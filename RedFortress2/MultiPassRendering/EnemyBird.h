#pragma once
#include "EnemyBase.h"

class EnemyBird : public EnemyBase
{
public:
    EnemyBird(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.35f; }
};
