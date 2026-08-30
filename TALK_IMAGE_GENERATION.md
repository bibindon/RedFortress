# 会話パート用立ち絵の生成手順

この文書は、会話パートで使用するキャラクター立ち絵を同じ品質・構図で再生成するための記録です。

## 基本方針

- Blenderは `C:\Program Files\Blender Foundation\Blender 5.1\blender.exe` を使用する。
- 本番出力は1024×1024、PNG RGBA、背景透過、Cyclesで行う。
- キャラクターは画面左側へ置ける余白を確保し、帽子や頭を上端からはみ出させない。
- 下端はおおむね膝付近で切れる大きさにする。
- 腕は横に伸ばさず、自然に下ろす。
- 表情差分間ではカメラ、照明、ポーズを変更しない。表情モーフだけを切り替える。
- ゲーム側の基準解像度は1600×900であり、立ち絵の配置と拡大縮小はスライドショー側で行う。画像側でゲーム画面の解像度を補正しない。

## 出力先

```text
C:\Users\bibindon\source\repos\bibindon\RedFortress\RedFortress2\MultiPassRendering\res\2D_Image
```

既存画像を上書きする前に、必要に応じて `tmp` 以下へバックアップする。

## 宝鐘マリン

### 入力モデル

```text
C:\Users\bibindon\Nextcloud\RedFortressAsset\marine\blender5.1.2\marine.blend
```

### 生成スクリプト

```text
C:\Users\bibindon\source\repos\bibindon\RedFortress\tools\RenderMarineColorCorrected.py
```

このスクリプトには、次の確定済み調整が含まれている。

- 肌テクスチャへ暖色補正を適用する。
- 瞳の彩度と明度を抑える。
- 保存されている `slash` アニメーションを解除し、腕を下ろす。
- 表情モーフを持つ高品質メッシュ `宝鐘マリンV2_mesh` を表示し、軽量メッシュを非表示にする。
- Standard、Medium Low Contrast、露出 +0.10を使用する。
- キー190、フィル100、リム115、ワールド強度0.20の照明を使用する。
- 正投影カメラのスケールは1.18、シフトはX=0.16、Y=0.07とする。

### モデル固有の注意点

- 元の `.blend` には `slash`、`slash2`、`slash2_source` のアクションがあり、`slash` がアーマチュアへ割り当てられている。単にポーズをリセットしただけでは、アニメーション評価によって腕上げポーズへ戻る。
- そのため、必ず `animation_data_clear()` の後で全ポーズボーンをリセットする。生成スクリプトでは実施済み。
- 表情モーフがあるのは高品質メッシュ `宝鐘マリンV2_mesh`。`宝鐘マリンV2_mesh_decimate50` には表情モーフがないため、表情差分の生成には使用しない。
- 原本の `marine.blend` は上書きしない。調整済みBlenderファイルを保存する場合は必ず別名にする。

### 本番生成コマンド

リポジトリ直下でPowerShellから実行する。

```powershell
$blender = 'C:\Program Files\Blender Foundation\Blender 5.1\blender.exe'
$model = 'C:\Users\bibindon\Nextcloud\RedFortressAsset\marine\blender5.1.2\marine.blend'
$script = (Resolve-Path 'tools\RenderMarineColorCorrected.py').Path
$output = (Resolve-Path 'RedFortress2\MultiPassRendering\res\2D_Image').Path
& $blender --background $model --python $script -- --output-dir $output --resolution 1024 --samples 48 --engine CYCLES --render-game-expressions
```

生成される表情は通常、笑顔、心配、決意、真剣、驚きの6種類。

## 戌神ころね

### 入力モデル

```text
C:\Users\bibindon\Nextcloud\RedFortressAsset\Korone\InugamiKorone\InugamiKorone\Korone.blend
```

### 生成スクリプト

```text
C:\Users\bibindon\source\repos\bibindon\RedFortress\tools\RenderKoroneColorCorrected.py
```

このスクリプトには、次の確定済み調整が含まれている。

- 顔、線なしの顔、肌へ暖色補正を適用する。
- 赤すぎた瞳の彩度と明度を抑える。
- 両腕と肘を調整して腕を下ろす。
- Standard、Medium Low Contrast、露出 -0.25を使用する。
- キー180、フィル75、リム100、ワールド強度0.18の照明を使用する。
- 正投影カメラのスケールは1.35、Xシフトは0.18とする。

### 本番生成コマンド

```powershell
$blender = 'C:\Program Files\Blender Foundation\Blender 5.1\blender.exe'
$model = 'C:\Users\bibindon\Nextcloud\RedFortressAsset\Korone\InugamiKorone\InugamiKorone\Korone.blend'
$script = (Resolve-Path 'tools\RenderKoroneColorCorrected.py').Path
$output = (Resolve-Path 'RedFortress2\MultiPassRendering\res\2D_Image').Path
& $blender --background $model --python $script -- --output-dir $output --resolution 1024 --samples 48 --engine CYCLES --render-game-expressions
```

生成される表情は通常、笑顔、謎めいた表情の3種類。

## 天音かなた

### 入力モデル

立ち絵に使用したのは、ボス用の `kanata_boss.blend` ではなく、VRMを直接インポートした次のファイル。

```text
C:\Users\bibindon\Nextcloud\RedFortressAsset\KanataPrototype\kanata_imported.blend
```

### 生成スクリプト

```text
C:\Users\bibindon\source\repos\bibindon\RedFortress\tools\render_kanata_portrait_test.py
```

このスクリプトは次の処理を行う。

- `Face` オブジェクトのVRM表情シェイプキーを使用する。
- 元のダウンロード先を参照している外部テクスチャが消えていても、同じフォルダの `textures\kanata_00.png` から `kanata_28.png` をマテリアル順に再接続する。
- 肌マテリアルをCycles向けに変換し、暖色補正を加える。
- マテリアルの自己発光を無効化し、実ライトによる陰影を出す。
- 存在しない法線マップの接続を解除し、顔や衣装に異常な模様が出るのを防ぐ。
- 腕を下ろす。
- `KanataRigidMesh` が存在する場合は非表示にする。
- Standard、Medium Low Contrast、露出 -0.25を使用し、白い肌が灰色になるのを防ぐ。
- 顔の肌へ `(1.0, 0.88, 0.84)`、身体の肌を含むマテリアルへ `(1.0, 0.86, 0.82)` の暖色乗算を適用する。
- キー180、フィル75、リム100、ワールド強度0.18を使用する。
- 正投影カメラのスケールは1.10、Yシフトは0.07とする。

### 本番生成コマンド

```powershell
$blender = 'C:\Program Files\Blender Foundation\Blender 5.1\blender.exe'
$model = 'C:\Users\bibindon\Nextcloud\RedFortressAsset\KanataPrototype\kanata_imported.blend'
$script = (Resolve-Path 'tools\render_kanata_portrait_test.py').Path
$output = (Resolve-Path 'RedFortress2\MultiPassRendering\res\2D_Image').Path
$env:KANATA_TEST_OUTPUT = Join-Path $output 'novel_chr_kanata_{expression}_transparent.png'
$env:KANATA_CYCLES_SAMPLES = '48'
$env:KANATA_EXPRESSION = 'all'
& $blender --background $model --python $script
Remove-Item Env:KANATA_TEST_OUTPUT
Remove-Item Env:KANATA_CYCLES_SAMPLES
Remove-Item Env:KANATA_EXPRESSION
```

生成される表情は通常、笑顔、心配、決意の4種類。

## 本番前の高速確認

マリンところねは、Cycles本番の前に次のように512×512、Eevee、8サンプルで全表情を確認できる。出力先にはゲーム用フォルダではなく `tmp` を指定する。

```powershell
& $blender --background $model --python $script -- --output-dir $previewOutput --resolution 512 --samples 8 --engine BLENDER_EEVEE --render-game-expressions
```

Blender 5.1でEeveeを指定するときの名前は `BLENDER_EEVEE`。`BLENDER_EEVEE_NEXT` は使用できない。重いモデルは連携機能経由だとタイムアウトする場合があるため、上記のようにPowerShellからBlenderを直接起動する。

確認する項目は次の通り。

1. 背景が透明である。
2. 画像が1024×1024である。
3. 肌が灰色または青色になっていない。
4. 瞳だけが不自然に高彩度になっていない。
5. 帽子、頭、髪が上端から切れていない。
6. 腕が横へ伸びていない。
7. 表情ごとの位置と大きさが完全に揃っている。
8. 表情の違いがゲーム画面上の大きさでも判別できる。

## ゲームへの反映

立ち絵を更新した後は、Debug x64でビルドする。ビルド後処理により `res` が実行用フォルダへコピーされる。

```powershell
$cmdLine = 'set RF_BUILD_PATH=%Path%& set PATH=& set Path=!RF_BUILD_PATH!& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" RedFortress2\MultiPassRendering.sln /p:Configuration=Debug /p:Platform=x64 /m:1 /v:minimal'
& cmd.exe /d /v:on /c $cmdLine
```

実行用コピー先は次の通り。

```text
C:\Users\bibindon\source\repos\bibindon\RedFortress\RedFortress2\x64\Debug\res\2D_Image
```

ソース側と実行用フォルダ側で、更新したPNGの内容が一致していることも確認する。
