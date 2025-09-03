#include "CppUnitTest.h"
#include "../starman/VoyageManager.h"
#include "../../RedFortressModel/Model/Voyage.h"
#include "../starman/SharedObj.h"
#include "Util.h"
#include "../starman/SaveManager.h"

#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#if defined(NDEBUG)
#pragma comment(lib, "d3dx9.lib")
#else
#pragma comment(lib, "d3dx9d.lib")
#endif

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest2
{
    TEST_CLASS(RaftTest)
    {
    public:

        TEST_CLASS_INITIALIZE(Initialize)
        {
        }

        TEST_CLASS_CLEANUP(CleanUp)
        {
        }

        void Init()
        {
        }
        
        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod01)
        {
            Raft2 raft;
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod02)
        {
            Util::InitWin_DX9_DI8();
            auto voyage = NSStarmanLib::Voyage::Get();
            voyage->Init(_T("raft1.csv"));

            Raft2 raft;

            // Target
            raft.Init(1);

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod03)
        {
            Raft2 raft;

            // Target
            raft.Finalize();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod04)
        {
            Util::InitWin_DX9_DI8();
            SaveManager::Get()->LoadOrigin();
            Map* map = NEW Map();
            map->Init();
            SharedObj::SetMap(map);

            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto player = NEW Player();
            SharedObj::SetPlayer(player);

            Raft2 raft;
            raft.Init(1);

            // Target
            raft.Update();

            raft.Finalize();

            delete player;
            SharedObj::SetPlayer(nullptr);

            NSStarmanLib::Voyage::Destroy();

            delete map;
            SharedObj::SetMap(nullptr);

            Util::DestroyLibData();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod05)
        {
            Util::InitWin_DX9_DI8();
            SaveManager::Get()->LoadOrigin();

            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Map* map = NEW Map();
            map->Init();
            SharedObj::SetMap(map);

            auto player = NEW Player();
            SharedObj::SetPlayer(player);

            Raft2 raft;
            raft.Init(1);

            SharedObj::GetD3DDevice()->Clear(0,
                                             NULL,
                                             D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                                             D3DCOLOR_XRGB(40, 40, 80),
                                             1.0f,
                                             0);
            SharedObj::GetD3DDevice()->BeginScene();

            // Target
            raft.Draw();

            SharedObj::GetD3DDevice()->EndScene();
            SharedObj::GetD3DDevice()->Present(NULL, NULL, NULL, NULL);

            delete player;
            SharedObj::SetPlayer(nullptr);

            delete map;
            SharedObj::SetMap(nullptr);

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod06)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
            raft.SetSail(true);
            raft.SetSail(false);

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod07)
        {
            Util::InitWin_DX9_DI8();
            SaveManager::Get()->LoadOrigin();
            Map* map = NEW Map();
            map->Init();
            SharedObj::SetMap(map);

            auto player = NEW Player();
            SharedObj::SetPlayer(player);

            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
            D3DXVECTOR3 _move(0.f, 0.f, 0.f);
            raft.PullOarBoth(&_move);

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();

            delete player;
            SharedObj::SetPlayer(nullptr);

            delete map;
            SharedObj::SetMap(nullptr);

            Util::DestroyLibData();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod08)
        {
            Util::InitWin_DX9_DI8();
            SaveManager::Get()->LoadOrigin();
            Map* map = NEW Map();
            map->Init();
            SharedObj::SetMap(map);

            auto player = NEW Player();
            SharedObj::SetPlayer(player);

            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
            raft.PullOarLeft();

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();

            delete player;
            SharedObj::SetPlayer(nullptr);

            delete map;
            SharedObj::SetMap(nullptr);
            Util::DestroyLibData();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod09)
        {
            Util::InitWin_DX9_DI8();
            SaveManager::Get()->LoadOrigin();
            Map* map = NEW Map();
            map->Init();
            SharedObj::SetMap(map);

            auto player = NEW Player();
            SharedObj::SetPlayer(player);

            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
            raft.PullOarRight();

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();

            delete player;
            SharedObj::SetPlayer(nullptr);

            delete map;
            SharedObj::SetMap(nullptr);
            Util::DestroyLibData();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod10)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
            raft.GetPos();

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod11)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
            D3DXVECTOR3 pos;
            raft.SetPos(pos);

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod12)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
            raft.GetRotate();

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod13)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
            D3DXVECTOR3 rot;
            raft.SetRotate(rot);

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(RaftTest_TestMethod14)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            Raft2 raft;
            raft.Init(1);

            // Target
//            raft.GetMesh();

            raft.Finalize();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }
    };

    TEST_CLASS(VoyageTest)
    {
    public:
        
        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod01)
        {
            // Target
            auto voyageMgr = VoyageManager::Get();

            VoyageManager::Destroy();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod02)
        {
            Util::InitWin_DX9_DI8();

            auto voyageMgr = VoyageManager::Get();

            // Target
            voyageMgr->Init();

            VoyageManager::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod03)
        {
            Util::InitWin_DX9_DI8();
            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            // Target
            voyageMgr->Finalize();

            VoyageManager::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod04)
        {
            auto voyageMgr = VoyageManager::Get();

            // Target
            voyageMgr->Update();

            VoyageManager::Destroy();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod05)
        {
            Util::InitWin_DX9_DI8(true);
            SaveManager::Get()->LoadOrigin();
            Map* map = NEW Map();
            map->Init();
            SharedObj::SetMap(map);
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            auto player = NEW Player();
            SharedObj::SetPlayer(player);

            SharedObj::GetD3DDevice()->Clear(0,
                                             NULL,
                                             D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
                                             D3DCOLOR_XRGB(40, 40, 80),
                                             1.0f,
                                             0);

            SharedObj::GetD3DDevice()->BeginScene();

            // Target
            voyageMgr->Draw();

            SharedObj::GetD3DDevice()->EndScene();
            SharedObj::GetD3DDevice()->Present(NULL, NULL, NULL, NULL);

            delete player;
            SharedObj::SetPlayer(nullptr);

            delete map;
            SharedObj::SetMap(nullptr);
            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();

            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod06)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            // Target
            voyageMgr->GetSail();

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod07)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            // Target
            voyageMgr->SetSail(true);
            voyageMgr->SetSail(false);

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod08)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();

            // Target
            voyageMgr->Set3HoursAuto();

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod09)
        {
            auto voyageMgr = VoyageManager::Get();

            // Target
            voyageMgr->SetRaftMode(true);
            voyageMgr->SetRaftMode(false);

            VoyageManager::Destroy();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod10)
        {
            auto voyageMgr = VoyageManager::Get();

            // Target
            voyageMgr->GetRaftMode();

            VoyageManager::Destroy();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod11)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();

            // Target
            voyageMgr->GetPosType();

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod12)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();

            // Target
            voyageMgr->SetPosType(NSStarmanLib::Raft::ePosType::River);
            voyageMgr->SetPosType(NSStarmanLib::Raft::ePosType::Sea);

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod13)
        {
            auto voyageMgr = VoyageManager::Get();

            // Target
            D3DXVECTOR3 pos;
            voyageMgr->CheckNearRaft(pos);

            VoyageManager::Destroy();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod14)
        {
            auto voyageMgr = VoyageManager::Get();

            // Target
            D3DXVECTOR3 pos;
            voyageMgr->GetNearRaftId(pos);

            VoyageManager::Destroy();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod17)
        {
            auto voyageMgr = VoyageManager::Get();

            // Target
            voyageMgr->GetRaftCount();

            VoyageManager::Destroy();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod18)
        {
            auto voyageMgr = VoyageManager::Get();

            // Target
            D3DXVECTOR3 pos;
            D3DXVECTOR3 move;
            voyageMgr->Intersect(pos, move);

            VoyageManager::Destroy();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod19)
        {
            Util::InitWin_DX9_DI8();

            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            // Target
            D3DXVECTOR3 pos;
            D3DXVECTOR3 move;
            voyageMgr->WallSlide(pos, move);

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod20)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            // Target
            voyageMgr->GetRaftXYZ(1);

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod21)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            // Target
            voyageMgr->GetRaftRotateY(1);

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod22)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            // Target
            voyageMgr->GetRaftDurability();

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }

        // 通常ケースで例外が起きないことを確認するテスト。単純にpublic関数を実行するだけ
        TEST_METHOD(VoyageTest_TestMethod23)
        {
            Util::InitWin_DX9_DI8();
            NSStarmanLib::Voyage::Get()->Init(_T("raft1.csv"));
            NSStarmanLib::Voyage::Get()->SetRaftCurrentId(1);

            auto voyageMgr = VoyageManager::Get();
            voyageMgr->Init();

            // Target
            voyageMgr->GetRaftLevel();

            VoyageManager::Destroy();
            NSStarmanLib::Voyage::Destroy();
            Util::ReleaseWin_DX9_DI8();
        }
    };
}
