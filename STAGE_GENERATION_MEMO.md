# ステージ生成仕様メモ

## はじめに

* 以下は、自作の3Dゲームのステージ作成についてのメモである。
* 全32ステージへ、木箱、移動床、ダメージ床、ダッシュブースター、敵、スイッチなどを配置するための共通仕様をまとめる。ステージごとの広さは「ステージの基本ルール」、固有テーマは各ステージの個別設計書を参照する。

## ゲームの基本ルール

* プレイヤーはダッシュ、二段ジャンプ、空中ダッシュができるが、これらはパワーアップアイテムなので、これらがなくてもクリアできる必要がある。
  * 地上ダッシュはWorld 1、空中ダッシュはWorld 2、二段ジャンプはWorld 4でクラフト可能になる。
* プレイヤーは常に以下の3つのアクションができる。すべてのステージは、この3つのアクションだけでクリアできる必要がある。
  * こん棒で殴る
  * 走る
  * ジャンプする（高さ1.28m、距離4.1m）
* 頭蓋骨が配置されているステージでは、以下のアクションも使用できる。ただし、頭蓋骨を使わなくてもクリアできる経路を用意する。
  * ドクロを掴んで投げる
* 落とし穴に落ちると落下死するが高所から地面に着地してもダメージはない。

## ステージの基本ルール

* ステージのデザインを考えるとき、既存のステージデータがあった時は一から作り直してください。
* プレイヤーは開始地点からスタートし、指定されたゴールを目指す。
* 通常ステージでは、ステージ内の敵をすべて倒すとゴールポータルが使用可能になる。ゴールポータルに触れるとステージクリアとなる。
* ボスステージにはゴールポータルが生成されず、ボスを倒すとステージクリアになる。
* 落下穴に落ちると死亡する。
* 正規ルートは、ダッシュ、二段ジャンプ、空中ダッシュがなくてもクリアできるようにする。
* メインルートに分岐→行き止まりの寄り道を頻繁に設ける。行き止まりには報酬（アイテム・QTE木・Y≥3台座など）と敵を配置し、メインルートに合流させない（報酬を取って戻り、メインに復帰する）。寄り道は長くてよい。分岐の先にさらに分岐（入れ子）があってもよい。
* 座標はXを左右、Yを上方向、Zを奥行きとして扱い、1ユニットを1mとする。
* ボスステージ（初回プレイ＝ボスありバージョン）ではQTE、移動床、頭蓋骨、押せる箱、感圧板、ダッシュブースター、ワープオブジェクト、破壊可能オブジェクト、アイテム、雑魚敵を配置しない。地形は岩を少し置く程度の簡素な構成にする。
* ボスステージをクリアした後の再訪（ボスなしバージョン）では、以下のオブジェクトを配置してよい。ダッシュブースター（大砲）、レバー、ボタン、ロープはボスあり・ボスなしの両バージョンとも配置しない。
  * 雑魚敵（`EnemyPositionsCleared.csv`）
  * 破壊可能オブジェクト（`Destructibles.csv`）
  * 落とし穴（`stage_ground.x` のピット）
  * QTE木（`Interactables.csv` と描画・物理CSVの3ファイル）
  * アイテム（`Collectibles.csv`）
* ボスステージでクリア後専用の配置を使う場合、通常ファイル名の拡張子直前へ`Cleared`を付ける。例：`XFileList_simpleCleared.csv`、`XFileListPhysicsCleared.csv`、`CollectiblesCleared.csv`。クリア済みで対応ファイルが存在するときだけ自動的に切り替わり、存在しない場合は通常ファイルへ戻る。
* ボスステージは一度クリアした後は雑魚敵が出現するようになる。
* ステージの攻略範囲と景観は以下のようになっている。地面生成スクリプトの`size`は半幅・半奥行きであり、ここに示す値は全幅・全奥行きである。
  * World 1 : 30x60mを配置上の安全範囲とする（生成される地面スラブは32x64m）。草原・昼。
  * World 2 : 2-1〜2-3は60x120m、2-4〜2-8は120x120m。洞窟。
  * World 3 : 120x240m。草原・夕方。
  * World 4 : 120x240m。草原・夜。

### 配置物のめり込み防止

* 配置可否を中心座標だけで判断しない。モデルまたは衝突形状の幅・高さ・奥行きへCSVの倍率を適用し、回転後の占有範囲で地面の側壁、固定床、壁、岩、木箱、ほかのギミックとの重なりを確認する。
* 接触させる意図がない配置物同士には、描画誤差を含めて水平方向に0.2m以上の隙間を確保する。押せる箱は移動開始直後に引っ掛からないよう、初期位置の全底面が歩行可能な床へ載り、周囲の壁や固定床へ接触していないことを確認する。
* 6x6mの`static_platform_2x2`など、中心座標で配置する床は半幅・半奥行きを考慮する。たとえば6x6m床の端は中心から3mであり、幅1.2mの押せる箱を隣接させる場合、箱の中心は床の端から箱半分の0.6mと隙間0.2mを加えた0.8m以上離す。
* `collision_wall.x`の配置検証上の占有範囲は倍率1で1.8x8.8mである。Y軸回転後は長さ方向と厚さ方向が入れ替わるため、壁と同じXまたはZだけを見て配置を決めない。
* ダッシュブースターの描画倍率はCSVの`Scale`だけではなく、実行時にさらに3倍され、接触演出中は最大1.35倍になる。最大表示範囲でも壁や床の側面へ入らない位置に置く。
* ダッシュブースターを移動した場合は、発射方向と速度も新しい始点から着地点へ届くよう再計算する。発射元が歩行可能な床にあり、トリガー半径へ通常操作で入れること、発射経路が壁を横切らないこと、着地点が足場の支持範囲内であることを確認する。
* 生成スクリプトから作られるステージでは、出力CSVだけを直さず生成元の座標も同時に修正する。再生成後にも同じ隙間が維持されることを確認する。

### ステージ番号とフォルダーの対応

通常ステージのフォルダー名は、stage_<ワールド番号>_<ステージ番号>の形式に統一する。たとえば2-8はstage_2_8、4-2はstage_4_2とする。

| ワールド | ステージとフォルダー |
|---|---|
| World 1 | 1-1=`stage_1_1`、1-2=`stage_1_2`、1-3=`stage_1_3`、1-4=`stage_1_4`、1-5=`stage_1_5`、1-6=`stage_1_6`、1-7=`stage_1_7`、1-8=`stage_1_8` |
| World 2 | 2-1=`stage_2_1`、2-2=`stage_2_2`、2-3=`stage_2_3`、2-4=`stage_2_4`、2-5=`stage_2_5`、2-6=`stage_2_6`、2-7=`stage_2_7`、2-8=`stage_2_8` |
| World 3 | 3-1=`stage_3_1`、3-2=`stage_3_2`、3-3=`stage_3_3`、3-4=`stage_3_4`、3-5=`stage_3_5`、3-6=`stage_3_6`、3-7=`stage_3_7`、3-8=`stage_3_8` |
| World 4 | 4-1=`stage_4_1`、4-2=`stage_4_2`、4-3=`stage_4_3`、4-4=`stage_4_4`、4-5=`stage_4_5`、4-6=`stage_4_6`、4-7=`stage_4_7`、4-8=`stage_4_8` |


## 設置可能なギミック

* 固定地形
  * 地面
    * 描画用 : stage_ground.x
    * 衝突判定用 : stage_ground.x
  * ポイントライト
    * ステージフォルダー内に`PointLights.csv`を配置することで、ポイントライトを自由に設置できる。ファイルが存在しない場合は何も読み込まれない（エラーなし）。
    * `ConfigureStagePointLights()`（GameApp.cpp）によるステージ固定ライト（拠点やセレクト画面のポータル灯など）は従来通り動作し、その後に`PointLights.csv`のライトが追加で読み込まれる。両立可能。
    * CSVヘッダー（1行目）:

      ```csv
      PosX,PosY,PosZ,Brightness,ColorR,ColorG,ColorB,ColorA,Shape,LineLength,SquareWidth,SquareHeight,RotX,RotY,RotZ,Range,OwnerTag
      ```
    * 必須列は`PosX,PosY,PosZ,Brightness,ColorR,ColorG,ColorB,ColorA`（8列）。以降の列は省略可能で、省略時はデフォルト値が使われる。
    * 各列の意味:

      | 列 | 説明 | デフォルト |
      |---|---|---|
      | PosX,PosY,PosZ | ライトの座標 | （必須） |
      | Brightness | 明るさ（0.0〜） | （必須） |
      | ColorR,ColorG,ColorB,ColorA | 色（0.0〜1.0） | （必須） |
      | Shape | ライト形状:`Point`,`Line`,`Square`,`Cube`,`Sphere` | `Point` |
      | LineLength | Line形状の長さ | 12.0 |
      | SquareWidth | Square/Cube形状の幅 | 10.0 |
      | SquareHeight | Square/Cube形状の高さ | 10.0 |
      | RotX,RotY,RotZ | 回転（度） | 0,0,0 |
      | Range | ライトの影響範囲 | 12.0 |
      | OwnerTag | 所有者タグ（空欄可） | （空欄） |
    * 記述例:

      ```csv
      PosX,PosY,PosZ,Brightness,ColorR,ColorG,ColorB,ColorA,Shape,LineLength,SquareWidth,SquareHeight,RotX,RotY,RotZ,Range,OwnerTag
      -8.0,2.8,-18.0,1.0,1.0,0.34,0.08,1.0,Point,12.0,10.0,10.0,0.0,0.0,0.0,12.0,
      0.0,3.0,26.0,1.0,0.08,0.72,1.0,1.0,Point,12.0,10.0,10.0,0.0,0.0,0.0,3.2,
      ```
  * 水面
    * 水面は描画専用であり、`XFileListPhysics.csv`には登録しない。水面そのものに足場、ダメージ、遊泳の判定はない。
    * ステージフォルダー内に、水面の形に合わせた水平なメッシュを`stage_water.x`などの名前で用意する。Blenderから公式DirectX Xエクスポーターを使用して出力する。
    * 水面モデルと同じフォルダーに、拡張子だけを`.csv`へ変えた設定ファイルを置く。たとえば`stage_water.x`には`stage_water.csv`を対応させる。名前が一致しない設定ファイルは読み込まれない。
    * `XFileList_simple.csv`へ未使用のCSV IDで登録し、`loadType=meshmix2`を指定する。拠点1では`base_water.x`をY=0.04mに配置している。地面と同じ高さへ置く場合も、ちらつきを避けるため水面を少し上に配置する。
    * 登録例 : `<未使用ID>,stage_water.x,0,0.04,0,0,0,0,1,meshmix2`
    * 設定CSVは、拠点1の`res/model/base/base_water.csv`と同じ次の内容を標準とする。

      ```csv
      MeshType,WaterMirror
      Wave,1
      WaveIntensity,0.025
      WaveSpeed,0.35
      WaveDensity,2.5
      Fresnel,1
      FresnelIntensity,0.65
      WaterReflectionStrength,0.38
      WaterReflectionTint,0.12
      Shadow,0
      SSAO,0
      LambertShadow,0
      ```

    * 水面を落下場所や危険地帯として使う場合は、地面モデル側に穴を作るか、別のダメージ判定を配置する。水面モデルを物理用CSVへ追加して代用しない。
  * 木箱
    * 描画用 : res/model/cubeWoodSmall/cube_wood_small.x
    * 衝突判定用 : res/model/cubeWoodSmall/cube_wood_small_collision.x
  * 岩1
    * 描画用 : `res/model/base/base_rock1.x`
    * 衝突判定用 : `res/model/base/base_rock1_collision.x`
  * 岩2
    * 描画用 : `res/model/base/base_rock2.x`
    * 衝突判定用 : `res/model/base/base_rock2_collision.x`
  * 拠点1と同じ木
    * 装飾と固定障害物に使用する木であり、QTE用の`res/model/tree2/lemonTree.x`とは別物である。配置しただけではQTEは発生しない。
    * 元モデルは`res/model/base/source_quaternius/Tree1.blend`、`Tree2.blend`、`Tree4.blend`の3種類である。ステージの景観に合わせて種類、向き、倍率を変え、同じ木を均等に並べるだけの配置にしない。
    * `res/model/base/base_decor.x`を各ステージへ直接配置しない。このモデルには拠点1の木、岩、草などが拠点1の座標でまとめて入っている。
    * 対象ステージのBlenderファイルまたは生成スクリプトへ必要な木を読み込み、配置を反映した`stage_trees.x`などのステージ専用描画モデルとしてまとめる。公式DirectX Xエクスポーターを使用し、`axis_forward="Z"`、`axis_up="Y"`で出力する。
    * 衝突判定も同じ配置で`stage_trees_collision.x`などへまとめる。拠点1と同じ基準では、木1本につき幹の中心へ半径`0.30 × 木の倍率`m、高さ`2.2 × 木の倍率`mの円柱を置く。円柱の中心Y座標は木の根元から`1.1 × 木の倍率`m上にする。
    * 描画モデルと衝突判定モデルを、`XFileList_simple.csv`と`XFileListPhysics.csv`へ同じCSV ID、座標、回転、倍率で登録する。モデル内に配置を反映した場合は、CSV側を座標0、回転0、倍率1にする。
    * 描画側は`loadType=normal`、物理側は`Type=Collision`、`Move=n`を指定する。
    * 木を足場として使うことを前提にしない。幹の衝突判定は通り抜け防止用であり、枝葉には衝突判定を付けない。
  * 草と木の大量配置（1-1方式）
    * 柵の外など、接触しない景観用の草と木を大量に置く場合は、1-1と同じインスタンシングを使用する。オブジェクトごとに`XFileList_simple.csv`へ1行ずつ追加しない。
    * `XFileList_simple.csv`の末尾に`PlacementCsv`列を設け、モデルの行で`loadType=instancing`と配置CSVの相対パスを指定する。
    * 草モデル : `res/model/grass/grass.x`
    * 木モデル : `res/model/tree2/lemonTree.Instancing.x`
    * 1-1の草の登録例 : `301,../grass/grass.x,0,0,0,0,0,0,1,instancing,../grass/grass1-1.csv`
    * 1-1の木の登録例 : `302,../tree2/lemonTree.Instancing.x,0,0,0,0,0,0,1,instancing,../tree2/lemonTree.Instancing.1-1.csv`
    * 配置CSVでは、1行につき1個を`X,Y,Z,RotY,Scale`の順で記述する。Xモデル側の配置座標を0にすれば、配置CSVの座標をワールド座標として扱える。
    * 草の配置CSVの先頭には`sway,wave`と`AutoHide,n`を記述する。木では`AutoHide,n`を記述し、その次に`#x,y,z,RotY,Scale`という見出しを置いてよい。
    * 草は落とし穴の上へ配置しない。配置中心だけで判定せず、モデルの水平方向の半径に`Scale`を掛けた余白を加えて落とし穴との重なりを判定する。`grass.x`の水平半径は倍率1で`0.5m`のため、配置ごとの必要余白は`0.5 × Scale`mとする。
    * 配置CSVの生成処理は、`stage_ground.x`の生成に使う現在の`pits`定義を参照して草を除外する。地面と配置処理に古い落とし穴範囲を別々に保持しない。
    * 落とし穴の追加・拡張、地面サイズの変更後は草の配置を再生成または再検証し、モデル半径を含めた落とし穴との重複が0件であることを確認する。地面モデルだけを再生成して完了にしない。
    * 配置数はステージの広さと見える範囲に合わせて調整する。
    * 回転と倍率をばらつかせ、均等な格子状ではなく、密集する場所と空く場所がある自然な配置にする。
    * この方法で置く草と木は描画専用であり、`XFileListPhysics.csv`や`Interactables.csv`には登録しない。プレイヤーが触れられる木、固定障害物にする木、QTE用の木には使用しない。
  * 柵
    * 描画用 : `res/model/fence.x`
    * 衝突判定はない。落とし穴やステージ外周の縁を見せるための目印として使用し、プレイヤーを止める壁としては使用しない。
  * 壁
    * 描画用 : `res/model/collision_wall/collision_wall.x`
    * 衝突判定用 : `res/model/collision_wall/collision_wall_collision.x`
    * `XFileList_simple.csv`と`XFileListPhysics.csv`へ、同じCSV ID、座標、回転、倍率で登録する。
    * 描画側は`loadType=meshmix2`、物理側は`Type=Collision`、`Move=n`を指定する。
  * 高さが2倍の壁
    * 描画用 : `res/model/collision_wall/collision_wall_tall.x`
    * 衝突判定用 : `res/model/collision_wall/collision_wall_tall_collision.x`
    * 通常の壁より高く、強化能力を使っても越えさせたくない境界や高い防壁に使用する。
    * `XFileList_simple.csv`と`XFileListPhysics.csv`へ、同じCSV ID、座標、回転、倍率で登録する。
    * 描画側は`loadType=meshmix2`、物理側は`Type=Collision`、`Move=n`を指定する。
  * ゴールポータル
    * 通常ステージ専用であり、`XFileList_simple.csv`や専用の配置CSVへは登録しない。
    * 位置は`StageManager.cpp`の`AddStage()`へ渡す`clearPosition`で指定する。石段の基準位置は`clearPosition`のY座標から1m下になる。
    * 石段の描画用 : `res/model/portal/stone_steps.x`
    * 石段の衝突判定用 : `res/model/portal/stone_steps_collision.x`
    * 敵が残っている間は石段だけが表示される。すべての敵を倒すと光の柱とゴール方向を示す矢印が表示される。
    * 光の柱の描画用 : `res/model/portal/light_pillar.x`
    * ゴール矢印の描画用 : `res/model/arrow/arrow.x`
    * 光の柱の中心から水平方向0.9m以内に入ると旗が出現し、プレイヤーの操作が止まる。150フレーム後にステージクリアとなる。
    * 旗の描画用 : `res/model/portal/black_flag.x`
    * 旗のアニメーション設定 : `res/model/portal/black_flag.csv`
    * ボスステージでは生成しない。ボスステージのクリアはボス撃破によって判定する。
  * 移動しない床(3x3m)
    * 描画用 : res/model/static_platform/static_platform_1x1.x
    * 衝突判定用 : res/model/static_platform/static_platform_1x1_collision.x
  * 移動しない床(3x6m)
    * 描画用 : res/model/static_platform/static_platform_1x2.x
    * 衝突判定用 : res/model/static_platform/static_platform_1x2_collision.x
  * 移動しない床(6x3m)
    * 描画用 : res/model/static_platform/static_platform_2x1.x
    * 衝突判定用 : res/model/static_platform/static_platform_2x1_collision.x
  * 移動しない床(6x6m)
    * 描画用 : res/model/static_platform/static_platform_2x2.x
    * 衝突判定用 : res/model/static_platform/static_platform_2x2_collision.x
  * 移動しない床(12x12m)
    * 描画用 : res/model/static_platform/static_platform_4x4.x
    * 衝突判定用 : res/model/static_platform/static_platform_4x4_collision.x
  * 移動床
    * 水平移動、昇降、往復、斜め移動する床。圧死判定がある。
    * 昇降する移動床で高台（静的床）へ到達させる場合、移動床の昇降位置（XZ）と高台を重ねないこと。
      移動床が上昇すると、プレイヤーが移動床の上面と高台の下面に挟まれて圧死する。
      高台は移動床からXZをずらして配置し、移動床で上昇してから隣の高台へジャンプで乗り移る構成にする。
    * 描画用 : res/model/collision_moving_platform/collision_moving_platform.x
    * 衝突判定用 : res/model/collision_moving_platform.x
* 落とし穴
  * 落とし穴は別オブジェクトとして設置できない。各ステージ専用の`stage_ground.x`へ穴や窪みを設けて作成する。
  * 落とし穴は深さを100メートル以上にすること。
  * 地面の外周壁は描画しない。
  * `stage_ground.x`を手書きで編集したり、独自のXファイル変換・シリアライズ処理を作成したりしない。
  * 地面モデルはBlenderで編集し、公式DirectX Xエクスポーターの`bpy.ops.export_scene.directx_x`を使用して、`axis_forward="Z"`、`axis_up="Y"`で直接エクスポートする。
* スイッチ系オブジェクト
* レバー、感圧板は使用してはいけない。代わりにレバー２、レバー３、感圧板２、感圧板３を使用しなくてはいけない。
  * 共通仕様
    * 配置情報は`AttackTriggers.csv`に記述する。トリガーの表示モデルはプログラムが自動生成するため、トリガー本体を`XFileList_simple.csv`や`XFileListPhysics.csv`へ追加しない。
    * トリガー本体に専用の物理モデルはなく、プレイヤーの攻撃範囲と`TriggerX`、`TriggerY`、`TriggerZ`の距離で反応する。
    * `TargetID`で連動対象を指定する場合、対象オブジェクトを`XFileList_simple.csv`と`XFileListPhysics.csv`の両方へ同じCSV IDで登録する。存在しないIDを指定すると異常終了する。
    * `Axis`には`X`、`Y`、`Z`のいずれかを指定する。連動対象は`BaseRotX`、`BaseRotY`、`BaseRotZ`を基準に、その軸で90度回転する。
  * レバー
    * `Type=Lever`を指定する。攻撃するたびにON/OFFが切り替わる。
    * 有効な`TargetID`が必須である。
    * 描画用モデル : `res/model/attack_trigger/lever.x`
  * レバー2（上昇扉レバー）
    * `Type=LeverLift`を指定する。攻撃するたびにON/OFFが切り替わり、ONの間`TargetID`のオブジェクトがY方向に上昇する（OFFで元の位置に戻る）。
    * `Scale`の直後の列（12列目）に`LiftHeight`（上昇量・メートル）を指定する。
    * 有効な`TargetID`が必須である。対象は`XFileList_simple.csv`と`XFileListPhysics.csv`の両方へ同じCSV IDで登録し、物理側は`Move=y`にする（動く壁として扱われ、プレイヤーを押す）。
    * 描画用モデル（レバー本体） : `res/model/attack_trigger/lever.x`（既存レバーと同じものを自動配置）
    * セットの箱モデル : `res/model/attack_trigger/lever_box.x`（外寸6x6x6m・壁厚0.5m・底面中心原点・-Z側が開口部）
    * 扉モデル : `res/model/attack_trigger/lever_box_door.x`（6x6x0.5m・下端原点・厚さ0.5m）
    * 床モデル : `res/model/attack_trigger/lever_box_floor.x`（10x10x1.0m・原点=下面中心）。箱の下に敷き、**床のPosYは箱・扉のPosYより1.0低くする**。箱・扉・レバーは床の上面に配置し、レバーは床の範囲内に置く。
    * 扉は**箱の開口部側（-Z側）**に配置する（箱のPosZ - 3の位置）。壁側（+Z側）に置かないこと。
    * 扉と箱のポリゴンが重なって点滅しないよう、扉の`Scale`は**0.98（2%縮小）**で登録する。`XFileList_simple.csv`・`XFileListPhysics.csv`・`AttackTriggers.csv`の3ファイルすべて同じ値にする（LeverLiftが`AttackTriggers.csv`のScaleで上書きするため）。
    * 箱の内側にアイテムを置く場合は箱の中央付近に置く。壁の外からアイテム取得距離（0.55m）に入らないため、レバーで扉を開けるまで取得できない。
    * 扉を箱の開口部（箱の-Z側）に合わせる場合、扉のCSV座標は箱の座標-Zに-3した位置にする（例: 箱がz=10なら扉はz=7）。
  * レバー3（両開き上昇扉レバー）
    * `Type=LeverLift`のバリエーションで、上下する壁が**2枚セット**になったもの。
    * 箱モデル : `res/model/attack_trigger/lever_box3.x`（外寸6x6x6m・壁厚0.5m・底面中心原点。±X側が壁、**+Zと-Zの2面が開口部**）
    * 扉モデル : `res/model/attack_trigger/lever_box3_door.x`（**2枚の扉が1つのモデルにまとまっている**。+Z側と-Z側に各1枚・下端原点・厚さ0.5m）
    * 床モデル : `res/model/attack_trigger/lever_box3_floor.x`（5x10x1.0m・原点=下面中心）。箱の下に敷き、**床のPosYは箱・扉のPosYより1.0低くする**。箱・扉・レバーは床の上面に配置し、レバーは床の範囲内に置く。
    * 扉のCSV座標は**箱と同じ座標**にする（モデル内で±Zに扉が配置済みのため）。
    * レバーで2枚の扉が**同時に上昇**し、箱を通り抜けられる（両側が開く）。
    * レバーは**箱の外側に2つ（各扉の前）と箱の内側に1つ**の合計3つを配置し、すべて同じ`TargetID`を共有する。外側の2つは減らさない。すべてのレバーを扉の中心線から扉幅方向へ1.5mずらし、通路の中央を空ける。
    * 箱内レバーは`TriggerY=箱のPosY+0.5`（内蔵床の上面）とする。箱内で扉を閉じても、このレバーから再び開けられるようにする。
  * レバー2・レバー3・感圧板2・感圧板3 まとめ（箱＋上下する壁のセット）
    * いずれも「6x6x6mの箱＋上下する壁（扉）」のセット。箱でアイテムを囲み、操作しないと取得できない（感圧板3/レバー3は「門」として通路を塞ぐ）。
    * 共通ルール
      * 箱・扉・箱外の操作装置（レバー/感圧板）は**床の上面に配置**する。1.0m厚の`lever_box_floor.x`・`lever_box_floor14.x`・`lever_box3_floor.x`・`lever_box3_floor14.x`では、床の`PosY`を箱・扉の`PosY - 1.0`にする。
      * `lever_box.x`・`lever_box3.x`には、箱の`PosY`から0.5m高い位置まで内蔵床がある。**箱内の感圧板を箱・扉と同じYに置くと内蔵床へ埋没し、判定だけ反応して表示されない。** 箱内感圧板の`PlatePosY`は`箱のPosY + 0.5 + 0.01`（床上面から1cm上。例: 箱Y=0.7なら板Y=1.21）にする。箱外の感圧板は外床上面から1cm上に置く。
      * 箱外レバーの`TriggerY`は**設置床の上面**（箱下の床なら箱の`PosY`と同じ）にする。箱内レバーの`TriggerY`は**箱のPosY + 0.5**（内蔵床の上面）にする。箱内外とも、扉の中心線から扉幅方向へ1.5mずらす。
      * 扉の`Scale`は**0.98（2%縮小）**。`XFileList_simple.csv`・`XFileListPhysics.csv`・`AttackTriggers.csv`（または`PressurePlates.csv`）のすべてで同じ値にする。
      * 扉は`XFileListPhysics.csv`で`Move=y`にする。
      * アイテムは箱の中央付近（`PosY=箱PosY + 0.45`）に置く。
      * 扉の上昇量は6m（レバーは`LiftHeight=6`、感圧板は`TravelDistance=6`）。
      * **全感圧板共通の閉じ込め防止ルール**: 感圧板と連動する扉が閉じたときにプレイヤーが取り残される可能性のあるすべての区画へ、同じ`WallID`を持つ感圧板を最低1つ配置する。箱ギミックでは**箱の外側だけでなく箱の内側にも必ず感圧板を置く**。この規則は新規生成だけでなく、既存ステージを修正・再生成するときにも適用する。
      * 箱内の感圧板は、扉が閉じた状態でもプレイヤーが到達して踏める位置に置く。押せる箱、ドクロ、退路、扉が閉じる前の駆け抜けを、箱内感圧板の代替にしてはいけない。
      * **床自体が足場として機能する**（衝突体あり）。レバー2/3・感圧板2/3は`static_platform`の上に置く必要はなく、地面なしステージでは足場間の橋として使える。床の上面と箱の`PosY`を0.5mより高くし、床の範囲を地面スクリプトの`static_platforms`に登録する。地面ありステージでは周囲の足場へ重ならない地上ゲートとして配置する。
      * **複数の操作装置で同じ扉を共有できる**（門）。感圧板は同じ`WallID`、レバーは同じ`TargetID`を複数行で指定してよい。
      * **レバー3/感圧板3を迂回不能な門にする場合、門の左右と奥側をピットまたは壁で塞ぐ。** 壁やピットと箱の間にプレイヤーが通れる隙間を残さず、壁の衝突矩形を足場へめり込ませない。

| ギミック | 箱モデル | 扉 | 操作装置 | 床 | 動作 |
|---|---|---|---|---|---|
| レバー2 | `lever_box.x` | 1枚 `lever_box_door.x` | レバー1つ | `lever_box_floor.x`（10x10x1.0） | レバーを攻撃するたびに扉が上昇⇔下降（トグル） |
| レバー3 | `lever_box3.x` | 2枚 `lever_box3_door.x` | レバー3つ（各扉の前に1つずつ＋箱内に1つ。すべて中心線から横へ1.5m） | `lever_box3_floor.x`（5x10x1.0） | どのレバーでも両扉が同時に上昇⇔下降（共有トグル）。箱内から再開放可能。**門** |
| 感圧板2 | `lever_box.x` | 1枚 `lever_box_door.x` | 感圧板2つ以上（扉の外側＋箱内） | `lever_box_floor14.x`（10x14x1.0） | どの感圧板でも同じ扉が上昇し、箱内から脱出可能（共有WallID・OR） |
| 感圧板3 | `lever_box3.x` | 2枚 `lever_box3_door.x` | 感圧板3つ以上（各扉の外側に1つずつ＋箱内中央に1つ） | `lever_box3_floor14.x`（5x14x1.0） | どの感圧板でも両扉が同時に開き、箱内から脱出可能（共有WallID・OR）。**門** |

  * ボタン
    * `Type=Button`または`Type=TimedButton`を指定する。いずれかのボタンを攻撃するとステージ内の全ボタンがONになり、10秒後にOFFになる。
    * 連動対象を動かさずライトだけを操作する場合は`TargetID=-1`を指定できる。
    * ライトを設定する場合は、`LightBrightness`、`LightRange`、`LightR`、`LightG`、`LightB`の5列をすべて指定する。
    * OFF描画用モデル : `res/model/pressure_plate/pressure_plate_black.x`
    * ON描画用モデル : `res/model/pressure_plate/pressure_plate_green.x`
  * ロープ
    * `Type=Rope`を指定する。攻撃すると一度だけ切断され、連動対象が指定軸で90度回転する。
    * 有効な`TargetID`が必須である。
    * 描画用モデル : `res/model/attack_trigger/rope.x`
  * 連動対象に使用できる床と壁
    * 床モデル : `res/model/attack_block/attack_floor.x`
    * 壁モデル : `res/model/attack_block/attack_wall.x`
    * 描画用と衝突判定用に同じモデルを使用し、両方のCSVへ同じCSV ID、座標、回転、倍率で登録する。
    * スイッチから動かす場合は、`AttackTriggers.csv`の`TargetID`にこのCSV IDを指定する。
* 敵
  * 共通仕様
    * 配置情報は`EnemyPositions.csv`に記述する
    * `Type`で敵の種類を指定する
    * 描画にはアニメーション付きXファイルを使用する
    * 専用の衝突判定用Xファイルは使用せず、プログラム側の円柱判定を使用する
    * スタート地点から12m以内にモンスターを配置しない。ゴール地点からも7m以上離す。
    * 通常ステージ1つあたりの配置数は、以下の数以上となること。
      * World 1 : 10体
      * World 2 : 15体
      * World 3、World 4 : 20体
    * ボスステージはこの対象外とし、既存のボスステージ用ルールに従って雑魚敵を配置しない。
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
  * ワールド別の配置方針
    * World 1
      * 主に`wolf`、`small_mushroom`、`crab`、`frog`を配置する。
      * `bird`は1-4以降で少数だけ登場させる。
      * ゴースト、クモ、スケルトン、ゴーレム、大型敵は配置しない。
      * 接近してくる敵を相手に、基本的な移動、回避、攻撃を覚えるワールドにする。
    * World 2
      * 主に`small_spider`、`spider`、`small_golem`を配置する。
      * `bird`は空中足場を使うステージと2-1に限って配置する。
      * オオカミ、カニ、カエル、キノコ、ゴースト、スケルトンは配置しない。
      * ジャンプする敵、空中の敵、足止めする敵を危険な地形と組み合わせるワールドにする。
    * World 3
      * 主に`ghost`、`small_skeleton`、`skeleton`を配置する。
      * `bird`は3-4だけ、`spider`は3-7だけに再登場させる。
      * World 1の地上動物、ゴーレム、キノコは配置しない。
      * 遠距離攻撃、浮遊、毒、クモの糸などに対応するワールドにする。
    * World 4
      * 主に`enemy2`、`mushroom`、`golem`を配置する。
      * ゴースト、スケルトン、クモは、ステージの仕掛けと役割が合う場合に選んで再登場させる。
      * カエルや小型キノコなど、序盤向けの敵は配置しない。
      * スーパーアーマーや高いHPを持つ大型敵と、過去の敵を組み合わせた総合戦のワールドにする。
      * それまでボスとして登場した敵を通常の強敵として再登場させ、プレイヤーの成長を感じられる構成にしてよい。
  * ボスの割り当て
    * 1-8 : `boss_giant_crab`
    * 2-8 : `boss_golem`
    * 3-8 : `boss_hoshigirl`
    * 4-8 : `boss_kanata`
    * ボスステージの初回プレイ（ボスありバージョン）ではボスだけを配置し、雑魚敵は配置しない。
  * クリア済みボスステージの雑魚敵
    * ボスステージをクリアした後は、`GetEnemyCsvPathForStage()`が`EnemyPositionsCleared.csv`を読み込むよう切り替わる。
    * このファイルはボスステージのフォルダー（`stage_1_8`など）へ配置し、`EnemyPositions.csv`と同じ列構成（`Type,PosX,PosY,PosZ,RotY`）にする。
    * 初回はボスだけを戦わせるため`EnemyPositions.csv`にはボス1体のみを記述し、クリア後の再訪用に`EnemyPositionsCleared.csv`へ雑魚敵を記述する。ファイルが存在しないとロード時に異常終了するため、ボスステージを作成するときは必ず両方のファイルを用意する。
  * 敵を初登場させるときのルール
    * 1ステージで初登場させる敵は、原則として1種類にする。
    * 小型版がある敵は、`small_spider`から`spider`、`small_skeleton`から`skeleton`、`small_golem`から`golem`、`small_mushroom`から`mushroom`の順に登場させる。
    * 初登場する区画では、その敵の動きや攻撃を確認できる配置にする。
    * 一度特徴を理解させた後は、移動床、敵、ダメージ床を近くに配置し、複数の脅威を同時に処理させてよい。
    * 1つの区画に多数の種類を混ぜず、役割の異なる2～3種類を組み合わせる。
  * その他の配置ルール
    * レバー2、レバー3、感圧板2、感圧板3に付属している箱の中にモンスターを配置してよい。ただし、箱の壁や床の中にモンスターを配置しない。
    * 木箱や壁オブジェクトの中にモンスターを配置しない。
    * `enemy3`、`enemy4`、`enemy5`、`enemy6`はプログラムには登録されているが、正式な仕様が決まるまで配置しない。
    * 敵が落とすクラフト素材の種類は敵ごとに分かれておらず、共通の候補からランダムに選ばれる。
    * 敵の種類は戦闘難度とワールドの雰囲気を基準に選ぶ。
    * クラフト素材の固定配置と確定ドロップは必要数の一部だけにし、それだけで対象レシピの必要数がそろわないようにする。
    * 残りの素材は敵のランダムドロップを集めて補う設計にし、解禁直後に必ずクラフトできる状態にはしない。
    * QTEや破壊可能オブジェクトから得られるランダム素材は追加報酬として扱い、必要数の計算には含めない。
    * ランダムドロップを集めることは必要にするが、同じステージの過度な周回を要求しないよう、通常経路にも十分な数の敵を配置する。
* 破壊可能木箱
  * 描画用 : `res/model/cubeWoodBreakable/cube_wood_breakable.x`
  * 衝突判定用 : `res/model/cubeWoodBreakable/cube_wood_breakable_collision.x`
  * 外寸は1x1x1mで、モデル原点は底面中心にある。ローカルY範囲は0〜1m。
  * 配置情報 : `Destructibles.csv`
    * `PosX`、`PosY`、`PosZ` : 配置座標。`PosY`は木箱の中心ではなく、木箱底面の高さを指定する。
    * `HP` : 耐久力。破壊可能木箱はすべて`1`とする
    * `DropItemId` : 破壊時に必ず落とすアイテム。`None`を指定すると何も落とさない
  * Y=0の平坦な地面へ置く場合は`PosY=0`とする。地面から浮かせるための一律オフセットを加えない。
  * 静的足場の中央床面へ置く場合は、`PosY=足場のPosY + 0.203 × 足場のScale`とする。外周の縁は中央床面より0.1m高いため、木箱の中心を縁へ重ねない。
  * 縦に積む場合は、上段の`PosY=下段のPosY + 1.0`とする。
  * 最下段の木箱中心は、地面または静的足場の支持範囲内に置く。支持面のないピット上へ空中固定してはいけない。
  * 道幅より広い破壊可能壁を作る場合も、外側の木箱を空中へ張り出させない。必要なら支持用の地形・静的足場を追加するか、支持範囲内まで木箱数を減らす。
* 感圧板と連動扉
  * プレイヤーが踏むか、「押せる箱」を乗せるか、ドクロを乗せると反応する。
  * 連動扉が上に移動する。
  * 連動扉はトリガーが有効な間だけ開き、離すと2m/sで閉じる。
  * **すべての感圧板で閉じ込め防止を必須とする。** 連動扉の両側、または扉によって閉鎖される各区画に、同じ`WallID`の感圧板を最低1つずつ置く。完全密閉部屋や箱には内側の感圧板が必須であり、外側にしか感圧板がない配置は禁止する。
  * ドクロや押せる箱を重しにする遊びを併設してもよいが、内側の感圧板を省略してはいけない。
  * `TravelDistance`列（10列目）を指定すると、連動扉の上昇量（メートル）を変更できる。省略時は3.0m（既存動作のまま）。
  * 感圧板2・感圧板3（箱と上昇扉のセット）
    * レバー2・レバー3と同じ箱ギミックを感圧板で起動するもの。`TravelDistance=6`を指定する（扉6mを開ける）。
    * 感圧板2 : 箱`res/model/attack_trigger/lever_box.x`＋扉1枚`lever_box_door.x`。感圧板は**扉の外側に1つ、箱の内側に1つの合計2つ以上**を配置し、同じ`WallID`を共有する。外側の感圧板は扉の目の前の面に配置する（感圧板と反対側に置かない）。箱は開口部が外側の感圧板を向くよう`RotY=180`等で回転させる（例: 外側の感圧板が+Z側なら箱をRotY=180にして扉は箱の+Z側に+3した位置）。箱内の感圧板は扉が閉じても踏める位置に置く。
    * 感圧板3 : 箱`res/model/attack_trigger/lever_box3.x`＋扉2枚`lever_box3_door.x`（両開き）。扉は箱と同じ座標。感圧板は**各扉の外側に1つずつ、箱の内側中央に1つの合計3つ以上**を配置し、すべて同じ`WallID`を共有する。どの感圧板に乗っても**両方の扉が同時に開く**（いずれか1つがアクティブな間は開いたまま）。レバー3と同じ**「門」**になり、箱内からも必ず再開放できる。
    * 床モデル : 感圧板2は`res/model/attack_trigger/lever_box_floor14.x`（10x14x1.0m・X=10m/Z=14m）、感圧板3は`res/model/attack_trigger/lever_box3_floor14.x`（5x14x1.0m・X=5m/Z=14m）。いずれも原点=下面中心。箱の下に敷き、箱・扉・箱外感圧板を床の上面に配置する。床のPosYは箱・扉のPosY-1.0とし、箱外感圧板は床上面から1cm上に置く。箱内感圧板は箱モデルの0.5m厚の内蔵床上面から1cm上、すなわち`箱のPosY + 0.51`に置く。感圧板は床の範囲内に置く。
    * 扉は`XFileListPhysics.csv`で`Move=y`にする。扉の`Scale`は0.98（2%縮小）で、`XFileList_simple.csv`・`XFileListPhysics.csv`・`PressurePlates.csv`の3ファイルすべて同じ値にする。
* 押せる箱
  * 押して移動する。`PushableBoxes.csv`に`ID`、座標、`RotY`、`Scale`を記述する（物理・描画CSVには登録しない）。
  * **1.2m立方**（`pushable_box.x` / `pushable_box_collision.x`）。ジャンプ（最大上昇1.28m）で上に乗れる高さ。
  * **「押せる箱 + 高台」登りギミック**: 高台（static_platform_2x2等）の上面を**2.2m以下**にすると、「箱の上（1.2m）→ ジャンプ（1.28m）→ 2.48m」で登れる。箱を高台の縁まで押してから乗る。高台の上に敵+アイテムを置くと「登って戦って取る」報酬型の寄り道になる。
  * 感圧板の上に押し込むと、感圧板が開きっぱなしになる（連動扉の閉じ込め回避に使える）。
* ワープオブジェクト
  * 触れると、同じ`PairID`を持つもう一方のワープオブジェクトへ移動する。
  * `WarpID`はステージ内で重複させず、1つの`PairID`につき必ず2個を配置する。
* ダッシュブースター
  * 見た目は大砲として実装されており、配置情報は`DashBoosters.csv`に記述する。
  * 判定半径内に入ったプレイヤーを、指定した方向、速度、効果時間で射出する。
  * `ChargeEnabled`を省略した場合は有効になる。既存CSVにこの列がある場合は列構成を維持する。
* ダメージ床
  * 描画用・衝突判定用 : `res/model/plateLava.x`
  * `XFileList_simple.csv`と`XFileListPhysics.csv`へ同じCSV ID、座標、回転、倍率で登録する。
  * `LavaZones.csv`で対象の物理オブジェクトを指定し、触れたプレイヤーへダメージを与える。
* 迫る溶岩
  * `LavaFlood.csv`で生成する。アンカー位置から`DirectionX`・`DirectionZ`方向へ進みながら、開始時の幅と長さから終了時の幅と長さまで変化する。
  * `Delay`で区間ごとの開始時刻をずらせる。複数行を経路順に起動し、角で少し重ねることで、S字や折れ曲がったコースを追う溶岩を作れる。
  * 旧形式の`DirectionZ`だけを持つ11列のCSVも引き続き読み込める。新規ステージでは方向と待機時間を指定できる13列形式を使う。
* せり上がる溶岩
  * `LavaRise.csv`で生成する。指定したXZ範囲を覆い、待機時間後に`StartY`から`EndY`まで上昇する。
* 頭蓋骨
  * 掴んで投げることができる。
* アイテム
  * クラフト素材とクラフト素材以外の収集物は`Collectibles.csv`へ配置する。
  * `Collectibles.csv`の`Type`には`Item`または`Weapon`だけを指定する。
  * `Type=Item`では、クラフト専用アイテムIDの`007`と`008`を`DataID`に指定しない。指定すると異常終了する。
  * `DataID`はゼロ埋め3桁（`001`〜`017`）で指定する。`9`や`10`のような非ゼロ埋めは、拾得時にアイテム名カタログのキーと一致せず異常終了する。カタログ、`CraftRecipes.csv`、敵・破壊物のドロップIDも同じ形式で統一する。
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
  * 例：`stage_1_1-tree-01,Tree,10,0,22,2.5`
  * `InteractionID`はステージ内で重複しない名前にする。`PromptDistance`は木の中心へ無理なく近づける範囲とし、既存例では2.5mを使用している。
  * QTE中はプレイヤー、カメラ、敵、インタラクトの更新が止まる。敵やダメージ床の近くに配置して開始前後の緊張感を作ってよいが、プロンプトを確認して意図的に起動できる位置にする。
  * QTEの成功をクリア必須条件にしない。失敗しても正規ルートの進行や必須敵の撃破が可能な構成にする。
  * ボスステージの初回プレイ（ボスありバージョン）には配置しない。クリア後のボスなしバージョンでは配置してよい。

## 配置データの出力先

各ステージの配置情報は、対応する`res/model/stageN/`フォルダー内のCSVへ記述する。既存CSVの列順を変更しない。

| CSV | 用途 | 主な列・注意点 |
|---|---|---|
| `XFileList_simple.csv` | 描画モデル | `ID`、`FileName`、座標、回転、`Scale`、`loadType`。インスタンシングでは末尾の`PlacementCsv`も使用する |
| `XFileListPhysics.csv` | 衝突判定 | `ID`、`FileName`、座標、回転、`Scale`、`Type`、`Move` |
| `XFileListMove.csv` | 移動床 | `ID`、`RenderID`、`PhysicsID`、`Start`、`End`、`Duration` |
| `EnemyPositions.csv` | 敵 | `Type`、`PosX`、`PosY`、`PosZ`、`RotY` |
| `Destructibles.csv` | 破壊可能オブジェクト | 座標、`HP`。既存ヘッダーに`DropItemId`がある場合だけドロップも指定する |
| `PressurePlates.csv` | 感圧板と連動扉 | 感圧板の座標、`WallID`、扉の回転と倍率。`WallID`は描画・物理CSVの両方に必要。10列目に`TravelDistance`（上昇量、省略時3.0m） |
| `PushableBoxes.csv` | 押せる箱 | `ID`、座標、`RotY`、`Scale` |
| `AttackTriggers.csv` | レバー、ボタン、ロープ | `ID`、`Type`、トリガー座標、`TargetID`、`Axis`、基準回転、倍率。ボタンでは任意のライト5列を末尾に追加 |
| `Interactables.csv` | QTEオブジェクトなど | `InteractionID`、`Type`、座標、`PromptDistance`。通常ステージのQTE用木は`Type=Tree` |
| `Skulls.csv` | 頭蓋骨 | `ID`、座標、`RotY` |
| `WarpBears.csv` | ワープオブジェクト | `WarpID`、`PairID`、座標、`RotY` |
| `DashBoosters.csv` | ダッシュブースター | `DashBoosterID`、座標、方向、`Speed`、`Duration`、`Radius`、`Scale`。任意列に`ChargeEnabled` |
| `LavaZones.csv` | 溶岩・ダメージ床 | `ID`、対象となる`PhysicsID`、`Damage` |
| `LavaFlood.csv` | 迫る溶岩 | `ID`、`Damage`、アンカー座標、`DirectionX`、`DirectionZ`、開始・終了時の幅と長さ、`Delay`、`Duration`。複数区間は角で重ね、直前区間の終了時刻に次区間を開始する |
| `LavaRise.csv` | せり上がる溶岩 | `ID`、`Damage`、XZ範囲、`StartY`、`EndY`、`Delay`、`Duration` |
| `Collectibles.csv` | アイテム・収集物 | `CollectibleID`、`Type`、`DataID`、座標、倍率。`Type`は`Item`または`Weapon`。`DataID`はゼロ埋め3桁（`001`〜`017`）で指定する（非ゼロ埋めは拾得時に異常終了） |
| `Stars.csv` | 一時無敵・最高速スター | `PosX`、`PosY`、`PosZ`。複数行を配置可能 |
| `SpeedUps.csv` | 基礎移動速度アップ | `PosX`、`PosY`、`PosZ`。現在の実装ではヘッダー直後の1行だけを読み込む |

移動床を追加するときは、`XFileList_simple.csv`、`XFileListPhysics.csv`、`XFileListMove.csv`の3ファイルに同じCSV IDの行を追加する。`RenderID`と`PhysicsID`も対応するIDと一致させる。

`AttackTriggers.csv`の`TargetID`と`PressurePlates.csv`の`WallID`で連動対象を指定するときは、対象を`XFileList_simple.csv`と`XFileListPhysics.csv`の両方へ同じCSV IDで登録する。レバーとロープでは`TargetID=-1`を使用できない。ボタンだけはライト専用として`TargetID=-1`を使用できる。

## 地面モデルの生成と検証

### 生成手順（`_build_stage_grounds.py`）

* `stage_ground.x`はカスタムシリアライザで生成しない。`res/model/ground/_build_stage_grounds.py`をBlenderで実行して生成する。
* 1ステージだけを再生成・検証する場合は環境変数`RED_FORTRESS_STAGE_GROUND`に`"<表示名>"`（例 : `3-5`）を設定して実行する。

  ```powershell
  $env:RED_FORTRESS_STAGE_GROUND='3-5'
  & "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background --factory-startup --python "res\model\ground\_build_stage_grounds.py"
  ```

* このスクリプトは**検証と地面出力を同時に実行**する。検証に失敗するとエラーで停止し、`stage_ground.x`は出力されない。
* 環境変数を設定せずに実行すると、全32ステージの検証・再生成と`stage_grounds.blend`の保存が行われる。
* 出力先は`res/model/<フォルダー名>/stage_ground.x`（BOMなしUTF-8・CRLF）。
* 検証は既存の描画CSV・物理CSV・敵CSV等を読み込むため、**CSV生成 → 地面生成 → ビルド**の順で実行する。

### `validate_stage()`の通過条件（失敗するとビルド停止）

* スタート位置とゴール位置がピット（落とし穴）の上にないこと。
* 敵・収集物・SpeedUps・DashBoostersの座標がピット内にないこと（静的プラットフォーム上なら例外）。
* World 1の`Destructibles.csv`と`DestructiblesCleared.csv`について、最下段が地面または静的足場へ接地し、上段が1m下の木箱に積まれていること。接地高さの許容誤差は±0.02mとする。
* `XFileList_simple.csv`の各オブジェクトがピット内にないこと（ground/platefield/skysphere/fence/移動床はスキップ）。
* 壁・木箱・木の衝突矩形（物理CSVのIDから算出）がピットと交差しないこと。
* スタート→ゴールの安全ルート（BFS）が存在すること。
* 移動床でピット帯を渡る場合、スイープ（Start〜Endを結ぶ帯 ± `1.5 × Scale`、BFSはプレイヤーマージン0.45を加算）がピット帯のZ範囲を覆う必要がある。1-7はフェリー2基をZ=0付近で重ねて対応、1-3はScale2の1基（ピット帯 z∈[-13,-7] にStart/End z=-10）で全域をカバー。
* **ワープで区画を密封する設計では、STAGES定義に`jump_links`（ワープペアの座標組）を追加しないと「no safe route」で失敗する。**
* **ピット上にY座標3以上の静的プラットフォームを置き、その上にアイテム等を配置する場合、STAGES定義に`static_platforms=((x, z, half),)`を追加しないと「ピット内配置」扱いで検証NGになる（1-3の堀上台座が該当）。**
* ピットはSTAGES定義の`pits`に`(x0, x1, z0, z1)`で指定し、外周まで拡張できる。外周際を歩いてギミックを迂回できないか確認する。

#### 定義の参照先

`STAGES`定義は頻繁に更新されるため、この文書へ定義のコピーを置かない。常に`_build_stage_grounds.py`内の対象ステージを参照する。

## ギミックの配線ルールとモデル寸法

### 配線ルール

* レバー・ロープの連動対象は、描画CSVと物理CSVの両方に同じIDで登録し、物理側は`Move=y`にする。
* ボタンは`TargetID=-1`でライト専用にできる。その場合`LightBrightness`〜`LightB`の5列を末尾に付加する。
* トリガーが連動対象を回転させる角度は90度固定（`kTargetAngle = π/2`）。
* `attack_wall`の90度回転は中心まわりで幅4×奥行0.6⇄0.6×4が入れ替わる。高さ3m（Scale1）なのでプレイヤー（高さ1.7m・ジャンプ約1.28m）は潜れず跳び越せず、封鎖は機能する。回転後は壁が0.6m幅の横向きになり通路が開通するため、通路幅に合わせてScaleを調整し、レバー・ロープで開く本物の扉として使える（1-1のレバー壁が該当）。
* ワープオブジェクトは描画CSVのみに登録し、物理CSVへは登録しない。
* 移動床の物理モデルのパスは`res/model/collision_moving_platform.x`（描画用とはパスが異なる）。

### モデル寸法一覧（配置設計の基準）

| モデル | 寸法（メートル） | 備考 |
|---|---|---|
| `attack_wall` | 幅4 × 高さ3 × 奥行0.6 | レバー連動の壁（メッシュ実測＋ゲーム内軸補正で確定。高さ3mの全高壁なので封鎖は機能し、90度回転で奥行側（0.6m）が通路方向を向き開通する） |
| `attack_floor` | 水平スラブ 幅4 × 奥行3、厚み0.45 | ロープ切断でX軸回転→橋（メッシュ実測） |
| `collision_wall` / `collision_wall_tall` | 衝突矩形 1.8 × 8.8 | `_build_stage_grounds.py`の`load_collision_rectangles`基準。RotY=90で幅・奥行が入れ替わる |
| `static_platform_2x2` | 6 × 6 | 床の上面は配置Y付近 |
| `static_platform_4x4` | 12 × 12 | 2x2を平面だけ2倍。レバー3/感圧板3の箱（6x6）＋前庭を載せられる |
| 移動床（衝突判定） | 3 × 0.4 × 3 | `Scale`で拡大。昇降・高台との重なりに注意 |
| `plateLava` | 半径 4.0 × Scale | `_build_stage_grounds.py`の`load_lava_zones`基準 |

## 配置設計の基準

* **区画数は必達要件。設計開始時に区画数を決めてから各ギミックを配置する。**
  * 区画数が不足する設計（ワールド3/4で18区画未満など）は要件を満たさないため、ステージとして完成と見なさない。
  * 区画は3〜18個の範囲で分ける。1区画に複数の主目的を詰め込まず、区画ごとに主目的を1つに絞る。
  * 独立した主目的を持つ場所だけを区画として数え、階段の各段や単純な乗降床は数えない。
* **ステージを広い一枚の平面として使わず、3～18個の区画に分ける。**
  * ステージ1-1 ~ 1-7 : 約3区画
  * ステージ2-1 ~ 2-7 : 約12区画
  * ステージ3-1 ~ 3-7 : 約18区画
  * ステージ4-1 ~ 4-7 : 約18区画
* 各区画の主目的は、移動、戦闘、ギミック学習、慎重な操作、探索・収集、ゴール前のまとめのいずれか1つに絞る。
* 区画の作成方法は、以下のどれかの方法で行う
  * 動かない床を配置する
  * 壁で覆う
  * 落とし穴で覆う
* 区画は格子状に配置してはいけない。位置と大きさにランダム性を持たせること。
* 区画の位置は左右前後だけではなく上下方向にもランダム性を持たせること。その時、登れない区画ができないか注意すること。
* **区画のX座標が中央（-15m〜+15m）に偏りすぎていないか確認すること。**
  * 例: ワールド3のステージはX方向が-60m〜+60mの広さ。しかし区画のX座標が-15m〜+15mに集中し、-15m以下・+15m以上には区画がほとんど存在しない。
  * 端ギミックを置くだけでなく、主たる攻略区画自体もX範囲の両端側へ分散させること。開始・ゴール周辺だけはこの限りではない。
* 開始地点の周囲とゴール地点には、敵、ダメージ床、落とし穴、固定障害物を重ねない。敵は開始地点から12m以上、ゴール地点から7m以上離す。
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
* 柵の外側を何もない空間にせず、景観用の木、岩、草を大量に配置する。柵のすぐ外だけへ一列に並べず、奥まで続くように密度、向き、倍率を変えて配置する。
* World 1、World 3、World 4の柵外には木、岩、草を組み合わせる。World 2の柵外は岩だけを配置し、木と草は配置しない。
* 柵外の装飾はプレイヤーが触れないため描画専用とする。カメラを遮ったり、進める道があるように誤解させたり、柵の内側の敵やギミックを見えにくくしたりしない。
* ボスステージでは複雑な地形を避け、ボスの攻撃を見て回避できる空間を確保する。

## 各ステージのテーマ

### 「地面なしステージ」と書かれていたら以下のルールに従う
* 地面はすべて落とし穴。動く床、動かない床で足場を構築する。
  * 実装パターン（1-3で確立）
    * ピットは外周壁まで拡張できる。外周に地面の帯を残す構成と全面ピット化のどちらでもよいが、外周際の徒歩迂回が成立しないことを確認する。
    * STAGES定義に`"elevated_route": True`を設定する（足場上の固定オブジェクト＝レバー3/感圧板3の箱などをBFSで通過可能にする）。
    * BFSはジャンプを考慮しないため、足場間のギャップは`jump_links`で接続する。**端点は`margin=-0.45`で縮んだ「supportedセル」**（例: 2x2足場(0,26,3.0)なら(0,24)〜(0,28)）に置く。
    * start/goalは足場の上に置ける（検証は「ピット外の地面 or 静的プラットフォームの上」を許可）。
    * レバー2/3・感圧板2/3の床上面・箱は`PosY`0.5超（例: 1.0m厚床の下面-0.3・上面0.7、箱0.7）で足場間のピット帯に置き、床の範囲を`static_platforms`に近似登録する（例: 床5x10→half5.0、床5x14→half7.0）。
    * 破壊可能オブジェクト（ガレキ）は`XFileListPhysics.csv`に登録しないため、検証のBFSでは障害物扱いしない。ゲーム内では衝突体が壊すまで通行を阻む。BFS通過だけで配置の正当性を判断せず、接地・積み重ね検証も通す。

### World 1

| ステージ | テーマ | 詳細 |
|---|---|---|
| 1-1 | 基本操作を試す | [STAGE_1_1.md](STAGE_1_1.md) |
| 1-2 | 木箱を使った基本操作 | [STAGE_1_2.md](STAGE_1_2.md) |
| 1-3 | ガレキを壊して進路を作る地面なしステージ | [STAGE_1_3.md](STAGE_1_3.md) |
| 1-4 | 高低差のある足場を登る | [STAGE_1_4.md](STAGE_1_4.md) |
| 1-5 | ダッシュブースターを乗り継ぐ | [STAGE_1_5.md](STAGE_1_5.md) |
| 1-6 | 地面のない砦を大砲で渡る | [STAGE_1_6.md](STAGE_1_6.md) |
| 1-7 | 多数の雑魚敵と戦う | [STAGE_1_7.md](STAGE_1_7.md) |
| 1-8 | World 1ボス戦 | [STAGE_1_8.md](STAGE_1_8.md) |

### World 2

| ステージ | テーマ | 詳細 |
|---|---|---|
| 2-1 | 鳥の攻撃を避けながら細い安全路を進む | [stage_2_1.md](stage_2_1.md) |
| 2-2 | せり上がる溶岩から上へ逃げる | [stage_2_2.md](stage_2_2.md) |
| 2-3 | 迫る溶岩からS字コースを逃げる | [stage_2_3.md](stage_2_3.md) |
| 2-4 | 多数の水平移動床を乗り継ぐ | [stage_2_4.md](stage_2_4.md) |
| 2-5 | ダメージ床の飛び石を渡る | [stage_2_5.md](stage_2_5.md) |
| 2-6 | ダメージ床に囲まれた安全島を渡る | [stage_2_6.md](stage_2_6.md) |
| 2-7 | スターの無敵時間をつないで進む | [stage_2_7.md](stage_2_7.md) |
| 2-8 | World 2ボス戦とクリア後の崩れた回廊 | [stage_2_8.md](stage_2_8.md) |

### World 3

| ステージ | テーマ | 詳細 |
|---|---|---|
| 3-1 | ボタンでライトを点灯する空中回廊 | [STAGE_3_1.md](STAGE_3_1.md) |
| 3-2 | 感圧板で扉を開ける水上回廊 | [STAGE_3_2.md](STAGE_3_2.md) |
| 3-3 | ロープを切って橋を作る | [STAGE_3_3.md](STAGE_3_3.md) |
| 3-4 | ダッシュブースターで夕焼けの峡谷を渡る | [STAGE_3_4.md](STAGE_3_4.md) |
| 3-5 | ワープの接続を覚えて迷宮を進む | [STAGE_3_5.md](STAGE_3_5.md) |
| 3-6 | 螺旋状の陸地を中央へ進む | [STAGE_3_6.md](STAGE_3_6.md) |
| 3-7 | 8エリア・16区画の総力戦 | [STAGE_3_7.md](STAGE_3_7.md) |
| 3-8 | World 3ボス戦 | [STAGE_3_8.md](STAGE_3_8.md) |

### World 4

| ステージ | テーマ | 詳細 |
|---|---|---|
| 4-1 | 押せる箱を足場・重し・遮蔽物に使う | [STAGE_4_1.md](STAGE_4_1.md) |
| 4-2 | 溶岩上の固定足場と移動床を渡る | [STAGE_4_2.md](STAGE_4_2.md) |
| 4-3 | 左右に分断された峡谷を交互に進む | [STAGE_4_3.md](STAGE_4_3.md) |
| 4-4 | 二つの大防壁を異なる方法で突破する | [STAGE_4_4.md](STAGE_4_4.md) |
| 4-5 | 七つの主要空中足場を渡る | [STAGE_4_5.md](STAGE_4_5.md) |
| 4-6 | 木箱と壁で区切られた迷路を進む | [STAGE_4_6.md](STAGE_4_6.md) |
| 4-7 | 溶岩・固定足場・移動床・大砲の総合 | [STAGE_4_7.md](STAGE_4_7.md) |
| 4-8 | 最終ボス戦 | [STAGE_4_8.md](STAGE_4_8.md) |
## 完了条件

* 通常の走行、通常ジャンプ、近接攻撃だけで開始地点から全必須敵を倒し、ゴールへ到達できる。
* 必須敵が隠し場所、落とし穴上、到達困難な場所にいない。
* 開始地点とゴール地点に、出現直後や到達直後の回避不能なダメージ判定が重なっていない。
* 通常ステージの`clearPosition`が地形に埋まらず、通常操作で石段と光の柱へ近づける位置にある。
* 移動床の待機場所と着地点に敵やダメージ床がある場合も、危険を視認でき、通常操作で回避または突破できる。
* 描画用CSVと物理用CSVの座標、回転、倍率が一致している。
* 水面モデルと同名の設定CSVが存在し、描画側の`loadType`が`meshmix2`になっている。水面を物理用CSVへ登録していない。
* 壁、高さが2倍の壁、岩、ダメージ床が、描画用CSVと物理用CSVの両方へ同じCSV IDで登録されている。
* 拠点1と同じ木を使用した場合、描画モデルと幹の衝突判定が同じ配置になっており、QTE用の木と混同していない。
* 草や木をインスタンシングで大量配置した場合、`PlacementCsv`のファイルが存在し、各行が`X,Y,Z,RotY,Scale`の順になっている。描画専用の配置を物理用CSVや`Interactables.csv`へ登録していない。
* 柵外に十分な景観用オブジェクトがあり、World 2では岩だけ、それ以外のワールドでは木、岩、草が使われている。
* 柵は当たり判定を持たないため、進行を止める境界として使っていない。
* 移動床のCSV ID、`RenderID`、`PhysicsID`が一致している。
* `AttackTriggers.csv`の有効な`TargetID`と`PressurePlates.csv`の`WallID`が、描画・物理CSVの両方に存在する。
* **すべての感圧板連動扉について、扉で分断される各側・各閉鎖区画に同じ`WallID`の感圧板があり、箱内からも扉を再開放できる。** 感圧板2は外側＋箱内の2つ以上、感圧板3は両扉の外側＋箱内中央の3つ以上になっている。
* `LavaZones.csv`の`PhysicsID`が対象の物理オブジェクトと一致している。
* 敵、アイテム、破壊可能オブジェクトが地形や落とし穴内に埋まっていない。
* 押せる箱、ダッシュブースター、そのほかの専用CSV配置物が、最大表示倍率と回転後の占有範囲を含めて壁、固定床、地面の側壁へめり込んでおらず、意図して接触させない物同士に0.2m以上の隙間がある。
* 破壊可能木箱の`PosY`が底面高として記述され、最下段は地面・静的足場へ接地し、上段は1m下の木箱へ積まれている。空中に浮いた列や、支持範囲外の列がない。
* QTEオブジェクトの描画、衝突判定、インタラクト位置が一致し、プロンプトを確認して意図的に起動できる。周囲に敵やダメージ床があってもよい。
* ダメージ床と落とし穴がカメラから判別できる。
* 強化能力を使った場合もステージ外や壁の裏へ侵入できない。
* 【通常ステージ】QTE木、ダッシュブースター、移動床、ロープ・レバー2/3・ボタンのいずれか、感圧板2/3・ワープオブジェクト・ダメージ床・押せる箱のいずれか、Y座標3以上の動かない床がそれぞれ1つ以上配置されている。
* 【通常ステージ】敵数がWorld 1は10体以上、World 2は15体以上、World 3・World 4は20体以上配置されている。
* 【通常ステージ】区画数がワールド別の目安を満たしている（World 1: 約3、World 2: 約12、World 3・4: 約18）。不足する設計は要件不達と見なす。
* 【通常ステージ】区画のX座標が中央（-15m〜+15m）に偏っていない。ステージのX範囲（例: World 3/4は-60〜+60）の両端側にも主たる攻略区画が存在する。
* 【通常ステージ】ステージの端にギミックが2つ以上配置されている。ただし開始地点とゴール地点の周囲を除く。
* 【ボスステージ】`EnemyPositions.csv`（ボス1体）と`EnemyPositionsCleared.csv`（雑魚敵）の両方が存在する。ボスなしバージョン（クリア後の再訪）では雑魚敵、破壊可能オブジェクト、落とし穴、QTE木、アイテムを配置し、ダッシュブースター、レバー、ボタン、ロープを配置しない。
* 敵全滅後に敵探しや長い逆走が発生しない。
* 分岐→行き止まりの寄り道が2箇所以上ある（報酬と敵を配置し、メインに合流させない。長くてよく、分岐の先にさらに分岐があってもよい。左右だけではなく上下に分岐しても良い）。
* CSVはBOM付きUTF-8、CRLFで保存する。`.x`と`.fx`はBOMなしUTF-8、CRLFで保存する。
* `Debug|x64`ビルドが成功し、出力先へ`res`がコピーされる。
