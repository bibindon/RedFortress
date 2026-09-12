# ゴールポータル制作メモ

- 生成スクリプト: tools/RebuildGoalPortal.py
- 表示モデル: stone_steps.x / 編集用: stone_steps.blend
- 衝突モデル: stone_steps_collision.x（公式エクスポーターで衝突専用の簡略形状を出力）
- 衝突のみ再生成: Blenderを `--background --factory-startup --python-exit-code 1 --python tools/RebuildGoalPortal.py -- --collision-only` で実行。ゲームは既存の専用パスからこのモデルを読み込む。
- テクスチャ: stone_bricks.png
- 表示モデルは22個の閉じた部品、936三角形。外向き法線と書き出し後の面の向きを検証済み。
- 既存のゲーム側スケール2.0と設置座標を使用。実寸幅4m、四方向の段差25cm、中央天面幅2.2m。
- モデル下部の1mは埋め込み基礎。想定床高はモデルZ=0.5で、天面は床から1m（中央敷石は追加1.6cm）。ステージの地形によって埋まり方は変わる。
- 旧モデルの天面はモデルZ=1.5。新モデルはZ=1.0に下げ、登りやすい段差にした。ゲームの柱接触判定は水平距離であり変更していない。
- Blenderの+Zが上。公式DirectXエクスポーターはaxis_forward="Z", axis_up="Y"を使用。
- 衝突モデルは箱3個、36三角形。スケール2適用後の下段は幅・奥行4m／Y=0～1.25m、中段は3.4m／Y=1.25～1.75m、上段は2.2m／Y=1.75～2.016m（Yは設置原点基準）。装飾柱・面取り・4段の輪郭は再現しない。
- 静的検証: 各箱が8頂点・6面の閉曲面であること、四方向からのレイで3段の天面高さと上向き法線を生成時に検証。
- ゲーム実行画面での登坂・見た目は未確認。

## ゴール演出（緑八面体群）

- 光の柱（light_pillar.x）は廃止し、緑八面体群に完全置換した。
- 生成スクリプト: tools/RebuildGoalOctahedron.py
- 表示モデル: green_octahedron.x / 編集用: green_octahedron.blend
- 正八面体、対頂点距離0.3m。8三角形。マテリアルは緑（0.15, 0.85, 0.35）。
- green_octahedron.csv: meshtype,Emit / Intensity 0.8 / Color 150,255,170 / PointLight 0.0 / shadow,ssao=n（弱発光）。
- ゲーム側で20体をAddMeshMixし、半径0.15-0.9mに配置。周期2.2-3.8秒で上昇1.6-2.4mしながら0.25倍まで縮小＋回転。出現位相・大きさにばらつきあり。
- 起動後（接触後）は60Fで一括縮小・減光し消去。光源は緑のPoint。ポータル基準位置からY=2.5m（中央敷石の天面より約1.48m上）、明るさ1.2、範囲5m。接触後は位置を固定して60Fで減光し消去。

## テクスチャ生成

内蔵image_genツールを使用。CLI/APIフォールバックは使用していない。

最終プロンプト:

> Use case: stylized-concept. Asset type: seamless square stone brick albedo texture for a walkable ancient stone goal pedestal in a 3D game. Full bleed flat orthographic masonry surface scan. Six horizontal courses of medium rectangular GRAY stone bricks in running bond, each brick about twice as wide as tall. Fine dark gray recessed mortar joints, subtly chipped edges and believable rough stone pores, gentle variation from warm gray to cool gray per brick. Clean readable medieval fortress masonry, lightly weathered but intact; no moss, no plants, no orange clay bricks, no cracks cutting across the whole image, no object or text. Even diffuse illumination, no directional shadows, no highlights or perspective. Seamless repeat on all four edges, horizontal rows straight and consistent. Medium values so it remains readable in a dark game scene.