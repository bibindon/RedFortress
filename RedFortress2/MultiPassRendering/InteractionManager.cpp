#include "InteractionManager.h"

#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Common.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"

namespace
{
const float kPromptExitMargin = 0.3f;
const UINT kPromptColor = D3DCOLOR_RGBA(255, 255, 255, 245);
const std::wstring kPromptText = L"Fキー or ○ボタン";
}

void InteractionManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
}

void InteractionManager::LoadForStage(const std::wstring& csvPath)
{
    Clear();
    if (csvPath.empty())
    {
        return;
    }

    std::vector<std::vector<std::wstring>> csvData;
    try
    {
        csvData = csv::Read(NSRender::Util::GetExeDir() + csvPath);
    }
    catch (...)
    {
        return;
    }

    for (std::size_t i = 0; i < csvData.size(); ++i)
    {
        const std::vector<std::wstring>& row = csvData.at(i);
        if (row.size() < 6 || row.at(0) == L"InteractionID")
        {
            continue;
        }

        Interactable interactable;
        try
        {
            interactable.id = row.at(0);
            interactable.type = row.at(1);
            interactable.position.x = std::stof(row.at(2));
            interactable.position.y = std::stof(row.at(3));
            interactable.position.z = std::stof(row.at(4));
            interactable.promptDistance = std::stof(row.at(5));
        }
        catch (...)
        {
            continue;
        }

        if (!interactable.id.empty() && interactable.promptDistance > 0.0f)
        {
            m_interactables.push_back(interactable);
        }
    }
}

void InteractionManager::Update(const D3DXVECTOR3& playerPosition)
{
    bool keepActiveInteractable = false;
    if (m_activeIndex >= 0 && m_activeIndex < static_cast<int>(m_interactables.size()))
    {
        keepActiveInteractable = IsWithinDistance(m_interactables.at(m_activeIndex),
                                                   playerPosition,
                                                   kPromptExitMargin);
    }

    if (!keepActiveInteractable)
    {
        m_activeIndex = FindNearestInteractable(playerPosition);
    }

    if (m_activeIndex < 0)
    {
        return;
    }

    const bool keyboardTriggered = InputDevice::SKeyBoard::IsDownFirstFrame(DIK_F);
    const bool gamePadTriggered = InputDevice::GamePad::IsDownFirstFrame(InputDevice::GAMEPAD_B);
    if (keyboardTriggered || gamePadTriggered)
    {
        m_triggeredInteractionId = m_interactables.at(m_activeIndex).id;
    }
}

void InteractionManager::DrawPrompt()
{
    if (m_render == nullptr || m_activeIndex < 0)
    {
        return;
    }

    if (m_promptFontId < 0)
    {
        m_promptFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 22, kPromptColor);
    }

    m_render->DrawTextExCenter(m_promptFontId,
                               kPromptText,
                               0,
                               790,
                               NSRender::Common::BASE_W,
                               48,
                               kPromptColor);
}

bool InteractionManager::ConsumeTriggeredInteraction(std::wstring* interactionId)
{
    if (m_triggeredInteractionId.empty())
    {
        return false;
    }

    if (interactionId != nullptr)
    {
        *interactionId = m_triggeredInteractionId;
    }
    m_triggeredInteractionId.clear();
    return true;
}

std::wstring InteractionManager::GetNearestOfType(const D3DXVECTOR3& playerPosition, const std::wstring& type) const
{
    int nearestIndex = -1;
    float nearestDistance = 0.0f;
    for (std::size_t i = 0; i < m_interactables.size(); ++i)
    {
        const Interactable& interactable = m_interactables.at(i);
        if (interactable.type != type)
        {
            continue;
        }

        const D3DXVECTOR3 difference = playerPosition - interactable.position;
        const float distance = D3DXVec3Length(&difference);
        if (distance > interactable.promptDistance)
        {
            continue;
        }

        if (nearestIndex < 0 || distance < nearestDistance)
        {
            nearestIndex = static_cast<int>(i);
            nearestDistance = distance;
        }
    }

    if (nearestIndex < 0)
    {
        return L"";
    }

    return m_interactables.at(nearestIndex).id;
}

void InteractionManager::Clear()
{
    m_interactables.clear();
    m_activeIndex = -1;
    m_triggeredInteractionId.clear();
}

int InteractionManager::FindNearestInteractable(const D3DXVECTOR3& playerPosition) const
{
    int nearestIndex = -1;
    float nearestDistance = 0.0f;
    for (std::size_t i = 0; i < m_interactables.size(); ++i)
    {
        const Interactable& interactable = m_interactables.at(i);
        const D3DXVECTOR3 difference = playerPosition - interactable.position;
        const float distance = D3DXVec3Length(&difference);
        if (distance > interactable.promptDistance)
        {
            continue;
        }

        if (nearestIndex < 0 || distance < nearestDistance)
        {
            nearestIndex = static_cast<int>(i);
            nearestDistance = distance;
        }
    }

    return nearestIndex;
}

bool InteractionManager::IsWithinDistance(const Interactable& interactable,
                                          const D3DXVECTOR3& playerPosition,
                                          const float extraDistance) const
{
    const D3DXVECTOR3 difference = playerPosition - interactable.position;
    return D3DXVec3Length(&difference) <= interactable.promptDistance + extraDistance;
}
