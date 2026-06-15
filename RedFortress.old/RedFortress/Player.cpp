#include "Player.h"
#include "SoundEffect.h"
#include "Camera.h"
#include "SharedObj.h"
#include "../../RedFortressModel/Model/StatusManager.h"
#include "KeyBoard.h"
#include "Mouse.h"
#include "GamePad.h"
#include "../../RedFortressModel/Model/MapObjManager.h"
#include "../../RedFortressModel/Model/Inventory.h"
#include "../../RedFortressModel/Model/WeaponManager.h"
#include "../../RedFortressModel/Model/Rynen.h"
#include "PopUp2.h"
#include "resource.h"
#include "BGM.h"
#include "NpcManager.h"
#include <cassert>

Player::Player()
{
    D3DXVECTOR3 pos = D3DXVECTOR3(0, 0, 0);
    D3DXVECTOR3 rot = D3DXVECTOR3(0, D3DX_PI, 0);
    AnimSetMap animSetMap;
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 0.f;
        animSetting.m_duration = 0.47f;
        animSetting.m_loop = true;
        animSetMap[_T("0_Idle")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 1.f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = false;
        animSetMap[_T("Walk")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 2.f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = false;
        animSetMap[_T("Attack")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 3.f;
        animSetting.m_duration = 0.47f;
        animSetting.m_loop = false;
        animSetMap[_T("Damaged")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 3.5f;
        animSetting.m_duration = 0.47f;
        animSetting.m_loop = false;
        animSetting.m_stopEnd = true;
        animSetMap[_T("Dead")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 5.f;
        animSetting.m_duration = 1.9f;
        animSetting.m_loop = false;
        animSetMap[_T("Jump")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 7.0f;
        animSetting.m_duration = 0.66f;
        animSetting.m_loop = true;
        animSetMap[_T("Sit")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 7.7f;
        animSetting.m_duration = 0.27f;
        animSetting.m_loop = false;
        animSetting.m_stopEnd = true;
        animSetMap[_T("LieDown")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 8.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = true;
        animSetMap[_T("IdleWater")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 9.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = true;
        animSetMap[_T("Swim")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 10.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = false;
        animSetMap[_T("Magic")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 11.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = false;
        animSetMap[_T("Arrow")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 12.0f;
        animSetting.m_duration = 1.97f;
        animSetting.m_loop = false;
        animSetMap[_T("Throw")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 14.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = false;
        animSetMap[_T("Step")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 16.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = false;
        animSetting.m_stopEnd = true;
        animSetMap[_T("PullOar")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 19.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = true;
        animSetMap[_T("TankaIdle")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 20.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = true;
        animSetMap[_T("TankaWalk")] = animSetting;
    }
    {
        AnimSetting animSetting { };
        animSetting.m_startPos = 21.0f;
        animSetting.m_duration = 0.97f;
        animSetting.m_loop = true;
        animSetMap[_T("TankaRest")] = animSetting;
    }
    m_AnimMesh2 = Ptr<AnimMesh>(NEW AnimMesh(_T("res\\model\\hoshiman.x"), pos, rot, 1.f, animSetMap));
    m_AnimMesh2->SetAnim(_T("0_Idle"));
    SoundEffect::get_ton()->load(_T("res\\sound\\attack01.wav"));

    // 読み込み処理の前に走ってしまうので以下のような書き方でXファイルの名前を参照できない。
    // 考え物である
//    auto weaponManager = NSModel::WeaponManager::GetObj();
//    auto weaponNameList = weaponManager->GetWeaponNameList();
//
//    for (auto it = weaponNameList.begin(); it != weaponNameList.end(); ++it)
//    {
//        auto weaponName = weaponManager->GetXfilename(*it);
//
//        D3DXVECTOR3 m_pos = D3DXVECTOR3(0.f, 0.f, 0.f);
//        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, 0.f);
//        Mesh* mesh = NEW Mesh(weaponName, m_pos, rot, 0.1f);
//        mesh->Init();
//        mesh->SetWeapon(true);
//        m_weaponMesh[*it] = mesh;
//    }

    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, 0.f);
        Mesh * mesh = NEW Mesh(_T("res\\model\\rock1.x"), pos, rot, 0.1f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w1")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\stick.x"), pos, rot, 1.f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w2")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\ax.x"), pos, rot, 1.0f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w7")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\atlatl.x"), pos, rot, 1.0f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w9")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\arrow.x"), pos, rot, 1.0f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w10")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\ironPipe.x"), pos, rot, 1.0f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w5")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\ironPipeEx.x"), pos, rot, 1.0f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w6")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\spear.x"), pos, rot, 1.0f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w3")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(D3DX_PI / 2, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\bow.x"), pos, rot, 1.0f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w4")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, -0.3f);
        D3DXVECTOR3 rot = D3DXVECTOR3(D3DX_PI / 2, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\arrow.x"), pos, rot, 1.0f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w8")] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, D3DX_PI);
        Mesh * mesh = NEW Mesh(_T("res\\model\\torch.x"), pos, rot, 1.f);
        mesh->Init();
        mesh->SetWeapon(true);
        m_weaponMesh[_T("w11")] = Ptr<Mesh>(mesh);
    }
    
    // 袋
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, 0.f);
        Mesh * mesh = NEW Mesh(L"res\\shader\\MeshShaderCullNone.fx",
                               _T("res\\model\\bag.x"), pos, rot, 1.0f);
        mesh->Init();
        m_bagMesh[NSModel::eBagPos::Back1] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, 0.f);
        Mesh * mesh = NEW Mesh(L"res\\shader\\MeshShaderCullNone.fx",
                               _T("res\\model\\bag.x"), pos, rot, 1.0f);
        mesh->Init();
        m_bagMesh[NSModel::eBagPos::Back2] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, 0.f);
        Mesh * mesh = NEW Mesh(L"res\\shader\\MeshShaderCullNone.fx",
                               _T("res\\model\\bag.x"), pos, rot, 1.0f);
        mesh->Init();
        m_bagMesh[NSModel::eBagPos::Front] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, 0.f);
        Mesh * mesh = NEW Mesh(L"res\\shader\\MeshShaderCullNone.fx",
                               _T("res\\model\\bag.x"), pos, rot, 1.0f);
        mesh->Init();
        m_bagMesh[NSModel::eBagPos::Left] = Ptr<Mesh>(mesh);
    }
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 0.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, 0.f);
        Mesh * mesh = NEW Mesh(L"res\\shader\\MeshShaderCullNone.fx",
                               _T("res\\model\\bag.x"), pos, rot, 1.0f);
        mesh->Init();
        m_bagMesh[NSModel::eBagPos::Right] = Ptr<Mesh>(mesh);
    }

    // 菅笠
    {
        D3DXVECTOR3 pos = D3DXVECTOR3(0.f, 2.f, 0.f);
        D3DXVECTOR3 rot = D3DXVECTOR3(0.f, 0.f, 0.f);
        Mesh * mesh = NEW Mesh(_T("res\\model\\sugegasa.blend.x"), pos, rot, 1.0f);
        mesh->Init();
        m_sugegasaMesh = Ptr<Mesh>(mesh);
    }

    float x = 0.f;
    float y = 0.f;
    float z = 0.f;
    NSModel::StatusManager::GetObj()->GetXYZ(&x, &y, &z);
    m_loadingPos.x = x;
    m_loadingPos.y = y + 1.f;
    m_loadingPos.z = z;

    SoundEffect::get_ton()->load(_T("res\\sound\\fireSet.wav"));
    SoundEffect::get_ton()->load(_T("res\\sound\\iceSet.wav"));
    SoundEffect::get_ton()->load(_T("res\\sound\\darkSet.wav"));

    D3DXCreateFont(SharedObj::GetD3DDevice(),
                   32,
                   0,
                   FW_NORMAL,
                   1,
                   false,
                   SHIFTJIS_CHARSET,
                   OUT_TT_ONLY_PRECIS,
                   CLEARTYPE_NATURAL_QUALITY,
                   FF_DONTCARE,
                   _T("BIZ UDMincho"),
                   &m_D3DFont);
}

Player::~Player()
{
}

void Player::Update(Map* map)
{
    m_bTryMove = false;

    //-----------------------------------------------
    // 海と接しているか
    //-----------------------------------------------
    {
        D3DXVECTOR3 pos = m_loadingPos;
        D3DXVECTOR3 down(0.f, -0.1f, 0.f);
        
        m_bUnderwater = map->IntersectWater(pos, down);
        if (m_bUnderwater)
        {
            m_AnimMesh2->SetAnim(_T("IdleWater"));
            auto status = NSModel::StatusManager::GetObj();
            status->SetPlayerAction(NSModel::StatusManager::PlayerState::IDLE_WATER);

            // 水中にいたら寝ることはない
            auto sleep = Common::Status()->GetSleep();

            if (sleep)
            {
                Common::Status()->SetSleep(false);
            }

            // 水中で脳のスタミナが0になったらおぼれて死ぬ
            if (status->GetBrainStaminaCurrent() <= 0.f)
            {
                if (!Common::Status()->GetDead())
                {
                    Common::Status()->SetDead(true);
                    Common::Status()->SetDeadReason(NSModel::eDeadReason::DROWNING);
                }
            }
        }
    }

    // 担架モードか
    {
        if (Common::Status()->IsStretcherMode())
        {
            if (Common::Status()->IsUnderwater())
            {
                m_AnimMesh2->SetAnim(_T("IdleWater"));
            }
            else
            {
                m_AnimMesh2->SetAnim(_T("TankaIdle"));
            }
        }
    }

    //-----------------------------------------------
    // 洞窟にいるか。
    // 
    // 洞窟の入り口の特定のエリアに触れたら洞窟内判定
    // 洞窟の入り口の特定のエリアに触れたら洞窟外判定、とする
    //-----------------------------------------------
    {
        D3DXVECTOR3 pos = m_loadingPos;

        D3DXVECTOR3 inCave(153.f, 475.f, -1392.f);

        D3DXVECTOR3 outCave(87.f, 483.f, -1393.f);

        if (!Common::Status()->GetInCave())
        {
            if (Common::HitByBoundingBox(pos, inCave, 40.f))
            {
                Common::Status()->SetInCave(true);
            }
        }
        else
        {
            if (Common::HitByBoundingBox(pos, outCave, 40.f))
            {
                Common::Status()->SetInCave(false);
            }
        }
    }

    //============================================================
    // Key Mouse Pad
    //============================================================

    //------------------------------------------------------------
    // Keyboard
    //------------------------------------------------------------

    D3DXVECTOR3 move { 0.f, 0.f, 0.f };
    D3DXVECTOR3 rotate { 0.f, 0.f, 0.f };
    float radian = Camera::GetRadian();
    float yaw = -1.f * (radian - (D3DX_PI / 2));

    if (SharedObj::KeyBoard()->IsDown(DIK_W))
    {
        // ジョギング
        if (SharedObj::KeyBoard()->IsDown(DIK_LSHIFT))
        {
            move.x += -std::sin(radian + D3DX_PI / 2) * 0.5f;
            move.z += std::sin(radian + D3DX_PI) * 0.5f;
            SetJogging();
        }
        // ダッシュ
        else if (SharedObj::KeyBoard()->IsDown(DIK_LCONTROL))
        {
            move.x += -std::sin(radian + D3DX_PI / 2);
            move.z += std::sin(radian + D3DX_PI);
            SetDash();
        }
        // 歩く
        else
        {
            move.x += -std::sin(radian + D3DX_PI / 2) * 0.1f;
            move.z += std::sin(radian + D3DX_PI) * 0.1f;
            SetWalk();
        }

        D3DXVECTOR3 rotate { 0.f, yaw, 0.f };
        SetRotate(rotate);
    }

    if (Common::DebugMode())
    {
        if (SharedObj::KeyBoard()->IsDown(DIK_G))
        {
            float magni = 200.f;

            // デバッグ目的でGキーだけ移動速度アップ
            move.x += -std::sin(radian + (D3DX_PI / 2)) * magni;
            move.z += std::sin(radian + D3DX_PI) * magni;

            D3DXVECTOR3 rotate{ 0.f, yaw, 0.f };
            SetRotate(rotate);
            SetWalk();

        }
    }

    if (SharedObj::KeyBoard()->IsDown(DIK_A))
    {
        // ジョギング
        if (SharedObj::KeyBoard()->IsDown(DIK_LSHIFT))
        {
            move.x += -std::sin(radian + D3DX_PI) * 0.5f;
            move.z += std::sin(radian + D3DX_PI * 3 / 2) * 0.5f;
            SetJogging();
        }
        // ダッシュ
        else if (SharedObj::KeyBoard()->IsDown(DIK_LCONTROL))
        {
            move.x += -std::sin(radian + D3DX_PI);
            move.z += std::sin(radian + D3DX_PI * 3 / 2);
            SetDash();
        }
        // 歩く
        else
        {
            move.x += -std::sin(radian + D3DX_PI) * 0.1f;
            move.z += std::sin(radian + D3DX_PI * 3 / 2) * 0.1f;
            SetWalk();
        }

        D3DXVECTOR3 rotate { 0.f, yaw + D3DX_PI * 3 / 2, 0.f };
        SetRotate(rotate);
    }

    if (SharedObj::KeyBoard()->IsDown(DIK_S))
    {
        // ジョギング
        if (SharedObj::KeyBoard()->IsDown(DIK_LSHIFT))
        {
            move.x += -std::sin(radian + D3DX_PI * 3 / 2) * 0.5f;
            move.z += std::sin(radian) * 0.5f;
            SetJogging();
        }
        // ダッシュ
        else if (SharedObj::KeyBoard()->IsDown(DIK_LCONTROL))
        {
            move.x += -std::sin(radian + D3DX_PI * 3 / 2);
            move.z += std::sin(radian);
            SetDash();
        }
        // 歩く
        else
        {
            move.x += -std::sin(radian + D3DX_PI * 3 / 2) * 0.1f;
            move.z += std::sin(radian) * 0.1f;
            SetWalk();
        }

        D3DXVECTOR3 rotate { 0.f, yaw + D3DX_PI, 0.f };
        SetRotate(rotate);
    }

    if (SharedObj::KeyBoard()->IsDown(DIK_D))
    {
        // ジョギング
        if (SharedObj::KeyBoard()->IsDown(DIK_LSHIFT))
        {
            move.x += -std::sin(radian) * 0.5f;
            move.z += std::sin(radian + D3DX_PI / 2) * 0.5f;
            SetJogging();
        }
        // ダッシュ
        else if (SharedObj::KeyBoard()->IsDown(DIK_LCONTROL))
        {
            move.x += -std::sin(radian);
            move.z += std::sin(radian + D3DX_PI / 2);
            SetDash();
        }
        // 歩く
        else
        {
            move.x += -std::sin(radian) * 0.2f;
            move.z += std::sin(radian + D3DX_PI / 2) * 0.2f;
            SetWalk();
        }

        D3DXVECTOR3 rotate { 0.f, yaw + D3DX_PI / 2, 0.f };
        SetRotate(rotate);
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_SPACE))
    {
        if (m_bUnderwater == false)
        {
            // 左、右、後ろが入力されているときにジャンプしようとしたらステップ移動
            if (SharedObj::KeyBoard()->IsDown(DIK_A))
            {
                SetStep(eDir::LEFT);
            }
            else if (SharedObj::KeyBoard()->IsDown(DIK_S))
            {
                SetStep(eDir::BACK);
            }
            else if (SharedObj::KeyBoard()->IsDown(DIK_D))
            {
                SetStep(eDir::RIGHT);
            }
            else
            {
                SetJump();
            }
        }
    }

    if (Common::DebugMode())
    {
        // スーパージャンプ
        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_J))
        {
            m_bJump = true;
            m_move.y = JUMP_INITIAL_VELOCITY * 5;
            m_AnimMesh2->SetAnim(_T("Jump"), 0.f);
        }

        // 木を消す
        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_Z))
        {
            auto mapObjManager = NSModel::MapObjManager::GetObj();
            std::vector<NSModel::stMapObj> mapObjs =
                mapObjManager->GetMapObjListR(m_loadingPos.x, m_loadingPos.z, 20);

            for (size_t i = 0; i < mapObjs.size(); ++i)
            {
                mapObjManager->SetVisible(mapObjs.at(i).m_frameX,
                                          mapObjs.at(i).m_frameZ,
                                          mapObjs.at(i).m_id,
                                          false);
            }
        }

        // 敵を消す
        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_X))
        {
            std::vector<EnemyBase*> vecEnemy = SharedObj::GetMap()->GetEnemy();

            for (std::size_t i = 0; i < vecEnemy.size(); i++)
            {
                D3DXVECTOR3 enemyPos(0.f, 0.f, 0.f);
                enemyPos = vecEnemy.at(i)->GetPos();

                D3DXVECTOR3 subPos = m_loadingPos - enemyPos;
                FLOAT distance = D3DXVec3Length(&subPos);

                if (distance <= 20.f)
                {
                    vecEnemy.at(i)->SetHP(0);
                }
            }

            SharedObj::GetMap()->SetEnemy(vecEnemy);
        }

        // マップ表示をリセットする
        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_F10))
        {
            SharedObj::GetMap()->ResetShow();
        }

        // 骨折を治す
        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_F9))
        {
            Common::Status()->SetFractureArm(false);
            Common::Status()->SetFractureLeg(false);
        }

        // 担架モード
        if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_F6))
        {
            auto tanka = Common::Status()->IsStretcherMode();
            if (tanka)
            {
                Common::Status()->SetStretcherMode(false);
            }
            else
            {
                Common::Status()->SetStretcherMode(true);
            }
        }
    }

    // Shift + F → もの投げ
    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_F))
    {
        if (SharedObj::KeyBoard()->IsDown(DIK_LSHIFT))
        {
            Throw();
        }
    }

    //------------------------------------------------------------
    // Mouse
    //------------------------------------------------------------

    if (Mouse::IsDownLeft())
    {
        if (SharedObj::KeyBoard()->IsDown(DIK_LSHIFT) == false)
        {
            bool ret = SetAttack();
        }
        else
        {
            // Shift + 左Click → 魔法
            SetMagic();
        }
    }

    // 魔法切り替え
    if (Mouse::IsWheelUp())
    {
        if (NSModel::Rynen::GetObj()->GetContracted())
        {
            auto status = NSModel::StatusManager::GetObj();
            auto magicType = status->GetMagicType();

            if (magicType == NSModel::eMagicType::None)
            {
                magicType = NSModel::eMagicType::Dark;
            }
            else if (magicType == NSModel::eMagicType::Fire)
            {
                magicType = NSModel::eMagicType::None;
            }
            else if (magicType == NSModel::eMagicType::Ice)
            {
                magicType = NSModel::eMagicType::Fire;
            }
            else if (magicType == NSModel::eMagicType::Dark)
            {
                magicType = NSModel::eMagicType::Ice;
            }

            status->SetMagicType(magicType);
            m_switchMagicCounter = 0;
            m_bSwitchMagic = true;
        }
    }
    else if (Mouse::IsWheelDown())
    {
        if (NSModel::Rynen::GetObj()->GetContracted())
        {
            auto status = NSModel::StatusManager::GetObj();
            auto magicType = status->GetMagicType();

            if (magicType == NSModel::eMagicType::None)
            {
                magicType = NSModel::eMagicType::Fire;
            }
            else if (magicType == NSModel::eMagicType::Fire)
            {
                magicType = NSModel::eMagicType::Ice;
            }
            else if (magicType == NSModel::eMagicType::Ice)
            {
                magicType = NSModel::eMagicType::Dark;
            }
            else if (magicType == NSModel::eMagicType::Dark)
            {
                magicType = NSModel::eMagicType::None;
            }

            status->SetMagicType(magicType);
            m_switchMagicCounter = 0;
            m_bSwitchMagic = true;
        }
    }

    //------------------------------------------------------------
    // GamePad
    //------------------------------------------------------------

    if (GamePad::IsLeftStickUsed())
    {
        float joyRadian = GamePad::GetLeftRadian();
        float cameRadian = Camera::GetRadian();
        float radian = joyRadian + (cameRadian - D3DX_PI * 3 / 2);

        // ジョギング
        if (GamePad::IsDown(eGamePadButtonType::L2))
        {
            move.x += std::cos(radian) * 0.5f;
            move.z += std::sin(radian) * 0.5f;
            SetJogging();
        }
        // ダッシュ
        else if (GamePad::IsDown(eGamePadButtonType::R2))
        {
            move.x += std::cos(radian);
            move.z += std::sin(radian);
            SetDash();
        }
        // 歩き
        else
        {
            move.x += std::cos(radian) * 0.2f;
            move.z += std::sin(radian) * 0.2f;
            SetWalk();
        }

        yaw = -1.f * (radian + D3DX_PI / 2);
        D3DXVECTOR3 rotate { 0.f, yaw, 0.f };
        SetRotate(rotate);

    }

    if (GamePad::IsDownFirst(eGamePadButtonType::R1))
    {
        if (GamePad::IsDown(eGamePadButtonType::R2))
        {
            SetMagic();
        }
        else
        {
            SetAttack();
        }
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::L1))
    {
        Throw();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::B))
    {
        if (m_bUnderwater == false)
        {
            float joyRadian = GamePad::GetLeftRadian();
            // 左、右、後ろが入力されているときにジャンプしようとしたらステップ移動
            // if (joyRadian < 1.f)
            if (GamePad::IsDown(eGamePadButtonType::LEFT))
            {
                SetStep(eDir::LEFT);
            }
            else if (GamePad::IsDown(eGamePadButtonType::DOWN))
            {
                SetStep(eDir::BACK);
            }
            else if (GamePad::IsDown(eGamePadButtonType::RIGHT))
            {
                SetStep(eDir::RIGHT);
            }
            else
            {
                SetJump();
            }
        }
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::UP) &&
        !GamePad::IsLeftStickUsed())
    {
        if (NSModel::Rynen::GetObj()->GetContracted())
        {
            auto status = NSModel::StatusManager::GetObj();
            auto magicType = status->GetMagicType();

            if (magicType == NSModel::eMagicType::None)
            {
                magicType = NSModel::eMagicType::Dark;
            }
            else if (magicType == NSModel::eMagicType::Fire)
            {
                magicType = NSModel::eMagicType::None;
            }
            else if (magicType == NSModel::eMagicType::Ice)
            {
                magicType = NSModel::eMagicType::Fire;
            }
            else if (magicType == NSModel::eMagicType::Dark)
            {
                magicType = NSModel::eMagicType::Ice;
            }

            status->SetMagicType(magicType);
            m_switchMagicCounter = 0;
            m_bSwitchMagic = true;
        }
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::DOWN) &&
        !GamePad::IsLeftStickUsed())
    {
        if (NSModel::Rynen::GetObj()->GetContracted())
        {
            auto status = NSModel::StatusManager::GetObj();
            auto magicType = status->GetMagicType();

            if (magicType == NSModel::eMagicType::None)
            {
                magicType = NSModel::eMagicType::Fire;
            }
            else if (magicType == NSModel::eMagicType::Fire)
            {
                magicType = NSModel::eMagicType::Ice;
            }
            else if (magicType == NSModel::eMagicType::Ice)
            {
                magicType = NSModel::eMagicType::Dark;
            }
            else if (magicType == NSModel::eMagicType::Dark)
            {
                magicType = NSModel::eMagicType::None;
            }

            status->SetMagicType(magicType);
            m_switchMagicCounter = 0;
            m_bSwitchMagic = true;
        }
    }

    //----------------------------------------------------------
    // Finalize
    //----------------------------------------------------------

    // ジャンプ中は移動方向を変えたり加速したりできない
    auto statusManager = NSModel::StatusManager::GetObj();
    if (m_bJump || m_bStep || statusManager->GetDead())
    {
        move = D3DXVECTOR3(0.f, 0.f, 0.f);
    }

    // 落下速度がある程度以上になったら移動できないようにする
    if (m_move.y <= -0.1f)
    {
        if (!Common::FasterMode())
        {
            move = D3DXVECTOR3(0.f, 0.f, 0.f);
        }
    }

    // 高度が1500を超えたら上昇をやめる
    if (m_loadingPos.y >= 1500.f)
    {
        move.y = -10.f;
    }

    m_move += move;

    float MAX_XZ_MOVE = 0.f;

    // 1フレームで50センチ以上移動しようとしたら50センチにする
    // ただし、XZ平面のみ。Y軸方向は加味しない。

    auto action = Common::Status()->GetPlayerAction();

    if (m_bUnderwater)
    {
        MAX_XZ_MOVE = 0.1f;
    }
    else
    {
        if (m_bStep)
        {
            MAX_XZ_MOVE = 1.f;
        }
        else
        {
            if (action == NSModel::StatusManager::PlayerState::WALK)
            {
                MAX_XZ_MOVE = 0.025f;
            }
            else if (action == NSModel::StatusManager::PlayerState::JOGGING)
            {
                MAX_XZ_MOVE = 0.075f;
            }
            else if (action == NSModel::StatusManager::PlayerState::SPRINTING)
            {
                MAX_XZ_MOVE = 0.125f;
            }
        }
    }

    if (Common::DebugMode())
    {
        if (SharedObj::KeyBoard()->IsDown(DIK_G) ||
            GamePad::IsDown(eGamePadButtonType::X))
        {
            if (m_bUnderwater)
            {
                MAX_XZ_MOVE *= 50000.f;
            }
            else
            {
                MAX_XZ_MOVE *= 100.f;
            }
        }
    }

    D3DXVECTOR2 move_XZ(m_move.x, m_move.z);
    FLOAT speed = D3DXVec2Length(&move_XZ);

    MAX_XZ_MOVE *= (statusManager->GetWalkSpeed());

    // もし100センチ移動しようとしていたら2で割ればよい。
    if (speed >= MAX_XZ_MOVE)
    {
        float work = 0.f;
        if (speed != 0.f)
        {
            work = MAX_XZ_MOVE / speed;
        }
        move_XZ *= work;
        m_move.x = move_XZ.x;
        m_move.z = move_XZ.y; // yをzに代入しているが正しい。
    }

    // XZ平面上の移動量は毎フレーム半分にする。ジャンプしているときは半分にしない。
    if (!m_bJump && !m_bStep)
    {
        m_move.x *= 0.8f;
        m_move.z *= 0.8f;
    }

    // 走って止まった時に上に浮かんでしまうのを防ぐ
    // ジャンプしておらず、移動していないのにY軸方向に移動している。
    if (!m_bJump && !m_bTryMove)
    {
        if (m_move.y > 0.05f)
        {
            m_move.y = 0.05f;
        }
    }

    if (map == nullptr)
    {
        return;
    }

    if (m_bAttack)
    {
        m_attackTimeCounter++;

        if (m_attackTimeCounter >= 30)
        {
            m_attackTimeCounter = 0;
            m_bAttack = false;
        }
    }

    if (m_bStep)
    {
        m_stepCounter++;
        if (m_stepCounter >= 15)
        {
            m_bStep = false;
            m_stepCounter = 0;
        }
    }

    if (m_bThrow)
    {
        m_throwTimeCounter++;

        if (m_throwTimeCounter >= 30)
        {
            m_throwTimeCounter = 0;
            m_bThrow = false;

            // 投げた後、インベントリに同一のアイテムがある場合、
            // 再度装備される。なければ素手になる。
            NSModel::Inventory* inventory = NSModel::Inventory::GetObj();
            std::vector<int> subIdList = inventory->GetSubIdList(m_throwItemId);
            if (subIdList.empty() == false)
            {
                NSModel::ItemInfo itemInfo =
                    inventory->GetItemInfo(m_throwItemId, subIdList.at(0));

                NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
                statusManager->SetEquipWeapon(itemInfo);
            }
        }
    }

    if (m_bMagic)
    {
        m_magicTimeCounter++;

        if (m_magicTimeCounter >= 90)
        {
            m_magicTimeCounter = 0;
            m_bMagic = false;
        }
    }

    if (m_bDamaged)
    {
        m_damagedTimeCounter++;

        if (m_damagedTimeCounter >= 30)
        {
            m_damagedTimeCounter = 0;
            m_bDamaged = false;
        }
    }

    // 重力
    m_move.y += -0.01f;

    // 壁ずり
    // 壁ずりは足に対してだけ行う
    bool bHit = false;
    bool bInside = false;

    // y方向に大きく移動していて、y方向の移動がなくなったら骨折
    float _y1 = m_move.y;
    float _y2 = 0.f;

    bool friction = true;
    if (m_bTryMove || m_bJump)
    {
        friction = false;
    }

    m_move = map->WallSlide(m_loadingPos, m_move, &bHit, &bInside, friction);

    m_bGround = bHit;
    _y2 = m_move.y;

    if (_y1 <= -1.f && _y2 > -1.f)
    {
        if (Common::DebugMode() && Common::StrongMode())
        {
            // Do nothing
        }
        else
        {
            unsigned int rand_ = SharedObj::GetRandom();

            if (rand_ % 101 < 25)
            {
                Common::Status()->SetFractureArm(true);
                PopUp2::Get()->SetText(IDS_STRING_ARM_BROKEN);
            }
            else if (rand_ % 101 < 50)
            {
                Common::Status()->SetFractureLeg(true);
                PopUp2::Get()->SetText(IDS_STRING_LEG_BROKEN);
            }
            else
            {
                Common::Status()->SetFractureArm(true);
                PopUp2::Get()->SetText(IDS_STRING_ARM_BROKEN);

                Common::Status()->SetFractureLeg(true);
                PopUp2::Get()->SetText(IDS_STRING_LEG_BROKEN);
            }
        }
    }

    if (!bInside)
    {
        // 足以外の衝突判定
        // 胴と頭
        D3DXVECTOR3 tmp = m_loadingPos;
        tmp.y += 0.8f;
        bool bHit2 = map->Intersect(tmp, m_move);
        if (bHit2)
        {
            m_move = D3DXVECTOR3(0.f, 0.f, 0.f);
        }

        tmp.y += 0.8f;
        bool bHit3 = map->Intersect(tmp, m_move);
        if (bHit3)
        {
            m_move = D3DXVECTOR3(0.f, 0.f, 0.f);
        }
    }

    // 足が何かに触れたらジャンプを解除する。
    // 壁ジャンプできてしまうが、ひとまずよしとする。
    // 何かに触れている状態が1秒続いたら再度ジャンプできる、でもよいかもしれない。
    if (bHit)
    {
        m_attachCount++;

        // 0.1秒?張り付き状態を維持したら再度ジャンプ可能になる
        // 緩やかな傾斜でbHitがtrue/falseを繰り返すことがある。うまくいかない。
//        if (m_attachCount >= 6)
        {
            m_bJumpEnable = true;
            m_bJump = false;
        }
    }
    else
    {
        m_attachCount = 0;
    }

    // イカダとの衝突判定
    // ぶつかっていたら移動量をゼロにする、のではなく、壁ずりを行う。
    // イカダの上を歩ける必要があるため。
    bool bHitRaft = false;
    m_move = SharedObj::Voyage()->WallSlide(m_loadingPos, m_move, &bHitRaft);

    if (bHitRaft)
    {
        if (m_bJump)
        {
            m_bJump = false;
            m_bJumpEnable = true;
        }
    }

    m_loadingPos += m_move;

    // 緊急手段
    // 頭から地面までの距離が1.5メートル以下だったら地面にめり込んでいる？
    D3DXVECTOR3 tmp2 = m_loadingPos;
    tmp2.y += 1.6f;
    bool bHit4 = map->Intersect(tmp2, D3DXVECTOR3(0.f, -1.3f, 0.f));
    if (bHit4)
    {
        m_loadingPos.y += 0.5f;
    }

    if (statusManager->GetDead())
    {
        SetDead();
        Common::Status()->SetInCave(false);
    }
}

void Player::Render()
{
    m_AnimMesh2->SetPos(m_loadingPos);
    m_AnimMesh2->SetRotate(m_rotate);
    m_AnimMesh2->Render();

    NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
    NSModel::ItemInfo itemInfo = statusManager->GetEquipWeapon();

    if (!itemInfo.GetId().empty())
    {
        NSModel::ItemManager* itemManager = NSModel::ItemManager::GetObj();
        NSModel::ItemDef itemDef = itemManager->GetItemDef(itemInfo.GetId());
        m_weaponMesh.at(itemDef.GetWeaponId())->Render();
    }

    auto bagState = Common::Status()->GetBagState();
    for (auto it = bagState.begin(); it != bagState.end(); ++it)
    {
        auto pos = m_loadingPos;

        if ((*it) == NSModel::eBagPos::Back1)
        {
            pos.y += 0.6f;
            pos.x += std::sin(m_rotate.y) * 0.1f;
            pos.z += std::cos(m_rotate.y) * 0.1f;

        }
        else if ((*it) == NSModel::eBagPos::Back2)
        {
            pos.y += 0.7f;
            pos.x += std::sin(m_rotate.y) * 0.5f;
            pos.z += std::cos(m_rotate.y) * 0.5f;
        }
        else if ((*it) == NSModel::eBagPos::Front)
        {
            pos.y += 0.5f;
            pos.x += std::sin(m_rotate.y + D3DX_PI) * 0.5f;
            pos.z += std::cos(m_rotate.y + D3DX_PI) * 0.5f;
        }
        else if ((*it) == NSModel::eBagPos::Left)
        {
            pos.y += 0.1f;
            pos.x += std::sin(m_rotate.y + (D3DX_PI/2.f)) * 0.5f;
            pos.z += std::cos(m_rotate.y + (D3DX_PI/2.f)) * 0.5f;
        }
        else if ((*it) == NSModel::eBagPos::Right)
        {
            pos.y += 0.1f;
            pos.x += std::sin(m_rotate.y - (D3DX_PI/2.f)) * 0.5f;
            pos.z += std::cos(m_rotate.y - (D3DX_PI/2.f)) * 0.5f;
        }

        if (Common::Status()->GetPlayerAction() == NSModel::StatusManager::PlayerState::SIT ||
            Common::Status()->GetPlayerAction() == NSModel::StatusManager::PlayerState::LYING_DOWN)
        {
            pos.y -= 0.4f;
        }
        else if (Common::Status()->GetPlayerAction() == NSModel::StatusManager::PlayerState::IDLE_WATER ||
                 Common::Status()->GetPlayerAction() == NSModel::StatusManager::PlayerState::SWIM)
        {
            pos.y -= 1.0f;
        }

        m_bagMesh[(*it)]->SetPos(pos);
        m_bagMesh[(*it)]->SetRotY(m_rotate.y);
        m_bagMesh[(*it)]->Render();
    }

    if (Common::Status()->GetSugegasa())
    {
        auto pos = m_loadingPos;

        if (Common::Status()->GetPlayerAction() == NSModel::StatusManager::PlayerState::SIT)
        {
            if (VoyageManager::Get()->GetRaftMode())
            {
                pos.y += 1.2f;
            }
            else
            {
                pos.y += 1.0f;
            }
        }
        else if (Common::Status()->GetPlayerAction() == NSModel::StatusManager::PlayerState::LYING_DOWN)
        {
            pos.y += 0.2f;
        }
        else if (Common::Status()->GetPlayerAction() == NSModel::StatusManager::PlayerState::IDLE_WATER)
        {
            pos.y += 0.6f;
        }
        else if (Common::Status()->GetPlayerAction() == NSModel::StatusManager::PlayerState::SWIM)
        {
            pos.y += 0.4f;
        }
        else
        {
            pos.y += 1.4f;
        }

        m_sugegasaMesh->SetPos(pos);
        m_sugegasaMesh->SetRotY(m_rotate.y);

        m_sugegasaMesh->Render();
    }
}

void Player::Render2D()
{
    if (m_bSwitchMagic)
    {
        ++m_switchMagicCounter;
        if (m_switchMagicCounter >= 60)
        {
            m_bSwitchMagic = false;
        }
        int transparency = 255 - (m_switchMagicCounter * 255 / 60);

        std::wstring magicType = Common::LoadString_(IDS_STRING137);

        auto statusManager = NSModel::StatusManager::GetObj();
        auto equipMagic = statusManager->GetMagicType();
        if (equipMagic == NSModel::eMagicType::Fire)
        {
            magicType = Common::LoadString_(IDS_STRING138);
        }
        else if (equipMagic == NSModel::eMagicType::Ice)
        {
            magicType = Common::LoadString_(IDS_STRING139);
        }
        else if (equipMagic == NSModel::eMagicType::Dark)
        {
            magicType = Common::LoadString_(IDS_STRING140);
        }

        RECT rect;
        SetRect(&rect, 750, 300, 850, 360);
        m_D3DFont->DrawText(NULL,
                            magicType.c_str(),
                            -1,
                            &rect,
                            DT_CENTER,
                            D3DCOLOR_ARGB(transparency, 255, 255, 255));
    }
}

void Player::SetPos(const D3DXVECTOR3& pos)
{
    m_loadingPos = pos;
}

D3DXVECTOR3 Player::GetPos() const
{
    return m_loadingPos;
}

void Player::SetMove(const D3DXVECTOR3& move)
{
    if (m_bJumpEnable && NSModel::StatusManager::GetObj()->GetDead() == false)
    {
        m_move = move;
    }
}

D3DXVECTOR3 Player::GetMove() const
{
    return m_move;
}

void Player::SetRotate(const D3DXVECTOR3& rotate)
{
    if (!m_bJump && !m_bStep && NSModel::StatusManager::GetObj()->GetDead() == false)
    {
        m_rotate = rotate;
    }
}

D3DXVECTOR3 Player::GetRotate() const
{
    return m_rotate;
}

bool Player::SetAttack()
{
    if (m_bAttack || m_bDamaged || NSModel::StatusManager::GetObj()->GetDead())
    {
        return false;
    }

    if (Common::Status()->IsStretcherMode())
    {
        return false;
    }

    auto statusManager = NSModel::StatusManager::GetObj();

    // 右手にバッグを持っているときは攻撃できない。
    if (statusManager->GetBag(NSModel::eBagPos::Right).GetId().size() >= 1)
    {
        return false;
    }

    statusManager->ConsumeAttackCost();

    auto itemManager = NSModel::ItemManager::GetObj();
    auto weaponId = statusManager->GetEquipWeapon().GetId();
    if (statusManager->GetEquipWeapon().GetId().size() >= 1)
    {
        auto itemDef = itemManager->GetItemDef(weaponId);
        auto weaponName = itemDef.GetName();
        if (weaponName == Common::LoadString_(IDS_STRING141))
        {
            SetAttackArrow();
            return true;
        }
        else if (weaponName == Common::LoadString_(IDS_STRING142))
        {
            SetAttackAtlatl();
            return true;
        }
    }

    SoundEffect::get_ton()->play(_T("res\\sound\\attack01.wav"), 90);
    m_AnimMesh2->SetAnim(_T("Attack"), 0.f);
    m_bAttack = true;

    D3DXVECTOR3 attackPos = GetAttackPos();
    D3DXVECTOR3 enemyPos { 0.f, 0.f, 0.f };
    std::vector<EnemyBase*> vecEnemy = SharedObj::GetMap()->GetEnemy();

    for (std::size_t i = 0; i < vecEnemy.size(); i++)
    {
        // 島民の霊は闇魔法でしかダメージを与えられない
        if (vecEnemy.at(i)->GetEnemyType() == eEnemyType::Ghost)
        {
            continue;
        }

        D3DXVECTOR3 enemyPos { 0.f, 0.f, 0.f };
        enemyPos = vecEnemy.at(i)->GetPos();

        D3DXVECTOR3 subPos { attackPos - enemyPos };
        FLOAT distance = D3DXVec3Length(&subPos);

        FLOAT range_ = 1.f;

        // スモールリッポウタイだったら当たりにくくする
        if (vecEnemy.at(i)->GetEnemyType() == eEnemyType::SmallCube)
        {
            range_ = 0.5f;
        }
        else if (vecEnemy.at(i)->GetEnemyType() == eEnemyType::BigCube)
        {
            range_ = 1.5f;
        }


        if (distance <= range_)
        {
            BGMManager::Get()->SetBattle(true);

            vecEnemy.at(i)->SetState(eEnemyState::DAMAGED);
            int hp = vecEnemy.at(i)->GetHP();

            float attackPower = statusManager->GetAttackPower();

            if (Common::StrongMode() && Common::DebugMode())
            {
                attackPower = 1000.f;
            }

            vecEnemy.at(i)->SetHP(hp - (int)attackPower);

            // 松明だったら一度で壊れる。火も消える
            auto itemInfo = Common::Status()->GetEquipWeapon();
            if (itemInfo.GetId().size() >= 1)
            {
                if (itemInfo.GetItemDef().GetName() == Common::LoadString_(IDS_STRING133))
                {
                    NSModel::WeaponManager::GetObj()->SetTorchLit(false);
                    Common::Inventory()->SetItemDurability(itemInfo.GetId(), itemInfo.GetSubId(), 0);
                }
            }
        }
    }

    SharedObj::GetMap()->SetEnemy(vecEnemy);

    return true;
}

bool Player::SetAttackArrow()
{
    // インベントリから弓矢の矢を一つ減らす。
    auto inventory = NSModel::Inventory::GetObj();
    auto itemManager = NSModel::ItemManager::GetObj();

    // 強化値の強い矢のほうからなくなる
    int arrowCnt = 0;
    arrowCnt = inventory->CountItem(L"arrow1");

    int arrowLevel = 0;

    NSModel::ItemDef itemDef;
    NSModel::ItemInfo itemInfo;

    if (arrowCnt > 0)
    {
        itemDef = itemManager->GetItemDef(L"arrow1");

        auto subIdList = inventory->GetSubIdList(itemDef.GetId());

        itemInfo = inventory->GetItemInfo(itemDef.GetId(), subIdList.at(0));

        inventory->RemoveItem(itemDef.GetId(), subIdList.at(0));
        arrowLevel = 1;
    }
    else
    {
        arrowCnt = inventory->CountItem(L"arrow");
        if (arrowCnt > 0)
        {
            itemDef = itemManager->GetItemDef(L"arrow");

            auto subIdList = inventory->GetSubIdList(itemDef.GetId());

            itemInfo = inventory->GetItemInfo(itemDef.GetId(), subIdList.at(0));

            inventory->RemoveItem(itemDef.GetId(), subIdList.at(0));
            arrowLevel = 0;
        }
        else
        {
            return false;
        }
    }

    m_AnimMesh2->SetAnim(_T("Arrow"), 0.f);

    int bowLevel = 0;
    {
        auto statusManager = NSModel::StatusManager::GetObj();
        NSModel::ItemInfo equip = statusManager->GetEquipWeapon();
        NSModel::ItemDef equip2 = itemManager->GetItemDef(equip.GetId());
        bowLevel = equip2.GetLevel();
        if (bowLevel == -1)
        {
            bowLevel = 0;
        }
    }

    m_bAttack = true;

    // 矢を投げるものとしてセット
    {
        D3DXVECTOR3 pos(m_loadingPos);
        pos.y += 0.85f;
        pos.x += std::sin(m_rotate.y + D3DX_PI) * 0.5f;
        pos.z += std::sin(m_rotate.y + (D3DX_PI * 3 / 2)) * 0.5f;

        D3DXVECTOR3 norm(0.f, 0.f, 0.f);
        norm.x = std::sin(m_rotate.y + D3DX_PI);
        norm.z = std::sin(m_rotate.y + (D3DX_PI * 3 / 2));

        norm *= 0.1f + (bowLevel * 0.1f) + (arrowLevel * 0.2f); // TODO 微調整
        norm.y = 0.05f;

        float power = 0.f;

        // TODO 微調整
        power = 0.01f;
        power += (bowLevel * 0.2f) + (arrowLevel * 10.f);

        SharedObj::GetMap()->AddThrownItem(pos, norm, Common::LoadString_(IDS_STRING143), itemInfo, 1.f, power, m_rotate.y);
    }

    // 体力を消耗する
    {
        auto statusManager = NSModel::StatusManager::GetObj();
        statusManager->ConsumeAttackCost();
    }
    return true;
}

bool Player::SetAttackAtlatl()
{
    // インベントリからアトラトルの矢を一つ減らす。
    auto inventory = NSModel::Inventory::GetObj();
    auto itemManager = NSModel::ItemManager::GetObj();

    // 強化値の強い槍のほうからなくなる
    // 強化値は-1,1,2,3,4,5の6種類
    int arrowLevel = 0;

    std::vector<std::wstring> spearLists;
    spearLists.push_back(L"spearForAtlatl");
    spearLists.push_back(L"spearForAtlatl1");
    spearLists.push_back(L"spearForAtlatl2");
    spearLists.push_back(L"spearForAtlatl3");
    spearLists.push_back(L"spearForAtlatl4");
    spearLists.push_back(L"spearForAtlatl5");

    for (int i = 5; ;)
    {
        int arrowCnt = inventory->CountItem(spearLists.at(i));
        if (arrowCnt >= 1)
        {
            arrowLevel = i;
            break;
        }

        --i;
        if (i == 0)
        {
            i = -1;
        }
        else if (i == -2)
        {
            break;
        }
    }

    if (arrowLevel == -2)
    {
        return false;
    }

    NSModel::ItemDef itemDef;
    NSModel::ItemInfo itemInfo;

    // インベントリから槍を一つ減らす
    {
        itemDef = itemManager->GetItemDef(spearLists.at(arrowLevel));
        auto subIdList = inventory->GetSubIdList(itemDef.GetId());
        itemInfo = inventory->GetItemInfo(itemDef.GetId(), subIdList.at(0));
        inventory->RemoveItem(itemDef.GetId(), subIdList.at(0));
    }

    m_AnimMesh2->SetAnim(_T("Attack"), 0.f);

    m_bAttack = true;

    // 矢を投げるものとしてセット
    {
        // 初期位置
        D3DXVECTOR3 pos(m_loadingPos);
        pos.y += 1.15f;
        pos.x += std::sin(m_rotate.y + D3DX_PI) * 0.5f;
        pos.z += std::sin(m_rotate.y + (D3DX_PI * 3 / 2)) * 0.5f;

        // 方向と速度
        D3DXVECTOR3 norm(0.f, 0.f, 0.f);
        norm.x = std::sin(m_rotate.y + D3DX_PI);
        norm.z = std::sin(m_rotate.y + (D3DX_PI * 3 / 2));

        norm *= 0.1f + (arrowLevel * 0.2f); // TODO 微調整
        norm.y = 0.05f;

        float power = 0.f;

        // TODO 微調整
        power = 20.f;
        power += (arrowLevel * 5.f);

        SharedObj::GetMap()->AddThrownItem(pos, norm, Common::LoadString_(IDS_STRING144), itemInfo, 2.f, power, m_rotate.y);
    }

    // 体力を消耗する
    {
        auto statusManager = NSModel::StatusManager::GetObj();
        statusManager->ConsumeAttackCost();
    }

    return true;
}

void Player::SetWalk()
{
    auto status = NSModel::StatusManager::GetObj();
    if (!m_bJump &&
        !m_bStep &&
        !status->GetDead() &&
        !m_bAttack)
    {
        if (m_bUnderwater)
        {
            m_AnimMesh2->SetAnim(_T("Swim"));
            status->SetPlayerAction(NSModel::StatusManager::PlayerState::SWIM);
        }
        else
        {
            if (Common::Status()->IsStretcherMode())
            {
                m_AnimMesh2->SetAnim(_T("TankaWalk"));
            }
            else
            {
                m_AnimMesh2->SetAnim(_T("Walk"));
            }
            status->SetPlayerAction(NSModel::StatusManager::PlayerState::WALK);
        }
    }

    m_bTryMove = true;
}

void Player::SetJogging()
{
    auto status = NSModel::StatusManager::GetObj();
    if (!m_bJump &&
        !m_bStep &&
        !status->GetDead() &&
        !m_bAttack)
    {
        if (m_bUnderwater)
        {
            m_AnimMesh2->SetAnim(_T("Swim"));
            status->SetPlayerAction(NSModel::StatusManager::PlayerState::SWIM);
        }
        else
        {
            if (Common::Status()->IsStretcherMode())
            {
                m_AnimMesh2->SetAnim(_T("TankaWalk"));
            }
            else
            {
                m_AnimMesh2->SetAnim(_T("Walk"));
            }
            status->SetPlayerAction(NSModel::StatusManager::PlayerState::JOGGING);
        }
    }

    m_bTryMove = true;
}

void Player::SetDash()
{
    auto status = NSModel::StatusManager::GetObj();
    if (!m_bJump &&
        !m_bStep &&
        !status->GetDead() &&
        !m_bAttack)
    {
        if (m_bUnderwater)
        {
            m_AnimMesh2->SetAnim(_T("Swim"));
            status->SetPlayerAction(NSModel::StatusManager::PlayerState::SWIM);
        }
        else
        {
            if (Common::Status()->IsStretcherMode())
            {
                m_AnimMesh2->SetAnim(_T("TankaWalk"));
            }
            else
            {
                m_AnimMesh2->SetAnim(_T("Walk"));
            }
            status->SetPlayerAction(NSModel::StatusManager::PlayerState::SPRINTING);
        }
    }

    m_bTryMove = true;
}

void Player::SetIdle()
{
    m_AnimMesh2->SetAnim(_T("0_Idle"));
    Common::Status()->SetPlayerAction(NSModel::StatusManager::PlayerState::STAND);
}

void Player::SetDamaged()
{
    if (!NSModel::StatusManager::GetObj()->GetDead())
    {
        SoundEffect::get_ton()->play(_T("res\\sound\\damage01.wav"), 90);
        m_AnimMesh2->SetAnim(_T("Damaged"), 0.f);
        m_bDamaged = true;
    }

    // 攻撃食らったら回復
    if (Common::StrongMode() && Common::DebugMode())
    {
        Common::Status()->SetMuscleCurrent(100);
        Common::Status()->SetBodyStaminaCurrent(100);
        Common::Status()->SetBodyStaminaMaxSub(100);
        Common::Status()->SetBrainStaminaCurrent(100);
        Common::Status()->SetBrainStaminaMaxSub(100);
        Common::Status()->SetWaterCurrent(100);
    }

    // 睡眠中に攻撃されたら即死
    if (NSModel::StatusManager::GetObj()->GetSleep())
    {
        NSModel::StatusManager::GetObj()->SetDead(true);
        NSModel::StatusManager::GetObj()->SetDeadReason(NSModel::eDeadReason::ATTACK_ON_SLEEP);
    }
}

void Player::SetDead()
{
    m_AnimMesh2->SetAnim(_T("Dead"), 0.f);

    if (Common::Status()->IsStretcherMode())
    {
        // 担架モードで水中だったら、NPCが全員死亡
        if (IsUnderWater())
        {
            Common::Status()->SetStretcherMode(false);

            auto npcStatus = NpcManager::Get()->GetNpcStatus(L"daikeiman");
            npcStatus.SetDead();
            NpcManager::Get()->SetNpcStatus(L"daikeiman", npcStatus);

            npcStatus = NpcManager::Get()->GetNpcStatus(L"sankakuman");
            npcStatus.SetDead();
            NpcManager::Get()->SetNpcStatus(L"sankakuman", npcStatus);

            npcStatus = NpcManager::Get()->GetNpcStatus(L"shikakuman");
            npcStatus.SetDead();
            NpcManager::Get()->SetNpcStatus(L"shikakuman", npcStatus);
        }
        // 水中じゃなくても、NPCが全員死亡
        else
        {
            Common::Status()->SetStretcherMode(false);

            auto npcStatus = NpcManager::Get()->GetNpcStatus(L"daikeiman");
            npcStatus.SetDead();
            NpcManager::Get()->SetNpcStatus(L"daikeiman", npcStatus);

            npcStatus = NpcManager::Get()->GetNpcStatus(L"sankakuman");
            npcStatus.SetDead();
            NpcManager::Get()->SetNpcStatus(L"sankakuman", npcStatus);

            npcStatus = NpcManager::Get()->GetNpcStatus(L"shikakuman");
            npcStatus.SetDead();
            NpcManager::Get()->SetNpcStatus(L"shikakuman", npcStatus);
        }
    }
}

bool Player::GetDead() const
{
    return NSModel::StatusManager::GetObj()->GetDead();
}

void Player::SetSleep(const bool arg)
{
    if (arg)
    {
        if (Common::Status()->IsStretcherMode())
        {
            m_AnimMesh2->SetAnim(_T("TankaRest"), 0.f);
        }
        else
        {
            m_AnimMesh2->SetAnim(_T("LieDown"), 0.f);
        }
        auto status = NSModel::StatusManager::GetObj();
        status->SetPlayerAction(NSModel::StatusManager::PlayerState::LYING_DOWN);
        status->SetSleep(true);
    }
    else
    {
        m_AnimMesh2->SetAnim(_T("0_Idle"), 0.f);
        auto status = NSModel::StatusManager::GetObj();
        status->SetPlayerAction(NSModel::StatusManager::PlayerState::STAND);
        status->SetSleep(false);
    }
}

D3DXVECTOR3 Player::GetAttackPos() const
{
    float length = 1.f;

    auto work = Common::Status()->GetEquipWeapon().GetId();

    if (!work.empty())
    {
        auto unreinforcedId = Common::ItemManager()->GetItemDef(work).GetUnreinforcedId();

        // 木の棒
        if (unreinforcedId == L"stick")
        {
            length = 1.5f;
        }
        // 石槍
        else if (unreinforcedId == L"stoneSpear")
        {
            length = 2.0f;
        }
        // 鉄パイプ
        else if (unreinforcedId == L"ironPipe")
        {
            length = 1.5f;
        }
        // 石付き鉄パイプ
        else if (unreinforcedId == L"ironPipeEx")
        {
            length = 1.5f;
        }
        // 石斧
        else if (unreinforcedId == L"stoneAxe")
        {
            length = 1.5f;
        }
    }

    D3DXVECTOR3 pos { m_loadingPos };
    D3DXVECTOR3 norm { 0.f, 0.f, 0.f };
    norm.x = std::sin(m_rotate.y + D3DX_PI);
    norm.z = std::sin(m_rotate.y + (D3DX_PI * 3 / 2));

    pos += norm * length;
    return pos;
}

void Player::SetJump()
{
    if (Common::Status()->IsStretcherMode())
    {
        return;
    }

    if (m_bJumpEnable && NSModel::StatusManager::GetObj()->GetDead() == false)
    {
        m_bJump = true;
        m_bJumpEnable = false;

        m_move.y = JUMP_INITIAL_VELOCITY;
        m_AnimMesh2->SetAnim(_T("Jump"), 0.f);

        int brokenBagNum1 = 0;
        int brokenBagNum2 = 0;
        {
            auto allBag = Common::Status()->GetAllBag();
            brokenBagNum1 = (int)std::count_if(allBag.begin(), allBag.end(),
                                               [&](auto x)
                                               {
                                                   return x.GetDurabilityCurrent() == 0;
                                               });
        }

        Common::Inventory()->ReduceEquipBagDurability();

        // 新たに耐久値が0になった袋があったか
        {
            auto allBag = Common::Status()->GetAllBag();
            brokenBagNum2 = (int)std::count_if(allBag.begin(), allBag.end(),
                                               [&](auto x)
                                               {
                                                   return x.GetDurabilityCurrent() == 0;
                                               });
        }

        if (brokenBagNum1 != brokenBagNum2)
        {
            PopUp2::Get()->SetText(IDS_STRING145);
        }

        SoundEffect::get_ton()->load(_T("res\\sound\\jump.wav"));
        SoundEffect::get_ton()->play(_T("res\\sound\\jump.wav"), 90);
    }
}

void Player::SetSit()
{
    if (Common::Status()->IsStretcherMode())
    {
        m_AnimMesh2->SetAnim(_T("TankaRest"), 0.f);
    }
    else
    {
        m_AnimMesh2->SetAnim(_T("Sit"), 0.f);
    }

    auto status = NSModel::StatusManager::GetObj();
    status->SetPlayerAction(NSModel::StatusManager::PlayerState::SIT);
}

void Player::SetLieDown()
{
    m_AnimMesh2->SetAnim(_T("LieDown"), 0.f);

    auto status = NSModel::StatusManager::GetObj();
    status->SetPlayerAction(NSModel::StatusManager::PlayerState::LYING_DOWN);
}

void Player::SetStep(const eDir dir)
{
    // 担架モードなら何もしない
    if (Common::Status()->IsStretcherMode())
    {
        return;
    }

    auto stamina = Common::Status()->GetBodyStaminaCurrent();
    if (stamina <= 30.f)
    {
        return;
    }

    const float STEP_VELOCITY = 1.f;
    float radian = Camera::GetRadian();
    float yaw = -1.f * (radian - (D3DX_PI / 2));
    D3DXVECTOR3 move(0.f, 0.f, 0.f);
    D3DXVECTOR3 rotate { 0.f, yaw, 0.f };

    if (dir == eDir::LEFT)
    {
        move.x += -std::sin(radian + D3DX_PI);
        move.z += std::sin(radian + D3DX_PI * 3 / 2);
        move *= STEP_VELOCITY;
    }
    else if (dir == eDir::BACK)
    {
        move.x += -std::sin(radian + D3DX_PI * 3 / 2);
        move.z += std::sin(radian);
        move *= STEP_VELOCITY;
    }
    else if (dir == eDir::RIGHT)
    {
        move.x += -std::sin(radian);
        move.z += std::sin(radian + D3DX_PI / 2);
        move *= STEP_VELOCITY;
    }
    SetMove(move);
    SetRotate(rotate);

    m_bStep = true;
    SoundEffect::get_ton()->load(_T("res\\sound\\jump.wav"));
    SoundEffect::get_ton()->play(_T("res\\sound\\jump.wav"), 90);

    Common::Status()->ConsumeJumpCost();

    m_AnimMesh2->SetAnim(_T("Step"), 0.f);
}

void Player::SetExamine()
{
    // 
}

void Player::RideRaft()
{
    auto id = SharedObj::Voyage()->GetNearRaftId(m_loadingPos);
    if (id == 0)
    {
        return;
    }

    auto pos = SharedObj::Voyage()->GetRaftXYZ(id);
    m_loadingPos = pos;
    m_loadingPos.y += 0.2f;

    auto rotateY = SharedObj::Voyage()->GetRaftRotateY(id);
    m_rotate.y = rotateY;

    m_AnimMesh2->SetAnim(_T("Sit"));
}

bool Player::IsInCave()
{
    return Common::Status()->GetInCave();
}

void Player::SetIsInCave(const bool arg)
{
    Common::Status()->SetInCave(arg);
}

bool Player::IsUnderWater() const
{
    return m_bUnderwater;
}

void Player::PlayPullOarAnim()
{
    m_AnimMesh2->SetAnim(_T("PullOar"), 0.f);
}

void Player::Throw()
{
    //-------------------------------------------------------------
    // 1. 素手になる。
    // 2. 殴るモーションを再生
    // 3. 装備アイテムが頭上から前方に飛んでいく。
    // 4. インベントリに同一のアイテムがある場合、再度装備される。なければ素手になる。
    //
    // ・投げられたものは物に衝突して停止する。
    // ・敵にあたった場合は、ダメージを与え、地面に落ちる。
    // ・地面に落ちた物は拾える。
    // ・投げる動作は2秒に一回
    // ・体力を消耗する
    //-------------------------------------------------------------

    auto statusManager = NSModel::StatusManager::GetObj();

    // 担架モードなら何もしない
    if (Common::Status()->IsStretcherMode())
    {
        return;
    }

    // 素手だったら何もしない。
    auto itemInfo = statusManager->GetEquipWeapon();
    if (itemInfo.GetId().empty())
    {
        return;
    }

    // 投げている最中なら何もしない
    if (m_bThrow)
    {
        return;
    }

    // インベントリから一つアイテムを減らす。
    auto inventory = NSModel::Inventory::GetObj();
    inventory->RemoveItem(itemInfo.GetId(), itemInfo.GetSubId());

    // 投げるものをセット
    {
        m_bThrow = true;

        D3DXVECTOR3 pos(m_loadingPos);
        pos.y += 2.f;

        D3DXVECTOR3 norm(0.f, 0.f, 0.f);
        norm.x = std::sin(m_rotate.y + D3DX_PI);
        norm.z = std::sin(m_rotate.y + (D3DX_PI * 3 / 2));

        norm *= 0.4f;
        norm.y = 0.1f;

        auto itemManager = NSModel::ItemManager::GetObj();
        auto itemDef = itemManager->GetItemDef(itemInfo.GetId());
        auto weaponManager = NSModel::WeaponManager::GetObj();
        std::wstring xfilename = weaponManager->GetXfilename2(itemDef.GetWeaponId());

        if (xfilename.find(_T("rock")) != std::wstring::npos)
        {
            SharedObj::GetMap()->AddThrownItem(pos, norm, itemDef.GetName(), itemInfo, 0.1f);
        }
        else
        {
            SharedObj::GetMap()->AddThrownItem(pos, norm, itemDef.GetName(), itemInfo);
        }

        m_throwItemId = itemInfo.GetId();
    }

    // 素手にする
    {
        NSModel::ItemInfo itemInfo;
        itemInfo.SetId(L"");
        statusManager->SetEquipWeapon(itemInfo);

        SoundEffect::get_ton()->play(_T("res\\sound\\attack01.wav"), 90);
        m_AnimMesh2->SetAnim(_T("Throw"), 0.f);
    }

    // 体力を消耗する
    statusManager->ConsumeAttackCost();
}

void Player::SetMagic()
{
    //-------------------------------------------------------------
    // 1. 殴るモーションを再生
    // 2. 火の玉が頭上から前方に飛んでいく。
    //
    // ・投げる動作は2秒に一回
    // ・体力を消耗する
    //-------------------------------------------------------------

    auto statusManager = NSModel::StatusManager::GetObj();

    auto magicType = statusManager->GetMagicType();
    if (magicType == NSModel::eMagicType::None)
    {
        return;
    }

    // 担架モードなら何もしない
    if (Common::Status()->IsStretcherMode())
    {
        return;
    }

    // 投げている最中なら何もしない
    if (m_bMagic)
    {
        return;
    }

    // 投げるものをセット
    {
        m_bMagic = true;

        D3DXVECTOR3 pos(m_loadingPos);
        pos.y += 1.f;
        pos.x += std::sin(m_rotate.y + D3DX_PI);
        pos.z += std::sin(m_rotate.y + (D3DX_PI * 3 / 2));

        D3DXVECTOR3 norm(0.f, 0.f, 0.f);
        norm.x = std::sin(m_rotate.y + D3DX_PI);
        norm.z = std::sin(m_rotate.y + (D3DX_PI * 3 / 2));

        norm *= 0.4f;
        norm.y = 0.1f;

        float power = 0.f;

        if (magicType == NSModel::eMagicType::Fire)
        {
            power = ((float)statusManager->GetLevelFire()+1) * 2;
        }
        else if (magicType == NSModel::eMagicType::Ice)
        {
            power = ((float)statusManager->GetLevelIce()+1) * 2;
        }
        else if (magicType == NSModel::eMagicType::Dark)
        {
            power = ((float)statusManager->GetLevelDark()+1) * 2;
        }

        SharedObj::GetMap()->SetThrownMagic(pos, norm, magicType, power);
    }

    if (magicType == NSModel::eMagicType::Fire)
    {
        SoundEffect::get_ton()->play(_T("res\\sound\\fireSet.wav"));
    }
    else if (magicType == NSModel::eMagicType::Ice)
    {
        SoundEffect::get_ton()->play(_T("res\\sound\\iceSet.wav"));
    }
    else if (magicType == NSModel::eMagicType::Dark)
    {
        SoundEffect::get_ton()->play(_T("res\\sound\\darkSet.wav"));
    }

    m_AnimMesh2->SetAnim(_T("Magic"), 0.f);

    // 体力を消耗する
    statusManager->UseMagic();
}

// 解像度やウィンドウモードを変更したときのための関数
void Player::OnDeviceLost()
{
    m_AnimMesh2->OnDeviceLost();
    for (auto& item : m_weaponMesh)
    {
        item.second->OnDeviceLost();
    }

    for (auto& item : m_bagMesh)
    {
        item.second->OnDeviceLost();
    }

    m_sugegasaMesh->OnDeviceLost();

    HRESULT hr = m_D3DFont->OnLostDevice();
    assert(hr == S_OK);
}

// 解像度やウィンドウモードを変更したときのための関数
void Player::OnDeviceReset()
{
    m_AnimMesh2->OnDeviceReset();
    for (auto& item : m_weaponMesh)
    {
        item.second->OnDeviceReset();
    }

    for (auto& item : m_bagMesh)
    {
        item.second->OnDeviceReset();
    }

    m_sugegasaMesh->OnDeviceReset();

    HRESULT hr = m_D3DFont->OnResetDevice();
    assert(hr == S_OK);
}

