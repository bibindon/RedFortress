#include "PickupManager.h"

#include "DestructibleManager.h"
#include "GameAudio.h"
#include "InventoryManager.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace
{
const int kStarDurationFrames = 1200;
const float kStarPickupDistance = 1.0f;
const float kStarRotationRadiansPerFrame = D3DX_PI / 120.0f;
const std::wstring kStarModelPath = L"res\\model\\itemIconStar\\itemIconStar.x";
const std::wstring kSpeedUpModelPath = L"res\\model\\itemIconPowerUp\\itemIconPowerUp.x";
const std::wstring kStarItemId = L"star_power_up";
const std::wstring kSpeedUpItemId = L"speed_up";
const float kSpeedUpPickupDistance = 1.0f;
const float kPickupIconScale = 1.0f;
const int kMaxSpeedLevel = 8;

NSRender::BlinkMode ToRenderBlinkMode(const StarBlinkMode mode)
{
    if (mode == StarBlinkMode::PinkWhite)
    {
        return NSRender::BlinkMode::PinkWhiteFlash;
    }

    if (mode == StarBlinkMode::CyanWhite)
    {
        return NSRender::BlinkMode::CyanWhiteFlash;
    }

    return NSRender::BlinkMode::StarFlash;
}

bool TryParseFloat(const std::wstring& text, float* outValue)
{
    if (outValue == nullptr)
    {
        return false;
    }

    try
    {
        *outValue = std::stof(text);
    }
    catch (...)
    {
        return false;
    }

    return true;
}
}

void PickupManager::Initialize(NSRender::Render& render, InventoryManager& inventory)
{
    m_render = &render;
    m_inventory = &inventory;
}

void PickupManager::Clear()
{
    if (m_render != nullptr)
    {
        for (StarPickup& pickup : m_starPickups)
        {
            if (pickup.meshId >= 0)
            {
                m_render->RemoveMeshMix(pickup.meshId);
                pickup.meshId = -1;
            }
        }

        if (m_speedUpMeshId >= 0)
        {
            m_render->RemoveMeshMix(m_speedUpMeshId);
            m_speedUpMeshId = -1;
        }
    }

    m_starPickups.clear();
}

void PickupManager::LoadForStage(const std::wstring& starCsvPath, const std::wstring& speedUpCsvPath)
{
    Clear();
    if (m_render == nullptr)
    {
        return;
    }

    const std::vector<D3DXVECTOR3> starPositions = LoadPickupPositions(starCsvPath);
    for (const D3DXVECTOR3& position : starPositions)
    {
        StarPickup pickup;
        pickup.position = position;
        pickup.meshId = AddStarMesh(position);
        m_starPickups.push_back(pickup);
    }

    if (LoadPickupPosition(speedUpCsvPath, &m_speedUpPosition))
    {
        m_speedUpMeshId = m_render->AddMeshMix(kSpeedUpModelPath,
                                               m_speedUpPosition,
                                               D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                               kPickupIconScale,
                                               -1.0f,
                                               false,
                                               false,
                                               false);
    }
}

void PickupManager::ResetPlayerEffects()
{
    m_starPowerupFrames = 0;
    GameAudio::StopHyperMode();
    m_baseSpeedLevel = 1;
    m_speedLevel = 1;
}

void PickupManager::RespawnStars()
{
    if (m_render == nullptr)
    {
        return;
    }

    for (StarPickup& pickup : m_starPickups)
    {
        if (pickup.meshId < 0)
        {
            pickup.meshId = AddStarMesh(pickup.position);
        }
    }
}

void PickupManager::ResetTemporaryEffects()
{
    m_starPowerupFrames = 0;
    GameAudio::StopHyperMode();
    m_speedLevel = m_baseSpeedLevel;
}

void PickupManager::UpdateTimers()
{
    if (m_starPowerupFrames > 0)
    {
        --m_starPowerupFrames;
        if (m_starPowerupFrames <= 0)
        {
            GameAudio::StopHyperMode();
            m_speedLevel = m_baseSpeedLevel;
        }
    }
}

void PickupManager::UpdatePickups(const D3DXVECTOR3& playerPosition,
                                  const int playerMeshId,
                                  DestructibleManager& destructibleManager)
{
    if (m_render == nullptr || m_inventory == nullptr)
    {
        return;
    }

    for (StarPickup& pickup : m_starPickups)
    {
        if (pickup.meshId < 0)
        {
            continue;
        }

        pickup.rotationY += kStarRotationRadiansPerFrame;
        if (pickup.rotationY >= D3DX_PI * 2.0f)
        {
            pickup.rotationY -= D3DX_PI * 2.0f;
        }
        m_render->SetMeshMixRotY(pickup.meshId, pickup.rotationY);

        const D3DXVECTOR3 diff = playerPosition - pickup.position;
        if (D3DXVec3Length(&diff) <= kStarPickupDistance)
        {
            m_render->RemoveMeshMix(pickup.meshId);
            pickup.meshId = -1;
            ActivateStar(playerMeshId);
            break;
        }
    }

    if (m_baseSpeedLevel < kMaxSpeedLevel && m_speedUpMeshId >= 0)
    {
        const D3DXVECTOR3 diff = playerPosition - m_speedUpPosition;
        if (D3DXVec3Length(&diff) <= kSpeedUpPickupDistance)
        {
            m_render->RemoveMeshMix(m_speedUpMeshId);
            m_speedUpMeshId = -1;
            if (AddSpeedLevel())
            {
                GameAudio::PlayPowerUp();
            }
        }
    }

    const std::vector<DroppedRedCube>& cubes = destructibleManager.GetDroppedRedCubes();
    for (std::size_t i = 0; i < cubes.size(); ++i)
    {
        if (cubes[i].meshId < 0)
        {
            continue;
        }
        if (cubes[i].pickupWaitFrames > 0)
        {
            continue;
        }

        const D3DXVECTOR3 diff = playerPosition - cubes[i].position;
        if (D3DXVec3Length(&diff) <= kSpeedUpPickupDistance)
        {
            destructibleManager.RemoveDroppedRedCube(*m_render, i);
            if (cubes[i].type == DestructibleDropType::AmmoHeart)
            {
                if (m_ammoRecoveredCallback)
                {
                    m_ammoRecoveredCallback();
                }
            }
            else
            {
                m_inventory->AddItem(cubes[i].itemId);
                if (m_itemCollectedCallback)
                {
                    m_itemCollectedCallback(cubes[i].itemId, 1);
                }
                GameAudio::PlayItemGet();
            }
        }
    }
}

void PickupManager::ActivateStar(const int playerMeshId)
{
    m_starPowerupFrames = kStarDurationFrames;
    m_speedLevel = kMaxSpeedLevel;
    if (m_starActivatedCallback)
    {
        m_starActivatedCallback();
    }
    if (m_itemCollectedCallback)
    {
        m_itemCollectedCallback(kStarItemId, 1);
    }
    GameAudio::StartHyperMode();
    if (m_render != nullptr && playerMeshId >= 0)
    {
        m_render->StartMeshMixSkinAnimBlink(playerMeshId,
                                            kStarDurationFrames,
                                            2,
                                            ToRenderBlinkMode(m_starBlinkMode));
    }
}

bool PickupManager::AddSpeedLevel()
{
    if (m_baseSpeedLevel >= kMaxSpeedLevel)
    {
        return false;
    }

    ++m_baseSpeedLevel;
    if (IsStarActive())
    {
        m_speedLevel = kMaxSpeedLevel;
    }
    else
    {
        m_speedLevel = m_baseSpeedLevel;
    }

    if (m_itemCollectedCallback)
    {
        m_itemCollectedCallback(kSpeedUpItemId, 1);
    }

    return true;
}

void PickupManager::SetItemCollectedCallback(std::function<void(const std::wstring&, int)> callback)
{
    m_itemCollectedCallback = std::move(callback);
}

void PickupManager::SetAmmoRecoveredCallback(std::function<void()> callback)
{
    m_ammoRecoveredCallback = std::move(callback);
}

void PickupManager::SetStarActivatedCallback(std::function<void()> callback)
{
    m_starActivatedCallback = std::move(callback);
}

void PickupManager::SetStarBlinkMode(const StarBlinkMode mode)
{
    m_starBlinkMode = mode;
}

void PickupManager::SetSpeedLevel(const int speedLevel)
{
    if (speedLevel < 1)
    {
        m_baseSpeedLevel = 1;
        m_speedLevel = 1;
        return;
    }

    if (speedLevel > kMaxSpeedLevel)
    {
        m_baseSpeedLevel = kMaxSpeedLevel;
        m_speedLevel = kMaxSpeedLevel;
        return;
    }

    m_baseSpeedLevel = speedLevel;
    if (IsStarActive())
    {
        m_speedLevel = kMaxSpeedLevel;
    }
    else
    {
        m_speedLevel = m_baseSpeedLevel;
    }
}

int PickupManager::GetSpeedLevel() const
{
    return m_speedLevel;
}

int PickupManager::GetMaxSpeedLevel() const
{
    return kMaxSpeedLevel;
}

bool PickupManager::IsStarActive() const
{
    return m_starPowerupFrames > 0;
}

float PickupManager::GetRunSpeedMultiplier() const
{
    const float minMultiplier = 1.2f;
    const float maxMultiplier = 2.0f;
    const float levelRange = static_cast<float>(kMaxSpeedLevel - 1);
    const float speedLevelOffset = static_cast<float>(m_speedLevel - 1);
    return minMultiplier + ((maxMultiplier - minMultiplier) / levelRange) * speedLevelOffset;
}

bool PickupManager::LoadPickupPosition(const std::wstring& csvPath, D3DXVECTOR3* outPosition) const
{
    if (outPosition == nullptr || csvPath.empty())
    {
        return false;
    }

    std::wifstream file(NSRender::Util::GetExeDir() + csvPath);
    if (!file.is_open())
    {
        return false;
    }

    std::wstring line;
    std::getline(file, line);
    if (!std::getline(file, line))
    {
        return false;
    }

    std::wstringstream ss(line);
    std::wstring cell;
    float posX = 0.0f;
    float posY = 0.0f;
    float posZ = 0.0f;
    if (std::getline(ss, cell, L','))
    {
        if (!TryParseFloat(cell, &posX))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (std::getline(ss, cell, L','))
    {
        if (!TryParseFloat(cell, &posY))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    if (std::getline(ss, cell, L','))
    {
        if (!TryParseFloat(cell, &posZ))
        {
            return false;
        }
    }
    else
    {
        return false;
    }

    *outPosition = D3DXVECTOR3(posX, posY, posZ);
    return true;
}

std::vector<D3DXVECTOR3> PickupManager::LoadPickupPositions(const std::wstring& csvPath) const
{
    std::vector<D3DXVECTOR3> positions;
    if (csvPath.empty())
    {
        return positions;
    }

    std::wifstream file(NSRender::Util::GetExeDir() + csvPath);
    if (!file.is_open())
    {
        return positions;
    }

    std::wstring line;
    std::getline(file, line);
    while (std::getline(file, line))
    {
        std::wstringstream ss(line);
        std::wstring cell;
        D3DXVECTOR3 position(0.0f, 0.0f, 0.0f);

        if (!std::getline(ss, cell, L',') || !TryParseFloat(cell, &position.x))
        {
            continue;
        }
        if (!std::getline(ss, cell, L',') || !TryParseFloat(cell, &position.y))
        {
            continue;
        }
        if (!std::getline(ss, cell, L',') || !TryParseFloat(cell, &position.z))
        {
            continue;
        }

        positions.push_back(position);
    }

    return positions;
}

int PickupManager::AddStarMesh(const D3DXVECTOR3& position) const
{
    if (m_render == nullptr)
    {
        return -1;
    }

    return m_render->AddMeshMix(kStarModelPath,
                                position,
                                D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                kPickupIconScale,
                                -1.0f,
                                false,
                                false,
                                false);
}
