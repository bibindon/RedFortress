#include "LavaZoneManager.h"

#include <cmath>

#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Util.h"

namespace
{
const float kLavaTriggerHeight = 2.0f;
}

void LavaZoneManager::LoadForStage(const std::wstring& csvPath)
{
    Clear();
    if (csvPath.empty())
    {
        return;
    }

    std::vector<std::vector<std::wstring>> csvData;
    const std::wstring fullCsvPath = NSRender::Util::GetExeDir() + csvPath;
    std::wifstream file(fullCsvPath);
    if (!file.is_open())
    {
        return;
    }
    file.close();

    try
    {
        csvData = csv::Read(fullCsvPath);
    }
    catch (...)
    {
        return;
    }

    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.size() < 6 || row.at(0) == L"ID")
        {
            continue;
        }

        LavaZone zone;
        try
        {
            zone.id = row.at(0);
            zone.position.x = std::stof(row.at(1));
            zone.position.y = std::stof(row.at(2));
            zone.position.z = std::stof(row.at(3));
            zone.radius = std::stof(row.at(4));
            zone.damage = std::stoi(row.at(5));
        }
        catch (...)
        {
            continue;
        }

        if (!zone.id.empty() && zone.radius > 0.0f && zone.damage > 0)
        {
            m_lavaZones.push_back(zone);
        }
    }
}

int LavaZoneManager::GetContactDamage(const D3DXVECTOR3& playerPosition) const
{
    int damage = 0;
    const float playerY = playerPosition.y;
    for (std::size_t i = 0; i < m_lavaZones.size(); ++i)
    {
        const LavaZone& zone = m_lavaZones.at(i);
        if (playerY > zone.position.y + kLavaTriggerHeight)
        {
            continue;
        }

        const float dx = playerPosition.x - zone.position.x;
        const float dz = playerPosition.z - zone.position.z;
        const float horizontalDistance = std::sqrt(dx * dx + dz * dz);
        if (horizontalDistance <= zone.radius)
        {
            if (zone.damage > damage)
            {
                damage = zone.damage;
            }
        }
    }
    return damage;
}

void LavaZoneManager::Clear()
{
    m_lavaZones.clear();
}
