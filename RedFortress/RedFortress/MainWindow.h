#pragma once

#include <windows.h>
#include <tchar.h>
#include <d3d9.h>
#include <d3dx9.h>

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#include <dinput.h>
#include <string>
#include <chrono>
#include <deque>

#include "Mesh.h"
#include "Sprite.h"
#include "AnimMesh.h"
#include "SeqBattle.h"
#include "Opening.h"
#include "SeqEnding.h"
#include "KeyBoard.h"
#include "SharedObj.h"

class MainWindow
{
public:

    explicit MainWindow(const HINSTANCE&, IKeyBoard* keyboard);
    ~MainWindow();
    int MainLoop();

    // WM_CLOSEメッセージを受信したときに対処する必要するための関数
    static SeqBattle* GetBattleSequence();

private:

    std::wstring m_title;
    MSG m_msg { };
    HWND m_hWnd = nullptr;
    LPDIRECT3D9 m_D3D = nullptr;
    LPDIRECTINPUT8 m_directInput = nullptr;

    eSequence m_sequence = eSequence::BATTLE;

    // WM_CLOSEメッセージを受信したときに対処する必要があるためstaticにする
    static SeqBattle* m_seqBattle;

    SeqEnding* m_seqEnding = nullptr;

    // デバッグ用
    void ShowDebugInfo(const int fps);

    // デバッグ用
    int CalcFPS();

    // デバッグ用
    std::vector<std::chrono::system_clock::time_point> m_vecTime;

    // デバッグ用
    LPD3DXFONT m_D3DFont = nullptr;

    eWindowStyle m_eCurrentWindowStyle = eWindowStyle::NONE;
};

