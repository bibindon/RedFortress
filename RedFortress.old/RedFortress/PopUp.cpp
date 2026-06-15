#include "PopUp.h"
#include "SharedObj.h"

PopUp* PopUp::m_singleTonObj { nullptr };

PopUp* PopUp::Get()
{
    return m_singleTonObj;
}

void PopUp::Finalize()
{
    SAFE_DELETE(m_singleTonObj);
}

PopUp::~PopUp()
{
    SAFE_RELEASE(m_D3DFont);
    SAFE_DELETE(m_sprBack);
}

void PopUp::Init(const bool bEnglish)
{
    m_singleTonObj = NEW PopUp();

    m_singleTonObj->m_sprBack = NEW Sprite(_T("res\\image\\PopupBack.png"));

    LPDIRECT3DDEVICE9 D3DDevice = SharedObj::GetD3DDevice();

    if (!bEnglish)
    {
        D3DXCreateFont(D3DDevice,
                       24,
                       0,
                       FW_NORMAL,
                       1,
                       false,
                       SHIFTJIS_CHARSET,
                       OUT_TT_ONLY_PRECIS,
                       CLEARTYPE_NATURAL_QUALITY,
                       FF_DONTCARE,
                       _T("BIZ UDMincho"),
                       &m_singleTonObj->m_D3DFont);
    }
    else
    {
        D3DXCreateFont(D3DDevice,
                       Common::FontSizeEng(),
                       0,
                       Common::FontThicknessEng(),
                       1,
                       false,
                       DEFAULT_CHARSET,
                       OUT_TT_ONLY_PRECIS,
                       CLEARTYPE_NATURAL_QUALITY,
                       FF_DONTCARE,
                       Common::FontNameEng(),
                       &m_singleTonObj->m_D3DFont);
    }

}

void PopUp::SetText(const std::vector<std::vector<std::wstring>>& arg)
{
    if (arg.size() == 0)
    {
        return;
    }
    m_currentIndex = 0;
    m_textList = arg;
    m_isShow = true;
    m_counter = 0;
}

bool PopUp::IsShow()
{
    return m_isShow;
}

void PopUp::Next()
{
    if (m_counter <= WAIT_NEXT_FRAME)
    {
        return;
    }
    m_currentIndex++;
    m_counter = 0;
    if (m_currentIndex >= (int)m_textList.size())
    {
        m_isShow = false;
        m_textList.clear();
        m_currentIndex = 0;
    }
}

void PopUp::Cancel()
{
    if (m_counter <= WAIT_NEXT_FRAME)
    {
        return;
    }
    m_isShow = false;
    m_textList.clear();
    m_currentIndex = 0;
    m_counter = 0;
}

void PopUp::Update()
{
    m_counter++;
}

void PopUp::Render()
{
    if (m_isShow)
    {
        D3DXVECTOR3 pos { 0.0f, 0.0f, 0.0f };
        m_sprBack->Render(pos);
        RECT rect;
        if (m_textList.at(m_currentIndex).size() >= 1)
        {
            SetRect(&rect, 50, 700, 1550, 200);
            m_D3DFont->DrawText(
                NULL,
                m_textList.at(m_currentIndex).at(0).c_str(),
                -1,
                &rect,
                DT_CENTER | DT_NOCLIP,
                D3DCOLOR_ARGB(255, 255, 255, 255));
        }

        if (m_textList.at(m_currentIndex).size() >= 2)
        {
            SetRect(&rect, 50, 750, 1550, 200);
            m_D3DFont->DrawText(
                NULL,
                m_textList.at(m_currentIndex).at(1).c_str(),
                -1,
                &rect,
                DT_CENTER | DT_NOCLIP,
                D3DCOLOR_ARGB(255, 255, 255, 255));
        }

        if (m_textList.at(m_currentIndex).size() >= 3)
        {
            SetRect(&rect, 50, 800, 1550, 200);
            m_D3DFont->DrawText(
                NULL,
                m_textList.at(m_currentIndex).at(2).c_str(),
                -1,
                &rect,
                DT_CENTER | DT_NOCLIP,
                D3DCOLOR_ARGB(255, 255, 255, 255));
        }
    }
}

// 解像度やウィンドウモードを変更したときのための関数
void PopUp::OnDeviceLost()
{

}

// 解像度やウィンドウモードを変更したときのための関数
void PopUp::OnDeviceReset()
{

}

