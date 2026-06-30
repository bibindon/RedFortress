#pragma once

class Player
{
public:
    static const int DEFAULT_MAX_HP = 100;
    static const int DEFAULT_MAX_LIVES = 3;
    static const int MAX_LIVES = 99;

    Player();

    void ResetHp();
    void ResetLives();
    void Die();
    bool IsHpZero() const;
    bool IsGameOver() const;

    int GetHp() const;
    int GetMaxHp() const;
    int GetLives() const;
    int GetMaxLives() const;
    void SetHp(int hp);
    bool AddLife();
    void Damage(int amount);
    void Heal(int amount);

private:
    int ClampHp(int hp) const;

    int m_maxHp = DEFAULT_MAX_HP;
    int m_hp = DEFAULT_MAX_HP;
    int m_maxLives = MAX_LIVES;
    int m_lives = DEFAULT_MAX_LIVES;
};
