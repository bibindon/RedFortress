#include "Mouse.h"
#include <exception>
#include "Common.h"

LPDIRECTINPUTDEVICE8 Mouse::m_DIMouse;
DIMOUSESTATE2 Mouse::m_DIMouseState;
DIMOUSESTATE2 Mouse::m_DIMouseStatePrev;

void Mouse::Init(LPDIRECTINPUT8 directInput, HWND hWnd)
{
    HRESULT ret = directInput->CreateDevice(GUID_SysMouse, &m_DIMouse, NULL);

    ret = m_DIMouse->SetDataFormat(&c_dfDIMouse2);
    ret = m_DIMouse->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

    // デバイスの設定
    DIPROPDWORD diprop;
    diprop.diph.dwSize = sizeof(diprop);
    diprop.diph.dwHeaderSize = sizeof(diprop.diph);
    diprop.diph.dwObj = 0;
    diprop.diph.dwHow = DIPH_DEVICE;
    diprop.dwData = DIPROPAXISMODE_REL; // 相対値モードで設定（絶対値はDIPROPAXISMODE_ABS）

    ret = m_DIMouse->SetProperty(DIPROP_AXISMODE, &diprop.diph);

    if (FAILED(ret))
    {
        throw std::exception("Failed initialize mouse");
    }
    ret = m_DIMouse->Acquire();
}

void Mouse::Update()
{
    m_DIMouse->Acquire();
    memcpy(&m_DIMouseStatePrev, &m_DIMouseState, sizeof(m_DIMouseStatePrev));

    HRESULT hr = m_DIMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &m_DIMouseState);
    if (hr == DIERR_INPUTLOST)
    {
        m_DIMouse->Acquire();
        hr = m_DIMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &m_DIMouseState);
    }
}

void Mouse::Finalize()
{
    SAFE_RELEASE(m_DIMouse);
}

LONG Mouse::GetXDelta()
{
    return m_DIMouseState.lX;
}

LONG Mouse::GetYDelta()
{
    return m_DIMouseState.lY;
}

LONG Mouse::GetZDelta()
{
    return m_DIMouseState.lZ;
}

bool Mouse::IsDownLeft()
{
    if (m_DIMouseState.rgbButtons[0] & 0x80)
    {
        if ((m_DIMouseStatePrev.rgbButtons[0] & 0x80) == 0x00)
        {
            return true;
        }
    }
    return false;
}

bool Mouse::IsHoldLeft()
{
    if (m_DIMouseState.rgbButtons[0] & 0x80)
    {
        return true;
    }
    return false;
}

bool Mouse::IsDownRight()
{
    if (m_DIMouseState.rgbButtons[1] & 0x80)
    {
        if ((m_DIMouseStatePrev.rgbButtons[1] & 0x80) == 0x00)
        {
            return true;
        }
    }
    return false;
}

bool Mouse::IsHoldRight()
{
    if (m_DIMouseState.rgbButtons[1] & 0x80)
    {
        return true;
    }
    return false;
}

bool Mouse::IsWheelUp()
{
    return m_DIMouseState.lZ > 0;
}

bool Mouse::IsWheelDown()
{
    return m_DIMouseState.lZ < 0;
}
