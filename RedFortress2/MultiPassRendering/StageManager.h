#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

class StageManager
{
public:
    struct StageData
    {
        int number = 1;
        std::wstring displayName;
        std::wstring renderCsvPath;
        std::wstring physicsCsvPath;
        std::wstring moveCsvPath;
        D3DXVECTOR3 playerStartPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 clearPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        float clearDistance = 1.0f;
    };

    void Initialize();
    const StageData& GetCurrentStage() const;
    bool MoveNextStage();
    bool IsLastStage() const;
    bool IsClearReached(const D3DXVECTOR3& playerPosition) const;
    int GetCurrentStageNumber() const;
    const std::wstring& GetCurrentStageDisplayName() const;

private:
    void AddStage(int number,
                  const std::wstring& displayName,
                  const std::wstring& folderName,
                  const D3DXVECTOR3& playerStartPosition,
                  const D3DXVECTOR3& clearPosition);

    std::vector<StageData> m_stages;
    std::size_t m_currentStageIndex = 0;
};
