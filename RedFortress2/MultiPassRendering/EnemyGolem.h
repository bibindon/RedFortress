#pragma once
#include "EnemyBase.h"

class EnemyGolem : public EnemyBase
{
public:
    EnemyGolem(const D3DXVECTOR3& pos, int meshId, float yaw);
    EnemyGolem(const D3DXVECTOR3& pos,
               int meshId,
               float yaw,
               const std::wstring& type,
               float sizeMultiplier);
    static float GetScale() { return 1.4f; }
};
