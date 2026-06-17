#pragma comment( lib, "comctl32.lib" )

#include "StageEditor.h"

#include <Windows.h>
#include <commctrl.h>
#include <d3dx9.h>
#include <vector>

#include "resource.h"
#include "StageManager.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"

StageEditor::StageEditor()
    : m_render(nullptr)
    , m_stageManager(nullptr)
    , m_playerMover(nullptr)
    , m_playerYaw(nullptr)
{
}

StageEditor::~StageEditor()
{
}

void StageEditor::Initialize(NSRender::Render* render,
                             StageManager* stageManager,
                             PhysicsLib::CharacterMover* playerMover,
                             float* playerYaw)
{
    m_render = render;
    m_stageManager = stageManager;
    m_playerMover = playerMover;
    m_playerYaw = playerYaw;
}

void StageEditor::OnInitDialog(HWND hDlg)
{
    InitListView(hDlg);
    PopulateList(hDlg);
}

void StageEditor::OnNotify(HWND hDlg, LPNMHDR pnmh)
{
    if (pnmh->code == LVN_ITEMCHANGED)
    {
        OnListSelChange(hDlg);
    }
}

void StageEditor::OnCommand(HWND hDlg, int controlId)
{
    if (controlId == IDC_BUTTON_SELECT_X)
    {
        OnSelectX(hDlg);
    }
    else if (controlId == IDC_BUTTON_PLACE_MESH)
    {
        OnPlaceMesh(hDlg);
    }
    else if (controlId == IDC_BUTTON_DELETE_MESH)
    {
        OnDeleteMesh(hDlg);
    }
    else if (controlId == IDC_BUTTON_SAVE_STAGE)
    {
        OnSave(hDlg);
    }
}

void StageEditor::InitListView(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST_MESHES);
    if (hList == NULL)
    {
        return;
    }

    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    LVCOLUMN col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    col.cx = 120;
    col.pszText = const_cast<LPWSTR>(L"FileName");
    ListView_InsertColumn(hList, 0, &col);

    col.cx = 50;
    col.pszText = const_cast<LPWSTR>(L"PosX");
    ListView_InsertColumn(hList, 1, &col);

    col.cx = 50;
    col.pszText = const_cast<LPWSTR>(L"PosY");
    ListView_InsertColumn(hList, 2, &col);

    col.cx = 50;
    col.pszText = const_cast<LPWSTR>(L"PosZ");
    ListView_InsertColumn(hList, 3, &col);

    col.cx = 50;
    col.pszText = const_cast<LPWSTR>(L"RotY");
    ListView_InsertColumn(hList, 4, &col);
}

void StageEditor::PopulateList(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST_MESHES);
    if (hList == NULL)
    {
        return;
    }

    ListView_DeleteAllItems(hList);

    const std::wstring stageFolder = GetStageFolderPath();
    if (stageFolder.empty())
    {
        return;
    }

    const std::vector<NSRender::RenderLoadedModelInfo> models = m_render->GetLoadedModelInfoList();
    for (const auto& model : models)
    {
        if (model.type != NSRender::RenderLoadedModelType::MeshMix)
        {
            continue;
        }

        if (!IsPathUnderStageFolder(model.filePath, stageFolder))
        {
            continue;
        }

        std::wstring fileName = model.filePath;
        const std::size_t slashPos = fileName.find_last_of(L"\\/");
        if (slashPos != std::wstring::npos)
        {
            fileName = fileName.substr(slashPos + 1);
        }

        LVITEM lvi = {};
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.iItem = ListView_GetItemCount(hList);
        lvi.lParam = static_cast<LPARAM>(model.renderId);
        lvi.pszText = const_cast<LPWSTR>(fileName.c_str());
        const int index = ListView_InsertItem(hList, &lvi);

        std::wstring posX = std::to_wstring(model.pos.x);
        std::wstring posY = std::to_wstring(model.pos.y);
        std::wstring posZ = std::to_wstring(model.pos.z);

        D3DXVECTOR3 rot = m_render->GetMeshMixRot(model.renderId);
        const float rotYDeg = rot.y * 180.0f / D3DX_PI;
        std::wstring rotY = std::to_wstring(rotYDeg);

        ListView_SetItemText(hList, index, 1, const_cast<LPWSTR>(posX.c_str()));
        ListView_SetItemText(hList, index, 2, const_cast<LPWSTR>(posY.c_str()));
        ListView_SetItemText(hList, index, 3, const_cast<LPWSTR>(posZ.c_str()));
        ListView_SetItemText(hList, index, 4, const_cast<LPWSTR>(rotY.c_str()));
    }
}

void StageEditor::OnSelectX(HWND hDlg)
{
    wchar_t fileName[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hDlg;
    ofn.lpstrFilter = L"X Files (*.x)\0*.x\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    const std::wstring stageFolder = GetStageFolderPath();
    if (!stageFolder.empty())
    {
        ofn.lpstrInitialDir = stageFolder.c_str();
    }

    if (GetOpenFileNameW(&ofn))
    {
        m_selectedXFilePath = fileName;
        SetDlgItemText(hDlg, IDC_STATIC_X_PATH, m_selectedXFilePath.c_str());
    }
}

void StageEditor::OnPlaceMesh(HWND hDlg)
{
    if (m_selectedXFilePath.empty())
    {
        return;
    }

    const D3DXVECTOR3 pos = m_playerMover->GetPosition();
    const D3DXVECTOR3 rot(0.0f, *m_playerYaw, 0.0f);

    const int renderId = m_render->AddMeshMix(m_selectedXFilePath, pos, rot, 1.0f);
    if (renderId < 0)
    {
        return;
    }

    PopulateList(hDlg);
}

void StageEditor::OnDeleteMesh(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST_MESHES);
    if (hList == NULL)
    {
        return;
    }

    const int selected = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
    if (selected < 0)
    {
        return;
    }

    LVITEM lvi = {};
    lvi.mask = LVIF_PARAM;
    lvi.iItem = selected;
    if (!ListView_GetItem(hList, &lvi))
    {
        return;
    }

    const int renderId = static_cast<int>(lvi.lParam);
    m_render->RemoveMeshMix(renderId);
    PopulateList(hDlg);
}

void StageEditor::OnUpdateMesh(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST_MESHES);
    if (hList == NULL)
    {
        return;
    }

    const int selected = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
    if (selected < 0)
    {
        return;
    }

    LVITEM lvi = {};
    lvi.mask = LVIF_PARAM;
    lvi.iItem = selected;
    if (!ListView_GetItem(hList, &lvi))
    {
        return;
    }

    const int renderId = static_cast<int>(lvi.lParam);

    wchar_t buf[64] = {};
    GetDlgItemText(hDlg, IDC_EDIT_POS_X, buf, 64);
    const float posX = static_cast<float>(std::wcstod(buf, nullptr));

    GetDlgItemText(hDlg, IDC_EDIT_POS_Y, buf, 64);
    const float posY = static_cast<float>(std::wcstod(buf, nullptr));

    GetDlgItemText(hDlg, IDC_EDIT_POS_Z, buf, 64);
    const float posZ = static_cast<float>(std::wcstod(buf, nullptr));

    GetDlgItemText(hDlg, IDC_EDIT_ROT_Y, buf, 64);
    const float rotYDeg = static_cast<float>(std::wcstod(buf, nullptr));
    const float rotY = D3DXToRadian(rotYDeg);

    m_render->SetMeshMixPos(renderId, D3DXVECTOR3(posX, posY, posZ));
    m_render->SetMeshMixRotY(renderId, rotY);
    PopulateList(hDlg);
}

void StageEditor::OnSave(HWND hDlg)
{
    const std::wstring stageFolder = GetStageFolderPath();
    if (stageFolder.empty())
    {
        return;
    }

    SYSTEMTIME st = {};
    GetLocalTime(&st);
    wchar_t timeStr[32] = {};
    swprintf_s(timeStr, L"%04d%02d%02d%02d%02d%02d",
               st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    const std::wstring filePath = stageFolder + L"\\XFileList." + timeStr + L".csv";

    const std::vector<NSRender::RenderLoadedModelInfo> models = m_render->GetLoadedModelInfoList();

    std::vector<std::vector<std::wstring>> csvData;
    std::vector<std::wstring> header;
    header.push_back(L"ID");
    header.push_back(L"FileName");
    header.push_back(L"PosX");
    header.push_back(L"PosY");
    header.push_back(L"PosZ");
    header.push_back(L"RotX");
    header.push_back(L"RotY");
    header.push_back(L"RotZ");
    header.push_back(L"Scale");
    header.push_back(L"loadType");
    csvData.push_back(header);

    int id = 1;
    for (const auto& model : models)
    {
        if (model.type != NSRender::RenderLoadedModelType::MeshMix)
        {
            continue;
        }

        if (!IsPathUnderStageFolder(model.filePath, stageFolder))
        {
            continue;
        }

        std::wstring relativePath = model.filePath;
        if (relativePath.find(stageFolder) == 0)
        {
            relativePath = relativePath.substr(stageFolder.length());
            if (!relativePath.empty() && (relativePath[0] == L'\\' || relativePath[0] == L'/'))
            {
                relativePath = relativePath.substr(1);
            }
        }

        D3DXVECTOR3 rot = m_render->GetMeshMixRot(model.renderId);
        const float rotYDeg = rot.y * 180.0f / D3DX_PI;

        std::vector<std::wstring> row;
        row.push_back(std::to_wstring(id));
        row.push_back(relativePath);
        row.push_back(std::to_wstring(model.pos.x));
        row.push_back(std::to_wstring(model.pos.y));
        row.push_back(std::to_wstring(model.pos.z));
        row.push_back(L"0");
        row.push_back(std::to_wstring(rotYDeg));
        row.push_back(L"0");
        row.push_back(std::to_wstring(model.scale));
        row.push_back(L"normal");
        csvData.push_back(row);
        ++id;
    }

    csv::Write(filePath, csvData);
}

void StageEditor::OnListSelChange(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST_MESHES);
    if (hList == NULL)
    {
        return;
    }

    const int selected = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
    if (selected < 0)
    {
        return;
    }

    LVITEM lvi = {};
    lvi.mask = LVIF_PARAM;
    lvi.iItem = selected;
    if (!ListView_GetItem(hList, &lvi))
    {
        return;
    }

    const int renderId = static_cast<int>(lvi.lParam);
    const D3DXVECTOR3 pos = m_render->GetMeshMixPos(renderId);
    const D3DXVECTOR3 rot = m_render->GetMeshMixRot(renderId);
    const float rotYDeg = rot.y * 180.0f / D3DX_PI;

    SetDlgItemText(hDlg, IDC_EDIT_POS_X, std::to_wstring(pos.x).c_str());
    SetDlgItemText(hDlg, IDC_EDIT_POS_Y, std::to_wstring(pos.y).c_str());
    SetDlgItemText(hDlg, IDC_EDIT_POS_Z, std::to_wstring(pos.z).c_str());
    SetDlgItemText(hDlg, IDC_EDIT_ROT_Y, std::to_wstring(rotYDeg).c_str());
}

std::wstring StageEditor::GetStageFolderPath() const
{
    const StageManager::StageData& stage = m_stageManager->GetCurrentStage();
    const std::wstring& csvPath = stage.renderCsvPath;
    const std::size_t pos = csvPath.find_last_of(L"\\/");
    if (pos == std::wstring::npos)
    {
        return std::wstring();
    }

    return csvPath.substr(0, pos);
}

bool StageEditor::IsPathUnderStageFolder(const std::wstring& filePath, const std::wstring& stageFolder) const
{
    if (stageFolder.empty() || filePath.empty())
    {
        return false;
    }

    if (filePath.size() < stageFolder.size())
    {
        return false;
    }

    const std::wstring prefix = filePath.substr(0, stageFolder.size());
    if (prefix != stageFolder)
    {
        return false;
    }

    if (filePath.size() > stageFolder.size())
    {
        const wchar_t next = filePath.at(stageFolder.size());
        if (next != L'\\' && next != L'/')
        {
            return false;
        }
    }

    return true;
}
