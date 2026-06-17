#include "SaveDataManager.h"

#include <Windows.h>
#include <cstddef>
#include <vector>

#include "StageManager.h"
#include "../../RedFortressRender/Render/Util.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"

SaveDataManager::SaveDataManager()
    : m_stageManager(nullptr)
    , m_hasSaveFile(false)
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
    m_hasSaveFile = false;

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
        return false;
    }

    if (csvData.empty())
    {
        return false;
    }

    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.empty())
        {
            continue;
        }

        if (row.at(0) == L"StageId")
        {
            continue;
        }

        if (row.size() < 2)
        {
            continue;
        }

        const std::wstring& stageId = row.at(0);
        const std::wstring& clearedText = row.at(1);
        if (clearedText == L"1")
        {
            m_clearedStageIds.insert(stageId);
        }
    }

    m_hasSaveFile = true;
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

        csvData.push_back(row);
    }

    csv::Write(m_filePath, csvData);
    m_hasSaveFile = true;
}

void SaveDataManager::MarkStageCleared(const std::wstring& stageId)
{
    if (!stageId.empty())
    {
        m_clearedStageIds.insert(stageId);
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
    return m_hasSaveFile;
}
