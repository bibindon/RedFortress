#pragma once

class Player
{
public:
    static const int DEFAULT_MAX_HP = 100;

    Player();

    void ResetHp();
    int GetHp() const;
    int GetMaxHp() const;
    void SetHp(int hp);
    void Damage(int amount);
    void Heal(int amount);
    bool IsDead() const;

private:
    int ClampHp(int hp) const;

    int m_maxHp = DEFAULT_MAX_HP;
    int m_hp = DEFAULT_MAX_HP;
};
