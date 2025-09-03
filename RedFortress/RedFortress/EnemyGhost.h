#pragma once

#include "AnimMesh.h"
#include "Sprite.h"
#include "EnemyBase.h"

#include <thread>
#include <d3d9.h>
#include <mutex>

class EnemyGhost : public EnemyBase
{
public:
    EnemyGhost();
    ~EnemyGhost() override;
    bool Init() override;
    void Update() override;
};

