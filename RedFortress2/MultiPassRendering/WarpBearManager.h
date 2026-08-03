#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

class WarpBearManager
{
public:
    struct Endpoint
    {
        std::wstring warpId;
        std::wstring pairId;
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        float rotationY = 0.0f;
    };

    void LoadForStage(const std::wstring& csvPath);
    void Clear();
    void Update(const D3DXVECTOR3& playerPosition);

    bool TryGetWarpTarget(const D3DXVECTOR3& playerPosition,
                          D3DXVECTOR3* targetPosition,
                          float* targetRotationY);
    const std::vector<Endpoint>& GetEndpoints() const;

private:
    bool IsPlayerTouching(const Endpoint& endpoint,
                          const D3DXVECTOR3& playerPosition) const;
    bool IsPlayerOutsideAllEndpoints(const D3DXVECTOR3& playerPosition) const;
    const Endpoint* FindPairedEndpoint(const Endpoint& endpoint) const;

    std::vector<Endpoint> m_endpoints;
    bool m_armed = true;
};
