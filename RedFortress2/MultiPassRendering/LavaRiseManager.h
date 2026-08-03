#pragma once

#include <d3dx9.h>
#include <cstddef>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class LavaRiseManager
{
public:
    void Initialize(NSRender::Render& render);
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Clear();
    void Update(NSRender::Render& render, float deltaSeconds);
    int GetContactDamage(const D3DXVECTOR3& playerPosition) const;
    std::size_t GetLavaCount() const;

private:
    struct Lava
    {
        std::wstring id;
        int meshId = -1;
        int physicsId = -1;
        int damage = 0;
        float minX = 0.0f;
        float maxX = 0.0f;
        float minZ = 0.0f;
        float maxZ = 0.0f;
        float startY = 0.0f;
        float endY = 0.0f;
        float delay = 0.0f;
        float duration = 0.0f;
        float elapsed = 0.0f;
    };

    void ApplyTransform(NSRender::Render& render, Lava& lava);

    NSRender::Render* m_render = nullptr;
    std::vector<Lava> m_lavas;
};
