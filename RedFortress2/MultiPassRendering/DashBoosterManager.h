#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

namespace PhysicsLib
{
class CharacterMover;
}

class DashBoosterManager
{
public:
    void Initialize(NSRender::Render& render);
    void LoadForStage(const std::wstring& csvPath);
    void Update(const D3DXVECTOR3& playerPosition, PhysicsLib::CharacterMover& playerMover);
    void Clear();

private:
    struct DashBoosterObject
    {
        std::wstring id;
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 direction = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
        float speed = 16.0f;
        float duration = 0.35f;
        float radius = 1.0f;
        float scale = 0.5f;
        bool chargeEnabled = true;
        int renderId = -1;
        int cooldownFrames = 0;
        int soundDelayFrames = 0;
    };

    NSRender::Render* m_render = nullptr;
    std::vector<DashBoosterObject> m_boosters;
};
