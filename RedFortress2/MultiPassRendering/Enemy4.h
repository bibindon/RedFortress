#pragma once
#include "EnemyBase.h"

class Enemy4 : public EnemyBase
{
public:
    Enemy4(const D3DXVECTOR3& pos, int meshId, float yaw);
    static const wchar_t* GetFolderName() { return L"Enemy4"; }
    static float GetScale() { return 2.0f; }
};
