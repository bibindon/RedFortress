#include "Player.h"

Player::Player()
{
    ResetHp();
    ResetLives();
}

void Player::ResetHp()
{
    m_hp = m_maxHp;
}

void Player::ResetLives()
{
    m_lives = DEFAULT_MAX_LIVES;
}

void Player::Die()
{
    if (m_lives > 0)
    {
        --m_lives;
    }
    ResetHp();
}

bool Player::IsHpZero() const
{
    return m_hp <= 0;
}

bool Player::IsGameOver() const
{
    return m_lives <= 0;
}

int Player::GetHp() const
{
    return m_hp;
}

int Player::GetMaxHp() const
{
    return m_maxHp;
}

int Player::GetLives() const
{
    return m_lives;
}

int Player::GetMaxLives() const
{
    return m_maxLives;
}

void Player::SetHp(int hp)
{
    m_hp = ClampHp(hp);
}

bool Player::AddLife()
{
    if (m_lives >= m_maxLives)
    {
        return false;
    }

    ++m_lives;
    return true;
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
