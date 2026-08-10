#pragma once

#include <d3dx9.h>
#include <cstddef>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class LavaFloodManager
{
public:
    void Initialize(NSRender::Render& render);
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Clear();
    void Update(NSRender::Render& render, float deltaSeconds);
    int GetContactDamage(const D3DXVECTOR3& playerPosition) const;
    int GetContactDamageForCylinder(const D3DXVECTOR3& position,
                                     float radius,
                                     float height) const;
    std::size_t GetFloodCount() const;

private:
    struct Flood
    {
        std::wstring id;
        int meshId = -1;
        int physicsId = -1;
        int damage = 0;
        D3DXVECTOR3 anchor = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 direction = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
        float startWidth = 0.0f;
        float startLength = 0.0f;
        float endWidth = 0.0f;
        float endLength = 0.0f;
        float delay = 0.0f;
        float duration = 0.0f;
        float elapsed = 0.0f;
        bool active = false;
    };

    void ApplyFloodTransform(NSRender::Render& render, Flood& flood);

    NSRender::Render* m_render = nullptr;
    std::vector<Flood> m_floods;
};
