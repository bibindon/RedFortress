# ステージ生成仕様メモ

## はじめに

* 以下は、自作の3Dゲームのステージ作成についてのメモである。
* ステージは全32個あり、ワールドごとに30x60m、120x120m、120x240mのいずれかの広さを持つ。その中に、木箱や動く床、ダメージ床、ダッシュ床、モンスター、レバーなどを適切に配置したい。ステージは32個あるため配置をAIに任せたい。このテキストは、AIが正しくステージを作成させるための説明テキストである。

## ゲームの基本ルール

* プレイヤーはダッシュ、二段ジャンプ、空中ダッシュができるが、これらはパワーアップアイテムなので、これらがなくてもクリアできる必要がある。
* プレイヤーは常に以下の3つのアクションができる。すべてのステージは、この3つのアクションだけでクリアできる必要がある。
  * こん棒で殴る
  * 走る
  * ジャンプする（高さ1.28m、距離4.1m）
* 頭蓋骨が配置されているステージでは、以下のアクションも使用できる。ただし、頭蓋骨を使わなくてもクリアできる経路を用意する。
  * ドクロを掴んで投げる
* 落とし穴に落ちると落下死するが高所から地面に着地してもダメージはない。

## ステージの基本ルール

* プレイヤーは開始地点からスタートし、指定されたゴールを目指す。
* ステージ内の敵をすべて倒すとゴールポータルが使用可能になる。ゴールポータルに触れるとステージクリアとなる。
* 落下穴に落ちると死亡する。
* 正規ルートは、ダッシュ、二段ジャンプ、空中ダッシュがなくてもクリアできるようにする。
* 座標はXを左右、Yを上方向、Zを奥行きとして扱い、1ユニットを1mとする。
* ボスステージではQTE、移動床、頭蓋骨、押せる箱、感圧板、ダッシュブースター、ワープオブジェクト、破壊可能オブジェクト、雑魚敵を配置しない。
* ステージの広さは以下のようになっている
  * ただし、ボスステージのサイズはすべて60x60m
  * ワールド1
    * 30x60m
      * 草原・昼
  * ワールド2
    * 120x120m
      * 洞窟
  * ワールド3
    * 120x240m
      * 草原・夕方
  * ワールド4
    * 120x240m
      * 草原・夜


## 正しい情報の参照元

* ステージ名、ステージフォルダー、開始地点、ゴール地点は`RedFortress2/MultiPassRendering/StageManager.cpp`を正とする。
* 既存の配置は、各ステージフォルダー内にある現在のCSVを正とする。
* `STAGE_PLAN.md`、`STAGE_THEME.md`、`tools/BuildPlannedStages.py`などの内容が現在の実装と異なる場合は、古い計画を無条件に適用しない。
* 既存ステージを修正するときは対象ステージだけを書き換え、他のステージのCSVや地面モデルを再生成しない。

### ステージ番号とフォルダーの対応

| ワールド | ステージとフォルダー |
|---|---|
| World 1 | 1-1=`stage1`、1-2=`stage2`、1-3=`stage3`、1-4=`stage4`、1-5=`stage17`、1-6=`stage18`、1-7=`stage19`、1-8=`stage20` |
| World 2 | 2-1=`stage5`、2-2=`stage6`、2-3=`stage7`、2-4=`stage8`、2-5=`stage21`、2-6=`stage22`、2-7=`stage23`、2-8=`stage24` |
| World 3 | 3-1=`stage9`、3-2=`stage10`、3-3=`stage11`、3-4=`stage12`、3-5=`stage25`、3-6=`stage26`、3-7=`stage27`、3-8=`stage28` |
| World 4 | 4-1=`stage13`、4-2=`stage14`、4-3=`stage15`、4-4=`stage16`、4-5=`stage29`、4-6=`stage30`、4-7=`stage31`、4-8=`stage32` |


## 設置可能なギミック

* 固定地形
  * 地面
    * 描画用 : stage_ground.x
    * 衝突判定用 : stage_ground.x
  * 木箱
    * 描画用 : res/model/cubeWoodSmall/cube_wood_small.x
    * 衝突判定用 : res/model/cubeWoodSmall/cube_wood_small_collision.x
  * 移動しない床(3x3m)
    * 描画用 : res/model/static_platform/static_platform_1x1.x
    * 衝突判定用 : res/model/static_platform/static_platform_1x1.x
  * 移動しない床(3x6m)
    * 描画用 : res/model/static_platform/static_platform_1x2.x
    * 衝突判定用 : res/model/static_platform/static_platform_1x2.x
  * 移動しない床(6x3m)
    * 描画用 : res/model/static_platform/static_platform_2x1.x
    * 衝突判定用 : res/model/static_platform/static_platform_2x1.x
  * 移動しない床(6x6m)
    * 描画用 : res/model/static_platform/static_platform_2x2.x
    * 衝突判定用 : res/model/static_platform/static_platform_2x2.x
  * 移動床
    * 水平移動、昇降、往復、斜め移動する床
      * 描画用 : res/model/collision_moving_platform/collision_moving_platform.x
      * 衝突判定用 : res/model/collision_moving_platform/collision_moving_platform.x
* 落とし穴
  * 落とし穴は設置できない。地面として、各ステージ専用のstage_ground.xが用意されている。  
    これを編集し窪みを追加することで落とし穴を作成する。
* スイッチ系オブジェクト
  * レバー
    * 攻撃するたびにON/OFFが切り替えられる
    * 連動する壁がY軸で90度回転する。
      * 描画用 : res/model/attack_trigger/lever.x
      * 衝突判定用 : res/model/attack_trigger/lever.x
  * ボタン
    * 攻撃するとONになるが10秒後にOFFになる
      * OFF
        * 描画用 : res/model/pressure_plate/pressure_plate_black.x
        * 衝突判定用 : res/model/pressure_plate/pressure_plate_black.x
      * ON
        * 描画用 : res/model/pressure_plate/pressure_plate_green.x
        * 衝突判定用 : res/model/pressure_plate/pressure_plate_green.x
  * ロープ
    * 攻撃すると切断される。一度きり。
    * 連動する床がX軸で90度回転する
      * 描画用 : res/model/attack_trigger/rope.x
      * 衝突判定用 : res/model/attack_trigger/rope.x
* 敵
  * 共通仕様
    * 配置情報は`EnemyPositions.csv`に記述する
    * `Type`で敵の種類を指定する
    * 描画にはアニメーション付きXファイルを使用する
    * 専用の衝突判定用Xファイルは使用せず、プログラム側の円柱判定を使用する
  * オオカミ
    * Type : `wolf`
    * 描画用 : `res/model2/separatedAnim/wolfAnim.x`
    * アニメーション設定 : `res/model2/separatedAnim/wolfAnim.csv`
  * 大型オオカミ
    * Type : `enemy2`
    * 描画用 : `res/model2/Enemy2/wolfAnim.x`
    * アニメーション設定 : `res/model2/Enemy2/wolfAnim.csv`
  * ゴースト
    * Type : `ghost`
    * 描画用 : `res/model2/Ghost/enemy.x`
    * アニメーション設定 : `res/model2/Ghost/enemy.csv`
  * 鳥
    * Type : `bird`
    * 描画用 : `res/model2/Bird/enemy.x`
    * アニメーション設定 : `res/model2/Bird/enemy.csv`
  * カニ
    * Type : `crab`
    * 描画用 : `res/model2/Crab/enemy.x`
    * アニメーション設定 : `res/model2/Crab/enemy.csv`
  * ジャイアントクラブ
    * Type : `giant_crab`
    * 描画用 : `res/model2/Crab/enemy.x`
    * アニメーション設定 : `res/model2/Crab/enemy.csv`
  * カエル
    * Type : `frog`
    * 描画用 : `res/model2/Frog/enemy.x`
    * アニメーション設定 : `res/model2/Frog/enemy.csv`
  * クモ
    * Type : `spider`
    * 描画用 : `res/model2/Spider/enemy.x`
    * アニメーション設定 : `res/model2/Spider/enemy.csv`
  * 小型クモ
    * Type : `small_spider`
    * 描画用 : `res/model2/Spider/enemy.x`
    * アニメーション設定 : `res/model2/Spider/enemy.csv`
  * スケルトン
    * Type : `skeleton`
    * 描画用 : `res/model2/Skeleton/enemy.x`
    * アニメーション設定 : `res/model2/Skeleton/enemy.csv`
  * 小型スケルトン
    * Type : `small_skeleton`
    * 描画用 : `res/model2/Skeleton/enemy.x`
    * アニメーション設定 : `res/model2/Skeleton/enemy.csv`
  * キノコ
    * Type : `mushroom`
    * 描画用 : `res/model2/Mushroom/enemy.x`
    * アニメーション設定 : `res/model2/Mushroom/enemy.csv`
  * 小型キノコ
    * Type : `small_mushroom`
    * 描画用 : `res/model2/Mushroom/enemy.x`
    * アニメーション設定 : `res/model2/Mushroom/enemy.csv`
  * ゴーレム
    * Type : `golem`
    * 描画用 : `res/model2/Golem/enemy.x`
    * アニメーション設定 : `res/model2/Golem/enemy.csv`
  * 小型ゴーレム
    * Type : `small_golem`
    * 描画用 : `res/model2/Golem/enemy.x`
    * アニメーション設定 : `res/model2/Golem/enemy.csv`
  * ボス敵
    * ウェアウルフ
      * Type : `boss_enemy2`
      * 描画用 : `res/model2/Enemy2/wolfAnim.x`
    * ジャイアントクラブ
      * Type : `boss_giant_crab`
      * 描画用 : `res/model2/Crab/enemy.x`
    * ゴーレム
      * Type : `boss_golem`
      * 描画用 : `res/model2/Golem/enemy.x`
    * 天音かなた
      * Type : `boss_kanata`
      * 描画用 : `res/model2/KanataPrototype/enemy.x`
    * ホシガール
      * Type : `boss_hoshigirl`
      * 描画用 : `res/model2/Hoshigirl/enemy.x`
* 破壊可能木箱
  * 描画用 : `res/model/cubeWoodBreakable/cube_wood_breakable.x`
  * 衝突判定用 : `res/model/cubeWoodBreakable/cube_wood_breakable_collision.x`
  * 配置情報 : `Destructibles.csv`
    * `PosX`、`PosY`、`PosZ` : 配置座標
    * `HP` : 耐久力
    * `DropItemId` : 破壊時に必ず落とすアイテム。`None`を指定すると何も落とさない
* 感圧板と連動扉
  * プレイヤーが踏むか、「押せる箱」を乗せるか、ドクロを乗せると反応する。
  * 連動扉が上に移動する。
* 押せる箱
  * 押して移動する。
* ワープオブジェクト
  * 触れると、同じ`PairID`を持つもう一方のワープオブジェクトへ移動する。
  * `WarpID`はステージ内で重複させず、1つの`PairID`につき必ず2個を配置する。
* ダッシュブースター
  * 判定半径内に入ったプレイヤーを、指定した方向、速度、効果時間で射出する。
  * `ChargeEnabled`を省略した場合は有効になる。既存CSVにこの列がある場合は列構成を維持する。
* ダメージ床
  * `LavaZones.csv`で対象の物理オブジェクトを指定し、触れたプレイヤーへダメージを与える。
* 迫る溶岩
  * `LavaFlood.csv`で生成する。アンカー位置からZ方向へ進みながら、開始時の幅と長さから終了時の幅と長さまで変化する。
* せり上がる溶岩
  * `LavaRise.csv`で生成する。指定したXZ範囲を覆い、待機時間後に`StartY`から`EndY`まで上昇する。
* 頭蓋骨
  * 掴んで投げることができる。
* アイテム
  * クラフト素材とクラフト素材以外の収集物は`Collectibles.csv`へ配置する。
  * スターは`Stars.csv`へ配置する。取得すると一定時間無敵になり、移動速度が最大になる。1ステージに複数配置できる。
  * スピードアップは`SpeedUps.csv`へ配置する。取得すると基礎移動速度の段階が1つ上がる。現在の実装が読み込む配置は、ヘッダー直後の1行だけである。
* QTEオブジェクト
  * 現在実装されている通常ステージ用QTEオブジェクトは、`Type`が`Tree`の木である。
  * プレイヤーが木の`PromptDistance`以内に入ると「Fキー or ○ボタン」と表示され、Fキーまたはゲームパッドの○ボタンでQTEを開始する。
  * QTE中は、拡大する円が目標の円に重なったタイミングでSpaceキーまたはゲームパッドの×ボタンを押す。
  * 判定が`Success`または`Normal`なら、ランダムなクラフト素材を1個獲得する。`Failure`ではアイテムを獲得できない。
  * `Tree`はQTE開始時に実行中のインタラクト対象から取り除かれるため、成否にかかわらず1回だけ使用できる。CSV自体は変更されず、見た目と衝突判定の木もその場に残る。
  * 配置には次の3つのCSVを使用する。3ファイルの座標を一致させ、描画用と衝突判定用では回転と倍率も一致させる。
    * `XFileList_simple.csv`：描画用の木。`res/model/tree2/lemonTree.x`を使用する。
    * `XFileListPhysics.csv`：衝突判定用の木。`res/model/tree2Physics/tree_cylinder_collision.x`を使用する。
    * `Interactables.csv`：QTEの起動位置と反応距離を指定する。
  * `Interactables.csv`の列は`InteractionID,Type,PosX,PosY,PosZ,PromptDistance`とする。
  * 例：`stage1-tree-01,Tree,10,0,22,2.5`
  * `InteractionID`はステージ内で重複しない名前にする。`PromptDistance`は木の中心へ無理なく近づける範囲とし、既存例では2.5mを使用している。
  * QTE中はプレイヤー、カメラ、敵、インタラクトの更新が止まる。敵やダメージ床の近くに配置して開始前後の緊張感を作ってよいが、プロンプトを確認して意図的に起動できる位置にする。
  * QTEの成功をクリア必須条件にしない。失敗しても正規ルートの進行や必須敵の撃破が可能な構成にする。
  * ボスステージには配置しない。

## 配置データの出力先

各ステージの配置情報は、対応する`res/model/stageN/`フォルダー内のCSVへ記述する。既存CSVの列順を変更しない。

| CSV | 用途 | 主な列・注意点 |
|---|---|---|
| `XFileList_simple.csv` | 描画モデル | `ID`、`FileName`、座標、回転、`Scale`、`loadType` |
| `XFileListPhysics.csv` | 衝突判定 | `ID`、`FileName`、座標、回転、`Scale`、`Type`、`Move` |
| `XFileListMove.csv` | 移動床 | `ID`、`RenderID`、`PhysicsID`、`Start`、`End`、`Duration` |
| `EnemyPositions.csv` | 敵 | `Type`、`PosX`、`PosY`、`PosZ`、`RotY` |
| `Destructibles.csv` | 破壊可能オブジェクト | 座標、`HP`。既存ヘッダーに`DropItemId`がある場合だけドロップも指定する |
| `PressurePlates.csv` | 感圧板と連動扉 | 感圧板の座標、`WallID`、扉の回転と倍率 |
| `PushableBoxes.csv` | 押せる箱 | `ID`、座標、`RotY`、`Scale` |
| `AttackTriggers.csv` | レバー、ボタン、ロープ | `Type`、トリガー座標、`TargetID`、回転軸 |
| `Interactables.csv` | QTEオブジェクトなど | `InteractionID`、`Type`、座標、`PromptDistance`。通常ステージのQTE用木は`Type=Tree` |
| `Skulls.csv` | 頭蓋骨 | `ID`、座標、`RotY` |
| `WarpBears.csv` | ワープオブジェクト | `WarpID`、`PairID`、座標、`RotY` |
| `DashBoosters.csv` | ダッシュブースター | `DashBoosterID`、座標、方向、`Speed`、`Duration`、`Radius`、`Scale`。任意列に`ChargeEnabled` |
| `LavaZones.csv` | 溶岩・ダメージ床 | `ID`、対象となる`PhysicsID`、`Damage` |
| `LavaFlood.csv` | 迫る溶岩 | `ID`、`Damage`、アンカー座標、`DirectionZ`、開始・終了時の幅と長さ、`Duration` |
| `LavaRise.csv` | せり上がる溶岩 | `ID`、`Damage`、XZ範囲、`StartY`、`EndY`、`Delay`、`Duration` |
| `Collectibles.csv` | アイテム・収集物 | `CollectibleID`、`Type`、`DataID`、座標、倍率 |
| `Stars.csv` | 一時無敵・最高速スター | `PosX`、`PosY`、`PosZ`。複数行を配置可能 |
| `SpeedUps.csv` | 基礎移動速度アップ | `PosX`、`PosY`、`PosZ`。現在の実装ではヘッダー直後の1行だけを読み込む |

移動床を追加するときは、`XFileList_simple.csv`、`XFileListPhysics.csv`、`XFileListMove.csv`の3ファイルに同じCSV IDの行を追加する。`RenderID`と`PhysicsID`も対応するIDと一致させる。

## 配置設計の基準

* ステージを広い一枚の平面として使わず、3～5個の区画に分ける。
* 各区画の主目的は、移動、戦闘、ギミック学習、慎重な操作、探索・収集、ゴール前のまとめのいずれか1つに絞る。
* 開始地点の周囲とゴール地点には、敵、ダメージ床、落とし穴、固定障害物を重ねない。
* 必須敵は主要導線上へ置き、隠し部屋や強化能力が必要な場所へ置かない。
* 敵は2～4体程度の戦闘グループを基本とする。安全区間を挟んでもよいが、すべての戦闘区画の間に必須とはしない。
* 移動床、敵、ダメージ床などの複数の脅威を近接配置し、同時にプレイヤーを妨害してよい。待機地点や着地点への敵・ダメージ床の配置も許可する。
* 複数の脅威を組み合わせる場合は、プレイヤーが状況を視認し、移動、ジャンプ、攻撃、待機タイミングの選択によって突破できるようにする。回避不能な確定ダメージや、操作できないまま落下する配置にはしない。
* 移動床の着地点は、ステージの難度と組み合わせる脅威に応じた広さにする。最低4x4mや敵との4m間隔を一律の条件にはしない。
* 必須の登り段差は1.0m前後とし、通常ジャンプの高さ1.28mぎりぎりにしない。
* 必須ジャンプの距離は理論値4.1mぎりぎりにしない。霧、カメラ、慣性、入力遅延を考慮する。
* ダメージ床と落とし穴は、床材、光、柵、石などを使って範囲と縁を判別できるようにする。
* ダッシュ、二段ジャンプ、空中ダッシュは、近道や任意の収集物に使う。正規ルートでは要求しない。
* ギミックを外周や壁の隙間から無視できないか確認する。
* ボスステージでは複雑な地形を避け、ボスの攻撃を見て回避できる空間を確保する。

## 気を付けること

* ステージの中央にだけギミックが配置されてあり、中央以外に何も配置されていないのはダメ。

## 各ステージのテーマ

* World 1
  * 1-1
    * 基本操作を試すステージ
  * 1-2
    * 基本操作を試すステージ
  * 1-3
    * 破壊可能オブジェクトを壊して通路を開拓する
      * ゴールへの道のりは破壊可能オブジェクトで阻まれていて、  
      プレイヤーはそれを壊さないとゴールに到達できません。
  * 1-4
    * 高低差のある足場を登っていく
      * テーマは登ること。ゴールが高いところにあり登っていく必要がある。  
      * 単純にジャンプするだけでは到達できず、狭い足場やモンスター、動く床によってプレイヤーは落下させられそうになる。
  * 1-5
    * ダッシュ板を乗り継いで上下に移動する
      * ゴールはダッシュ板を乗り継いだ先にあり、プレイヤーはダッシュ板を
  * 1-6
    * 落とし穴を避けて足場と砲台を渡る
  * 1-7
    * 移動床、木箱、岩、破壊可能オブジェクトで分断を突破する
  * 1-8
    * ボスステージ
* World 2
  * 2-1
    * ダメージ床を避けて安全な足場をつなぐ
  * 2-2
    * 高所の飛び石を連続ジャンプで渡る
  * 2-3
    * 無敵アイテムの効果時間を管理して走り抜ける
  * 2-4
    * 水平・昇降・斜めに動く床を乗り継ぐ
  * 2-5
    * 複数の巣を巡って敵の鳥を探し出す
  * 2-6
    * 溶岩が下からせり上がってくるので上にあるゴールを目指す
  * 2-7
    * 背後から溶岩が迫ってくるステージ
  * 2-8
    * ボスステージ
* World 3
  * 3-1
    * ボタンでポイントライトを点灯するステージ
  * 3-2
    * 感圧板で扉を開けながら水上回廊を進む
  * 3-3
    * ロープを切断して橋を作り、障害物を突破する
  * 3-4
    * ダッシュ床がたくさんあるステージ
  * 3-5
    * ワープオブジェクトがメインのステージ
  * 3-6
    * 螺旋状の陸地を回りながら中央のゴールを目指す。
      * らせん状の陸地以外は落とし穴になっている
      * リスクのある近道が存在する
  * 3-7
    * 雑魚敵がたくさんいる。
    　* ギミックは少ない。岩と弾薬系のアイテムが配置されている
  * 3-8
    * ボスステージ
* World 4
  * 4-1「押して運んで箱だらけ」
    * 押せる箱を足場、感圧板の重し、ダメージ床を渡る道として使う。
    * 押せる箱で壁を越えたり、塞がった道を開けたりする。
  * 4-2「溶岩海の横断デッキ」
    * 溶岩上の固定足場と移動床を乗り継いで対岸を目指す。
    * 移動床を使わない安全な遠回りも用意する。
    * 足場の待機地点や着地点に敵や溶岩を組み合わせてよいが、対岸と危険を事前に視認でき、操作によって突破できる配置にする。
  * 4-3「崩れた左右の峡谷」
    * 左右に分断された峡谷を交互に進む。
    * 固定足場と破壊可能オブジェクトで進路を切り替える。
    * 狭い足場で飛行敵と地上敵を同時に出す場合は、攻撃と回避に必要な足場を残し、両方の攻撃を視認できる向きに配置する。
  * 4-4「ふたつの壁をくぐる砦」
    * 二つの大きな防壁を、破壊可能な通路と迂回路で突破する。
    * 壁ごとに異なる突破方法を使い、同じ操作の繰り返しを避ける。
    * 必須敵は壁の裏に隠さず、主要導線上に配置する。
  * 4-5「空中足場の七段跳び」
    * ダッシュブースターと空中足場を連続して渡る。
    * 強化能力がなくても固定足場を通常ジャンプで進める経路を残す。
    * 着地点に敵やダメージ床を組み合わせてよい。発射中に危険を確認でき、着地後の操作で回避または戦闘できる空間を残す。
  * 4-6「木箱迷路の獣道」
    * 木箱で区切られた迷路を進み、移動床と少量のダメージ床を組み合わせる。
    * 破壊可能な箱から近道を開けるが、正規ルートは破壊能力に依存させない。
    * 敵を区画単位に分け、壁越しに多数反応させない。
  * 4-7「ゆれる溶岩の水路」
    * 溶岩の水路を固定足場、移動床、ダッシュブースターで横断する総合ステージ。
    * 固定足場、移動床、ダッシュブースター、溶岩、敵を単独または組み合わせて配置し、総合ステージとして難度を作る。
    * 最終区画は敵探しではなく、これまでの移動技術を確認する構成にする。
  * 4-8「赤砦の守護者」
    * ボスステージ。
    * 複雑な移動ギミックを置かず、ボスの攻撃を見て回避できる闘技場にする。

## 完了条件

* 通常の走行、通常ジャンプ、近接攻撃だけで開始地点から全必須敵を倒し、ゴールへ到達できる。
* 必須敵が隠し場所、落とし穴上、到達困難な場所にいない。
* 開始地点とゴール地点に、出現直後や到達直後の回避不能なダメージ判定が重なっていない。
* 移動床の待機場所と着地点に敵やダメージ床がある場合も、危険を視認でき、通常操作で回避または突破できる。
* 描画用CSVと物理用CSVの座標、回転、倍率が一致している。
* 移動床のCSV ID、`RenderID`、`PhysicsID`が一致している。
* `LavaZones.csv`の`PhysicsID`が対象の物理オブジェクトと一致している。
* 敵、アイテム、破壊可能オブジェクトが地形や落とし穴内に埋まっていない。
* QTEオブジェクトの描画、衝突判定、インタラクト位置が一致し、プロンプトを確認して意図的に起動できる。周囲に敵やダメージ床があってもよい。
* ダメージ床と落とし穴がカメラから判別できる。
* 強化能力を使った場合もステージ外や壁の裏へ侵入できない。
* 敵全滅後に敵探しや長い逆走が発生しない。
* CSVはBOM付きUTF-8、CRLFで保存する。`.x`と`.fx`はBOMなしUTF-8、CRLFで保存する。
* `Debug|x64`ビルドが成功し、出力先へ`res`がコピーされる。
