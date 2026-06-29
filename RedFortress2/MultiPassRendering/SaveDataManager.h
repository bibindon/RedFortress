#pragma once

#include <string>
#include <unordered_set>

class StageManager;

class SaveDataManager
{
public:
    SaveDataManager();
    ~SaveDataManager();

    void Initialize(const StageManager& stageManager);

    bool Load();
    void Save();

    void MarkStageCleared(const std::wstring& stageId);
    bool IsStageCleared(const std::wstring& stageId) const;

    void MarkStageUnlocked(const std::wstring& stageId);
    bool IsStageUnlocked(const std::wstring& stageId) const;
    void MarkAllStagesClearedAndUnlocked();

    bool IsStageClearedByIndex(std::size_t stageIndex) const;
    void MarkStageClearedByIndex(std::size_t stageIndex);

    bool HasSaveFile() const;

    void InitializeDefaultUnlocks();
    void ResetToDefaults();
    void DeleteSaveData();

private:
    SaveDataManager(const SaveDataManager&);
    SaveDataManager& operator=(const SaveDataManager&);

    void BuildFilePath();
    bool EnsureDirectoryExists() const;

    const StageManager* m_stageManager;
    std::unordered_set<std::wstring> m_clearedStageIds;
    std::unordered_set<std::wstring> m_unlockedStageIds;
    std::wstring m_filePath;
    bool m_hasSaveFile;
};
