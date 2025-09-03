#include "Common.h"
#include <sstream>
#include <iomanip>
#include "SharedObj.h"
#include <cfloat>
#include <wtsapi32.h>
#include "resource.h"
#include <cstdio>

// リモート接続検出用
#pragma comment(lib, "Wtsapi32.lib")

#if defined(_DEBUG)
bool Common::m_bDebugMode = true;
#else
bool Common::m_bDebugMode = false;
#endif

#ifdef _WIN64
bool Common::m_x64Bit = true;
#else
bool Common::m_x64Bit = false;
#endif

std::wstring Common::m_GPUName;

std::vector<char> Common::get_model_texture_resource(
    const std::wstring& model_name, const std::wstring& texture_name)
{
    return std::vector<char>();
}

std::vector<char> Common::get_model_resource(const std::wstring& model_name)
{
    return std::vector<char>();
}

std::vector<char> Common::get_sound_resource(const std::wstring& filename)
{
    return std::vector<char>();
}

std::vector<std::wstring> Common::split(const std::wstring& s, wchar_t delim)
{
    std::vector<std::wstring> result;
    std::wstringstream ss(s);
    std::wstring item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

std::wstring Common::ToStringWithPrecision(const float value, const int precision)
{
    std::wostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

bool Common::DebugMode()
{
    return m_bDebugMode;
}

bool Common::EncryptMode()
{
    return false;
}

bool Common::FasterMode()
{
    bool bFasterMode = true;

    // デバッグモードじゃなければ常にOFF
    if (!m_bDebugMode)
    {
        return false;
    }

    return bFasterMode;
}

bool Common::StrongMode()
{
    bool bStrongMode = false;

    // デバッグモードじゃなければ常にOFF
    if (!m_bDebugMode)
    {
        return false;
    }

    return bStrongMode;
}

bool Common::X64Bit()
{
    return m_x64Bit;
}

void Common::SetCursorVisibility(const bool visibility)
{
    // Win32APIのShowCursor関数は内部でカウンターを持っており、
    // 内部カウンターが0以下にならないと非表示にならない。
    // 1以上にならないと表示にならない。
    // Directx9にもShowCursor関数があり、こちらはカウンターを持っていない。
    // しかし、D3DCREATE_SOFTWARE_VERTEXPROCESSINGを指定して
    // 頂点処理をソフトウェアで処理しないとカーソルが表示できないらしい。

    int work = 0;
    if (visibility)
    {
        do
        {
            work = ShowCursor(true);
        }
        while (work <= -1);

        ClipCursor(NULL);

        auto hWnd = SharedObj::GetWindowHandle();

        if (hWnd != nullptr)
        {
            RECT rectClient { };
            GetClientRect(hWnd, &rectClient);

            POINT pt { };
            ClientToScreen(hWnd, &pt);

            rectClient.top += pt.y;
            rectClient.left += pt.x;

            // CursorOn処理でゲームパッドと取り合いになるので
            // マウスカーソルを左上に移動しておく
            SetCursorPos(rectClient.left, rectClient.top);
        }
    }
    else
    {
        do
        {
            work = ShowCursor(false);
        }
        while (work >= 0);

        auto hWnd = SharedObj::GetWindowHandle();

        if (hWnd != nullptr)
        {
            RECT rectClient { };
            GetClientRect(hWnd, &rectClient);

            POINT pt { };
            ClientToScreen(hWnd, &pt);

            rectClient.top += pt.y;
            rectClient.left += pt.x;
            rectClient.bottom += pt.y;
            rectClient.right += pt.x;

            ClipCursor(&rectClient);

            // CursorOn処理でゲームパッドと取り合いになるので
            // マウスカーソルを左上に移動しておく
            SetCursorPos(rectClient.left, rectClient.top);
        }
    }
}

POINT Common::GetScreenPos()
{
    POINT p;
    GetCursorPos(&p);

    ScreenToClient(FindWindow(LoadString_(IDS_STRING101).c_str(), nullptr), &p);

    return p;
}

void Common::OutputMsg(const std::wstring& str, const int arg)
{
    std::wstring work;
    work = str + std::to_wstring(arg) + _T("\n");
    OutputDebugString(work.c_str());
}

void Common::OutputMsg(const std::wstring& str, const D3DXVECTOR3& arg)
{
    std::wstring work;
    work = str + _T("x: ") + std::to_wstring(arg.x) + _T("), ");
    work += _T("y: ") + std::to_wstring(arg.y) + _T("), ");
    work += _T("z: ") + std::to_wstring(arg.z) + _T("\n");
    OutputDebugString(work.c_str());
}

std::wstring Common::RemoveSubstring(const std::wstring& str, const std::wstring& toRemove)
{
    std::wstring result = str;
    size_t pos = std::wstring::npos;

    // 指定された文字列が見つかる限りループ
    while ((pos = result.find(toRemove)) != std::wstring::npos)
    {
        result.erase(pos, toRemove.length());
    }
    return result;
}

bool Common::EqualF(const float arg1, const float arg2)
{
    return abs(arg1 - arg2) < FLT_EPSILON;
}

    // 点と線分の距離を計算する関数
float Common::PointToSegmentDistance(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, const D3DXVECTOR3& point)
{
    // 線分をベクトルとして扱う
    D3DXVECTOR3 segment = p2 - p1;
    D3DXVECTOR3 pointToStart = point - p1;

    // 線分の長さの2乗
    float segmentLengthSquared = D3DXVec3LengthSq(&segment);

    if (segmentLengthSquared == 0.0f)
    {
        // 線分が点の場合、点と始点（または終点）の距離を返す
        auto temp1(point - p1);
        return D3DXVec3Length(&temp1);
    }

    // 点を線分に投影
    float t = D3DXVec3Dot(&pointToStart, &segment) / segmentLengthSquared;

    // t を 0 ～ 1 にクランプ（投影点が線分上にあるか確認）
    t = (std::max)(0.0f, (std::min)(1.0f, t));

    // 投影点の座標を計算
    D3DXVECTOR3 projection = p1 + t * segment;

    // 点と投影点の距離を計算
    auto temp2(point - projection);
    return D3DXVec3Length(&temp2);
}

std::wstring Common::ModExt(const std::wstring& filepath)
{
    std::wstring work = filepath;
    if (Common::EncryptMode())
    {
        work = work.replace(work.size() - 3, 3, _T("enc"));
    }

    return work;
}

NSStarmanLib::StatusManager* Common::Status()
{
    return NSStarmanLib::StatusManager::GetObj();
}

NSStarmanLib::Inventory* Common::Inventory()
{
    return NSStarmanLib::Inventory::GetObj();
}

NSStarmanLib::ItemManager* Common::ItemManager()
{
    return NSStarmanLib::ItemManager::GetObj();
}

void Common::ReduceBrainStaminaCurrent(const float arg)
{
    auto work = Status()->GetBrainStaminaCurrent();
    Status()->SetBrainStaminaCurrent(work - arg);
}

void Common::ReduceBrainStaminaMaxSub(const float arg)
{
    auto work = Status()->GetBrainStaminaMaxSub();
    Status()->SetBrainStaminaMaxSub(work - arg);
}

void Common::ReduceBodyStaminaCurrent(const float arg)
{
    auto work = Status()->GetBodyStaminaCurrent();
    Status()->SetBodyStaminaCurrent(work - arg);
}

void Common::ReduceBodyStaminaMaxSub(const float arg)
{
    auto work = Status()->GetBodyStaminaMaxSub();
    Status()->SetBodyStaminaMaxSub(work - arg);
}

bool Common::IsRemoteSession()
{
    // 最初の一回目だけチェックして、2回目以降はチェックした内容を返すようにする。
    static bool isRemote = false;
    static bool checked = false;

    if (!checked)
    {
        // 正常動作せず
        
//        WTS_SESSION_INFO* pSessionInfo = nullptr;
//        DWORD sessionCount = 0;
//
//        // 現在のセッションを取得
//        if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &sessionCount))
//        {
//            for (DWORD i = 0; i < sessionCount; ++i)
//            {
//                std::wstring work(pSessionInfo[i].pWinStationName);
//                if (work.find(_T("RDP")) != std::wstring::npos)
//                {
//                    isRemote = true;
//                    break;
//                }
//            }
//            WTSFreeMemory(pSessionInfo);
//        }
        checked = true;
    }
    return isRemote; // 判定できない場合
}

bool Common::HitByBoundingBox(const D3DXVECTOR3& p1,
                              const D3DXVECTOR3& p2,
                              const float length)
{
    float work = 0.f;
    work = std::abs(p1.x - p2.x);
    if (work <= length)
    {
        work = std::abs(p1.y - p2.y);
        if (work <= length)
        {
            work = std::abs(p1.z - p2.z);
            if (work <= length)
            {
                return true;
            }
        }
    }
    return false;
}

void Common::SetGPUName(const std::wstring& GPUName)
{
    m_GPUName = GPUName;
}

std::wstring Common::GetGPUName()
{
    return m_GPUName;
}

std::wstring Common::LoadString_(const UINT resID)
{
    TCHAR buf[1024];
    HMODULE hInstance = GetModuleHandle(NULL);
    LoadString(hInstance, resID, buf, 1024);
    return std::wstring(buf);
}

std::wstring Common::LoadStringWithArg(const UINT resID, const std::wstring& arg1)
{
    // 「_T("%s")を受け取った」みたいな文字列
    TCHAR buf[1024];
    HMODULE hInstance = GetModuleHandle(NULL);
    LoadString(hInstance, resID, buf, 1024);
    std::wstring work(buf);

    wchar_t buffer[1024];
    _snwprintf_s(buffer, sizeof(buffer) / sizeof(buffer[0]), _TRUNCATE, work.c_str(), arg1.c_str());
    std::wstring message(buffer);
    return message;
}

std::wstring Common::Utf8ToWstring(const std::string& utf8)
{
    if (utf8.empty()) return std::wstring();

    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (len == 0) throw std::runtime_error("UTF-8 to UTF-16 conversion failed.");

    std::wstring result(len - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], len);
    return result;
}

std::string Common::WstringToUtf8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();

    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len == 0) throw std::runtime_error("UTF-16 to UTF-8 conversion failed.");

    std::string result(len - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], len, nullptr, nullptr);
    return result;
}

std::wstring Common::AddEnIfEng(const std::wstring& filename)
{
    if (!SharedObj::IsEnglish())
    {
        return filename;
    }

    const std::wstring extension = L".csv";
    size_t pos = filename.rfind(extension);

    if (pos == std::wstring::npos ||
        pos + extension.length() != filename.length())
    {
        // 拡張子が .csv でない場合は何もしない
        return filename;
    }

    // "abc" + ".en" + ".csv"
    return filename.substr(0, pos) + L".en" + extension;
}

LPCWSTR Common::FontNameEng()
{
    if (true)
    {
        //return L"Calibri";
        return L"Calibri Light";
    }
    else
    {
        return L"Courier New";
    }
}

int Common::FontThicknessEng()
{
    return FW_EXTRALIGHT;
}

int Common::FontSizeEng()
{
    return 24;
}

