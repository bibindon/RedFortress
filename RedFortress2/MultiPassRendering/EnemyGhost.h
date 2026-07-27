#pragma once
#include "EnemyBase.h"

class EnemyGhost : public EnemyBase
{
public:
    EnemyGhost(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.42f; }
};
