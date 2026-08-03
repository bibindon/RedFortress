#pragma once

#include <d3dx9.h>
#include <cstddef>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

struct PushableBox
{
    int id = -1;
    int meshId = -1;
    int physicsId = -1;
    D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 rotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 scale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
};

class PushableBoxManager
{
public:
    void Initialize(NSRender::Render& render);
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Clear();

    // Move one box using the horizontal player velocity.
    void Update(const D3DXVECTOR3& playerPosition,
                const D3DXVECTOR3& playerVelocity,
                float deltaSeconds);

    bool IsAnyBoxOnPlate(const D3DXVECTOR3& platePosition,
                         float plateHalfWidth,
                         float plateHalfDepth) const;
    const std::vector<PushableBox>& GetBoxes() const;
    std::size_t GetBoxCount() const;

private:
    bool IsPlayerPushingBox(const PushableBox& box,
                           const D3DXVECTOR3& playerPosition,
                           const D3DXVECTOR3& playerVelocity) const;

    NSRender::Render* m_render = nullptr;
    std::vector<PushableBox> m_boxes;
};