#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include "GamePad.h"
#include "Common.h"
#include <d3dx9.h>

using std::unordered_map;
using std::deque;

BOOL CALLBACK DeviceFindCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
BOOL SetupGamePadProperty(LPDIRECTINPUTDEVICE8 device);

LPDIRECTINPUT8 GamePad::m_DI;
LPDIRECTINPUTDEVICE8 GamePad::m_DIDevice;
deque<GamePad::GamePadInfo> GamePad::m_deqButton;
float GamePad::m_leftRadian { 0.f };
bool GamePad::m_bLeftStickUsed { false };

bool GamePad::Init(LPDIRECTINPUT8 DI, HWND hwnd)
{
    m_DI = DI;
    // Init button_queue_
    GamePadInfo temp { };
    temp.m_statusMap.insert({ eGamePadButtonType::UP, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::DOWN, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::LEFT, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::RIGHT, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::X, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Y, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::A, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::B, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Z_LEFT, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Z_RIGHT, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Z_UP, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Z_DOWN, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::L1, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::R1, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::L2, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::R2, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::BACK, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::START, eGamePadButtonState::NONE });
    m_deqButton.push_back(temp);
    m_deqButton.push_back(temp);
    m_deqButton.push_back(temp);
    m_deqButton.push_back(temp);
    m_deqButton.push_back(temp);

    // Create device.
    if (!CreateDevice(hwnd))
    {
        // It's not problem failing to create input device.
        Finalize();
        return false;
    }
    return true;
}

void GamePad::Finalize()
{
    if (m_DIDevice != nullptr)
    {
        m_DIDevice->Unacquire();
    }
    SAFE_RELEASE(m_DIDevice);
}

// A struct that receives device enumeration result.
struct DeviceEnumParam
{
    LPDIRECTINPUTDEVICE8* m_argDIDevice;
    int m_findCount;
    HWND m_hWnd;
};

BOOL GamePad::StartGamePadControl()
{
    if (m_DIDevice == nullptr)
    {
        return false;
    }

    if (FAILED(m_DIDevice->Acquire()))
    {
        return false;
    }

    DIDEVCAPS cap = { 0 };
    m_DIDevice->GetCapabilities(&cap);
    // Check polling
    if (cap.dwFlags & DIDC_POLLEDDATAFORMAT)
    {
        DWORD error = GetLastError();
        if (FAILED(m_DIDevice->Poll()))
        {
            return false;
        }
    }
    return true;
}

BOOL CALLBACK DeviceFindCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
    DeviceEnumParam* parameter { (DeviceEnumParam*)pvRef };
    LPDIRECTINPUTDEVICE8 DIDevice { nullptr };

    // Finish if already found.
    if (parameter->m_findCount >= 1)
    {
        return DIENUM_STOP;
    }

    // Create device.
    HRESULT hresult { GamePad::m_DI->CreateDevice(
        lpddi->guidInstance,
        parameter->m_argDIDevice,
        nullptr) };

    if (FAILED(hresult))
    {
        return DIENUM_STOP;
    }

    // Set format.
    DIDevice = *parameter->m_argDIDevice;
    hresult = DIDevice->SetDataFormat(&c_dfDIJoystick);

    if (FAILED(hresult))
    {
        return DIENUM_STOP;
    }

    if (SetupGamePadProperty(DIDevice) == false)
    {
        return DIENUM_STOP;
    }

    // Set Cooperative level
    if (FAILED(DIDevice->SetCooperativeLevel(
        parameter->m_hWnd,
        DISCL_EXCLUSIVE | DISCL_FOREGROUND)))
    {
        return DIENUM_STOP;
    }

    ++parameter->m_findCount;

    return DIENUM_CONTINUE;
}



void GamePad::Update()
{
    if (m_DIDevice == nullptr)
    {
        return;
    }

    DIJOYSTATE padData { };

    // Get device.
    HRESULT hresult { m_DIDevice->GetDeviceState(sizeof(DIJOYSTATE), &padData) };
    if (FAILED(hresult))
    {
        // Start control again.
        if (FAILED(m_DIDevice->Acquire()))
        {
            m_DIDevice->Poll();
        }
        return;
    }

    GamePadInfo temp { };
    temp.m_statusMap.insert({ eGamePadButtonType::UP, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::DOWN, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::LEFT, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::RIGHT, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::X, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Y, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::A, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::B, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Z_LEFT, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Z_RIGHT, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Z_UP, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::Z_DOWN, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::L1, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::R1, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::L2, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::R2, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::BACK, eGamePadButtonState::NONE });
    temp.m_statusMap.insert({ eGamePadButtonType::START, eGamePadButtonState::NONE });
    m_deqButton.push_front(temp);

    if (m_deqButton.size() > Common::KEY_DEQUE_MAX_SIZE)
    {
        m_deqButton.erase(m_deqButton.begin() + Common::KEY_DEQUE_MAX_SIZE,
            m_deqButton.end());
    }

    unordered_map<eGamePadButtonType, bool> is_push_map {
        { eGamePadButtonType::UP, false },
        { eGamePadButtonType::DOWN, false },
        { eGamePadButtonType::LEFT, false },
        { eGamePadButtonType::RIGHT, false },
        { eGamePadButtonType::X, false },
        { eGamePadButtonType::Y, false },
        { eGamePadButtonType::A, false },
        { eGamePadButtonType::B, false },
        { eGamePadButtonType::Z_LEFT, false },
        { eGamePadButtonType::Z_RIGHT, false },
        { eGamePadButtonType::Z_UP, false },
        { eGamePadButtonType::Z_DOWN, false },
        { eGamePadButtonType::L1, false },
        { eGamePadButtonType::R1, false },
        { eGamePadButtonType::L2, false },
        { eGamePadButtonType::R2, false },
        { eGamePadButtonType::BACK, false },
        { eGamePadButtonType::START, false },
    };

    // Check stick.
    int unresponsiveRange { 200 };
    if (padData.lX < -unresponsiveRange)
    {
        is_push_map.at(eGamePadButtonType::LEFT) = true;
    }
    else if (padData.lX > unresponsiveRange)
    {
        is_push_map.at(eGamePadButtonType::RIGHT) = true;
    }

    if (padData.lY < -unresponsiveRange)
    {
        is_push_map.at(eGamePadButtonType::UP) = true;
    }
    else if (padData.lY > unresponsiveRange)
    {
        is_push_map.at(eGamePadButtonType::DOWN) = true;
    }

    if (padData.lZ < -unresponsiveRange)
    {
        is_push_map.at(eGamePadButtonType::Z_LEFT) = true;
    }
    else if (padData.lZ > unresponsiveRange)
    {
        is_push_map.at(eGamePadButtonType::Z_RIGHT) = true;
    }
    if (padData.lRz < -unresponsiveRange)
    {
        is_push_map.at(eGamePadButtonType::Z_UP) = true;
    }
    else if (padData.lRz > unresponsiveRange)
    {
        is_push_map.at(eGamePadButtonType::Z_DOWN) = true;
    }

    m_leftRadian = std::atan2f((float)padData.lX, (float)padData.lY);
    m_leftRadian += D3DX_PI * 3 / 2;
    if (m_leftRadian < 0.f)
    {
        m_leftRadian += D3DX_PI * 2;
    }
    else if (D3DX_PI * 2 <= m_leftRadian)
    {
        m_leftRadian -= D3DX_PI * 2;
    }
    if ((-unresponsiveRange < padData.lX && padData.lX < unresponsiveRange)
        && (-unresponsiveRange < padData.lY && padData.lY < unresponsiveRange))
    {
        m_bLeftStickUsed = false;
    }
    else
    {
        m_bLeftStickUsed = true;
    }

    // Check D-pad
    if (padData.rgdwPOV[0] != 0xFFFFFFFF)
    {
        float rad { D3DXToRadian(padData.rgdwPOV[0] / 100.0f) };
        float x { sinf(rad) };
        float y { cosf(rad) };

        if (x < -0.01f)
        {
            is_push_map.at(eGamePadButtonType::LEFT) = true;
        }
        else if (x > 0.01f)
        {
            is_push_map.at(eGamePadButtonType::RIGHT) = true;
        }

        if (y > 0.01f)
        {
            is_push_map.at(eGamePadButtonType::UP) = true;
        }
        else if (y < -0.01f)
        {
            is_push_map.at(eGamePadButtonType::DOWN) = true;
        }
    }

    // Check button.
    for (int i = 0; i < 32; i++)
    {
        if (!(padData.rgbButtons[i] & 0x80))
        {
            continue;
        }

        switch (i)
        {
        case 0:
            is_push_map.at(eGamePadButtonType::X) = true;
            break;
        case 1:
            is_push_map.at(eGamePadButtonType::A) = true;
            break;
        case 2:
            is_push_map.at(eGamePadButtonType::B) = true;
            break;
        case 3:
            is_push_map.at(eGamePadButtonType::Y) = true;
            break;
        case 4:
            is_push_map.at(eGamePadButtonType::L1) = true;
            break;
        case 5:
            is_push_map.at(eGamePadButtonType::R1) = true;
            break;
        case 6:
            is_push_map.at(eGamePadButtonType::L2) = true;
            break;
        case 7:
            is_push_map.at(eGamePadButtonType::R2) = true;
            break;
        case 8:
            is_push_map.at(eGamePadButtonType::BACK) = true;
            break;
        case 9:
            is_push_map.at(eGamePadButtonType::START) = true;
            break;
        }
    }

    // Update button status.
    for (auto& _is_push : is_push_map)
    {
        if (_is_push.second == true)
        {
            if (m_deqButton.at(1).m_statusMap.at(_is_push.first) == eGamePadButtonState::NONE)
            {
                m_deqButton.at(0).m_statusMap.at(_is_push.first) = eGamePadButtonState::DOWN_FIRST;
            }
            else
            {
                if (m_deqButton.at(1).m_statusMap.at(_is_push.first) == eGamePadButtonState::HOLD)
                {
                    m_deqButton.at(0).m_statusMap.at(_is_push.first) = eGamePadButtonState::HOLD;
                }
                else if (m_deqButton.at(1).m_statusMap.at(_is_push.first) == eGamePadButtonState::DOWN_FIRST)
                {
                    m_deqButton.at(0).m_statusMap.at(_is_push.first) = eGamePadButtonState::DOWN;
                }
                else if (m_deqButton.at(1).m_statusMap.at(_is_push.first) == eGamePadButtonState::HOLD)
                {
                    m_deqButton.at(0).m_statusMap.at(_is_push.first) = eGamePadButtonState::HOLD;
                }
                else if (m_deqButton.at(1).m_statusMap.at(_is_push.first) == eGamePadButtonState::DOWN)
                {
                    bool bHold = false;

                    for (int i = 1; i < m_deqButton.size(); ++i)
                    {
                        if (i >= 30)
                        {
                            bHold = true;
                            break;
                        }

                        if (m_deqButton.at(i).m_statusMap.at(_is_push.first) == eGamePadButtonState::DOWN)
                        {
                            continue;
                        }
                        else
                        {
                            break;
                        }
                    }

                    if (bHold)
                    {
                        m_deqButton.at(0).m_statusMap.at(_is_push.first) = eGamePadButtonState::HOLD;
                    }
                    else
                    {
                        m_deqButton.at(0).m_statusMap.at(_is_push.first) = eGamePadButtonState::DOWN;
                    }
                }
            }
        }
        else
        {
            if (m_deqButton.at(1).m_statusMap.at(_is_push.first) == eGamePadButtonState::HOLD ||
                m_deqButton.at(1).m_statusMap.at(_is_push.first) == eGamePadButtonState::DOWN ||
                m_deqButton.at(1).m_statusMap.at(_is_push.first) == eGamePadButtonState::DOWN_FIRST
                )
            {
                m_deqButton.at(0).m_statusMap.at(_is_push.first) = eGamePadButtonState::UP;
            }
            else
            {
                m_deqButton.at(0).m_statusMap.at(_is_push.first) = eGamePadButtonState::NONE;
            }
        }
    }
}

bool GamePad::IsHold(eGamePadButtonType button)
{
    if (m_deqButton.at(0).m_statusMap.at(button) == eGamePadButtonState::HOLD)
    {
        return true;
    }
    return false;
}

bool GamePad::IsUp(eGamePadButtonType button)
{
    if (m_deqButton.at(0).m_statusMap.at(button) == eGamePadButtonState::UP)
    {
        return true;
    }
    return false;
}

bool GamePad::IsDownFirst(eGamePadButtonType button)
{
    if (m_deqButton.at(0).m_statusMap.at(button) == eGamePadButtonState::DOWN_FIRST)
    {
        return true;
    }
    return false;
}

bool GamePad::IsDown(eGamePadButtonType button)
{
    if (m_deqButton.at(0).m_statusMap.at(button) == eGamePadButtonState::DOWN ||
        m_deqButton.at(0).m_statusMap.at(button) == eGamePadButtonState::HOLD)
    {
        return true;
    }
    return false;
}

float GamePad::GetLeftRadian()
{
    return m_leftRadian;
}

bool GamePad::IsLeftStickUsed()
{
    return m_bLeftStickUsed;
}

bool GamePad::CheckSimultaneous(eGamePadButtonType button)
{
    for (std::size_t i = 0; i < Common::SIMULTANEOUS_ALLOW_FRAME; ++i)
    {
        if (i >= m_deqButton.size())
        {
            break;
        }
        if (m_deqButton.at(i).m_statusMap.at(button) == eGamePadButtonState::DOWN_FIRST)
        {
            return true;
        }
    }
    return false;
}

bool GamePad::CreateDevice(HWND hwnd)
{
    DeviceEnumParam parameter = { 0 };

    LPDIRECTINPUTDEVICE8 tempDIDevice { nullptr };
    parameter.m_argDIDevice = &tempDIDevice;
    parameter.m_hWnd = hwnd;

    // Search GAMEPAD
    m_DI->EnumDevices(
        DI8DEVTYPE_GAMEPAD,
        DeviceFindCallback,
        &parameter,
        DIEDFL_ATTACHEDONLY
    );

    // Search JOYSTICK
    m_DI->EnumDevices(
        DI8DEVTYPE_JOYSTICK,
        DeviceFindCallback,
        &parameter,
        DIEDFL_ATTACHEDONLY
    );

    if (parameter.m_findCount == 0)
    {
        return false;
    }

    SAFE_RELEASE(m_DIDevice);
    m_DIDevice = tempDIDevice;

    int count { 0 };
    while (StartGamePadControl() == false)
    {
        Sleep(100);
        count++;
        if (count >= 5)
        {
            break;
        }
    }
    return true;
}

BOOL SetupGamePadProperty(LPDIRECTINPUTDEVICE8 device)
{
    // Set coordinate type to abusolute value mode.
    DIPROPDWORD diprop;
    ZeroMemory(&diprop, sizeof(diprop));
    diprop.diph.dwSize = sizeof(diprop);
    diprop.diph.dwHeaderSize = sizeof(diprop.diph);
    diprop.diph.dwHow = DIPH_DEVICE;
    diprop.diph.dwObj = 0;
    diprop.dwData = DIPROPAXISMODE_ABS;
    if (FAILED(device->SetProperty(DIPROP_AXISMODE, &diprop.diph)))
    {
        return false;
    }

    // Set X ranges
    DIPROPRANGE diprg;
    ZeroMemory(&diprg, sizeof(diprg));
    diprg.diph.dwSize = sizeof(diprg);
    diprg.diph.dwHeaderSize = sizeof(diprg.diph);
    diprg.diph.dwHow = DIPH_BYOFFSET;
    diprg.diph.dwObj = DIJOFS_X;
    diprg.lMin = -1000;
    diprg.lMax = 1000;
    if (FAILED(device->SetProperty(DIPROP_RANGE, &diprg.diph)))
    {
        return false;
    }

    // Set Y ranges.
    diprg.diph.dwObj = DIJOFS_Y;
    if (FAILED(device->SetProperty(DIPROP_RANGE, &diprg.diph)))
    {
        return false;
    }

    // Set Z ranges.
    diprg.diph.dwObj = DIJOFS_Z;
    if (FAILED(device->SetProperty(DIPROP_RANGE, &diprg.diph)))
    {
        return false;
    }

    // Set Right guriguri ranges.
    diprg.diph.dwObj = DIJOFS_RZ;
    if (FAILED(device->SetProperty(DIPROP_RANGE, &diprg.diph)))
    {
        return false;
    }
    return true;
}

