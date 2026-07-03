#pragma once

#include <d3dx9.h>
#include <string>
#include <vector>

namespace NSRender
{
class Render;
}

class InteractionManager
{
public:
    struct Interactable
    {
        std::wstring id;
        std::wstring type;
        D3DXVECTOR3 position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        float promptDistance = 2.0f;
    };

    void Initialize(NSRender::Render& render);
    void LoadForStage(const std::wstring& csvPath);
    void Update(const D3DXVECTOR3& playerPosition);
    void DrawPrompt();
    bool ConsumeTriggeredInteraction(std::wstring* interactionId);
    std::wstring GetNearestOfType(const D3DXVECTOR3& playerPosition, const std::wstring& type) const;
    std::wstring GetInteractableType(const std::wstring& interactionId) const;
    const std::vector<Interactable>& GetInteractables() const;
    void RemoveInteractableById(const std::wstring& interactionId);
    void Clear();

private:
    int FindNearestInteractable(const D3DXVECTOR3& playerPosition) const;
    bool IsWithinDistance(const Interactable& interactable,
                          const D3DXVECTOR3& playerPosition,
                          float extraDistance) const;

    NSRender::Render* m_render = nullptr;
    std::vector<Interactable> m_interactables;
    int m_activeIndex = -1;
    int m_promptFontId = -1;
    std::wstring m_triggeredInteractionId;
};
