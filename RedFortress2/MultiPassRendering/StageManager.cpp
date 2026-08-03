#include "StageManager.h"

void StageManager::Initialize()
{
    m_stages.clear();
    m_currentStageIndex = 0;

    AddStage(L"1-1", 1, L"はじまりの砦", L"stage1", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"1-2", 2, L"木箱ごろごろ峠", L"stage2", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv");
    AddStage(L"1-3", 3, L"ガレキごろごろ封鎖線", L"stage3", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv");
    AddStage(L"1-4", 4, L"風に削られた登り塔", L"stage4", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-10.0f, 14.0f, -24.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.rain.csv", StageWeather::Rain);
    AddStage(L"1-5", 5, L"ドカンと樽砲回廊", L"stage17", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.rain.csv", StageWeather::Rain);
    AddStage(L"1-6", 6, L"沈み砦の砲台渡り", L"stage18", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.rain.csv", StageWeather::Rain);
    AddStage(L"1-7", 7, L"つながれ！バラバラ砦", L"stage19", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.fog.csv", StageWeather::Fog);
    AddStage(L"1-8", 8, L"砦喰らいの大将", L"stage20", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.fog.csv", StageWeather::Fog);

    AddStage(L"2-1", 9, L"チクチク床の飛び石", L"stage5", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.fog.csv", StageWeather::Fog, true);
    AddStage(L"2-2", 10, L"灼熱飛び石ロード", L"stage6", D3DXVECTOR3(-38.0f, 3.2f, 0.0f), D3DXVECTOR3(38.0f, 4.0f, 0.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"2-3", 11, L"無敵でゴーゴー火の道", L"stage7", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"2-4", 12, L"空飛ぶ足場の乗り継ぎ", L"stage8", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"2-5", 13, L"鳥だらけ巣だらけ", L"stage21", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 48.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"2-6", 14, L"ゴゴゴ！迫る溶岩塔", L"stage22", D3DXVECTOR3(0.0f, 0.2f, -26.0f), D3DXVECTOR3(8.0f, 8.4f, 16.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"2-7", 15, L"追ってくる溶岩の道", L"stage23", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"2-8", 16, L"溶岩王の灼熱城", L"stage24", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);

    AddStage(L"3-1", 17, L"ポチポチ空中回廊", L"stage9", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"3-2", 18, L"ポチッと水上扉回廊", L"stage10", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(L"3-3", 19, L"切ってつないで断崖橋", L"stage11", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f));
    AddStage(L"3-4", 20, L"ドカン！夕焼け樽砲峡谷", L"stage12", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f));
    AddStage(L"3-5", 21, L"ひゅんひゅんワープ迷宮", L"stage25", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 114.0f));
    AddStage(L"3-6", 22, L"ぐるぐる奈落スパイラル", L"stage26", D3DXVECTOR3(-36.8f, 0.2f, -36.8f), D3DXVECTOR3(0.0f, 1.0f, 0.0f));
    AddStage(L"3-7", 23, L"うじゃうじゃ魔獣の八の字遺跡", L"stage27", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f));
    AddStage(L"3-8", 24, L"八の字遺跡の主", L"stage28", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f));

    AddStage(L"4-1", 25, L"押して運んで箱だらけ", L"stage13", D3DXVECTOR3(0.0f, 0.2f, -80.0f), D3DXVECTOR3(0.0f, 1.0f, 80.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"4-2", 26, L"溶岩海の横断デッキ", L"stage14", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"4-3", 27, L"崩れた左右の峡谷", L"stage15", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"4-4", 28, L"ふたつの壁をくぐる砦", L"stage16", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"4-5", 29, L"空中足場の七段跳び", L"stage29", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"4-6", 30, L"木箱迷路の獣道", L"stage30", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"4-7", 31, L"ゆれる溶岩の水路", L"stage31", D3DXVECTOR3(0.0f, 0.2f, 28.0f), D3DXVECTOR3(0.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);
    AddStage(L"4-8", 32, L"赤砦の守護者", L"stage32", D3DXVECTOR3(14.0f, 0.2f, 28.0f), D3DXVECTOR3(-14.0f, 1.0f, -28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.csv", StageWeather::None, true);

    AddStage(L"base", 33, L"拠点", L"base", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f));
    AddStage(L"base2", 38, L"拠点2", L"base2", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.fog.csv", StageWeather::None, true);
    AddStage(L"base3", 39, L"拠点3", L"base3", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.evening.csv", StageWeather::None, false);
    AddStage(L"base4", 40, L"拠点4", L"base4", D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             false, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f),
             L"res\\RenderSettings.night.csv", StageWeather::None, true);

    const D3DXVECTOR3 kSelect1CameraPos(0.0f, 18.0f, -26.0f);
    const D3DXVECTOR3 kSelect1CameraLookAt(0.0f, 2.0f, -2.0f);
    const D3DXVECTOR3 kSelect2CameraPos(0.0f, 15.0f, -32.0f);
    const D3DXVECTOR3 kSelect2CameraLookAt(0.0f, 0.8f, 5.5f);
    const D3DXVECTOR3 kSelect3CameraPos(0.0f, 23.0f, -38.0f);
    const D3DXVECTOR3 kSelect3CameraLookAt(0.0f, 4.2f, 6.0f);
    const D3DXVECTOR3 kSelect4CameraPos(0.0f, 20.0f, -38.0f);
    const D3DXVECTOR3 kSelect4CameraLookAt(0.0f, 2.3f, 5.0f);
    const D3DXVECTOR3 kSelectCameraPos(0.0f, 8.0f, -32.0f);
    const D3DXVECTOR3 kSelectCameraLookAt(0.0f, 2.5f, 6.0f);
    AddStage(L"select1", 34, L"Stage Select 1", L"stage-select1",
             D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             true, kSelect1CameraPos, kSelect1CameraLookAt,
             L"res\\RenderSettings.stage-select1.csv");
    AddStage(L"select2", 35, L"Stage Select 2", L"stage-select2",
             D3DXVECTOR3(0.0f, 0.2f, -28.0f), D3DXVECTOR3(0.0f, 1.0f, 28.0f),
             true, kSelect2CameraPos, kSelect2CameraLookAt,
             L"res\\RenderSettings.stage-select2.csv");
    AddStage(L"select3", 36, L"Stage Select 3", L"stage-select3",
             D3DXVECTOR3(-16.0f, 0.7f, -10.0f), D3DXVECTOR3(0.0f, 7.4f, 17.5f),
             true, kSelect3CameraPos, kSelect3CameraLookAt,
             L"res\\RenderSettings.stage-select3.csv");
    AddStage(L"select4", 37, L"Stage Select 4", L"stage-select4",
             D3DXVECTOR3(-18.0f, 0.65f, -12.0f), D3DXVECTOR3(0.0f, 3.55f, 15.5f),
             true, kSelect4CameraPos, kSelect4CameraLookAt,
             L"res\\RenderSettings.stage-select4.csv");
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
                            StageWeather weather,
                            bool playerPointLightEnabled)
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
    stage.stageSelectNavigationCsvPath = basePath + L"StageSelectNavigation.csv";
    stage.starCsvPath = basePath + L"Stars.csv";
    stage.speedUpCsvPath = basePath + L"SpeedUps.csv";
    stage.destructibleCsvPath = basePath + L"Destructibles.csv";
    stage.dashBoosterCsvPath = basePath + L"DashBoosters.csv";
    stage.lavaCsvPath = basePath + L"LavaZones.csv";
    stage.lavaFloodCsvPath = basePath + L"LavaFlood.csv";
    stage.lavaRiseCsvPath = basePath + L"LavaRise.csv";
    stage.skullCsvPath = basePath + L"Skulls.csv";
    stage.pressurePlateCsvPath = basePath + L"PressurePlates.csv";
    stage.pushableBoxCsvPath = basePath + L"PushableBoxes.csv";
    stage.attackTriggerCsvPath = basePath + L"AttackTriggers.csv";
    stage.warpBearCsvPath = basePath + L"WarpBears.csv";
    stage.renderSettingsCsvPath = renderSettingsCsvPath;
    stage.weather = weather;
    stage.playerPointLightEnabled = playerPointLightEnabled;
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
