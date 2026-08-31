#pragma comment( lib, "d3d9.lib" )
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "d3dx9d.lib" )
#else
#pragma comment( lib, "d3dx9.lib" )
#endif

#include <crtdbg.h>
#include <cstdlib>
#include <sstream>
#include <string>
#include <tchar.h>
#include <windows.h>

#include "GameApp.h"
#include "GameAudio.h"

namespace
{
    bool HasSilentSwitch(const wchar_t* commandLine)
    {
        std::wistringstream commandStream(commandLine);
        std::wstring argument;
        while (commandStream >> argument)
        {
            if (argument == L"--silent")
            {
                return true;
            }
        }
        return false;
    }

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
    std::wstring GetDebugStartupStageId(const wchar_t* commandLine)
    {
        std::wistringstream commandStream(commandLine);
        std::wstring argument;
        while (commandStream >> argument)
        {
            if (argument != L"--stage")
            {
                continue;
            }

            std::wstring stageId;
            commandStream >> stageId;
            if (stageId.empty())
            {
                std::abort();
            }
            return stageId;
        }

        return L"";
    }
#endif
}

int WINAPI _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR lpCmdLine,
                     _In_ int nCmdShow)
{
    (void)hPrevInstance;

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    GameAudio::SetSilentMode(HasSilentSwitch(lpCmdLine));

#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
    const std::wstring debugStartupStageId = GetDebugStartupStageId(lpCmdLine);
#else
    (void)lpCmdLine;
#endif

    GameApp& app = GameApp::Instance();
    if (app.Initialize(hInstance, nCmdShow))
    {
#if defined(_DEBUG) || defined(REDFORTRESS_ENABLE_RPC)
        if (!debugStartupStageId.empty() && !app.LoadStageForDebug(debugStartupStageId))
        {
            std::abort();
        }
#endif
        app.Run();
    }
    app.Finalize();

    return 0;
}
