#include "StageManager.h"

void StageManager::Initialize()
{
    m_stages.clear();
    m_currentStageIndex = 0;

    StageData stage1;
    stage1.number = 1;
    stage1.displayName = L"Stage 1-1";
    stage1.renderCsvPath = L"res\\model\\stage1\\XFileList_simple.csv";
    stage1.physicsCsvPath = L"res\\model\\stage1\\XFileListPhysics.csv";
    stage1.moveCsvPath = L"res\\model\\stage1\\XFileListMove.csv";
    stage1.playerStartPosition = D3DXVECTOR3(0.0f, 0.2f, -14.0f);
    stage1.clearPosition = D3DXVECTOR3(0.0f, 1.0f, 14.0f);
    stage1.clearDistance = 1.0f;
    m_stages.push_back(stage1);

    StageData stage2;
    stage2.number = 2;
    stage2.displayName = L"Stage 1-2";
    stage2.renderCsvPath = L"res\\model\\stage2\\XFileList_simple.csv";
    stage2.physicsCsvPath = L"res\\model\\stage2\\XFileListPhysics.csv";
    stage2.moveCsvPath = L"res\\model\\stage2\\XFileListMove.csv";
    stage2.playerStartPosition = D3DXVECTOR3(-14.0f, 0.2f, 0.0f);
    stage2.clearPosition = D3DXVECTOR3(14.0f, 1.0f, 0.0f);
    stage2.clearDistance = 1.0f;
    m_stages.push_back(stage2);

    StageData stage3;
    stage3.number = 3;
    stage3.displayName = L"Stage 1-3";
    stage3.renderCsvPath = L"res\\model\\stage3\\XFileList_simple.csv";
    stage3.physicsCsvPath = L"res\\model\\stage3\\XFileListPhysics.csv";
    stage3.moveCsvPath = L"res\\model\\stage3\\XFileListMove.csv";
    stage3.playerStartPosition = D3DXVECTOR3(0.0f, 0.2f, 14.0f);
    stage3.clearPosition = D3DXVECTOR3(0.0f, 1.0f, -14.0f);
    stage3.clearDistance = 1.0f;
    m_stages.push_back(stage3);

    StageData stage4;
    stage4.number = 4;
    stage4.displayName = L"Stage 1-4";
    stage4.renderCsvPath = L"res\\model\\stage4\\XFileList_simple.csv";
    stage4.physicsCsvPath = L"res\\model\\stage4\\XFileListPhysics.csv";
    stage4.moveCsvPath = L"res\\model\\stage4\\XFileListMove.csv";
    stage4.playerStartPosition = D3DXVECTOR3(14.0f, 0.2f, 14.0f);
    stage4.clearPosition = D3DXVECTOR3(-14.0f, 1.0f, -14.0f);
    stage4.clearDistance = 1.0f;
    m_stages.push_back(stage4);
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
