#pragma once
#include "EnemyBase.h"

class Enemy3 : public EnemyBase
{
public:
    Enemy3(const D3DXVECTOR3& pos, int meshId, float yaw);
    static const wchar_t* GetFolderName() { return L"Enemy3"; }
    static float GetScale() { return 2.0f; }
};
