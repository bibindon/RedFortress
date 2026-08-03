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
        int physicsCsvId = 0;
        int damage = 0;
    };

    void LoadForStage(const std::wstring& csvPath);
    int GetContactDamage(const D3DXVECTOR3& playerPosition) const;
    int GetContactDamageForCylinder(const D3DXVECTOR3& position,
                                     float radius,
                                     float height) const;
    void Clear();

private:
    std::vector<LavaZone> m_lavaZones;
};
