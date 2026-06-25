#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

class StageManager
{
public:
    struct StageData
    {
        std::wstring id;
        int number = 1;
        std::wstring displayName;
        std::wstring renderCsvPath;
        std::wstring physicsCsvPath;
        std::wstring moveCsvPath;
        std::wstring enemyCsvPath;
        std::wstring collectibleCsvPath;
        std::wstring interactableCsvPath;
        std::wstring starCsvPath;
        std::wstring speedUpCsvPath;
        std::wstring destructibleCsvPath;
        D3DXVECTOR3 playerStartPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 clearPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        float clearDistance = 1.0f;
        bool useFixedCamera = false;
        D3DXVECTOR3 fixedCameraPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 fixedCameraLookAt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    };

    void Initialize();
    const StageData& GetCurrentStage() const;
    const StageData& GetStage(std::size_t index) const;
    std::size_t GetStageCount() const;
    std::size_t GetCurrentStageIndex() const;
    std::size_t FindStageIndexById(const std::wstring& id) const;
    bool MoveNextStage();
    bool MoveToStage(std::size_t index);
    bool IsLastStage() const;
    bool IsClearReached(const D3DXVECTOR3& playerPosition) const;
    int GetCurrentStageNumber() const;
    const std::wstring& GetCurrentStageDisplayName() const;
    std::size_t GetClearDestinationIndex(int stageNumber) const;
    bool MoveToStageById(const std::wstring& id);
    std::wstring GetStageIdByNumber(int number) const;
    std::vector<std::wstring> GetUnlockStageIds(int stageNumber) const;

private:
    void AddStage(const std::wstring& id,
                  int number,
                  const std::wstring& displayName,
                  const std::wstring& folderName,
                  const D3DXVECTOR3& playerStartPosition,
                  const D3DXVECTOR3& clearPosition,
                  bool useFixedCamera = false,
                  const D3DXVECTOR3& fixedCameraPos = D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                  const D3DXVECTOR3& fixedCameraLookAt = D3DXVECTOR3(0.0f, 0.0f, 0.0f));

    std::vector<StageData> m_stages;
    std::size_t m_currentStageIndex = 0;
};
