#pragma once

#include <string>

namespace GameAudio
{
void Initialize();
void Finalize();
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
void PlayQteStart();
void PlayQteStop();
void PlayQteSuccess();
void PlayQteNormal();
void PlayQteFailure();
}
