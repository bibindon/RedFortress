#include "WarpBearManager.h"

#include "../../RedFortressRender/Render/Util.h"

#include <cmath>
#include <cstdlib>
#include <cwchar>
#include <fstream>
#include <sstream>

namespace
{
const float kTriggerRadius = 0.4f;
const float kTriggerHeight = 1.7f;
const float kPlayerRadius = 0.3f;
const float kTriggerVerticalTolerance = 0.25f;

std::wstring Trim(const std::wstring& value)
{
    std::size_t start = 0;
    while (start < value.size() &&
           (value[start] == L' ' || value[start] == L'\t' || value[start] == L'\r'))
    {
        ++start;
    }

    std::size_t end = value.size();
    while (end > start &&
           (value[end - 1] == L' ' || value[end - 1] == L'\t' || value[end - 1] == L'\r'))
    {
        --end;
    }
    return value.substr(start, end - start);
}

std::vector<std::wstring> SplitCsvLine(const std::wstring& line)
{
    std::vector<std::wstring> cells;
    std::wstringstream stream(line);
    std::wstring cell;
    while (std::getline(stream, cell, L','))
    {
        cells.push_back(Trim(cell));
    }
    return cells;
}

float ParseFloat(const std::wstring& value)
{
    const std::wstring trimmed = Trim(value);
    if (trimmed.empty())
    {
        std::abort();
    }

    wchar_t* end = nullptr;
    const float parsed = std::wcstof(trimmed.c_str(), &end);
    if (end == trimmed.c_str() || *end != L'\0')
    {
        std::abort();
    }
    return parsed;
}
}

void WarpBearManager::LoadForStage(const std::wstring& csvPath)
{
    Clear();
    if (csvPath.empty())
    {
        return;
    }

    const std::wstring fullPath = NSRender::Util::GetExeDir() + csvPath;
    std::wifstream file(fullPath);
    if (!file.is_open())
    {
        return;
    }

    std::wstring line;
    bool isFirstLine = true;
    while (std::getline(file, line))
    {
        if (isFirstLine)
        {
            isFirstLine = false;
            continue;
        }
        if (Trim(line).empty())
        {
            continue;
        }

        const std::vector<std::wstring> cells = SplitCsvLine(line);
        if (cells.size() < 6)
        {
            std::abort();
        }

        Endpoint endpoint;
        endpoint.warpId = cells.at(0);
        endpoint.pairId = cells.at(1);
        endpoint.position.x = ParseFloat(cells.at(2));
        endpoint.position.y = ParseFloat(cells.at(3));
        endpoint.position.z = ParseFloat(cells.at(4));
        endpoint.rotationY = D3DXToRadian(ParseFloat(cells.at(5)));
        if (endpoint.warpId.empty() || endpoint.pairId.empty())
        {
            std::abort();
        }

        for (const Endpoint& loaded : m_endpoints)
        {
            if (loaded.warpId == endpoint.warpId)
            {
                std::abort();
            }
        }
        m_endpoints.push_back(endpoint);
    }

    for (const Endpoint& endpoint : m_endpoints)
    {
        int pairCount = 0;
        for (const Endpoint& candidate : m_endpoints)
        {
            if (candidate.pairId == endpoint.pairId)
            {
                ++pairCount;
            }
        }
        if (pairCount != 2)
        {
            std::abort();
        }
    }
}

void WarpBearManager::Clear()
{
    m_endpoints.clear();
    m_armed = true;
}

void WarpBearManager::Update(const D3DXVECTOR3& playerPosition)
{
    if (m_armed)
    {
        return;
    }

    if (IsPlayerOutsideAllEndpoints(playerPosition))
    {
        m_armed = true;
    }
}

bool WarpBearManager::TryGetWarpTarget(const D3DXVECTOR3& playerPosition,
                                       D3DXVECTOR3* targetPosition,
                                       float* targetRotationY)
{
    if (!m_armed || targetPosition == nullptr || targetRotationY == nullptr)
    {
        return false;
    }

    for (const Endpoint& endpoint : m_endpoints)
    {
        if (!IsPlayerTouching(endpoint, playerPosition))
        {
            continue;
        }

        const Endpoint* paired = FindPairedEndpoint(endpoint);
        if (paired == nullptr)
        {
            std::abort();
        }

        const D3DXVECTOR3 exitDirection(-sinf(paired->rotationY),
                                         0.0f,
                                         -cosf(paired->rotationY));
        *targetPosition = paired->position + exitDirection * 1.5f;
        *targetRotationY = paired->rotationY;
        m_armed = false;
        return true;
    }

    return false;
}

const std::vector<WarpBearManager::Endpoint>& WarpBearManager::GetEndpoints() const
{
    return m_endpoints;
}

bool WarpBearManager::IsPlayerTouching(const Endpoint& endpoint,
                                       const D3DXVECTOR3& playerPosition) const
{
    const float deltaX = playerPosition.x - endpoint.position.x;
    const float deltaZ = playerPosition.z - endpoint.position.z;
    const float horizontalDistanceSq = deltaX * deltaX + deltaZ * deltaZ;
    if (horizontalDistanceSq > (kTriggerRadius + kPlayerRadius) *
                               (kTriggerRadius + kPlayerRadius))
    {
        return false;
    }

    const float playerBottom = playerPosition.y;
    const float playerTop = playerPosition.y + kTriggerHeight;
    const float endpointBottom = endpoint.position.y - kTriggerVerticalTolerance;
    const float endpointTop = endpoint.position.y + kTriggerHeight + kTriggerVerticalTolerance;
    return playerTop >= endpointBottom && playerBottom <= endpointTop;
}

bool WarpBearManager::IsPlayerOutsideAllEndpoints(const D3DXVECTOR3& playerPosition) const
{
    for (const Endpoint& endpoint : m_endpoints)
    {
        if (IsPlayerTouching(endpoint, playerPosition))
        {
            return false;
        }
    }
    return true;
}

const WarpBearManager::Endpoint* WarpBearManager::FindPairedEndpoint(
    const Endpoint& endpoint) const
{
    for (const Endpoint& candidate : m_endpoints)
    {
        if (candidate.pairId == endpoint.pairId && candidate.warpId != endpoint.warpId)
        {
            return &candidate;
        }
    }
    return nullptr;
}
