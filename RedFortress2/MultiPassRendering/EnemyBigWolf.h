#pragma once
#include "EnemyBase.h"

class EnemyBigWolf : public EnemyBase
{
public:
    EnemyBigWolf(const D3DXVECTOR3& pos, int meshId, float yaw);
    static const wchar_t* GetFolderName() { return L"Enemy2"; }
    static float GetScale() { return 3.0f; }
};
