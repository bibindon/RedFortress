#pragma once

#include "AnimMesh.h"
#include "Sprite.h"
#include "EnemyBase.h"

#include <thread>
#include <d3d9.h>
#include <mutex>

class EnemyHanen : public EnemyBase
{
public:
    EnemyHanen();
    ~EnemyHanen() override;
    bool Init() override;
    void Update() override;
private:
    bool m_bJump = false;

    D3DXVECTOR3 m_vDash { 0.f, 0.f, 0.f };
    
    // 攻撃判定ありフラグ。
    // ギルティギアのダークエンジェルみたいにダメージが何度も発生するのを防ぐ。
    bool m_bAttack = false;
};

