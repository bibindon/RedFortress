#pragma once

#include <Windows.h>
#include <string>

#include "../../SoundLib/SoundLib/SoundLib.h"

namespace GameAudio
{
void Initialize();
void Finalize();
// --silent 起動時はサウンドデバイス初期化と再生を一切行わない。
void SetSilentMode(bool silent);
bool IsSilentMode();
// ゲームが非アクティブな間だけ BGM をフェードして無音にする。
void SetWindowFocused(bool focused);
void Update(HWND windowHandle,
            const SoundLib::Vector3& listenerPosition,
            const SoundLib::Vector3& listenerFront,
            const SoundLib::Vector3& listenerTop);
void PlayLoadingEnvironment();
void PlayTitleMusic();
void PlayGameOverMusic();
void PlayEndingMusic();
void PlayStoryMusic();
void UpdateStageMusic(const std::wstring& stageId, const int stageNumber, const bool useRainEnvironment, const int world, const bool isCleared);
void PlayMenuMove();
void PlayMenuConfirm();
void PlayMenuCancel();
void PlayItemUnavailable();
void PlayMenuOpen();
void PlayCraftOpen();
void PlayExplanationOpen();
void PlaySaveComplete();
void BeginBgmFadeOut(int frames);
void UpdateBgmFadeOut();
void PlayBossDefeat();
void PlayStageSelectMove();
void PlayStageSelectConfirm();
void PlayPlayerAttack();
void PlaySwordSwing();
void PlaySlashHit();
void PlayAttackHit();
void PlaySwordDeflected();
void PlayLeverToggle();
void PlayRopeCut();
void PlayMechanismStop();
void SetDoorMovementActive(bool active);
void StopDoorMovement();
void StartPushableBoxMovement();
void StopPushableBoxMovement();
void PlayBusterHit();
void PlayEnemyAttack();
void PlayPlayerDamage();
void PlayPlayerDeath();
void PlayItemGet();
void PlayAmmoMax();
void PlayAmmoGet();
void PlayJump();
void PlayPowerUp();
void PlayDrink();
void PlayItemHeal();
void PlayItemLifeUp();
void StartHyperMode();
void StopHyperMode();
void PlayDash();
void PlayDashBooster();
void PlayExplosion();
void PlayBombPlace();
void PlayStomp();
void PlayBuster();
void PlaySkullGrab();
void PlaySkullThrow();
void PlaySkullHit();
void PlaySkullLand();
void PlayWeaponChange();
void PlayArrow();
void PlayWarp();
void PlayStageClear();
void PlayQteStart();
void PlayQteStop();
void PlayQteSuccess();
void PlayQteNormal();
void PlayQteFailure();
}
