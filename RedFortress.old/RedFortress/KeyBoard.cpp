#include "KeyBoard.h"
#include "Common.h"

void KeyBoard::Init(LPDIRECTINPUT8 directInput, HWND hWnd)
{
    HRESULT ret = directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);

    ret = m_keyboard->SetDataFormat(&c_dfDIKeyboard);

    // 排他制御のセット
    ret = m_keyboard->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);

    // 動作開始
    ret = m_keyboard->Acquire();
}

void KeyBoard::Update()
{
    // キーの入力
    memcpy(m_keyPrev, m_key, 256);
    ZeroMemory(m_key, sizeof(m_key));
    HRESULT ret = m_keyboard->GetDeviceState(sizeof(m_key), m_key);
    if (FAILED(ret))
    {
        // 失敗なら再開させてもう一度取得
        m_keyboard->Acquire();
        m_keyboard->GetDeviceState(sizeof(m_key), m_key);
    }

    std::vector<BYTE> temp(256);
    std::copy(&m_key[0], &m_key[256], temp.begin());
    m_keyDeque.push_front(temp);

    // 5秒分のキー情報、以上のキー情報が保存されているなら消す
    if (m_keyDeque.size() >= 60 * 5)
    {
        m_keyDeque.erase(m_keyDeque.begin() + 60 * 5, m_keyDeque.end());
    }
}

void KeyBoard::Finalize()
{
    SAFE_RELEASE(m_keyboard);
}

bool KeyBoard::IsDown(int keyCode)
{
    if (m_key[keyCode] & 0x80)
    {
        return true;
    }
    return false;
}

bool KeyBoard::IsDownFirstFrame(int keyCode)
{
    if (m_key[keyCode] & 0x80)
    {
        if ((m_keyPrev[keyCode] & 0x80) == 0)
        {
            return true;
        }
    }
    return false;
}

bool KeyBoard::IsHold(int keyCode)
{
    // 500ミリ秒以上押されていたら長押しと判断する
    if (m_keyDeque.size() <= 30)
    {
        return false;
    }

    bool isHold = true;
    for (std::size_t i = 0; i < 30; ++i)
    {
        if (m_keyDeque.at(i).at((std::size_t)keyCode) & 0x80)
        {
            continue;
        }
        else
        {
            isHold = false;
            break;
        }
    }

    if (isHold)
    {
        return true;
    }

    return isHold;
}


