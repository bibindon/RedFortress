#include "SaveDataManager.h"

#include <Windows.h>
#include <algorithm>
#include <cstddef>
#include <vector>

#include "StageManager.h"
#include "../../RedFortressRender/Render/Util.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"

namespace
{
const wchar_t kExplanationIdSeparator = L'|';
const wchar_t kStageExplanationSeparator = L':';

std::wstring MakeExplanationSaveId(const std::wstring& stageId,
                                   const std::wstring& explanationId)
{
    return stageId + kStageExplanationSeparator + explanationId;
}
}

SaveDataManager::SaveDataManager()
    : m_stageManager(nullptr)
    , m_hasSaveFile(false)
    , m_hasUnsavedChanges(false)
{
}

SaveDataManager::~SaveDataManager()
{
}

void SaveDataManager::Initialize(const StageManager& stageManager)
{
    m_stageManager = &stageManager;
    BuildFilePath();
}

void SaveDataManager::BuildFilePath()
{
    m_filePath = NSRender::Util::GetExeDir() + L"res\\savedata\\save.csv";
}

bool SaveDataManager::EnsureDirectoryExists() const
{
    const std::size_t lastSlash = m_filePath.find_last_of(L"\\/");
    if (lastSlash == std::wstring::npos)
    {
        return false;
    }

    const std::wstring dir = m_filePath.substr(0, lastSlash);
    const BOOL result = CreateDirectoryW(dir.c_str(), nullptr);
    if (result != 0)
    {
        return true;
    }

    const DWORD error = GetLastError();
    if (error == ERROR_ALREADY_EXISTS)
    {
        return true;
    }

    return false;
}

bool SaveDataManager::Load()
{
    m_clearedStageIds.clear();
    m_unlockedStageIds.clear();
    m_shownExplanationIds.clear();
    m_stageSelectId.clear();
    m_stageSelectPortalId.clear();
    m_hasSaveFile = false;
    m_hasUnsavedChanges = false;

    if (m_stageManager == nullptr)
    {
        return false;
    }

    std::vector<std::vector<std::wstring>> csvData;
    try
    {
        csvData = csv::Read(m_filePath);
    }
    catch (...)
    {
        InitializeDefaultUnlocks();
        return false;
    }

    if (csvData.empty())
    {
        InitializeDefaultUnlocks();
        return false;
    }

    bool hasUnlockedColumn = false;
    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.empty())
        {
            continue;
        }

        if (row.at(0) == L"StageId")
        {
            if (row.size() >= 3 && row.at(2) == L"Unlocked")
            {
                hasUnlockedColumn = true;
            }
            continue;
        }

        if (row.size() < 2)
        {
            continue;
        }

        const std::wstring& stageId = row.at(0);
        if (row.size() >= 4 && !row.at(3).empty())
        {
            m_stageSelectId = stageId;
            m_stageSelectPortalId = row.at(3);
        }
        const std::wstring& clearedText = row.at(1);
        if (clearedText == L"1")
        {
            m_clearedStageIds.insert(stageId);
            m_unlockedStageIds.insert(stageId);
        }

        if (hasUnlockedColumn && row.size() >= 3 && row.at(2) == L"1")
        {
            m_unlockedStageIds.insert(stageId);
        }

        if (row.size() >= 5 && !row.at(4).empty())
        {
            const std::wstring& shownIds = row.at(4);
            std::size_t begin = 0;
            while (begin <= shownIds.length())
            {
                const std::size_t end = shownIds.find(kExplanationIdSeparator, begin);
                std::wstring explanationId;
                if (end == std::wstring::npos)
                {
                    explanationId = shownIds.substr(begin);
                }
                else
                {
                    explanationId = shownIds.substr(begin, end - begin);
                }

                if (!explanationId.empty())
                {
                    m_shownExplanationIds.insert(MakeExplanationSaveId(stageId, explanationId));
                }
                if (end == std::wstring::npos)
                {
                    break;
                }
                begin = end + 1;
            }
        }
    }

    InitializeDefaultUnlocks();

    for (const std::wstring& stageId : m_clearedStageIds)
    {
        m_unlockedStageIds.insert(stageId);
        for (std::size_t i = 0; i < m_stageManager->GetStageCount(); ++i)
        {
            const StageManager::StageData& stage = m_stageManager->GetStage(i);
            if (stage.id == stageId)
            {
                const std::vector<std::wstring> unlockIds = m_stageManager->GetUnlockStageIds(stage.number);
                for (const std::wstring& id : unlockIds)
                {
                    m_unlockedStageIds.insert(id);
                }
                break;
            }
        }
    }

    m_hasSaveFile = true;
    m_hasUnsavedChanges = false;
    return true;
}

void SaveDataManager::Save()
{
    if (m_stageManager == nullptr)
    {
        return;
    }

    if (!EnsureDirectoryExists())
    {
        return;
    }

    std::vector<std::vector<std::wstring>> csvData;
    std::vector<std::wstring> header;
    header.push_back(L"StageId");
    header.push_back(L"Cleared");
    header.push_back(L"Unlocked");
    header.push_back(L"SelectedPortalId");
    header.push_back(L"ShownExplanationIds");
    csvData.push_back(header);

    const std::size_t stageCount = m_stageManager->GetStageCount();
    for (std::size_t i = 0; i < stageCount; ++i)
    {
        const StageManager::StageData& stage = m_stageManager->GetStage(i);
        std::vector<std::wstring> row;
        row.push_back(stage.id);

        const bool isCleared = IsStageCleared(stage.id);
        if (isCleared)
        {
            row.push_back(L"1");
        }
        else
        {
            row.push_back(L"0");
        }

        const bool isUnlocked = IsStageUnlocked(stage.id);
        if (isUnlocked)
        {
            row.push_back(L"1");
        }
        else
        {
            row.push_back(L"0");
        }

        if (stage.id == m_stageSelectId)
        {
            row.push_back(m_stageSelectPortalId);
        }
        else
        {
            row.push_back(L"");
        }

        const std::wstring explanationPrefix = stage.id + kStageExplanationSeparator;
        std::vector<std::wstring> stageExplanationIds;
        for (const std::wstring& saveId : m_shownExplanationIds)
        {
            if (saveId.length() > explanationPrefix.length() &&
                saveId.substr(0, explanationPrefix.length()) == explanationPrefix)
            {
                stageExplanationIds.push_back(saveId.substr(explanationPrefix.length()));
            }
        }
        std::sort(stageExplanationIds.begin(), stageExplanationIds.end());

        std::wstring shownExplanationIds;
        for (std::size_t explanationIndex = 0;
             explanationIndex < stageExplanationIds.size();
             ++explanationIndex)
        {
            if (!shownExplanationIds.empty())
            {
                shownExplanationIds += kExplanationIdSeparator;
            }
            shownExplanationIds += stageExplanationIds.at(explanationIndex);
        }
        row.push_back(shownExplanationIds);

        csvData.push_back(row);
    }

    csv::Write(m_filePath, csvData);
    m_hasSaveFile = true;
    m_hasUnsavedChanges = false;
}

void SaveDataManager::SetStageSelectPosition(const std::wstring& stageSelectId, const std::wstring& portalId)
{
    if (m_stageSelectId != stageSelectId || m_stageSelectPortalId != portalId)
    {
        m_hasUnsavedChanges = true;
    }
    m_stageSelectId = stageSelectId;
    m_stageSelectPortalId = portalId;
}

bool SaveDataManager::HasStageSelectPosition() const
{
    return !m_stageSelectId.empty() && !m_stageSelectPortalId.empty();
}

const std::wstring& SaveDataManager::GetStageSelectId() const
{
    return m_stageSelectId;
}

const std::wstring& SaveDataManager::GetStageSelectPortalId() const
{
    return m_stageSelectPortalId;
}

void SaveDataManager::MarkStageCleared(const std::wstring& stageId)
{
    if (!stageId.empty())
    {
        if (m_clearedStageIds.insert(stageId).second)
        {
            m_hasUnsavedChanges = true;
        }
    }
}

bool SaveDataManager::IsStageCleared(const std::wstring& stageId) const
{
    if (stageId.empty())
    {
        return false;
    }

    const auto foundIter = m_clearedStageIds.find(stageId);
    if (foundIter != m_clearedStageIds.end())
    {
        return true;
    }

    return false;
}

bool SaveDataManager::IsStageClearedByIndex(std::size_t stageIndex) const
{
    if (m_stageManager == nullptr)
    {
        return false;
    }

    const StageManager::StageData& stage = m_stageManager->GetStage(stageIndex);
    return IsStageCleared(stage.id);
}

void SaveDataManager::MarkStageClearedByIndex(std::size_t stageIndex)
{
    if (m_stageManager == nullptr)
    {
        return;
    }

    const StageManager::StageData& stage = m_stageManager->GetStage(stageIndex);
    MarkStageCleared(stage.id);
}

bool SaveDataManager::HasSaveFile() const
{
    if (m_hasSaveFile)
    {
        return true;
    }

    if (m_filePath.empty())
    {
        return false;
    }

    const DWORD attributes = GetFileAttributesW(m_filePath.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    if ((attributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        return false;
    }

    return true;
}

bool SaveDataManager::HasUnsavedChanges() const
{
    return m_hasUnsavedChanges;
}

void SaveDataManager::MarkStageUnlocked(const std::wstring& stageId)
{
    if (!stageId.empty())
    {
        if (m_unlockedStageIds.insert(stageId).second)
        {
            m_hasUnsavedChanges = true;
        }
    }
}

bool SaveDataManager::IsStageUnlocked(const std::wstring& stageId) const
{
    if (stageId.empty())
    {
        return false;
    }

    return m_unlockedStageIds.find(stageId) != m_unlockedStageIds.end();
}

void SaveDataManager::MarkExplanationShown(const std::wstring& stageId,
                                           const std::wstring& explanationId)
{
    if (stageId.empty() || explanationId.empty())
    {
        return;
    }
    if (m_shownExplanationIds.insert(MakeExplanationSaveId(stageId, explanationId)).second)
    {
        m_hasUnsavedChanges = true;
    }
}

bool SaveDataManager::IsExplanationShown(const std::wstring& stageId,
                                         const std::wstring& explanationId) const
{
    if (stageId.empty() || explanationId.empty())
    {
        return false;
    }
    const std::wstring saveId = MakeExplanationSaveId(stageId, explanationId);
    return m_shownExplanationIds.find(saveId) != m_shownExplanationIds.end();
}

void SaveDataManager::MarkAllStagesClearedAndUnlocked()
{
    if (m_stageManager == nullptr)
    {
        return;
    }

    const std::size_t stageCount = m_stageManager->GetStageCount();
    for (std::size_t i = 0; i < stageCount; ++i)
    {
        const StageManager::StageData& stage = m_stageManager->GetStage(i);
        MarkStageCleared(stage.id);
        MarkStageUnlocked(stage.id);
    }
}

void SaveDataManager::InitializeDefaultUnlocks()
{
    m_unlockedStageIds.insert(L"select1");
    m_unlockedStageIds.insert(L"1-1");
}

void SaveDataManager::ResetToDefaults(const bool markUnsaved)
{
    m_clearedStageIds.clear();
    m_unlockedStageIds.clear();
    m_shownExplanationIds.clear();
    m_stageSelectId.clear();
    m_stageSelectPortalId.clear();
    m_hasSaveFile = false;
    InitializeDefaultUnlocks();
    m_hasUnsavedChanges = markUnsaved;
}

void SaveDataManager::DeleteSaveData()
{
    DeleteFileW(m_filePath.c_str());
    ResetToDefaults(false);
}
