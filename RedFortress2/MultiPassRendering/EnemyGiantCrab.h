#pragma once
#include "EnemyBase.h"

class EnemyGiantCrab : public EnemyBase
{
public:
    EnemyGiantCrab(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.38f * 3.0f; }
};
