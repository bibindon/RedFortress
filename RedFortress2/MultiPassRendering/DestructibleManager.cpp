#include "DestructibleManager.h"

#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"
#include <fstream>
#include <sstream>
#include <string>
#include <random>

namespace
{
    const std::wstring kModelPath = L"res\\model\\cubeWoodBreakable\\cube_wood_breakable.x";
    const std::wstring kCollisionModelPath = L"res\\model\\cubeWoodBreakable\\cube_wood_breakable_collision.x";
    const std::wstring kRedCubeModelPath = L"res\\model\\itemIconMaterial\\itemIconMaterial.x";
    const std::wstring kAmmoHeartModelPath = L"res\\model\\itemIconAmmo\\itemIconAmmo.x";
    const D3DXVECTOR3 kPhysicsDisabledPosition(0.0f, -10000.0f, 0.0f);
    const D3DXVECTOR3 kDefaultRotation(0.0f, 0.0f, 0.0f);
    const D3DXVECTOR3 kDefaultScale(1.0f, 1.0f, 1.0f);
    const float kTargetFrameSeconds = 1.0f / 60.0f;
    const float kDroppedItemGravity = 18.0f;
    const float kDroppedItemRadius = 0.25f;
    const float kDroppedItemCollisionCenterY = 0.25f;
    const float kDroppedItemSpawnHeight = 1.0f;
    const float kPlayerAttackCenterHeight = 1.0f;
    const float kDestructibleAttackTargetHeight = 0.5f;
    const std::wstring kCraftMaterialItemIds[] =
    {
        L"001", L"002", L"003", L"004", L"005", L"006", L"009",
        L"010", L"011", L"012", L"013", L"014", L"015", L"016"
    };
    const int kCraftMaterialItemCount =
        static_cast<int>(sizeof(kCraftMaterialItemIds) / sizeof(kCraftMaterialItemIds[0]));

    std::wstring Trim(const std::wstring& str)
    {
        size_t start = 0;
        while (start < str.size() && (str[start] == L' ' || str[start] == L'\t'))
        {
            ++start;
        }
        size_t end = str.size();
        while (end > start && (str[end - 1] == L' ' || str[end - 1] == L'\t' || str[end - 1] == L'\r'))
        {
            --end;
        }
        return str.substr(start, end - start);
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

    int GetRandomPercent()
    {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> dist(0, 99);
        return dist(rng);
    }

    std::wstring GetRandomCraftMaterialItemId()
    {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<int> dist(0, kCraftMaterialItemCount - 1);
        return kCraftMaterialItemIds[dist(rng)];
    }

    void DisablePhysicsObject(const int physicsId)
    {
        if (physicsId < 0)
        {
            return;
        }

        PhysicsLib::PhysicsLib::SetTransform(physicsId,
                                             kPhysicsDisabledPosition,
                                             kDefaultRotation,
                                             kDefaultScale);
    }

    void UpdateDroppedItemPhysics(NSRender::Render& render, DroppedRedCube& item)
    {
        if (item.isGrounded)
        {
            return;
        }

        item.velocity.y -= kDroppedItemGravity * kTargetFrameSeconds;

        const D3DXVECTOR3 collisionPosition =
            item.position + D3DXVECTOR3(0.0f, kDroppedItemCollisionCenterY, 0.0f);
        D3DXVECTOR3 nextCollisionPosition = collisionPosition;
        D3DXVECTOR3 nextVelocity = item.velocity;
        D3DXVECTOR3 hitNormal(0.0f, 0.0f, 0.0f);

        const bool collided = PhysicsLib::PhysicsLib::CheckCollide(collisionPosition,
                                                                    item.velocity,
                                                                    PhysicsLib::PhysicsLib::ShapeType::Sphere,
                                                                    &nextCollisionPosition,
                                                                    &nextVelocity,
                                                                    nullptr,
                                                                    nullptr,
                                                                    kDroppedItemRadius,
                                                                    0.0f,
                                                                    nullptr,
                                                                    &hitNormal);

        item.position = nextCollisionPosition - D3DXVECTOR3(0.0f, kDroppedItemCollisionCenterY, 0.0f);
        item.velocity = nextVelocity;

        if (collided && hitNormal.y > 0.0f)
        {
            item.isGrounded = true;
            item.velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        }

        if (item.meshId >= 0)
        {
            render.SetMeshMixPos(item.meshId, item.position);
        }
    }
}

void DestructibleManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
    m_objects.clear();
    m_droppedRedCubes.clear();
}

void DestructibleManager::LoadForStage(NSRender::Render& render, const std::wstring& csvPath)
{
    Clear();

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
        if (cells.size() < 3)
        {
            continue;
        }

        DestructibleObject obj;
        obj.position.x = std::stof(cells.at(0));
        obj.position.y = std::stof(cells.at(1));
        obj.position.z = std::stof(cells.at(2));

        if (cells.size() >= 4)
        {
            const int hp = std::stoi(cells.at(3));
            obj.maxHp = hp;
            obj.hp = hp;
        }

        if (cells.size() >= 5)
        {
            obj.dropItemId = cells.at(4);
            obj.hasExplicitDrop = true;
        }

        obj.meshId = m_render->AddMeshMix(kModelPath,
                                           obj.position,
                                           kDefaultRotation,
                                           1.0f);
        if (obj.meshId >= 0)
        {
            obj.physicsId = PhysicsLib::PhysicsLib::Load(kCollisionModelPath.c_str(),
                                                         PhysicsLib::PhysicsLib::ObjectType::Slide,
                                                         0.0f);
            PhysicsLib::PhysicsLib::SetTransform(obj.physicsId,
                                                 obj.position,
                                                 kDefaultRotation,
                                                 kDefaultScale);
            m_objects.push_back(obj);
        }
    }
}

void DestructibleManager::Clear()
{
    if (m_render != nullptr)
    {
        for (auto& obj : m_objects)
        {
            if (obj.meshId >= 0)
            {
                m_render->SetMeshMixDamageFlash(obj.meshId, false);
                m_render->SetMeshMixEnabled(obj.meshId, true);
                m_render->RemoveMeshMix(obj.meshId);
            }
            DisablePhysicsObject(obj.physicsId);
        }
        for (auto& cube : m_droppedRedCubes)
        {
            if (cube.meshId >= 0)
            {
                m_render->RemoveMeshMix(cube.meshId);
            }
        }
    }

    m_objects.clear();
    m_droppedRedCubes.clear();
}

void DestructibleManager::ResetForRespawn(NSRender::Render& render)
{
    m_render = &render;

    for (auto& drop : m_droppedRedCubes)
    {
        if (drop.meshId >= 0)
        {
            render.RemoveMeshMix(drop.meshId);
        }
    }
    m_droppedRedCubes.clear();

    for (auto& obj : m_objects)
    {
        if (obj.isDead || obj.meshId < 0)
        {
            obj.meshId = render.AddMeshMix(kModelPath,
                                           obj.position,
                                           kDefaultRotation,
                                           1.0f);
        }

        obj.hp = obj.maxHp;
        obj.blinkFrames = 0;

        if (obj.meshId < 0)
        {
            obj.isDead = true;
            DisablePhysicsObject(obj.physicsId);
            continue;
        }

        obj.isDead = false;
        render.SetMeshMixPos(obj.meshId, obj.position);
        render.SetMeshMixDamageFlash(obj.meshId, false);
        render.SetMeshMixEnabled(obj.meshId, true);

        if (obj.physicsId >= 0)
        {
            PhysicsLib::PhysicsLib::SetTransform(obj.physicsId,
                                                 obj.position,
                                                 kDefaultRotation,
                                                 kDefaultScale);
        }
    }
}

void DestructibleManager::Update(NSRender::Render& render)
{
    for (auto& obj : m_objects)
    {
        if (obj.isDead || obj.meshId < 0)
        {
            continue;
        }

        if (obj.blinkFrames > 0)
        {
            --obj.blinkFrames;
            const bool flash = ((obj.blinkFrames / kDestructibleBlinkInterval) % 2) == 0;
            render.SetMeshMixEnabled(obj.meshId, true);
            render.SetMeshMixDamageFlash(obj.meshId, flash);

            if (obj.blinkFrames == 0)
            {
                render.SetMeshMixDamageFlash(obj.meshId, false);
                if (obj.hp <= 0)
                {
                    obj.isDead = true;
                    render.RemoveMeshMix(obj.meshId);
                    DisablePhysicsObject(obj.physicsId);
                    TryDropItem(render, obj);
                }
                else
                {
                    render.SetMeshMixEnabled(obj.meshId, true);
                }
            }
        }
    }

    for (auto& cube : m_droppedRedCubes)
    {
        if (cube.meshId < 0)
        {
            continue;
        }

        if (cube.pickupWaitFrames > 0)
        {
            --cube.pickupWaitFrames;
        }

        UpdateDroppedItemPhysics(render, cube);
    }
}

const DestructibleObject* DestructibleManager::FindInAttackRange(
    const D3DXVECTOR3& playerPos,
    const float playerYaw,
    const float range,
    const float verticalRange,
    const float halfAngleRadians) const
{
    const D3DXVECTOR3 forward(-sinf(playerYaw), 0.0f, -cosf(playerYaw));
    const float attackCenterY = playerPos.y + kPlayerAttackCenterHeight;
    const DestructibleObject* bestObj = nullptr;
    float bestDistanceSq = -1.0f;

    for (const auto& obj : m_objects)
    {
        if (obj.isDead || obj.hp <= 0)
        {
            continue;
        }

        const float targetCenterY = obj.position.y + kDestructibleAttackTargetHeight;
        if (fabsf(targetCenterY - attackCenterY) > verticalRange)
        {
            continue;
        }

        D3DXVECTOR3 dir = obj.position - playerPos;
        dir.y = 0.0f;
        const float distanceSq = D3DXVec3LengthSq(&dir);
        if (distanceSq > range * range)
        {
            continue;
        }

        if (distanceSq > 0.0001f)
        {
            D3DXVec3Normalize(&dir, &dir);
        }
        else
        {
            dir = forward;
        }

        const float dot = D3DXVec3Dot(&forward, &dir);
        if (dot <= cosf(halfAngleRadians))
        {
            continue;
        }

        if (bestObj == nullptr || distanceSq < bestDistanceSq)
        {
            bestDistanceSq = distanceSq;
            bestObj = &obj;
        }
    }

    return bestObj;
}

bool DestructibleManager::TryDamage(NSRender::Render& render,
                                     const DestructibleObject& obj,
                                     const int damage)
{
    for (auto& o : m_objects)
    {
        if (&o == &obj)
        {
            if (o.isDead || o.hp <= 0)
            {
                return false;
            }

            o.hp -= damage;
            render.PlaceParticleEffect(NSRender::ParticleEffectPreset::Damage, o.position);
            o.blinkFrames = kDestructibleBlinkFrames;
            render.SetMeshMixEnabled(o.meshId, true);
            render.SetMeshMixDamageFlash(o.meshId, true);
            return true;
        }
    }

    return false;
}

const std::vector<DestructibleObject>& DestructibleManager::GetObjects() const
{
    return m_objects;
}

const std::vector<DroppedRedCube>& DestructibleManager::GetDroppedRedCubes() const
{
    return m_droppedRedCubes;
}

void DestructibleManager::SetStarDropCallback(std::function<void()> callback)
{
    m_starDropCallback = std::move(callback);
}

void DestructibleManager::SetSpeedUpCallback(std::function<void()> callback)
{
    m_speedUpCallback = std::move(callback);
}

bool DestructibleManager::TryDropRedCube(NSRender::Render& render,
                                         const D3DXVECTOR3& pos,
                                         const int dropPercent)
{
    if (dropPercent <= 0)
    {
        return false;
    }

    const int r = GetRandomPercent();
    if (r >= dropPercent)
    {
        return false;
    }

    return DropRedCube(render, pos, GetRandomCraftMaterialItemId());
}

bool DestructibleManager::TryDropAmmoHeart(NSRender::Render& render,
                                           const D3DXVECTOR3& pos,
                                           const int dropPercent)
{
    if (dropPercent <= 0)
    {
        return false;
    }

    const int r = GetRandomPercent();
    if (r >= dropPercent)
    {
        return false;
    }

    return DropAmmoHeart(render, pos);
}

void DestructibleManager::RemoveDroppedRedCube(NSRender::Render& render, const std::size_t index)
{
    if (index >= m_droppedRedCubes.size())
    {
        return;
    }

    DroppedRedCube& cube = m_droppedRedCubes.at(index);
    if (cube.meshId >= 0)
    {
        render.RemoveMeshMix(cube.meshId);
        cube.meshId = -1;
    }
}

bool DestructibleManager::DropRedCube(NSRender::Render& render, const D3DXVECTOR3& pos, const std::wstring& itemId)
{
    DroppedRedCube cube;
    cube.position = D3DXVECTOR3(pos.x, pos.y + kDroppedItemSpawnHeight, pos.z);
    cube.pickupWaitFrames = kDroppedRedCubePickupDelayFrames;
    cube.type = DestructibleDropType::RedCube;
    cube.itemId = itemId;
    cube.meshId = render.AddMeshMix(kRedCubeModelPath,
                                     cube.position,
                                     D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                     0.5f,
                                     -1.0f,
                                     false,
                                     false,
                                     false);
    if (cube.meshId < 0)
    {
        return false;
    }

    m_droppedRedCubes.push_back(cube);
    return true;
}

bool DestructibleManager::DropAmmoHeart(NSRender::Render& render, const D3DXVECTOR3& pos)
{
    DroppedRedCube heart;
    heart.position = D3DXVECTOR3(pos.x, pos.y + kDroppedItemSpawnHeight, pos.z);
    heart.pickupWaitFrames = kDroppedRedCubePickupDelayFrames;
    heart.type = DestructibleDropType::AmmoHeart;
    heart.meshId = render.AddMeshMix(kAmmoHeartModelPath,
                                     heart.position,
                                     D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                     1.0f,
                                     -1.0f,
                                     false,
                                     false,
                                     false);
    if (heart.meshId < 0)
    {
        return false;
    }

    m_droppedRedCubes.push_back(heart);
    return true;
}

void DestructibleManager::TryDropItem(NSRender::Render& render, const DestructibleObject& obj)
{
    if (obj.hasExplicitDrop)
    {
        if (obj.dropItemId.empty() || obj.dropItemId == L"None")
        {
            return;
        }

        DropRedCube(render, obj.position, obj.dropItemId);
        return;
    }

    const int r = GetRandomPercent();

    if (r < 80)
    {
        return;
    }

    if (r < 87)
    {
        if (m_starDropCallback)
        {
            m_starDropCallback();
        }
        return;
    }

    if (r < 94)
    {
        if (m_speedUpCallback)
        {
            m_speedUpCallback();
        }
        return;
    }

    DropRedCube(render, obj.position, GetRandomCraftMaterialItemId());
}
