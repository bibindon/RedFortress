#pragma once

#include "AnimMesh.h"
#include "Sprite.h"
#include "EnemyBase.h"

#include <thread>
#include <d3d9.h>
#include <mutex>

class EnemyCube : public EnemyBase
{
public:
    EnemyCube();
    ~EnemyCube() override;
    bool Init() override;
    void Update() override;
};

