# STAGE_1_2「木箱ごろごろとうげ」設計書

- ワールド: World 1（草原・昼）
- 広さ: 30x60m（X: -16..16, Z: -32..32, STAGES size=(16.0, 32.0)）
- スタート: (-14, 0.2, 0) 西端 / ゴール: (14, 1.0, 0) 東端
- フォルダ: `RedFortress2/MultiPassRendering/res/model/stage_1_2/`
- 設計ソース: `STAGE_GENERATION_MEMO.md` の 1-2「木箱ごろごろとうげ」

## テーマ（メモより）

> 木箱が並ぶ起伏のある道を使い、移動、ジャンプ、攻撃を一つの流れの中で使わせる。
> 木箱は単なる飾りではなく、乗り越える足場、敵の攻撃を遮る壁、進路を選ぶ目印として配置する。
> 前方だけを見て進むと敵に囲まれる場所を作り、周囲を確認して戦う習慣を身につけさせる。
> 1-1より少し長い区画を用意し、基本操作を自分の判断で組み合わせるステージにする。

コンセプト: **西から東へ峠を越える一本道 + 中央の谷を渡る橋（移動床）**。
- 木箱を「段差」「遮蔽」「目印」として使う
- 中央のピットは移動床フェリーで渡る（前方だけ見て突っ込むと左右の敵に挟まれる）

## 区画（3区画, World 1 目安）

| 区画 | X範囲 | 主目的 | 配置 |
|---|---|---|---|
| S1 西の入口 | -16..-6 | 移動・ジャンプ・攻撃の基本流れ | スタート、木箱足場、最初の敵、QTE木 |
| S2 中央の谷 | -6..6 | 移動床でピットを渡る | ピット M、フェリー 3511、挟撃ポケット |
| S3 東のとりで | 6..16 | 攻略と報酬の組み合わせ | レバー近道、高所台座、星、ゴール |

## 地面（_build_stage_grounds.py STAGES 1-2 定義）

```
size: (16.0, 32.0)
start: (-14.0, 0.0)
goal: (14.0, 0.0)
pits: (
  (-2.5, 2.5, -16.0, 8.0),    # Pit M 中央の谷（フェリーが横断）
  (-8.0, -6.0, -18.0, -16.0), # Pit N 北の小穴（ジャンプで渡る → 木箱+アイテム）
  (4.0, 8.0, 6.0, 10.0),      # Pit S 南の穴（敵の挟撃ポケット）
)
jump_links: (((-7.0, -19.0), (-7.0, -15.0)),)   # Pit N をジャンプで横断
static_platforms: ((10.0, 1.0, 3.0),)            # 高所台座 3620（X=10, Z=1, 半幅3）
```

- ピットは外周（X±16, Z±32）に触れない / 相互に重ならないことを確認済み
- スタート・ゴールは地面の上（ピット外）
- 衝突矩形（壁 0.9x4.4, 木箱 1.25, 木 1.2）がピットに食い込まない配置

## ギミック・チェックリスト

| 要件 | 本ステージ | 配置 |
|---|---|---|
| QTE木 ≥1 | lemonTree 3801 | (-10, 0, -24), Interactable 距離 2.5 |
| ダッシュブースター ≥1 | stage12-booster-01 | (10, 0.5, -1) dir(0, 0.93, -0.37) speed16 dur0.9 → 高所台座へ |
| 移動床 ≥1 | フェリー 3511 | (0, 0.4, -17) → (0, 0.4, 10), Dur 8.0（Pit M 横断） |
| レバー ≥1 | Lever → 攻撃壁 11001 | レバー(5, 0.6, 14), 壁(10, 1.5, 12) Scale2 → 隠しアイテム I05 |
| 感圧板/ワープ/ダメージ床/押せる箱 | 感圧板 + 押せる箱 | 感圧板(0, 0.01, 16)→壁12、押せる箱(2, 0, 14) |
| Y≥3 静的床 + 登る手段 + 何か | 台座 3620 | (10, 3.4, 1) 6x6, アイテム I04 をダッシュ床で取得 |
| 敵 ≥10（W1: wolf/mushroom/crab/frog） | 12体 | 下表 |
| 区画 3・X両端に広がる | S1西/S2中央/S3東 | X -10..-6 / 0 / 10..13 |

## 配置一覧

### XFileList_simple / Physics（ID 対応）

| ID | モデル | 位置 | 備考 |
|---|---|---|---|
| 1 | stage_visual_ground_world1.x / cubeNormalInverse30x60.x | 原点 | 外装地面・外周コリジョン |
| 2 | stage_ground.x | 原点 | 実地面 |
| 3101 | collision_wall | (-10, 1.5, -12) | S1 北の壁（遮蔽） |
| 3102 | collision_wall | (8, 1.5, -24) RotY90 | 北東の壁 |
| 3103 | collision_wall | (-11, 1.5, 12) | 南西の壁 |
| 3104 | collision_wall | (11, 1.5, -20) RotY90 | 北東の壁2 |
| 12 | collision_wall | (0, 1.5, 18) RotY90, Move=y | 感圧板で開く扉 |
| 3111 | cube_wood_small | (-7, 0, -13) | 木箱足場（I02 の下） |
| 3112 | cube_wood_small | (-7, 0.95, -13) | 木箱2段目 |
| 3113 | cube_wood_small | (-4, 0, 4) | 遮蔽 |
| 3114 | cube_wood_small | (-4, 0.95, 4) | 遮蔽2段目 |
| 3115 | cube_wood_small | (1, 0, 12) | 遮蔽 |
| 3116 | cube_wood_small | (1, 0.95, 12) | 遮蔽2段目 |
| 3117 | cube_wood_small | (5, 0, 4) | フェリー南岸の遮蔽 |
| 3118 | cube_wood_small | (5, 0.95, 4) | 同上2段目 |
| 3119 | cube_wood_small | (-8, 0, 6) | S1 南側の遮蔽 |
| 3120 | cube_wood_small | (-8, 0.95, 6) | 同上2段目 |
| 3121 | cube_wood_small | (12, 0, -4) | S3 目印 |
| 3122 | cube_wood_small | (12, 0.95, -4) | 同上2段目 |
| 3123 | cube_wood_small | (8, 0, 12) | レバー近道の壁 |
| 3124 | cube_wood_small | (12, 0, 12) | レバー近道の壁 |
| 3125 | cube_wood_small | (0, 0, 22) | 感圧板の先の足場（I06 の下） |
| 3126 | cube_wood_small | (0, 0.95, 22) | 同上2段目 |
| 3131 | base_rock1 | (-11, 0, 10) Scale0.7 | 岩・装飾 |
| 3132 | base_rock2 | (11, 0, 22) Scale0.7 | 岩・装飾 |
| 3133 | base_rock1 | (-12, 0, -20) Scale0.8 | 岩・装飾 |
| 3134 | base_rock2 | (13, 0, -28) Scale0.8 | 岩・装飾 |
| 3511 | collision_moving_platform | (0, 0.4, -17) Move=y | フェリー（Pit M 横断） |
| 3620 | static_platform_2x2 | (10, 3.4, 1) | 高所台座（Y=3.4 ≥3） |
| 3801 | lemonTree / tree_cylinder_collision | (-10, 0, -24) | QTE木 |
| 11001 | attack_wall | (10, 1.5, 12) Scale2, Move=y | レバーで回転する壁 |
| 9290 | SkySphere.blend.x | (0, 0.01, 0) | 天空（render のみ） |
| 9200 | grass instancing | grass1-2.csv | 外装草 |
| 9201 | lemonTree.Instancing.x | lemonTree.Instancing.1-2.csv | 外装木 |
| 8001..8024 | fence.x | 外周 | 柵（render のみ） |

### XFileListMove.csv

```
1,3511,3511,0,0.4,-17,0,0,0,1,0,0.4,-17,0,0.4,10,8.0
```

### EnemyPositions.csv（12体, W1ロスター）

| Type | 位置 | 役割 |
|---|---|---|
| wolf | (-8, 0.2, -6) | S1 最初の敵 |
| small_mushroom | (-6, 0.2, -10) | S1 北 |
| crab | (-5, 0.2, 8) | S1 南 |
| frog | (-8, 0.2, -14) | S1 木箱足場付近 |
| wolf | (5, 0.2, -8) | S2 谷の北岸 |
| small_mushroom | (4, 0.2, -6) | S2 谷の北岸 |
| crab | (-5, 0.2, -2) | S2 谷の西岸 |
| frog | (4, 0.2, 4) | S2 谷の南岸 |
| wolf | (6, 0.2, 4) | S2 フェリー南岸（挟撃） |
| small_mushroom | (9, 0.2, 12) | S2 南東（挟撃） |
| crab | (3, 0.2, 12) | S2 南（挟撃） |
| wolf | (13, 0.2, -8) | S3 ゴール前 |

挟撃ポケット: フェリーで谷を渡った直後、前方だけ見て進むと (6,4) (9,12) (3,12) の3方向から囲まれる。

### Collectibles.csv（DataID 001-006, 009, 014 — 007/008 は不使用）

| ID | DataID | 位置 | 内容 |
|---|---|---|---|
| stage12-I01 | 001 | (-9, 0.45, -8) | S1 木箱そば |
| stage12-I02 | 002 | (-7, 1.45, -13) | 木箱2段の上（ジャンプで取得） |
| stage12-I03 | 003 | (14, 0.45, 14) | レバー近道の先 |
| stage12-I04 | 005 | (10, 3.85, 1) | 高所台座上（ダッシュ床で取得） |
| stage12-I05 | 006 | (0, 1.45, 22) | 感圧板の先の木箱の上 |
| stage12-I06 | 009 | (11, 0.45, 16) | S3 南東 |

### Destructibles.csv / Stars / SpeedUps / Skulls / PointLights

- Destructibles: (-5,0.45,-16)HP2, (5,0.45,-8)HP2, (-2,0.45,18)HP3, (10,0.45,8)HP2, (6,0.45,-22)HP2, (-8,0.45,14)HP3
- Stars: (11, 0.45, 0)
- SpeedUps: (8, 0.3, 0)
- Skulls: 1=(-5,0.2,12), 2=(7,0.2,-6)
- PointLights: (0,2.2,20), (0,2.2,-20) 暖色

### AttackTriggers.csv / Interactables / PressurePlates / PushableBoxes / LavaZones

- AttackTriggers: `1,Lever,5,0.6,14,11001,Y,0,0,0,2,2.0,8,0.8,0.7,0.3`
- Interactables: `stage12-tree-01,Tree,-10,0,-24,2.5`
- PressurePlates: `1,0,0.01,16,12,0,90,0,1`（壁12を開く）
- PushableBoxes: `1,2,0,14,0,1`（押せる木箱）
- LavaZones: なし（感圧板+押せる箱で代替）

## 検証手順

1. `python tools/validate_stage.py 1-2`（BOM/CRLF・ID整合・start/goal 3ソース同期）
2. Blender: `RED_FORTRESS_STAGE_GROUND='1-2'` で地面生成 + validate_stage
3. MSBuild Debug|x64 → `x64\Debug\res\model\stage_1_2\` コピー確認
