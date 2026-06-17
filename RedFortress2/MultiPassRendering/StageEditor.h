#pragma once

#include <Windows.h>
#include <string>

class StageManager;

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
                    PhysicsLib::CharacterMover* playerMover,
                    float* playerYaw);

    void OnInitDialog(HWND hDlg);
    void OnNotify(HWND hDlg, LPNMHDR pnmh);
    void OnCommand(HWND hDlg, int controlId);

private:
    StageEditor(const StageEditor&);
    StageEditor& operator=(const StageEditor&);

    NSRender::Render* m_render;
    StageManager* m_stageManager;
    PhysicsLib::CharacterMover* m_playerMover;
    float* m_playerYaw;
    std::wstring m_selectedXFilePath;

    void InitListView(HWND hDlg);
    void PopulateList(HWND hDlg);
    void OnSelectX(HWND hDlg);
    void OnPlaceMesh(HWND hDlg);
    void OnDeleteMesh(HWND hDlg);
    void OnUpdateMesh(HWND hDlg);
    void OnSave(HWND hDlg);
    void OnListSelChange(HWND hDlg);
    std::wstring GetStageFolderPath() const;
    bool IsPathUnderStageFolder(const std::wstring& filePath, const std::wstring& stageFolder) const;
};
