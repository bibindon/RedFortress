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

    // Hoshigirl モデルは足元(最下部頂点)が原点より 0.1m 下に作られており、
    // offset=0 だと足元が接地基準より下に描画されて地面に埋まる。
    // 足元を原点の高さまで押し上げるため scale(2.5) * 0.1 = 0.25m 上にずらす。
    // (Crab など正常敵は足元が原点より上に作られ offset 不要だが、
    //  Hoshigirl は逆方向なのでこの補正が必要)
    float GetMeshVerticalOffset() const override { return 0.25f; }
};
