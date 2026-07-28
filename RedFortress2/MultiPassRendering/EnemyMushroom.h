#pragma once
#include "EnemyBase.h"

class EnemyMushroom : public EnemyBase
{
public:
    EnemyMushroom(const D3DXVECTOR3& pos, int meshId, float yaw);
    EnemyMushroom(const D3DXVECTOR3& pos,
                  int meshId,
                  float yaw,
                  const std::wstring& type,
                  float sizeMultiplier);
    static float GetScale() { return 1.0f; }
};
