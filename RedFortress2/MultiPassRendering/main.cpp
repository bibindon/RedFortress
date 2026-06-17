#pragma comment( lib, "d3d9.lib" )
#if defined(DEBUG) || defined(_DEBUG)
#pragma comment( lib, "d3dx9d.lib" )
#else
#pragma comment( lib, "d3dx9.lib" )
#endif

#include <crtdbg.h>
#include <tchar.h>
#include <windows.h>

#include "GameApp.h"

int WINAPI _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR lpCmdLine,
                     _In_ int nCmdShow)
{
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    GameApp& app = GameApp::Instance();
    if (app.Initialize(hInstance, nCmdShow))
    {
        app.Run();
    }
    app.Finalize();

    return 0;
}
