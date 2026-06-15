#include "SaveManager.h"
#include "SharedObj.h"

#include "../../RedFortressModel/Model/HumanInfoManager.h"
#include "../../RedFortressModel/Model/MapInfoManager.h"
#include "../../RedFortressModel/Model/ItemManager.h"
#include "../../RedFortressModel/Model/Inventory.h"
#include "../../RedFortressModel/Model/Storehouse.h"
#include "../../RedFortressModel/Model/WeaponManager.h"
#include "../../RedFortressModel/Model/EnemyInfoManager.h"
#include "../../RedFortressModel/Model/SkillManager.h"
#include "../../RedFortressModel/Model/StatusManager.h"
#include "../../RedFortressModel/Model/Guide.h"
#include "../../RedFortressModel/Model/PowereggDateTime.h"
#include "../../RedFortressModel/Model/MapObjManager.h"
#include "../../RedFortressModel/Model/NpcStatusManager.h"
#include "../../RedFortressModel/Model/Rynen.h"
#include "QuestManager.h"
#include "../../RedFortressModel/Model/PatchTestManager.h"
#include "../../RedFortressModel/Model/Voyage.h"
#include "../../RedFortressModel/Model/ActivityBase.h"
#include "../../RedFortressModel/Model/CraftInfoManager.h"
#include "../../RedFortressModel/Model/CraftSystem.h"
#include "../../RedFortressModel/Model/Help.h"

#include <windows.h>
#include <shlobj.h>

SaveManager* SaveManager::m_obj = nullptr;

SaveManager* SaveManager::Get()
{
    if (m_obj == nullptr)
    {
        m_obj = NEW SaveManager();

        if (Common::EncryptMode())
        {
            m_obj->m_encrypt = true;
        }
        else
        {
            m_obj->m_encrypt = false;
        }
    }

    return m_obj;
}

bool CreateDirectoriesRecursively(const std::wstring& path) {
    std::wstringstream ss(path);
    std::wstring item;
    std::wstring currentPath;

    // Windowsのパス区切りに対応（\ または /）
    wchar_t delimiter = L'\\';
    if (path.find(L'/') != std::wstring::npos)
    {
        delimiter = L'/';
    }

    while (std::getline(ss, item, delimiter))
    {
        if (item.empty())
        {
            continue;
        }

        currentPath += item + delimiter;

        if (GetFileAttributes(currentPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            if (!CreateDirectory(currentPath.c_str(), NULL))
            {
                DWORD err = GetLastError();
                if (err != ERROR_ALREADY_EXISTS)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

SaveManager::SaveManager()
{
    {
        wchar_t work[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, work); // %APPDATA%

        std::wstring savedir;
        savedir = work;
        savedir += _T("\\Starman\\res\\script");

        if (PathFileExists(savedir.c_str()) == FALSE)
        {
            auto result = CreateDirectoriesRecursively(savedir);
            if (!result)
            {
                throw std::exception();
            }
        }
    }

    {
        wchar_t path[MAX_PATH];
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path); // %APPDATA%

        m_savedata_path = path;
        m_savedata_path += _T("\\Starman\\");
        m_savedata_path += SAVEDATA_PATH;

        m_savedata_folder = path;
        m_savedata_folder += _T("\\Starman\\");
        m_savedata_folder += SAVEDATA_FOLDER;
    }
}

void SaveManager::Destroy()
{
    SAFE_DELETE(m_obj);
}

std::wstring SaveManager::CreateOriginFilePath(const std::wstring& filename)
{
    std::wstring originDataPath;

    originDataPath = ORIGIN_DATA_PATH;
    originDataPath += filename;
    if (Common::EncryptMode())
    {
        if (m_encrypt)
        {
            originDataPath.replace(originDataPath.size() - 3, 3, _T("enc"));
        }
    }
    return originDataPath;
}

std::wstring SaveManager::CreateSaveFilePath(const std::wstring& filename)
{
    std::wstring saveDataPath;

    saveDataPath = m_savedata_path;
    saveDataPath += filename;

    if (Common::EncryptMode())
    {
        if (m_encrypt)
        {
            saveDataPath.replace(saveDataPath.size() - 3, 3, _T("enc"));
        }
    }
    return saveDataPath;
}

std::wstring SaveManager::CreateDemoFilePath(const std::wstring& filename)
{
    std::wstring saveDataPath;

    saveDataPath = m_demodata_path;
    saveDataPath += filename;

    if (Common::EncryptMode())
    {
        if (m_encrypt)
        {
            saveDataPath.replace(saveDataPath.size() - 3, 3, _T("enc"));
        }
    }
    return saveDataPath;
}

std::wstring SaveManager::GetOriginMapPath()
{
    std::wstring path= ORIGIN_DATA_PATH + _T("map_obj.bin");
    return path;
}

std::wstring SaveManager::GetSavefileMapPath()
{
    std::wstring path = m_savedata_path + _T("map_obj.bin");
    return path;
}

std::wstring SaveManager::GetDemofileMapPath()
{
    std::wstring path = m_demodata_path + _T("map_obj.bin");
    return path;
}

void SaveManager::Save()
{
    // フォルダがなければ作る
    std::wstring savedir;

    wchar_t work[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, work); // %APPDATA%

    savedir = work;
    savedir += _T("\\Starman\\");
    savedir += _T("res\\script\\save");

    if (PathFileExists(savedir.c_str()) == FALSE)
    {
        BOOL result = CreateDirectory(savedir.c_str(), NULL);
        if (result == FALSE)
        {
            throw std::exception();
        }
    }

    auto rynen = NSModel::Rynen::GetObj();
    rynen->Save(CreateSaveFilePath(_T("rynen.csv")), m_encrypt);

    NSModel::NpcStatusManager* mgr = NSModel::NpcStatusManager::GetObj();
    mgr->Save(CreateSaveFilePath(_T("npcStatus.csv")), m_encrypt);

    NSModel::HumanInfoManager* him = NSModel::HumanInfoManager::GetObj();
    him->Save(CreateSaveFilePath(_T("humanInfoSub.csv")), m_encrypt);

    NSModel::MapInfoManager* mapManager = NSModel::MapInfoManager::GetObj();
    mapManager->Save(CreateSaveFilePath(Common::AddEnIfEng(_T("mapInfoSave.csv"))), m_encrypt);

    NSModel::ItemManager* itemManager = NSModel::ItemManager::GetObj();
    itemManager->Save(CreateSaveFilePath(_T("item_pos.csv")), m_encrypt);

    NSModel::Inventory* inventory = NSModel::Inventory::GetObj();
    inventory->Save(CreateSaveFilePath(_T("inventory.csv")), m_encrypt);

    auto storehouseManager = NSModel::StorehouseManager::Get();
    storehouseManager->Save(CreateSaveFilePath(_T("storehouseListSave.csv")), m_savedata_folder);

    NSModel::WeaponManager* weaponManager = NSModel::WeaponManager::GetObj();
    weaponManager->Save(CreateSaveFilePath(_T("weaponSave.csv")), m_encrypt);

    NSModel::EnemyInfoManager* enemyInfoManager = NSModel::EnemyInfoManager::GetObj();
    enemyInfoManager->Save(CreateSaveFilePath(_T("enemy.bin")),
                           CreateSaveFilePath(_T("enemyVisible.csv")),
                           m_encrypt,
                           true);

    NSModel::SkillManager* skillManager = NSModel::SkillManager::GetObj();
    skillManager->Save(CreateSaveFilePath(_T("skillSub.csv")), m_encrypt);

    NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
    statusManager->Save(CreateSaveFilePath(_T("status.csv")),
                        SharedObj::GetPlayer()->GetPos().x,
                        SharedObj::GetPlayer()->GetPos().y,
                        SharedObj::GetPlayer()->GetPos().z,
                        m_encrypt);

    NSModel::Guide* guide = NSModel::Guide::GetObj();
    guide->Save(CreateSaveFilePath(Common::AddEnIfEng(_T("guideSave.csv"))), m_encrypt);

    NSModel::PowereggDateTime* datetime = NSModel::PowereggDateTime::GetObj();
    datetime->Save(CreateSaveFilePath(_T("datetime.csv")), m_encrypt);

    NSModel::MapObjManager* mapObjManager = NSModel::MapObjManager::GetObj();
    mapObjManager->SaveWithBinary(GetSavefileMapPath());

    QuestManager::Get()->Save(CreateSaveFilePath(_T("questSave.csv")));

    NSModel::PatchTestManager* patchTestManager = NSModel::PatchTestManager::Get();
    patchTestManager->Save(CreateSaveFilePath(_T("patchTestInfoSave.csv")),
                           CreateSaveFilePath(_T("patchTestQueSave.csv")));

    auto voyage = NSModel::Voyage::Get();
    voyage->Save(CreateSaveFilePath(_T("raftSave.csv")));

    auto activityBase = NSModel::ActivityBase::Get();
    activityBase->Save(CreateSaveFilePath(_T("activityBase.csv")));

    NSModel::CraftSystem::GetObj()->Save(CreateSaveFilePath(_T("craftsmanSkillSave.csv")),
                                              CreateSaveFilePath(_T("craftsmanQueueSave.csv")));

    NSModel::Help::Get()->Save(CreateSaveFilePath(_T("helpSave.csv")));
}

void SaveManager::LoadOrigin()
{
    m_progress.store(0);

    // 「ゲームをはじめからスタート→死亡→もう一度オープニングから」
    // この操作を行うとセーブデータがないのに死亡したステータスが記録されている。
    // 必ず初期化しないといけない。
//    if (m_savedataLoaded)
    {
        NSModel::Rynen::Destroy();
        NSModel::NpcStatusManager::Destroy();
        NSModel::HumanInfoManager::Destroy();
        NSModel::MapInfoManager::Destroy();
        NSModel::ItemManager::Destroy();
        NSModel::Inventory::Destroy();
        NSModel::StorehouseManager::Destroy();
        NSModel::WeaponManager::Destroy();
        NSModel::EnemyInfoManager::Destroy();
        NSModel::SkillManager::Destroy();
        NSModel::StatusManager::Destroy();
        NSModel::Guide::Destroy();
        NSModel::PowereggDateTime::Destroy();
        NSModel::MapObjManager::Destroy();
        NSModel::ActivityBase::Get()->Finalize();
        NSModel::Voyage::Get()->Destroy();
        NSModel::CraftInfoManager::Destroy();
        NSModel::CraftSystem::Destroy();
        NSModel::Help::Destroy();

        QuestManager::Finalize();
    }

    auto rynen = NSModel::Rynen::GetObj();
    rynen->Init(CreateOriginFilePath(_T("rynen.csv")), m_encrypt);

    m_progress.store(10);
    NSModel::NpcStatusManager* mgr = NSModel::NpcStatusManager::GetObj();
    mgr->Init(CreateOriginFilePath(_T("npcStatus.csv")), m_encrypt);

    NSModel::HumanInfoManager* him = NSModel::HumanInfoManager::GetObj();
    him->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("humanInfo.csv"))),
              CreateOriginFilePath(_T("humanInfoSub.csv")),
              m_encrypt);

    m_progress.store(20);
    NSModel::MapInfoManager* mapManager = NSModel::MapInfoManager::GetObj();
    mapManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("mapInfoDef.csv"))),
                     CreateOriginFilePath(_T("mapInfoOrigin.csv")),
                     m_encrypt);

    NSModel::ItemManager* itemManager = NSModel::ItemManager::GetObj();
    itemManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("item.csv"))),
                      CreateOriginFilePath(_T("item_pos.csv")),
                      m_encrypt);

    m_progress.store(25);
    NSModel::Inventory* inventory = NSModel::Inventory::GetObj();
    inventory->Init(CreateOriginFilePath(_T("inventory.csv")), m_encrypt);

    auto storehouseManager = NSModel::StorehouseManager::Get();
    storehouseManager->Init(CreateOriginFilePath(_T("storehouseListOrigin.csv")));

    NSModel::WeaponManager* weaponManager = NSModel::WeaponManager::GetObj();
    weaponManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("weapon.csv"))),
                        CreateOriginFilePath(_T("weaponSave.csv")),
                        m_encrypt);

    m_progress.store(30);
    NSModel::EnemyInfoManager* enemyInfoManager = NSModel::EnemyInfoManager::GetObj();
    enemyInfoManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("enemyDef.csv"))),
                           CreateOriginFilePath(_T("enemy.bin")),
                           CreateOriginFilePath(_T("enemyVisible.csv")),
                           m_encrypt,
                           true);
            
        
    NSModel::SkillManager* skillManager = NSModel::SkillManager::GetObj();
    skillManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("skill.csv"))),
                       CreateOriginFilePath(_T("skillSub.csv")),
                        m_encrypt);
            
    m_progress.store(35);
    NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
    statusManager->Init(CreateOriginFilePath(_T("status.csv")), m_encrypt);

    NSModel::Guide* guide = NSModel::Guide::GetObj();
    guide->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("guideDef.csv"))),
                CreateOriginFilePath(_T("guideOrigin.csv")),
                m_encrypt);

    NSModel::PowereggDateTime* datetime = NSModel::PowereggDateTime::GetObj();
    datetime->Init(CreateOriginFilePath(_T("datetime.csv")), m_encrypt);

    m_progress.store(40);
    NSModel::MapObjManager* mapObjManager = NSModel::MapObjManager::GetObj();
    mapObjManager->InitWithBinary(GetOriginMapPath(),
                                  CreateOriginFilePath(_T("map_obj_type.csv")), m_encrypt);

    QuestManager::Get()->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("quest.csv"))), _T(""));

    m_progress.store(45);
    NSModel::PatchTestManager* patchTestManager = NSModel::PatchTestManager::Get();
    patchTestManager->Init(CreateOriginFilePath(_T("patchTestOrigin.csv")),
                           _T(""),
                           _T(""));

    auto voyage = NSModel::Voyage::Get();
    voyage->Init(_T(""));

    m_progress.store(75);
    auto activityBase = NSModel::ActivityBase::Get();
    activityBase->Init(CreateOriginFilePath(_T("activityBase.csv")));

    NSModel::CraftInfoManager::GetObj()->Init(CreateOriginFilePath(_T("craftDef.csv")));

    NSModel::CraftSystem::GetObj()->Init(CreateOriginFilePath(_T("craftsmanSkill.csv")),
                                              CreateOriginFilePath(_T("craftsmanQueue.csv")));

    NSModel::Help::Get()->Init(CreateOriginFilePath(_T("help.csv")));

    m_progress.store(100);
}

void SaveManager::Load()
{
    m_progress.store(0);

    {
        NSModel::Rynen::Destroy();
        NSModel::NpcStatusManager::Destroy();
        NSModel::HumanInfoManager::Destroy();
        NSModel::MapInfoManager::Destroy();
        NSModel::ItemManager::Destroy();
        NSModel::Inventory::Destroy();
        NSModel::StorehouseManager::Destroy();
        NSModel::WeaponManager::Destroy();
        NSModel::EnemyInfoManager::Destroy();
        NSModel::SkillManager::Destroy();
        NSModel::StatusManager::Destroy();
        NSModel::Guide::Destroy();
        NSModel::PowereggDateTime::Destroy();
        NSModel::MapObjManager::Destroy();
        NSModel::ActivityBase::Get()->Finalize();
        NSModel::Voyage::Get()->Destroy();
        NSModel::CraftInfoManager::Destroy();
        NSModel::CraftSystem::Destroy();
        NSModel::Help::Destroy();

        QuestManager::Finalize();
    }

    auto rynen = NSModel::Rynen::GetObj();
    rynen->Init(CreateSaveFilePath(_T("rynen.csv")), m_encrypt);

    NSModel::NpcStatusManager* mgr = NSModel::NpcStatusManager::GetObj();
    mgr->Init(CreateSaveFilePath(_T("npcStatus.csv")), m_encrypt);

    NSModel::HumanInfoManager* him = NSModel::HumanInfoManager::GetObj();
    him->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("humanInfo.csv"))),
              CreateSaveFilePath(_T("humanInfoSub.csv")),
              m_encrypt);

    m_progress.store(5);
    NSModel::MapInfoManager* mapManager = NSModel::MapInfoManager::GetObj();
    mapManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("mapInfoDef.csv"))),
                     CreateSaveFilePath(_T("mapInfoSave.csv")),
                     m_encrypt);

    m_progress.store(5);
    NSModel::ItemManager* itemManager = NSModel::ItemManager::GetObj();
    itemManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("item.csv"))),
                      CreateSaveFilePath(_T("item_pos.csv")),
                      m_encrypt);

    m_progress.store(5);
    NSModel::Inventory* inventory = NSModel::Inventory::GetObj();
    inventory->Init(CreateSaveFilePath(_T("inventory.csv")), m_encrypt);

    m_progress.store(5);

    auto storehouseManager = NSModel::StorehouseManager::Get();
    storehouseManager->Init(CreateSaveFilePath(_T("storehouseListSave.csv")));

    m_progress.store(10);
    NSModel::WeaponManager* weaponManager = NSModel::WeaponManager::GetObj();
    weaponManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("weapon.csv"))),
                        CreateSaveFilePath(_T("weaponSave.csv")),
                        m_encrypt);

    m_progress.store(15);
    NSModel::EnemyInfoManager* enemyInfoManager = NSModel::EnemyInfoManager::GetObj();
    enemyInfoManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("enemyDef.csv"))),
                           CreateSaveFilePath(_T("enemy.bin")),
                           CreateSaveFilePath(_T("enemyVisible.csv")),
                           m_encrypt,
                           true);
            
    m_progress.store(20);
        
    NSModel::SkillManager* skillManager = NSModel::SkillManager::GetObj();
    skillManager->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("skill.csv"))),
                       CreateSaveFilePath(_T("skillSub.csv")),
                        m_encrypt);
            
    m_progress.store(25);

    NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
    statusManager->Init(CreateSaveFilePath(_T("status.csv")), m_encrypt);

    m_progress.store(30);

    NSModel::Guide* guide = NSModel::Guide::GetObj();
    guide->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("guideDef.csv"))),
                CreateSaveFilePath(_T("guideSave.csv")), 
                m_encrypt);

    m_progress.store(35);

    NSModel::PowereggDateTime* datetime = NSModel::PowereggDateTime::GetObj();
    datetime->Init(CreateSaveFilePath(_T("datetime.csv")), m_encrypt);

    m_progress.store(40);

    NSModel::MapObjManager* mapObjManager = NSModel::MapObjManager::GetObj();
//    mapObjManager->Init(CreateSaveFilePath(_T("map_obj.csv")),
//                        CreateSaveFilePath(_T("map_obj_type.csv")), m_encrypt);

    mapObjManager->InitWithBinary(GetSavefileMapPath(),
                                  CreateOriginFilePath(_T("map_obj_type.csv")), m_encrypt);

    m_progress.store(45);

    QuestManager::Get()->Init(CreateOriginFilePath(Common::AddEnIfEng(_T("quest.csv"))),
                              CreateSaveFilePath(_T("questSave.csv")));

    m_progress.store(75);

    NSModel::PatchTestManager* patchTestManager = NSModel::PatchTestManager::Get();
    patchTestManager->Init(CreateOriginFilePath(_T("patchTestOrigin.csv")),
                           CreateSaveFilePath(_T("patchTestInfoSave.csv")),
                           CreateSaveFilePath(_T("patchTestQueSave.csv")));

    auto voyage = NSModel::Voyage::Get();
    voyage->Init(CreateSaveFilePath(_T("raftSave.csv")));

    auto activityBase = NSModel::ActivityBase::Get();
    activityBase->Init(CreateSaveFilePath(_T("activityBase.csv")));

    NSModel::CraftInfoManager::GetObj()->Init(CreateOriginFilePath(_T("craftDef.csv")));

    NSModel::CraftSystem::GetObj()->Init(CreateSaveFilePath(_T("craftsmanSkillSave.csv")),
                                              CreateSaveFilePath(_T("craftsmanQueueSave.csv")));

    NSModel::Help::Get()->Init(CreateSaveFilePath(_T("helpSave.csv")));

    m_savedataLoaded = true;

    m_progress.store(100);
}

void SaveManager::LoadDemoData()
{
    m_progress.store(0);

    // 「ゲームをはじめからスタート→死亡→もう一度オープニングから」
    // この操作を行うとセーブデータがないのに死亡したステータスが記録されている。
    // 必ず初期化しないといけない。
//    if (m_savedataLoaded)
    {
        NSModel::Rynen::Destroy();
        NSModel::NpcStatusManager::Destroy();
        NSModel::HumanInfoManager::Destroy();
        NSModel::MapInfoManager::Destroy();
        NSModel::ItemManager::Destroy();
        NSModel::Inventory::Destroy();
        NSModel::StorehouseManager::Destroy();
        NSModel::WeaponManager::Destroy();
        NSModel::EnemyInfoManager::Destroy();
        NSModel::SkillManager::Destroy();
        NSModel::StatusManager::Destroy();
        NSModel::Guide::Destroy();
        NSModel::PowereggDateTime::Destroy();
        NSModel::MapObjManager::Destroy();
        NSModel::ActivityBase::Get()->Finalize();
        NSModel::Voyage::Get()->Destroy();
        NSModel::CraftInfoManager::Destroy();
        NSModel::CraftSystem::Destroy();
        NSModel::Help::Destroy();

        QuestManager::Finalize();
    }

    auto rynen = NSModel::Rynen::GetObj();
    rynen->Init(CreateDemoFilePath(_T("rynen.csv")), m_encrypt);

    m_progress.store(10);
    NSModel::NpcStatusManager* mgr = NSModel::NpcStatusManager::GetObj();
    mgr->Init(CreateDemoFilePath(_T("npcStatus.csv")), m_encrypt);

    NSModel::HumanInfoManager* him = NSModel::HumanInfoManager::GetObj();
    him->Init(CreateDemoFilePath(Common::AddEnIfEng(_T("humanInfo.csv"))),
              CreateDemoFilePath(_T("humanInfoSub.csv")),
              m_encrypt);

    m_progress.store(20);
    NSModel::MapInfoManager* mapManager = NSModel::MapInfoManager::GetObj();
    mapManager->Init(CreateDemoFilePath(Common::AddEnIfEng(_T("mapInfoDef.csv"))),
                     CreateDemoFilePath(_T("mapInfoOrigin.csv")),
                     m_encrypt);

    NSModel::ItemManager* itemManager = NSModel::ItemManager::GetObj();
    itemManager->Init(CreateDemoFilePath(Common::AddEnIfEng(_T("item.csv"))),
                      CreateDemoFilePath(_T("item_pos.csv")),
                      m_encrypt);

    m_progress.store(25);
    NSModel::Inventory* inventory = NSModel::Inventory::GetObj();
    inventory->Init(CreateDemoFilePath(_T("inventory.csv")), m_encrypt);

    auto storehouseManager = NSModel::StorehouseManager::Get();
    storehouseManager->Init(CreateDemoFilePath(_T("storehouseListOrigin.csv")));

    NSModel::WeaponManager* weaponManager = NSModel::WeaponManager::GetObj();
    weaponManager->Init(CreateDemoFilePath(Common::AddEnIfEng(_T("weapon.csv"))),
                        CreateDemoFilePath(_T("weaponSave.csv")),
                        m_encrypt);

    m_progress.store(30);
    NSModel::EnemyInfoManager* enemyInfoManager = NSModel::EnemyInfoManager::GetObj();
    enemyInfoManager->Init(CreateDemoFilePath(Common::AddEnIfEng(_T("enemyDef.csv"))),
                           CreateDemoFilePath(_T("enemy.bin")),
                           CreateDemoFilePath(_T("enemyVisible.csv")),
                           m_encrypt,
                           true);
            
        
    NSModel::SkillManager* skillManager = NSModel::SkillManager::GetObj();
    skillManager->Init(CreateDemoFilePath(Common::AddEnIfEng(_T("skill.csv"))),
                       CreateDemoFilePath(_T("skillSub.csv")),
                        m_encrypt);
            
    m_progress.store(35);
    NSModel::StatusManager* statusManager = NSModel::StatusManager::GetObj();
    statusManager->Init(CreateDemoFilePath(_T("status.csv")), m_encrypt);

    NSModel::Guide* guide = NSModel::Guide::GetObj();
    guide->Init(CreateDemoFilePath(Common::AddEnIfEng(_T("guideDef.csv"))),
                CreateDemoFilePath(_T("guideOrigin.csv")),
                m_encrypt);

    NSModel::PowereggDateTime* datetime = NSModel::PowereggDateTime::GetObj();
    datetime->Init(CreateDemoFilePath(_T("datetime.csv")), m_encrypt);

    m_progress.store(40);
    NSModel::MapObjManager* mapObjManager = NSModel::MapObjManager::GetObj();
    mapObjManager->InitWithBinary(GetDemofileMapPath(),
                                  CreateDemoFilePath(_T("map_obj_type.csv")), m_encrypt);

    QuestManager::Get()->Init(CreateDemoFilePath(Common::AddEnIfEng(_T("quest.csv"))), _T(""));

    m_progress.store(45);
    NSModel::PatchTestManager* patchTestManager = NSModel::PatchTestManager::Get();
    patchTestManager->Init(CreateDemoFilePath(_T("patchTestOrigin.csv")),
                           _T(""),
                           _T(""));

    auto voyage = NSModel::Voyage::Get();
    voyage->Init(CreateDemoFilePath(_T("raftSave.csv")));


    m_progress.store(75);
    auto activityBase = NSModel::ActivityBase::Get();
    activityBase->Init(CreateDemoFilePath(_T("activityBase.csv")));

    NSModel::CraftInfoManager::GetObj()->Init(CreateDemoFilePath(_T("craftDef.csv")));

    NSModel::CraftSystem::GetObj()->Init(CreateDemoFilePath(_T("craftsmanSkill.csv")),
                                              CreateDemoFilePath(_T("craftsmanQueue.csv")));

    NSModel::Help::Get()->Init(CreateDemoFilePath(_T("help.csv")));

    m_progress.store(100);
}


bool SaveManager::DeleteFolderContents(const std::wstring& folderPath)
{
    std::string work;
    std::wstring searchPath = folderPath + _T("\\*");
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        throw std::exception(work.c_str());
    }

    do
    {
        std::wstring fileName = findFileData.cFileName;

        // スキップする項目 (_T("." と ".."))
        if (fileName == _T(".") || fileName == _T("..")) {
            continue;
        }

        std::wstring fullPath = folderPath + _T("\\") + fileName;

        // ディレクトリの場合
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (!DeleteFolderContents(fullPath)) {
                FindClose(hFind);
                return false;
            }
            if (!RemoveDirectory(fullPath.c_str())) {
                FindClose(hFind);

                work = "Failed to delete directory: " + Common::WstringToUtf8(fullPath) + "\n";
                throw std::exception(work.c_str());

                return false;
            }
        }
        else { // ファイルの場合
            if (!DeleteFile(fullPath.c_str())) {
                FindClose(hFind);

                work = "Failed to delete file: " + Common::WstringToUtf8(fullPath) + "\n";
                throw std::exception(work.c_str());

                return false;
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    return true;
}

bool SaveManager::DeleteFolder(const std::wstring& folderPath)
{
    if (!DeleteFolderContents(folderPath))
    {
        return false;
    }

    if (!RemoveDirectory(folderPath.c_str()))
    {
        std::string work = "Failed to delete folder: " + Common::WstringToUtf8(folderPath) + "\n";
        throw std::exception(work.c_str());
        return false;
    }
    return true;
}

void SaveManager::DeleteSavedata()
{
    // セーブデータがなければセーブデータの削除は行わない（行えない）
    // セーブデータがないのにセーブデータの削除を行う関数が呼ばれることは問題ない。
    BOOL result = PathIsDirectory(m_savedata_folder.c_str());

    if (result == FALSE)
    {
        return;
    }

    DeleteFolder(m_savedata_folder);
}

int SaveManager::GetProgress()
{
    return m_progress.load();
}

std::wstring SaveManager::GetLangFile()
{
    std::wstring result;

    std::wstring langFilePath;
    wchar_t appData[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appData); // %APPDATA%

    langFilePath = appData;
    langFilePath += _T("\\Starman\\res\\script\\lang.ini");

    auto exists = PathFileExists(langFilePath.c_str());
    if (exists == TRUE)
    {
        std::ifstream file(langFilePath);

        std::wstring word;
        std::string word2;
        file >> word2;
        file.close();

        word = Common::Utf8ToWstring(word2);
        result = word;
    }

    return result;
}

void SaveManager::SetLangFile(const std::wstring lang)
{
    std::wstring langFilePath;
    wchar_t appData[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, appData); // %APPDATA%

    langFilePath = appData;
    langFilePath += _T("\\Starman\\res\\script\\lang.ini");

    std::ofstream file(langFilePath, std::ios::out | std::ios::trunc);
    file << Common::WstringToUtf8(lang);
    file.close();
}

bool SaveManager::SaveFolderExists()
{
    // ディレクトリだった時、TRUEではなく16が返ってくるので注意
    BOOL result = PathIsDirectory(m_savedata_folder.c_str());

    if (result != FALSE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool SaveManager::DemoFolderExists()
{
    // ディレクトリだった時、TRUEではなく16が返ってくるので注意
    BOOL result = PathIsDirectory(m_demodata_folder.c_str());

    if (result != FALSE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

