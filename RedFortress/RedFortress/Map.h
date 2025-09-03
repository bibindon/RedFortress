#pragma once
#include <unordered_map>
#include <thread>
#include "Mesh.h"
#include "EnemyBase.h"
#include "MeshNoShade.h"
#include "MeshClone.h"
#include "../../RedFortressModel/Model/ItemManager.h"
#include "../../RedFortressModel/Model/StatusManager.h"
#include "../../RedFortressModel/Model/MapObjManager.h"

enum class eMapObjType
{
    TREE = 1,
    WATER,
    ROCK_WITH_DEPRESSIONS,
    GRASS,
    PLANT,
    ROCK,
    DONGURI,
    PLANT_MANY,
    YASHI,
    SOTETSU,
    STONES,
    BIG_TREE,
    THIN_TREE,
};

class Map
{
public:
    Map();
    ~Map();
    void Init();
    void Update();
    void Render();
    std::vector<EnemyBase*> GetEnemy();
    void SetEnemy(const std::vector<EnemyBase*>& vecEnemy);

    // 衝突判定
    bool Intersect(const D3DXVECTOR3& pos, const D3DXVECTOR3& rot);
    bool IntersectSub(const D3DXVECTOR3& pos, const D3DXVECTOR3& rot, Mesh* mesh);

    // 高速化バージョン
    bool IntersectSub2(const D3DXVECTOR3& pos, const D3DXVECTOR3& rot, Mesh* mesh);
    bool IntersectSub(const D3DXVECTOR3& pos, const D3DXVECTOR3& rot, MeshClone* mesh);

    // 海と接しているか
    bool IntersectWater(const D3DXVECTOR3& pos, const D3DXVECTOR3& rot);

    //--------------------------------------------------
    // 設置判定
    //--------------------------------------------------

    // 地面のみ
    bool CollisionGround(const D3DXVECTOR3& pos, const D3DXVECTOR3& move);
    bool CollisionGroundSub(const D3DXVECTOR3& pos, const D3DXVECTOR3& rot, Mesh* mesh);
    bool CollisionGroundSub(const D3DXVECTOR3& pos, const D3DXVECTOR3& rot, MeshClone* mesh);

    // 壁ずり処理
    D3DXVECTOR3 WallSlide(const D3DXVECTOR3& pos,
                          const D3DXVECTOR3& move,
                          bool* bHit,
                          bool* bInside,
                          bool friction = true);

    //------------------------------------
    // もの投げ
    //------------------------------------

    void AddThrownItem(const D3DXVECTOR3& pos,
                       const D3DXVECTOR3& move,
                       const std::wstring& weaponName,
                       const NSStarmanLib::ItemInfo& itemInfo,
                       const float scale = 1.f,
                       const float power = 10.f,
                       const float rotY = 0.f);

    // 投げて地面に落ちた武器のうち近くのものを1つ取得（半径2メートル以内）
    NSStarmanLib::ItemInfo GetThrownItem(const D3DXVECTOR3& pos);

    // 投げて地面に落ちた武器を削除
    void DeleteThrownItem(const NSStarmanLib::ItemInfo& thrownItem);

    //------------------------------------
    // 魔法攻撃
    //------------------------------------

    // 魔法攻撃は武器を投げるのと同じような処理を行う
    // 武器を投げた時と違い、地面に落ちたら消える。
    void SetThrownMagic(const D3DXVECTOR3& pos,
                        const D3DXVECTOR3& move,
                        const NSStarmanLib::eMagicType& magicType,
                        const float power);

    // 草の近くにいるか
    bool NearPlant(const D3DXVECTOR3& pos);

    // ココナツの木の近くにいるか
    bool NearCoconut(const D3DXVECTOR3& pos);

    // ソテツの木が近くにあるか
    bool NearSotetsu(const D3DXVECTOR3& pos);

    // 細い木の近くにいるか
    bool NearThinTree(const D3DXVECTOR3& pos);

    // 木の近くにいるか
    bool NearTree(const D3DXVECTOR3& pos);

    // どんぐりの山の近くにいるか
    bool NearDonguri(const D3DXVECTOR3& pos);

    // 砂利の近くにいるか
    bool NearStone(const D3DXVECTOR3& pos);

    // 海岸洞窟のあたりにいるか
    bool NearKaiganDoukutsu(const D3DXVECTOR3& pos);

    // 港跡のあたりにいるか
    bool NearMinatoato(const D3DXVECTOR3& pos);

    // ２メートル以内の木を１本消す
    void DeleteThinTree(const D3DXVECTOR3& pos);

    // ２メートル以内の草を１本消す
    void DeletePlant(const D3DXVECTOR3& pos);

    // ２メートル以内のマップオブジェクトを１個消す
    void DeleteObj(const D3DXVECTOR3& pos, const eMapObjType eMapObjType_);

    int GetProgress();

    bool IsFinishCaveInFade() const { return m_bInCaveFadeFinish; }

    // 死体の近くにいるか
    bool NearDead(const D3DXVECTOR3& pos);

    // 死体の近くのアイテムを取得した
    void SetDeadItem(const D3DXVECTOR3& pos);

    // 
    void ResetShow();

private:

    NSStarmanLib::MapObjManager* MapLib();

    // bInside : 壁の中にいる場合にtrue
    // bEnableWallWalk : 通常、3Dモデルの内側から外側へ移動しようとした場合は、衝突判定を行わない。
    // 岩や木の中に入ってしまったときに、出られなくなってしまうため。
    // しかし、一部のオブジェクトでは内側から外側への移動を禁止したい。そのために使うフラグ
    D3DXVECTOR3 WallSlideSub(const D3DXVECTOR3& pos,
                             Mesh* mesh,
                             const D3DXVECTOR3& move,
                             bool* bHit,
                             bool* bInside,
                             const bool bEnableWallWalk = true);

    D3DXVECTOR3 WallSlideSub(const D3DXVECTOR3& pos,
                             MeshClone* mesh,
                             const D3DXVECTOR3& move,
                             bool* bHit,
                             bool* bInside);

    std::unordered_map<std::wstring, Mesh*> m_meshMap;

    std::vector<EnemyBase*> m_vecEnemy { };

    // 太陽
    MeshNoShade* m_pSun = nullptr;

    // NPC
    std::unordered_map<std::wstring, AnimMesh*> m_NPC;

    std::unordered_map<int, Ptr<MeshClone>> m_meshCloneMap;

    //-------------------------------------------------------
    // 投げられたもの（武器）
    //-------------------------------------------------------

    // 投げるもの
    struct ThrownItem
    {
        NSStarmanLib::ItemInfo m_itemInfo;
        D3DXVECTOR3 m_move;
        MeshClone* m_mesh = nullptr;

        // 一度でも敵にヒットした
        bool m_bHit = false;

        // 地面に落ちた
        bool m_bStop = false;

        // 0.5秒待ってから発射
        int m_counter = 0;

        // 威力
        float m_power = 0.f;
    };

    std::vector<ThrownItem> m_thrownList;

    //-------------------------------------------------------
    // 投げられたもの（魔法）
    //-------------------------------------------------------

    // 投げるもの（魔法）
    struct ThrownMagic
    {
        NSStarmanLib::eMagicType m_eMagicType = NSStarmanLib::eMagicType::None;
        D3DXVECTOR3 m_move;

        MeshNoShade* m_mesh = nullptr;

        // 一度でも敵にヒットした
        bool m_bHit = false;

        // 1秒待ってから発射
        int m_counter = 0;

        // 威力
        float m_magicPower = 0.f;
    };

    std::vector<ThrownMagic> m_thrownMagicList;

    // ロード処理の進み具合
    std::atomic<int> m_progress = 0;

    // 洞窟内に入ったら３秒かけて暗くする。３秒経過し暗くするのが完了したことのフラグ
    bool m_bInCaveFadeFinish = true;

    // 洞窟の死体で採取を行った
    // ゲームを再起動したらアイテムが復活する
    bool m_bDeadItem1Exist = true;
    bool m_bDeadItem2Exist = true;
    bool m_bDeadItem3Exist = true;
    bool m_bDeadItem4Exist = true;
    bool m_bDeadItem5Exist = true;

    bool m_bInitialized = false;

    void UpdateNpcPos(const std::wstring& npcId);
};

