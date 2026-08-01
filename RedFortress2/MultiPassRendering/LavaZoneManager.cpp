#include "LavaZoneManager.h"

#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Util.h"

namespace
{
using PhysicsWorld = PhysicsLib::PhysicsLib;
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
        if (row.size() < 3 || row.at(0) == L"ID")
        {
            continue;
        }

        LavaZone zone;
        try
        {
            zone.id = row.at(0);
            zone.physicsCsvId = std::stoi(row.at(1));
            zone.damage = std::stoi(row.at(2));
        }
        catch (...)
        {
            continue;
        }

        if (!zone.id.empty() && zone.physicsCsvId > 0 && zone.damage > 0)
        {
            m_lavaZones.push_back(zone);
        }
    }
}

int LavaZoneManager::GetContactDamage(const D3DXVECTOR3& playerPosition) const
{
    int damage = 0;
    const PhysicsWorld::ShapeType shapeType = PhysicsWorld::GetShapeType();
    const float radius = PhysicsWorld::GetCylinderRadius();
    const float height = PhysicsWorld::GetCylinderHeight();

    for (std::size_t i = 0; i < m_lavaZones.size(); ++i)
    {
        const LavaZone& zone = m_lavaZones.at(i);
        const int objectId = PhysicsWorld::GetCsvObjectId(zone.physicsCsvId);
        if (objectId < 0)
        {
            continue;
        }

        if (PhysicsWorld::CheckContactShape(objectId, playerPosition, shapeType, radius, height))
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
