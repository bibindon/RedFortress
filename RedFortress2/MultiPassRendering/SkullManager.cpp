#include "SkullManager.h"

#include "EnemyBase.h"
#include "GameAudio.h"
#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <sstream>

namespace
{
    const std::wstring kSkullModelPath = L"res\\model\\skull\\skull.x";
    const std::wstring kSkullCollisionModelPath =
        L"res\\model\\skull\\skull_collision.x";
    const D3DXVECTOR3 kCollisionRotation(0.0f, 0.0f, 0.0f);
    const D3DXVECTOR3 kCollisionScale(1.0f, 1.0f, 1.0f);
    const D3DXVECTOR3 kDisabledCollisionPosition(0.0f, -10000.0f, 0.0f);
    const float kTargetFrameSeconds = 1.0f / 60.0f;
    const float kGrabDistance = 1.5f;
    const float kSkullRadius = 0.38f;
    const float kSkullCollisionCenterY = 0.38f;
    const float kGravity = 15.0f;
    const float kThrowForwardSpeed = 11.0f;
    const float kThrowUpSpeed = 3.5f;
    const float kThrowStartForwardOffset = 0.8f;
    const float kThrowStartHeight = 1.15f;
    const float kHeldForwardOffset = 0.65f;
    const float kHeldHeightOffset = 1.0f;
    const float kFallRemovalY = -50.0f;
    const int kRespawnFrames = 600;
    const std::size_t kMaximumSkullCount = 10;

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
        std::vector<std::wstring> result;
        std::wstringstream stream(line);
        std::wstring cell;
        while (std::getline(stream, cell, L','))
        {
            result.push_back(Trim(cell));
        }
        return result;
    }

    bool IsSkullTouchingEnemy(const SkullObject& skull, const EnemyBase& enemy)
    {
        const D3DXVECTOR3 enemyPosition = enemy.GetPosition();
        const float deltaX = skull.position.x - enemyPosition.x;
        const float deltaZ = skull.position.z - enemyPosition.z;
        const float combinedRadius = kSkullRadius + enemy.GetPhysicsRadius();
        const float horizontalDistanceSquared = deltaX * deltaX + deltaZ * deltaZ;
        if (horizontalDistanceSquared > combinedRadius * combinedRadius)
        {
            return false;
        }

        const float skullCenterY = skull.position.y + kSkullCollisionCenterY;
        const float enemyMinimumY = enemyPosition.y - enemy.GetHeight() * 0.5f - kSkullRadius;
        const float enemyMaximumY = enemyPosition.y + enemy.GetHeight() * 0.5f + kSkullRadius;
        return skullCenterY >= enemyMinimumY && skullCenterY <= enemyMaximumY;
    }
}

void SkullManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
    m_skulls.clear();
    m_spawnPoints.clear();
    m_nextSerial = 1;
    m_nextCreationOrder = 1;
    m_heldSkullSerial = 0;
}

void SkullManager::LoadForStage(NSRender::Render& render, const std::wstring& csvPath)
{
    Clear(render);
    m_render = &render;

    if (csvPath.empty())
    {
        return;
    }

    std::wifstream file(NSRender::Util::GetExeDir() + csvPath);
    if (!file.is_open())
    {
        return;
    }

    std::wstring line;
    bool isFirstLine = true;
    while (std::getline(file, line))
    {
        if (isFirstLine)
        {
            isFirstLine = false;
            continue;
        }

        const std::vector<std::wstring> cells = SplitCsvLine(line);
        if (cells.size() < 5)
        {
            continue;
        }

        SkullSpawnPoint spawnPoint;
        spawnPoint.id = std::stoi(cells[0]);
        spawnPoint.position.x = std::stof(cells[1]);
        spawnPoint.position.y = std::stof(cells[2]);
        spawnPoint.position.z = std::stof(cells[3]);
        spawnPoint.rotationY = D3DXToRadian(std::stof(cells[4]));
        m_spawnPoints.push_back(spawnPoint);
    }

    for (SkullSpawnPoint& spawnPoint : m_spawnPoints)
    {
        SpawnAtPoint(render, spawnPoint);
    }
}

void SkullManager::Clear(NSRender::Render& render)
{
    for (SkullObject& skull : m_skulls)
    {
        if (skull.meshId >= 0)
        {
            render.RemoveMeshMix(skull.meshId);
        }
        if (skull.physicsId >= 0)
        {
            PhysicsLib::PhysicsLib::RemoveObject(skull.physicsId);
        }
    }

    m_skulls.clear();
    m_spawnPoints.clear();
    m_heldSkullSerial = 0;
}

void SkullManager::Update(NSRender::Render& render,
                          const D3DXVECTOR3& playerPosition,
                          const float playerYaw,
                          const std::vector<std::unique_ptr<EnemyBase>>& enemies,
                          const EnemyHitCallback& enemyHitCallback)
{
    for (SkullSpawnPoint& spawnPoint : m_spawnPoints)
    {
        if (spawnPoint.readySkullSerial != 0)
        {
            continue;
        }

        if (spawnPoint.respawnFrames > 0)
        {
            --spawnPoint.respawnFrames;
        }
        if (spawnPoint.respawnFrames <= 0)
        {
            SpawnAtPoint(render, spawnPoint);
        }
    }

    for (std::size_t index = 0; index < m_skulls.size();)
    {
        SkullObject& skull = m_skulls[index];
        if (skull.state == SkullState::Flying)
        {
            UpdateFlyingSkull(render, skull, enemies, enemyHitCallback);
        }
        UpdateCollisionTransform(skull);

        if (skull.position.y < kFallRemovalY && skull.state != SkullState::Held)
        {
            RemoveSkull(render, index);
            continue;
        }
        ++index;
    }

    SkullObject* heldSkull = FindHeldSkull();
    if (heldSkull != nullptr)
    {
        const D3DXVECTOR3 forward(-sinf(playerYaw), 0.0f, -cosf(playerYaw));
        heldSkull->position = playerPosition + forward * kHeldForwardOffset;
        heldSkull->position.y += kHeldHeightOffset;
        heldSkull->rotation.y = playerYaw;
        UpdateWorldMatrix(render, *heldSkull);
    }
}

bool SkullManager::HandleLeftClick(NSRender::Render& render,
                                   const D3DXVECTOR3& playerPosition,
                                   const float playerYaw)
{
    SkullObject* heldSkull = FindHeldSkull();
    if (heldSkull != nullptr)
    {
        const D3DXVECTOR3 forward(-sinf(playerYaw), 0.0f, -cosf(playerYaw));
        heldSkull->position = playerPosition + forward * kThrowStartForwardOffset;
        heldSkull->position.y += kThrowStartHeight;
        heldSkull->velocity = forward * kThrowForwardSpeed;
        heldSkull->velocity.y = kThrowUpSpeed;
        heldSkull->state = SkullState::Flying;
        heldSkull->hitEnemyDuringFlight = false;
        m_heldSkullSerial = 0;
        UpdateCollisionTransform(*heldSkull);
        UpdateWorldMatrix(render, *heldSkull);
        GameAudio::PlaySkullThrow();
        return true;
    }

    SkullObject* nearestSkull = nullptr;
    float nearestDistanceSquared = kGrabDistance * kGrabDistance;
    for (SkullObject& skull : m_skulls)
    {
        if (skull.state != SkullState::Resting)
        {
            continue;
        }

        D3DXVECTOR3 difference = skull.position - playerPosition;
        const float distanceSquared = D3DXVec3LengthSq(&difference);
        if (distanceSquared <= nearestDistanceSquared)
        {
            nearestDistanceSquared = distanceSquared;
            nearestSkull = &skull;
        }
    }

    if (nearestSkull == nullptr)
    {
        return false;
    }

    NotifySpawnPointTaken(*nearestSkull);
    nearestSkull->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    nearestSkull->angularVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    nearestSkull->state = SkullState::Held;
    m_heldSkullSerial = nearestSkull->serial;
    UpdateCollisionTransform(*nearestSkull);

    const D3DXVECTOR3 forward(-sinf(playerYaw), 0.0f, -cosf(playerYaw));
    nearestSkull->position = playerPosition + forward * kHeldForwardOffset;
    nearestSkull->position.y += kHeldHeightOffset;
    nearestSkull->rotation.y = playerYaw;
    UpdateWorldMatrix(render, *nearestSkull);
    return true;
}

void SkullManager::ReleaseHeld(NSRender::Render& render, const D3DXVECTOR3& position)
{
    SkullObject* heldSkull = FindHeldSkull();
    if (heldSkull == nullptr)
    {
        return;
    }

    heldSkull->position = position;
    heldSkull->velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    heldSkull->rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    heldSkull->angularVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    heldSkull->state = SkullState::Resting;
    heldSkull->hitEnemyDuringFlight = false;
    m_heldSkullSerial = 0;
    UpdateCollisionTransform(*heldSkull);
    UpdateWorldMatrix(render, *heldSkull);
}

bool SkullManager::IsHolding() const
{
    return FindHeldSkull() != nullptr;
}

std::size_t SkullManager::GetSkullCount() const
{
    return m_skulls.size();
}
const std::vector<SkullObject>& SkullManager::GetSkulls() const
{
    return m_skulls;
}


bool SkullManager::SpawnAtPoint(NSRender::Render& render, SkullSpawnPoint& spawnPoint)
{
    if (!EnsureCapacityForSpawn(render))
    {
        return false;
    }

    SkullObject skull;
    skull.serial = m_nextSerial++;
    skull.creationOrder = m_nextCreationOrder++;
    skull.spawnId = spawnPoint.id;
    skull.position = spawnPoint.position;
    skull.rotation.y = spawnPoint.rotationY;
    skull.meshId = render.AddMeshMix(kSkullModelPath,
                                     skull.position,
                                     skull.rotation,
                                     1.0f,
                                     -1.0f,
                                     false,
                                     false,
                                     false);
    if (skull.meshId < 0)
    {
        return false;
    }

    skull.physicsId = PhysicsLib::PhysicsLib::Load(
        kSkullCollisionModelPath.c_str(),
        PhysicsLib::PhysicsLib::ObjectType::Slide,
        0.0f);
    if (skull.physicsId < 0)
    {
        std::abort();
    }
    UpdateCollisionTransform(skull);

    spawnPoint.readySkullSerial = skull.serial;
    spawnPoint.respawnFrames = 0;
    m_skulls.push_back(skull);
    return true;
}

bool SkullManager::EnsureCapacityForSpawn(NSRender::Render& render)
{
    while (m_skulls.size() >= kMaximumSkullCount)
    {
        std::size_t oldestIndex = m_skulls.size();
        std::uint64_t oldestOrder = (std::numeric_limits<std::uint64_t>::max)();
        for (std::size_t index = 0; index < m_skulls.size(); ++index)
        {
            const SkullObject& skull = m_skulls[index];
            if (skull.state == SkullState::Held || skull.spawnId >= 0)
            {
                continue;
            }
            if (skull.creationOrder < oldestOrder)
            {
                oldestOrder = skull.creationOrder;
                oldestIndex = index;
            }
        }

        if (oldestIndex >= m_skulls.size())
        {
            return false;
        }
        RemoveSkull(render, oldestIndex);
    }
    return true;
}

void SkullManager::RemoveSkull(NSRender::Render& render, const std::size_t index)
{
    if (index >= m_skulls.size())
    {
        return;
    }

    const SkullObject& skull = m_skulls[index];
    if (skull.meshId >= 0)
    {
        render.RemoveMeshMix(skull.meshId);
    }
    if (skull.physicsId >= 0)
    {
        PhysicsLib::PhysicsLib::RemoveObject(skull.physicsId);
    }

    if (skull.serial == m_heldSkullSerial)
    {
        m_heldSkullSerial = 0;
    }

    if (skull.spawnId >= 0)
    {
        for (SkullSpawnPoint& spawnPoint : m_spawnPoints)
        {
            if (spawnPoint.id == skull.spawnId && spawnPoint.readySkullSerial == skull.serial)
            {
                spawnPoint.readySkullSerial = 0;
                spawnPoint.respawnFrames = 0;
                break;
            }
        }
    }

    m_skulls.erase(m_skulls.begin() + index);
}

void SkullManager::NotifySpawnPointTaken(SkullObject& skull)
{
    if (skull.spawnId < 0)
    {
        return;
    }

    for (SkullSpawnPoint& spawnPoint : m_spawnPoints)
    {
        if (spawnPoint.id == skull.spawnId && spawnPoint.readySkullSerial == skull.serial)
        {
            spawnPoint.readySkullSerial = 0;
            spawnPoint.respawnFrames = kRespawnFrames;
            skull.spawnId = -1;
            return;
        }
    }
    skull.spawnId = -1;
}

void SkullManager::UpdateFlyingSkull(
    NSRender::Render& render,
    SkullObject& skull,
    const std::vector<std::unique_ptr<EnemyBase>>& enemies,
    const EnemyHitCallback& enemyHitCallback)
{
    skull.velocity.y -= kGravity * kTargetFrameSeconds;
    const D3DXVECTOR3 collisionPosition =
        skull.position + D3DXVECTOR3(0.0f, kSkullCollisionCenterY, 0.0f);
    D3DXVECTOR3 nextCollisionPosition = collisionPosition;
    D3DXVECTOR3 nextVelocity = skull.velocity;
    D3DXVECTOR3 hitNormal(0.0f, 0.0f, 0.0f);

    const bool collided = PhysicsLib::PhysicsLib::CheckCollide(collisionPosition,
                                                                skull.velocity,
                                                                PhysicsLib::PhysicsLib::ShapeType::Sphere,
                                                                &nextCollisionPosition,
                                                                &nextVelocity,
                                                                nullptr,
                                                                nullptr,
                                                                kSkullRadius,
                                                                0.0f,
                                                                nullptr,
                                                                &hitNormal);
    skull.position = nextCollisionPosition - D3DXVECTOR3(0.0f, kSkullCollisionCenterY, 0.0f);
    skull.velocity = nextVelocity;

    if (collided)
    {
        const float velocityIntoSurface = D3DXVec3Dot(&skull.velocity, &hitNormal);
        if (hitNormal.y > 0.5f)
        {
            // 地面・オブジェクトの上面に着地したら即停止
            skull.state = SkullState::Resting;
            skull.velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            skull.angularVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
            GameAudio::PlaySkullLand();
        }
        else
        {
            // 壁などの側面: めり込み方向の速度だけ打ち消して落ちる
            if (velocityIntoSurface < 0.0f)
            {
                skull.velocity -= hitNormal * velocityIntoSurface;
            }
            skull.velocity.x *= 0.25f;
            skull.velocity.z *= 0.25f;
        }
    }

    if (!skull.hitEnemyDuringFlight)
    {
        for (const std::unique_ptr<EnemyBase>& enemy : enemies)
        {
            if (enemy == nullptr || enemy->IsDead())
            {
                continue;
            }
            if (IsSkullTouchingEnemy(skull, *enemy))
            {
                skull.hitEnemyDuringFlight = true;
                GameAudio::PlaySkullHit();
                enemyHitCallback(*enemy, skull.position);
                skull.velocity.x *= -0.25f;
                skull.velocity.z *= -0.25f;
                if (skull.velocity.y < 2.0f)
                {
                    skull.velocity.y = 2.0f;
                }
                break;
            }
        }
    }

    UpdateWorldMatrix(render, skull);
}

void SkullManager::UpdateCollisionTransform(const SkullObject& skull)
{
    if (skull.physicsId < 0)
    {
        return;
    }

    D3DXVECTOR3 collisionPosition = kDisabledCollisionPosition;
    if (skull.state == SkullState::Resting)
    {
        collisionPosition = skull.position;
    }
    PhysicsLib::PhysicsLib::SetTransform(skull.physicsId,
                                         collisionPosition,
                                         kCollisionRotation,
                                         kCollisionScale);
    PhysicsLib::PhysicsLib::SetVelocity(skull.physicsId,
                                        D3DXVECTOR3(0.0f, 0.0f, 0.0f));
}

void SkullManager::UpdateWorldMatrix(NSRender::Render& render, const SkullObject& skull)
{
    if (skull.meshId < 0)
    {
        return;
    }

    D3DXMATRIX rotationMatrix;
    D3DXMatrixRotationYawPitchRoll(&rotationMatrix,
                                   skull.rotation.y,
                                   skull.rotation.x,
                                   skull.rotation.z);
    D3DXMATRIX translationMatrix;
    D3DXMatrixTranslation(&translationMatrix,
                          skull.position.x,
                          skull.position.y,
                          skull.position.z);
    render.SetMeshMixWorldMatrix(skull.meshId, rotationMatrix * translationMatrix);
}

SkullObject* SkullManager::FindHeldSkull()
{
    if (m_heldSkullSerial == 0)
    {
        return nullptr;
    }

    for (SkullObject& skull : m_skulls)
    {
        if (skull.serial == m_heldSkullSerial && skull.state == SkullState::Held)
        {
            return &skull;
        }
    }
    return nullptr;
}

const SkullObject* SkullManager::FindHeldSkull() const
{
    if (m_heldSkullSerial == 0)
    {
        return nullptr;
    }

    for (const SkullObject& skull : m_skulls)
    {
        if (skull.serial == m_heldSkullSerial && skull.state == SkullState::Held)
        {
            return &skull;
        }
    }
    return nullptr;
}
