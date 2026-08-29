# TALK_IMAGE_GENERATION.md

MMDモデル(宝鐘マリンV2)からゲーム用「立ち絵(腿から上)」画像を生成するための作業メモ。
他のLLMがこのタスクを引き継ぐ場合、このメモで再現できるようにする。

## アセット

- **Blenderファイル**: `C:\Users\bibindon\Nextcloud\RedFortressAsset\marine\blender5.1.2\marine.blend`
- **Blender実行ファイル**: `C:\Program Files\Blender Foundation\Blender 5.1\blender.exe` (Blender 5.1.2)
- 撮影テスト済み、Aポーズの腿から上(正面/背面)をレンダリング可能。

## モデル構成(重要)

- アーマチュア: `宝鐘マリンV2_arm`(bones=549、MMD標準の骨名: センター/上半身/頭/首/肩.R/腕.R など。「上腕」という名前の骨は存在せず、肩→腕の構成)
- メッシュは3つ(すべてこのアーマチュアでスキニング):
  | オブジェクト名 | 頂点数 | hide_render |
  |---|---|
  | `宝鐘マリンV2_mesh` | 78511 | **True(描画されない)** |
  | `宝鐘マリンV2_mesh_before_face_uniform_normals` | 78511 | **True(描画されない)** |
  | `宝鐘マリンV2_mesh_decimate50` | 44980 | **False(これが画面に出る本体)** |
- 形状キー: 2、コンストレイント: 22(脚IK・MMD用TRANSFORM/COPY_TRANSFORMS・目のダミー。腕のDAMPED_TRACKは影響0)
- 体位: 身長1.69m、足元 z=0、**正面は -Y 方向**、キャラから見た右腕は -X 側(正面レンダーで画像左)。
- スケルトンのボーン名は日本語。コンソールで文字化けする(後述)。

## 重大な罠: アニメーション「slash」が割り当て済み

- この.blendにはアクションが3つ入っている: `slash`(5490本のfc)、`slash2`(5491)、`slash2_source`(543)。
- **アーマチュアに `slash` がアクティブアクションとして割り当て済み**。このため、Blenderを開いてポーズリセット(matrix_basis=Identity)しても、毎フレームのdepsgraph評価でアニメーションがポーズを上書きする。
- 初期表示の「右腕を上げて赤い筒を持ったポーズ」は slash アニメの再生結果(攻撃モーション)。bind poseの問題ではない。
- **Aポーズに戻すには**: `arm_obj.animation_data_clear()` でアクションを外してからポーズリセット(さらに必要なら全コンストレイントを mute)。
- ※ 背景CLI実行では変更はメモリ内のみ。Aポーズの状態を保存するなら `bpy.ops.wm.save_as_mainfile` で**別名保存**する(アセット原本を上書きしない)。

## 罠2: 画像をopencodeのチャットに表示できない

- opencodeのTUIは画像をインライン表示できない。アシスタントには添付として渡るがユーザーには表示されない。
- 大きいPNG(1MB超)どころか小さいJPEGでも表示されないので、**一律「保存先のパスを案内して、explorer等で開いてもらう」**方針。

## 罠3: コンソール出力の文字化け

- Blenderの標準出力はShift-JIS等で乱れるため、日本語を含むデータを扱うときは必ず `json.dumps(...)`(ensure_asciiデフォルトTrue → \uXXXX エスケープ)で出力し、PowerShell側で `[regex]::Match($text, '(?s)RESULT_JSON_START\s*\r?\n(.*?)\r?\nRESULT_JSON_END')` を取り出して `ConvertFrom-Json` でパースする。

## 罠4: MCP経由ではタイムアウト

- Blender MCP(ローカル/CLIツール)はモデル読み込みが重くてタイムアウトする。**PowerShellから直接**:

```powershell
& "C:\Program Files\Blender Foundation\Blender 5.1\blender.exe" --background "<blend>" --python "<script>" 2>&1 | Out-File out.txt -Encoding utf8
```

- タイムアウトは長めに(600000ms程度)。
- Blender 5.1のエンジン名は `BLENDER_EEVEE`(NOT `BLENDER_EEVEE_NEXT`)。`show_render`ではなく`hide_render`。

## 撮影設定(テスト済み・ベースライン)

- エンジン: EEVEE。ワールド背景はファイル既定が暗いので `Background` ノードを (0.9,0.9,0.9)×強度1.0 に変更(明るいグレー)。
- カメラ: 既存の `Camera`(50mmレンズ)を流用、`to_track_quat('-Z','Y')` で注視。
  - 腿から上フレーミング: 位置 `(0, ±2.3, 1.25)`、注視点 `(0,0,1.25)` → 座標z約0.70〜1.80が縦範囲(大腿上〜帽子頭頂)。
  - 正面 = -Y側から撮影(y=-2.3)、背面 = +Y側。
- ライト: 既存の `Light`(ポイントライト)を camera側+3m高 に移動し `energy=3.0`。
- 解像度: 1000×1200。
- 透過PNGにしたい場合: `scene.render.film_transparent = True` + `color_mode='RGBA'`。

## 再現スクリプト(コピペで使える)

```python
import bpy, json
from mathutils import Vector, Matrix

scene = bpy.context.scene
arm_obj = None
for obj in bpy.data.objects:
    if obj.type == 'ARMATURE':
        arm_obj = obj
        break

# 1. slashアニメを外してAポーズへ
if arm_obj.animation_data is not None:
    arm_obj.animation_data_clear()
for pb in arm_obj.pose.bones:
    pb.matrix_basis = Matrix.Identity(4)
    for c in pb.constraints:
        c.mute = True          # 脚IKを切る(素直なAポーズになる)
bpy.context.view_layer.update()

# 2. 背景を明るいグレーに
if scene.world is None:
    scene.world = bpy.data.worlds.new("World")
scene.world.use_nodes = True
bg = scene.world.node_tree.nodes.get("Background")
if bg is None:
    bg = scene.world.node_tree.nodes.new("ShaderNodeBackground")
    out = scene.world.node_tree.nodes.get("World Output")
    scene.world.node_tree.links.new(bg.outputs[0], out.inputs[0])
bg.inputs[0].default_value = (0.9, 0.9, 0.9, 1.0)
bg.inputs[1].default_value = 1.0

# 3. カメラ・ライト設定
cam_obj = bpy.data.objects.get("Camera")
light_obj = bpy.data.objects.get("Light")
target = Vector((0.0, 0.0, 1.25))
d = 2.3
scene.render.resolution_x = 1000
scene.render.resolution_y = 1200
scene.render.resolution_percentage = 100

for side in ["front", "back"]:
    y = -d if side == "front" else d
    cam_obj.location = Vector((0.0, y, 1.25))
    cam_obj.rotation_euler = (target - cam_obj.location).to_track_quat('-Z', 'Y').to_euler()
    light_obj.location = cam_obj.location + Vector((1.5, 0.0, 2.0))
    light_obj.rotation_euler = (target - light_obj.location).to_track_quat('-Z', 'Y').to_euler()
    scene.render.filepath = r"C:/Users/bibindon/AppData/Local/Temp/opencode/marine_rnd_%s.png" % side
    bpy.ops.render.render(write_still=True)
```

## 実績

- 2026-08-29: Aポーズ正面/背面の腿から上レンダリングに成功。
  - 出力例: `C:\Users\bibindon\AppData\Local\Temp\opencode\marine_clean_front.png`, `marine_clean_back.png`
  - 正面の見た目は良好。背面は後頭部の髪メッシュがピンクの塊に見える(モデル自体の問題)。
- 由来: 初回レンダーは「slash」アニメの腕上げポーズだった → 上記手順で解消。

## 次の候補アクション

- 保存先アセットフォルダへの透過PNG書き出し(例: `RedFortressAsset\marine\stand.png`)
- `slash`/`slash2` のフレームから「立ち絵用のカッコいいポーズ」の選定
- ゲーム側(Render::DrawImage)へ載せ替え
