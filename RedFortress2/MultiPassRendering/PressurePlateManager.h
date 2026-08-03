#pragma once

#include <d3dx9.h>
#include <cstddef>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class SkullManager;
class PushableBoxManager;

class PressurePlateManager
{
public:
    void Initialize(NSRender::Render& render);
    void LoadForStage(NSRender::Render& render, const std::wstring& csvPath);
    void Clear(NSRender::Render& render);
    void Update(NSRender::Render& render,
                const D3DXVECTOR3& playerPosition,
                const SkullManager& skullManager,
                const PushableBoxManager& pushableBoxManager,
                float deltaSeconds);
    std::size_t GetPairCount() const;

private:
    struct PressurePlatePair
    {
        int id = -1;
        int inactivePlateMeshId = -1;
        int activePlateMeshId = -1;
        int wallCsvId = -1;
        int wallPhysicsId = -1;
        bool active = false;
        D3DXVECTOR3 platePosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 wallClosedPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 wallPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 wallRotation = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 wallScale = D3DXVECTOR3(1.0f, 1.0f, 1.0f);
    };

    bool IsPlayerOnPlate(const PressurePlatePair& pair,
                         const D3DXVECTOR3& playerPosition) const;
    bool IsSkullOnPlate(const PressurePlatePair& pair,
                        const SkullManager& skullManager) const;
    bool IsBoxOnPlate(const PressurePlatePair& pair,
                      const PushableBoxManager& pushableBoxManager) const;
    void SetPlateActive(NSRender::Render& render,
                        PressurePlatePair& pair,
                        bool active);

    NSRender::Render* m_render = nullptr;
    std::vector<PressurePlatePair> m_pairs;
};
