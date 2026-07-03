#pragma once

#include <d3dx9.h>
#include <string>

namespace NSRender
{
class Render;
}

namespace PhysicsLib
{
class CharacterMover;
}

class DashBooster
{
public:
    void Initialize(NSRender::Render& render,
                    const std::wstring& id,
                    const D3DXVECTOR3& position,
                    const D3DXVECTOR3& direction,
                    float speed,
                    float duration,
                    float radius,
                    float scale,
                    bool chargeEnabled);
    void Update(NSRender::Render& render,
                const D3DXVECTOR3& playerPosition,
                PhysicsLib::CharacterMover& playerMover);
    void Release(NSRender::Render& render);

private:
    void UpdateVisual(NSRender::Render& render);
    void Trigger(NSRender::Render& render, PhysicsLib::CharacterMover& playerMover);
    void PlayLaunchEffects(NSRender::Render& render);
    bool CanTrigger(const D3DXVECTOR3& playerPosition,
                    const PhysicsLib::CharacterMover& playerMover) const;

    std::wstring m_id;
    D3DXVECTOR3 m_position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 m_direction = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    D3DXVECTOR3 m_visualRotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float m_speed = 16.0f;
    float m_duration = 0.35f;
    float m_radius = 1.0f;
    float m_scale = 0.5f;
    bool m_chargeEnabled = true;
    int m_renderId = -1;
    int m_cooldownFrames = 0;
    int m_launchEffectDelayFrames = 0;
    int m_damageFlashFrames = 0;
    bool m_waitingForLaunchEffect = false;
};
