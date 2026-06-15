#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif

#include <dinput.h>
#include <deque>
#include <vector>

class IKeyBoard
{
public:
    virtual void Init(LPDIRECTINPUT8 directInput, HWND hWnd) = 0;
    virtual void Update() = 0;
    virtual void Finalize() = 0;

    // 押されている
    virtual bool IsDown(int keyCode) = 0;

    // 押されていて、その直前まで押されていなかった
    virtual bool IsDownFirstFrame(int keyCode) = 0;

    // 0.5秒以上長押ししていた
    virtual bool IsHold(int keyCode) = 0;

};

class KeyBoard : public IKeyBoard
{
public:
    void Init(LPDIRECTINPUT8 directInput, HWND hWnd);
    void Update();
    void Finalize();

    // 押されている
    bool IsDown(int keyCode);

    // 押されていて、その直前まで押されていなかった
    bool IsDownFirstFrame(int keyCode);

    // 0.5秒以上長押ししていた
    bool IsHold(int keyCode);

private:
    LPDIRECTINPUTDEVICE8 m_keyboard;
    BYTE m_key[256];
    BYTE m_keyPrev[256];
    std::deque<std::vector<BYTE>> m_keyDeque;
};

