#pragma once

#include <d3dx9math.h>
#include "../../RedFortressQuestSystem/QuestSystem/QuestSystem.h"

class QuestManager
{
public:

    static QuestManager* Get();
    static void Finalize();

    void Init(const std::wstring& originFile, const std::wstring& saveFile);
    void Save(const std::wstring& originFile);

    void SetExamine();
    void SetTalk(const std::wstring& npc);
    std::wstring GetQuestIdStartByExamine();
    std::wstring GetQuestIdFinishByExamine();

    std::vector<std::wstring> GetStartQuest();
    std::vector<std::wstring> GetQuestStartEvent(const std::wstring& quest);

    std::vector<std::wstring> GetFinishQuest();
    std::vector<std::wstring> GetQuestFinishEvent(const std::wstring& quest);
    bool GetQuestFinished(const std::wstring& id);

    void SetQuestFinish(const std::wstring& quest);

    void Update();

    void SetHint(const std::wstring& hint);
    std::wstring GetHint() const;

private:

    static QuestManager* m_single;
    NSQuestSystem::QuestSystem m_questSystem;

    std::wstring m_hint;
};

