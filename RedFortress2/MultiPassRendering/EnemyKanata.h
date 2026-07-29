#pragma once

#include "EnemyBase.h"

class EnemyKanata : public EnemyBase
{
public:
    EnemyKanata(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 1.0f; }
    bool IsBoss() const override { return true; }
    std::wstring GetBossName() const override { return L"天音かなた"; }

protected:
    static float GetCollisionHeight() { return 3.0f; }
    float GetMeshYawOffset() const override { return D3DX_PI; }
};
