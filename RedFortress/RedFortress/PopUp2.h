#pragma once

#include "Sprite.h"
#include <vector>
#include <string>
#include <queue>

class IPopUpFont
{
public:
    virtual void Draw(const std::wstring& text, const int transparent) = 0;
    virtual ~IPopUpFont() { };

    // 解像度やウィンドウモードを変更したときのための関数
    virtual void OnDeviceLost() = 0;

    // 解像度やウィンドウモードを変更したときのための関数
    virtual void OnDeviceReset() = 0;
};

class PopUpFont : public IPopUpFont
{
public:
    PopUpFont(LPDIRECT3DDEVICE9 device, const bool bEnglish);
    void Draw(const std::wstring& text, const int transparent);
    ~PopUpFont();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:
    LPD3DXFONT m_D3DFont = nullptr;
};

// アイテムを拾ったときの演出
// 文字列を1.5秒表示する
// 表示が終わる前に次の文字列が渡されたらキューイングされる
class PopUp2
{
public:
    static void Init(IPopUpFont* font);
    static PopUp2* Get();
    static void Finalize();
    void SetText(const std::wstring& arg);
    void SetText(const UINT res);
    bool IsShow() const;
    void Update();
    void Render();
    void Cancel();
    ~PopUp2();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:
    static PopUp2* m_singleTonObj;

    std::queue<std::wstring> m_textQ;
    int m_counter { 0 };
    bool m_isShow { false };
    const int FADE_IN = 30;
    const int SHOW = 60;
    const int FADE_OUT= 30;
    int m_transparent = 0;

    IPopUpFont* m_font = nullptr;
};


