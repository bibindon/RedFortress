#include "GameAudio.h"

#include "../../SoundLib/SoundLib/SoundLib.h"

namespace
{
const std::wstring kTitleBgm = L"res\\sound\\title2.wav";
const std::wstring kLoadingBgm = L"res\\sound\\loading.wav";
const std::wstring kEndingBgm = L"res\\sound\\ending.wav";
const std::wstring kBaseBgm = L"res\\sound\\kokeniwa.wav";
const std::wstring kKaiganDoukutsuBgm = L"res\\sound\\kaiganDoukutsu.wav";
const std::wstring kNightBgm = L"res\\sound\\night.wav";
const std::wstring kStoryBgm = L"res\\sound\\bgm_story.wav";
const std::wstring kStageSelect1Bgm = L"res\\sound\\bgm_select1.wav";
const std::wstring kStageSelect2Bgm = L"res\\sound\\stageselect2.wav";
const std::wstring kStageSelect3Bgm = L"res\\sound\\bgm_select3_ronri.wav";
const std::wstring kStageSelect4Bgm = L"res\\sound\\bgm_select2.wav";
const std::wstring kW1FieldBgm = L"res\\sound\\bgm_w1_field.wav";
const std::wstring kW1SwampBgm = L"res\\sound\\bgm_w1_swamp.wav";
const std::wstring kW2CaveBgm = L"res\\sound\\bgm_w2_cave.wav";
const std::wstring kW2MineBgm = L"res\\sound\\bgm_w2_mine.wav";
const std::wstring kW3RuinsBgm = L"res\\sound\\bgm_w3_ruins.wav";
const std::wstring kW3TrailBgm = L"res\\sound\\bgm_w3_trail.wav";
const std::wstring kW4FortressBgm = L"res\\sound\\bgm_w4_fortress.wav";
const std::wstring kW4AssaultBgm = L"res\\sound\\bgm_w4_assault.wav";
const std::wstring kBossBgm = L"res\\sound\\bgm_boss_crazyhill.wav";
const std::wstring kBoss2Bgm = L"res\\sound\\bgm_boss2.wav";
const std::wstring kLastBossBgm = L"res\\sound\\bgm_lastboss.wav";
const std::wstring kForestEnvironment = L"res\\sound\\ENV_forest.wav";
const std::wstring kSeaEnvironment = L"res\\sound\\ENV_sea.wav";
const std::wstring kRainEnvironment = L"res\\sound\\ENV_rain.wav";
const std::wstring kMenuMove = L"res\\sound\\menu_cursor_move.wav";
const std::wstring kMenuConfirm = L"res\\sound\\menu_cursor_confirm.wav";
const std::wstring kMenuCancel = L"res\\sound\\menu_cursor_cancel.wav";
const std::wstring kSaveComplete = L"res\\sound\\save_complete.wav";
const std::wstring kStageSelectMove = L"res\\sound\\cursor_move.wav";
const std::wstring kStageSelectConfirm = L"res\\sound\\cursor_confirm.wav";
const std::wstring kPlayerAttack = L"res\\sound\\attack01.wav";
const std::wstring kSwordSwing = L"res\\sound\\sword_swing.wav";
const std::wstring kSlashHit = L"res\\sound\\slashHit.wav";
const std::wstring kLeverToggle = L"res\\sound\\pullOar.wav";
const std::wstring kRopeCut = L"res\\sound\\slashHit.wav";
const std::wstring kMechanismStop = L"res\\sound\\stomp_impact.wav";
const std::wstring kAttackHit = L"res\\sound\\club_hit.wav";
const std::wstring kBusterHit = L"res\\sound\\buster_hit.wav";
const std::wstring kEnemyAttack = L"res\\sound\\enemyAttack.wav";
const std::wstring kPlayerDamage = L"res\\sound\\damage01.wav";
const std::wstring kPlayerDeath = L"res\\sound\\death.wav";
const std::wstring kItemGet = L"res\\sound\\itemGet.wav";
const std::wstring kAmmoMax = L"res\\sound\\ammoMax.wav";
const std::wstring kAmmoGet = L"res\\sound\\ammoGet.wav";
const std::wstring kJump = L"res\\sound\\jump_action.wav";
const std::wstring kPowerUp = L"res\\sound\\powerup.wav";
const std::wstring kDrink = L"res\\sound\\drink.wav";
const std::wstring kHyperMode = L"res\\sound\\hyperMode.wav";
const std::wstring kDash = L"res\\sound\\dash.wav";
const std::wstring kDashBooster = L"res\\sound\\dashBooster2.wav";
const std::wstring kExplosion = L"res\\sound\\explosion.wav";
const std::wstring kBombPlace = L"res\\sound\\bombDrop.wav";
const std::wstring kStomp = L"res\\sound\\stomp_impact.wav";
const std::wstring kBuster = L"res\\sound\\buster.wav";
const std::wstring kWeaponChange = L"res\\sound\\weaponChange.wav";
const std::wstring kQte = L"res\\sound\\qte.wav";
const std::wstring kQteBest = L"res\\sound\\qte_best.wav";
const std::wstring kArrow = L"res\\sound\\arrow.wav";
const std::wstring kWarp = L"res\\sound\\warp.wav";
const int kTitleBgmVolume = 22;
const int kEndingBgmVolume = 50;
const int kFieldBgmVolume = 40;
const int kStoryBgmVolume = 40;

std::wstring g_currentBgm;
std::wstring g_currentEnvironment;
int g_environmentId = -1;
int g_hyperModeId = -1;
int g_currentBgmVolume = 0;
int g_effectiveBgmVolume = -1;
bool g_initialized = false;
bool g_bgmFadeOutActive = false;
int g_bgmFadeOutFramesRemaining = 0;
int g_bgmFadeOutTotalFrames = 0;
bool g_recoveryPending = false;
bool g_hyperModeRequested = false;
ULONGLONG g_nextRecoveryTick = 0;
const ULONGLONG kRecoveryRetryIntervalMilliseconds = 500;

void ResetTrackingState()
{
    g_currentBgm.clear();
    g_currentEnvironment.clear();
    g_environmentId = -1;
    g_hyperModeId = -1;
    g_currentBgmVolume = 0;
    g_effectiveBgmVolume = -1;
    g_bgmFadeOutActive = false;
    g_bgmFadeOutFramesRemaining = 0;
    g_bgmFadeOutTotalFrames = 0;
}

void BeginAudioDeviceRecovery()
{
    g_initialized = false;
    ResetTrackingState();
    SoundLib::SoundLib::Finalize();
    g_recoveryPending = true;
    g_nextRecoveryTick = GetTickCount64() + kRecoveryRetryIntervalMilliseconds;
}

int GetEffectiveBgmVolume(const int volume)
{
    if (g_hyperModeId >= 0)
    {
        return 0;
    }

    return volume;
}

void ApplyCurrentBgmVolume()
{
    if (!g_initialized)
    {
        return;
    }

    if (g_currentBgm.empty())
    {
        g_effectiveBgmVolume = -1;
        return;
    }

    const int effectiveVolume = GetEffectiveBgmVolume(g_currentBgmVolume);
    if (g_effectiveBgmVolume == effectiveVolume)
    {
        return;
    }

    try
    {
        SoundLib::SoundLib::SetBgmVolume(effectiveVolume);
    }
    catch (const SoundLib::AudioDeviceException&)
    {
        BeginAudioDeviceRecovery();
        return;
    }
    g_effectiveBgmVolume = effectiveVolume;
}

void PlayBgmIfChanged(const std::wstring& path, const int volume)
{
    if (!g_initialized)
    {
        return;
    }

    if (g_currentBgm == path)
    {
        g_currentBgmVolume = volume;
        g_bgmFadeOutActive = false;
        g_bgmFadeOutFramesRemaining = 0;
        g_bgmFadeOutTotalFrames = 0;
        ApplyCurrentBgmVolume();
        return;
    }

    const int effectiveVolume = GetEffectiveBgmVolume(volume);
    try
    {
        SoundLib::SoundLib::PlayBgm(path, effectiveVolume);
    }
    catch (const SoundLib::AudioDeviceException&)
    {
        BeginAudioDeviceRecovery();
        return;
    }
    g_currentBgm = path;
    g_currentBgmVolume = volume;
    g_effectiveBgmVolume = effectiveVolume;
    g_bgmFadeOutActive = false;
    g_bgmFadeOutFramesRemaining = 0;
    g_bgmFadeOutTotalFrames = 0;
}

void PlayEnvironmentIfChanged(const std::wstring& path, const int volume)
{
    if (!g_initialized)
    {
        return;
    }

    if (g_currentEnvironment == path)
    {
        return;
    }
    try
    {
        if (g_environmentId >= 0)
        {
            SoundLib::SoundLib::StopEnvironmentSound(g_environmentId);
        }
        g_environmentId = SoundLib::SoundLib::PlayEnvironmentSound(path, volume);
    }
    catch (const SoundLib::AudioDeviceException&)
    {
        BeginAudioDeviceRecovery();
        return;
    }
    g_currentEnvironment = path;
}

void StopBgmIfPlaying()
{
    g_bgmFadeOutActive = false;
    g_bgmFadeOutFramesRemaining = 0;
    g_bgmFadeOutTotalFrames = 0;
    if (!g_initialized)
    {
        return;
    }

    if (!g_currentBgm.empty())
    {
        try
        {
            SoundLib::SoundLib::StopBgm();
        }
        catch (const SoundLib::AudioDeviceException&)
        {
            BeginAudioDeviceRecovery();
            return;
        }
        g_currentBgm.clear();
        g_currentBgmVolume = 0;
        g_effectiveBgmVolume = -1;
    }
}

void StopEnvironment()
{
    if (!g_initialized)
    {
        return;
    }

    if (g_environmentId >= 0)
    {
        try
        {
            SoundLib::SoundLib::StopEnvironmentSound(g_environmentId);
        }
        catch (const SoundLib::AudioDeviceException&)
        {
            BeginAudioDeviceRecovery();
            return;
        }
        g_environmentId = -1;
    }
    g_currentEnvironment.clear();
}

void BeginBgmFadeOutInternal(const int frames)
{
    if (!g_initialized || g_currentBgm.empty())
    {
        return;
    }

    if (frames <= 0)
    {
        StopBgmIfPlaying();
        return;
    }

    g_bgmFadeOutActive = true;
    g_bgmFadeOutFramesRemaining = frames;
    g_bgmFadeOutTotalFrames = frames;
}

void UpdateBgmFadeOutInternal()
{
    if (!g_bgmFadeOutActive)
    {
        return;
    }

    if (!g_initialized || g_currentBgm.empty())
    {
        g_bgmFadeOutActive = false;
        return;
    }

    if (g_bgmFadeOutFramesRemaining > 0)
    {
        --g_bgmFadeOutFramesRemaining;
    }

    const int volume = g_currentBgmVolume * g_bgmFadeOutFramesRemaining /
                       g_bgmFadeOutTotalFrames;
    try
    {
        SoundLib::SoundLib::SetBgmVolume(volume);
    }
    catch (const SoundLib::AudioDeviceException&)
    {
        BeginAudioDeviceRecovery();
        return;
    }
    g_effectiveBgmVolume = volume;

    if (g_bgmFadeOutFramesRemaining <= 0)
    {
        StopBgmIfPlaying();
    }
}

void PlayEffect(const std::wstring& path, const int volume)
{
    if (!g_initialized)
    {
        return;
    }

    try
    {
        SoundLib::SoundLib::PlaySoundEffect(path, volume);
    }
    catch (const SoundLib::AudioDeviceException&)
    {
        BeginAudioDeviceRecovery();
    }
}
}

namespace GameAudio
{
void Initialize()
{
    g_initialized = false;
    ResetTrackingState();
    const std::wstring effects[] =
    {
        kMenuMove, kMenuConfirm, kMenuCancel, kSaveComplete, kPlayerAttack, kSwordSwing, kSlashHit, kAttackHit, kBusterHit,
        kEnemyAttack, kPlayerDamage, kPlayerDeath, kItemGet, kAmmoGet, kAmmoMax, kJump, kPowerUp, kDash, kDashBooster,
        kExplosion, kBombPlace, kStomp, kLeverToggle, kBuster, kWeaponChange, kStageSelectMove, kStageSelectConfirm,
        kDrink, kQte, kQteBest, kArrow, kWarp
    };
    for (const std::wstring& effect : effects)
    {
        SoundLib::SoundLib::LoadSoundEffect(effect);
    }
    g_initialized = true;
    g_recoveryPending = false;
    g_nextRecoveryTick = 0;
    if (g_hyperModeRequested)
    {
        StartHyperMode();
    }
}

void Finalize()
{
    if (g_initialized)
    {
        StopHyperMode();
        StopEnvironment();
        StopBgmIfPlaying();
    }
    g_initialized = false;
    g_recoveryPending = false;
    g_hyperModeRequested = false;
    g_nextRecoveryTick = 0;
    ResetTrackingState();
}

void Update(HWND windowHandle,
            const SoundLib::Vector3& listenerPosition,
            const SoundLib::Vector3& listenerFront,
            const SoundLib::Vector3& listenerTop)
{
    if (g_recoveryPending)
    {
        const ULONGLONG currentTick = GetTickCount64();
        if (currentTick < g_nextRecoveryTick)
        {
            return;
        }

        try
        {
            SoundLib::SoundLib::Initialize(windowHandle);
            Initialize();
        }
        catch (const SoundLib::AudioDeviceException&)
        {
            SoundLib::SoundLib::Finalize();
            g_nextRecoveryTick = currentTick + kRecoveryRetryIntervalMilliseconds;
            return;
        }
    }

    if (!g_initialized)
    {
        return;
    }

    try
    {
        SoundLib::SoundLib::Update(listenerPosition, listenerFront, listenerTop);
    }
    catch (const SoundLib::AudioDeviceException&)
    {
        BeginAudioDeviceRecovery();
    }
}

void PlayLoadingEnvironment()
{
    PlayEnvironmentIfChanged(kForestEnvironment, 14);
    PlayBgmIfChanged(kLoadingBgm, kTitleBgmVolume);
}

void PlayTitleMusic()
{
    StopEnvironment();
    PlayBgmIfChanged(kTitleBgm, kTitleBgmVolume);
}

void PlayEndingMusic()
{
    StopEnvironment();
    PlayBgmIfChanged(kEndingBgm, kEndingBgmVolume);
}

void PlayStoryMusic()
{
    StopEnvironment();
    PlayBgmIfChanged(kStoryBgm, kStoryBgmVolume);
}

void UpdateStageMusic(const std::wstring& stageId, const int stageNumber, const bool useRainEnvironment, const int world, const bool isCleared)
{
    std::wstring fieldBgm = kW1FieldBgm;
    std::wstring environment = kForestEnvironment;
    int environmentVolume = 18;
    if (useRainEnvironment)
    {
        environment = kRainEnvironment;
        environmentVolume = 18;
    }

    if (stageId.length() >= 6 && stageId.substr(0, 6) == L"select")
    {
        if (world == 2)
        {
            fieldBgm = kStageSelect2Bgm;
        }
        else if (world == 3)
        {
            fieldBgm = kStageSelect3Bgm;
        }
        else if (world == 4)
        {
            fieldBgm = kStageSelect4Bgm;
        }
        else
        {
            fieldBgm = kStageSelect1Bgm;
        }
        environmentVolume = 14;
    }
    else if (stageId.length() >= 4 && stageId.substr(0, 4) == L"base")
    {
        if (world >= 3)
        {
            fieldBgm = kNightBgm;
        }
        else if (world == 2)
        {
            fieldBgm = kKaiganDoukutsuBgm;
        }
        else
        {
            fieldBgm = kBaseBgm;
        }
        environmentVolume = 14;
    }
    else if (stageNumber == 32 && !isCleared)
    {
        // ラスボス戦 (4-8)
        fieldBgm = kLastBossBgm;
        environmentVolume = 14;
    }
    else if (stageNumber == 16 && !isCleared)
    {
        // ボス戦 (2-8)
        fieldBgm = kBoss2Bgm;
        environmentVolume = 14;
    }
    else if (stageNumber >= 1 && stageNumber % 8 == 0 && !isCleared)
    {
        // ボス戦 (1-8, 3-8)
        fieldBgm = kBossBgm;
        environmentVolume = 14;
    }
    else if (stageNumber >= 1 && stageNumber <= 3)
    {
        // ワールド1 草原 (1-1～1-3)
        fieldBgm = kW1FieldBgm;
    }
    else if (stageNumber >= 4 && stageNumber <= 7)
    {
        // ワールド1 湿地・雨霧 (1-4～1-7)
        fieldBgm = kW1SwampBgm;
    }
    else if (stageNumber >= 9 && stageNumber <= 12)
    {
        // ワールド2 洞窟 (2-1～2-4)
        fieldBgm = kW2CaveBgm;
        environmentVolume = 15;
    }
    else if (stageNumber >= 13 && stageNumber <= 15)
    {
        // ワールド2 鉱山 (2-5～2-7)
        fieldBgm = kW2MineBgm;
        environmentVolume = 15;
    }
    else if (stageNumber >= 17 && stageNumber <= 20)
    {
        // ワールド3 遺跡 (3-1～3-4)
        fieldBgm = kW3RuinsBgm;
        environmentVolume = 16;
    }
    else if (stageNumber >= 21 && stageNumber <= 23)
    {
        // ワールド3 山岳道中 (3-5～3-7)
        fieldBgm = kW3TrailBgm;
        environmentVolume = 16;
    }
    else if (stageNumber >= 25 && stageNumber <= 28)
    {
        // ワールド4 要塞外郭 (4-1～4-4)
        fieldBgm = kW4FortressBgm;
        environmentVolume = 16;
    }
    else if (stageNumber >= 29 && stageNumber <= 31)
    {
        // ワールド4 要塞内部 (4-5～4-7)
        fieldBgm = kW4AssaultBgm;
        environmentVolume = 16;
    }
    PlayEnvironmentIfChanged(environment, environmentVolume);
    PlayBgmIfChanged(fieldBgm, kFieldBgmVolume);
}

void PlayMenuMove() { PlayEffect(kMenuMove, 70); }
void PlayMenuConfirm() { PlayEffect(kMenuConfirm, 78); }
void PlayMenuCancel() { PlayEffect(kMenuCancel, 72); }
void PlaySaveComplete() { PlayEffect(kSaveComplete, 78); }
void BeginBgmFadeOut(const int frames) { BeginBgmFadeOutInternal(frames); }
void UpdateBgmFadeOut() { UpdateBgmFadeOutInternal(); }
void PlayBossDefeat() { PlayEffect(kPlayerDeath, 92); }
void PlayStageSelectMove() { PlayEffect(kStageSelectMove, 72); }
void PlayStageSelectConfirm() { PlayEffect(kStageSelectConfirm, 78); }
void PlayPlayerAttack() { PlayEffect(kPlayerAttack, 82); }
void PlaySwordSwing() { PlayEffect(kSwordSwing, 80); }
void PlaySlashHit() { PlayEffect(kSlashHit, 82); }
void PlayAttackHit() { PlayEffect(kAttackHit, 82); }
void PlayLeverToggle() { PlayEffect(kLeverToggle, 80); }
void PlayRopeCut() { PlayEffect(kRopeCut, 82); }
void PlayMechanismStop() { PlayEffect(kMechanismStop, 80); }
void PlayBusterHit() { PlayEffect(kBusterHit, 76); }
void PlayEnemyAttack() { PlayEffect(kEnemyAttack, 72); }
void PlayPlayerDamage() { PlayEffect(kPlayerDamage, 88); }
void PlayPlayerDeath() { PlayEffect(kPlayerDeath, 88); }
void PlayItemGet() { PlayEffect(kItemGet, 82); }
void PlayAmmoMax() { PlayEffect(kAmmoMax, 78); }
void PlayAmmoGet() { PlayEffect(kAmmoGet, 80); }
void PlayJump() { PlayEffect(kJump, 62); }
void PlayPowerUp() { PlayEffect(kPowerUp, 82); }
void PlayDrink() { PlayEffect(kDrink, 80); }
void StartHyperMode()
{
    g_hyperModeRequested = true;
    if (!g_initialized)
    {
        return;
    }

    if (g_hyperModeId >= 0)
    {
        return;
    }

    try
    {
        g_hyperModeId = SoundLib::SoundLib::PlayEnvironmentSound(kHyperMode, 78);
    }
    catch (const SoundLib::AudioDeviceException&)
    {
        BeginAudioDeviceRecovery();
        return;
    }
    ApplyCurrentBgmVolume();
}

void StopHyperMode()
{
    g_hyperModeRequested = false;
    if (!g_initialized)
    {
        g_hyperModeId = -1;
        return;
    }

    if (g_hyperModeId < 0)
    {
        return;
    }

    try
    {
        SoundLib::SoundLib::StopEnvironmentSound(g_hyperModeId);
    }
    catch (const SoundLib::AudioDeviceException&)
    {
        BeginAudioDeviceRecovery();
        return;
    }
    g_hyperModeId = -1;
    ApplyCurrentBgmVolume();
}

void PlayDash() { PlayEffect(kDash, 72); }
void PlayDashBooster() { PlayEffect(kDashBooster, 78); }
void PlayExplosion() { PlayEffect(kExplosion, 75); }
void PlayBombPlace() { PlayEffect(kBombPlace, 78); }
void PlayStomp() { PlayEffect(kStomp, 82); }
void PlayBuster() { PlayEffect(kBuster, 55); }
void PlayWeaponChange() { PlayEffect(kWeaponChange, 72); }
void PlayArrow() { PlayEffect(kArrow, 100); }
void PlayWarp() { PlayEffect(kWarp, 80); }
void PlayStageClear() { PlayEffect(kQteBest, 86); }
void PlayQteStart() { PlayEffect(kQte, 70); }
void PlayQteStop() { PlayEffect(kStageSelectConfirm, 70); }
void PlayQteSuccess() { PlayEffect(kQteBest, 82); }
void PlayQteNormal() { PlayEffect(kMenuConfirm, 76); }
void PlayQteFailure() { PlayEffect(kMenuCancel, 76); }
}
