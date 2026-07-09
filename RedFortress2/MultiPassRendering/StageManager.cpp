#include "StageManager.h"

void StageManager::Initialize()
{
    m_stages.clear();
    m_currentStageIndex = 0;

    AddStage(L"1-1", 1, L"Stage 1-1", L"stage1", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"1-2", 2, L"Stage 1-2", L"stage2", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.night.csv");
    AddStage(L"1-3", 3, L"Stage 1-3", L"stage3", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.evening.csv");
    AddStage(L"1-4", 4, L"Stage 1-4", L"stage4", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.rain.csv", StageWeather::Rain);
    AddStage(L"1-5", 5, L"Stage 1-5", L"stage17", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.night-rain.csv", StageWeather::Rain);
    AddStage(L"1-6", 6, L"Stage 1-6", L"stage18", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.evening-rain.csv", StageWeather::Rain);
    AddStage(L"1-7", 7, L"Stage 1-7", L"stage19", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.fog.csv", StageWeather::Fog);
    AddStage(L"1-8", 8, L"Stage 1-8", L"stage20", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.night-fog.csv", StageWeather::Fog);

    AddStage(L"2-1", 9, L"Stage 2-1", L"stage5", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.evening-fog.csv", StageWeather::Fog);
    AddStage(L"2-2", 10, L"Stage 2-2", L"stage6", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(L"2-3", 11, L"Stage 2-3", L"stage7", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f));
    AddStage(L"2-4", 12, L"Stage 2-4", L"stage8", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f));
    AddStage(L"2-5", 13, L"Stage 2-5", L"stage21", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"2-6", 14, L"Stage 2-6", L"stage22", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(L"2-7", 15, L"Stage 2-7", L"stage23", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f));
    AddStage(L"2-8", 16, L"Stage 2-8", L"stage24", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f));

    AddStage(L"3-1", 17, L"Stage 3-1", L"stage9", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"3-2", 18, L"Stage 3-2", L"stage10", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(L"3-3", 19, L"Stage 3-3", L"stage11", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f));
    AddStage(L"3-4", 20, L"Stage 3-4", L"stage12", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f));
    AddStage(L"3-5", 21, L"Stage 3-5", L"stage25", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"3-6", 22, L"Stage 3-6", L"stage26", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(L"3-7", 23, L"Stage 3-7", L"stage27", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f));
    AddStage(L"3-8", 24, L"Stage 3-8", L"stage28", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f));

    AddStage(L"4-1", 25, L"Stage 4-1", L"stage13", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"4-2", 26, L"Stage 4-2", L"stage14", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(L"4-3", 27, L"Stage 4-3", L"stage15", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f));
    AddStage(L"4-4", 28, L"Stage 4-4", L"stage16", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f));
    AddStage(L"4-5", 29, L"Stage 4-5", L"stage29", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"4-6", 30, L"Stage 4-6", L"stage30", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(L"4-7", 31, L"Stage 4-7", L"stage31", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f));
    AddStage(L"4-8", 32, L"Stage 4-8", L"stage32", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f));

    AddStage(L"base", 33, L"拠点", L"base", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));

    const D3DXVECTOR3 kSelect1CameraPos(-7.0f, 8.0f, -24.0f);
    const D3DXVECTOR3 kSelect1CameraLookAt(-7.0f, 2.5f, 6.0f);
    const D3DXVECTOR3 kSelectCameraPos(0.0f, 8.0f, -32.0f);
    const D3DXVECTOR3 kSelectCameraLookAt(0.0f, 2.5f, 6.0f);
    AddStage(L"select1", 34, L"Stage Select 1", L"stage-select1",
             D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             true, kSelect1CameraPos, kSelect1CameraLookAt);
    AddStage(L"select2", 35, L"Stage Select 2", L"stage-select2",
             D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             true, kSelectCameraPos, kSelectCameraLookAt);
    AddStage(L"select3", 36, L"Stage Select 3", L"stage-select3",
             D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             true, kSelectCameraPos, kSelectCameraLookAt);
    AddStage(L"select4", 37, L"Stage Select 4", L"stage-select4",
             D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             true, kSelectCameraPos, kSelectCameraLookAt);
}

void StageManager::AddStage(const std::wstring& id,
                            int number,
                            const std::wstring& displayName,
                            const std::wstring& folderName,
                            const D3DXVECTOR3& playerStartPosition,
                            const D3DXVECTOR3& clearPosition,
                            bool useFixedCamera,
                            const D3DXVECTOR3& fixedCameraPos,
                            const D3DXVECTOR3& fixedCameraLookAt,
                            const std::wstring& renderSettingsCsvPath,
                            StageWeather weather)
{
    const std::wstring basePath = L"res\\model\\" + folderName + L"\\";

    StageData stage;
    stage.id = id;
    stage.number = number;
    stage.displayName = displayName;
    stage.renderCsvPath = basePath + L"XFileList_simple.csv";
    stage.physicsCsvPath = basePath + L"XFileListPhysics.csv";
    stage.moveCsvPath = basePath + L"XFileListMove.csv";
    stage.enemyCsvPath = basePath + L"EnemyPositions.csv";
    stage.collectibleCsvPath = basePath + L"Collectibles.csv";
    stage.interactableCsvPath = basePath + L"Interactables.csv";
    stage.starCsvPath = basePath + L"Stars.csv";
    stage.speedUpCsvPath = basePath + L"SpeedUps.csv";
    stage.destructibleCsvPath = basePath + L"Destructibles.csv";
    stage.dashBoosterCsvPath = basePath + L"DashBoosters.csv";
    stage.lavaCsvPath = basePath + L"LavaZones.csv";
    stage.renderSettingsCsvPath = renderSettingsCsvPath;
    stage.weather = weather;
    stage.playerStartPosition = playerStartPosition;
    stage.clearPosition = clearPosition;
    stage.clearDistance = 2.0f;
    stage.useFixedCamera = useFixedCamera;
    stage.fixedCameraPos = fixedCameraPos;
    stage.fixedCameraLookAt = fixedCameraLookAt;
    m_stages.push_back(stage);
}

const StageManager::StageData& StageManager::GetCurrentStage() const
{
    return m_stages.at(m_currentStageIndex);
}

const StageManager::StageData& StageManager::GetStage(std::size_t index) const
{
    return m_stages.at(index);
}

std::size_t StageManager::GetStageCount() const
{
    return m_stages.size();
}

std::size_t StageManager::GetCurrentStageIndex() const
{
    return m_currentStageIndex;
}

std::size_t StageManager::FindStageIndexById(const std::wstring& id) const
{
    for (std::size_t i = 0; i < m_stages.size(); ++i)
    {
        if (m_stages.at(i).id == id)
        {
            return i;
        }
    }

    return m_stages.size();
}

bool StageManager::MoveNextStage()
{
    if (IsLastStage())
    {
        return false;
    }

    ++m_currentStageIndex;
    return true;
}

bool StageManager::MoveToStage(std::size_t index)
{
    if (index >= m_stages.size())
    {
        return false;
    }

    m_currentStageIndex = index;
    return true;
}

bool StageManager::IsLastStage() const
{
    if (m_stages.empty())
    {
        return true;
    }

    if (m_currentStageIndex + 1 >= m_stages.size())
    {
        return true;
    }

    return false;
}

bool StageManager::IsClearReached(const D3DXVECTOR3& playerPosition) const
{
    const StageData& stage = GetCurrentStage();
    const D3DXVECTOR3 difference = playerPosition - stage.clearPosition;
    const float distance = D3DXVec3Length(&difference);
    if (distance <= stage.clearDistance)
    {
        return true;
    }

    return false;
}

int StageManager::GetCurrentStageNumber() const
{
    return GetCurrentStage().number;
}

const std::wstring& StageManager::GetCurrentStageDisplayName() const
{
    return GetCurrentStage().displayName;
}

std::size_t StageManager::GetClearDestinationIndex(int stageNumber) const
{
    if (stageNumber >= 1 && stageNumber <= 8)
    {
        return FindStageIndexById(L"select1");
    }
    if (stageNumber >= 9 && stageNumber <= 16)
    {
        return FindStageIndexById(L"select2");
    }
    if (stageNumber >= 17 && stageNumber <= 24)
    {
        return FindStageIndexById(L"select3");
    }
    if (stageNumber >= 25 && stageNumber <= 32)
    {
        return FindStageIndexById(L"select4");
    }
    return m_stages.size();
}

bool StageManager::MoveToStageById(const std::wstring& id)
{
    const std::size_t index = FindStageIndexById(id);
    if (index >= m_stages.size())
    {
        return false;
    }

    m_currentStageIndex = index;
    return true;
}

std::wstring StageManager::GetStageIdByNumber(int number) const
{
    for (const StageData& stage : m_stages)
    {
        if (stage.number == number)
        {
            return stage.id;
        }
    }

    return L"";
}

std::vector<std::wstring> StageManager::GetUnlockStageIds(int stageNumber) const
{
    std::vector<std::wstring> result;
    if (stageNumber < 1 || stageNumber > 32)
    {
        return result;
    }

    if (stageNumber % 8 != 0)
    {
        const std::wstring nextId = GetStageIdByNumber(stageNumber + 1);
        if (!nextId.empty())
        {
            result.push_back(nextId);
        }
    }
    else if (stageNumber != 32)
    {
        const int nextWorld = stageNumber / 8 + 1;
        const std::wstring selectId = L"select" + std::to_wstring(nextWorld);
        const std::wstring nextId = GetStageIdByNumber(stageNumber + 1);
        if (!selectId.empty())
        {
            result.push_back(selectId);
        }
        if (!nextId.empty())
        {
            result.push_back(nextId);
        }
    }

    return result;
}
