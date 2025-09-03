#include "EnemyBase.h"

#include <ctime>
#include "SoundEffect.h"
#include "SharedObj.h"
#include "Sprite.h"
#include "Camera.h"
#include <random>
#include "../../RedFortressModel/Model/StatusManager.h"
#include "../../RedFortressModel/Model/EnemyInfoManager.h"

EnemyBase::EnemyBase()
{
    SoundEffect::get_ton()->load(_T("res\\sound\\enemyStep.wav"));
    SoundEffect::get_ton()->load(_T("res\\sound\\enemyAttack.wav"));
}

EnemyBase::~EnemyBase()
{
    m_thread->join();
    SAFE_DELETE(m_thread);
    SAFE_DELETE(m_spriteHP);
    SAFE_DELETE(m_spriteHPBack);
    SAFE_DELETE(m_AnimMesh);
}

std::mutex EnemyBase::s_mutex;

bool EnemyBase::Init()
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
            m_AnimMesh = NEW AnimMesh(_T("res\\model\\rippoutai.x"),
                m_loadingPos, m_rotate, 0.5f, animSetMap);
            SoundEffect::get_ton()->load(_T("res\\sound\\damage01.wav"));
            
            m_spriteHP = NEW Sprite(_T("res\\image\\hp_green.png"));
            m_spriteHPBack = NEW Sprite(_T("res\\image\\hp_black.png"));

            m_AnimMesh->SetAnim(_T("0_Idle"), 0.f);
            m_loaded.store(true);
        });
    return true;
}

void EnemyBase::Update()
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
        }
    }
    else if (m_state == eEnemyState::WALK)
    {
        Player* player = SharedObj::GetPlayer();
        D3DXVECTOR3 pos = player->GetPos();
        D3DXVECTOR3 enemyVector = pos - m_loadingPos;
        FLOAT distance = D3DXVec3Length(&enemyVector);
        if (distance < 3.f)
        {
            int randNum = SharedObj::GetRandom();

            //std::wstring msg;
            //msg = _T("randNum: " + std::to_wstring(randNum) + "\n");
            if (randNum % 30 == 0)
            {
                m_state = eEnemyState::ATTACK;
            }
        }
        else if (3.f <= distance && distance < 20.f)
        {
            D3DXVECTOR3 norm { 0.f, 0.f, 0.f };
            D3DXVec3Normalize(&norm, &enemyVector);
            m_loadingPos += norm / 50;
            m_rotate.y = atan2(-enemyVector.x, -enemyVector.z);
        }
        else if (20.f <= distance)
        {
            m_state = eEnemyState::IDLE;
            m_AnimMesh->SetAnim(_T("0_Idle"), 0.f);
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
}

void EnemyBase::Render()
{
    if (m_loaded.load() == false)
    {
        return;
    }

    m_AnimMesh->SetPos(m_loadingPos);
    m_AnimMesh->SetRotate(m_rotate);
    m_AnimMesh->Render();

    POINT screenPos = Camera::GetScreenPos(m_loadingPos);

    //-----------------------------------------------------------------
    // カメラと敵の距離が近ければ表示する
    //
    // カメラとプレイヤーの距離ではないことに注意。
    // 「カメラとプレイヤーの距離が近ければ表示」としてしまうと、
    // カメラがプレイヤーから離れたときも表示されてしまう。
    //-----------------------------------------------------------------
    auto eyePos = Camera::GetEyePos();

    bool nBear = Common::HitByBoundingBox(m_loadingPos, eyePos, 25.f);

    if (nBear)
    {
        if (screenPos.x >= 0)
        {
            D3DXVECTOR3 pos ((FLOAT)screenPos.x - 64, (FLOAT)screenPos.y - 128, 0.f);

            m_spriteHPBack->Render(pos);

            if (m_HP >= 0)
            {
                m_spriteHP->Render(pos, 255, (m_HP * 128 / 100));
            }
        }
    }
}

void EnemyBase::SetPos(const D3DXVECTOR3& pos)
{
    m_loadingPos = pos;
}

D3DXVECTOR3 EnemyBase::GetPos() const
{
    return m_loadingPos;
}

void EnemyBase::SetRotate(const D3DXVECTOR3& rotate)
{
    m_rotate = rotate;
}

D3DXVECTOR3 EnemyBase::GetRotate() const
{
    return m_rotate;
}

void EnemyBase::SetHP(const int hp)
{
    m_HP = hp;
    if (m_HP <= 0)
    {
        m_state = eEnemyState::DEAD;
    }
}

int EnemyBase::GetHP() const
{
    return m_HP;
}

void EnemyBase::SetState(const eEnemyState state)
{
    if (m_loaded.load() == false)
    {
        return;
    }

    if (state == eEnemyState::DAMAGED)
    {
        SoundEffect::get_ton()->play(_T("res\\sound\\damage01.wav"), 90);
        m_AnimMesh->SetAnim(_T("Damaged"), 0.f);
    }
    else if (state == eEnemyState::DEAD)
    {
        SoundEffect::get_ton()->play(_T("res\\sound\\damage01.wav"), 90);
        m_AnimMesh->SetAnim(_T("Damaged"), 0.f);
    }
    m_state = state;
}

eEnemyState EnemyBase::GetState() const
{
    return m_state;
}

void EnemyBase::SetIdSub(const int arg)
{
    m_idSub = arg;
}

int EnemyBase::GetIdSub() const
{
    return m_idSub;
}

eEnemyType EnemyBase::GetEnemyType()
{
    return m_eType;
}

void EnemyBase::SetShowMenu(const std::wstring& id)
{
    auto enemyInfoManager = NSModel::EnemyInfoManager::GetObj();
    auto enemyIdList = enemyInfoManager->GetEnemyIdList();

    std::wstring enemyName_;
    for (auto& enemyId: enemyIdList)
    {
        if (id == enemyId)
        {
            auto enemyDef = enemyInfoManager->GetEnemyDef(enemyId);
            enemyName_ = enemyDef.GetName();
            break;
        }
    }

    enemyInfoManager->SetEnemyVisible(enemyName_, true);
}

D3DXVECTOR3 EnemyBase::GetAttackPos() const
{
    D3DXVECTOR3 pos { m_loadingPos };
    D3DXVECTOR3 norm { 0.f, 0.f, 0.f };
    norm.x = std::sin(m_rotate.y + D3DX_PI);
    norm.z = std::sin(m_rotate.y + (D3DX_PI * 3 / 2));
    pos += norm * 2;
    return pos;
}

