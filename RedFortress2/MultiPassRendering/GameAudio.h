#pragma once

#include <Windows.h>
#include <string>

#include "../../SoundLib/SoundLib/SoundLib.h"

namespace GameAudio
{
void Initialize();
void Finalize();
void Update(HWND windowHandle,
            const SoundLib::Vector3& listenerPosition,
            const SoundLib::Vector3& listenerFront,
            const SoundLib::Vector3& listenerTop);
void PlayLoadingEnvironment();
void PlayTitleMusic();
void PlayEndingMusic();
void UpdateStageMusic(const std::wstring& stageId, const int stageNumber, const bool useRainEnvironment, const int world);
void PlayMenuMove();
void PlayMenuConfirm();
void PlayMenuCancel();
void PlayStageSelectMove();
void PlayStageSelectConfirm();
void PlayPlayerAttack();
void PlaySlashHit();
void PlayAttackHit();
void PlayEnemyAttack();
void PlayPlayerDamage();
void PlayPlayerDeath();
void PlayItemGet();
void PlayAmmoMax();
void PlayJump();
void PlayPowerUp();
void PlayDrink();
void StartHyperMode();
void StopHyperMode();
void PlayDash();
void PlayDashBooster();
void PlayExplosion();
void PlayBombPlace();
void PlayStomp();
void PlayBuster();
void PlayWeaponChange();
void PlayArrow();
void PlayStageClear();
void PlayQteStart();
void PlayQteStop();
void PlayQteSuccess();
void PlayQteNormal();
void PlayQteFailure();
}
