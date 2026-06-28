#pragma once

#include <string>

namespace GameAudio
{
void Initialize();
void Finalize();
void PlayLoadingEnvironment();
void PlayTitleMusic();
void PlayEndingMusic();
void UpdateStageMusic(const std::wstring& stageId, int stageNumber, bool useRainEnvironment);
void PlayMenuMove();
void PlayMenuConfirm();
void PlayMenuCancel();
void PlayPlayerAttack();
void PlaySlashHit();
void PlayAttackHit();
void PlayEnemyAttack();
void PlayPlayerDamage();
void PlayItemGet();
void PlayJump();
void PlayPowerUp();
void PlayDash();
void PlayExplosion();
void PlayStomp();
void PlayBuster();
}
