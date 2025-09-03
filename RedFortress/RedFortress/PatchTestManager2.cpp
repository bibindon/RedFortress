
#include <d3d9.h>
#include <d3dx9.h>

#include "../../RedFortressModel/Model/PowereggDateTime.h"
#include "../../RedFortressModel/Model/ItemManager.h"
#include "../../RedFortressModel/Model/Inventory.h"

#include "Common.h"
#include "SharedObj.h"
#include "SoundEffect.h"
#include "KeyBoard.h"
#include "Mouse.h"
#include "SaveManager.h"
#include "GamePad.h"

#include "PatchTestManager2.h"
#include "PopUp2.h"
#include "resource.h"
#include "Camera.h"

namespace NSResearch
{
// 画像のファイル名が同じなら読み込まずに共有するようにする。
// さもないと簡単にメモリー不足で落ちる
class Sprite : public ISprite
{
public:

    Sprite(LPDIRECT3DDEVICE9 dev)
        : m_pD3DDevice(dev)
    {
    }

    void DrawImage(const int x, const int y, const int transparency) override
    {
        D3DXVECTOR3 pos { (float)x, (float)y, 0.f };
        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);
        RECT rect = {
            0,
            0,
            static_cast<LONG>(m_width),
            static_cast<LONG>(m_height) };
        D3DXVECTOR3 center { 0, 0, 0 };
        m_D3DSprite->Draw(
            m_texMap.at(m_filepath),
            &rect,
            &center,
            &pos,
            D3DCOLOR_ARGB(transparency, 255, 255, 255));
        m_D3DSprite->End();

    }

    void Load(const std::wstring& filepath) override
    {
        // スプライトは一つのみ確保し使いまわす
        if (m_D3DSprite == NULL)
        {
            if (FAILED(D3DXCreateSprite(m_pD3DDevice, &m_D3DSprite)))
            {
                throw std::exception("Failed to create a sprite.");
            }
        }

        m_filepath = filepath;

        // 同じ画像ファイルで作られたテクスチャが既にあるなら、
        // 画像のサイズだけ確保しテクスチャの作成を行わない
        auto it = m_texMap.find(filepath);
        if (it != m_texMap.end())
        {
            D3DSURFACE_DESC desc { };
            if (FAILED(m_texMap.at(m_filepath)->GetLevelDesc(0, &desc)))
            {
                throw std::exception("Failed to create a texture.");
            }
            m_width = desc.Width;
            m_height = desc.Height;
            it->second->AddRef();
            return;
        }

        // テクスチャの作成
        LPDIRECT3DTEXTURE9 pD3DTexture = NULL;
        HRESULT hr = D3DXCreateTextureFromFile(m_pD3DDevice, filepath.c_str(), &pD3DTexture);
        if (FAILED(hr))
        {
            std::string work;
            work = "Failed to create a texture. HRESULT: " + std::to_string(hr);
            throw std::exception(work.c_str());
        }

        m_texMap[filepath] = pD3DTexture;


        D3DSURFACE_DESC desc { };
        if (FAILED(pD3DTexture->GetLevelDesc(0, &desc)))
        {
            throw std::exception("Failed to create a texture.");
        }
        m_width = desc.Width;
        m_height = desc.Height;
    }

    ~Sprite()
    {
        ULONG refCnt = m_texMap.at(m_filepath)->Release();
        if (refCnt == 0)
        {
            m_texMap.erase(m_filepath);
        }

        if (m_texMap.empty())
        {
            SAFE_RELEASE(m_D3DSprite);
        }
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;

    // スプライトは一つを使いまわす
    static LPD3DXSPRITE m_D3DSprite;

    std::wstring m_filepath;
    UINT m_width { 0 };
    UINT m_height { 0 };

    // 同じ名前の画像ファイルで作られたテクスチャは使いまわす
    static std::unordered_map<std::wstring, LPDIRECT3DTEXTURE9> m_texMap;
};

LPD3DXSPRITE Sprite::m_D3DSprite = NULL;
std::unordered_map<std::wstring, LPDIRECT3DTEXTURE9> Sprite::m_texMap;

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
                                20,
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
                                &m_pFont);
        }
    }

    virtual void DrawText_(const std::wstring& msg,
                           const int x,
                           const int y,
                           const int transparency)
    {
        RECT rect = { x, y, x + 200, y + 20 };
        m_pFont->DrawText(NULL, msg.c_str(),
                          -1,
                          &rect,
                          DT_VCENTER | DT_NOCLIP,
                          D3DCOLOR_ARGB(transparency, 255, 255, 255));
    }

    ~Font()
    {
        SAFE_RELEASE(m_pFont);
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXFONT m_pFont = NULL;
};


class SoundEffect : public ISoundEffect
{
    virtual void PlayMove() override
    {
        ::SoundEffect::get_ton()->play(_T("res\\sound\\menu_cursor_move.wav"));
    }
    virtual void PlayClick() override
    {
        ::SoundEffect::get_ton()->play(_T("res\\sound\\menu_cursor_confirm.wav"));
    }
    virtual void PlayBack() override
    {
        ::SoundEffect::get_ton()->play(_T("res\\sound\\menu_cursor_cancel.wav"));
    }
    virtual void Init() override
    {
        ::SoundEffect::get_ton()->load(_T("res\\sound\\menu_cursor_move.wav"));
        ::SoundEffect::get_ton()->load(_T("res\\sound\\menu_cursor_confirm.wav"));
        ::SoundEffect::get_ton()->load(_T("res\\sound\\menu_cursor_cancel.wav"));
    }
};
}


void PatchTestManager2::Finalize()
{
    m_guiLib.Finalize();
}

void PatchTestManager2::InitPatch()
{
    Camera::SetCameraMode(eCameraMode::SLEEP);
    Common::SetCursorVisibility(true);

    using namespace NSResearch;
    NSResearch::Sprite* sprCursor = NEW NSResearch::Sprite(SharedObj::GetD3DDevice());
    sprCursor->Load(_T("res\\image\\menu_cursor.png"));

    NSResearch::Sprite* sprBackground = NEW NSResearch::Sprite(SharedObj::GetD3DDevice());
    sprBackground->Load(_T("res\\image\\menu_back.png"));

    NSResearch::Sprite* sprVBar = NEW NSResearch::Sprite(SharedObj::GetD3DDevice());
    sprVBar->Load(_T("res\\image\\vbar.png"));

    NSResearch::IFont* pFont = NEW NSResearch::Font(SharedObj::GetD3DDevice());

    NSResearch::ISoundEffect* pSE = NEW NSResearch::SoundEffect();

    m_guiLib.Init(pFont, pSE, sprCursor, sprBackground, sprVBar, SharedObj::IsEnglish());

    CreateList();
}

std::wstring PatchTestManager2::Operate()
{
    static long long counter = 0;
    {
        ++counter;

        if (counter == 1)
        {
            Camera::SetCameraMode(eCameraMode::SLEEP);
        }

        // 一秒に一回くらいの処理
        if (counter % 60 == 0)
        {
            auto datetime = NSModel::PowereggDateTime::GetObj();
            m_guiLib.UpdateDateTime(datetime->GetYear(),
                                          datetime->GetMonth(),
                                          datetime->GetDay(),
                                          datetime->GetHour(),
                                          datetime->GetMinute(),
                                          datetime->GetSecond());

            GetPatchLib()->Update();

            // リストを作り直す
            CreateList();
        }
    }

    //-------------------------------------------------
    // Mouse, Keyboard, GamePad
    //-------------------------------------------------

    std::wstring result;
    std::wstring work_str;

    static int holdCounter = 0;

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_UP))
    {
        m_guiLib.Up();
    }

    if (SharedObj::KeyBoard()->IsHold(DIK_UP))
    {
        holdCounter++;
        if (holdCounter % 3 == 0)
        {
            m_guiLib.Up();
        }
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_DOWN))
    {
        m_guiLib.Down();
    }

    if (SharedObj::KeyBoard()->IsHold(DIK_DOWN))
    {
        holdCounter++;
        if (holdCounter % 3 == 0)
        {
            m_guiLib.Down();
        }
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_LEFT))
    {
        m_guiLib.Left();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RIGHT))
    {
        m_guiLib.Right();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RETURN))
    {
        result = m_guiLib.Into();

        QueueTest(result);
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_ESCAPE))
    {
        result = m_guiLib.Back();
        counter = 0;
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_BACK))
    {
        result = m_guiLib.Back();
        counter = 0;
    }

    if (Mouse::IsDownLeft())
    {
        POINT p = Common::GetScreenPos();;
        result = m_guiLib.Click(p.x, p.y);

        QueueTest(result);
    }
    else
    {
        static POINT previousPoint = { 0, 0 };
        POINT p = Common::GetScreenPos();

        if (p.x == previousPoint.x &&
            p.y == previousPoint.y)
        {
            // do nothing
        }
        else
        {
            m_guiLib.CursorOn(p.x, p.y);
        }
        
        previousPoint = p;
    }

    if (Mouse::GetZDelta() < 0)
    {
        m_guiLib.Next();
    }
    else if (Mouse::GetZDelta() > 0)
    {
        m_guiLib.Previous();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::UP))
    {
        m_guiLib.Up();
    }

    if (GamePad::IsHold(eGamePadButtonType::UP))
    {
        holdCounter++;
        if (holdCounter % 3 == 0)
        {
            m_guiLib.Up();
        }
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::DOWN))
    {
        m_guiLib.Down();
    }

    if (GamePad::IsHold(eGamePadButtonType::DOWN))
    {
        holdCounter++;
        if (holdCounter % 3 == 0)
        {
            m_guiLib.Down();
        }
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::LEFT))
    {
        m_guiLib.Left();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::RIGHT))
    {
        m_guiLib.Right();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::A))
    {
        result = m_guiLib.Into();
        QueueTest(result);
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::B))
    {
        result = m_guiLib.Back();
        counter = 0;
    }

    return result;
}

void PatchTestManager2::Draw()
{
    m_guiLib.Draw();
}

void PatchTestManager2::CreateList()
{
    m_guiLib.ClearAll();

    // 左側のリストを作る
    // 現在所持している物で、パッチテスト対象のものを表示する
    // 同じアイテムを複数所持していたら複数表示する。
    {
        auto keyList = GetPatchLib()->GetKeyList();

        for (size_t i = 0; i < keyList.size(); ++i)
        {
            auto itemDef = Common::ItemManager()->GetItemDef(keyList.at(i));
            auto subIdList = Common::Inventory()->GetSubIdList(itemDef.GetId());

            for (size_t j = 0; j < subIdList.size(); ++j)
            {
                NSResearch::TestItem testItem;

                testItem.SetName(itemDef.GetName());
                testItem.SetId(itemDef.GetId());
                testItem.SetSubId(subIdList.at(j));

                m_guiLib.AddTestItem(testItem);
            }
        }
    }

    // 右側のリストを作る
    {
        auto queList = GetPatchLib()->GetQueue();

        for (size_t i = 0; i < queList.size(); ++i)
        {
            NSResearch::QueuedTestItem itemInfo;

            itemInfo.SetName(queList.at(i).GetItemName());

            int y, M, d, h, m, s;
            queList.at(i).GetDateTimeReq(&y, &M, &d, &h, &m, &s);
            itemInfo.SetDateReq(y, M, d, h, m, s);

            queList.at(i).GetDateTimeStart(&y, &M, &d, &h, &m, &s);
            itemInfo.SetDateStart(y, M, d, h, m, s);

            if (queList.at(i).GetState() == NSModel::PatchTest::eState::FINISHED)
            {
                queList.at(i).GetDateTimeEnd(&y, &M, &d, &h, &m, &s);
                itemInfo.SetDateEnd(y, M, d, h, m, s);
            }

            if (queList.at(i).GetState() == NSModel::PatchTest::eState::NOT_START)
            {
                itemInfo.SetResult(Common::LoadString_(IDS_STRING164));
            }
            else if (queList.at(i).GetState() == NSModel::PatchTest::eState::STARTED)
            {
                itemInfo.SetResult(Common::LoadString_(IDS_STRING165));
            }
            else if (queList.at(i).GetState() == NSModel::PatchTest::eState::FINISHED)
            {
                if (queList.at(i).GetResult() == NSModel::PatchTest::eResult::POISON)
                {
                    itemInfo.SetResult(Common::LoadString_(IDS_STRING166));
                }
                else if (queList.at(i).GetResult() == NSModel::PatchTest::eResult::NOT_POISON)
                {
                    itemInfo.SetResult(Common::LoadString_(IDS_STRING167));
                }
            }

            m_guiLib.AddQueueItem(itemInfo);
        }
    }

    m_guiLib.UpdateCursorPos();
}

void PatchTestManager2::QueueTest(const std::wstring& result)
{
    if (result.empty())
    {
        return;
    }

    std::vector<std::wstring> vs = Common::split(result, ':');

    std::wstring id = vs.at(2);

    // パッチテストをキューイングする
    bool result2 = GetPatchLib()->QueuePatchTest(id);

    if (result2)
    {
        int subId = std::stoi(vs.at(3));

        Common::Inventory()->RemoveItem(id, subId);

        // リストを作り直す
        CreateList();
    }
    else
    {
        PopUp2::Get()->SetText(IDS_STRING168);
    }
}

NSModel::PatchTestManager* PatchTestManager2::GetPatchLib()
{
    return NSModel::PatchTestManager::Get();
}
