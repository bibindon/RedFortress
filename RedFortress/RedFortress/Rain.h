#pragma once

#include <array>

#include <d3dx9.h>

// 雨の水滴を描画することはできるが、空を暗くすることはできない。
// 外を暗くする処理だけは例外的にMapクラスにやってもらう。
class Rain
{
public:

    static Rain* Get();
    static void Finalize();
    void Update();
    void Draw();
    bool IsRain();

    // タイトル画面を表示している、など、何らかの理由で雨の描画をしないでおきたいときに呼ぶ。
    // 雨が降っている・いないこととは関係ない。
    void SetShow(const bool bShow);

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceLost();

    // 解像度やウィンドウモードを変更したときのための関数
    void OnDeviceReset();

private:

    void Init();

    static Rain* m_obj;
    LPD3DXSPRITE m_sprite = NULL;
    LPDIRECT3DTEXTURE9 m_texture = NULL;

    static constexpr int RAINDROP = 6;
    std::array<D3DXVECTOR3, RAINDROP> m_pos;

    bool m_bShow = false;
};

