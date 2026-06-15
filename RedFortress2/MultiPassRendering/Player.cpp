#include "Player.h"

Player::Player()
{
    ResetHp();
}

void Player::ResetHp()
{
    m_hp = m_maxHp;
}

int Player::GetHp() const
{
    return m_hp;
}

int Player::GetMaxHp() const
{
    return m_maxHp;
}

void Player::SetHp(int hp)
{
    m_hp = ClampHp(hp);
}

void Player::Damage(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    SetHp(m_hp - amount);
}

void Player::Heal(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    SetHp(m_hp + amount);
}

bool Player::IsDead() const
{
    if (m_hp <= 0)
    {
        return true;
    }

    return false;
}

int Player::ClampHp(int hp) const
{
    if (hp < 0)
    {
        return 0;
    }

    if (m_maxHp < hp)
    {
        return m_maxHp;
    }

    return hp;
}
