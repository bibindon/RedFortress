#pragma once
#include "../../RedFortressResearch/Research/Research.h"
#include "../../RedFortressModel/Model/PatchTestManager.h"

class PatchTestManager2
{

public:

    void Finalize();
    void InitPatch();
    std::wstring Operate();
    void Draw();
    void CreateList();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:

    void QueueTest(const std::wstring& result);

    NSModel::PatchTestManager* GetPatchLib();

    NSResearch::Research m_guiLib;

};

