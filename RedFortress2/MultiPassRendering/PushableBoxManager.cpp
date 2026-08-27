#include "PushableBoxManager.h"

#include "../../PhysicsLib/PhysicsLib/PhysicsLib.h"
#include "../../RedFortressRender/Render/Render.h"
#include "../../RedFortressRender/Render/Util.h"
#include "GameAudio.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_set>

namespace
{
    const std::wstring kVisualModelPath =
        L"res\\model\\pushable_box\\pushable_box.x";
    const std::wstring kCollisionModelPath =
        L"res\\model\\pushable_box\\pushable_box_collision.x";
    const D3DXVECTOR3 kDefaultRotation(0.0f, 0.0f, 0.0f);
    const D3DXVECTOR3 kDefaultScale(1.0f, 1.0f, 1.0f);
    const D3DXVECTOR3 kDisabledPosition(0.0f, -10000.0f, 0.0f);
    const float kBoxWidth = 1.2f;
    const float kBoxHeight = 1.0f;
    const float kBoxDepth = 1.2f;
    const float kPlayerRadius = 0.3f;
    const float kPlayerHeight = 1.7f;
    const float kContactTolerance = 0.08f;
    const float kMinimumPushDirectionAmount = 0.8660254f;
    const float kPushDelaySeconds = 1.0f;

    std::wstring Trim(const std::wstring& value)
    {
        std::size_t start = 0;
        while (start < value.size() && (value[start] == L' ' || value[start] == L'\t'))
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

    float PositiveScale(float value)
    {
        const float absoluteValue = std::fabs(value);
        if (absoluteValue <= 0.0001f)
        {
            return 1.0f;
        }
        return absoluteValue;
    }
}

void PushableBoxManager::Initialize(NSRender::Render& render)
{
    m_render = &render;
    m_boxes.clear();
    ResetPushState();
}

void PushableBoxManager::LoadForStage(NSRender::Render& render, const std::wstring& csvPath)
{
    Clear();
    m_render = &render;

    if (csvPath.empty())
    {
        return;
    }

    std::wifstream file(NSRender::Util::GetExeDir() + csvPath);
    if (!file.is_open())
    {
        return;
    }

    std::unordered_set<int> loadedIds;
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

        PushableBox box;
        box.id = std::stoi(cells[0]);
        box.position.x = std::stof(cells[1]);
        box.position.y = std::stof(cells[2]);
        box.position.z = std::stof(cells[3]);
        box.rotation.y = D3DXToRadian(std::stof(cells[4]));
        const float uniformScale = std::stof(cells[5]);
        box.scale = D3DXVECTOR3(uniformScale, uniformScale, uniformScale);

        if (!loadedIds.insert(box.id).second)
        {
            std::abort();
        }

        box.meshId = m_render->AddMeshMix(kVisualModelPath,
                                          box.position,
                                          box.rotation,
                                          box.scale.x);
        if (box.meshId < 0)
        {
            std::abort();
        }

        box.physicsId = PhysicsLib::PhysicsLib::Load(
            kCollisionModelPath.c_str(),
            PhysicsLib::PhysicsLib::ObjectType::Pushable,
            0.0f);
        if (box.physicsId < 0)
        {
            std::abort();
        }
        PhysicsLib::PhysicsLib::SetTransform(box.physicsId,
                                             box.position,
                                             box.rotation,
                                             box.scale);
        m_boxes.push_back(box);
    }
}

void PushableBoxManager::Clear()
{
    GameAudio::StopPushableBoxMovement();
    ResetPushState();
    for (PushableBox& box : m_boxes)
    {
        if (m_render != nullptr && box.meshId >= 0)
        {
            m_render->RemoveMeshMix(box.meshId);
        }
        if (box.physicsId >= 0)
        {
            PhysicsLib::PhysicsLib::SetVelocity(
                box.physicsId,
                D3DXVECTOR3(0.0f, 0.0f, 0.0f));
            PhysicsLib::PhysicsLib::SetTransform(box.physicsId,
                                                 kDisabledPosition,
                                                 kDefaultRotation,
                                                 kDefaultScale);
        }
    }
    m_boxes.clear();
}

void PushableBoxManager::Update(const D3DXVECTOR3& playerPosition,
                                const D3DXVECTOR3& playerMoveDirection,
                                const float playerMoveSpeed,
                                const bool playerGrounded,
                                const float deltaSeconds)
{
    if (deltaSeconds <= 0.0f)
    {
        std::abort();
    }

    if (m_render == nullptr || m_boxes.empty() || !playerGrounded ||
        playerMoveSpeed <= 0.0001f)
    {
        GameAudio::StopPushableBoxMovement();
        ResetPushState();
        return;
    }

    const D3DXVECTOR3 horizontalDirection(playerMoveDirection.x,
                                           0.0f,
                                           playerMoveDirection.z);
    if (D3DXVec3LengthSq(&horizontalDirection) <= 0.0001f)
    {
        GameAudio::StopPushableBoxMovement();
        ResetPushState();
        return;
    }

    std::size_t selectedIndex = m_boxes.size();
    float selectedDistanceSq = 0.0f;
    PushFace selectedFace = PushFace::None;
    D3DXVECTOR3 selectedPushDirection(0.0f, 0.0f, 0.0f);
    float selectedDirectionAmount = 0.0f;
    for (std::size_t i = 0; i < m_boxes.size(); ++i)
    {
        PushFace face = PushFace::None;
        D3DXVECTOR3 pushDirection(0.0f, 0.0f, 0.0f);
        float directionAmount = 0.0f;
        if (!TryGetPlayerPush(m_boxes[i],
                              playerPosition,
                              horizontalDirection,
                              &face,
                              &pushDirection,
                              &directionAmount))
        {
            continue;
        }

        const D3DXVECTOR3 difference = m_boxes[i].position - playerPosition;
        const float distanceSq = difference.x * difference.x + difference.z * difference.z;
        if (selectedIndex >= m_boxes.size() || distanceSq < selectedDistanceSq)
        {
            selectedIndex = i;
            selectedDistanceSq = distanceSq;
            selectedFace = face;
            selectedPushDirection = pushDirection;
            selectedDirectionAmount = directionAmount;
        }
    }

    if (selectedIndex >= m_boxes.size())
    {
        GameAudio::StopPushableBoxMovement();
        ResetPushState();
        return;
    }

    PushableBox& box = m_boxes[selectedIndex];
    if (m_pushBoxId != box.id || m_pushFace != selectedFace)
    {
        ResetPushState();
        m_pushBoxId = box.id;
        m_pushFace = selectedFace;
    }

    m_pushElapsedSeconds += deltaSeconds;
    if (m_pushElapsedSeconds < kPushDelaySeconds)
    {
        GameAudio::StopPushableBoxMovement();
        return;
    }

    const float pushSpeed = playerMoveSpeed * selectedDirectionAmount;
    const D3DXVECTOR3 requestedMovement =
        selectedPushDirection * pushSpeed * deltaSeconds;
    D3DXVECTOR3 movedMovement(0.0f, 0.0f, 0.0f);
    PhysicsLib::PhysicsLib::TryMovePushable(box.physicsId,
                                            requestedMovement,
                                            &movedMovement);
    if (D3DXVec3LengthSq(&movedMovement) <= 0.0000001f)
    {
        GameAudio::StopPushableBoxMovement();
        return;
    }

    box.position += movedMovement;
    m_render->SetMeshMixPos(box.meshId, box.position);
    GameAudio::StartPushableBoxMovement();
}

bool PushableBoxManager::IsPlayerPushingAnyBox(
    const D3DXVECTOR3& playerPosition,
    const D3DXVECTOR3& playerMoveDirection,
    const bool playerGrounded) const
{
    if (!playerGrounded)
    {
        return false;
    }

    const D3DXVECTOR3 horizontalDirection(playerMoveDirection.x,
                                           0.0f,
                                           playerMoveDirection.z);
    if (D3DXVec3LengthSq(&horizontalDirection) <= 0.0001f)
    {
        return false;
    }

    for (const PushableBox& box : m_boxes)
    {
        PushFace face = PushFace::None;
        D3DXVECTOR3 pushDirection(0.0f, 0.0f, 0.0f);
        float directionAmount = 0.0f;
        if (TryGetPlayerPush(box,
                             playerPosition,
                             horizontalDirection,
                             &face,
                             &pushDirection,
                             &directionAmount))
        {
            return true;
        }
    }
    return false;
}

bool PushableBoxManager::IsAnyBoxOnPlate(const D3DXVECTOR3& platePosition,
                                         const float plateHalfWidth,
                                         const float plateHalfDepth) const
{
    for (const PushableBox& box : m_boxes)
    {
        const float halfWidth = kBoxWidth * PositiveScale(box.scale.x) * 0.5f;
        const float halfDepth = kBoxDepth * PositiveScale(box.scale.z) * 0.5f;
        const float boxBottom = box.position.y;
        const float boxTop = box.position.y + kBoxHeight * PositiveScale(box.scale.y);
        const bool overlapsX =
            std::fabs(box.position.x - platePosition.x) <= plateHalfWidth + halfWidth;
        const bool overlapsZ =
            std::fabs(box.position.z - platePosition.z) <= plateHalfDepth + halfDepth;
        const bool overlapsY = boxBottom <= platePosition.y + 0.25f &&
                               boxTop >= platePosition.y - 0.1f;
        if (overlapsX && overlapsZ && overlapsY)
        {
            return true;
        }
    }
    return false;
}

const std::vector<PushableBox>& PushableBoxManager::GetBoxes() const
{
    return m_boxes;
}

std::size_t PushableBoxManager::GetBoxCount() const
{
    return m_boxes.size();
}

bool PushableBoxManager::TryGetPlayerPush(
    const PushableBox& box,
    const D3DXVECTOR3& playerPosition,
    const D3DXVECTOR3& playerMoveDirection,
    PushFace* outFace,
    D3DXVECTOR3* outPushDirection,
    float* outDirectionAmount) const
{
    if (outFace == nullptr || outPushDirection == nullptr ||
        outDirectionAmount == nullptr)
    {
        std::abort();
    }

    *outFace = PushFace::None;
    *outPushDirection = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    *outDirectionAmount = 0.0f;

    D3DXVECTOR3 normalizedDirection(playerMoveDirection.x,
                                     0.0f,
                                     playerMoveDirection.z);
    if (D3DXVec3LengthSq(&normalizedDirection) <= 0.0001f)
    {
        return false;
    }
    D3DXVec3Normalize(&normalizedDirection, &normalizedDirection);

    const float halfWidth = kBoxWidth * PositiveScale(box.scale.x) * 0.5f;
    const float halfDepth = kBoxDepth * PositiveScale(box.scale.z) * 0.5f;
    const float boxBottom = box.position.y;
    const float boxTop = box.position.y + kBoxHeight * PositiveScale(box.scale.y);
    if (playerPosition.y >= boxTop - kContactTolerance)
    {
        return false;
    }

    const bool overlapsY = playerPosition.y <= boxTop + kContactTolerance &&
                           playerPosition.y + kPlayerHeight >= boxBottom - kContactTolerance;
    if (!overlapsY)
    {
        return false;
    }

    const float playerMinX = playerPosition.x - kPlayerRadius;
    const float playerMaxX = playerPosition.x + kPlayerRadius;
    const float playerMinZ = playerPosition.z - kPlayerRadius;
    const float playerMaxZ = playerPosition.z + kPlayerRadius;
    const float boxMinX = box.position.x - halfWidth;
    const float boxMaxX = box.position.x + halfWidth;
    const float boxMinZ = box.position.z - halfDepth;
    const float boxMaxZ = box.position.z + halfDepth;
    const bool overlapsX = playerMaxX >= boxMinX - kContactTolerance &&
                           playerMinX <= boxMaxX + kContactTolerance;
    const bool overlapsZ = playerMaxZ >= boxMinZ - kContactTolerance &&
                           playerMinZ <= boxMaxZ + kContactTolerance;
    if (!overlapsX || !overlapsZ)
    {
        return false;
    }

    PushFace bestFace = PushFace::None;
    D3DXVECTOR3 bestPushDirection(0.0f, 0.0f, 0.0f);
    float bestDirectionAmount = 0.0f;

    const bool touchesNegativeX =
        std::fabs(playerMaxX - boxMinX) <= kContactTolerance &&
        playerPosition.x < box.position.x;
    if (touchesNegativeX)
    {
        const D3DXVECTOR3 pushDirection(1.0f, 0.0f, 0.0f);
        const float directionAmount =
            D3DXVec3Dot(&normalizedDirection, &pushDirection);
        if (directionAmount > bestDirectionAmount)
        {
            bestFace = PushFace::NegativeX;
            bestPushDirection = pushDirection;
            bestDirectionAmount = directionAmount;
        }
    }

    const bool touchesPositiveX =
        std::fabs(playerMinX - boxMaxX) <= kContactTolerance &&
        playerPosition.x > box.position.x;
    if (touchesPositiveX)
    {
        const D3DXVECTOR3 pushDirection(-1.0f, 0.0f, 0.0f);
        const float directionAmount =
            D3DXVec3Dot(&normalizedDirection, &pushDirection);
        if (directionAmount > bestDirectionAmount)
        {
            bestFace = PushFace::PositiveX;
            bestPushDirection = pushDirection;
            bestDirectionAmount = directionAmount;
        }
    }

    const bool touchesNegativeZ =
        std::fabs(playerMaxZ - boxMinZ) <= kContactTolerance &&
        playerPosition.z < box.position.z;
    if (touchesNegativeZ)
    {
        const D3DXVECTOR3 pushDirection(0.0f, 0.0f, 1.0f);
        const float directionAmount =
            D3DXVec3Dot(&normalizedDirection, &pushDirection);
        if (directionAmount > bestDirectionAmount)
        {
            bestFace = PushFace::NegativeZ;
            bestPushDirection = pushDirection;
            bestDirectionAmount = directionAmount;
        }
    }

    const bool touchesPositiveZ =
        std::fabs(playerMinZ - boxMaxZ) <= kContactTolerance &&
        playerPosition.z > box.position.z;
    if (touchesPositiveZ)
    {
        const D3DXVECTOR3 pushDirection(0.0f, 0.0f, -1.0f);
        const float directionAmount =
            D3DXVec3Dot(&normalizedDirection, &pushDirection);
        if (directionAmount > bestDirectionAmount)
        {
            bestFace = PushFace::PositiveZ;
            bestPushDirection = pushDirection;
            bestDirectionAmount = directionAmount;
        }
    }

    if (bestDirectionAmount < kMinimumPushDirectionAmount)
    {
        return false;
    }

    *outFace = bestFace;
    *outPushDirection = bestPushDirection;
    *outDirectionAmount = bestDirectionAmount;
    return true;
}

void PushableBoxManager::ResetPushState()
{
    m_pushBoxId = -1;
    m_pushFace = PushFace::None;
    m_pushElapsedSeconds = 0.0f;
}
