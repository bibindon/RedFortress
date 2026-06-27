#include "DashBoosterManager.h"

#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"
#include <fstream>

namespace
{
const std::wstring kDashBoosterModelPath = L"res\\model\\cubeEmitYellow.x";
const int kDashBoosterCooldownFrames = 30;
}

void DashBoosterManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
}

void DashBoosterManager::LoadForStage(const std::wstring& csvPath)
{
    Clear();
    if (m_render == nullptr || csvPath.empty())
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
        if (row.size() < 11 || row.at(0) == L"DashBoosterID")
        {
            continue;
        }

        DashBoosterObject booster;
        try
        {
            booster.id = row.at(0);
            booster.position.x = std::stof(row.at(1));
            booster.position.y = std::stof(row.at(2));
            booster.position.z = std::stof(row.at(3));
            booster.direction.x = std::stof(row.at(4));
            booster.direction.y = std::stof(row.at(5));
            booster.direction.z = std::stof(row.at(6));
            booster.speed = std::stof(row.at(7));
            booster.duration = std::stof(row.at(8));
            booster.radius = std::stof(row.at(9));
            booster.scale = std::stof(row.at(10));
        }
        catch (...)
        {
            continue;
        }

        if (D3DXVec3LengthSq(&booster.direction) <= 0.0001f)
        {
            booster.direction = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
        }

        booster.renderId = m_render->AddMeshMix(NSRender::Util::GetExeDir() + kDashBoosterModelPath,
                                                booster.position,
                                                D3DXVECTOR3(0.0f, 0.0f, 0.0f),
                                                booster.scale);
        m_boosters.push_back(booster);
    }
}

void DashBoosterManager::Update(const D3DXVECTOR3& playerPosition,
                                PhysicsLib::CharacterMover& playerMover)
{
    for (DashBoosterObject& booster : m_boosters)
    {
        if (booster.cooldownFrames > 0)
        {
            --booster.cooldownFrames;
        }

        const D3DXVECTOR3 difference = playerPosition - booster.position;
        if (D3DXVec3Length(&difference) > booster.radius)
        {
            continue;
        }

        if (booster.cooldownFrames > 0)
        {
            continue;
        }

        playerMover.ApplyDashBooster(booster.direction, booster.speed, booster.duration);
        booster.cooldownFrames = kDashBoosterCooldownFrames;
    }
}

void DashBoosterManager::Clear()
{
    if (m_render != nullptr)
    {
        for (DashBoosterObject& booster : m_boosters)
        {
            if (booster.renderId >= 0)
            {
                m_render->RemoveMeshMix(booster.renderId);
                booster.renderId = -1;
            }
        }
    }

    m_boosters.clear();
}
