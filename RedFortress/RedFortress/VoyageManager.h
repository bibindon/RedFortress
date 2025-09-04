#pragma once

#include "../../RedFortressModel/Model/Voyage.h"
#include "Common.h"
#include "AnimMesh.h"
#include "Mesh.h"

class Raft2
{
public:

    void Init(const int id);
    void Finalize();
    void Update();
    void Draw();
    void SetSail(const bool arg);

    void PullOarBoth(D3DXVECTOR3* move);
    void PullOarLeft();
    void PullOarRight();

    void Pull3Hours();

    D3DXVECTOR3 GetPos() const;
    void SetPos(const D3DXVECTOR3& pos);

    D3DXVECTOR3 GetRotate() const;
    void SetRotate(const D3DXVECTOR3& rot);

    Mesh* GetCollisionMesh();
    int GetId() const;

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:

    int m_id = 0;

    Mesh* m_meshRaft = nullptr;
    Mesh* m_meshRaftCollision = nullptr;
    AnimMesh* m_meshSail = nullptr;
    AnimMesh* m_meshOarLeft = nullptr;
    AnimMesh* m_meshOarRight = nullptr;
    AnimMesh* m_meshCord = nullptr;

    // 移動量
    D3DXVECTOR3 m_move = D3DXVECTOR3(0.f, 0.f, 0.f);

    // 移動量（回転）
    D3DXVECTOR3 m_moveRot = D3DXVECTOR3(0.f, 0.f, 0.f);

    D3DXVECTOR3 m_pos = D3DXVECTOR3(0.f, 0.f, 0.f);
    D3DXVECTOR3 m_rotate = D3DXVECTOR3(0.f, D3DX_PI, 0.f);

    // オールを漕ぐ間隔
    int m_coolTime = 0;
};

class VoyageManager
{
public:
    static VoyageManager* Get();
    static void Destroy();
    void Init();
    void Finalize();
    void Update();
    void Operate(eBattleState* state);
    void Draw();
    void SetSail(const bool arg);
    bool GetSail() const;

    // 現在の方角に３時間漕ぐ
    void Set3HoursAuto();
    void SetRaftMode(const bool arg);
    bool GetRaftMode() const;

    NSModel::Raft::ePosType GetPosType();
    void SetPosType(const NSModel::Raft::ePosType arg);

    // イカダが2メートル以内にあるか？
    bool CheckNearRaft(const D3DXVECTOR3& pos);
    int GetNearRaftId(const D3DXVECTOR3& pos);

    // 乗船しているイカダのID
    // -1は乗船していないを意味する？
    void Ride();

    // イカダをいくつ所有しているか
    int GetRaftCount();

    bool Intersect(const D3DXVECTOR3& pos, const D3DXVECTOR3& move);
    D3DXVECTOR3 WallSlide(const D3DXVECTOR3& pos, const D3DXVECTOR3& move, bool* bHit = nullptr);

    D3DXVECTOR3 GetRaftXYZ(const int id);
    float GetRaftRotateY(const int id);

    // 乗船中のイカダの耐久値
    int GetRaftDurability();

    // 乗船中のイカダの強化値
    int GetRaftLevel();

    bool Can3HoursAuto();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:

    D3DXVECTOR3 WallSlideSub(const D3DXVECTOR3& pos,
                             Mesh* mesh,
                             const D3DXVECTOR3& move,
                             bool* bHit);

    static VoyageManager* m_obj;

    VoyageManager();

    // イカダは複数存在できる
    std::map<int, Raft2> m_raftMap;
};

