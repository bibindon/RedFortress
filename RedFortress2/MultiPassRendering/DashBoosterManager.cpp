#include "DashBoosterManager.h"

#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Util.h"
#include <fstream>
#include <utility>

namespace
{
bool ParseChargeEnabled(const std::wstring& value)
{
    if (value == L"0")
    {
        return false;
    }
    if (value == L"false" || value == L"False" || value == L"FALSE")
    {
        return false;
    }
    if (value == L"n" || value == L"N")
    {
        return false;
    }
    if (value == L"no" || value == L"No" || value == L"NO")
    {
        return false;
    }
    if (value == L"off" || value == L"Off" || value == L"OFF")
    {
        return false;
    }
    if (value == L"なし" || value == L"無し" || value == L"無")
    {
        return false;
    }

    return true;
}
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

        std::wstring id;
        D3DXVECTOR3 position(0.0f, 0.0f, 0.0f);
        D3DXVECTOR3 direction(0.0f, 1.0f, 0.0f);
        float speed = 16.0f;
        float duration = 0.35f;
        float radius = 1.0f;
        float scale = 0.5f;
        bool chargeEnabled = true;
        try
        {
            id = row.at(0);
            position.x = std::stof(row.at(1));
            position.y = std::stof(row.at(2));
            position.z = std::stof(row.at(3));
            direction.x = std::stof(row.at(4));
            direction.y = std::stof(row.at(5));
            direction.z = std::stof(row.at(6));
            speed = std::stof(row.at(7));
            duration = std::stof(row.at(8));
            radius = std::stof(row.at(9));
            scale = std::stof(row.at(10));
            if (row.size() >= 12)
            {
                chargeEnabled = ParseChargeEnabled(row.at(11));
            }
        }
        catch (...)
        {
            continue;
        }

        DashBooster booster;
        booster.Initialize(*m_render,
                           id,
                           position,
                           direction,
                           speed,
                           duration,
                           radius,
                           scale,
                           chargeEnabled);
        m_boosters.push_back(std::move(booster));
    }
}

void DashBoosterManager::Update(const D3DXVECTOR3& playerPosition,
                                PhysicsLib::CharacterMover& playerMover)
{
    if (m_render == nullptr)
    {
        return;
    }

    for (DashBooster& booster : m_boosters)
    {
        booster.Update(*m_render, playerPosition, playerMover);
    }
}

void DashBoosterManager::Clear()
{
    if (m_render != nullptr)
    {
        for (DashBooster& booster : m_boosters)
        {
            booster.Release(*m_render);
        }
    }

    m_boosters.clear();
}
