#pragma once

#include "DashBooster.h"
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
    NSRender::Render* m_render = nullptr;
    std::vector<DashBooster> m_boosters;
};
