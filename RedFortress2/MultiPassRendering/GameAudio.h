#pragma once

#include <string>

namespace GameAudio
{
void Initialize();
void Finalize();
void PlayTitleMusic();
void PlayEndingMusic();
void UpdateStageMusic(const std::wstring& stageId, int stageNumber, bool combatActive);
void PlayMenuMove();
void PlayMenuConfirm();
void PlayMenuCancel();
void PlayPlayerAttack();
void PlayAttackHit();
void PlayEnemyAttack();
void PlayPlayerDamage();
void PlayItemGet();
void PlayJump();
}
