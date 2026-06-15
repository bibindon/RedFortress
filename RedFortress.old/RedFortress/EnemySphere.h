#pragma once

#include <d3d9.h>
#include "AnimMesh.h"
#include "Sprite.h"
#include "EnemyBase.h"

class EnemySphere : public EnemyBase
{
public:
    EnemySphere();
    ~EnemySphere();
    bool Init() override;
    void Update() override;
};


