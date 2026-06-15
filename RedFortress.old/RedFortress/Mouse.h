#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

class Mouse
{
public:
    static void Init(LPDIRECTINPUT8 directInput, HWND hWnd);
    static void Update();
    static void Finalize();

    static LONG GetXDelta();
    static LONG GetYDelta();
    static LONG GetZDelta();
    static bool IsDownLeft();
    static bool IsHoldLeft();
    static bool IsDownRight();
    static bool IsHoldRight();

    static bool IsWheelUp();
    static bool IsWheelDown();

private:
    static LPDIRECTINPUTDEVICE8 m_DIMouse;
    static DIMOUSESTATE2 m_DIMouseState;
    static DIMOUSESTATE2 m_DIMouseStatePrev;
};

