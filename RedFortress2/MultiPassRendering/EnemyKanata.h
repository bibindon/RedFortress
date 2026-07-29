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

    // EnemyBase stores the center of the collision cylinder, while enemy.x uses the feet as Y=0.
    float GetMeshVerticalOffset() const override { return -GetCollisionHeight() * 0.5f; }

    // The prepared model follows the standard Blender -Y facing convention. Do not rotate it by 180 degrees.
    float GetMeshYawOffset() const override { return 0.0f; }
};
