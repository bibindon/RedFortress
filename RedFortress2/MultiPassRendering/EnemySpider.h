#pragma once
#include "EnemyBase.h"

class EnemySpider : public EnemyBase
{
public:
    EnemySpider(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.6f; }
};
