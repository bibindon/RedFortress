#pragma once

#include <d3d9.h>
#include "Common.h"
#include "Sprite.h"
#include "Mesh.h"
#include "AnimMesh.h"
#include "Player.h"
#include "EnemyCube.h"
#include "Map.h"

#include "../../RedFortressTalk2D/Talk2D/Talk2D.h"
#include "../../RedFortressQuestSystem/QuestSystem/QuestSystem.h"
#include "../../RedFortressModel/Model/WeaponManager.h"
#include "../../RedFortressModel/Model/StatusManager.h"
#include "../../RedFortressModel/Model/PowereggDateTime.h"
#include "../../RedFortressModel/Model/Storehouse.h"
#include "../../RedFortressModel/Model/Inventory.h"
#include "../../RedFortressModel/Model/ItemManager.h"
#include "../../RedFortressChest/Chest/Chest.h"
#include "../../RedFortressCraft/Craft/Craft.h"
#include "../../RedFortressResearch/Research/Research.h"
#include "MenuManager.h"
#include "HudManager.h"
#include "CommandManager.h"
#include "MeshNoShade.h"
#include "EnemyManager.h"

#include <thread>
#include "Opening.h"
#include "Title.h"
#include "StoryManager.h"
#include "PatchTestManager2.h"
#include "VoyageManager.h"
#include "CraftManager.h"

class Title;

class SeqBattle
{
public:
    SeqBattle();
    ~SeqBattle();
    void Update(eSequence* sequence);
    void Render();

    eBattleState GetState() const;
private:
    void Confirm(eSequence* sequence);
    void UpdateCommon();
    void RenderCommon();

    // 2D要素は一番最後に描画する必要がある
    void RenderCommon2D();

    void RenderNormal();
    void RenderGameover();

    void UpdateDebug();
    void UpdatePerSecond();
    void OperateNormal(eSequence* sequence);
    Player* m_player { nullptr };
    eBattleState m_eState { eBattleState::NORMAL };

    Map* m_map = nullptr;
    EnemyManager* m_enemyManager = nullptr;

    std::deque<std::wstring> m_finishQuestQue;

    // エンディングを開始してほしいときにtrueにする
    bool m_bEndingRequest { false };

    //----------------------------------------------------------
    // 調べる
    //----------------------------------------------------------

    // 調べる、が可能な時
    bool m_bShowExamine = false;
    Sprite* m_spriteExamine = nullptr;

    // アイテムが拾える時
    bool m_bObtainable = false;

    // 投げて地面に落ちた武器を拾える時
    bool m_bObtainWeapon = false;

    // 話しかけられるとき
    bool m_bTalkable = false;

    //----------------------------------------------------------
    // 会話
    // TalkManagerがあったほうがよい？
    //----------------------------------------------------------
    NSTalkLib2::Talk* m_talk = nullptr;
    void OperateTalk();

    //----------------------------------------------------------
    // 紙芝居
    //----------------------------------------------------------
    StoryManager* m_story = nullptr;
    void OperateStory();

    //----------------------------------------------------------
    // メニュー
    //----------------------------------------------------------
    void OperateMenu(eSequence* sequence);
    MenuManager* m_menuManager = nullptr;

    //----------------------------------------------------------
    // ステータス表示
    //----------------------------------------------------------
    HudManager m_hudManager;

    //----------------------------------------------------------
    // 倉庫画面
    //----------------------------------------------------------
    void OperateStorehouse();
    NSStorehouseLib::StorehouseLib* m_storehouse = nullptr;
    bool m_bShowStorehouse = false;
    void ShowStorehouse();

    //----------------------------------------------------------
    // クラフト画面
    //----------------------------------------------------------
    void OperateCraft();
    CraftManager m_craft;

    //----------------------------------------------------------
    // コマンド画面
    //----------------------------------------------------------
    void OperateCommand();
    CommandManager m_commandManager;

    //----------------------------------------------------------
    // タイトル画面
    //----------------------------------------------------------
    void OperateTitle(eSequence* sequence);
    Title* m_title = nullptr;

    //----------------------------------------------------------
    // ゲームオーバー画面
    //----------------------------------------------------------
    void OperateGameover(eSequence* sequence);
    Sprite* m_spriteGameover = nullptr;
    int m_nDeadCounter = 0;

    //----------------------------------------------------------
    // ロード画面
    // TODO クラス化
    // 真っ暗画面で砂時計のくるくるを表示
    // セーブデータがなければ初期データを読み、
    // セーブデータがあればセーブデータを読む。
    //----------------------------------------------------------
    void InitLoad();
    void UpdateLoad();
    void InitializeAfterLoad();
    void RenderLoad();
    void FinalizeLoad();
    Sprite* m_sprLoadBack = nullptr;
    Sprite* m_sprLoadLogo = nullptr;
    Sprite* m_sprLoadLoading = nullptr;
    Sprite* m_sprLoadClock = nullptr;
    bool m_bSavedataExists = false;

    std::thread* m_loadThread = nullptr;
    std::atomic<bool> m_loadLoaded = false;

    //----------------------------------------------------------
    // オープニング画面
    // TODO 名前を変更
    //----------------------------------------------------------
    void OperateOpening();
    Opening* m_Opening = nullptr;

    //----------------------------------------------------------
    // 睡眠・気絶
    // 現実時間で5秒攻撃されなければパワーエッグ星の6時間後に移動する。
    // 睡眠中に攻撃されたら即死
    //----------------------------------------------------------
    void OperateSleep();
    void RenderSleep();

    //-------------------------------------------------------
    // フェードイン・アウト処理
    //-------------------------------------------------------

    void StartFadeInOut();
    void UpdateFadeInOut();
    void DrawFadeInOut();

    enum class eFadeSeq
    {
        NotStart, // 睡眠中ではない
        FadeOut, // 5秒かけてフェードアウト
        Sleep, // 黒背景
        FadeIn, // 目覚め。フェードイン
        Finish, // 完了
    };

    eFadeSeq m_eFadeSeq = eFadeSeq::NotStart;
    int m_fadeOutCount = 0;
    int m_fadeBlackCount = 0;
    int m_fadeInCount = 0;

    Ptr<Sprite> m_sprBlack = nullptr;

    //----------------------------------------------
    // クエスト関連処理
    //----------------------------------------------
    void OperateQuest(eSequence* sequence);

    //----------------------------------------------------
    // パッチテスト画面
    //----------------------------------------------------
    void OperatePatch();
    PatchTestManager2 m_patchManager2;

    //----------------------------------------------------
    // 植物採取
    //----------------------------------------------------
    void OperatePickPlant();
    void RenderPickPlant();

    //----------------------------------------------------
    // 伐採
    //----------------------------------------------------
    void OperateCutTree();
    void RenderCutTree();

    //----------------------------------------------------
    // 航海機能
    //----------------------------------------------------
    void OperateVoyage(eSequence* sequence);

    // 3時間航海
    void OperateVoyage3Hours();

    void OperateCreateTorch();

    //----------------------------------------------------
    // ポップアップ機能
    //----------------------------------------------------
    void OperatePopUp();
    void RenderPopUp();

    //----------------------------------------------------
    // 3時間休憩
    //----------------------------------------------------
    void OperateRest();
    void RenderRest();

};

