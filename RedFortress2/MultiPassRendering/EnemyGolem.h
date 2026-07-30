#pragma once
#include "EnemyBase.h"

class EnemyGolem : public EnemyBase
{
public:
    EnemyGolem(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.4f; }
};
