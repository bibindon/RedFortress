#pragma once
#include "EnemyBase.h"

class EnemyFrog : public EnemyBase
{
public:
    EnemyFrog(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.35f; }
};
