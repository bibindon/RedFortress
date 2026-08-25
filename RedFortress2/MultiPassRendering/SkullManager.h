#pragma once

#include <cstdint>
#include <d3dx9.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class EnemyBase;

enum class SkullState
{
    Resting,
    Held,
    Flying
};

struct SkullObject
{
    std::uint64_t serial = 0;
    std::uint64_t creationOrder = 0;
    int meshId = -1;
    int physicsId = -1;
    int spawnId = -1;
    SkullState state = SkullState::Resting;
    D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 angularVelocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    bool hitEnemyDuringFlight = false;
};

struct SkullSpawnPoint
{
    int id = -1;
    D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    float rotationY = 0.0f;
    int respawnFrames = 0;
    std::uint64_t readySkullSerial = 0;
};

class SkullManager
{
public:
    using EnemyHitCallback = std::function<void(EnemyBase&, const D3DXVECTOR3&)>;

    void Initialize(NSRender::Render& render);
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Clear(NSRender::Render& render);
    void ResetForRespawn(NSRender::Render& render);
    void Update(NSRender::Render& render,
                const D3DXVECTOR3& playerPosition,
                float playerYaw,
                const std::vector<std::unique_ptr<EnemyBase>>& enemies,
                const EnemyHitCallback& enemyHitCallback);

    bool HandleLeftClick(NSRender::Render& render,
                         const D3DXVECTOR3& playerPosition,
                         float playerYaw);
    void ReleaseHeld(NSRender::Render& render, const D3DXVECTOR3& position);
    bool IsHolding() const;
    std::size_t GetSkullCount() const;
    const std::vector<SkullObject>& GetSkulls() const;

private:
    bool SpawnAtPoint(NSRender::Render& render, SkullSpawnPoint& spawnPoint);
    bool EnsureCapacityForSpawn(NSRender::Render& render);
    void RemoveSkull(NSRender::Render& render, std::size_t index);
    void NotifySpawnPointTaken(SkullObject& skull);
    void UpdateFlyingSkull(NSRender::Render& render,
                           SkullObject& skull,
                           const std::vector<std::unique_ptr<EnemyBase>>& enemies,
                           const EnemyHitCallback& enemyHitCallback);
    void UpdateCollisionTransform(const SkullObject& skull);
    void UpdateWorldMatrix(NSRender::Render& render, const SkullObject& skull);
    SkullObject* FindHeldSkull();
    const SkullObject* FindHeldSkull() const;

    NSRender::Render* m_render = nullptr;
    std::vector<SkullObject> m_skulls;
    std::vector<SkullSpawnPoint> m_spawnPoints;
    std::uint64_t m_nextSerial = 1;
    std::uint64_t m_nextCreationOrder = 1;
    std::uint64_t m_heldSkullSerial = 0;
};
