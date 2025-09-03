#include <cassert>
#include <shlobj.h>

#include "CppUnitTest.h"
#include "Util.h"
#include "../RedFortress/SharedObj.h"
#include "../RedFortress/MainWindow.h"
#include "../RedFortress/QuestManager.h"
#include "../RedFortress/SaveManager.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTest2
{

TEST_MODULE_INITIALIZE(ModuleInit)
{
    bool exist1 = Util::IsDirectory(_T("res\\script\\save"));
    bool exist2 = Util::IsDirectory(_T("res\\script\\save.bak"));

    if (exist1 && exist2)
    {
        Util::DeleteDirectory(_T("res\\script\\save"));
        int result1 = rename("res\\script\\save.bak", "res\\script\\save");
    }
    else if (!exist1 && exist2)
    {
        int result1 = rename("res\\script\\save.bak", "res\\script\\save");
    }
}

TEST_CLASS(MainWindowTest)
{
public:

    TEST_CLASS_INITIALIZE(Initialize)
    {
    }

    TEST_CLASS_CLEANUP(CleanUp)
    {
    }

    // 単純にpublic関数を呼ぶだけのテスト
    // （コンストラクタ）
    TEST_METHOD(MainWindowTest_TestMethod01)
    {
        MockKeyBoard keyboard;
        auto hInstance = (HINSTANCE)GetModuleHandle(0);

        // Target
        try
        {
            MainWindow sut(hInstance, &keyboard);
        }
        catch (...)
        {
            Assert::Fail();
        }

        auto hWnd = SharedObj::GetWindowHandle();
        BOOL result = DestroyWindow(hWnd);
        assert(result == 1);

        // DestroyWindowを行った後、メッセージ処理が完了するのを待つ
        MSG msg = { 0 };
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        hInstance = (HINSTANCE)GetModuleHandle(0);
        BOOL result2 = UnregisterClass(_T("ホシマン"), hInstance);
        assert(result2 == 1);
    }

    // 単純にpublic関数を呼ぶだけのテスト
    // MainLoop関数
    TEST_METHOD(MainWindowTest_TestMethod02)
    {
        MockKeyBoard* keyboard = NEW MockKeyBoard();
        auto hInstance = (HINSTANCE)GetModuleHandle(0);

        try
        {
            MainWindow sut(hInstance, keyboard);

            std::thread th1([&]
                            {
                                Sleep(40 * 1000);

                                keyboard->SetAltF4();
                            });

            // Target
            sut.MainLoop();

            th1.join();
        }
        catch (...)
        {
            Assert::Fail();
        }

        hInstance = (HINSTANCE)GetModuleHandle(0);
        BOOL result2 = UnregisterClass(_T("ホシマン"), hInstance);
        assert(result2 == 1);
    }

    //--------------------------------------------
    // ロード
    // →タイトル
    // →オープニング
    // →フィールド
    // →クエスト１開始
    // →クエスト１完了
    // の順でゲームを進めるたときにクラッシュしないことを確認するテスト
    // エンターキーを何度も押しているだけで到達できる
    //--------------------------------------------
    TEST_METHOD(MainWindowTest_TestMethod03)
    {
        MockKeyBoard* keyboard = NEW MockKeyBoard();;
        auto hInstance = (HINSTANCE)GetModuleHandle(0);

        try
        {
            MainWindow sut(hInstance, keyboard);

            std::thread th1([&]
                            {
                                // 1分エンターを押し続ける
                                for (int i = 0; i < 90; ++i)
                                {
                                    Sleep(1000 * 1);
                                    keyboard->SetKeyDownFirst(DIK_RETURN);
                                }

                                // エンターを押し続けるとクエスト１が完了する。
                                // しかしクエスト1が完了したことを知る方法がない。
                                // そこで、セーブを実行しセーブデータでクエスト１が完了となっているか確認する。
                                SaveManager::Get()->Save();
                                Sleep(500);

                                keyboard->SetAltF4();
                            });

            sut.MainLoop();

            th1.join();
        }
        catch (...)
        {
            Assert::Fail();
        }

        std::wstring savedata;

        // セーブデータ読み取り
        {
            // %APPDATA%\Starman\res\script\save\questSave.csv のパスを取得
            WCHAR appDataPath[MAX_PATH];
            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath)))
            {
                std::wstring fullPath = std::wstring(appDataPath) + L"\\Starman\\res\\script\\save\\questSave.csv";
                std::wifstream ifs(fullPath);
                if (ifs)
                {
                    ifs >> savedata;
                    ifs >> savedata;
                }
                else
                {
                    Assert::Fail();
                }
            }
            else
            {
                Assert::Fail();
            }
        }

        // Target
        auto it = savedata.find(L"Q100,FINISHED");
        Assert::AreNotEqual(std::wstring::npos, it);

        hInstance = (HINSTANCE)GetModuleHandle(0);
        BOOL result2 = UnregisterClass(_T("ホシマン"), hInstance);
        assert(result2 == 1);

    }

    // セーブデータを保存して読み込んだとき落ちないかを確認するテスト
    TEST_METHOD(MainWindowTest_TestMethod04)
    {

        MockKeyBoard* keyboard = NEW MockKeyBoard();
        auto hInstance = (HINSTANCE)GetModuleHandle(0);

        try
        {
            MainWindow sut(hInstance, keyboard);

            std::thread th1([&]
                            {
                                // 1分エンターを押し続ける
                                for (int i = 0; i < 90; ++i)
                                {
                                    Sleep(1000 * 1); 
                                    keyboard->SetKeyDownFirst(DIK_RETURN);
                                }

                                SaveManager::Get()->Save();

                                keyboard->SetAltF4();
                            });

            sut.MainLoop();

            th1.join();
        }
        catch (...)
        {
            Assert::Fail();
        }

        hInstance = (HINSTANCE)GetModuleHandle(0);
        BOOL result2 = UnregisterClass(_T("ホシマン"), hInstance);
        assert(result2 == 1);

        try
        {
            keyboard = NEW MockKeyBoard();
            MainWindow sut(hInstance, keyboard);

            std::thread th1([&]
                            {
                                Sleep(1000 * 30); 

                                // Continueを選択して決定
                                keyboard->SetKeyDownFirst(DIK_RIGHT);

                                Sleep(500 * 1); 

                                keyboard->SetKeyDownFirst(DIK_RETURN);

                                Sleep(1000 * 15); 

                                keyboard->SetAltF4();
                            });

            sut.MainLoop();

            th1.join();
        }
        catch (...)
        {
            Assert::Fail();
        }

        hInstance = (HINSTANCE)GetModuleHandle(0);
        BOOL result3 = UnregisterClass(_T("ホシマン"), hInstance);
        assert(result3 == 1);
    }
};
}

