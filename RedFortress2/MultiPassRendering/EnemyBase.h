#pragma once

#include <d3dx9.h>
#include <string>

namespace NSRender
{
class Render;
}

class EnemyBase
{
public:
    enum class State
    {
        Idle,
        Alert,
        Chase,
        Retreat,
        Dead
    };

    enum class AnimState
    {
        Idle,
        Walk,
        Creep,
        Run
    };

    enum class MovementMode
    {
        Ground,
        Frog,
        Hover,
        Swoop
    };

    enum class HitReactionMode
    {
        Normal,
        SuperArmor
    };

    struct AttackHit
    {
        int damage = 0;
        D3DXVECTOR3 sourcePosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
        int knockbackFrames = 0;
        int slowFrames = 0;
    };

    virtual ~EnemyBase() = default;

    void Update(NSRender::Render& render, const D3DXVECTOR3& playerPos, bool playerInvincible);
    void SyncMesh(NSRender::Render& render);
    bool ConsumeAttackHit(AttackHit* outHit);
    virtual bool UsesSpecialAttacks() const;

    void TakeDamage(NSRender::Render& render, int amount, const D3DXVECTOR3& attackerPos);
    void TakeDamageWithoutFacing(NSRender::Render& render, int amount);
    bool IsDead() const;
    bool IsReadyToRemove() const;
    void MarkAttackedPlayer(NSRender::Render& render);
    int GetHp() const;
    int GetMaxHp() const;
    D3DXVECTOR3 GetPosition() const;
    // スポーン座標系（足元基準）の座標を返す。コンストラクタで行う円柱中心座標への
    // 変換と対になる逆変換であり、CSV保存や敵種差し替えの際に使う。
    D3DXVECTOR3 GetSpawnPosition() const;
    void SetPosition(const D3DXVECTOR3& pos);
    // 足元基準のスポーン座標を受け取り、円柱中心座標へ変換して設定する。
    void SetSpawnPosition(const D3DXVECTOR3& pos);
    void StartKnockbackFrom(const D3DXVECTOR3& sourcePosition, float distance, int durationFrames);
    float GetYaw() const;
    void SetYaw(float yaw);
    int GetMeshId() const;
    void SetMeshId(int meshId);
    const std::wstring& GetType() const;
    void SetType(const std::wstring& type);
    void SetBossName(const std::wstring& bossName);

    bool IsTouchingPlayer(const D3DXVECTOR3& playerPos, float playerRadius) const;
    void SuppressContactDamageUntilPlayerSeparates();
    bool CanDamagePlayerOnContact(bool playerTouching);
    bool IsStompedByPlayer(const D3DXVECTOR3& previousPlayerPos,
                           const D3DXVECTOR3& playerPos,
                           bool playerIsJumping,
                           float playerYVelocity,
                           float playerRadius) const;
    // 物理円柱の半径（自己移動の衝突解決に使う）。接触攻撃判定とは別に設定できる。
    float GetPhysicsRadius() const { return m_physicsRadius; }
    // 物理円柱の高さ。プレイヤー攻撃の食らい判定（体全体との重なり）に使う。
    float GetHeight() const { return m_height; }

    // ボス敵判定。ボス体力バーの表示対象となる敵は true を返す。
    virtual bool IsBoss() const { return !m_bossName.empty(); }
    // ボス体力バー上部に表示する表示名。ボス以外は空文字列。
    virtual std::wstring GetBossName() const { return m_bossName; }

protected:
    EnemyBase(const D3DXVECTOR3& startPosition,
              int meshId,
              const std::wstring& type,
              float yaw,
              int maxHp,
              float moveSpeed,
              float viewDistance,
              float contactRadius,
              float height,
              float meshVerticalOffset,
              MovementMode movementMode = MovementMode::Ground,
              bool usesExtendedAnimations = false,
              HitReactionMode hitReactionMode = HitReactionMode::Normal,
              // 物理円柱の半径。負の値は contactRadius と同じ（後方互換）を意味する。
              float physicsRadius = -1.0f);

    virtual bool UpdateSpecialAttack(NSRender::Render& render,
                                     const D3DXVECTOR3& playerPos,
                                     bool playerInvincible);
    virtual float GetMeshVerticalOffset() const { return m_meshVerticalOffset; }
    // モデル原点から見た見た目の中心を、回転に追従させて補正する。
    virtual D3DXVECTOR3 GetMeshPositionOffset() const
    {
        return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    }
    virtual float GetMeshYawOffset() const { return 0.0f; }
    bool IsSpecialAttackReady() const;
    void FaceSpecialAttackTarget(const D3DXVECTOR3& targetPos);
    bool MoveForSpecialAttack(const D3DXVECTOR3& velocity);
    bool MoveSpecialProjectile(D3DXVECTOR3* position,
                               const D3DXVECTOR3& velocity,
                               float radius);
    void PlaySpecialAttackAnimation(NSRender::Render& render, const std::wstring& animationName);
    void FinishSpecialAttack();
    void EmitAttackHit(int damage,
                       const D3DXVECTOR3& sourcePosition,
                       int knockbackFrames,
                       int slowFrames);

private:
    void StartIdleBehavior();
    void UpdateIdleBehavior();
    void ApplyDamage(NSRender::Render& render, int amount);
    void StartDeath(NSRender::Render& render);
    void ApplyGravity(NSRender::Render& render);
    void BeginAlert(const D3DXVECTOR3& playerPos, bool faceImmediately);
    void UpdateChaseBehavior(const D3DXVECTOR3& playerPos, bool playerInvincible);
    void UpdateRetreatBehavior(NSRender::Render& render, const D3DXVECTOR3& playerPos);
    void UpdateFlyingIdleBehavior();
    void UpdateFlyingChaseBehavior(const D3DXVECTOR3& playerPos, bool playerInvincible);
    void UpdateFrogMovement(const D3DXVECTOR3& moveDirection, float speedMultiplier);
    void ApplyAnimation(NSRender::Render& render, AnimState nextAnim);
    void FaceTargetImmediately(const D3DXVECTOR3& targetPos);
    void StartFacePlayerTurn();
    void UpdateFacePlayerTurn(const D3DXVECTOR3& playerPos);
    void UpdateFacing(const D3DXVECTOR3& targetPos);
    bool MoveWithCollision(const D3DXVECTOR3& velocity, D3DXVECTOR3* outHitNormal = nullptr);
    bool IsPlayerInView(const D3DXVECTOR3& playerPos) const;
    float NextRandom01();
    int NextRandomInt(int minValueInclusive, int maxValueInclusive);

    D3DXVECTOR3 m_position;
    D3DXVECTOR3 m_homePosition;
    D3DXVECTOR3 m_lastKnownPlayerPosition;
    D3DXVECTOR3 m_retreatDirection = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    float m_yaw = 0.0f;
    int m_hp = 10;
    int m_maxHp = 10;
    int m_meshId = -1;
    std::wstring m_type;
    std::wstring m_bossName;
    State m_state = State::Idle;
    AnimState m_animState = AnimState::Idle;

    float m_viewDistance = 5.0f;
    float m_viewHalfAngle = D3DXToRadian(90.0f);
    float m_moveSpeed = 2.5f;
    float m_retreatDistance = 3.0f;
    float m_contactRadius = 0.5f;
    // 物理円柱の半径。enemy->player の接触攻撃には使わず、自己移動の衝突解決のみに使う。
    float m_physicsRadius = 0.5f;
    float m_height = 1.0f;
    float m_meshVerticalOffset = 0.0f;
    MovementMode m_movementMode = MovementMode::Ground;
    bool m_usesExtendedAnimations = false;
    HitReactionMode m_hitReactionMode = HitReactionMode::Normal;
    float m_verticalVelocity = 0.0f;
    bool m_isGrounded = false;
    int m_groundedCheckCooldownFrames = 0;
    int m_frogJumpCooldownFrames = 0;
    bool m_frogJumpActive = false;
    D3DXVECTOR3 m_frogJumpDirection = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
    int m_flightFrame = 0;
    int m_forcedAnimationFrames = 0;
    bool m_animationNeedsRefresh = false;
    int m_blinkFrames = 0;
    int m_hitStunFrames = 0;
    D3DXVECTOR3 m_knockbackPerFrame = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    int m_knockbackFrames = 0;
    int m_removalFrames = 0;
    float m_deathFallStartY = 0.0f;
    float m_deathFallTargetY = 0.0f;
    int m_deathFallFrames = 0;
    int m_facePlayerTurnFrames = 0;
    int m_alertFrames = 0;
    int m_idleWaitFrames = 0;
    int m_idleMoveFrames = 0;
    int m_lastKnownPlayerFrames = 0;
    int m_chaseStrafeFrames = 0;
    int m_retreatFrames = 0;
    float m_idleMoveYaw = 0.0f;
    float m_chaseStrafeDirection = 1.0f;
    float m_personalityBias = 0.0f;
    unsigned int m_behaviorSeed = 1;
    AttackHit m_pendingAttackHit;
    bool m_hasPendingAttackHit = false;
    bool m_contactDamageSuppressedUntilPlayerSeparates = false;
};
