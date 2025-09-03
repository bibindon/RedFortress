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

private:

    void QueueTest(const std::wstring& result);

    NSModel::PatchTestManager* GetPatchLib();

    NSPatchTestLib::PatchTestLib m_guiLib;

};

