#pragma once
#include "EnemyBase.h"

class Enemy5 : public EnemyBase
{
public:
    Enemy5(const D3DXVECTOR3& pos, int meshId, float yaw);
    static const wchar_t* GetFolderName() { return L"Enemy5"; }
    static float GetScale() { return 2.0f; }
};
