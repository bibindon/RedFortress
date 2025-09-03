#include "MenuManager.h"
#include "SoundEffect.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "SharedObj.h"
#include "KeyBoard.h"
#include "Mouse.h"
#include "SaveManager.h"
#include "../../RedFortressModel/Model/HumanInfoManager.h"
#include "../../RedFortressModel/Model/MapInfoManager.h"
#include "../../RedFortressModel/Model/ItemManager.h"
#include "../../RedFortressModel/Model/Inventory.h"
#include "../../RedFortressModel/Model/WeaponManager.h"
#include "../../RedFortressModel/Model/EnemyInfoManager.h"
#include "../../RedFortressModel/Model/SkillManager.h"
#include "../../RedFortressModel/Model/StatusManager.h"
#include "../../RedFortressModel/Model/Guide.h"
#include "GamePad.h"
#include "../../RedFortressModel/Model/Rynen.h"
#include "PopUp2.h"
#include <cassert>
#include "resource.h"
#include "../../RedFortressModel/Model/NpcStatusManager.h"
#include <cstdio>

namespace NSMenu
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
        if (m_D3DSprite == nullptr)
        {
            return;
        }

        D3DXVECTOR3 pos { (float)x, (float)y, 0.f };

        m_D3DSprite->Begin(D3DXSPRITE_ALPHABLEND);

        RECT rect { 0, 0, static_cast<LONG>(m_width), static_cast<LONG>(m_height) };

        D3DXVECTOR3 center { 0.f, 0.f, 0.f };

        m_D3DSprite->Draw(m_texMap.at(m_filepath),
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
    }

    static void Finalize()
    {
        m_texMap.clear();
        m_D3DSprite.Release();
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;

    // スプライトは一つを使いまわす
    static CComPtr<ID3DXSprite> m_D3DSprite;

    Wstr m_filepath;
    UINT m_width = 0;
    UINT m_height = 0;

    // 同じ名前の画像ファイルで作られたテクスチャは使いまわす
    static std::unordered_map<std::wstring, CComPtr<IDirect3DTexture9>> m_texMap;
};

CComPtr<ID3DXSprite> Sprite::m_D3DSprite = NULL;
std::unordered_map<std::wstring, CComPtr<IDirect3DTexture9>> Sprite::m_texMap;

class Font : public IFont
{
public:

    Font(LPDIRECT3DDEVICE9 pD3DDevice, const std::wstring fontName, const int fontSize)
        : m_pD3DDevice(pD3DDevice)
        , m_fontName(fontName)
        , m_fontSize(fontSize)
    {
    }

    void Init(const bool bEnglish)
    {
        HRESULT hr = S_OK;

        if (!bEnglish)
        {
            hr = D3DXCreateFont(m_pD3DDevice,
                                m_fontSize,
                                0,
                                FW_NORMAL,
                                1,
                                false,
                                SHIFTJIS_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                CLEARTYPE_NATURAL_QUALITY,
                                FF_DONTCARE,
                                m_fontName.c_str(),
                                &m_pFont);
        }
        else
        {
            hr = D3DXCreateFont(m_pD3DDevice,
                                m_fontSize,
                                0,
                                Common::FontThicknessEng(),
                                1,
                                false,
                                DEFAULT_CHARSET,
                                OUT_TT_ONLY_PRECIS,
                                CLEARTYPE_NATURAL_QUALITY,
                                FF_DONTCARE,
                                m_fontName.c_str(),
                                &m_pFont);
        }

        assert(hr == S_OK);
    }

    virtual void DrawText_(const std::wstring& msg,
                           const int x,
                           const int y,
                           const bool hcenter,
                           const int transparency)
    {
        if (hcenter)
        {
            RECT rect = { x, y, x + 100, y + 20 };
            m_pFont->DrawText(NULL, msg.c_str(),
                              -1,
                              &rect,
                              DT_VCENTER | DT_CENTER | DT_NOCLIP,
                              D3DCOLOR_ARGB(transparency, 255, 255, 255));
        }
        else
        {
            RECT rect = { x, y, x + 200, y + 20 };
            m_pFont->DrawText(NULL, msg.c_str(),
                              -1,
                              &rect,
                              DT_VCENTER | DT_NOCLIP,
                              D3DCOLOR_ARGB(transparency, 255, 255, 255));
        }
    }

    ~Font()
    {
        SAFE_RELEASE(m_pFont);
    }

private:

    LPDIRECT3DDEVICE9 m_pD3DDevice = NULL;
    LPD3DXFONT m_pFont = NULL;
    const std::wstring m_fontName;
    const int m_fontSize = 0;
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

MenuManager::MenuManager()
{
}

MenuManager::~MenuManager()
{
    m_menu.Finalize();
    NSMenu::Sprite::Finalize();
}

void MenuManager::InitMenu()
{
    using namespace NSMenu;
    NSMenu::ISprite* sprCursor = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
    sprCursor->Load(_T("res\\image\\menu_cursor.png"));

    NSMenu::ISprite* sprBackground = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
    sprBackground->Load(_T("res\\image\\menu_back.png"));

    NSMenu::IFont* pFont = nullptr;
    NSMenu::IFont* pFontStatus = nullptr;

    if (SharedObj::IsEnglish())
    {
        pFont = NEW NSMenu::Font(SharedObj::GetD3DDevice(), Common::FontNameEng(), Common::FontSizeEng());
        pFontStatus = NEW NSMenu::Font(SharedObj::GetD3DDevice(), L"Courier New", Common::FontSizeEng() - 4);
    }
    else
    {
        pFont = NEW NSMenu::Font(SharedObj::GetD3DDevice(), L"BIZ UDMincho", 20);
        pFontStatus = NEW NSMenu::Font(SharedObj::GetD3DDevice(), L"BIZ UDMincho", 16);
    }

    NSMenu::ISoundEffect* pSE = NEW NSMenu::SoundEffect();

    m_menu.Init(_T(""), pFont, pFontStatus, pSE, sprCursor, sprBackground, SharedObj::IsEnglish());

    NSModel::ItemManager* itemManager = NSModel::ItemManager::GetObj();
    NSModel::WeaponManager* weaponManager = NSModel::WeaponManager::GetObj();
    //------------------------------------------------------
    // アイテム情報（＝インベントリ）
    //------------------------------------------------------
    {
        NSModel::Inventory* inventory = NSModel::Inventory::GetObj();

        std::vector<std::wstring> idList = itemManager->GetItemIdList();

        std::vector<ItemInfo> itemInfoList;
        for (std::size_t i = 0; i < idList.size(); ++i)
        {
            NSModel::ItemDef itemDef = itemManager->GetItemDef(idList.at(i));
            std::vector<int> subIdList = inventory->GetSubIdList(idList.at(i));
            {
                for (std::size_t j = 0; j < subIdList.size(); ++j)
                {
                    std::wstring work_str;

                    NSMenu::ItemInfo itemInfoG;

                    itemInfoG.SetName(itemDef.GetName());
                    itemInfoG.SetLevel(itemDef.GetLevel());

                    NSModel::ItemInfo itemInfo = inventory->GetItemInfo(idList.at(i), subIdList.at(j));

                    itemInfoG.SetDurability(itemInfo.GetDurabilityCurrent());

                    NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());

                    // 画像ファイル名を取得して設定
                    // アイテム種別が武器の時は武器クラスから取得する必要がある
                    if (itemDef.GetType() != NSModel::ItemDef::ItemType::WEAPON)
                    {
                        work_str = itemDef.GetImagePath();
                    }
                    else
                    {
                        work_str = weaponManager->GetImageName2(itemDef.GetWeaponId());
                    }

                    sprItem->Load(work_str);
                    itemInfoG.SetSprite(sprItem);

                    if (itemDef.GetType() != NSModel::ItemDef::ItemType::WEAPON)
                    {
                        work_str = itemDef.GetDetail();
                    }
                    else
                    {
                        work_str = weaponManager->GetDetail2(itemDef.GetWeaponId());
                    }
                    work_str = itemDef.GetDetail();
                    work_str.erase(std::remove(work_str.begin(), work_str.end(), '"'), work_str.end());

                    itemInfoG.SetId(itemInfo.GetId());
                    itemInfoG.SetSubId(itemInfo.GetSubId());
                    itemInfoG.SetDetail(work_str);

                    // 袋
                    if (itemInfo.GetItemDef().GetUnreinforcedId() == L"bag")
                    {
                        itemInfoG.SetEquipEnable(true);

                        if (IsBagEquiped(itemInfo.GetId(), itemInfo.GetSubId()))
                        {
                            itemInfoG.SetEquip(true);
                        }
                    }
                    // 菅笠
                    else if (itemInfo.GetItemDef().GetId() == L"sugegasa")
                    {
                        itemInfoG.SetEquipEnable(true);
                        if (Common::Status()->GetSugegasa())
                        {
                            itemInfoG.SetEquip(true);
                        }
                    }
                    else if (itemDef.GetType() == NSModel::ItemDef::ItemType::WEAPON)
                    {
                        itemInfoG.SetEquipEnable(true);

                        auto weapon = Common::Status()->GetEquipWeapon();
                        if (weapon.GetId() == itemInfo.GetId() &&
                            weapon.GetSubId() == itemInfo.GetSubId())
                        {
                            itemInfoG.SetEquip(true);
                        }
                    }

                    itemInfoG.SetWeight(itemInfo.GetItemDef().GetWeight());
                    itemInfoG.SetVolume((int)itemInfo.GetItemDef().GetVolume());

                    itemInfoList.push_back(itemInfoG);
                }
            }
        }
        m_menu.SetItem(itemInfoList);

        m_menu.SetWeightAll(Common::Inventory()->GetWeight());
        m_menu.SetVolumeAll((int)Common::Inventory()->GetVolume());
        m_menu.SetVolumeMax((int)Common::Inventory()->GetVolumeMax());
    }

    //------------------------------------------------------
    // 人物情報
    //------------------------------------------------------
    std::vector<HumanInfo> humanInfoList;
    {
        NSModel::HumanInfoManager* humanInfoManager = NSModel::HumanInfoManager::GetObj();
        std::vector<std::wstring> humanIdList = humanInfoManager->GetHumanIdList();
        for (std::size_t i = 0; i < humanIdList.size(); ++i)
        {
            HumanInfo humanInfo;

            NSModel::HumanInfo libHumanInfo = humanInfoManager->GetHumanInfo(humanIdList.at(i));

            if (libHumanInfo.GetVisible() == false)
            {
                continue;
            }

            humanInfo.SetName(libHumanInfo.GetName());

            NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
            sprItem->Load(libHumanInfo.GetImagePath());
            humanInfo.SetSprite(sprItem);

            humanInfo.SetDetail(libHumanInfo.GetDetail());

            humanInfoList.push_back(humanInfo);
        }
    }
    m_menu.SetHuman(humanInfoList);

    //------------------------------------------------------
    // 武器情報
    //------------------------------------------------------
    {
        NSModel::Inventory* inventory = NSModel::Inventory::GetObj();

        auto idList = itemManager->GetItemIdList();

        std::vector<WeaponInfo> weaponInfoList;
        for (std::size_t i = 0; i < idList.size(); ++i)
        {
            NSModel::ItemDef itemDef = itemManager->GetItemDef(idList.at(i));
            NSModel::ItemDef::ItemType itemType = itemDef.GetType();

            // 武器じゃなかったら無視
            if (itemType != NSModel::ItemDef::ItemType::WEAPON)
            {
                continue;
            }

            std::vector<int> subIdList = inventory->GetSubIdList(idList.at(i));
            {
                for (std::size_t j = 0; j < subIdList.size(); ++j)
                {
                    std::wstring work_str;

                    NSMenu::WeaponInfo weaponInfoG;

                    weaponInfoG.SetId(idList.at(i));
                    weaponInfoG.SetSubId(subIdList.at(j));

                    weaponInfoG.SetName(itemDef.GetName());
                    weaponInfoG.SetLevel(itemDef.GetLevel());

                    NSModel::ItemInfo itemInfo = inventory->GetItemInfo(idList.at(i), subIdList.at(j));

                    weaponInfoG.SetDurability(itemInfo.GetDurabilityCurrent());

                    NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());

                    work_str = weaponManager->GetImageName2(itemDef.GetWeaponId());

                    sprItem->Load(work_str);
                    weaponInfoG.SetSprite(sprItem);

                    work_str = weaponManager->GetDetail2(itemDef.GetWeaponId());
                    work_str.erase(std::remove(work_str.begin(), work_str.end(), '"'), work_str.end());

                    weaponInfoG.SetDetail(work_str);
                    weaponInfoList.push_back(weaponInfoG);
                }
            }
        }
        m_menu.SetWeapon(weaponInfoList);
    }

    //------------------------------------------------------
    // 操作説明
    //------------------------------------------------------
    {
        std::vector<GuideInfo> infoList;
        NSModel::Guide* guide = NSModel::Guide::GetObj();
        std::vector<std::wstring> vs = guide->GetCategoryList();
        for (std::size_t i = 0; i < vs.size(); ++i)
        {
            std::vector<std::wstring> vsSub = guide->GetSubCategoryList(vs.at(i));
            for (std::size_t j = 0; j < vsSub.size(); ++j)
            {
                if (guide->GetVisible(vs.at(i), vsSub.at(j)) == false)
                {
                    continue;
                }
                GuideInfo info;
                info.SetCategory(vs.at(i));
                info.SetSubCategory(vsSub.at(j));
                std::wstring detail = guide->GetText(vs.at(i), vsSub.at(j));
                info.SetDetail(detail);
                infoList.push_back(info);
            }
        }
        m_menu.SetGuide(infoList);
    }

    //------------------------------------------------------
    // 敵情報
    //------------------------------------------------------
    {
        std::vector<EnemyInfo> infoList;
        NSModel::EnemyInfoManager* enemyInfoManager = NSModel::EnemyInfoManager::GetObj();
        auto idList = enemyInfoManager->GetEnemyIdList();
        for (size_t i = 0; i < idList.size(); ++i)
        {
            auto enemyDef = enemyInfoManager->GetEnemyDef(idList.at(i));
            if (enemyDef.GetVisible() == false)
            {
                continue;
            }

            NSMenu::EnemyInfo enemyInfo;
            enemyInfo.SetName(enemyDef.GetName());
            enemyInfo.SetDetail(enemyDef.GetDetail());

            NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
            sprItem->Load(enemyDef.GetImagePath());
            enemyInfo.SetSprite(sprItem);
            infoList.push_back(enemyInfo);
        }
        m_menu.SetEnemy(infoList);
    }

    //------------------------------------------------------
    // 技・魔法情報
    //------------------------------------------------------
    {
        std::vector<SkillInfo> infoList;

        NSModel::SkillManager* skillManager = NSModel::SkillManager::GetObj();
        std::vector<std::wstring> nameList = skillManager->GetNameList();
        for (std::size_t i = 0; i < nameList.size(); ++i)
        {
            SkillInfo info;
            info.SetName(nameList.at(i));

            std::wstring work_str;
            work_str = skillManager->GetDetail(nameList.at(i));
            info.SetDetail(work_str);

            NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());

            if (i == 0)
            {
                sprItem->Load(_T("res\\image\\magicFire.png"));
            }
            else if (i == 1)
            {
                sprItem->Load(_T("res\\image\\magicIce.png"));
            }
            else if (i == 2)
            {
                sprItem->Load(_T("res\\image\\magicDark.png"));
            }

            info.SetSprite(sprItem);
            infoList.push_back(info);
        }
        m_menu.SetSkill(infoList);
    }

    //------------------------------------------------------
    // ステータス
    //------------------------------------------------------
    {
        // 毎秒更新する。ここではやらない。
    }

    //------------------------------------------------------
    // 地図情報
    //------------------------------------------------------
    {
        std::vector<MapInfo> mapInfoList;
        {
            NSModel::MapInfoManager* mapInfoManager = NSModel::MapInfoManager::GetObj();

            std::vector<std::wstring> mapIdList = mapInfoManager->GetIdList();
            for (std::size_t i = 0; i < mapIdList.size(); ++i)
            {
                std::wstring mapId = mapIdList.at(i);

                bool visible = mapInfoManager->IsDiscovered(mapId);

                if (visible == false)
                {
                    continue;
                }

                MapInfo mapInfo;
                mapInfo.SetName(mapInfoManager->GetName(mapId));
                mapInfo.SetDetail(mapInfoManager->GetDetail(mapId));

                NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
                sprItem->Load(mapInfoManager->GetImagePath(mapId));
                mapInfo.SetSprite(sprItem);

                mapInfoList.push_back(mapInfo);
            }
        }
        m_menu.SetMap(mapInfoList);
    }
}

void MenuManager::Finalize()
{
    m_menu.Finalize();
}

void MenuManager::Draw()
{
    m_menu.Draw();
}

static int GetDisplayWidth(const std::wstring& str)
{
    int width = 0;
    for (wchar_t ch : str)
    {
        if ((ch >= 0x20 && ch <= 0x7E) || (ch >= 0xFF61 && ch <= 0xFF9F)) {
            width += 1; // 半角
        }
        else {
            width += 2; // 全角
        }
    }
    return width;
}

static std::wstring PadRightDisplayWidth(const std::wstring& str, int targetWidth)
{
    int currentWidth = GetDisplayWidth(str);
    int padCount = targetWidth - currentWidth;
    if (padCount > 0)
    {
        return str + std::wstring(padCount, L' ');
    }
    return str;
}

std::wstring MenuManager::OperateMenu()
{
    std::wstring result;
    std::wstring work_str;

    // 長押ししたときのカーソルの移動が速すぎるので3回に1回の移動に減速させる
    static int holdCounter = 0;

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_UP))
    {
        m_menu.Up();
    }

    if (SharedObj::KeyBoard()->IsHold(DIK_UP))
    {
        holdCounter++;
        if (holdCounter % 3 == 0)
        {
            m_menu.Up();
        }
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_DOWN))
    {
        m_menu.Down();
    }

    if (SharedObj::KeyBoard()->IsHold(DIK_DOWN))
    {
        holdCounter++;
        if (holdCounter % 3 == 0)
        {
            m_menu.Down();
        }
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_LEFT))
    {
        m_menu.Left();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RIGHT))
    {
        m_menu.Right();
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_RETURN))
    {
        result = m_menu.Into();

        std::vector<std::wstring> vs = Common::split(result, ':');
        if (vs.size() == 5 && vs.at(0) == _T("Item"))
        {
            // アイテムを使う
            if (vs.at(4) == _T("Use"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));

                if (UseItem(id, subId))
                {
                    DeleteItem(id, subId);
                }
            }
            // アイテムを捨てる
            else if (vs.at(4) == _T("Discard"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));
                DeleteItem(id, subId);
            }
            else if (vs.at(4) == _T("Equip"))
            {
                // 火のついた松明を装備していたら武器を装備できない。袋も装備できない。
                bool lit = NSModel::WeaponManager::GetObj()->IsTorchLit();

                if (lit)
                {
                    PopUp2::Get()->SetText(IDS_STRING197);
                }
                else
                {
                    auto& id = vs.at(2);
                    int subId = std::stoi(vs.at(3));
                    Equip(id, subId);
                }
            }
            else if (vs.at(4) == _T("Unequip"))
            {
                // 点灯中の松明の装備を外すことはできない。
                bool lit = NSModel::WeaponManager::GetObj()->IsTorchLit();
                if (lit)
                {
                    PopUp2::Get()->SetText(IDS_STRING198);
                }
                else
                {
                    auto& id = vs.at(2);
                    int subId = std::stoi(vs.at(3));
                    Unequip(id, subId);
                }
            }
        }
        else if (vs.size() == 5 && vs.at(0) == _T("Weapon"))
        {
            if (vs.at(4) == _T("Equip"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));

                Equip(id, subId);
            }
        }
        else if (vs.size() >= 1 && vs.at(0) == _T("Save"))
        {
            auto saveManager = SaveManager::Get();
            saveManager->Save();
            PopUp2::Get()->SetText(IDS_STRING_SAVE_COMPLETE);
        }
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_ESCAPE))
    {
        // EXIT以外なら無視する
        result = m_menu.Back();
        if (result != _T("EXIT"))
        {
            result = _T("");
        }
    }

    if (SharedObj::KeyBoard()->IsDownFirstFrame(DIK_BACK))
    {
        // EXIT以外なら無視する
        result = m_menu.Back();
        if (result != _T("EXIT"))
        {
            result = _T("");
        }
    }

    if (Mouse::IsDownLeft())
    {
        POINT p = Common::GetScreenPos();;
        result = m_menu.Click(p.x, p.y);

        std::vector<std::wstring> vs = Common::split(result, ':');
        if (vs.size() == 5 && vs.at(0) == _T("Item"))
        {
            // アイテムを使う
            if (vs.at(4) == _T("Use"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));

                if (UseItem(id, subId))
                {
                    DeleteItem(id, subId);
                }
            }
            // アイテムを捨てる
            else if (vs.at(4) == _T("Discard"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));
                DeleteItem(id, subId);
            }
            else if (vs.at(4) == _T("Equip"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));
                Equip(id, subId);
            }
            else if (vs.at(4) == _T("Unequip"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));
                Unequip(id, subId);
            }
        }
        else if (vs.size() == 5 && vs.at(0) == _T("Weapon"))
        {
            if (vs.at(4) == _T("Equip"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));

                NSModel::Inventory* inventory = NSModel::Inventory::GetObj();
                NSModel::ItemInfo itemInfo = inventory->GetItemInfo(id, subId);

                NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
                statusManager->SetEquipWeapon(itemInfo);
            }
        }
        else if (vs.size() >= 1 && vs.at(0) == _T("Save"))
        {
            auto saveManager = SaveManager::Get();
            saveManager->Save();
            PopUp2::Get()->SetText(IDS_STRING_SAVE_COMPLETE);
        }
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
            m_menu.CursorOn(p.x, p.y);
        }
        
        previousPoint = p;
    }

    if (Mouse::IsDownRight())
    {
        m_menu.RightClick();
    }

    if (Mouse::GetZDelta() < 0)
    {
        m_menu.Next();
    }
    else if (Mouse::GetZDelta() > 0)
    {
        m_menu.Previous();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::UP))
    {
        m_menu.Up();
    }

    if (GamePad::IsHold(eGamePadButtonType::UP))
    {
        holdCounter++;
        if (holdCounter % 3 == 0)
        {
            m_menu.Up();
        }
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::DOWN))
    {
        m_menu.Down();
    }

    if (GamePad::IsHold(eGamePadButtonType::DOWN))
    {
        holdCounter++;
        if (holdCounter % 3 == 0)
        {
            m_menu.Down();
        }
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::LEFT))
    {
        m_menu.Left();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::RIGHT))
    {
        m_menu.Right();
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::A))
    {
        result = m_menu.Into();

        std::vector<std::wstring> vs = Common::split(result, ':');
        if (vs.size() == 5 && vs.at(0) == _T("Item"))
        {
            // アイテムを使う
            if (vs.at(4) == _T("Use"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));

                if (UseItem(id, subId))
                {
                    DeleteItem(id, subId);
                }
            }
            // アイテムを捨てる
            else if (vs.at(4) == _T("Discard"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));
                DeleteItem(id, subId);
            }
            else if (vs.at(4) == _T("Equip"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));
                Equip(id, subId);
            }
            else if (vs.at(4) == _T("Unequip"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));
                Unequip(id, subId);
            }
        }
        else if (vs.size() == 5 && vs.at(0) == _T("Weapon"))
        {
            if (vs.at(4) == _T("Equip"))
            {
                auto& id = vs.at(2);
                int subId = std::stoi(vs.at(3));

                NSModel::Inventory* inventory = NSModel::Inventory::GetObj();
                NSModel::ItemInfo itemInfo = inventory->GetItemInfo(id, subId);

                NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
                statusManager->SetEquipWeapon(itemInfo);
            }
        }
        else if (vs.size() >= 1 && vs.at(0) == _T("Save"))
        {
            auto saveManager = SaveManager::Get();
            saveManager->Save();
            PopUp2::Get()->SetText(IDS_STRING_SAVE_COMPLETE);
        }
    }

    if (GamePad::IsDownFirst(eGamePadButtonType::B))
    {
        // EXIT以外なら無視する
        result = m_menu.Back();
        if (result != _T("EXIT"))
        {
            result = _T("");
        }
    }

    // ステータス画面の表示内容を更新
    {
        std::vector<NSMenu::StatusInfo> infoList;

        // ホシマン
        {

            NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();

            NSModel::ItemInfo itemInfo = statusManager->GetEquipWeapon();
            std::wstring weaponName;

            if (itemInfo.GetId().size() >= 1)
            {
                NSModel::ItemManager* itemManager = NSModel::ItemManager::GetObj();
                NSModel::ItemDef itemDef = itemManager->GetItemDef(itemInfo.GetId());
                weaponName = itemDef.GetName();
            }

            // 状態異常リスト
            std::wstring condition;
            {
                if (statusManager->GetFractureArm())
                {
                    condition += Common::LoadString_(IDS_STRING155)+ _T("/");
                }

                if (statusManager->GetFractureLeg())
                {
                    condition += Common::LoadString_(IDS_STRING156)+ _T("/");
                }

                if (statusManager->GetHeadache())
                {
                    condition += Common::LoadString_(IDS_STRING157)+ _T("/");
                }

                if (statusManager->GetCold())
                {
                    condition += Common::LoadString_(IDS_STRING158)+ _T("/");
                }

                if (statusManager->GetStomachache())
                {
                    condition += Common::LoadString_(IDS_STRING159)+ _T("/");
                }

                if (statusManager->GetSleep())
                {
                    condition += Common::LoadString_(IDS_STRING160)+ _T("/");
                }

                if (statusManager->GetDehydration())
                {
                    condition += Common::LoadString_(IDS_STRING161)+ _T("/");
                }

                if (statusManager->GetLackOfSleep())
                {
                    condition += Common::LoadString_(IDS_STRING162)+ _T("/");
                }

                if (statusManager->GetDead())
                {
                    condition += Common::LoadString_(IDS_STRING163)+ _T("/");
                }

                if (condition.empty() == false)
                {
                    condition.pop_back();
                }
            }

            NSMenu::StatusInfo info;
            info.SetName(Common::LoadString_(IDS_STRING211));

            NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
            sprItem->Load(_T("res\\image\\hoshiman00.png"));
            info.SetSprite(sprItem);

            wchar_t buf[256];
            std::wstring work;
            work =  Common::LoadString_(IDS_STRING210) + _T("\n");

            std::wstring label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING146), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetBodyStaminaCurrent(), 2) + _T("/");
            work += Common::ToStringWithPrecision(statusManager->GetBodyStaminaMaxSub(), 2) + _T("/");
            work += Common::ToStringWithPrecision(statusManager->GetBodyStaminaMax(), 2) + _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING147), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetBrainStaminaCurrent(), 2) + _T("/");
            work += Common::ToStringWithPrecision(statusManager->GetBrainStaminaMaxSub(), 2) + _T("/");
            work += Common::ToStringWithPrecision(statusManager->GetBrainStaminaMax(), 2) + _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING209), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetExplosivePower(), 2) + _T("/-/-\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING149), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetMuscleCurrent(), 2) + _T("/-/");
            work += Common::ToStringWithPrecision(statusManager->GetMuscleMax(), 2) + _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING150), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetCarboCurrent(), 2) + _T("/-/");
            work += Common::ToStringWithPrecision(statusManager->GetCarboMax(), 2) + _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING151), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetProteinCurrent(), 2) + _T("/-/");
            work += Common::ToStringWithPrecision(statusManager->GetProteinMax(), 2) + _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING152), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetLipidCurrent(), 2) + _T("/-/");
            work += Common::ToStringWithPrecision(statusManager->GetLipidMax(), 2) + _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING153), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetVitaminCurrent(), 2) + _T("/-/");
            work += Common::ToStringWithPrecision(statusManager->GetVitaminMax(), 2) + _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING154), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetMineralCurrent(), 2) + _T("/-/");
            work += Common::ToStringWithPrecision(statusManager->GetMineralMax(), 2) + _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING148), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetWaterCurrent(), 2) + _T("/-/");
            work += Common::ToStringWithPrecision(statusManager->GetWaterMax(), 2) + _T("\n");
    //        work += _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING213), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += condition;
            work += _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING214), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += weaponName;
            work += _T("\n");

            label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING215), 20);
            _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
            work += buf;
            work += Common::ToStringWithPrecision(statusManager->GetAttackPower(), 2) + _T("\n");

            // 見づらい。ページを変えるとか右のほうに表示するとかしないとまずい。
            if (statusManager->GetFractureArm())
            {
                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING_CURE_TIME_ARM), 30);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
                work += buf;
                work += Common::ToStringWithPrecision((float)statusManager->GetFractureArmCureRemain() / 3600 / 24 * 12, 2) + _T("\n");
            }

            if (statusManager->GetFractureLeg())
            {
                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING_CURE_TIME_LEG), 30);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
                work += buf;
                work += Common::ToStringWithPrecision((float)statusManager->GetFractureLegCureRemain() / 3600 / 24 * 12, 2) + _T("\n");
            }

            if (statusManager->GetHeadache())
            {
                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING_CURE_TIME_HEAD), 30);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
                work += buf;
                work += Common::ToStringWithPrecision((float)statusManager->GetHeadacheCureRemain() / 3600 / 24 * 12, 2) + _T("\n");
            }

            if (statusManager->GetCold())
            {
                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING_CURE_TIME_COLD), 30);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
                work += buf;
                work += Common::ToStringWithPrecision((float)statusManager->GetColdCureRemain() / 3600 / 24 * 12, 2) + _T("\n");
            }

            if (statusManager->GetStomachache())
            {
                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING_CURE_TIME_STOMACH), 30);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
                work += buf;
                work += Common::ToStringWithPrecision((float)statusManager->GetStomachacheCureRemain() / 3600 / 24 * 12, 2) + _T("\n");
            }

            if (statusManager->GetStomachache())
            {
                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING_CURE_TIME_WATER), 30);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());
                work += buf;
                work += Common::ToStringWithPrecision((float)statusManager->GetDehydrationCureRemain() / 3600 / 24 * 12, 2) + _T("\n");
            }

            auto rynen = NSModel::Rynen::GetObj();
            if (rynen->GetContracted())
            {
                work += PadRightDisplayWidth(Common::LoadString_(IDS_STRING_WORDBRESS), 20);

                if (rynen->GetReviveEnable())
                {
                    work += Common::LoadString_(IDS_STRING_REVIVE_OK) + L"\n";
                }
                else
                {
                    work += Common::LoadString_(IDS_STRING_REVIVED) + L"\n";
                }
            }

            info.SetDetail(work);
            infoList.push_back(info);
        }

        //----------------------------------------------------
        // イカダのステータス
        //----------------------------------------------------
        {
            if (SharedObj::Voyage()->GetRaftMode())
            {
                // イカダは複数個持つことができるが、ステータスに表示するのは乗船中のイカダだけ
                // ・耐久度
                // ・強化値
                NSMenu::StatusInfo info;
                info.SetName(Common::LoadString_(IDS_STRING169));
                NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
                sprItem->Load(_T("res\\image\\raft.png"));
                info.SetSprite(sprItem);
                std::wstring work;
                work += Common::LoadString_(IDS_STRING_CURRENT_DURA) + L"\n";
                work += std::to_wstring(SharedObj::Voyage()->GetRaftDurability()) + _T("\n");
                work += Common::LoadString_(IDS_STRING_CURRENT_LEVEL) + L"\n";
                work += std::to_wstring(SharedObj::Voyage()->GetRaftLevel()) + _T("\n");

                info.SetDetail(work);
                infoList.push_back(info);
            }
        }

        // ダイケイマン
        {
            auto npcStatusMgr = NSModel::NpcStatusManager::GetObj();

            auto status = npcStatusMgr->GetNpcStatus(_T("daikeiman"));
            auto humanInfo = NSModel::HumanInfoManager::GetObj()->GetHumanInfo(L"daikeiman");
            if (humanInfo.GetVisible())
            {
                NSMenu::StatusInfo info;
                info.SetName(Common::LoadString_(IDS_STRING121));

                NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
                sprItem->Load(_T("res\\image\\daikeiman00.png"));
                info.SetSprite(sprItem);

                wchar_t buf[256];
                std::wstring work;
                work =  Common::LoadString_(IDS_STRING210) + _T("\n");

                std::wstring label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING150), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetCarbo(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING151), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetProtein(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING152), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetLipid(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING153), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetVitamin(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING154), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetMineral(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING148), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetWater(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");
                work += _T("\n");

                if (status.GetRynenContract())
                {
                    work += PadRightDisplayWidth(Common::LoadString_(IDS_STRING_WORDBRESS), 20);

                    if (status.GetDrinkWordbress())
                    {
                        work += Common::LoadString_(IDS_STRING_WORDBRESS_USED) + L"\n";
                    }
                    else
                    {
                        work += Common::LoadString_(IDS_STRING_WORDBRESS_UNUSED) + L"\n";
                    }
                }

                if (status.GetDead())
                {
                    work += Common::LoadString_(IDS_STRING_NPC_DEAD) + L"\n";
                }
                else
                {
                    work += Common::LoadString_(IDS_STRING_NPC_ALIVE) + L"\n";
                }

                info.SetDetail(work);
                infoList.push_back(info);
            }
        }

        // サンカクマン
        {
            auto npcStatusMgr = NSModel::NpcStatusManager::GetObj();

            auto status = npcStatusMgr->GetNpcStatus(_T("sankakuman"));
            auto humanInfo = NSModel::HumanInfoManager::GetObj()->GetHumanInfo(L"sankakuman");
            if (humanInfo.GetVisible())
            {
                NSMenu::StatusInfo info;
                info.SetName(Common::LoadString_(IDS_STRING112));

                NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
                sprItem->Load(_T("res\\image\\sankakuman00.png"));
                info.SetSprite(sprItem);

                wchar_t buf[256];
                std::wstring work;
                work =  Common::LoadString_(IDS_STRING210) + _T("\n");

                std::wstring label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING150), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetCarbo(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING151), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetProtein(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING152), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetLipid(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING153), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetVitamin(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING154), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetMineral(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING148), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetWater(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");
                work += _T("\n");

                if (status.GetRynenContract())
                {
                    work += PadRightDisplayWidth(Common::LoadString_(IDS_STRING_WORDBRESS), 20);

                    if (status.GetDrinkWordbress())
                    {
                        work += Common::LoadString_(IDS_STRING_WORDBRESS_USED) + L"\n";
                    }
                    else
                    {
                        work += Common::LoadString_(IDS_STRING_WORDBRESS_UNUSED) + L"\n";
                    }
                }

                if (status.GetDead())
                {
                    work += Common::LoadString_(IDS_STRING_NPC_DEAD) + L"\n";
                }
                else
                {
                    work += Common::LoadString_(IDS_STRING_NPC_ALIVE) + L"\n";
                }


                info.SetDetail(work);
                infoList.push_back(info);
            }
        }

        // シカクマン
        {
            auto npcStatusMgr = NSModel::NpcStatusManager::GetObj();

            auto status = npcStatusMgr->GetNpcStatus(_T("shikakuman"));
            auto humanInfo = NSModel::HumanInfoManager::GetObj()->GetHumanInfo(L"shikakuman");
            if (humanInfo.GetVisible())
            {
                NSMenu::StatusInfo info;
                info.SetName(Common::LoadString_(IDS_STRING113));

                NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());
                sprItem->Load(_T("res\\image\\shikakuman00.png"));
                info.SetSprite(sprItem);

                wchar_t buf[256];
                std::wstring work;
                work =  Common::LoadString_(IDS_STRING210) + _T("\n");

                std::wstring label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING150), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetCarbo(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING151), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetProtein(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING152), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetLipid(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING153), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetVitamin(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING154), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetMineral(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");

                label = PadRightDisplayWidth(Common::LoadString_(IDS_STRING148), 20);
                _snwprintf_s(buf, sizeof(buf) / sizeof(buf[0]), _TRUNCATE, _T("%s"), label.c_str());

                work += buf;
                work += Common::ToStringWithPrecision(status.GetWater(), 2) + _T("/-/");
                work += Common::ToStringWithPrecision(100, 2) + _T("\n");
                work += _T("\n");

                if (status.GetRynenContract())
                {
                    work += PadRightDisplayWidth(Common::LoadString_(IDS_STRING_WORDBRESS), 20);

                    if (status.GetDrinkWordbress())
                    {
                        work += Common::LoadString_(IDS_STRING_WORDBRESS_USED) + L"\n";
                    }
                    else
                    {
                        work += Common::LoadString_(IDS_STRING_WORDBRESS_UNUSED) + L"\n";
                    }
                }

                if (status.GetDead())
                {
                    work += Common::LoadString_(IDS_STRING_NPC_DEAD) + L"\n";
                }
                else
                {
                    work += Common::LoadString_(IDS_STRING_NPC_ALIVE) + L"\n";
                }

                info.SetDetail(work);
                infoList.push_back(info);
            }
        }

        m_menu.SetStatus(infoList);
    }
    return result;
}

bool MenuManager::UseItem(const std::wstring& id, const int subId)
{
    bool result = false;

    // 食材だったらステータスを更新
    // TODO 食材以外はあとで考える
    auto itemManager = NSModel::ItemManager::GetObj();
    auto itemDef = itemManager->GetItemDef(id);
    auto statusManager = NSModel::StatusManager::GetObj();

    if (itemDef.GetType() == NSModel::ItemDef::ItemType::FOOD)
    {
        // 満腹だったらfalseが返ってくる
        result = statusManager->Eat(itemDef);
        if (!result)
        {
            PopUp2::Get()->SetText(IDS_STRING_CANNOT_EAT);
        }
    }
    else if (itemDef.GetType() == NSModel::ItemDef::ItemType::MATERIAL)
    {
        PopUp2::Get()->SetText(IDS_STRING_USE_MATERIAL);
        result = false;
    }
    else if (itemDef.GetType() == NSModel::ItemDef::ItemType::VALUABLES)
    {
        // スマホ
        if (itemDef.GetId() == L"sumaho")
        {
            PopUp2::Get()->SetText(IDS_STRING_USE_SUMAHO);
            result = false;
        }
        // 家の鍵
        else if (itemDef.GetId() == L"homeKey")
        {
            PopUp2::Get()->SetText(IDS_STRING_USE_HOMEKEY);
            result = false;
        }
    }
    else if (itemDef.GetType() == NSModel::ItemDef::ItemType::OTHERS)
    {
        // ワードブレス
        if (itemDef.GetId() == L"wordbress")
        {
            auto pos = SharedObj::GetPlayer();
            statusManager->DrinkWordBress(pos->GetPos().x, pos->GetPos().y, pos->GetPos().z);
            result = true;
        }
        else
        {
            PopUp2::Get()->SetText(IDS_STRING_USE_DEFAULT);
            result = false;
        }
    }

    return result;
}

void MenuManager::DeleteItem(const std::wstring& id, const int subId)
{
    // 装備中の袋だったら削除できないようにする
    {
        auto allBag = Common::Status()->GetAllBag();
        for (auto it = allBag.begin(); it != allBag.end(); ++it)
        {
            if (it->GetId() == id && it->GetSubId() == subId)
            {
                PopUp2::Get()->SetText(IDS_STRING199);
                return;
            }
        }
    }

    // 装備中の武器だったら削除できないようにする
    {
        auto weapon = Common::Status()->GetEquipWeapon();
        if (weapon.GetId() == id && weapon.GetSubId() == subId)
        {
            PopUp2::Get()->SetText(IDS_STRING200);
            return;
        }
    }

    Common::Inventory()->RemoveItem(id, subId);

    int brokenBagNum1 = 0;
    {
        auto allBag = Common::Status()->GetAllBag();
        brokenBagNum1 = (int)std::count_if(allBag.begin(), allBag.end(),
                                           [&](auto x)
                                           {
                                               return x.GetDurabilityCurrent() == 0;
                                           });
    }

    Common::ReduceBrainStaminaCurrent(0.1f);
    Common::Inventory()->ReduceEquipBagDurability();

    m_menu.DeleteItem(id, subId);

    // 耐久度が下がったので更新
    {
        auto allBag = Common::Status()->GetAllBag();
        for (auto it = allBag.begin(); it != allBag.end(); ++it)
        {
            if (it->GetId().empty())
            {
                continue;
            }

            // NSMenu::ItemInfoとNSModel::ItemInfoの変換
            // レベルと装備状態と耐久度だけでいい
            NSMenu::ItemInfo itemInfo;
            itemInfo.SetId(it->GetId());
            itemInfo.SetSubId(it->GetSubId());
            itemInfo.SetLevel(it->GetItemDef().GetLevel());
            itemInfo.SetEquip(true);
            itemInfo.SetDurability(it->GetDurabilityCurrent());
            m_menu.UpdateItem(itemInfo);
        }
    }

    // 新たに耐久値が0になった袋があったか
    int brokenBagNum2 = 0;
    {
        auto allBag = Common::Status()->GetAllBag();
        brokenBagNum2 = (int)std::count_if(allBag.begin(), allBag.end(),
                                           [&](auto x)
                                           {
                                               return x.GetDurabilityCurrent() == 0;
                                           });
    }

    if (brokenBagNum1 != brokenBagNum2)
    {
        PopUp2::Get()->SetText(IDS_STRING145);
    }

    m_menu.SetWeightAll(Common::Inventory()->GetWeight());
    m_menu.SetVolumeAll((int)Common::Inventory()->GetVolume());
    m_menu.SetVolumeMax((int)Common::Inventory()->GetVolumeMax());
}

void MenuManager::AddItem(const std::wstring& id, const int subId, const int durability)
{
    using namespace NSMenu;
    NSModel::ItemManager* itemManager = NSModel::ItemManager::GetObj();
    NSModel::Inventory* inventory = NSModel::Inventory::GetObj();
    NSModel::WeaponManager* weaponManager = NSModel::WeaponManager::GetObj();

    NSModel::ItemDef itemDef = itemManager->GetItemDef(id);
    std::wstring work_str;

    NSMenu::ItemInfo itemInfoG;

    itemInfoG.SetName(itemDef.GetName());
    itemInfoG.SetDurability(durability);

    NSMenu::Sprite* sprItem = NEW NSMenu::Sprite(SharedObj::GetD3DDevice());

    // 画像ファイル名を取得して設定
    // アイテム種別が武器の時は武器クラスから取得する必要がある
    if (itemDef.GetType() != NSModel::ItemDef::ItemType::WEAPON)
    {
        work_str = itemDef.GetImagePath();
    }
    else
    {
        work_str = weaponManager->GetImageName2(itemDef.GetWeaponId());
    }

    sprItem->Load(work_str);
    itemInfoG.SetSprite(sprItem);

    if (itemDef.GetType() != NSModel::ItemDef::ItemType::WEAPON)
    {
        work_str = itemDef.GetDetail();
    }
    else
    {
        work_str = weaponManager->GetDetail2(itemDef.GetWeaponId());
    }
    work_str = itemDef.GetDetail();
    work_str.erase(std::remove(work_str.begin(), work_str.end(), '"'), work_str.end());

    itemInfoG.SetId(id);
    itemInfoG.SetSubId(subId);

    // ID : 27 ~ 32 == 袋
    if (itemDef.GetUnreinforcedId() == L"bag")
    {
        itemInfoG.SetEquipEnable(true);

        if (IsBagEquiped(id, subId))
        {
            itemInfoG.SetEquip(true);
        }
    }
    else if (itemDef.GetId() == L"sugegasa")
    {
        itemInfoG.SetEquipEnable(true);

        if (Common::Status()->GetSugegasa())
        {
            itemInfoG.SetEquip(true);
        }
    }
    else if (itemDef.GetType() == NSModel::ItemDef::ItemType::WEAPON)
    {
        itemInfoG.SetEquipEnable(true);

        auto weapon = Common::Status()->GetEquipWeapon();
        if (weapon.GetId() == id &&
            weapon.GetSubId() == subId)
        {
            itemInfoG.SetEquip(true);
        }
    }

    itemInfoG.SetDetail(work_str);

    itemInfoG.SetWeight(itemDef.GetWeight());
    itemInfoG.SetVolume((int)itemDef.GetVolume());

    Common::ReduceBrainStaminaCurrent(0.1f);

    int brokenBagNum1 = 0;
    {
        auto allBag = Common::Status()->GetAllBag();
        brokenBagNum1 = (int)std::count_if(allBag.begin(), allBag.end(),
                                           [&](auto x)
                                           {
                                               return x.GetDurabilityCurrent() == 0;
                                           });
    }

    Common::Inventory()->ReduceEquipBagDurability();

    m_menu.AddItem(itemInfoG);

    // 耐久値が下がったので更新
    {
        auto allBag = Common::Status()->GetAllBag();
        for (auto it = allBag.begin(); it != allBag.end(); ++it)
        {
            if (it->GetId().empty())
            {
                continue;
            }

            // NSMenu::ItemInfoとNSModel::ItemInfoの変換
            // レベルと装備状態と耐久度だけでいい
            NSMenu::ItemInfo itemInfo;
            itemInfo.SetId(it->GetId());
            itemInfo.SetSubId(it->GetSubId());
            itemInfo.SetLevel(it->GetItemDef().GetLevel());
            itemInfo.SetEquip(true);
            itemInfo.SetDurability(it->GetDurabilityCurrent());
            m_menu.UpdateItem(itemInfo);
        }
    }

    // 新たに耐久値が0になった袋があったか
    int brokenBagNum2 = 0;
    {
        auto allBag = Common::Status()->GetAllBag();
        brokenBagNum2 = (int)std::count_if(allBag.begin(), allBag.end(),
                                           [&](auto x)
                                           {
                                               return x.GetDurabilityCurrent() == 0;
                                           });
    }

    if (brokenBagNum1 != brokenBagNum2)
    {
        PopUp2::Get()->SetText(IDS_STRING145);
    }

    m_menu.SetWeightAll(Common::Inventory()->GetWeight());
    m_menu.SetVolumeAll((int)Common::Inventory()->GetVolume());
    m_menu.SetVolumeMax((int)Common::Inventory()->GetVolumeMax());
}

bool MenuManager::IsBagEquiped(const std::wstring& id, const int subId)
{
    auto statusManager = NSModel::StatusManager::GetObj();
    auto allBag = statusManager->GetAllBag();
    auto it = std::find_if(allBag.begin(), allBag.end(),
                           [&](const NSModel::ItemInfo& x)
                           {
                               return x.GetId() == id && x.GetSubId() == subId;
                           });

    if (it != allBag.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void MenuManager::Equip(const std::wstring& id, const int subId)
{
    //--------------------------------------
    // 袋か武器か
    //--------------------------------------
    auto itemdef = Common::ItemManager()->GetItemDef(id);

    // 武器だったら
    if (itemdef.GetType() == NSModel::ItemDef::ItemType::WEAPON)
    {
        //--------------------------------------
        // すでに装備していたものがあったら、その装備状態は解除する
        // ただし、装備していたものが袋や松明の場合、やや複雑になる。
        // 袋を装備していた場合、武器を装備しても袋は装備したままでよい。
        // しかし、右手に袋を装備していた場合、武器を装備ができないようにする。
        //--------------------------------------
        {
            bool lit = NSModel::WeaponManager::GetObj()->IsTorchLit();
            if (lit)
            {
                PopUp2::Get()->SetText(IDS_STRING198);
                return;
            }

            if (Common::Status()->GetBagState().size() >= 5)
            {
                PopUp2::Get()->SetText(IDS_STRING_RIGHT_HAND);
                return;
            }

            auto weapon = Common::Status()->GetEquipWeapon();
            if (weapon.GetId().size() >= 1)
            {
                Unequip(weapon.GetId(), weapon.GetSubId());
            }
        }

        auto itemInfo = Common::Inventory()->GetItemInfo(id, subId);
        Common::Status()->SetEquipWeapon(itemInfo);

        NSMenu::ItemInfo itemInfoG;
        itemInfoG.SetId(id);
        itemInfoG.SetSubId(subId);
        itemInfoG.SetDurability(itemInfo.GetDurabilityCurrent());
        itemInfoG.SetLevel(itemInfo.GetItemDef().GetLevel());
        itemInfoG.SetEquip(true);
        m_menu.UpdateItem(itemInfoG);

    }
    // 菅笠だったら
    else if (id == L"sugegasa")
    {
        auto itemInfo = Common::Inventory()->GetItemInfo(id, subId);
        Common::Status()->SetSugegasa(true);

        NSMenu::ItemInfo itemInfoG;
        itemInfoG.SetId(id);
        itemInfoG.SetSubId(subId);
        itemInfoG.SetDurability(itemInfo.GetDurabilityCurrent());
        itemInfoG.SetLevel(itemInfo.GetItemDef().GetLevel());
        itemInfoG.SetEquip(true);
        m_menu.UpdateItem(itemInfoG);
    }
    // 袋だったら
    else if (itemdef.GetUnreinforcedId() == L"bag")
    {
        // 袋を5個装備していたら装備しない
        if (Common::Status()->GetBagState().size() >= 5)
        {
            PopUp2::Get()->SetText(IDS_STRING202);
            return;
        }

        auto itemInfo = Common::Inventory()->GetItemInfo(id, subId);
        Common::Status()->EquipBag(itemInfo);

        NSMenu::ItemInfo itemInfoG;
        itemInfoG.SetId(id);
        itemInfoG.SetSubId(subId);
        itemInfoG.SetDurability(itemInfo.GetDurabilityCurrent());
        itemInfoG.SetLevel(itemInfo.GetItemDef().GetLevel());
        itemInfoG.SetEquip(true);
        m_menu.UpdateItem(itemInfoG);

        m_menu.SetWeightAll(Common::Inventory()->GetWeight());
        m_menu.SetVolumeAll((int)Common::Inventory()->GetVolume());
        m_menu.SetVolumeMax((int)Common::Inventory()->GetVolumeMax());
    }
}

void MenuManager::Unequip(const std::wstring& id, const int subId)
{
    auto itemInfo = Common::Inventory()->GetItemInfo(id, subId);
    NSMenu::ItemInfo itemInfoG;
    itemInfoG.SetId(id);
    itemInfoG.SetSubId(subId);
    itemInfoG.SetDurability(itemInfo.GetDurabilityCurrent());
    itemInfoG.SetLevel(itemInfo.GetItemDef().GetLevel());
    itemInfoG.SetEquip(false);
    m_menu.UpdateItem(itemInfoG);

    auto itemdef = Common::ItemManager()->GetItemDef(id);

    // 武器だったら
    if (itemdef.GetType() == NSModel::ItemDef::ItemType::WEAPON)
    {
        NSModel::ItemInfo itemInfo;
        itemInfo.SetId(L"");
        Common::Status()->SetEquipWeapon(itemInfo);
    }
    // 菅笠だったら
    else if (id == L"sugegasa")
    {
        Common::Status()->SetSugegasa(false);
    }
    // 袋だったら
    else if (itemdef.GetUnreinforcedId() == L"bag")
    {
        Common::Status()->UnequipBag(id, subId);

        m_menu.SetWeightAll(Common::Inventory()->GetWeight());
        m_menu.SetVolumeAll((int)Common::Inventory()->GetVolume());
        m_menu.SetVolumeMax((int)Common::Inventory()->GetVolumeMax());
    }
}


