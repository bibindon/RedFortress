#pragma once

#include <vector>
#include <string>
#include <Windows.h>
#include <d3dx9.h>
#include "../../RedFortressModel/Model/StatusManager.h"
#include "../../RedFortressModel/Model/Inventory.h"
#include "../../RedFortressModel/Model/ItemManager.h"
#include <tchar.h>

#if defined(_DEBUG)
#define NEW ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define NEW new
#endif

enum class eSequence
{
    BATTLE,
    ENDING,
    EXIT
};

enum class eBattleState
{
    LOAD,
    TITLE,
    OPENING,
    NORMAL,
    STORY,
    MENU,
    COMMAND,
    CRAFT,
    STOREHOUSE,
    TALK,
    DEAD,
    SLEEP,
    PATCH_TEST,
    PICK_PLANT,
    CUT_TREE,
    VOYAGE,
    VOYAGE3HOURS,
    CREATE_TORCH,
    POPUP,
    REST,
    NONE,
};

enum class eTitleMenu
{
    NOT_DECIDE,
    START,
    CONTINUE,
    DEMO,
    EXIT,
};

class Common
{
public:
    static std::vector<char> get_model_texture_resource(
        const std::wstring& model_name, const std::wstring& texture_name);
    static std::vector<char> get_model_resource(const std::wstring& model_name);
    static std::vector<char> get_sound_resource(const std::wstring& filename);

    static std::vector<std::wstring> split(const std::wstring& s, wchar_t delim);
    static std::wstring ToStringWithPrecision(const float value, const int precision = 1);

    static constexpr int KEY_DEQUE_MAX_SIZE { 300 };
    static constexpr int SIMULTANEOUS_ALLOW_FRAME { 3 };

    static constexpr float ANIMATION_SPEED { 1.0f / 60 };

    static bool DebugMode();

    // ゲームデータを暗号化されているものとして扱い、セーブデータは暗号化するモード
    static bool EncryptMode();

    static bool FasterMode();
    static bool StrongMode();

    // 即売会でのデモなど
    static bool DemoMode() { return true; }

    static bool X64Bit();

    static void SetCursorVisibility(const bool visibility);
    static POINT GetScreenPos();

    static void OutputMsg(const std::wstring&, const int arg);
    static void OutputMsg(const std::wstring&, const D3DXVECTOR3& arg);

    static std::wstring RemoveSubstring(const std::wstring& str, const std::wstring& toRemove);

    static bool EqualF(const float arg1, const float arg2);

    // 点と線分の距離
    static float PointToSegmentDistance(const D3DXVECTOR3& p1,
                                        const D3DXVECTOR3& p2,
                                        const D3DXVECTOR3& point);

    static std::wstring ModExt(const std::wstring& filepath);
    static NSModel::StatusManager* Status();
    static NSModel::Inventory* Inventory();
    static NSModel::ItemManager* ItemManager();

    static void ReduceBrainStaminaCurrent(const float arg);
    static void ReduceBrainStaminaMaxSub(const float arg);

    static void ReduceBodyStaminaCurrent(const float arg);
    static void ReduceBodyStaminaMaxSub(const float arg);

    static bool IsRemoteSession();

    // バウンディングボックスで衝突判定
    static bool HitByBoundingBox(const D3DXVECTOR3& p1,
                                 const D3DXVECTOR3& p2,
                                 const float length);

    static void SetGPUName(const std::wstring& GPUName);
    static std::wstring GetGPUName();

    static std::wstring LoadString_(const UINT resID);
    static std::wstring LoadStringWithArg(const UINT resID, const std::wstring& arg1);

    static std::wstring Utf8ToWstring(const std::string& utf8);

    static std::string WstringToUtf8(const std::wstring& wstr);

    static std::wstring AddEnIfEng(const std::wstring& filename);

    static LPCWSTR FontNameEng();

    static int FontThicknessEng();

    static int FontSizeEng();

private:

    static bool m_bDebugMode;
    static bool m_x64Bit;
    static std::wstring m_GPUName;
};

template <typename T>
inline void SAFE_RELEASE(T*& p)
{
    if (p == nullptr)
    {
        return;
    }

    while (true)
    {
        auto refCnt = p->Release();
        if (refCnt == 0)
        {
            break;
        }
    }
    p = nullptr;
}

template <typename T>
inline void SAFE_DELETE(T*& p)
{
    delete p;
    p = nullptr;
}

template <typename T>
inline void SAFE_DELETE_ARRAY(T*& p)
{
    delete[] p;
    p = nullptr;
}

template<typename T>
using Ptr = std::shared_ptr<T>;

template<typename T>
using Vec = std::vector<T>;

template<typename T1, typename T2>
using Umap = std::unordered_map<T1, T2>;

using Wstr = std::wstring;


