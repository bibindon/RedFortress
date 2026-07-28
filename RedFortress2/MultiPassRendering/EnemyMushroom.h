#pragma once
#include "EnemyBase.h"

class EnemyMushroom : public EnemyBase
{
public:
    EnemyMushroom(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.0f; }
};
