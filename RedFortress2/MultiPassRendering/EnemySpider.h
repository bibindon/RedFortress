#pragma once
#include "EnemyBase.h"

class EnemySpider : public EnemyBase
{
public:
    EnemySpider(const D3DXVECTOR3& pos, int meshId, float yaw);
    EnemySpider(const D3DXVECTOR3& pos,
                int meshId,
                float yaw,
                const std::wstring& type,
                float sizeMultiplier);
    static float GetScale() { return 0.6f; }
};
