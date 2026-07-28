#pragma once
#include "EnemyBase.h"

class EnemySmallSkeleton : public EnemyBase
{
public:
    EnemySmallSkeleton(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.0f * 0.5f; }
};
