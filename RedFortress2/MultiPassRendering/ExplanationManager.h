#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class SaveDataManager;

class ExplanationManager
{
public:
    void Initialize(NSRender::Render& render, SaveDataManager& saveDataManager);
    void LoadForStage(const std::wstring& stageId, const std::wstring& csvPath);
    void TryActivate(const D3DXVECTOR3& playerPosition);
    void Update();
    void Render();
    void Close();
    bool IsActive() const;

private:
    struct Explanation
    {
        std::wstring id;
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        float radius = 0.0f;
        std::vector<std::wstring> lines;
    };

    bool IsDismissInputPressed() const;
    void Open(std::size_t explanationIndex);

    NSRender::Render* m_render = nullptr;
    SaveDataManager* m_saveDataManager = nullptr;
    std::wstring m_stageId;
    std::vector<Explanation> m_explanations;
    std::size_t m_activeIndex = 0;
    bool m_isActive = false;
    bool m_skipInputFrame = false;
    int m_textFontId = -1;
    int m_hintFontId = -1;
};
