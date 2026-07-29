#pragma once

#include "EnemyBase.h"

// ホシガール: ステージ3-8のボス。
// 黒い布ゴースト型の敵。待機(idle)モーションのみ用意されており、
// すべてのアニメーション状態が idle にマッピングされる。
// 着地して歩行で追尾し、スーパーアーマーで被弾時にノックバックしない。
class EnemyHoshigirl : public EnemyBase
{
public:
    EnemyHoshigirl(const D3DXVECTOR3& pos, int meshId, float yaw);
    static float GetScale() { return 2.5f; }
    bool IsBoss() const override { return true; }
    std::wstring GetBossName() const override { return L"ホシガール"; }
};
