#pragma once
#include "EnemyBase.h"

class Enemy6 : public EnemyBase
{
public:
    Enemy6(const D3DXVECTOR3& pos, int meshId, float yaw);
    static const wchar_t* GetFolderName() { return L"Enemy6"; }
    static float GetScale() { return 2.0f; }
};
