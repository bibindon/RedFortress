#include "ExplanationManager.h"

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cwchar>
#include <fstream>

#include "SaveDataManager.h"
#include "../../InputDevice/InputDevice/InputDevice.h"
#include "../../RedFortressCommand/Command/HeaderOnlyCsv.hpp"
#include "../../RedFortressRender/Render/Common.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"

namespace
{
const std::wstring kMaskPath = L"res\\2D_Image\\menu_mask.png";
const int kMaskedGaussianSampleSize = 25;
const UINT kTextColor = D3DCOLOR_RGBA(255, 255, 255, 245);
const UINT kHintColor = D3DCOLOR_RGBA(220, 232, 245, 235);
const int kLineHeight = 58;

float ParseFloat(const std::wstring& text)
{
    if (text.empty())
    {
        std::abort();
    }

    errno = 0;
    wchar_t* end = nullptr;
    const float value = std::wcstof(text.c_str(), &end);
    if (errno == ERANGE || end == text.c_str() || *end != L'\0' || !std::isfinite(value))
    {
        std::abort();
    }
    return value;
}

std::vector<std::wstring> SplitLines(const std::wstring& text)
{
    std::vector<std::wstring> lines;
    std::size_t begin = 0;
    while (begin <= text.length())
    {
        const std::size_t end = text.find(L"\\n", begin);
        if (end == std::wstring::npos)
        {
            lines.push_back(text.substr(begin));
            break;
        }
        lines.push_back(text.substr(begin, end - begin));
        begin = end + 2;
    }
    return lines;
}
}

void ExplanationManager::Initialize(NSRender::Render& render,
                                    SaveDataManager& saveDataManager)
{
    m_render = &render;
    m_saveDataManager = &saveDataManager;
}

void ExplanationManager::LoadForStage(const std::wstring& stageId,
                                      const std::wstring& csvPath)
{
    Close();
    m_stageId = stageId;
    m_explanations.clear();

    if (stageId.empty() || csvPath.empty())
    {
        return;
    }

    const std::wstring fullCsvPath = NSRender::Util::GetExeDir() + csvPath;
    std::wifstream file(fullCsvPath);
    if (!file.is_open())
    {
        return;
    }
    file.close();

    const std::vector<std::vector<std::wstring>> csvData = csv::Read(fullCsvPath);
    for (const std::vector<std::wstring>& row : csvData)
    {
        if (!row.empty() && row.at(0) == L"ExplanationID")
        {
            continue;
        }
        if (row.size() < 6)
        {
            std::abort();
        }

        Explanation explanation;
        explanation.id = row.at(0);
        explanation.position.x = ParseFloat(row.at(1));
        explanation.position.y = ParseFloat(row.at(2));
        explanation.position.z = ParseFloat(row.at(3));
        explanation.radius = ParseFloat(row.at(4));
        explanation.lines = SplitLines(row.at(5));
        if (explanation.id.empty() || explanation.radius <= 0.0f ||
            explanation.lines.empty() || explanation.lines.at(0).empty())
        {
            std::abort();
        }

        for (const Explanation& existing : m_explanations)
        {
            if (existing.id == explanation.id)
            {
                std::abort();
            }
        }
        m_explanations.push_back(explanation);
    }
}

void ExplanationManager::TryActivate(const D3DXVECTOR3& playerPosition)
{
    if (m_isActive || m_render == nullptr || m_saveDataManager == nullptr)
    {
        return;
    }

    for (std::size_t index = 0; index < m_explanations.size(); ++index)
    {
        const Explanation& explanation = m_explanations.at(index);
        if (m_saveDataManager->IsExplanationShown(m_stageId, explanation.id))
        {
            continue;
        }

        D3DXVECTOR3 distance = playerPosition - explanation.position;
        const float distanceSq = D3DXVec3LengthSq(&distance);
        if (distanceSq <= explanation.radius * explanation.radius)
        {
            Open(index);
            return;
        }
    }
}

void ExplanationManager::Update()
{
    if (!m_isActive)
    {
        return;
    }
    if (m_skipInputFrame)
    {
        m_skipInputFrame = false;
        return;
    }
    if (IsDismissInputPressed())
    {
        Close();
    }
}

void ExplanationManager::Render()
{
    if (!m_isActive || m_render == nullptr || m_activeIndex >= m_explanations.size())
    {
        return;
    }

    if (m_textFontId < 0)
    {
        m_textFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 34, kTextColor);
        m_hintFontId = m_render->SetUpFontEx(L"BIZ UDGothic", 20, kHintColor);
    }

    const std::vector<std::wstring>& lines = m_explanations.at(m_activeIndex).lines;
    const int totalHeight = static_cast<int>(lines.size()) * kLineHeight;
    const int startY = (NSRender::Common::BASE_H - totalHeight) / 2;
    for (std::size_t index = 0; index < lines.size(); ++index)
    {
        m_render->DrawTextExCenter(m_textFontId,
                                   lines.at(index),
                                   120,
                                   startY + static_cast<int>(index) * kLineHeight,
                                   NSRender::Common::BASE_W - 240,
                                   kLineHeight,
                                   kTextColor);
    }

    m_render->DrawTextExCenter(m_hintFontId,
                               L"何かキー・ボタンを押すとゲームに戻ります",
                               0,
                               745,
                               NSRender::Common::BASE_W,
                               44,
                               kHintColor);
}

void ExplanationManager::Close()
{
    if (!m_isActive)
    {
        return;
    }
    if (m_render != nullptr)
    {
        m_render->SetPostEffectMaskedGaussianFilter(false);
        m_render->SetSceneUpdatePaused(false);
    }
    m_isActive = false;
    m_skipInputFrame = false;
}

bool ExplanationManager::IsActive() const
{
    return m_isActive;
}

bool ExplanationManager::IsDismissInputPressed() const
{
    for (int keyCode = 0; keyCode < 256; ++keyCode)
    {
        if (InputDevice::SKeyBoard::IsDownFirstFrame(keyCode))
        {
            return true;
        }
    }

    const InputDevice::MouseButton mouseButtons[] = {
        InputDevice::MOUSE_LEFT,
        InputDevice::MOUSE_RIGHT,
        InputDevice::MOUSE_MIDDLE,
        InputDevice::MOUSE_SIDE1,
        InputDevice::MOUSE_SIDE2
    };
    for (const InputDevice::MouseButton button : mouseButtons)
    {
        if (InputDevice::Mouse::IsDownFirstFrame(button))
        {
            return true;
        }
    }

    const InputDevice::GamePadButton gamePadButtons[] = {
        InputDevice::GAMEPAD_X,
        InputDevice::GAMEPAD_A,
        InputDevice::GAMEPAD_B,
        InputDevice::GAMEPAD_Y,
        InputDevice::GAMEPAD_L1,
        InputDevice::GAMEPAD_R1,
        InputDevice::GAMEPAD_L2,
        InputDevice::GAMEPAD_R2,
        InputDevice::GAMEPAD_BACK,
        InputDevice::GAMEPAD_START,
        InputDevice::GAMEPAD_POV_UP,
        InputDevice::GAMEPAD_POV_RIGHT,
        InputDevice::GAMEPAD_POV_DOWN,
        InputDevice::GAMEPAD_POV_LEFT
    };
    for (const InputDevice::GamePadButton button : gamePadButtons)
    {
        if (InputDevice::GamePad::IsDownFirstFrame(button))
        {
            return true;
        }
    }
    return false;
}

void ExplanationManager::Open(const std::size_t explanationIndex)
{
    if (m_render == nullptr || m_saveDataManager == nullptr ||
        explanationIndex >= m_explanations.size())
    {
        std::abort();
    }

    m_activeIndex = explanationIndex;
    m_isActive = true;
    m_skipInputFrame = true;
    const Explanation& explanation = m_explanations.at(explanationIndex);
    m_saveDataManager->MarkExplanationShown(m_stageId, explanation.id);
    m_saveDataManager->Save();
    m_render->SetSceneUpdatePaused(true);
    m_render->SetPostEffectMaskedGaussianMaskPath(kMaskPath);
    m_render->SetPostEffectMaskedGaussianSampleSize(kMaskedGaussianSampleSize);
    m_render->SetPostEffectMaskedGaussianFilter(true);
}
