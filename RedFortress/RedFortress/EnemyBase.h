#pragma once

#include "AnimMesh.h"
#include "Sprite.h"

#include <thread>
#include <d3d9.h>
#include <mutex>

enum class eEnemyState
{
    IDLE,
    WALK,
    DAMAGED,
    DEAD,
    DISABLE,
    ATTACK,
    STEP,
    DASH,
};

enum class eEnemyType
{
    Cube,
    Sphere,
    Disk,
    Enchu,
    Hanen,
    Hankyuu,
    OrangeCube,
    BigCube,
    SmallCube,
    Ghost,
};

class EnemyBase
{
public:
    EnemyBase();
    virtual ~EnemyBase();
    virtual bool Init();
    void Render();
    virtual void Update();
    void SetPos(const D3DXVECTOR3& pos);
    D3DXVECTOR3 GetPos() const;
    void SetRotate(const D3DXVECTOR3& rotate);
    D3DXVECTOR3 GetRotate() const;
    void SetHP(const int hp);
    int GetHP() const;
    void SetState(const eEnemyState state);
    eEnemyState GetState() const;

    void SetIdSub(const int arg);
    int GetIdSub() const;

    eEnemyType GetEnemyType();

    void SetShowMenu(const std::wstring& id);

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

protected:

    AnimMesh* m_AnimMesh = nullptr;

    D3DXVECTOR3 m_loadingPos { 0.f, 0.f, 0.f };
    D3DXVECTOR3 m_rotate { 0.f, 0.f, 0.f };

    bool m_bAttack = false;
    int m_attackTimeCounter = 0;

    int m_HP = 100;
    Sprite* m_spriteHP = nullptr;
    Sprite* m_spriteHPBack = nullptr;
    eEnemyState m_state = eEnemyState::IDLE;
    int m_deadTimeCounter = 0;
    int m_damagedTimeCounter = 0;
    D3DXVECTOR3 GetAttackPos() const;

    int m_idSub = 0;

    std::thread* m_thread = nullptr;
    std::atomic<bool> m_loaded = false;
    static std::mutex s_mutex;

    eEnemyType m_eType = eEnemyType::Cube;
};

