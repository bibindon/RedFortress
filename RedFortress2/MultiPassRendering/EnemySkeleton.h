#pragma once
#include "EnemyBase.h"

class EnemySkeleton : public EnemyBase
{
public:
    EnemySkeleton(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.0f; }
};
