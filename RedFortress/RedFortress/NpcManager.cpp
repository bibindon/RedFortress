#include "NpcManager.h"

#include "Common.h"
#include "SharedObj.h"
#include "PopUp2.h"
#include "resource.h"
#include "../../RedFortressModel/Model/Rynen.h"

NpcManager* NpcManager::m_ton = nullptr;

NpcManager* NpcManager::Get()
{
    if (m_ton == nullptr)
    {
        m_ton = NEW NpcManager();
    }
    return m_ton;
}

void NpcManager::Finalize()
{
    delete m_ton;
    m_ton = nullptr;
}

void NpcManager::SetPos(const std::wstring& npcName, const float fx, const float fy, const float fz)
{
    auto npcStatusMgr = NSStarmanLib::NpcStatusManager::GetObj();
    auto status = npcStatusMgr->GetNpcStatus(npcName);
    status.SetX(fx);
    status.SetY(fy);
    status.SetZ(fz);
    npcStatusMgr->SetNpcStatus(npcName, status);
}

void NpcManager::SetRot(const std::wstring& npcName, const float fRot)
{
    auto npcStatusMgr = NSStarmanLib::NpcStatusManager::GetObj();
    auto status = npcStatusMgr->GetNpcStatus(npcName);
    status.SetRotY(fRot);
    npcStatusMgr->SetNpcStatus(npcName, status);
}

void NpcManager::SetTalkEnable(const std::wstring& npcName, const bool arg)
{
    auto npcStatusMgr = NSStarmanLib::NpcStatusManager::GetObj();
    auto status = npcStatusMgr->GetNpcStatus(npcName);
    status.SetHasTalk(arg);
    npcStatusMgr->SetNpcStatus(npcName, status);
}

void NpcManager::SetTalkScript(const std::wstring& npcName, const std::wstring& arg)
{
    auto npcStatusMgr = NSStarmanLib::NpcStatusManager::GetObj();
    auto status = npcStatusMgr->GetNpcStatus(npcName);
    status.SetTalkCsv(Common::ModExt(arg));
    npcStatusMgr->SetNpcStatus(npcName, status);
}

void NpcManager::SetEnableFeature(const std::wstring& npcName,
                                  const bool arg)
{
    auto npcStatusMgr = NSStarmanLib::NpcStatusManager::GetObj();
    auto status = npcStatusMgr->GetNpcStatus(npcName);
    status.SetFeatureEnable(arg);
    npcStatusMgr->SetNpcStatus(npcName, status);
}

NSStarmanLib::NpcStatus NpcManager::GetNpcStatus(const std::wstring& npcName)
{
    auto npcStatusMgr = NSStarmanLib::NpcStatusManager::GetObj();
    auto status = npcStatusMgr->GetNpcStatus(npcName);

    return status;
}

void NpcManager::SetNpcStatus(const std::wstring& npcName, const NSStarmanLib::NpcStatus& status)
{
    auto npcStatusMgr = NSStarmanLib::NpcStatusManager::GetObj();
    npcStatusMgr->SetNpcStatus(npcName, status);
}

bool NpcManager::GetNpcTalkable(const D3DXVECTOR3& pos, std::wstring* npcName)
{
    // 3メートル以内に話しかけられるNPCがいるか
    // 球だと重いのでバウンディングボックス

    bool exist = false;
    auto npcStatusMgr = NSStarmanLib::NpcStatusManager::GetObj();
    auto nameList = npcStatusMgr->GetNameList();

    for (size_t i = 0; i < nameList.size(); ++i)
    {
        auto status = npcStatusMgr->GetNpcStatus(nameList.at(i));
        float work = 0.f;

        work = std::abs(status.GetX() - pos.x);
        if (work <= 3.f)
        {
            work = std::abs(status.GetZ() - pos.z);
            if (work <= 3.f)
            {
                work = std::abs(status.GetY() - pos.y);
                if (work <= 3.f)
                {
                    if (status.GetHasTalk())
                    {
                        exist = true;
                        if (npcName != nullptr)
                        {
                            *npcName = nameList.at(i);
                        }
                        break;
                    }
                }
            }
        }
    }

    return exist;
}

void NpcManager::Update()
{
    // ライネンの契約をするまではNpcのステータスを消費させない
    auto rynen = NSStarmanLib::Rynen::GetObj();
    if (!rynen->GetContracted())
    {
        return;
    }

    // 死亡していない→死亡しているに変わったらポップアップを表示
    auto daikei = GetNpcStatus(_T("daikeiman")).GetDead();
    auto sankaku = GetNpcStatus(_T("sankakuman")).GetDead();
    auto shikaku = GetNpcStatus(_T("shikakuman")).GetDead();

    NSStarmanLib::NpcStatusManager::GetObj()->Update();

    auto daikei2 = GetNpcStatus(_T("daikeiman")).GetDead();
    auto sankaku2 = GetNpcStatus(_T("sankakuman")).GetDead();
    auto shikaku2 = GetNpcStatus(_T("shikakuman")).GetDead();

    if (daikei != daikei2 && daikei2)
    {
        auto npcName = Common::LoadString_(IDS_STRING_DAIKEIMAN);
        auto msg = Common::LoadStringWithArg(IDS_STRING_NPC_DIED, npcName);
        PopUp2::Get()->SetText(msg);

        // NPCが死んでいたら、座標を(0, 0, 0)に移動
        SetPos(_T("daikeiman"), 0.f, 0.f, 0.f);

        // 担架モードだったら、担架モードを終了。死んでいないNPCはその場に座る
        if (Common::Status()->IsStretcherMode())
        {
            Common::Status()->SetStretcherMode(false);

            // もし水中だったら全員死亡
            if (SharedObj::GetPlayer()->IsUnderWater())
            {
                auto npcStatus = GetNpcStatus(L"sankakuman");
                npcStatus.SetDead();
                SetNpcStatus(L"sankakuman", npcStatus);

                npcStatus = GetNpcStatus(L"shikakuman");
                npcStatus.SetDead();
                SetNpcStatus(L"shikakuman", npcStatus);
            }
            else
            {
                auto ppos = SharedObj::GetPlayer()->GetPos();
                SetPos(L"sankakuman", ppos.x, ppos.y, ppos.z + 0.5f);
                SetPos(L"shikakuman", ppos.x, ppos.y, ppos.z - 0.5f);
            }
        }
    }

    if (sankaku != sankaku2 && sankaku2)
    {
        auto npcName = Common::LoadString_(IDS_STRING_SANKAKUMAN);
        auto msg = Common::LoadStringWithArg(IDS_STRING_NPC_DIED, npcName);
        PopUp2::Get()->SetText(msg);

        // NPCが死んでいたら、座標を(0, 0, 0)に移動
        SetPos(_T("sankakuman"), 0.f, 0.f, 0.f);

        // 担架モードだったら、担架モードを終了。死んでいないNPCはその場に座る
        if (Common::Status()->IsStretcherMode())
        {
            Common::Status()->SetStretcherMode(false);

            // もし水中だったら全員死亡
            if (SharedObj::GetPlayer()->IsUnderWater())
            {
                auto npcStatus = GetNpcStatus(L"daikeiman");
                npcStatus.SetDead();
                SetNpcStatus(L"daikeiman", npcStatus);

                npcStatus = GetNpcStatus(L"shikakuman");
                npcStatus.SetDead();
                SetNpcStatus(L"shikakuman", npcStatus);
            }
            else
            {
                auto ppos = SharedObj::GetPlayer()->GetPos();
                SetPos(L"daikeiman", ppos.x, ppos.y, ppos.z + 0.5f);
                SetPos(L"shikakuman", ppos.x, ppos.y, ppos.z - 0.5f);
            }
        }
    }

    if (shikaku != shikaku2 && shikaku2)
    {
        auto npcName = Common::LoadString_(IDS_STRING_SHIKAKUMAN);
        auto msg = Common::LoadStringWithArg(IDS_STRING_NPC_DIED, npcName);
        PopUp2::Get()->SetText(msg);


        // NPCが死んでいたら、座標を(0, 0, 0)に移動
        SetPos(_T("shikakuman"), 0.f, 0.f, 0.f);

        // 担架モードだったら、担架モードを終了。死んでいないNPCはその場に座る
        if (Common::Status()->IsStretcherMode())
        {
            Common::Status()->SetStretcherMode(false);

            // もし水中だったら全員死亡
            if (SharedObj::GetPlayer()->IsUnderWater())
            {
                auto npcStatus = GetNpcStatus(L"daikeiman");
                npcStatus.SetDead();
                SetNpcStatus(L"daikeiman", npcStatus);

                npcStatus = GetNpcStatus(L"sankakuman");
                npcStatus.SetDead();
                SetNpcStatus(L"sankakuman", npcStatus);
            }
            else
            {
                auto ppos = SharedObj::GetPlayer()->GetPos();
                SetPos(L"daikeiman", ppos.x, ppos.y, ppos.z + 0.5f);
                SetPos(L"sankakuman", ppos.x, ppos.y, ppos.z - 0.5f);
            }
        }
    }
}
