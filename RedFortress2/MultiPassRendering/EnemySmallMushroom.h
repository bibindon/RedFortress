#pragma once
#include "EnemyBase.h"

class EnemySmallMushroom : public EnemyBase
{
public:
    EnemySmallMushroom(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.0f * 0.5f; }
};
