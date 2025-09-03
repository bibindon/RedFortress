#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include <string>

class Sprite
{
public:
    Sprite(std::wstring filename);
    ~Sprite();

    // ??? m_pos.x shows z-order. That should be 0.0f ~ 1.0f.
    void Render(const D3DXVECTOR3& pos, const BYTE transparency = 255, const LONG width = -1);
    void SetFill(const bool isFill);
private:
    LPD3DXSPRITE m_D3DSprite;
    LPDIRECT3DTEXTURE9 m_D3DTexture;

    UINT m_width { };
    UINT m_height { };
    bool m_isFill { false };
};

