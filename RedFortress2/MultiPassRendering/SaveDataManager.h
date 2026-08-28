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

    void SetStageSelectPosition(const std::wstring& stageSelectId, const std::wstring& portalId);
    bool HasStageSelectPosition() const;
    const std::wstring& GetStageSelectId() const;
    const std::wstring& GetStageSelectPortalId() const;

    void MarkStageCleared(const std::wstring& stageId);
    bool IsStageCleared(const std::wstring& stageId) const;

    void MarkStageUnlocked(const std::wstring& stageId);
    bool IsStageUnlocked(const std::wstring& stageId) const;
    void MarkAllStagesClearedAndUnlocked();

    void MarkExplanationShown(const std::wstring& stageId, const std::wstring& explanationId);
    bool IsExplanationShown(const std::wstring& stageId, const std::wstring& explanationId) const;

    bool IsStageClearedByIndex(std::size_t stageIndex) const;
    void MarkStageClearedByIndex(std::size_t stageIndex);

    bool HasSaveFile() const;
    bool HasUnsavedChanges() const;

    void InitializeDefaultUnlocks();
    void ResetToDefaults(bool markUnsaved);
    void DeleteSaveData();

private:
    SaveDataManager(const SaveDataManager&);
    SaveDataManager& operator=(const SaveDataManager&);

    void BuildFilePath();
    bool EnsureDirectoryExists() const;

    const StageManager* m_stageManager;
    std::unordered_set<std::wstring> m_clearedStageIds;
    std::unordered_set<std::wstring> m_unlockedStageIds;
    std::unordered_set<std::wstring> m_shownExplanationIds;
    std::wstring m_filePath;
    std::wstring m_stageSelectId;
    std::wstring m_stageSelectPortalId;
    bool m_hasSaveFile;
    bool m_hasUnsavedChanges;
};
