#pragma once

#include <string>

namespace NSRender
{
class Render;
}

class EnemyBase;

// ダークソウル風のボス体力バー。
// 画面下部中央に表示し、前面バーと遅延ダメージバーの二重アニメーションを行う。
// ターゲットは生存中のボス敵（EnemyBase::IsBoss() == true）。
// 毎フレーム Update() でボスの HP を監視し、減少を検出したタイミングで
// ダメージアニメーションを駆動する。被弾処理側の修正は不要。
class BossHpBar
{
public:
    BossHpBar();

    void Initialize(NSRender::Render* pRender);
    void SetBoss(EnemyBase* pBoss);
    void Update();
    void Draw();

private:
    static float Clamp(float value, float minValue, float maxValue);
    static int CalcBarWidth(float hpValue, int imageWidth, int maxHp);

    void ResetDisplay(int hp, int maxHp);
    void OnDamage(int oldHp, int newHp);

    NSRender::Render* m_pRender = nullptr;
    EnemyBase* m_pBoss = nullptr;

    int m_fontId = -1;
    int m_lastObservedHp = -1;

    float m_frontDisplay = 100.0f;
    float m_damageDisplay = 100.0f;
    float m_frontStart = 100.0f;
    float m_frontTarget = 100.0f;
    float m_frontAnimFrame = 0.0f;
    bool m_frontAnimating = false;
    float m_damageStart = 100.0f;
    float m_damageTarget = 100.0f;
    float m_damageDelayFrame = 0.0f;
    float m_damageAnimFrame = 0.0f;
    bool m_damageWaiting = false;
    bool m_damageAnimating = false;
    bool m_damageFollowFrontAfterHeal = false;

    static const std::wstring kBackGaussImagePath;
    static const std::wstring kBackImagePath;
    static const std::wstring kFrontImagePath;
    static const std::wstring kDamageImagePath;

    // バー全体の描画スケール。DrawImageAutoResizeSizedRect には
    // (sourceWidth * scale) が描画ピクセル幅として渡る。
    // kSourceWidth(=1000) * 0.96 = 960px ≎ 画面幅(1600)の 60%。
    static constexpr float kBossBarScale = 0.96f;
    // バー下端の正規化 Y 座標（画面上辺からの比率）。
    static constexpr float kPosY = 0.88f;
    // ボス名表示のフォントサイズ（base-resolution ピクセル）。
    static constexpr int kNameFontSize = 28;
    // バー画像のソース解像度（HpBar と同じアセットを流用）。
    static constexpr int kSourceWidth = 1000;
    static constexpr int kSourceHeight = 16;
    static constexpr int kBackGaussSourceWidth = 1024;
    static constexpr int kBackGaussSourceHeight = 64;
    static constexpr float kAnimFrameMax = 30.0f;
    static constexpr float kDamageDelayFrameMax = 30.0f;
};
