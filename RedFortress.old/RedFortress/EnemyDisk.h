#pragma once

#include <d3d9.h>
#include "AnimMesh.h"
#include "Sprite.h"
#include "EnemyBase.h"

class EnemyDisk : public EnemyBase
{
public:
    EnemyDisk();
    ~EnemyDisk();
    bool Init() override;
    void Update() override;
};

