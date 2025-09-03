#pragma comment ( lib, "dxguid.lib")
#pragma comment ( lib, "dsound.lib")
#pragma comment ( lib, "winmm.lib")

#include "BGM.h"
#include "Common.h"

#include <mmsystem.h>
#include <thread>
#include "SharedObj.h"
#include "MainWindow.h"
#include "Rain.h"
#include "../../RedFortressModel/Model/PowereggDateTime.h"

BGMManager* BGMManager::m_obj = nullptr;

void BGM::Init(HWND hwnd)
{
    // Create sound device.
    LPDIRECTSOUND8 _dx8sound { nullptr };
    DirectSoundCreate8(nullptr, &_dx8sound, nullptr);
    SAFE_RELEASE(dx8sound_);
    dx8sound_ = _dx8sound;
    dx8sound_->SetCooperativeLevel(hwnd, DSSCL_PRIORITY);
}

void BGM::Finalize()
{
    for (auto& pair : dx8sound_buffers_)
    {
        SAFE_RELEASE(pair.second);
    }

    dx8sound_buffers_.clear();

    SAFE_RELEASE(dx8sound_);
}

// Reference
// http://marupeke296.com/DSSMP_No2_GetSoundFromWave.html
bool BGM::Load(const std::wstring& filename)
{
    // Already loaded.
    if (dx8sound_buffers_.find(filename) != dx8sound_buffers_.end())
    {
        return true;
    }

    // Open wave file.
    WAVEFORMATEX _waveformatex { };
    std::vector<char> _wave_data;
    DWORD _wave_size { 0 };
    if (!OpenWave(filename, _waveformatex, &_wave_data, _wave_size))
    {
        return false;
    }

    DSBUFFERDESC DSBufferDesc { };
    DSBufferDesc.dwSize = sizeof(DSBUFFERDESC);
    DSBufferDesc.dwFlags = DSBCAPS_CTRLVOLUME;
    DSBufferDesc.dwBufferBytes = _wave_size;
    DSBufferDesc.dwReserved = 0;
    DSBufferDesc.lpwfxFormat = &_waveformatex;
    DSBufferDesc.guid3DAlgorithm = GUID_NULL;

    // Create secondary buffer.
    LPDIRECTSOUNDBUFFER _temp_buffer { nullptr };
    LPDIRECTSOUNDBUFFER8 _temp_buffer8 { nullptr };
    dx8sound_->CreateSoundBuffer(&DSBufferDesc, &_temp_buffer, nullptr);
    _temp_buffer->QueryInterface(IID_IDirectSoundBuffer8, (void**)&_temp_buffer8);
    _temp_buffer->Release();
    if (_temp_buffer8 == 0)
    {
        return false;
    }
    LPDIRECTSOUNDBUFFER8 temp_shared_buff { _temp_buffer8 };
    dx8sound_buffers_[filename] = temp_shared_buff;

    // Write wave data to secondary buffer.
    LPVOID secondary_buffer { nullptr };
    DWORD _length { 0 };
    if (DS_OK == dx8sound_buffers_.at(filename)->Lock(
        0, 0, &secondary_buffer, &_length, nullptr, nullptr, DSBLOCK_ENTIREBUFFER))
    {
        memcpy(secondary_buffer, &_wave_data.at(0), _length);
        dx8sound_buffers_.at(filename)->Unlock(secondary_buffer, _length, nullptr, 0);
    }

    return true;
}

void BGM::Play(const std::wstring& filename, const int a_volume, const bool fadeIn)
{
    // Transform volume
    // 0 ~ 100 -> -10000 ~ 0
    int volume { 0 };
    volume = PerToDecimal(a_volume);
    dx8sound_buffers_.at(filename)->SetVolume(volume);
    dx8sound_buffers_.at(filename)->SetCurrentPosition(0);
    dx8sound_buffers_.at(filename)->Play(0, 0, DSBPLAY_LOOPING);

    if (fadeIn)
    {
        LPDIRECTSOUNDBUFFER8 soundBuffer = dx8sound_buffers_.at(filename);

//        if (m_th1 == nullptr)
//        {
//            m_cancel1 = false;
//            m_cancel2 = true;
//            m_th2 = nullptr;
//            m_th1 = new std::thread([=]
//                                    {
//                                        // 30回に分けて音量を0.1秒ごとに上げる
//                                        for (int i = 0; i < 30; ++i)
//                                        {
//                                            int volume2 = PerToDecimal(a_volume * i / 30);
//                                            soundBuffer->SetVolume(volume2);
//                                            Sleep(100);
//                                            if (this->m_cancel1)
//                                            {
//                                                break;
//                                            }
//                                        }
//                                    });
//        }
//        else if (m_th2 == nullptr)
//        {
//            m_cancel1 = true;
//            m_cancel2 = false;
//            m_th1 = nullptr;
//            m_th2 = new std::thread([=]
//                                    {
//                                        // 100回に分けて音量を0.1秒ごとに上げる
//                                        for (int i = 0; i < 30; ++i)
//                                        {
//                                            int volume2 = PerToDecimal(a_volume * i / 30);
//                                            soundBuffer->SetVolume(volume2);
//                                            Sleep(100);
//                                            if (this->m_cancel2)
//                                            {
//                                                break;
//                                            }
//                                        }
//                                    });
//        }
    }
}

void BGM::Stop(const std::wstring& filename)
{
    dx8sound_buffers_.at(filename)->Stop();
}

bool BGM::OpenWave(const std::wstring& filepath,
                    WAVEFORMATEX& waveformatex,
                    std::vector<char>* buff,
                    DWORD& wave_size)
{
    if (filepath.empty())
    {
        return false;
    }

    HMMIO _hmmio { nullptr };

    wchar_t* Name = NEW wchar_t[filepath.length() + 1];
    wcscpy_s(Name, filepath.length() + 1, filepath.c_str());
    _hmmio = mmioOpen(Name, nullptr, MMIO_READ);

    delete[] Name;
    if (_hmmio == nullptr)
    {
        return false;
    }

    // Search RIFF chunk.
    MMRESULT mmresult { 0 };
    MMCKINFO riff_chunk { };
    riff_chunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    mmresult = mmioDescend(_hmmio, &riff_chunk, NULL, MMIO_FINDRIFF);
    if (mmresult != MMSYSERR_NOERROR)
    {
        mmioClose(_hmmio, 0);
        return false;
    }

    // Search format chunk
    MMCKINFO format_chunk { };
    format_chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    mmresult = mmioDescend(_hmmio, &format_chunk, &riff_chunk, MMIO_FINDCHUNK);
    if (mmresult != MMSYSERR_NOERROR)
    {
        mmioClose(_hmmio, 0);
        return false;
    }
    DWORD fmsize { format_chunk.cksize };
    DWORD size { (DWORD)mmioRead(_hmmio, (HPSTR)&waveformatex, fmsize) };
    if (size != fmsize)
    {
        mmioClose(_hmmio, 0);
        return false;
    }

    mmioAscend(_hmmio, &format_chunk, 0);

    // Search data chunk.
    MMCKINFO data_chunk { };
    data_chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    mmresult = mmioDescend(_hmmio, &data_chunk, &riff_chunk, MMIO_FINDCHUNK);
    if (mmresult != MMSYSERR_NOERROR)
    {
        mmioClose(_hmmio, 0);
        return false;
    }
    buff->resize(data_chunk.cksize);
    size = mmioRead(_hmmio, (HPSTR)buff->data(), data_chunk.cksize);
    if (size != data_chunk.cksize)
    {
        return false;
    }
    wave_size = size;

    mmioClose(_hmmio, 0);

    return true;
}

// convert percent to decibel.
// https://katze.hatenablog.jp/entry/2013/07/01/000343
int BGM::PerToDecimal(int percent)
{
    int decibel { 0 };

    // 0 ` -600
    if (percent >= 50)
    {
        decibel = -600 + 600 * (percent - 50) / 50;
    }
    // -600 ` -1200
    else if (percent >= 25)
    {
        decibel = -1200 + 600 * (percent - 25) / 25;
    }
    // -1200 ` -1800
    else if (percent >= 12)
    {
        decibel = -1800 + 600 * (percent - 12) / 12;
    }
    // -1800 ` -2400
    else if (percent >= 6)
    {
        decibel = -2400 + 600 * (percent - 6) / 6;
    }
    // -2400 ` -3000
    else if (percent >= 3)
    {
        decibel = -3000 + 600 * (percent - 3) / 3;
    }
    // -3000`-3600
    else if (percent >= 1)
    {
        decibel = -3600 + 600 * (percent - 1);
    }
    else
    {
        decibel = DSBVOLUME_MIN;
    }
    return decibel;
}

BGMModel::BGMModel()
{
    srand((unsigned int)NULL);
}

// 1秒に1回呼ばれる想定
void BGMModel::Update()
{
    InvestigateCurrentStatus();

    // BGMの選曲
    std::wstring newBGM =  SelectBGM();

    if (newBGM.empty())
    {
        return;
    }
    else
    {
        m_stBGMPrev = m_stBGM;
        m_stBGM = stBGM { };
        m_stBGM.m_filename = newBGM;
        m_stBGM.m_eBGMStatus = eBGMStatus::START_REQUEST;
        m_stBGM.m_volume = 10;
    }
}

bool BGMModel::GetChangeRequest(stBGM* stBGM1, stBGM* stBGM2)
{
    bool bResult = false;
    if (m_stBGM.m_eBGMStatus == eBGMStatus::START_REQUEST)
    {
        *stBGM1 = m_stBGM;
        *stBGM2 = m_stBGMPrev;
        bResult = true;
    }

    return bResult;
}

void BGMModel::SetChangeRequestComplete()
{
    if (m_stBGM.m_eBGMStatus == eBGMStatus::START_REQUEST)
    {
        m_stBGM.m_eBGMStatus = eBGMStatus::STARTED;
        m_stBGMPrev.m_eBGMStatus = eBGMStatus::STOPPED;
    }
    else if (m_stBGM.m_eBGMStatus == eBGMStatus::STOP_REQUEST)
    {
        m_stBGM.m_eBGMStatus = eBGMStatus::STOPPED;
    }
}

void BGMModel::InvestigateCurrentStatus()
{
    //------------------------------------------
    // 死亡
    //------------------------------------------
    if (Common::Status()->GetDead())
    {
        m_bDead = true;
    }
    else
    {
        m_bDead = false;
    }

    auto seq = MainWindow::GetBattleSequence();
    if (seq != nullptr)
    {
        //------------------------------------------
        // タイトル
        //------------------------------------------
        if (seq->GetState() == eBattleState::TITLE ||
            seq->GetState() == eBattleState::LOAD)
        {
            m_bTitle = true;
        }
        else
        {
            m_bTitle = false;
        }

        //------------------------------------------
        // オープニング
        //------------------------------------------
        if (seq->GetState() == eBattleState::OPENING)
        {
            m_bOpening = true;
        }
        else
        {
            m_bOpening = false;
        }
    }

    //------------------------------------------
    // エンディング
    //------------------------------------------
    // 外部から渡す

    //------------------------------------------
    // 戦闘
    //------------------------------------------
    // 外部から渡す
    // 攻撃したらバトル開始
    // 20秒攻撃しなかったらバトル終了

    //------------------------------------------
    // 瀕死
    //------------------------------------------
    if (Common::Status()->GetMuscleCurrent() < 10.f ||
        Common::Status()->GetWaterCurrent() < 92.f ||
        (Common::Status()->GetCarboCurrent() < 10.f && Common::Status()->GetLipidCurrent() < 10.f))
    {
        m_bDying = true;
    }
    else
    {
        m_bDying = false;
    }

    //------------------------------------------
    // 体力少な目
    //------------------------------------------
    if (Common::Status()->GetMuscleCurrent() < 50.f ||
        Common::Status()->GetWaterCurrent() < 95.f ||
        (Common::Status()->GetCarboCurrent() < 50.f && Common::Status()->GetLipidCurrent() < 10.f))
    {
        m_bWeak = true;
    }
    else
    {
        m_bWeak = false;
    }

    //------------------------------------------
    // 夜
    //------------------------------------------
    auto datetime = NSModel::PowereggDateTime::GetObj();

    if (6 < datetime->GetHour() && datetime->GetHour() < 18)
    {
        m_bNight = false;
    }
    else
    {
        m_bNight = true;
    }

    //------------------------------------------
    // 航海
    //------------------------------------------
    auto voyage = VoyageManager::Get();
    if (voyage->GetRaftMode())
    {
        m_bVoyage = true;
    }
    else
    {
        m_bVoyage = false;
    }

    //------------------------------------------
    // 廃墟の近くにいる
    //------------------------------------------
    {
        if (SharedObj::GetPlayer() != nullptr)
        {
            auto ppos = SharedObj::GetPlayer()->GetPos();

            D3DXVECTOR3 targetPos{ 10.6f, 491.5f, -564.f };
            auto nearHaikyo = Common::HitByBoundingBox(ppos, targetPos, 100.f);
            if (nearHaikyo)
            {
                m_bHaikyo = true;
            }
            else
            {
                m_bHaikyo = false;
            }
        }
    }

    //------------------------------------------
    // 灯台の近くにいる
    //------------------------------------------
    {
        if (SharedObj::GetPlayer() != nullptr)
        {
            auto ppos = SharedObj::GetPlayer()->GetPos();

            D3DXVECTOR3 targetPos{ -1321.f, 632.f, -1529.f };
            auto nearTarget = Common::HitByBoundingBox(ppos, targetPos, 100.f);
            if (nearTarget)
            {
                m_bToudai = true;
            }
            else
            {
                m_bToudai = false;
            }
        }
    }

    //------------------------------------------
    // 海岸洞窟の近くにいる
    //------------------------------------------
    {
        if (SharedObj::GetPlayer() != nullptr)
        {
            auto ppos = SharedObj::GetPlayer()->GetPos();

            D3DXVECTOR3 targetPos{ 1234.0f, 16.8f, 530.1f };
            auto nearTarget = Common::HitByBoundingBox(ppos, targetPos, 300.f);
            if (nearTarget)
            {
                m_bKaiganDoukutsu = true;
            }
            else
            {
                m_bKaiganDoukutsu = false;
            }
        }
    }

    //------------------------------------------
    // 神社の近くにいる
    //------------------------------------------
    {
        if (SharedObj::GetPlayer() != nullptr)
        {
            auto ppos = SharedObj::GetPlayer()->GetPos();

            D3DXVECTOR3 targetPos{ 15.8f, 492.8f, -1221.2f };
            auto nearTarget = Common::HitByBoundingBox(ppos, targetPos, 100.f);
            if (nearTarget)
            {
                m_bJinja = true;
            }
            else
            {
                m_bJinja = false;
            }
        }
    }

    //------------------------------------------
    // 苔庭の近くにいる
    //------------------------------------------
    {
        if (SharedObj::GetPlayer() != nullptr)
        {
            auto ppos = SharedObj::GetPlayer()->GetPos();

            D3DXVECTOR3 targetPos{ 629.f, 773.9f, -1861.8f };
            auto nearTarget = Common::HitByBoundingBox(ppos, targetPos, 100.f);
            if (nearTarget)
            {
                m_bKokeniwa = true;
            }
            else
            {
                m_bKokeniwa = false;
            }
        }
    }

    //------------------------------------------
    // 港跡の近くにいる
    //------------------------------------------
    {
        if (SharedObj::GetPlayer() != nullptr)
        {
            auto ppos = SharedObj::GetPlayer()->GetPos();

            D3DXVECTOR3 targetPos{ 1814.3f, 28.6f, -899.2f };
            auto nearTarget = Common::HitByBoundingBox(ppos, targetPos, 100.f);
            if (nearTarget)
            {
                m_bMinatoAto = true;
            }
            else
            {
                m_bMinatoAto = false;
            }
        }
    }

    //------------------------------------------
    // 洞窟の近くにいる
    //------------------------------------------
    {
        if (SharedObj::GetPlayer() != nullptr)
        {
            auto inCave = SharedObj::GetPlayer()->IsInCave();
            if (inCave)
            {
                m_bDoukutsu = true;
            }
            else
            {
                m_bDoukutsu = false;
            }
        }
    }
}

// 変更ナシなら空文字
// 
// bgmの選曲
// 上のルールほど優先度が高い
//
// 会話シーンで特定のbgmが指定されている場合はそのbgmを再生する。
// 死亡していたら死亡時のbgm
// タイトルだったらタイトルのbgm
// オープニングだったらオープニングのbgm
// エンディングだったらエンディングのbgm
// 戦闘だったら戦闘のbgm
// 瀕死だったら瀕死のbgm
// 体力が少ないなら体力が少ないbgm
// 夜だったら夜のbgm
// 航海中だったら航海中のbgm
// 特定の地域なら特定のbgm
// それ以外ならランダム再生
//
// 基本的にbgmmodel内で判定をする。外からじゃないと設定できない時もある。
// そのときはそのときだ。
std::wstring BGMModel::SelectBGM()
{
    std::wstring newBGM;

    // 複数のフラグがONになることがあることに注意

    if (!m_strTalkBGM.empty())
    {
        newBGM = m_strTalkBGM;
    }
    else if (m_bDead)
    {
        newBGM = m_strDead;
    }
    else if (m_bTitle)
    {
        newBGM = m_strTitle;
    }
    else if (m_bOpening)
    {
        newBGM = m_strOpening;
    }
    else if (m_bEnding)
    {
        newBGM = m_strEnding;
    }
    else if (m_bTrueEnd)
    {
        newBGM = m_strTrueEnd;
    }
    else if (m_bBattle)
    {
        m_battleCounter--;
        if (m_battleCounter <= 0)
        {
            m_bBattle = false;
        }

        // 戦闘曲じゃないところから戦闘曲になったら
        // 戦闘曲１か戦闘曲２のどちらかを選曲する
        if (m_stBGM.m_filename == m_strBattle1 || m_stBGM.m_filename == m_strBattle2)
        {
            // Do nothing
        }
        else
        {
            auto rand_ = SharedObj::GetRandom() % 2;
            if (rand_ == 0)
            {
                newBGM = m_strBattle1;
            }
            else
            {
                newBGM = m_strBattle2;
            }
        }
    }
    else if (m_bDying)
    {
        newBGM = m_strDying;
    }
    else if (m_bWeak)
    {
        newBGM = m_strWeak;
    }
    else if (m_bToudai)
    {
        newBGM = m_strToudai;
    }
    else if (m_bDoukutsu)
    {
        newBGM = m_strDoukutsu;
    }
    else if (m_bNight)
    {
        newBGM = m_strNight;
    }
    else if (m_bVoyage)
    {
        newBGM = m_strVoyage;
    }
    else if (m_bHaikyo)
    {
        newBGM = m_strHaikyo;
    }
    else if (m_bKaiganDoukutsu)
    {
        newBGM = m_strKaiganDoukutsu;
    }
    else if (m_bJinja)
    {
        newBGM = m_strJinja;
    }
    else if (m_bKokeniwa)
    {
        newBGM = m_strKokeniwa;
    }
    else if (m_bMinatoAto)
    {
        newBGM = m_strMinatoAto;
    }
    // どれでもないなら通常曲をランダムで選曲
    else
    {
        // 通常曲じゃないところから通常曲になったら
        // 通常曲の中からどれかを選曲する
        if (m_stBGM.m_filename == m_strField1 ||
            m_stBGM.m_filename == m_strField2 ||
            m_stBGM.m_filename == m_strField3)
        {
            // Do nothing
        }
        else
        {
            auto rand_ = SharedObj::GetRandom() % 3;
            if (rand_ == 0)
            {
                newBGM = m_strField1;
            }
            else if (rand_ == 1)
            {
                newBGM = m_strField2;
            }
            else
            {
                newBGM = m_strField3;
            }
        }
    }

    if (m_stBGM.m_filename == newBGM)
    {
        return std::wstring();
    }

    return newBGM;
}

BGMEnvModel::BGMEnvModel()
{
    envBGM work;
    work.m_eBGMStatus = eBGMStatus::NONE;

    work.m_volume = 20;
    work.m_filename = m_strTorch;
    m_envBgmMap[m_strTorch] = work;

    work.m_volume = 20;
    work.m_filename = m_strSea;
    m_envBgmMap[m_strSea] = work;

    work.m_volume = 60;
    work.m_filename = m_strForest;
    m_envBgmMap[m_strForest] = work;

    work.m_volume = 20;
    work.m_filename = m_strRain;
    m_envBgmMap[m_strRain] = work;
}

void BGMEnvModel::Update()
{
    InvestigateBGMEnv();

}

void BGMEnvModel::InvestigateBGMEnv()
{
    // プレイヤーの標高が低かったら海の環境音
    // プレイヤーの標高が高かったら森の環境音
    // 松明持ってたら松明の環境音
    // 雨が降っていたら雨の環境音

    // 環境音は複数が同時になっていても問題ない。

    if (SharedObj::GetPlayer() != nullptr)
    {
        auto ppos = SharedObj::GetPlayer()->GetPos();
        if (ppos.y < 20.f)
        {
            m_bSea = true;
            m_bForest = false;
        }
        else
        {
            m_bSea = false;
            m_bForest = true;
        }
    }

    auto lit = NSModel::WeaponManager::GetObj()->IsTorchLit();
    if (lit)
    {
        m_bTorch = true;
    }
    else
    {
        m_bTorch = false;
    }

    if (Rain::Get()->IsRain())
    {
        m_bRain = true;
    }
    else
    {
        m_bRain = false;
    }

    if (m_bTrueEnd || m_bEnding)
    {
        m_bSea = false;
        m_bForest = false;
        m_bTorch = false;
        m_bRain = false;
    }

    // 松明の環境音
    // 松明ついてるのに、松明の環境音が鳴っていなかったら再生リクエスト
    if (m_bTorch &&
        (m_envBgmMap.at(m_strTorch).m_eBGMStatus == eBGMStatus::NONE ||
         m_envBgmMap.at(m_strTorch).m_eBGMStatus == eBGMStatus::STOPPED))
    {
        m_envBgmMap.at(m_strTorch).m_eBGMStatus = eBGMStatus::START_REQUEST;
    }
    // 松明ついてないのに、松明の環境音が鳴っていたら停止リクエスト
    else if (!m_bTorch &&
             m_envBgmMap.at(m_strTorch).m_eBGMStatus == eBGMStatus::STARTED)
    {
        m_envBgmMap.at(m_strTorch).m_eBGMStatus = eBGMStatus::STOP_REQUEST;
    }

    // 海の環境音
    if (m_bSea &&
        (m_envBgmMap.at(m_strSea).m_eBGMStatus == eBGMStatus::NONE ||
         m_envBgmMap.at(m_strSea).m_eBGMStatus == eBGMStatus::STOPPED))
    {
        m_envBgmMap.at(m_strSea).m_eBGMStatus = eBGMStatus::START_REQUEST;
    }
    else if (!m_bSea &&
             m_envBgmMap.at(m_strSea).m_eBGMStatus == eBGMStatus::STARTED)
    {
        m_envBgmMap.at(m_strSea).m_eBGMStatus = eBGMStatus::STOP_REQUEST;
    }

    // 森の環境音
    if (m_bForest &&
        (m_envBgmMap.at(m_strForest).m_eBGMStatus == eBGMStatus::NONE ||
         m_envBgmMap.at(m_strForest).m_eBGMStatus == eBGMStatus::STOPPED))
    {
        m_envBgmMap.at(m_strForest).m_eBGMStatus = eBGMStatus::START_REQUEST;
    }
    else if (!m_bForest &&
             m_envBgmMap.at(m_strForest).m_eBGMStatus == eBGMStatus::STARTED)
    {
        m_envBgmMap.at(m_strForest).m_eBGMStatus = eBGMStatus::STOP_REQUEST;
    }

    // 雨の環境音
    if (m_bRain &&
        (m_envBgmMap.at(m_strRain).m_eBGMStatus == eBGMStatus::NONE ||
         m_envBgmMap.at(m_strRain).m_eBGMStatus == eBGMStatus::STOPPED))
    {
        m_envBgmMap.at(m_strRain).m_eBGMStatus = eBGMStatus::START_REQUEST;
    }
    else if (!m_bRain &&
             m_envBgmMap.at(m_strRain).m_eBGMStatus == eBGMStatus::STARTED)
    {
        m_envBgmMap.at(m_strRain).m_eBGMStatus = eBGMStatus::STOP_REQUEST;
    }
}

std::vector<envBGM> BGMEnvModel::GetChangeRequest()
{
    std::vector<envBGM> requests;

    for (auto& BGMEnv : m_envBgmMap)
    {
        if (BGMEnv.second.m_eBGMStatus == eBGMStatus::START_REQUEST ||
            BGMEnv.second.m_eBGMStatus == eBGMStatus::STOP_REQUEST)
        {
            requests.push_back(BGMEnv.second);
        }
    }

    return requests;
}

void BGMEnvModel::SetChangeRequestComplete()
{
    for (auto& BGMEnv : m_envBgmMap)
    {
        if (BGMEnv.second.m_eBGMStatus == eBGMStatus::START_REQUEST)
        {
            BGMEnv.second.m_eBGMStatus = eBGMStatus::STARTED;
        }
        else if (BGMEnv.second.m_eBGMStatus == eBGMStatus::STOP_REQUEST)
        {
            BGMEnv.second.m_eBGMStatus = eBGMStatus::STOPPED;
        }
    }
}

void BGMEnvModel::SetTrueEnd(const bool arg)
{
    m_bTrueEnd = arg;
}

void BGMEnvModel::SetEnding(const bool arg)
{
    m_bEnding = arg;
}

BGMManager* BGMManager::Get()
{
    if (m_obj == nullptr)
    {
        m_obj = NEW BGMManager();
    }

    return m_obj;
}

BGMManager::BGMManager()
{
}

void BGMManager::Init(HWND hWnd)
{
    m_BGM.Init(hWnd);
}

void BGMManager::Finalize()
{
    m_BGM.Finalize();

    SAFE_DELETE(m_obj);
}

void BGMManager::Update()
{
    // BGM
    {
        m_BGMModel.Update();

        stBGM currentBGM;
        stBGM prevBGM;

        bool bRequest = m_BGMModel.GetChangeRequest(&currentBGM, &prevBGM);

        if (bRequest)
        {
            if (currentBGM.m_eBGMStatus == eBGMStatus::START_REQUEST)
            {
                if (prevBGM.m_eBGMStatus == eBGMStatus::STARTED)
                {
                    m_BGM.Stop(prevBGM.m_filename);
                }

                m_BGM.Load(currentBGM.m_filename);
                m_BGM.Play(currentBGM.m_filename, currentBGM.m_volume, false);

                m_BGMModel.SetChangeRequestComplete();
            }
        }
    }

    // 環境音
    {
        m_BGMEnvModel.Update();
        auto requests = m_BGMEnvModel.GetChangeRequest();

        for (auto& request : requests)
        {
            if (request.m_eBGMStatus == eBGMStatus::START_REQUEST)
            {
                m_BGM.Load(request.m_filename);
                m_BGM.Play(request.m_filename, request.m_volume, false);
            }
            else if (request.m_eBGMStatus == eBGMStatus::STOP_REQUEST)
            {
                m_BGM.Stop(request.m_filename);
            }
        }

        m_BGMEnvModel.SetChangeRequestComplete();
    }
}

void BGMModel::SetEnding(const bool arg)
{
    m_bEnding = arg;
}

void BGMModel::SetTrueEnd(const bool arg)
{
    m_bTrueEnd = arg;
}

void BGMModel::SetBattle(const bool arg)
{
    m_bBattle = arg;

    if (arg)
    {
        m_battleCounter = 20;
    }
}

void BGMModel::SetTalkBGM(const std::wstring& filename)
{
    m_strTalkBGM = filename;
}

void BGMModel::FinalizeTalkBGM()
{
    m_strTalkBGM.clear();
}

void BGMManager::SetEnding(const bool arg)
{
    m_BGMModel.SetEnding(arg);
    m_BGMEnvModel.SetEnding(arg);
}

void BGMManager::SetTrueEnd(const bool arg)
{
    m_BGMModel.SetTrueEnd(arg);
    m_BGMEnvModel.SetTrueEnd(arg);
}

// 攻撃が当たったらバトル開始
// ２０秒攻撃が当たらなかったらバトル終了
void BGMManager::SetBattle(const bool arg)
{
    m_BGMModel.SetBattle(arg);
}

void BGMManager::SetTalkBGM(const std::wstring& filename)
{
    m_BGMModel.SetTalkBGM(filename);
}

void BGMManager::FinalizeTalkBGM()
{
    m_BGMModel.FinalizeTalkBGM();
}



