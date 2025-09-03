
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3d9.lib")
#if defined(NDEBUG)
#pragma comment(lib, "d3dx9.lib")
#else
#pragma comment(lib, "d3dx9d.lib")
#endif
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "dbghelp.lib")

#if (_MSC_VER >= 1920) && (_MSC_VER <= 1929)
#   pragma comment(lib, "../../RedFortressSlideShow/x64/Debug_VisualStudio2019/SlideShow.lib")
#   pragma comment(lib, "../../RedFortressTalk2D/x64/Debug_VisualStudio2019/Talk2D.lib")
#   pragma comment(lib, "../../RedFortressQuestSystem/x64/Debug_VisualStudio2019/QuestSystem.lib")
#   pragma comment(lib, "../../RedFortressMenu/x64/Debug_VisualStudio2019/Menu.lib")
#   pragma comment(lib, "../../RedFortressModel/x64/Debug_VisualStudio2019/Model.lib")
#   pragma comment(lib, "../../RedFortressChest/x64/Debug_VisualStudio2019/Chest.lib")
#   pragma comment(lib, "../../RedFortressCraft/x64/Debug_VisualStudio2019/Craft.lib")
#   pragma comment(lib, "../../RedFortressHUD/x64/Debug_VisualStudio2019/HUD.lib")
#   pragma comment(lib, "../../RedFortressCommand/x64/Debug_VisualStudio2019/Command.lib")
#   pragma comment(lib, "../../RedFortressResearch/x64/Debug_VisualStudio2019/Research.lib")
#else
#   if defined(NDEBUG)
#       pragma comment(lib, "../../RedFortressSlideShow/x64/Release/SlideShow.lib")
#       pragma comment(lib, "../../RedFortressTalk2D/x64/Release/Talk2D.lib")
#       pragma comment(lib, "../../RedFortressQuestSystem/x64/Release/QuestSystem.lib")
#       pragma comment(lib, "../../RedFortressMenu/x64/Release/Menu.lib")
#       pragma comment(lib, "../../RedFortressModel/x64/Release/Model.lib")
#       pragma comment(lib, "../../RedFortressChest/x64/Release/Chest.lib")
#       pragma comment(lib, "../../RedFortressCraft/x64/Release/Craft.lib")
#       pragma comment(lib, "../../RedFortressHUD/x64/Release/HUD.lib")
#       pragma comment(lib, "../../RedFortressCommand/x64/Release/Command.lib")
#       pragma comment(lib, "../../RedFortressResearch/x64/Release/Research.lib")
#   else
#       pragma comment(lib, "../../RedFortressSlideShow/x64/Debug/SlideShow.lib")
#       pragma comment(lib, "../../RedFortressTalk2D/x64/Debug/Talk2D.lib")
#       pragma comment(lib, "../../RedFortressQuestSystem/x64/Debug/QuestSystem.lib")
#       pragma comment(lib, "../../RedFortressMenu/x64/Debug/Menu.lib")
#       pragma comment(lib, "../../RedFortressModel/x64/Debug/Model.lib")
#       pragma comment(lib, "../../RedFortressChest/x64/Debug/Chest.lib")
#       pragma comment(lib, "../../RedFortressCraft/x64/Debug/Craft.lib")
#       pragma comment(lib, "../../RedFortressHUD/x64/Debug/HUD.lib")
#       pragma comment(lib, "../../RedFortressCommand/x64/Debug/Command.lib")
#       pragma comment(lib, "../../RedFortressResearch/x64/Debug/Research.lib")
#   endif
#endif

#include <windows.h>
#include "MainWindow.h"
#include "StackBackTrace.h"
#include <crtdbg.h>

// 例外で終了したときに、例外発生時のスタックトレースを出力する
static void se_translator(unsigned int u, _EXCEPTION_POINTERS* e)
{
    StackBackTrace stackBackTrace;
    std::string output = stackBackTrace.build();
    std::ofstream ofs(_T("error.log"));
    ofs << output;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance,
    _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    // メモリリーク検出 一番最初に書かないと動作しない
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    if (false)
    {
        _CrtSetBreakAlloc(184998);
    }

    // 例外で終了したときに、例外発生時のスタックトレースを出力する
    _set_se_translator(se_translator);

    // Windowsによるスケーリングをさせないようにする。
    SetProcessDPIAware();

    HWND hWnd = FindWindow(_T("ホシマン"), nullptr);
    if (hWnd != nullptr)
    {
        MessageBox(NULL, _T("ホシマンはすでに起動済みです。"), _T("二重起動エラー"), MB_OK);
        return 0;
    }

    try
    {
        Ptr<IKeyBoard> keyboard ( NEW KeyBoard());
        MainWindow window(hInstance, keyboard.get());
        window.MainLoop();
    }
    catch (...)
    {
    }

    return 0;
}

