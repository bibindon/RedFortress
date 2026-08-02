#pragma once
#include "EnemyBase.h"

class EnemySmallGolem : public EnemyBase
{
public:
    EnemySmallGolem(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.4f * 0.5f; }
    D3DXVECTOR3 GetMeshPositionOffset() const override
    {
        return D3DXVECTOR3(0.0f, 0.0f, 0.80f);
    }
};
