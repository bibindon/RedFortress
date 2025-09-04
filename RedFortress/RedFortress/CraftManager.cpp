#include "CraftManager.h"
#include "KeyBoard.h"
#include "Camera.h"
#include "Mouse.h"
#include "GamePad.h"
#include "SoundEffect.h"
#include "SharedObj.h"
#include "../../RedFortressModel/Model/CraftSystem.h"
#include "../../RedFortressModel/Model/CraftInfoManager.h"
#include "../../RedFortressModel/Model/ActivityBase.h"
#include "../../RedFortressModel/Model/WeaponManager.h"
#include "PopUp2.h"
#include <cassert>
#include "Common.h"
#include "resource.h"

namespace NSCraft
{
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

    UINT m_width = 0;
    UINT m_height = 0;

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

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXFONT m_pFont = NULL;
};


class SoundEffect : public ISoundEffect
{
public:
    SoundEffect()
    {
    }

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

void CraftManager::Init()
{
    NSCraft::Sprite* sprCursor = NEW NSCraft::Sprite(SharedObj::GetD3DDevice());
    sprCursor->Load(_T("res\\image\\menu_cursor.png"));

    NSCraft::Sprite* sprBackground = NEW NSCraft::Sprite(SharedObj::GetD3DDevice());
    sprBackground->Load(_T("res\\image\\background.png"));

    NSCraft::Sprite* sprPanelLeft = NEW NSCraft::Sprite(SharedObj::GetD3DDevice());
    sprPanelLeft->Load(_T("res\\image\\panelLeft.png"));

    NSCraft::Sprite* sprPanelTop = NEW NSCraft::Sprite(SharedObj::GetD3DDevice());
    sprPanelTop->Load(_T("res\\image\\craftPanel.png"));

    NSCraft::IFont* pFont = NEW NSCraft::Font(SharedObj::GetD3DDevice());

    NSCraft::ISoundEffect* pSE = NEW NSCraft::SoundEffect();

    m_gui.Init(pFont, pSE, sprCursor, sprBackground,
               sprPanelLeft, sprPanelTop, SharedObj::IsEnglish());

    Build();
}

void CraftManager::Finalize()
{
    m_gui.Finalize();
}

// この関数はクラフト画面が表示されていないときも呼ばれることに気を付ける
void CraftManager::Update()
{
    auto craftSys = NSModel::CraftSystem::GetObj();

    static int counter = 0;
    ++counter;

    // 1秒に一回
    if (counter % 60 == 1)
    {
        craftSys->UpdateCraftStatus();
    }
}

void CraftManager::Operate(eBattleState* state)
{
    auto craftSys = NSModel::CraftSystem::GetObj();

    // 定期的に画面を作り直す。
    static int counter = 0;
    ++counter;

    if (counter == 1)
    {
        Common::SetCursorVisibility(true);
        Camera::SetCameraMode(eCameraMode::SLEEP);
    }

    // 1秒に一回
    if (counter % 60 == 1)
    {
        if (*state == eBattleState::CRAFT)
        {
            // 画面更新
            Build();
        }
    }

    std::wstring result;

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_UP))
    {
        m_gui.Up();
    }

    if (SharedObj::KeyBoard()->IsHold(DIK_UP))
    {
        m_gui.Up();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_DOWN))
    {
        m_gui.Down();
    }

    if (SharedObj::KeyBoard()->IsHold(DIK_DOWN))
    {
        m_gui.Down();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_LEFT))
    {
        m_gui.Left();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RIGHT))
    {
        m_gui.Right();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RETURN))
    {
        result = m_gui.Into();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_ESCAPE))
    {
        result = m_gui.Back();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_BACK))
    {
        result = m_gui.Back();
    }

    if (Mouse::IsDownLeft())
    {
        POINT p = Common::GetScreenPos();
        result = m_gui.Click(p.x, p.y);
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
            m_gui.CursorOn(p.x, p.y);
        }

        previousPoint = p;
    }

    if (Mouse::GetZDelta() < 0)
    {
        m_gui.Next();
    }
    else if (Mouse::GetZDelta() > 0)
    {
        m_gui.Previous();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::UP))
    {
        m_gui.Up();
    }

    if (GamePad::IsHold(eGamePadButtonType::UP))
    {
        m_gui.Up();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::DOWN))
    {
        m_gui.Down();
    }

    if (GamePad::IsHold(eGamePadButtonType::DOWN))
    {
        m_gui.Down();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::LEFT))
    {
        m_gui.Left();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::RIGHT))
    {
        m_gui.Right();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::A))
    {
        result = m_gui.Into();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::B))
    {
        result = m_gui.Back();
    }

    if (!result.empty())
    {
        if (result == _T("EXIT"))
        {
            *state = eBattleState::NORMAL;
            Camera::SetCameraMode(eCameraMode::BATTLE);
            Common::SetCursorVisibility(false);
            counter = 0;
        }
        // イカダの場合は活動拠点の船着き場にイカダがないかチェック
        else if (result == L"raft")
        {
            bool raftExist = NSModel::ActivityBase::Get()->CheckRaftExist();

            if (raftExist)
            {
                PopUp2::Get()->SetText(IDS_STRING170);
                PopUp2::Get()->SetText(IDS_STRING171);
            }
            else
            {
                // クラフト開始
                std::wstring work = result;
                auto index = work.find(_T("+"));
                if (index != std::wstring::npos)
                {
                    work.erase(index);
                }

                std::wstring errMsg;
                bool started = craftSys->QueueCraftRequest(work, &errMsg);
                if (!started)
                {
                    if (!SharedObj::IsEnglish())
                    {
                        PopUp2::Get()->SetText(errMsg);
                    }
                    else
                    {
                        if (errMsg == _T("予約は５件までにしておこう"))
                        {
                            PopUp2::Get()->SetText(L"Let's limit the reservations to five.");
                        }
                        else if (errMsg == _T("素材が足りない"))
                        {
                            PopUp2::Get()->SetText(L"Lack of materials");
                        }
                    }
                }
                else
                {
                    Build();
                }
            }
        }
        else
        {
            // クラフト開始
            std::wstring work = result;
            auto index = work.find(_T("+"));
            int num = 0;
            if (index != std::wstring::npos)
            {
                auto sub = work.substr(index + 1);
                num = std::stoi(sub);
                work.erase(index);
            }

            std::wstring errMsg;

            auto itemId = Common::ItemManager()->GetItemDef(work).GetId();
            bool started = false;
            if (itemId == L"sotetsuDetox")
            {
                num += 5;
                started = craftSys->QueueCraftRequest(work, &errMsg, -1, num);
            }
            else if (itemId == L"donguriDetox")
            {
                num += 5;
                started = craftSys->QueueCraftRequest(work, &errMsg, -1, num);
            }
            else
            {
                started = craftSys->QueueCraftRequest(work, &errMsg);
            }

            if (!started)
            {
                if (!SharedObj::IsEnglish())
                {
                    PopUp2::Get()->SetText(errMsg);
                }
                else
                {
                    if (errMsg == _T("予約は５件までにしておこう"))
                    {
                        PopUp2::Get()->SetText(L"Let's limit the reservations to five.");
                    }
                    else if (errMsg == _T("素材が足りない"))
                    {
                        PopUp2::Get()->SetText(L"Lack of materials");
                    }
                }
            }
            else
            {
                Build();
            }
        }
    }
}

void CraftManager::Draw()
{
    m_gui.Draw();
}

void CraftManager::Build()
{
    auto craftSys = NSModel::CraftSystem::GetObj();
    auto craftInfo = NSModel::CraftInfoManager::GetObj();

    {
        std::vector<std::wstring> idList;
        std::vector<std::wstring> nameList;

        auto infoList = craftInfo->GetCraftItemList();

        // クラフト可能リストにはクラフト可能なアイテムを一覧で表示する。
        // イカダは強化値10まであり、イカダ、イカダ+1、イカダ+2・・・イカダ+10をクラフトできるが
        // リストに表示されるのは「イカダ」だけ
        // 絵の上手い人が下手な絵を描くことが困難なように、自動でクラフト可能な強化値が選ばれるため
        // 強化値を選択できる必要はない
        for (auto& info : infoList)
        {
            auto id = info.GetItemId();
            auto name = Common::ItemManager()->GetItemDef(id).GetName();
            auto level = info.GetLevel();
            if (level >= 1)
            {
                continue;
            }

            // 毒抜きの場合、成果物は同じだが個数が違うというクラフトがある。
            // 例えば、毒抜き用の袋（レベル1）で毒抜きすると多めに成果物を得られる。
            if (info.GetItemId() == L"sotetsuDetox" || info.GetItemId() == L"donguriDetox")
            {
                int bagLevel = 0;
                auto materials = craftInfo->GetCraftInfo(info).GetCraftMaterialDef();
                for (auto& material : materials)
                {
                    // 98 == 毒抜き用の袋
                    if (material.GetId() == L"bagForDetox")
                    {
                        bagLevel = 0;
                        break;
                    }
                    else if (material.GetId() == L"bagForDetox1")
                    {
                        bagLevel = 1;
                        break;
                    }
                    else if (material.GetId() == L"bagForDetox2")
                    {
                        bagLevel = 2;
                        break;
                    }
                    else if (material.GetId() == L"bagForDetox3")
                    {
                        bagLevel = 3;
                        break;
                    }
                    else if (material.GetId() == L"bagForDetox4")
                    {
                        bagLevel = 4;
                        break;
                    }
                    else if (material.GetId() == L"bagForDetox5")
                    {
                        bagLevel = 5;
                        break;
                    }
                }

                if (bagLevel != 0)
                {
                    id += L"+" + std::to_wstring(bagLevel);
                    name += L"+" + std::to_wstring(bagLevel);
                }
            }

            idList.push_back(id);
            nameList.push_back(name);
        }

        m_gui.SetOutputList(idList, nameList);

        auto reqList = craftSys->GetCraftRequestList();

        if (reqList.size() >= 1)
        {
            auto itemid = reqList.front().GetCraftInfo().GetOutput().GetItemId();
            auto outputName = Common::ItemManager()->GetItemDef(itemid).GetName();
            auto progress = craftSys->GetProgress();

            m_gui.SetCraftingItem(outputName, progress);
        }
        else
        {
            m_gui.SetCraftingItem(_T(""), 0);
        }

        nameList.clear();

        // 2番目以降を予約リストに表示
        if (reqList.size() >= 2)
        {
            std::wstring name;
            auto it = reqList.begin();
            ++it;

            for (; it != reqList.end(); ++it)
            {
                auto itemid = it->GetCraftInfo().GetOutput().GetItemId();
                name = Common::ItemManager()->GetItemDef(itemid).GetName();
                nameList.push_back(name);
            }

            m_gui.SetCraftQue(nameList);
        }
        else
        {
            m_gui.SetCraftQue(nameList);
        }

        // 画像、説明文を登録
        {
            std::wstring work;

            auto allCraftList = craftInfo->GetCraftItemList();

            // クラフトマンの熟練度に応じて成果物の強化値を変更
            auto craftSys = NSModel::CraftSystem::GetObj();

            for (auto& info : allCraftList)
            {
                auto id = info.GetItemId();
                auto unreinforcedId = Common::ItemManager()->GetItemDef(id).GetUnreinforcedId();
                auto skill = craftSys->GetCraftsmanSkill(unreinforcedId);

                if (skill != info.GetLevel())
                {
                    continue;
                }

                auto name = Common::ItemManager()->GetItemDef(id).GetName();
                work += Common::LoadString_(IDS_STRING172) + name + _T("\n");
                work += Common::LoadString_(IDS_STRING173) + std::to_wstring(info.GetNumber()) + _T("\n");
                if (info.GetLevel() != -1)
                {
                    work += Common::LoadString_(IDS_STRING174) + std::to_wstring(info.GetLevel()) + _T("\n");
                }
                else
                {
                    work += Common::LoadString_(IDS_STRING174) + _T("---\n");
                }
                work += _T("\n");

                auto materials = craftInfo->GetCraftInfo(info).GetCraftMaterialDef();

                int i = 1;
                for (auto& material : materials)
                {
                    work += Common::LoadStringWithArg(IDS_STRING175, std::to_wstring(i)) + material.GetName() + _T("\n");
                    work += Common::LoadStringWithArg(IDS_STRING176, std::to_wstring(i)) + std::to_wstring(material.GetNumber()) + _T("\n");
                    if (material.GetLevel() >= 1)
                    {
                        work += Common::LoadStringWithArg(IDS_STRING177, std::to_wstring(i)) + std::to_wstring(material.GetLevel()) + _T("\n");
                    }
                    work += _T("\n");

                    ++i;
                }

                // 毒抜きの場合、成果物は同じだが個数が違うというクラフトがある。
                // 例えば、毒抜き用の袋（レベル1）で毒抜きすると多めに成果物を得られる。
                if (info.GetItemId() == L"sotetsuDetox" || info.GetItemId() == L"donguriDetox")
                {
                    int bagLevel = 0;
                    for (auto& material : materials)
                    {
                        if (material.GetId() == L"bagForDetox")
                        {
                            bagLevel = 0;
                            break;
                        }
                        else if (material.GetId() == L"bagForDetox1")
                        {
                            bagLevel = 1;
                            break;
                        }
                        else if (material.GetId() == L"bagForDetox2")
                        {
                            bagLevel = 2;
                            break;
                        }
                        else if (material.GetId() == L"bagForDetox3")
                        {
                            bagLevel = 3;
                            break;
                        }
                        else if (material.GetId() == L"bagForDetox4")
                        {
                            bagLevel = 4;
                            break;
                        }
                        else if (material.GetId() == L"bagForDetox5")
                        {
                            bagLevel = 5;
                            break;
                        }
                    }
                    if (bagLevel != 0)
                    {
                        unreinforcedId += L"+" + std::to_wstring(bagLevel);
                    }
                }

                m_gui.SetOutputInfo(unreinforcedId, work);
                work.clear();

                NSCraft::ISprite* sprite1 = NEW NSCraft::Sprite(SharedObj::GetD3DDevice());
                auto itemDef = Common::ItemManager()->GetItemDef(info.GetItemId());
                auto imagePath = itemDef.GetImagePath();

                m_gui.SetOutputImage(unreinforcedId, imagePath, sprite1);
            }

        }
    }
}

// 解像度やウィンドウモードを変更したときのための関数
void CraftManager::OnDeviceLost()
{

}

// 解像度やウィンドウモードを変更したときのための関数
void CraftManager::OnDeviceReset()
{

}

