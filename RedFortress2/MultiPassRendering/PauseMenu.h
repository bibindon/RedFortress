#pragma once

#include <string>

namespace NSRender
{
class Render;
}

class PauseMenu
{
public:
    void Initialize(NSRender::Render& render,
                    bool& mouseCursorVisible);
    void Toggle();
    void Open();
    void Close();
    void Update();
    void Render(const std::wstring& stageName);
    bool IsOpen() const;
    bool BlocksGameInput() const;

private:
    void SetMouseCursorVisible(bool visible);

    NSRender::Render* m_render = nullptr;
    bool* m_mouseCursorVisible = nullptr;
    bool m_isOpen = false;
    int m_stageNameFontId = -1;
    int m_menuItemFontId = -1;
    int m_qualityFontId = -1;
};
