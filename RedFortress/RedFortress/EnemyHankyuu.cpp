#include "EnemyHankyuu.h"
#include <ctime>
#include "SoundEffect.h"
#include "SharedObj.h"
#include "Sprite.h"
#include "Camera.h"
#include <random>
#include "../../RedFortressModel/Model/StatusManager.h"

EnemyHankyuu::EnemyHankyuu()
{
    m_eType = eEnemyType::Hankyuu;
}

EnemyHankyuu::~EnemyHankyuu()
{
}

bool EnemyHankyuu::Init()
{
    m_thread = NEW std::thread(
        [&]
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            AnimSetMap animSetMap;
            {
                AnimSetting animSetting { };
                animSetting.m_startPos = 0.5f;
                animSetting.m_duration = 0.5f;
                animSetting.m_loop = true;
                animSetMap[_T("0_Idle")] = animSetting;
            }
            {
                AnimSetting animSetting { };
                animSetting.m_startPos = 0.f;
                animSetting.m_duration = 0.5f;
                animSetting.m_loop = true;
                animSetMap[_T("Walk")] = animSetting;
            }
            {
                AnimSetting animSetting { };
                animSetting.m_startPos = 1.f;
                animSetting.m_duration = 1.f;
                animSetting.m_loop = false;
                animSetMap[_T("Damaged")] = animSetting;
            }
            {
                AnimSetting animSetting { };
                animSetting.m_startPos = 2.f;
                animSetting.m_duration = 0.5f;
                animSetting.m_loop = false;
                animSetMap[_T("Attack")] = animSetting;
            }
            m_AnimMesh = NEW AnimMesh(_T("res\\model\\enemyHankyuu.x"),
                m_loadingPos, m_rotate, 0.5f, animSetMap);
            SoundEffect::get_ton()->load(_T("res\\sound\\damage01.wav"));
            
            m_spriteHP = NEW Sprite(_T("res\\image\\hp_green.png"));
            m_spriteHPBack = NEW Sprite(_T("res\\image\\hp_black.png"));

            m_AnimMesh->SetAnim(_T("0_Idle"), 0.f);
            m_loaded.store(true);
        }
        );
    return true;
}

void EnemyHankyuu::Update()
{
    if (m_loaded.load() == false)
    {
        return;
    }

    if (m_state == eEnemyState::DEAD)
    {
        ++m_deadTimeCounter;
        if (m_deadTimeCounter >= 60)
        {
            m_state = eEnemyState::DISABLE;
            SetShowMenu(L"hankyu");
        }
    }
    else if (m_state == eEnemyState::IDLE)
    {
        Player* player = SharedObj::GetPlayer();
        D3DXVECTOR3 pos = player->GetPos();
        D3DXVECTOR3 enemyVector = pos - m_loadingPos;
        FLOAT distance = D3DXVec3Length(&enemyVector);
        if (distance < 10.f)
        {
            m_state = eEnemyState::WALK;
            m_AnimMesh->SetAnim(_T("Walk"), 0.f);
            SoundEffect::get_ton()->play(_T("res\\sound\\enemyStep.wav"), 90);
        }
    }
    else if (m_state == eEnemyState::WALK)
    {
        Player* player = SharedObj::GetPlayer();
        D3DXVECTOR3 pos = player->GetPos();
        D3DXVECTOR3 enemyVector = pos - m_loadingPos;
        FLOAT distance = D3DXVec3Length(&enemyVector);
        if (distance < 2.f)
        {
            int randNum = SharedObj::GetRandom() % 100;

            if (randNum < 3)
            {
                m_state = eEnemyState::ATTACK;
                SoundEffect::get_ton()->stop(_T("res\\sound\\enemyStep.wav"));
            }
            else if (randNum < 8)
            {
                m_state = eEnemyState::STEP;
                SoundEffect::get_ton()->stop(_T("res\\sound\\enemyStep.wav"));
            }
        }
        else if (2.f <= distance && distance < 20.f)
        {
            D3DXVECTOR3 norm { 0.f, 0.f, 0.f };
            D3DXVec3Normalize(&norm, &enemyVector);
            // 壁ずり
            Map* map = SharedObj::GetMap();
            D3DXVECTOR3 move = norm * 0.02f;
            bool bHit = false;
            bool bInside = false;
            move = map->WallSlide(m_loadingPos, move, &bHit, &bInside, false);
            m_loadingPos += move;
            m_rotate.y = atan2(-enemyVector.x, -enemyVector.z);
        }
        else if (20.f <= distance)
        {
            m_state = eEnemyState::IDLE;
            m_AnimMesh->SetAnim(_T("0_Idle"), 0.f);
            SoundEffect::get_ton()->stop(_T("res\\sound\\enemyStep.wav"));
        }
    }
    else if (m_state == eEnemyState::DAMAGED)
    {
        m_damagedTimeCounter++;
        if (m_damagedTimeCounter >= 60)
        {
            m_damagedTimeCounter = 0;
            m_state = eEnemyState::IDLE;
        }
    }
    else if (m_state == eEnemyState::ATTACK)
    {
        ++m_attackTimeCounter;
        if (m_attackTimeCounter == 1)
        {
            m_AnimMesh->SetAnim(_T("Attack"), 0.f);
            SoundEffect::get_ton()->play(_T("res\\sound\\enemyAttack.wav"), 90);
            Player* player = SharedObj::GetPlayer();
            D3DXVECTOR3 pos = player->GetPos();
            D3DXVECTOR3 rot = pos - m_loadingPos;
            m_rotate.y = -atan2(rot.z, rot.x) + D3DX_PI*3/2;


            D3DXVECTOR3 attackPos { GetAttackPos() };
            D3DXVECTOR3 playerPos { 0.f, 0.f, 0.f };
            playerPos = player->GetPos();
            D3DXVECTOR3 subPos { attackPos - playerPos };
            FLOAT distance = D3DXVec3Length(&subPos);
            std::wstring msg;
            msg = _T("distance: ") + std::to_wstring(distance) + _T("\n");

            if (distance <= 1.0f)
            {
                player->SetDamaged();
                auto status = NSModel::StatusManager::GetObj();

                auto muscle = status->GetMuscleCurrent();
                status->SetMuscleCurrent(muscle - 1);

                auto brain = status->GetBrainStaminaCurrent();
                status->SetBrainStaminaCurrent(brain - 1);
            }

        }
        else if (m_attackTimeCounter >= 60)
        {
            m_attackTimeCounter = 0;
            m_state = eEnemyState::IDLE;
        }
    }
    else if (m_state == eEnemyState::STEP)
    {
        ++m_stepTimeCounter;

        if (m_stepTimeCounter == 1)
        {
            m_AnimMesh->SetAnim(_T("0_Idle"), 0.f);

            Player* player = SharedObj::GetPlayer();
            D3DXVECTOR3 pos = player->GetPos();
            D3DXVECTOR3 enemyVector = pos - m_loadingPos;
            D3DXMATRIX rotY;
            D3DXMatrixRotationY(&rotY, D3DXToRadian(90));
            D3DXVECTOR3 moveDir2;
            D3DXVec3TransformCoord(&moveDir2, &enemyVector, &rotY);
            D3DXVec3Normalize(&m_vStep, &moveDir2);
            m_vStep *= 0.2f;
        }
        else if (m_stepTimeCounter < 10)
        {
            m_loadingPos += m_vStep;
        }
        else if (m_stepTimeCounter >= 30)
        {
            m_stepTimeCounter = 0;
            m_state = eEnemyState::IDLE;
        }
    }
}

