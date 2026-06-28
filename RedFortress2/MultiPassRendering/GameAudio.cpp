#include "GameAudio.h"

#include "../../SoundLib/SoundLib/SoundLib.h"

namespace
{
const std::wstring kTitleBgm = L"res\\sound\\title.wav";
const std::wstring kEndingBgm = L"res\\sound\\ending.wav";
const std::wstring kBaseBgm = L"res\\sound\\kokeniwa.wav";
const std::wstring kStageSelectBgm = L"res\\sound\\stageselect1.wav";
const std::wstring kStageSelect2Bgm = L"res\\sound\\stageselect2.wav";
const std::wstring kWorld1Bgm = L"res\\sound\\world1.wav";
const std::wstring kField2Bgm = L"res\\sound\\field2.wav";
const std::wstring kField3Bgm = L"res\\sound\\field3.wav";
const std::wstring kForestEnvironment = L"res\\sound\\ENV_forest.wav";
const std::wstring kSeaEnvironment = L"res\\sound\\ENV_sea.wav";
const std::wstring kRainEnvironment = L"res\\sound\\ENV_rain.wav";
const std::wstring kMenuMove = L"res\\sound\\menu_cursor_move.wav";
const std::wstring kMenuConfirm = L"res\\sound\\menu_cursor_confirm.wav";
const std::wstring kMenuCancel = L"res\\sound\\menu_cursor_cancel.wav";
const std::wstring kPlayerAttack = L"res\\sound\\attack01.wav";
const std::wstring kSlashHit = L"res\\sound\\slashHit.wav";
const std::wstring kAttackHit = L"res\\sound\\enemyHanen.wav";
const std::wstring kEnemyAttack = L"res\\sound\\enemyAttack.wav";
const std::wstring kPlayerDamage = L"res\\sound\\damage01.wav";
const std::wstring kItemGet = L"res\\sound\\itemGet.wav";
const std::wstring kJump = L"res\\sound\\jump2.wav";
const std::wstring kPowerUp = L"res\\sound\\powerup.wav";
const std::wstring kDash = L"res\\sound\\dash.wav";
const std::wstring kExplosion = L"res\\sound\\explosion.wav";
const std::wstring kStomp = L"res\\sound\\stomp.wav";

std::wstring g_currentBgm;
std::wstring g_currentEnvironment;
int g_environmentId = -1;

void PlayBgmIfChanged(const std::wstring& path, const int volume)
{
    if (g_currentBgm == path)
    {
        return;
    }
    SoundLib::SoundLib::PlayBgm(path, volume);
    g_currentBgm = path;
}

void PlayEnvironmentIfChanged(const std::wstring& path, const int volume)
{
    if (g_currentEnvironment == path)
    {
        return;
    }
    if (g_environmentId >= 0)
    {
        SoundLib::SoundLib::StopEnvironmentSound(g_environmentId);
    }
    g_environmentId = SoundLib::SoundLib::PlayEnvironmentSound(path, volume);
    g_currentEnvironment = path;
}

void StopBgmIfPlaying()
{
    if (!g_currentBgm.empty())
    {
        SoundLib::SoundLib::StopBgm();
        g_currentBgm.clear();
    }
}

void StopEnvironment()
{
    if (g_environmentId >= 0)
    {
        SoundLib::SoundLib::StopEnvironmentSound(g_environmentId);
        g_environmentId = -1;
    }
    g_currentEnvironment.clear();
}

void PlayEffect(const std::wstring& path, const int volume)
{
    SoundLib::SoundLib::PlaySoundEffect(path, volume);
}
}

namespace GameAudio
{
void Initialize()
{
    const std::wstring effects[] =
    {
        kMenuMove, kMenuConfirm, kMenuCancel, kPlayerAttack, kSlashHit, kAttackHit,
        kEnemyAttack, kPlayerDamage, kItemGet, kJump, kPowerUp, kDash, kExplosion, kStomp
    };
    for (const std::wstring& effect : effects)
    {
        SoundLib::SoundLib::LoadSoundEffect(effect);
    }
    g_currentBgm.clear();
    g_currentEnvironment.clear();
    g_environmentId = -1;
}

void Finalize()
{
    StopEnvironment();
    StopBgmIfPlaying();
}

void PlayLoadingEnvironment()
{
    StopBgmIfPlaying();
    PlayEnvironmentIfChanged(kForestEnvironment, 14);
}

void PlayTitleMusic()
{
    StopEnvironment();
    PlayBgmIfChanged(kTitleBgm, 55);
}

void PlayEndingMusic()
{
    StopEnvironment();
    PlayBgmIfChanged(kEndingBgm, 60);
}

void UpdateStageMusic(const std::wstring& stageId, const int stageNumber, const bool useRainEnvironment)
{
    std::wstring fieldBgm = kWorld1Bgm;
    std::wstring environment = kForestEnvironment;
    int environmentVolume = 18;
    if (useRainEnvironment)
    {
        environment = kRainEnvironment;
        environmentVolume = 18;
    }

    if (stageId == L"select2")
    {
        fieldBgm = kStageSelect2Bgm;
        environmentVolume = 14;
    }
    else if (stageId.length() >= 6 && stageId.substr(0, 6) == L"select")
    {
        fieldBgm = kStageSelectBgm;
        environmentVolume = 14;
    }
    else if (stageId == L"base")
    {
        fieldBgm = kBaseBgm;
        environmentVolume = 14;
    }
    else if (stageNumber >= 9 && stageNumber <= 16)
    {
        fieldBgm = kField2Bgm;
        environmentVolume = 15;
    }
    else if (stageNumber >= 17)
    {
        fieldBgm = kField3Bgm;
        environmentVolume = 16;
    }
    PlayEnvironmentIfChanged(environment, environmentVolume);
    PlayBgmIfChanged(fieldBgm, 48);
}

void PlayMenuMove() { PlayEffect(kMenuMove, 70); }
void PlayMenuConfirm() { PlayEffect(kMenuConfirm, 78); }
void PlayMenuCancel() { PlayEffect(kMenuCancel, 72); }
void PlayPlayerAttack() { PlayEffect(kPlayerAttack, 82); }
void PlaySlashHit() { PlayEffect(kSlashHit, 82); }
void PlayAttackHit() { PlayEffect(kAttackHit, 82); }
void PlayEnemyAttack() { PlayEffect(kEnemyAttack, 72); }
void PlayPlayerDamage() { PlayEffect(kPlayerDamage, 88); }
void PlayItemGet() { PlayEffect(kItemGet, 82); }
void PlayJump() { PlayEffect(kJump, 62); }
void PlayPowerUp() { PlayEffect(kPowerUp, 82); }
void PlayDash() { PlayEffect(kDash, 72); }
void PlayExplosion() { PlayEffect(kExplosion, 75); }
void PlayStomp() { PlayEffect(kStomp, 82); }
}
