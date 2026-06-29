#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>
#include <functional>

namespace NSRender
{
class Render;
}

const int kDestructibleBlinkFrames = 15;
const int kDestructibleBlinkInterval = 2;
const int kDestructibleDefaultHp = 3;
const int kDroppedRedCubePickupDelayFrames = 60;

struct DestructibleObject
{
    int meshId = -1;
    int physicsId = -1;
    D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    int maxHp = kDestructibleDefaultHp;
    int hp = kDestructibleDefaultHp;
    int blinkFrames = 0;
    bool isDead = false;
};

enum class DestructibleDropType
{
    None,
    Star,
    SpeedUp,
    RedCube
};

struct DroppedRedCube
{
    int meshId = -1;
    D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    int pickupWaitFrames = 0;
};

class DestructibleManager
{
public:
    void Initialize(NSRender::Render& render);
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Clear();
    void Update(NSRender::Render& render);

    const DestructibleObject* FindInAttackRange(const D3DXVECTOR3& playerPos,
                                                 float playerYaw,
                                                 float range,
                                                 float halfAngleRadians) const;

    bool TryDamage(NSRender::Render& render, const DestructibleObject& obj, int damage);

    const std::vector<DestructibleObject>& GetObjects() const;
    const std::vector<DroppedRedCube>& GetDroppedRedCubes() const;

    void SetStarDropCallback(std::function<void()> callback);
    void SetSpeedUpCallback(std::function<void()> callback);

    bool TryDropRedCube(NSRender::Render& render, const D3DXVECTOR3& pos, int dropPercent);
    void RemoveDroppedRedCube(NSRender::Render& render, std::size_t index);

private:
    bool DropRedCube(NSRender::Render& render, const D3DXVECTOR3& pos);
    void TryDropItem(NSRender::Render& render, const D3DXVECTOR3& pos);

    NSRender::Render* m_render = nullptr;
    std::vector<DestructibleObject> m_objects;
    std::vector<DroppedRedCube> m_droppedRedCubes;

    std::function<void()> m_starDropCallback;
    std::function<void()> m_speedUpCallback;
};
