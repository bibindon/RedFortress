#include "DashBooster.h"

#include "GameAudio.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"
#include <cmath>

namespace
{
const std::wstring kDashBoosterModelPath = L"res\\model\\dashBooster\\dashBooster.x";
const int kDashBoosterCooldownFrames = 30;
const int kDashBoosterSoundDelayFrames = 30;
const int kDashBoosterDamageFlashFrames = 10;
const int kDashBoosterPulseFrames = 18;
const float kDashBoosterVisualScale = 3.0f;
const float kDashBoosterModelYawOffset = D3DX_PI / 6.0f;
const float kDashBoosterIdleScaleAmplitude = 0.08f;
const float kDashBoosterPulseScaleAmplitude = 0.22f;
const float kDashBoosterIdleSpeed = 0.11f;

D3DXVECTOR3 CalculateDashBoosterVisualRotation(const D3DXVECTOR3& direction)
{
    D3DXVECTOR3 normalizedDirection = direction;
    if (D3DXVec3LengthSq(&normalizedDirection) <= 0.0001f)
    {
        return D3DXVECTOR3(0.0f, -kDashBoosterModelYawOffset, 0.0f);
    }

    D3DXVec3Normalize(&normalizedDirection, &normalizedDirection);

    const float horizontalLength =
        std::sqrt(normalizedDirection.x * normalizedDirection.x +
                  normalizedDirection.z * normalizedDirection.z);

    float yaw = 0.0f;
    if (horizontalLength > 0.0001f)
    {
        yaw = std::atan2(normalizedDirection.x, normalizedDirection.z);
    }

    const float pitch = std::atan2(normalizedDirection.y, horizontalLength);
    return D3DXVECTOR3(pitch, yaw - kDashBoosterModelYawOffset, 0.0f);
}
}

void DashBooster::Initialize(NSRender::Render& render,
                             const std::wstring& id,
                             const D3DXVECTOR3& position,
                             const D3DXVECTOR3& direction,
                             const float speed,
                             const float duration,
                             const float radius,
                             const float scale,
                             const bool chargeEnabled)
{
    m_id = id;
    m_position = position;
    m_direction = direction;
    if (D3DXVec3LengthSq(&m_direction) <= 0.0001f)
    {
        m_direction = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    }
    D3DXVec3Normalize(&m_direction, &m_direction);

    m_speed = speed;
    m_duration = duration;
    m_radius = radius;
    m_scale = scale;
    m_chargeEnabled = chargeEnabled;
    m_visualRotation = CalculateDashBoosterVisualRotation(m_direction);
    m_cooldownFrames = 0;
    m_soundDelayFrames = 0;
    m_damageFlashFrames = 0;
    m_visualFrame = 0;
    m_pulseFrames = 0;

    m_renderId = render.AddMeshMix(NSRender::Util::GetExeDir() + kDashBoosterModelPath,
                                   m_position,
                                   m_visualRotation,
                                   m_scale * kDashBoosterVisualScale,
                                   -1.0f,
                                   false,
                                   false,
                                   false);
    UpdateVisual(render);
}

void DashBooster::Update(NSRender::Render& render,
                         const D3DXVECTOR3& playerPosition,
                         PhysicsLib::CharacterMover& playerMover)
{
    if (m_soundDelayFrames > 0)
    {
        --m_soundDelayFrames;
        if (m_soundDelayFrames <= 0)
        {
            GameAudio::PlayDashBooster();
        }
    }

    if (m_cooldownFrames > 0)
    {
        --m_cooldownFrames;
    }

    if (m_damageFlashFrames > 0)
    {
        --m_damageFlashFrames;
        if (m_damageFlashFrames <= 0 && m_renderId >= 0)
        {
            render.SetMeshMixDamageFlash(m_renderId, false);
        }
    }

    UpdateVisual(render);

    if (CanTrigger(playerPosition, playerMover))
    {
        Trigger(render, playerMover);
    }
}

void DashBooster::Release(NSRender::Render& render)
{
    if (m_renderId >= 0)
    {
        render.SetMeshMixDamageFlash(m_renderId, false);
        render.RemoveMeshMix(m_renderId);
        m_renderId = -1;
    }
}

void DashBooster::UpdateVisual(NSRender::Render& render)
{
    if (m_renderId < 0)
    {
        return;
    }

    ++m_visualFrame;
    const float idleScale = 1.0f + std::sin(static_cast<float>(m_visualFrame) * kDashBoosterIdleSpeed) *
                                      kDashBoosterIdleScaleAmplitude;

    float pulseScale = 0.0f;
    if (m_pulseFrames > 0)
    {
        const float pulseProgress =
            static_cast<float>(m_pulseFrames) / static_cast<float>(kDashBoosterPulseFrames);
        pulseScale = std::sin(pulseProgress * D3DX_PI) * kDashBoosterPulseScaleAmplitude;
        --m_pulseFrames;
    }

    const float visualScale = m_scale * kDashBoosterVisualScale * (idleScale + pulseScale);

    D3DXMATRIX scaleMatrix;
    D3DXMATRIX rotationMatrix;
    D3DXMATRIX translationMatrix;
    D3DXMatrixScaling(&scaleMatrix, visualScale, visualScale, visualScale);
    D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
                                   m_visualRotation.y,
                                   m_visualRotation.x,
                                   m_visualRotation.z);
    D3DXMatrixTranslation(&translationMatrix, m_position.x, m_position.y, m_position.z);

    const D3DXMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;
    render.SetMeshMixWorldMatrix(m_renderId, worldMatrix);
}

void DashBooster::Trigger(NSRender::Render& render, PhysicsLib::CharacterMover& playerMover)
{
    playerMover.SetPosition(m_position);
    playerMover.ApplyDashBooster(m_direction,
                                 m_speed,
                                 m_duration,
                                 m_chargeEnabled);

    const D3DXVECTOR3 effectPosition = m_position + m_direction * 0.55f;
    render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage, effectPosition);
    if (m_renderId >= 0)
    {
        render.SetMeshMixDamageFlash(m_renderId, true);
    }

    m_damageFlashFrames = kDashBoosterDamageFlashFrames;
    m_pulseFrames = kDashBoosterPulseFrames;
    m_soundDelayFrames = kDashBoosterSoundDelayFrames;
    m_cooldownFrames = kDashBoosterCooldownFrames;
}

bool DashBooster::CanTrigger(const D3DXVECTOR3& playerPosition,
                             const PhysicsLib::CharacterMover& playerMover) const
{
    if (playerMover.IsBoosted())
    {
        return false;
    }

    if (m_cooldownFrames > 0)
    {
        return false;
    }

    const D3DXVECTOR3 difference = playerPosition - m_position;
    if (D3DXVec3Length(&difference) > m_radius)
    {
        return false;
    }

    return true;
}
