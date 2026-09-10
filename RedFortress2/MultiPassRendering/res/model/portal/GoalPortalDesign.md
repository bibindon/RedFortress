# ゴールポータル制作メモ

- 生成スクリプト: tools/RebuildGoalPortal.py
- 表示モデル: stone_steps.x / 編集用: stone_steps.blend
- 衝突モデル: stone_steps_collision.x（公式エクスポーターで同じ形状を再出力）
- テクスチャ: stone_bricks.png
- 22個の閉じた部品、936三角形。外向き法線と書き出し後の面の向きを検証済み。
- 既存のゲーム側スケール2.0と設置座標を使用。実寸幅4m、四方向の段差25cm、中央天面幅2.2m。
- モデル下部の1mは埋め込み基礎。想定床高はモデルZ=0.5で、天面は床から1m（中央敷石は追加1.6cm）。ステージの地形によって埋まり方は変わる。
- 旧モデルの天面はモデルZ=1.5。新モデルはZ=1.0に下げ、登りやすい段差にした。ゲームの柱接触判定は水平距離であり変更していない。
- Blenderの+Zが上。公式DirectXエクスポーターはaxis_forward="Z", axis_up="Y"を使用。
- 静的検証: 四方向の各踏み面へレイを落とし、高さと上向き法線を確認。表示/衝突のXファイルが一致することを確認。
- ゲーム実行画面での登坂・見た目は未確認。

## テクスチャ生成

内蔵image_genツールを使用。CLI/APIフォールバックは使用していない。

最終プロンプト:

> Use case: stylized-concept. Asset type: seamless square stone brick albedo texture for a walkable ancient stone goal pedestal in a 3D game. Full bleed flat orthographic masonry surface scan. Six horizontal courses of medium rectangular GRAY stone bricks in running bond, each brick about twice as wide as tall. Fine dark gray recessed mortar joints, subtly chipped edges and believable rough stone pores, gentle variation from warm gray to cool gray per brick. Clean readable medieval fortress masonry, lightly weathered but intact; no moss, no plants, no orange clay bricks, no cracks cutting across the whole image, no object or text. Even diffuse illumination, no directional shadows, no highlights or perspective. Seamless repeat on all four edges, horizontal rows straight and consistent. Medium values so it remains readable in a dark game scene.