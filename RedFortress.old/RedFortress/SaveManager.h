#pragma once

#include <string>
#include <atomic>
#include <tchar.h>

class SaveManager
{
public:
    static SaveManager* Get();
    static void Destroy();
    void Save();
    void LoadOrigin();
    void Load();
    void LoadDemoData();
    void DeleteSavedata();

    // 別スレッドから進捗を取得できるようにする
    int GetProgress();

    std::wstring GetLangFile();
    void SetLangFile(const std::wstring lang);

    bool SaveFolderExists();
    bool DemoFolderExists();

private:

    SaveManager();

    bool DeleteFolderContents(const std::wstring& folderPath);
    bool DeleteFolder(const std::wstring& folderPath);

    static SaveManager* m_obj;

    const std::wstring ORIGIN_DATA_PATH = _T("res\\script\\origin\\");
    const std::wstring SAVEDATA_PATH = _T("res\\script\\save\\");
    const std::wstring SAVEDATA_FOLDER = _T("res\\script\\save");

    const std::wstring DEMODATA_PATH = _T("res\\script\\demo\\");
    const std::wstring DEMODATA_FOLDER = _T("res\\script\\demo");

    std::wstring m_savedata_path;
    std::wstring m_savedata_folder;

    std::wstring m_demodata_path = DEMODATA_PATH;
    std::wstring m_demodata_folder = DEMODATA_FOLDER;

    std::wstring CreateOriginFilePath(const std::wstring& filename);
    std::wstring CreateSaveFilePath(const std::wstring& filename);
    std::wstring CreateDemoFilePath(const std::wstring& filename);

    std::wstring GetOriginMapPath();
    std::wstring GetSavefileMapPath();
    std::wstring GetDemofileMapPath();

    bool m_savedataLoaded = false;

    bool m_encrypt = true;

    std::atomic<int> m_progress = 0;
};

