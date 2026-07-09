#pragma once
#include "EnemyBase.h"

class EnemyWolf : public EnemyBase
{
public:
    EnemyWolf(const D3DXVECTOR3& pos, int meshId, float yaw);
    static const wchar_t* GetFolderName() { return L"separatedAnim"; }
    static float GetScale() { return 2.0f; }
};
