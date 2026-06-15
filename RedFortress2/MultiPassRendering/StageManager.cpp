#include "StageManager.h"

void StageManager::Initialize()
{
    m_stages.clear();
    m_currentStageIndex = 0;

    AddStage(1, L"Stage 1-1", L"stage1", D3DXVECTOR3(0.0f, 0.2f, -14.0f), D3DXVECTOR3(0.0f, 1.0f, 14.0f));
    AddStage(2, L"Stage 1-2", L"stage2", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(3, L"Stage 1-3", L"stage3", D3DXVECTOR3(0.0f, 0.2f, 14.0f), D3DXVECTOR3(0.0f, 1.0f, -14.0f));
    AddStage(4, L"Stage 1-4", L"stage4", D3DXVECTOR3(14.0f, 0.2f, 14.0f), D3DXVECTOR3(-14.0f, 1.0f, -14.0f));

    AddStage(5, L"Stage 2-1", L"stage5", D3DXVECTOR3(0.0f, 0.2f, -14.0f), D3DXVECTOR3(0.0f, 1.0f, 14.0f));
    AddStage(6, L"Stage 2-2", L"stage6", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(7, L"Stage 2-3", L"stage7", D3DXVECTOR3(0.0f, 0.2f, 14.0f), D3DXVECTOR3(0.0f, 1.0f, -14.0f));
    AddStage(8, L"Stage 2-4", L"stage8", D3DXVECTOR3(14.0f, 0.2f, 14.0f), D3DXVECTOR3(-14.0f, 1.0f, -14.0f));

    AddStage(9, L"Stage 3-1", L"stage9", D3DXVECTOR3(0.0f, 0.2f, -14.0f), D3DXVECTOR3(0.0f, 1.0f, 14.0f));
    AddStage(10, L"Stage 3-2", L"stage10", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(11, L"Stage 3-3", L"stage11", D3DXVECTOR3(0.0f, 0.2f, 14.0f), D3DXVECTOR3(0.0f, 1.0f, -14.0f));
    AddStage(12, L"Stage 3-4", L"stage12", D3DXVECTOR3(14.0f, 0.2f, 14.0f), D3DXVECTOR3(-14.0f, 1.0f, -14.0f));

    AddStage(13, L"Stage 4-1", L"stage13", D3DXVECTOR3(0.0f, 0.2f, -14.0f), D3DXVECTOR3(0.0f, 1.0f, 14.0f));
    AddStage(14, L"Stage 4-2", L"stage14", D3DXVECTOR3(-14.0f, 0.2f, 0.0f), D3DXVECTOR3(14.0f, 1.0f, 0.0f));
    AddStage(15, L"Stage 4-3", L"stage15", D3DXVECTOR3(0.0f, 0.2f, 14.0f), D3DXVECTOR3(0.0f, 1.0f, -14.0f));
    AddStage(16, L"Stage 4-4", L"stage16", D3DXVECTOR3(14.0f, 0.2f, 14.0f), D3DXVECTOR3(-14.0f, 1.0f, -14.0f));
}

void StageManager::AddStage(int number,
                            const std::wstring& displayName,
                            const std::wstring& folderName,
                            const D3DXVECTOR3& playerStartPosition,
                            const D3DXVECTOR3& clearPosition)
{
    const std::wstring basePath = L"res\\model\\" + folderName + L"\\";

    StageData stage;
    stage.number = number;
    stage.displayName = displayName;
    stage.renderCsvPath = basePath + L"XFileList_simple.csv";
    stage.physicsCsvPath = basePath + L"XFileListPhysics.csv";
    stage.moveCsvPath = basePath + L"XFileListMove.csv";
    stage.playerStartPosition = playerStartPosition;
    stage.clearPosition = clearPosition;
    stage.clearDistance = 1.0f;
    m_stages.push_back(stage);
}

const StageManager::StageData& StageManager::GetCurrentStage() const
{
    return m_stages.at(m_currentStageIndex);
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
