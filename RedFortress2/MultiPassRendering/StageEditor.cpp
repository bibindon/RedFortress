#pragma comment( lib, "comctl32.lib" )

#include "StageEditor.h"

#include <Windows.h>
#include <commctrl.h>
#include <d3dx9.h>
#include <vector>

#include "resource.h"
#include "StageManager.h"
#include "EnemyManager.h"
#include "Enemy.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"

StageEditor::StageEditor()
    : m_render(nullptr)
    , m_stageManager(nullptr)
    , m_enemyManager(nullptr)
    , m_playerMover(nullptr)
    , m_playerYaw(nullptr)
    , m_editMode(EditMode::Mesh)
{
}

StageEditor::~StageEditor()
{
}

void StageEditor::Initialize(NSRender::Render* render,
                             StageManager* stageManager,
                             EnemyManager* enemyManager,
                             PhysicsLib::CharacterMover* playerMover,
                             float* playerYaw)
{
    m_render = render;
    m_stageManager = stageManager;
    m_enemyManager = enemyManager;
    m_playerMover = playerMover;
    m_playerYaw = playerYaw;
}

void StageEditor::OnInitDialog(HWND hDlg)
{
    CheckDlgButton(hDlg, IDC_RADIO_EDIT_MESH, BST_CHECKED);
    InitEnemyTypeCombo(hDlg);
    SetEditMode(hDlg, EditMode::Mesh);
}

void StageEditor::OnNotify(HWND hDlg, LPNMHDR pnmh)
{
    if (pnmh->code == LVN_ITEMCHANGED)
    {
        if (m_editMode == EditMode::Mesh)
        {
            OnListSelChange(hDlg);
        }
        else
        {
            OnEnemyListSelChange(hDlg);
        }
    }
}

void StageEditor::OnCommand(HWND hDlg, int controlId)
{
    if (controlId == IDC_RADIO_EDIT_MESH)
    {
        SetEditMode(hDlg, EditMode::Mesh);
    }
    else if (controlId == IDC_RADIO_EDIT_ENEMY)
    {
        SetEditMode(hDlg, EditMode::Enemy);
    }
    else if (controlId == IDC_BUTTON_SELECT_X)
    {
        OnSelectX(hDlg);
    }
    else if (controlId == IDC_BUTTON_PLACE_MESH)
    {
        if (m_editMode == EditMode::Mesh)
        {
            OnPlaceMesh(hDlg);
        }
        else
        {
            OnPlaceEnemy(hDlg);
        }
    }
    else if (controlId == IDC_BUTTON_DELETE_MESH)
    {
        if (m_editMode == EditMode::Mesh)
        {
            OnDeleteMesh(hDlg);
        }
        else
        {
            OnDeleteEnemy(hDlg);
        }
    }
    else if (controlId == IDC_BUTTON_SAVE_STAGE)
    {
        OnSave(hDlg);
    }
}

void StageEditor::SetEditMode(HWND hDlg, EditMode mode)
{
    m_editMode = mode;
    InitListView(hDlg);
    PopulateList(hDlg);

    if (mode == EditMode::Mesh)
    {
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SELECT_X), TRUE);
        EnableWindow(GetDlgItem(hDlg, IDC_COMBO_ENEMY_TYPE), FALSE);
    }
    else
    {
        EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_SELECT_X), FALSE);
        SetDlgItemText(hDlg, IDC_STATIC_X_PATH, L"");
        EnableWindow(GetDlgItem(hDlg, IDC_COMBO_ENEMY_TYPE), TRUE);
    }

    SetDlgItemText(hDlg, IDC_EDIT_POS_X, L"");
    SetDlgItemText(hDlg, IDC_EDIT_POS_Y, L"");
    SetDlgItemText(hDlg, IDC_EDIT_POS_Z, L"");
    SetDlgItemText(hDlg, IDC_EDIT_ROT_Y, L"");
}

void StageEditor::InitListView(HWND hDlg)
{
    HWND hList = GetDlgItem(hDlg, IDC_LIST_MESHES);
    if (hList == NULL)
    {
        return;
    }

    ListView_SetExtendedListViewStyle(hList, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

    while (ListView_DeleteColumn(hList, 0))
    {
    }

    LVCOLUMN col = {};
    col.mask = LVCF_TEXT | LVCF_WIDTH;

    if (m_editMode == EditMode::Mesh)
    {
        col.cx = 120;
        col.pszText = const_cast<LPWSTR>(L"FileName");
        ListView_InsertColumn(hList, 0, &col);
    }
    else
    {
        col.cx = 120;
        col.pszText = const_cast<LPWSTR>(L"Type");
        ListView_InsertColumn(hList, 0, &col);
    }

    col.cx = 50;
    col.pszText = const_cast<LPWSTR>(L"PosX");
    ListView_InsertColumn(hList, 1, &col);

    col.pszText = const_cast<LPWSTR>(L"PosY");
    ListView_InsertColumn(hList, 2, &col);

    col.pszText = const_cast<LPWSTR>(L"PosZ");
    ListView_InsertColumn(hList, 3, &col);

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

    if (m_editMode == EditMode::Mesh)
    {
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
    else
    {
        std::vector<Enemy>& enemies = m_enemyManager->GetEnemies();
        for (std::size_t i = 0; i < enemies.size(); ++i)
        {
            Enemy& enemy = enemies[i];
            if (enemy.IsReadyToRemove())
            {
                continue;
            }

            LVITEM lvi = {};
            lvi.mask = LVIF_TEXT | LVIF_PARAM;
            lvi.iItem = ListView_GetItemCount(hList);
            lvi.lParam = static_cast<LPARAM>(i);
            lvi.pszText = const_cast<LPWSTR>(enemy.GetType().c_str());
            const int index = ListView_InsertItem(hList, &lvi);

            const D3DXVECTOR3 pos = enemy.GetPosition();
            const float rotYDeg = enemy.GetYaw() * 180.0f / D3DX_PI;

            std::wstring posX = std::to_wstring(pos.x);
            std::wstring posY = std::to_wstring(pos.y);
            std::wstring posZ = std::to_wstring(pos.z);
            std::wstring rotY = std::to_wstring(rotYDeg);

            ListView_SetItemText(hList, index, 1, const_cast<LPWSTR>(posX.c_str()));
            ListView_SetItemText(hList, index, 2, const_cast<LPWSTR>(posY.c_str()));
            ListView_SetItemText(hList, index, 3, const_cast<LPWSTR>(posZ.c_str()));
            ListView_SetItemText(hList, index, 4, const_cast<LPWSTR>(rotY.c_str()));
        }
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

    const int physicsId = PhysicsLib::PhysicsLib::Load(
        m_selectedXFilePath.c_str(),
        PhysicsLib::PhysicsLib::ObjectType::Slide,
        0.5f);
    if (physicsId >= 0)
    {
        PhysicsLib::PhysicsLib::SetTransform(physicsId, pos, rot,
            D3DXVECTOR3(1.0f, 1.0f, 1.0f));
    }

    HWND hCheckMoving = GetDlgItem(hDlg, IDC_CHECK_MOVING);
    if (hCheckMoving != NULL && SendMessage(hCheckMoving, BM_GETCHECK, 0, 0) == BST_CHECKED)
    {
        MovingPlatformInfo info;
        info.startPos = pos;
        info.endPos = pos;
        info.duration = 5.0f;

        wchar_t buf[32] = {};
        if (GetDlgItemText(hDlg, IDC_EDIT_START_X, buf, 32) > 0) info.startPos.x = static_cast<float>(_wtof(buf));
        if (GetDlgItemText(hDlg, IDC_EDIT_START_Y, buf, 32) > 0) info.startPos.y = static_cast<float>(_wtof(buf));
        if (GetDlgItemText(hDlg, IDC_EDIT_START_Z, buf, 32) > 0) info.startPos.z = static_cast<float>(_wtof(buf));
        if (GetDlgItemText(hDlg, IDC_EDIT_END_X, buf, 32) > 0) info.endPos.x = static_cast<float>(_wtof(buf));
        if (GetDlgItemText(hDlg, IDC_EDIT_END_Y, buf, 32) > 0) info.endPos.y = static_cast<float>(_wtof(buf));
        if (GetDlgItemText(hDlg, IDC_EDIT_END_Z, buf, 32) > 0) info.endPos.z = static_cast<float>(_wtof(buf));
        if (GetDlgItemText(hDlg, IDC_EDIT_DURATION, buf, 32) > 0) info.duration = static_cast<float>(_wtof(buf));

        m_movingPlatformInfos[renderId] = info;
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
    m_movingPlatformInfos.erase(renderId);
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

void StageEditor::InitEnemyTypeCombo(HWND hDlg)
{
    HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_ENEMY_TYPE);
    if (hCombo == NULL)
    {
        return;
    }

    SendMessage(hCombo, CB_RESETCONTENT, 0, 0);
    SendMessage(hCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"wolf"));
    SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

void StageEditor::OnPlaceEnemy(HWND hDlg)
{
    HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_ENEMY_TYPE);
    if (hCombo == NULL)
    {
        return;
    }

    const int sel = static_cast<int>(SendMessage(hCombo, CB_GETCURSEL, 0, 0));
    if (sel < 0)
    {
        return;
    }

    wchar_t typeBuf[64] = {};
    SendMessage(hCombo, CB_GETLBTEXT, sel, reinterpret_cast<LPARAM>(typeBuf));
    const std::wstring type = typeBuf;

    const D3DXVECTOR3 pos = m_playerMover->GetPosition();
    const float yaw = *m_playerYaw;

    m_enemyManager->SpawnAt(*m_render, pos, type, yaw);
    PopulateList(hDlg);
}

void StageEditor::OnDeleteEnemy(HWND hDlg)
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

    const std::size_t index = static_cast<std::size_t>(lvi.lParam);
    m_enemyManager->RemoveEnemy(*m_render, index);
    PopulateList(hDlg);
}

void StageEditor::OnEnemyListSelChange(HWND hDlg)
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

    const std::size_t index = static_cast<std::size_t>(lvi.lParam);
    std::vector<Enemy>& enemies = m_enemyManager->GetEnemies();
    if (index >= enemies.size())
    {
        return;
    }

    const Enemy& enemy = enemies[index];
    const D3DXVECTOR3 pos = enemy.GetPosition();
    const float rotYDeg = enemy.GetYaw() * 180.0f / D3DX_PI;

    SetDlgItemText(hDlg, IDC_EDIT_POS_X, std::to_wstring(pos.x).c_str());
    SetDlgItemText(hDlg, IDC_EDIT_POS_Y, std::to_wstring(pos.y).c_str());
    SetDlgItemText(hDlg, IDC_EDIT_POS_Z, std::to_wstring(pos.z).c_str());
    SetDlgItemText(hDlg, IDC_EDIT_ROT_Y, std::to_wstring(rotYDeg).c_str());

    HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_ENEMY_TYPE);
    if (hCombo != NULL)
    {
        const int count = static_cast<int>(SendMessage(hCombo, CB_GETCOUNT, 0, 0));
        for (int i = 0; i < count; ++i)
        {
            wchar_t buf[64] = {};
            SendMessage(hCombo, CB_GETLBTEXT, i, reinterpret_cast<LPARAM>(buf));
            if (enemy.GetType() == buf)
            {
                SendMessage(hCombo, CB_SETCURSEL, i, 0);
                break;
            }
        }
    }
}

void StageEditor::OnUpdateEnemy(HWND hDlg)
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

    const std::size_t index = static_cast<std::size_t>(lvi.lParam);
    std::vector<Enemy>& enemies = m_enemyManager->GetEnemies();
    if (index >= enemies.size())
    {
        return;
    }

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

    enemies[index].SetPosition(D3DXVECTOR3(posX, posY, posZ));
    enemies[index].SetYaw(rotY);
    enemies[index].SyncMesh(*m_render);

    HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_ENEMY_TYPE);
    if (hCombo != NULL)
    {
        const int sel = static_cast<int>(SendMessage(hCombo, CB_GETCURSEL, 0, 0));
        if (sel >= 0)
        {
            wchar_t typeBuf[64] = {};
            SendMessage(hCombo, CB_GETLBTEXT, sel, reinterpret_cast<LPARAM>(typeBuf));
            enemies[index].SetType(typeBuf);
        }
    }

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

    if (m_editMode == EditMode::Mesh)
    {
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

        const std::wstring physicsFilePath = stageFolder + L"\\XFileListPhysics." + timeStr + L".csv";

        std::vector<std::vector<std::wstring>> physicsCsv;
        std::vector<std::wstring> physicsHeader;
        physicsHeader.push_back(L"ID");
        physicsHeader.push_back(L"FileName");
        physicsHeader.push_back(L"PosX");
        physicsHeader.push_back(L"PosY");
        physicsHeader.push_back(L"PosZ");
        physicsHeader.push_back(L"RotX");
        physicsHeader.push_back(L"RotY");
        physicsHeader.push_back(L"RotZ");
        physicsHeader.push_back(L"Scale");
        physicsHeader.push_back(L"Type");
        physicsHeader.push_back(L"Move");
        physicsHeader.push_back(L"Instancing");
        physicsCsv.push_back(physicsHeader);

        int physicsId = 1;
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
            row.push_back(std::to_wstring(physicsId));
            row.push_back(relativePath);
            row.push_back(std::to_wstring(model.pos.x));
            row.push_back(std::to_wstring(model.pos.y));
            row.push_back(std::to_wstring(model.pos.z));
            row.push_back(L"0");
            row.push_back(std::to_wstring(rotYDeg));
            row.push_back(L"0");
            row.push_back(std::to_wstring(model.scale));
            row.push_back(L"Collision");
            row.push_back(m_movingPlatformInfos.count(model.renderId) ? L"y" : L"n");
            row.push_back(L"n");
            physicsCsv.push_back(row);
            ++physicsId;
        }

        csv::Write(physicsFilePath, physicsCsv);

        const std::wstring moveFilePath = stageFolder + L"\\XFileListMove." + timeStr + L".csv";

        std::vector<std::vector<std::wstring>> moveCsv;
        std::vector<std::wstring> moveHeader;
        moveHeader.push_back(L"ID");
        moveHeader.push_back(L"RenderID");
        moveHeader.push_back(L"PhysicsID");
        moveHeader.push_back(L"PosX");
        moveHeader.push_back(L"PosY");
        moveHeader.push_back(L"PosZ");
        moveHeader.push_back(L"RotX");
        moveHeader.push_back(L"RotY");
        moveHeader.push_back(L"RotZ");
        moveHeader.push_back(L"Scale");
        moveHeader.push_back(L"StartX");
        moveHeader.push_back(L"StartY");
        moveHeader.push_back(L"StartZ");
        moveHeader.push_back(L"EndX");
        moveHeader.push_back(L"EndY");
        moveHeader.push_back(L"EndZ");
        moveHeader.push_back(L"Duration");
        moveCsv.push_back(moveHeader);

        int moveId = 1;
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

            if (!m_movingPlatformInfos.count(model.renderId))
            {
                continue;
            }

            const MovingPlatformInfo& moveInfo = m_movingPlatformInfos.at(model.renderId);
            D3DXVECTOR3 rot = m_render->GetMeshMixRot(model.renderId);
            const float rotYDeg = rot.y * 180.0f / D3DX_PI;

            std::vector<std::wstring> row;
            row.push_back(std::to_wstring(moveId));
            row.push_back(std::to_wstring(moveId));
            row.push_back(std::to_wstring(moveId));
            row.push_back(std::to_wstring(model.pos.x));
            row.push_back(std::to_wstring(model.pos.y));
            row.push_back(std::to_wstring(model.pos.z));
            row.push_back(L"0");
            row.push_back(std::to_wstring(rotYDeg));
            row.push_back(L"0");
            row.push_back(std::to_wstring(model.scale));
            row.push_back(std::to_wstring(moveInfo.startPos.x));
            row.push_back(std::to_wstring(moveInfo.startPos.y));
            row.push_back(std::to_wstring(moveInfo.startPos.z));
            row.push_back(std::to_wstring(moveInfo.endPos.x));
            row.push_back(std::to_wstring(moveInfo.endPos.y));
            row.push_back(std::to_wstring(moveInfo.endPos.z));
            row.push_back(std::to_wstring(moveInfo.duration));
            moveCsv.push_back(row);
            ++moveId;
        }

        csv::Write(moveFilePath, moveCsv);
    }
    else
    {
        const std::wstring filePath = stageFolder + L"\\EnemyPositions." + timeStr + L".csv";
        m_enemyManager->SaveToCsv(filePath);
    }
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

    const auto moveIt = m_movingPlatformInfos.find(renderId);
    if (moveIt != m_movingPlatformInfos.end())
    {
        const MovingPlatformInfo& info = moveIt->second;
        SetDlgItemText(hDlg, IDC_EDIT_START_X, std::to_wstring(info.startPos.x).c_str());
        SetDlgItemText(hDlg, IDC_EDIT_START_Y, std::to_wstring(info.startPos.y).c_str());
        SetDlgItemText(hDlg, IDC_EDIT_START_Z, std::to_wstring(info.startPos.z).c_str());
        SetDlgItemText(hDlg, IDC_EDIT_END_X, std::to_wstring(info.endPos.x).c_str());
        SetDlgItemText(hDlg, IDC_EDIT_END_Y, std::to_wstring(info.endPos.y).c_str());
        SetDlgItemText(hDlg, IDC_EDIT_END_Z, std::to_wstring(info.endPos.z).c_str());
        SetDlgItemText(hDlg, IDC_EDIT_DURATION, std::to_wstring(info.duration).c_str());
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_MOVING), BM_SETCHECK, BST_CHECKED, 0);
    }
    else
    {
        SetDlgItemText(hDlg, IDC_EDIT_START_X, L"");
        SetDlgItemText(hDlg, IDC_EDIT_START_Y, L"");
        SetDlgItemText(hDlg, IDC_EDIT_START_Z, L"");
        SetDlgItemText(hDlg, IDC_EDIT_END_X, L"");
        SetDlgItemText(hDlg, IDC_EDIT_END_Y, L"");
        SetDlgItemText(hDlg, IDC_EDIT_END_Z, L"");
        SetDlgItemText(hDlg, IDC_EDIT_DURATION, L"");
        SendMessage(GetDlgItem(hDlg, IDC_CHECK_MOVING), BM_SETCHECK, BST_UNCHECKED, 0);
    }
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
