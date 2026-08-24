#include "AttackTriggerManager.h"

#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"
#include "GameAudio.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace
{
    const std::wstring kLeverModelPath =
        L"res\\model\\attack_trigger\\lever.x";
    const std::wstring kRopeModelPath =
        L"res\\model\\attack_trigger\\rope.x";
    const std::wstring kButtonInactiveModelPath =
        L"res\\model\\pressure_plate\\pressure_plate_black.x";
    const std::wstring kButtonActiveModelPath =
        L"res\\model\\pressure_plate\\pressure_plate_green.x";
    const float kTimedButtonDurationSeconds = 10.0f;
    const float kButtonLightOffsetY = 2.5f;
    const float kButtonLocatorBrightness = 0.10f;
    const float kButtonLocatorRange = 2.0f;
    const float kTargetAngle = D3DX_PI * 0.5f;
    const float kRotationSpeed = D3DX_PI * 0.5f;
    const float kLiftSpeed = 3.0f;
    const float kPlayerAttackCenterHeight = 1.0f;

    std::wstring Trim(const std::wstring& value)
    {
        std::size_t start = 0;
        while (start < value.size() && (value[start] == L' ' || value[start] == L'\t'))
        {
            ++start;
        }

        std::size_t end = value.size();
        while (end > start &&
               (value[end - 1] == L' ' || value[end - 1] == L'\t' || value[end - 1] == L'\r'))
        {
            --end;
        }
        return value.substr(start, end - start);
    }

    std::vector<std::wstring> SplitCsvLine(const std::wstring& line)
    {
        std::vector<std::wstring> cells;
        std::wstringstream stream(line);
        std::wstring cell;
        while (std::getline(stream, cell, L','))
        {
            cells.push_back(Trim(cell));
        }
        return cells;
    }

    AttackTriggerType ParseTriggerType(const std::wstring& value)
    {
        if (value == L"Lever")
        {
            return AttackTriggerType::Lever;
        }
        if (value == L"LeverLift")
        {
            return AttackTriggerType::LeverLift;
        }
        if (value == L"Rope")
        {
            return AttackTriggerType::Rope;
        }
        if (value == L"Button" || value == L"TimedButton")
        {
            return AttackTriggerType::Button;
        }
        std::abort();
    }

    D3DXVECTOR3 ParseAxis(const std::wstring& value)
    {
        if (value == L"X")
        {
            return D3DXVECTOR3(1.0f, 0.0f, 0.0f);
        }
        if (value == L"Y")
        {
            return D3DXVECTOR3(0.0f, 1.0f, 0.0f);
        }
        if (value == L"Z")
        {
            return D3DXVECTOR3(0.0f, 0.0f, 1.0f);
        }
        std::abort();
    }
}

void AttackTriggerManager::Initialize()
{
    m_triggers.clear();
    m_buttonLightsActive = false;
    m_buttonLightsElapsed = 0.0f;
    m_targetMoving = false;
}

void AttackTriggerManager::LoadForStage(NSRender::Render& render,
                                        const std::wstring& csvPath)
{
    Clear(render);

    if (csvPath.empty())
    {
        return;
    }

    std::wifstream file(NSRender::Util::GetExeDir() + csvPath);
    if (!file.is_open())
    {
        return;
    }

    std::unordered_set<int> loadedTriggerIds;
    std::wstring line;
    bool isFirstLine = true;
    while (std::getline(file, line))
    {
        if (isFirstLine)
        {
            isFirstLine = false;
            continue;
        }

        if (Trim(line).empty())
        {
            continue;
        }

        const std::vector<std::wstring> cells = SplitCsvLine(line);
        if (cells.size() < 11)
        {
            std::abort();
        }

        Trigger trigger;
        trigger.id = std::stoi(cells[0]);
        trigger.type = ParseTriggerType(cells[1]);
        trigger.triggerPosition.x = std::stof(cells[2]);
        trigger.triggerPosition.y = std::stof(cells[3]);
        trigger.triggerPosition.z = std::stof(cells[4]);
        trigger.targetCsvId = std::stoi(cells[5]);
        trigger.targetAxis = ParseAxis(cells[6]);
        trigger.targetRotation.x = D3DXToRadian(std::stof(cells[7]));
        trigger.targetRotation.y = D3DXToRadian(std::stof(cells[8]));
        trigger.targetRotation.z = D3DXToRadian(std::stof(cells[9]));
        const float targetScale = std::stof(cells[10]);
        trigger.targetScale = D3DXVECTOR3(targetScale, targetScale, targetScale);

        if (trigger.type == AttackTriggerType::LeverLift)
        {
            if (cells.size() < 12)
            {
                std::abort();
            }
            trigger.liftHeight = std::stof(cells[11]);
            if (trigger.liftHeight <= 0.0f)
            {
                std::abort();
            }
        }

        if (trigger.type == AttackTriggerType::Button)
        {
            if (cells.size() >= 16)
            {
                trigger.lightBrightness = std::stof(cells[11]);
                trigger.lightRange = std::stof(cells[12]);
                trigger.lightColor.r = std::stof(cells[13]);
                trigger.lightColor.g = std::stof(cells[14]);
                trigger.lightColor.b = std::stof(cells[15]);
                if (trigger.lightBrightness <= 0.0f || trigger.lightRange <= 0.0f)
                {
                    std::abort();
                }
            }
            trigger.lightOwnerTag =
                std::wstring(L"attack-trigger-button-") + std::to_wstring(trigger.id);
            trigger.locatorOwnerTag =
                std::wstring(L"attack-trigger-button-locator-") + std::to_wstring(trigger.id);
        }

        if (!loadedTriggerIds.insert(trigger.id).second)
        {
            std::abort();
        }

        if (trigger.targetCsvId >= 0)
        {
            // 同じ TargetID を複数のトリガーで共有できる（レバー3: どちらのレバーでも扉が開く）。
            if (!render.TryGetCsvMeshPosition(trigger.targetCsvId, &trigger.targetPosition))
            {
                std::abort();
            }

            trigger.targetPhysicsId =
                PhysicsLib::PhysicsLib::GetCsvObjectId(trigger.targetCsvId);
            if (trigger.targetPhysicsId < 0)
            {
                std::abort();
            }
            trigger.hasTarget = true;
        }
        else if (trigger.type != AttackTriggerType::Button)
        {
            std::abort();
        }

        if (trigger.type == AttackTriggerType::Button)
        {
            trigger.visualMeshId = render.AddMeshMix(
                kButtonInactiveModelPath,
                trigger.triggerPosition,
                D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                1.0f,
                -1.0f,
                false,
                false,
                false);
            trigger.activeVisualMeshId = render.AddMeshMix(
                kButtonActiveModelPath,
                trigger.triggerPosition,
                D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                1.0f,
                -1.0f,
                false,
                false,
                false);
            if (trigger.visualMeshId < 0 || trigger.activeVisualMeshId < 0)
            {
                std::abort();
            }
            render.SetMeshMixEnabled(trigger.activeVisualMeshId, false);
            AddButtonLocatorLight(render, trigger);
        }
        else
        {
            std::wstring visualModelPath;
            if (trigger.type == AttackTriggerType::Lever ||
                trigger.type == AttackTriggerType::LeverLift)
            {
                visualModelPath = kLeverModelPath;
            }
            else
            {
                visualModelPath = kRopeModelPath;
            }
            trigger.visualMeshId = render.AddMeshMix(visualModelPath,
                                                     trigger.triggerPosition,
                                                     D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                     1.0f,
                                                     -1.0f,
                                                     false,
                                                     false,
                                                     false);
            if (trigger.visualMeshId < 0)
            {
                std::abort();
            }
        }

        m_triggers.push_back(trigger);
    }
}

void AttackTriggerManager::Clear(NSRender::Render& render)
{
    for (Trigger& trigger : m_triggers)
    {
        if (trigger.type == AttackTriggerType::Button)
        {
            DeactivateButtonLight(render, trigger);
            DeactivateButtonLocatorLight(render, trigger);
        }
        if (trigger.visualMeshId >= 0)
        {
            render.RemoveMeshMix(trigger.visualMeshId);
            trigger.visualMeshId = -1;
        }
        if (trigger.activeVisualMeshId >= 0)
        {
            render.RemoveMeshMix(trigger.activeVisualMeshId);
            trigger.activeVisualMeshId = -1;
        }
    }
    m_buttonLightsActive = false;
    m_buttonLightsElapsed = 0.0f;
    m_targetMoving = false;
    m_triggers.clear();
}

void AttackTriggerManager::Update(NSRender::Render& render,
                                  const float deltaSeconds)
{
    if (deltaSeconds <= 0.0f)
    {
        std::abort();
    }

    m_targetMoving = false;

    if (m_buttonLightsActive)
    {
        m_buttonLightsElapsed += deltaSeconds;
        if (m_buttonLightsElapsed >= kTimedButtonDurationSeconds)
        {
            m_buttonLightsActive = false;
            m_buttonLightsElapsed = 0.0f;
            for (Trigger& candidate : m_triggers)
            {
                if (candidate.type != AttackTriggerType::Button)
                {
                    continue;
                }
                candidate.buttonActive = false;
                DeactivateButtonLight(render, candidate);
            }
            PlayMovementStopSound();
        }
    }

    for (Trigger& trigger : m_triggers)
    {
        UpdateTrigger(render, trigger, deltaSeconds);
    }
}

void AttackTriggerManager::ResetLevers(NSRender::Render& render,
                                       const std::vector<int>& triggerIds)
{
    std::unordered_set<int> targetCsvIds;
    for (const Trigger& trigger : m_triggers)
    {
        if ((trigger.type == AttackTriggerType::Lever ||
             trigger.type == AttackTriggerType::LeverLift) &&
            trigger.hasTarget &&
            std::find(triggerIds.begin(), triggerIds.end(), trigger.id) != triggerIds.end())
        {
            targetCsvIds.insert(trigger.targetCsvId);
        }
    }

    for (Trigger& trigger : m_triggers)
    {
        if (trigger.type != AttackTriggerType::Lever &&
            trigger.type != AttackTriggerType::LeverLift)
        {
            continue;
        }
        if (!trigger.hasTarget ||
            targetCsvIds.find(trigger.targetCsvId) == targetCsvIds.end())
        {
            continue;
        }

        // 同じ門を共有するレバーをすべて戻し、次の更新で再び開かないようにする。
        trigger.leverActive = false;
        trigger.currentAngle = 0.0f;
        trigger.currentLift = 0.0f;
        trigger.stopSoundPlayed = false;
        ApplyTargetTransform(render, trigger);
    }
}

AttackTriggerActivation AttackTriggerManager::TryActivateInAttackRange(
    NSRender::Render& render,
    const D3DXVECTOR3& playerPosition,
    const float playerYaw,
    const float range,
    const float verticalRange,
    const float halfAngleRadians)
{
    const D3DXVECTOR3 forward(-sinf(playerYaw), 0.0f, -cosf(playerYaw));
    int nearestIndex = -1;
    float nearestDot = -1.0f;

    for (std::size_t index = 0; index < m_triggers.size(); ++index)
    {
        const Trigger& trigger = m_triggers[index];
        if (!IsReadyForAttack(trigger) ||
            !IsTargetInAttackRange(trigger,
                                   playerPosition,
                                   playerYaw,
                                   range,
                                   verticalRange,
                                   halfAngleRadians))
        {
            continue;
        }

        D3DXVECTOR3 direction = trigger.triggerPosition - playerPosition;
        if (D3DXVec3LengthSq(&direction) > 0.0001f)
        {
            D3DXVec3Normalize(&direction, &direction);
        }
        else
        {
            direction = forward;
        }

        const float dot = D3DXVec3Dot(&forward, &direction);
        if (dot > nearestDot)
        {
            nearestDot = dot;
            nearestIndex = static_cast<int>(index);
        }
    }

    if (nearestIndex < 0)
    {
        return AttackTriggerActivation::None;
    }

    Trigger& trigger = m_triggers.at(static_cast<std::size_t>(nearestIndex));
    if (trigger.type == AttackTriggerType::Lever ||
        trigger.type == AttackTriggerType::LeverLift)
    {
        trigger.leverActive = !trigger.leverActive;
        trigger.stopSoundPlayed = false;
        // 同じ TargetID を共有する LeverLift (複数レバーで同じ扉を操作) に状態を同期する。
        // どちらのレバーを操作しても両方の扉が同じように開閉する。
        if (trigger.type == AttackTriggerType::LeverLift && trigger.hasTarget)
        {
            for (Trigger& other : m_triggers)
            {
                if (&other != &trigger &&
                    other.type == AttackTriggerType::LeverLift &&
                    other.hasTarget &&
                    other.targetCsvId == trigger.targetCsvId)
                {
                    other.leverActive = trigger.leverActive;
                    other.stopSoundPlayed = false;
                }
            }
        }
        PlayMovementStartSound(trigger);
        return AttackTriggerActivation::Lever;
    }

    if (trigger.type == AttackTriggerType::Button)
    {
        m_buttonLightsActive = true;
        m_buttonLightsElapsed = 0.0f;
        for (Trigger& candidate : m_triggers)
        {
            if (candidate.type != AttackTriggerType::Button)
            {
                continue;
            }
            candidate.buttonActive = true;
            candidate.stopSoundPlayed = false;
            ActivateButtonLight(render, candidate);
        }
        PlayMovementStartSound(trigger);
        return AttackTriggerActivation::Button;
    }

    trigger.ropeUsed = true;
    trigger.stopSoundPlayed = false;
    PlayMovementStartSound(trigger);
    return AttackTriggerActivation::Rope;
}

std::size_t AttackTriggerManager::GetTriggerCount() const
{
    return m_triggers.size();
}

bool AttackTriggerManager::IsTargetMoving() const
{
    return m_targetMoving;
}

bool AttackTriggerManager::IsTargetInAttackRange(
    const Trigger& trigger,
    const D3DXVECTOR3& playerPosition,
    const float playerYaw,
    const float range,
    const float verticalRange,
    const float halfAngleRadians) const
{
    const D3DXVECTOR3 forward(-sinf(playerYaw), 0.0f, -cosf(playerYaw));
    const float attackCenterY = playerPosition.y + kPlayerAttackCenterHeight;
    const float targetCenterY = trigger.triggerPosition.y + kPlayerAttackCenterHeight;
    if (std::fabs(targetCenterY - attackCenterY) > verticalRange)
    {
        return false;
    }

    D3DXVECTOR3 direction = trigger.triggerPosition - playerPosition;
    const float distance = D3DXVec3Length(&direction);
    if (distance > range)
    {
        return false;
    }

    if (D3DXVec3LengthSq(&direction) > 0.0001f)
    {
        D3DXVec3Normalize(&direction, &direction);
    }
    else
    {
        direction = forward;
    }

    const float dot = D3DXVec3Dot(&forward, &direction);
    return dot > cosf(halfAngleRadians);
}

bool AttackTriggerManager::IsReadyForAttack(const Trigger& trigger) const
{
    if (trigger.type == AttackTriggerType::Rope && trigger.ropeUsed)
    {
        return false;
    }
    return true;
}

void AttackTriggerManager::UpdateTrigger(NSRender::Render& render,
                                          Trigger& trigger,
                                          const float deltaSeconds)
{
    if (trigger.type == AttackTriggerType::Button)
    {
        render.SetMeshMixEnabled(trigger.visualMeshId, !trigger.buttonActive);
        render.SetMeshMixEnabled(trigger.activeVisualMeshId, trigger.buttonActive);
    }

    const float previousAngle = trigger.currentAngle;
    const float previousLift = trigger.currentLift;

    float targetAngle = 0.0f;
    if (trigger.type == AttackTriggerType::LeverLift)
    {
        float targetLift = 0.0f;
        if (trigger.leverActive)
        {
            targetLift = trigger.liftHeight;
        }
        const float liftStep = kLiftSpeed * deltaSeconds;
        if (trigger.currentLift < targetLift)
        {
            trigger.currentLift = (std::min)(trigger.currentLift + liftStep, targetLift);
        }
        else if (trigger.currentLift > targetLift)
        {
            trigger.currentLift = (std::max)(trigger.currentLift - liftStep, targetLift);
        }
    }
    else
    {
        if (trigger.type == AttackTriggerType::Lever)
        {
            if (trigger.leverActive)
            {
                targetAngle = kTargetAngle;
            }
        }
        else if (trigger.type == AttackTriggerType::Rope && trigger.ropeUsed)
        {
            targetAngle = kTargetAngle;
        }
        else if (trigger.type == AttackTriggerType::Button &&
                 trigger.hasTarget &&
                 trigger.buttonActive)
        {
            targetAngle = kTargetAngle;
        }

        const float angleStep = kRotationSpeed * deltaSeconds;
        if (trigger.currentAngle < targetAngle)
        {
            trigger.currentAngle = (std::min)(trigger.currentAngle + angleStep, targetAngle);
        }
        else if (trigger.currentAngle > targetAngle)
        {
            trigger.currentAngle = (std::max)(trigger.currentAngle - angleStep, targetAngle);
        }
    }

    const bool angleChanged = trigger.currentAngle != previousAngle;
    const bool liftChanged = trigger.currentLift != previousLift;
    if ((angleChanged || liftChanged) && trigger.hasTarget)
    {
        m_targetMoving = true;
        ApplyTargetTransform(render, trigger);
    }

    bool atTarget = false;
    if (trigger.type == AttackTriggerType::LeverLift)
    {
        float targetLift = 0.0f;
        if (trigger.leverActive)
        {
            targetLift = trigger.liftHeight;
        }
        atTarget = (trigger.currentLift == targetLift);
    }
    else
    {
        atTarget = (trigger.currentAngle == targetAngle);
    }
    if (trigger.hasTarget &&
        atTarget &&
        (angleChanged || liftChanged) &&
        !trigger.stopSoundPlayed)
    {
        PlayMovementStopSound();
        trigger.stopSoundPlayed = true;
    }
}

void AttackTriggerManager::ApplyTargetTransform(NSRender::Render& render,
                                                const Trigger& trigger)
{
    D3DXVECTOR3 position = trigger.targetPosition;
    if (trigger.type == AttackTriggerType::LeverLift)
    {
        position.y += trigger.currentLift;
    }

    D3DXVECTOR3 rotation = trigger.targetRotation;
    if (trigger.type != AttackTriggerType::LeverLift)
    {
        rotation.x += trigger.targetAxis.x * trigger.currentAngle;
        rotation.y += trigger.targetAxis.y * trigger.currentAngle;
        rotation.z += trigger.targetAxis.z * trigger.currentAngle;
    }

    D3DXMATRIX scaleMatrix;
    D3DXMatrixScaling(&scaleMatrix,
                      trigger.targetScale.x,
                      trigger.targetScale.y,
                      trigger.targetScale.z);
    D3DXMATRIX rotationMatrix;
    D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
                                   rotation.y,
                                   rotation.x,
                                   rotation.z);
    D3DXMATRIX translationMatrix;
    D3DXMatrixTranslation(&translationMatrix,
                          position.x,
                          position.y,
                          position.z);
    const D3DXMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

    if (!render.SetCsvMeshWorldMatrix(trigger.targetCsvId, worldMatrix))
    {
        std::abort();
    }

    PhysicsLib::PhysicsLib::UpdateCsvTransform(trigger.targetCsvId,
                                               position,
                                               rotation,
                                               trigger.targetScale);
    if (trigger.type != AttackTriggerType::LeverLift)
    {
        PhysicsLib::PhysicsLib::SetVelocity(trigger.targetPhysicsId,
                                            D3DXVECTOR3(0.0f, 0.0f, 0.0f));
    }
}

void AttackTriggerManager::AddButtonLocatorLight(NSRender::Render& render,
                                                  const Trigger& trigger)
{
    render.RemovePointLightsByOwnerTag(trigger.locatorOwnerTag);
    const D3DXVECTOR3 lightPosition =
        trigger.triggerPosition + D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    render.AddPointLight(lightPosition,
                         kButtonLocatorBrightness,
                         D3DXCOLOR(1.0f, 0.30f, 0.08f, 1.0f),
                         NSRender::PointLightShape::Point,
                         12.0f,
                         10.0f,
                         10.0f,
                         D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                         kButtonLocatorRange,
                         trigger.locatorOwnerTag);
}

void AttackTriggerManager::ActivateButtonLight(NSRender::Render& render,
                                                const Trigger& trigger)
{
    render.RemovePointLightsByOwnerTag(trigger.lightOwnerTag);
    const D3DXVECTOR3 lightPosition =
        trigger.triggerPosition + D3DXVECTOR3(0.0f, kButtonLightOffsetY, 0.0f);
    render.AddPointLight(lightPosition,
                         trigger.lightBrightness,
                         trigger.lightColor,
                         NSRender::PointLightShape::Point,
                         12.0f,
                         10.0f,
                         10.0f,
                         D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                         trigger.lightRange,
                         trigger.lightOwnerTag);
}

void AttackTriggerManager::DeactivateButtonLight(NSRender::Render& render,
                                                  const Trigger& trigger)
{
    render.RemovePointLightsByOwnerTag(trigger.lightOwnerTag);
}

void AttackTriggerManager::DeactivateButtonLocatorLight(NSRender::Render& render,
                                                         const Trigger& trigger)
{
    render.RemovePointLightsByOwnerTag(trigger.locatorOwnerTag);
}

void AttackTriggerManager::PlayMovementStartSound(const Trigger& trigger)
{
    if (trigger.type == AttackTriggerType::Lever ||
        trigger.type == AttackTriggerType::LeverLift ||
        trigger.type == AttackTriggerType::Button)
    {
        GameAudio::PlayLeverToggle();
    }
    else
    {
        GameAudio::PlayRopeCut();
    }
}

void AttackTriggerManager::PlayMovementStopSound()
{
    GameAudio::PlayMechanismStop();
}
