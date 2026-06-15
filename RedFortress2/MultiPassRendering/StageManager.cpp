#include "StageManager.h"

void StageManager::Initialize()
{
    m_stages.clear();
    m_currentStageIndex = 0;

    StageData stage1;
    stage1.number = 1;
    stage1.renderCsvPath = L"res\\model\\stage1\\XFileList_simple.csv";
    stage1.physicsCsvPath = L"res\\model\\stage1\\XFileListPhysics.csv";
    stage1.moveCsvPath = L"res\\model\\stage1\\XFileListMove.csv";
    stage1.playerStartPosition = D3DXVECTOR3(0.0f, 0.2f, -14.0f);
    stage1.clearPosition = D3DXVECTOR3(0.0f, 1.0f, 14.0f);
    stage1.clearDistance = 1.0f;
    m_stages.push_back(stage1);

    StageData stage2;
    stage2.number = 2;
    stage2.renderCsvPath = L"res\\model\\stage2\\XFileList_simple.csv";
    stage2.physicsCsvPath = L"res\\model\\stage2\\XFileListPhysics.csv";
    stage2.moveCsvPath = L"res\\model\\stage2\\XFileListMove.csv";
    stage2.playerStartPosition = D3DXVECTOR3(-14.0f, 0.2f, 0.0f);
    stage2.clearPosition = D3DXVECTOR3(14.0f, 1.0f, 0.0f);
    stage2.clearDistance = 1.0f;
    m_stages.push_back(stage2);
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
