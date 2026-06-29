#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

class LavaZoneManager
{
public:
    struct LavaZone
    {
        std::wstring id;
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        float radius = 0.0f;
        int damage = 0;
    };

    void LoadForStage(const std::wstring& csvPath);
    int GetContactDamage(const D3DXVECTOR3& playerPosition) const;
    void Clear();

private:
    std::vector<LavaZone> m_lavaZones;
};
