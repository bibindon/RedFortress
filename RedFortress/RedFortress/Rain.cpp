#include "Rain.h"

#include <cassert>

#include "../../RedFortressModel/Model/RainModel.h"

#include "Common.h"
#include "SharedObj.h"
#include "BGM.h"
#include "../../RedFortressModel/Model/WeaponManager.h"

Rain* Rain::m_obj = nullptr;

Rain* Rain::Get()
{
    if (m_obj == nullptr)
    {
        m_obj = NEW Rain();
        m_obj->Init();
    }

    return m_obj;
}

void Rain::Init()
{
    HRESULT result = E_FAIL;
    auto d3dDevice = SharedObj::GetD3DDevice();

    //テクスチャの作成
    result = D3DXCreateTextureFromFile(d3dDevice,
                                       _T("res\\image\\raindrop.png"),
                                       &m_texture);
    assert(result == S_OK);

    // スプライトの作成
    result = D3DXCreateSprite(d3dDevice, &m_sprite);
    assert(result == S_OK);

    for (int i = 0; i < RAINDROP; ++i)
    {
        m_pos.at(i) = D3DXVECTOR3(0.f, 100.f * i, 0.f);
        m_pos.at(i).x = (float)(rand() % 1600);

        // 上のほうに表示しておく
        m_pos.at(i).y = (float)(rand() % 900) - 1000.f;
    }

    m_bShow = true;
}

void Rain::Finalize()
{
    if (m_obj != nullptr)
    {
        SAFE_RELEASE(m_obj->m_texture);
        SAFE_RELEASE(m_obj->m_sprite);

        delete m_obj;
        m_obj = nullptr;
    }
}

void Rain::Update()
{
    static int counter = 0;
    counter++;

    if (counter % 60 == 1)
    {
        NSModel::RainModel::Get()->Update();
    }

    for (int i = 0; i < RAINDROP; ++i)
    {
        m_pos.at(i).y += 80;
        if (m_pos.at(i).y >= 1000.f)
        {
            m_pos.at(i).y = (float)(rand() % 900) - 1000;
            m_pos.at(i).x = (float)(rand() % 1600);
        }
    }
}

void Rain::Draw()
{
    if (!m_bShow)
    {
        for (auto& x : m_pos)
        {
            x.y = -100.f;
        }
    }
    
    bool isRain = NSModel::RainModel::Get()->IsRain();

    if (isRain)
    {
        m_sprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect { };
        rect.left = 0;
        rect.top = 0;
        rect.right = 100;
        rect.bottom = 100;
        D3DXVECTOR3 center(0.f, 0.f, 0.f);
        for (int i = 0; i < RAINDROP; ++i)
        {
            m_sprite->Draw(m_texture,
                           &rect,
                           &center,
                           &m_pos.at(i),
                           0xBBBBBBBB);
        }
        m_sprite->End();
    }
}

bool Rain::IsRain()
{
    return NSModel::RainModel::Get()->IsRain();
}

// タイトル画面を表示している、など、何らかの理由で雨の描画をしないでおきたいときに呼ぶ。
// 雨が降っている・いないこととは関係ない。
void Rain::SetShow(const bool bShow)
{
    m_bShow = bShow;
}


