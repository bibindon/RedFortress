#pragma once
#include "EnemyBase.h"

class EnemyCrab : public EnemyBase
{
public:
    EnemyCrab(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.38f; }
};
