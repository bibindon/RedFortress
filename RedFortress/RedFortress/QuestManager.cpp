#include "QuestManager.h"
#include "SharedObj.h"
#include "../../RedFortressModel/Model/Storehouse.h"
#include "../../RedFortressModel/Model/PowereggDateTime.h"
#include "NpcManager.h"

QuestManager* QuestManager::m_single = nullptr;

void QuestManager::Init(const std::wstring& originFile, const std::wstring& saveFile)
{
    m_single->m_questSystem.Init(originFile, saveFile, false);
    auto startedQuest = m_single->m_questSystem.GetStartedQuest();

    // STARTEDクエストのヒントを再読み込みする
    for (auto& quest : startedQuest)
    {
        auto eventList = m_single->m_questSystem.GetQuestStartEvent(quest);
        for (auto& event_ : eventList)
        {
            if (event_.find(_T("<hint>")) != std::wstring::npos)
            {
                std::wstring work = event_;
                work = Common::RemoveSubstring(work, _T("<hint>"));
                SetHint(work);
            }
        }
    }
}

void QuestManager::Save(const std::wstring& originFile)
{
    m_single->m_questSystem.Save(originFile, false);
}

QuestManager* QuestManager::Get()
{
    if (m_single == nullptr)
    {
        m_single = new QuestManager();
    }

    return m_single;
}

void QuestManager::Finalize()
{
    SAFE_DELETE(m_single);
    return ;
}

void QuestManager::SetExamine()
{
    D3DXVECTOR3 playerPos = SharedObj::GetPlayer()->GetPos();
    m_questSystem.SetExamine(playerPos.x, playerPos.y, playerPos.z);
}

void QuestManager::SetTalk(const std::wstring& npc)
{
    m_questSystem.SetTalk(npc);
}

std::wstring QuestManager::GetQuestIdStartByExamine()
{
    D3DXVECTOR3 playerPos = SharedObj::GetPlayer()->GetPos();
    return m_questSystem.GetQuestIdStartByExamine(playerPos.x, playerPos.y, playerPos.z);
}

std::wstring QuestManager::GetQuestIdFinishByExamine()
{
    D3DXVECTOR3 playerPos = SharedObj::GetPlayer()->GetPos();
    return m_questSystem.GetQuestIdFinishByExamine(playerPos.x, playerPos.y, playerPos.z);
}

std::vector<std::wstring> QuestManager::GetStartQuest()
{
    auto startQuest = m_questSystem.GetStartQuest();
    return startQuest;
}

std::vector<std::wstring> QuestManager::GetQuestStartEvent(const std::wstring& quest)
{
    auto startEvent = m_questSystem.GetQuestStartEvent(quest);
    return startEvent;
}

std::vector<std::wstring> QuestManager::GetFinishQuest()
{
    auto questList = m_questSystem.GetFinishQuest();
    return questList;
}

std::vector<std::wstring> QuestManager::GetQuestFinishEvent(const std::wstring& quest)
{
    auto eventList = m_questSystem.GetQuestFinishEvent(quest);
    return eventList;
}

void QuestManager::SetQuestFinish(const std::wstring& quest)
{
    m_questSystem.SetQuestFinish(quest);
}

void QuestManager::Update()
{
    //-------------------------------------
    // クエストシステムにプレイヤーの現在地を知らせる。
    //-------------------------------------
    {
        D3DXVECTOR3 playerPos = SharedObj::GetPlayer()->GetPos();
        m_questSystem.SetPos(playerPos.x, playerPos.y, playerPos.z, false);
    }

    //-------------------------------------
    // クエストシステムに現在時刻を設定。
    //-------------------------------------
    {
        auto datetime = NSModel::PowereggDateTime::GetObj();
        m_questSystem.SetCurrentDateTime(datetime->GetYear(),
                                         datetime->GetMonth(),
                                         datetime->GetDay(),
                                         datetime->GetHour(),
                                         datetime->GetMinute(),
                                         datetime->GetSecond(),
                                         false);
    }

    //---------------------------------------------------------------------
    // クエストシステムに登録されている所持品情報を更新
    //---------------------------------------------------------------------

    // インベントリ
    {
        auto inventory = NSModel::Inventory::GetObj();
        auto itemManager = NSModel::ItemManager::GetObj();
        auto allItem = inventory->GetAllItem();

        std::vector<NSQuestSystem::ItemInfo> itemList;
        for (auto it = allItem.begin(); it != allItem.end(); ++it)
        {
            NSQuestSystem::ItemInfo itemInfo;
            itemInfo.m_itemId = it->GetItemDef().GetUnreinforcedId();

            auto itemDef = itemManager->GetItemDef(it->GetId());
            itemInfo.m_level = itemDef.GetLevel();
            itemList.push_back(itemInfo);
        }

        m_questSystem.SetInventoryContent(itemList, false);
    }

    // 倉庫
    {
        auto storehouseIdList = NSModel::StorehouseManager::Get()->GetStorehouseIdList();
        auto itemManager = NSModel::ItemManager::GetObj();

        for (size_t i = 0; i < storehouseIdList.size(); ++i)
        {
            auto id = storehouseIdList.at(i);
            auto storehouse = NSModel::StorehouseManager::Get()->GetStorehouse(id);
            auto allItem = storehouse->GetAllItem();

            std::vector<NSQuestSystem::ItemInfo> itemList;
            for (auto it = allItem.begin(); it != allItem.end(); ++it)
            {
                NSQuestSystem::ItemInfo itemInfo;
                itemInfo.m_itemId = it->GetItemDef().GetUnreinforcedId();

                auto itemDef = itemManager->GetItemDef(it->GetId());
                itemInfo.m_level = itemDef.GetLevel();
                itemList.push_back(itemInfo);
            }

            m_questSystem.SetStorehouseContent(id, itemList, false);
        }
    }

    //-------------------------------------
    // クエストシステムにNPCの生存状況を設定
    //-------------------------------------
    {
        auto bAlive = false;
        auto bDead = false;

        bDead = NpcManager::Get()->GetNpcStatus(_T("daikeiman")).GetDead();
        bAlive = !bDead;
        m_questSystem.SetNpcIsAlive(_T("daikeiman"), bAlive, false);

        bDead = NpcManager::Get()->GetNpcStatus(_T("sankakuman")).GetDead();
        bAlive = !bDead;
        m_questSystem.SetNpcIsAlive(_T("sankakuman"), bAlive, false);

        bDead = NpcManager::Get()->GetNpcStatus(_T("shikakuman")).GetDead();
        bAlive = !bDead;
        m_questSystem.SetNpcIsAlive(_T("shikakuman"), bAlive, false);
    }

    // イカダの個数
    {
        auto raftList = NSModel::Voyage::Get()->GetRaftList();
        m_questSystem.SetRaftNum((int)raftList.size(), false);
    }

    m_questSystem.UpdateQuestStatus();
}

bool QuestManager::GetQuestFinished(const std::wstring& id)
{
    bool bFinished = m_questSystem.GetQuestFinished(id);
    return bFinished;
}

void QuestManager::SetHint(const std::wstring& hint)
{
    m_hint = hint;
}

std::wstring QuestManager::GetHint() const
{
    return m_hint;
}

