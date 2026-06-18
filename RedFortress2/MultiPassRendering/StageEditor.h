#pragma once

#include <Windows.h>
#include <string>
#include <unordered_set>

class StageManager;
class EnemyManager;

namespace PhysicsLib
{
    class CharacterMover;
}

namespace NSRender
{
    class Render;
}

class StageEditor
{
public:
    StageEditor();
    ~StageEditor();

    void Initialize(NSRender::Render* render,
                    StageManager* stageManager,
                    EnemyManager* enemyManager,
                    PhysicsLib::CharacterMover* playerMover,
                    float* playerYaw);

    void OnInitDialog(HWND hDlg);
    void OnNotify(HWND hDlg, LPNMHDR pnmh);
    void OnCommand(HWND hDlg, int controlId);

private:
    StageEditor(const StageEditor&);
    StageEditor& operator=(const StageEditor&);

    enum class EditMode
    {
        Mesh,
        Enemy
    };

    NSRender::Render* m_render;
    StageManager* m_stageManager;
    EnemyManager* m_enemyManager;
    PhysicsLib::CharacterMover* m_playerMover;
    float* m_playerYaw;
    std::wstring m_selectedXFilePath;
    EditMode m_editMode;

    void SetEditMode(HWND hDlg, EditMode mode);
    void InitListView(HWND hDlg);
    void PopulateList(HWND hDlg);
    void OnSelectX(HWND hDlg);
    void OnPlaceMesh(HWND hDlg);
    void OnPlaceEnemy(HWND hDlg);
    void OnDeleteMesh(HWND hDlg);
    void OnDeleteEnemy(HWND hDlg);
    void OnUpdateMesh(HWND hDlg);
    void OnUpdateEnemy(HWND hDlg);
    void OnSave(HWND hDlg);
    void OnListSelChange(HWND hDlg);
    void OnEnemyListSelChange(HWND hDlg);
    void InitEnemyTypeCombo(HWND hDlg);
    std::wstring GetStageFolderPath() const;
    bool IsPathUnderStageFolder(const std::wstring& filePath, const std::wstring& stageFolder) const;

    std::unordered_set<int> m_movingRenderIds;
};
