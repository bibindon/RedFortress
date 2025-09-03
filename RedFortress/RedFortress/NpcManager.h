#pragma once

#include <d3dx9.h>
#include "../../RedFortressModel/Model/NpcStatusManager.h"


class NpcManager
{
public:

    static NpcManager* Get();
    void SetPos(const std::wstring& npcName,
                const float fx,
                const float fy,
                const float fz);

    void SetRot(const std::wstring& npcName,
                const float fRot);

    void SetTalkEnable(const std::wstring& npcName,
                       const bool arg);

    void SetTalkScript(const std::wstring& npcName,
                       const std::wstring& arg);

    void SetEnableFeature(const std::wstring& npcName,
                          const bool arg);

    NSModel::NpcStatus GetNpcStatus(const std::wstring& npcName);

    void SetNpcStatus(const std::wstring& npcName, const NSModel::NpcStatus& status);

    bool GetNpcTalkable(const D3DXVECTOR3& pos, std::wstring* npcName = nullptr);

    void Update();

    static void Finalize();

private:

    static NpcManager* m_ton;

};

