#pragma once

#include <d3dx9.h>
#include <cstddef>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

enum class AttackTriggerType
{
    Lever,
    LeverLift,
    Rope,
    Button
};

enum class AttackTriggerActivation
{
    None,
    Lever,
    Rope,
    Button
};

class AttackTriggerManager
{
public:
    void Initialize();
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Clear(NSRender::Render& render);
    void Update(NSRender::Render& render, float deltaSeconds);
    void ResetLevers(NSRender::Render& render,
                     const std::vector<int>& triggerIds);

    AttackTriggerActivation TryActivateInAttackRange(
        NSRender::Render& render,
        const D3DXVECTOR3& playerPosition,
        float playerYaw,
        float range,
        float verticalMinOffset,
        float verticalMaxOffset,
        float halfAngleRadians);

    std::size_t GetTriggerCount() const;
    bool IsTargetMoving() const;

private:
    struct Trigger
    {
        int id = -1;
        AttackTriggerType type = AttackTriggerType::Lever;
        int targetCsvId = -1;
        int targetPhysicsId = -1;
        bool hasTarget = false;
        int visualMeshId = -1;
        int leverHandleVisualMeshId = -1;
        int activeVisualMeshId = -1;
        bool leverActive = false;
        bool buttonActive = false;
        float lightBrightness = 2.8f;
        float lightRange = 10.0f;
        D3DXCOLOR lightColor = D3DXCOLOR(1.0f, 0.86f, 0.62f, 1.0f);
        std::wstring lightOwnerTag;
        std::wstring locatorOwnerTag;
        bool ropeUsed = false;
        bool stopSoundPlayed = false;
        D3DXVECTOR3 triggerPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 targetPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 targetRotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 targetScale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
        D3DXVECTOR3 targetAxis = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
        float currentAngle = 0.0f;
        float currentLeverVisualAngle = 0.0f;
        float liftHeight = 0.0f;
        float currentLift = 0.0f;
    };

    bool IsTargetInAttackRange(const Trigger& trigger,
                               const D3DXVECTOR3& playerPosition,
                               float playerYaw,
                               float range,
                               float verticalMinOffset,
                               float verticalMaxOffset,
                               float halfAngleRadians) const;
    bool IsReadyForAttack(const Trigger& trigger) const;
    void UpdateTrigger(NSRender::Render& render,
                       Trigger& trigger,
                       float deltaSeconds);
    void ApplyTargetTransform(NSRender::Render& render,
                              const Trigger& trigger);
    void ApplyLeverVisualTransform(NSRender::Render& render,
                                   const Trigger& trigger);
    void AddButtonLocatorLight(NSRender::Render& render,
                               const Trigger& trigger);
    void ActivateButtonLight(NSRender::Render& render,
                             const Trigger& trigger);
    void DeactivateButtonLight(NSRender::Render& render,
                               const Trigger& trigger);
    void DeactivateButtonLocatorLight(NSRender::Render& render,
                                      const Trigger& trigger);
    void PlayMovementStartSound(const Trigger& trigger);
    void PlayMovementStopSound();

    std::vector<Trigger> m_triggers;
    bool m_buttonLightsActive = false;
    float m_buttonLightsElapsed = 0.0f;
    bool m_targetMoving = false;
};
