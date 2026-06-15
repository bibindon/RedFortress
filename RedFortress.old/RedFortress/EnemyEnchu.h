#pragma once

#include "AnimMesh.h"
#include "Sprite.h"
#include "EnemyBase.h"

#include <thread>
#include <d3d9.h>
#include <mutex>

class EnemyEnchu : public EnemyBase
{
public:
    EnemyEnchu();
    ~EnemyEnchu() override;
    bool Init() override;
    void Update() override;

private:
    D3DXVECTOR3 m_vDash { 0.f, 0.f, 0.f };
    
    // 攻撃判定ありフラグ。
    // ギルティギアのダークエンジェルみたいにダメージが何度も発生するのを防ぐ。
    bool m_bAttack = false;
};

