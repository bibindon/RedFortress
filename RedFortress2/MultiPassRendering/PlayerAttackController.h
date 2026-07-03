#pragma once

#include <string>

enum class PlayerAttackType
{
    WeakAttack,
    StrongAttack,
    SwordAttack,
    SwordStrongAttack,
    BombAttack,
    BombStrongAttack,
    BusterAttack,
    BusterStrongAttack
};

struct PlayerAttackDefinition
{
    int durationFrames = 0;
    int hitDelayFrames = 0;
    int damage = 0;
    float range = 0.0f;
    float halfAngleRadians = 0.0f;
    float moveSpeed = 0.0f;
    int moveStartRemainingFrames = 0;
    int moveEndRemainingFrames = 0;
    std::wstring animationName;
    float animationSpeed = 1.0f;
};

class PlayerAttackController
{
public:
    PlayerAttackController();

    bool TryStart(PlayerAttackType attackType);
    void Update();
    void Reset();

    bool IsAttacking() const;
    bool IsMovementActive() const;
    bool ConsumeHitRequested();
    const PlayerAttackDefinition& GetCurrentDefinition() const;

    void SelectClubCategory();
    void CycleAttackCategory(int direction = 1);
    PlayerAttackType GetAttackType(bool isStrong) const;
    const wchar_t* GetCurrentCategoryName() const;

private:
    PlayerAttackDefinition GetDefinition(PlayerAttackType attackType) const;

    PlayerAttackType m_currentAttackType = PlayerAttackType::WeakAttack;
    PlayerAttackDefinition m_currentDefinition;
    int m_selectedCategory = 0;
    int m_remainingFrames = 0;
    int m_hitDelayFrames = -1;
    bool m_hitRequested = false;
    bool m_hitResolved = false;
};
