#pragma once
#include "EnemyBase.h"

class EnemySmallSpider : public EnemyBase
{
public:
    EnemySmallSpider(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 0.6f * 0.5f; }
};
