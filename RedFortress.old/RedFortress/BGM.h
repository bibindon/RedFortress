#pragma once

#include <dsound.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>

//------------------------------------------------
// BGMと環境音はアルゴリズムが異なるので分けて考える必要がある。
// ・BGMが一度に二種類再生されることはない。
// 　・そのため、BGMをセットしたら以前再生していたBGMは停止してよい。（停止しないといけない）
// ・環境音は一度に二種類以上再生できる。
// ・BGMは時々曲を変更したいが環境音にその必要はない。
// ・BGMと環境音が同時になることは問題ない。
// ・スクリプトの都合上、enumを定義せずに「ファイル名」で管理する必要がある
// ・BGMの音量だけが変更されたときは最初から再生するのではなく音量の変更だけ行われるようにする
//------------------------------------------------

enum class eBGMStatus
{
    NONE,
    START_REQUEST,
    STARTED,
    STOP_REQUEST,
    STOPPED,
};

struct stBGM
{
    std::wstring m_filename;
    eBGMStatus m_eBGMStatus = eBGMStatus::NONE;
    int m_volume = 0;

    // 音量が変わったか
    bool m_bChangedVolume = false;
};

struct envBGM
{
    std::wstring m_filename;
    eBGMStatus m_eBGMStatus = eBGMStatus::NONE;
    int m_volume = 0;

    // 音量が変わったか
    bool m_bChangedVolume = false;
};

// BGMのロジック部分
class BGMModel
{
public:
    BGMModel();

    // 1秒に一度呼ばれる想定
    void Update();

    bool GetChangeRequest(stBGM* stBGM1, stBGM* stBGM2);
    void SetChangeRequestComplete();

    void SetEnding(const bool arg);
    void SetTrueEnd(const bool arg);
    void SetBattle(const bool arg);

    // 基本的にはBGMModelがBGMを選曲する。
    // ただし、会話シーン中に特定のBGMを再生したいときは外から指定してもらう必要がある。
    void SetTalkBGM(const std::wstring& filename);
    void FinalizeTalkBGM();

private:

    stBGM m_stBGM;
    stBGM m_stBGMPrev;

    // BGMの選曲
    // 上のルールほど優先度が高い
    //
    // 会話シーンで特定のBGMが指定されている場合はそのBGMを再生する。
    // 死亡していたら死亡時のBGM
    // タイトルだったらタイトルのBGM
    // オープニングだったらオープニングのBGM
    // エンディングだったらエンディングのBGM
    // 戦闘だったら戦闘のBGM
    // 瀕死だったら瀕死のBGM
    // 体力が少ないなら体力が少ないBGM
    // 夜だったら夜のBGM
    // 航海中だったら航海中のBGM
    // 特定の地域なら特定のBGM
    // それ以外ならランダム再生
    //
    // 基本的にBGMModel内で判定をする。外からじゃないと設定できない時もある。
    // そのときはそのときだ。

    bool m_bDead = false;
    bool m_bTitle = false;
    bool m_bOpening = false;
    bool m_bEnding = false;
    bool m_bTrueEnd = false;
    bool m_bBattle = false;
    bool m_bDying = false;
    bool m_bWeak = false;
    bool m_bNight = false;
    bool m_bVoyage = false;
    bool m_bHaikyo = false; // 廃墟の近くにいる
    bool m_bToudai = false; // 灯台の近くにいる
    bool m_bKaiganDoukutsu = false; // 海岸洞窟の近くにいる
    bool m_bJinja = false; // 神社の近くにいる
    bool m_bKokeniwa = false; // 苔庭の近くにいる
    bool m_bMinatoAto = false; // 港跡の近くにいる
    bool m_bDoukutsu = false; // 洞窟の中にいる

    std::wstring m_strTalkBGM;
    std::wstring m_strDead = L"res\\sound\\dead.wav";

    std::wstring m_strTitle = L"res\\sound\\title.wav";
    std::wstring m_strOpening = L"res\\sound\\opening.wav";

    std::wstring m_strEnding = L"res\\sound\\ending.wav";
    std::wstring m_strTrueEnd = L"res\\sound\\trueEnd.wav";
    std::wstring m_strBattle1 = L"res\\sound\\battle1.wav";
    std::wstring m_strBattle2 = L"res\\sound\\battle2.wav";
    std::wstring m_strDying = L"res\\sound\\dying.wav";
    std::wstring m_strWeak = L"res\\sound\\weak.wav";
    std::wstring m_strNight = L"res\\sound\\night.wav";
    std::wstring m_strVoyage = L"res\\sound\\voyage.wav";
    std::wstring m_strHaikyo = L"res\\sound\\haikyo.wav";
    std::wstring m_strToudai = L"res\\sound\\toudai.wav";
    std::wstring m_strKaiganDoukutsu = L"res\\sound\\kaiganDoukutsu.wav";
    std::wstring m_strJinja = L"res\\sound\\jinja.wav";
    std::wstring m_strKokeniwa = L"res\\sound\\kokeniwa.wav";
    std::wstring m_strMinatoAto = L"res\\sound\\minatoato.wav";
    std::wstring m_strDoukutsu = L"res\\sound\\doukutsu.wav";

    std::wstring m_strField1 = L"res\\sound\\field1.wav";
    std::wstring m_strField2 = L"res\\sound\\field2.wav";
    std::wstring m_strField3 = L"res\\sound\\field3.wav";

    void InvestigateCurrentStatus();
    std::wstring SelectBGM();

    int m_battleCounter = 0;

};

// 環境音のモデル
class BGMEnvModel
{
public:
    BGMEnvModel();
    void Update();
    void InvestigateBGMEnv();

    std::vector<envBGM> GetChangeRequest();
    void SetChangeRequestComplete();

    void SetEnding(const bool arg);
    void SetTrueEnd(const bool arg);

private:

    std::unordered_map<std::wstring, envBGM> m_envBgmMap;

    bool m_bTorch = false;
    bool m_bSea = false;
    bool m_bForest = false;
    bool m_bRain = false;
    bool m_bEnding = false;
    bool m_bTrueEnd = false;

    std::wstring m_strTorch = L"res\\sound\\torch.wav";
    std::wstring m_strSea = L"res\\sound\\sea.wav";
    std::wstring m_strForest = L"res\\sound\\forest.wav";
    std::wstring m_strRain = L"res\\sound\\rain.wav";
};

class BGM
{
public:
    void Init(HWND hwnd);
    void Finalize(); // For memory leak check.

    bool Load(const std::wstring& filename);
    void Play(const std::wstring& filename, const int a_volume = 100, const bool fadeIn = false);
    void Stop(const std::wstring& filename);

private:

    bool OpenWave(
        const std::wstring& filepath,
        WAVEFORMATEX& waveFormatEx,
        std::vector<char>* ppData,
        DWORD& dataSize);

    int PerToDecimal(const int percent);

    LPDIRECTSOUND8 dx8sound_ { nullptr };
    std::unordered_map<std::wstring, LPDIRECTSOUNDBUFFER8> dx8sound_buffers_ { };

//    std::thread* m_th1 = nullptr;
//    std::thread* m_th2 = nullptr;
    bool m_cancel1 = false;
    bool m_cancel2 = false;
};

class BGMManager
{
public:
    static BGMManager* Get();
    void Init(HWND hWnd);
    void Finalize();
    void Update();

    // エンディング中、とバトル中はBGMManagerのほうで知ることができないので外部から渡す。
    void SetEnding(const bool arg);
    void SetTrueEnd(const bool arg);
    void SetBattle(const bool arg);

    // 基本的にはBGMModelのほうでBGMを選曲する。
    // ただし、会話シーン中に特定のBGMを再生したいときはここで指定する。
    void SetTalkBGM(const std::wstring& filename);
    void FinalizeTalkBGM();

private:
    static BGMManager* m_obj;

    BGMManager();

    BGMModel m_BGMModel;
    BGMEnvModel m_BGMEnvModel;
    BGM m_BGM;
};

