#include "HudManager.h"
#include <d3d9.h>
#include <d3dx9math.h>
#include "SharedObj.h"
#include "../../RedFortressModel/Model/StatusManager.h"
#include <cassert>
#include "resource.h"

namespace NSHUD
{
class Sprite : public ISprite
{
public:

    Sprite(LPDIRECT3DDEVICE9 dev)
        : m_pD3DDevice(dev)
    {
    }

    void DrawImage(const int percent, const int x, const int y, const int transparency) override
    {
        D3DXVECTOR3 pos { (float)x, (float)y, 0.f };
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect = {
            0,
            0,
            static_cast<LONG>(m_width * percent / 100),
            static_cast<LONG>(m_height / 2) };
        D3DXVECTOR3 center { 0, 0, 0 };
        m_D3DSprite->Draw(
            m_pD3DTexture,
            &rect,
            &center,
            &pos,
            D3DCOLOR_ARGB(transparency, 255, 255, 255));
        m_D3DSprite->End();

    }

    void Load(const std::wstring& filepath) override
    {
        LPD3DXSPRITE tempSprite { nullptr };
        if (FAILED(D3DXCreateSprite(m_pD3DDevice, &m_D3DSprite)))
        {
            throw std::exception("Failed to create a sprite.");
        }

        if (FAILED(D3DXCreateTextureFromFile(
            m_pD3DDevice,
            filepath.c_str(),
            &m_pD3DTexture)))
        {
            throw std::exception("Failed to create a texture.");
        }

        D3DSURFACE_DESC desc { };
        if (FAILED(m_pD3DTexture->GetLevelDesc(0, &desc)))
        {
            throw std::exception("Failed to create a texture.");
        }
        m_width = desc.Width;
        m_height = desc.Height;
    }

    ~Sprite()
    {
        m_D3DSprite->Release();
        m_pD3DTexture->Release();
    }

    void OnDeviceLost()
    {
        m_D3DSprite->OnLostDevice();
    }

    void OnDeviceReset()
    {
        m_D3DSprite->OnResetDevice();
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXSPRITE m_D3DSprite = NULL;
    LPDIRECT3DTEXTURE9 m_pD3DTexture = NULL;
    UINT m_width { 0 };
    UINT m_height { 0 };
};

class Font : public IFont
{
public:

    Font(LPDIRECT3DDEVICE9 pD3DDevice)
        : m_pD3DDevice(pD3DDevice)
    {
    }

    void Init(const bool bEnglish)
    {
        HRESULT hr = S_OK;

        if (!bEnglish)
        {
            hr = D3DXCreateFont(m_pD3DDevice,
                                16,
                                0,
                                FW_NORMAL,
                                1,
                                false,
                                SHIFTJIS_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                CLEARTYPE_NATURAL_QUALITY,
                                FF_DONTCARE,
                                _T("BIZ UDMincho"),
                                &m_pFont);
        }
        else
        {
            hr = D3DXCreateFont(m_pD3DDevice,
                                Common::FontSizeEng() - 4,
                                0,
                                Common::FontThicknessEng(),
                                1,
                                false,
                                DEFAULT_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                CLEARTYPE_NATURAL_QUALITY,
                                FF_DONTCARE,
                                Common::FontNameEng(),
                                &m_pFont);
        }
        assert(hr == S_OK);
    }

    virtual void DrawText_(const std::wstring& msg, const int x, const int y)
    {
        RECT rect = { x, y, 0, 0 };
        m_pFont->DrawText(NULL, msg.c_str(), -1, &rect, DT_LEFT | DT_NOCLIP,
                          D3DCOLOR_ARGB(255, 255, 255, 255));
    }

    ~Font()
    {
        m_pFont->Release();
    }

    void OnDeviceLost()
    {
        m_pFont->OnLostDevice();
    }

    void OnDeviceReset()
    {
        m_pFont->OnResetDevice();
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXFONT m_pFont = NULL;
};

}

void HudManager::Init()
{
    // HUD
    {
        if (m_hud != nullptr)
        {
            delete m_hud;
        }
        m_hud = NEW NSHUD::HUD();
        NSHUD::IFont* pFont = NEW NSHUD::Font(SharedObj::GetD3DDevice());

        NSHUD::ISprite* sprBack = NEW NSHUD::Sprite(SharedObj::GetD3DDevice());
        sprBack->Load(_T("res\\image\\status_back.png"));

        NSHUD::ISprite* sprMiddle = NEW NSHUD::Sprite(SharedObj::GetD3DDevice());
        sprMiddle->Load(_T("res\\image\\status_middle.png"));

        NSHUD::ISprite* sprFront = NEW NSHUD::Sprite(SharedObj::GetD3DDevice());
        sprFront->Load(_T("res\\image\\status_front.png"));

        m_hud->Init(pFont, sprBack, sprMiddle, sprFront, SharedObj::IsEnglish());

        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING146), 100, 100, true);
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING147), 100, 100, true);
    }
}

void HudManager::Finalize()
{
    m_hud->Finalize();
    SAFE_DELETE(m_hud);
}

void HudManager::Update()
{
    NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
    float work1 = 0.f;
    float work2 = 0.f;
    float work3 = 0.f;

    work1 = statusManager->GetBodyStaminaMax();
    work2 = statusManager->GetBodyStaminaMaxSub();
    work3 = statusManager->GetBodyStaminaCurrent();

    m_hud->UpsertStatus(Common::LoadString_(IDS_STRING146),
                        (int)(work3 * 100 / work1),
                        (int)(work2 * 100 / work1),
                        true);

    work1 = statusManager->GetBrainStaminaMax();
    work2 = statusManager->GetBrainStaminaMaxSub();
    work3 = statusManager->GetBrainStaminaCurrent();

    m_hud->UpsertStatus(Common::LoadString_(IDS_STRING147),
                        (int)(work3 * 100 / work1),
                        (int)(work2 * 100 / work1),
                        true);

    work1 = statusManager->GetWaterMax();
    work2 = statusManager->GetWaterCurrent();
    work3 = work2 * 100 / work1;

    // 水分
    // 98%以下なら表示する
    if ((int)work3 <= 98)
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING148), (int)work3, (int)work3, true);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING148));
    }

    // 肉体の修復度
    // 90%以下なら表示する
    work1 = statusManager->GetMuscleMax();
    work2 = statusManager->GetMuscleCurrent();
    work3 = work2 * 100 / work1;

    if ((int)work3 <= 90)
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING149),
                            (int)(work3 * 100 / work1),
                            (int)(work2 * 100 / work1),
                            true);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING149));
    }

    // 10％以下なら表示する
    work1 = statusManager->GetCarboCurrent() * 100 / statusManager->GetCarboMax();
    if (work1 <= 10.f)
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING150), (int)work1, (int)work1, true);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING150));
    }

    work1 = statusManager->GetProteinCurrent() * 100 / statusManager->GetProteinMax();
    if (work1 <= 10.f)
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING151), (int)work1, (int)work1, true);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING151));
    }

    work1 = statusManager->GetLipidCurrent() * 100 / statusManager->GetLipidMax();
    if (work1 <= 10.f)
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING152), (int)work1, (int)work1, true);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING152));
    }

    work1 = statusManager->GetVitaminCurrent() * 100 / statusManager->GetVitaminMax();
    if (work1 <= 10.f)
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING153), (int)work1, (int)work1, true);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING153));
    }

    work1 = statusManager->GetMineralCurrent() * 100 / statusManager->GetMineralMax();
    if (work1 <= 10.f)
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING154), (int)work1, (int)work1, true);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING154));
    }

    if (statusManager->GetFractureArm())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING155), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING155));
    }

    if (statusManager->GetFractureLeg())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING156), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING156));
    }

    if (statusManager->GetHeadache())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING157), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING157));
    }

    if (statusManager->GetCold())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING158), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING158));
    }

    if (statusManager->GetStomachache())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING159), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING159));
    }

    if (statusManager->GetSleep())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING160), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING160));
    }

    if (statusManager->GetDehydration())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING161), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING161));
    }

    if (statusManager->GetLackOfSleep())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING162), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING162));
    }

    if (statusManager->GetDead())
    {
        m_hud->UpsertStatus(Common::LoadString_(IDS_STRING163), 0, 0, false);
    }
    else
    {
        m_hud->RemoveStatus(Common::LoadString_(IDS_STRING163));
    }
}

void HudManager::Draw()
{
    m_hud->Draw();
}

// 解像度やウィンドウモードを変更したときのための関数
void HudManager::OnDeviceLost()
{
    m_hud->OnDeviceLost();
}

// 解像度やウィンドウモードを変更したときのための関数
void HudManager::OnDeviceReset()
{
    m_hud->OnDeviceReset();
}

