# 座標変換の一本化リファクタリング計画（レンダー / 衝突の軸変換非対称の解消）

> **For Hermes:** この計画はユーザー承認後に実行する。実行時は subagent-driven-development スキルでタスク単位に進める。

**Goal:** レンダリングエンジン（MeshMix2）と衝突ライブラリ（PhysicsLib）の間にある `.x` メッシュ読み込み時の軸変換の非対称性を解消し、「正しくてシンプル」な状態にする。

**Architecture:** 現在は「レンダーは常に Blender 軸変換を適用、物理は FTM 文字列パターン一致時のみ適用」という 2 系統。これを「頂点をワールド Y-up にベーク統一し、両エンジンとも素の頂点データ（変換なし）を読む」方式に一本化する。ただし**アニメーション付きメッシュ（ボーン/Frame 階層）は素の D3DXLoadMeshFromX では読めない**ため、適用範囲を明確に分けて進める。

**Tech Stack:** C++ / DirectX 9 / D3DX / Blender 公式 DirectX X エクスポーター

---

## 調査で確定した事実（2026-08-06）

### 1. 座標系の共通認識
- ゲーム世界・レンダー・物理とも **Y-up（Y が高さ）** で一貫。
- `GameApp.cpp:1652-1675` の移動床同期はレンダー位置をそのまま物理へ渡す（変換なし）。
- `PhysicsLib::SetTransform`（PhysicsLib.cpp:2737）も素直な代入のみ。

### 2. 軸変換実装は両エンジンで「同じ」だが「適用条件が非対称」

| | MeshMix2（レンダー） | PhysicsLib（物理） |
|---|---|---|
| 変換関数 | `CorrectBlenderOfficialAxisTransforms` (MeshMix2.cpp:1142) | `CorrectBlenderOfficialAxisTransforms` (PhysicsLib.cpp:1886) — **同一実装の重複** |
| 適用条件 | **常に**適用（カスタムローダー + Frame 変換） | `UsesBlenderOfficialAxisTransform()` が FTM パターン一致時のみ。不一致は素の `D3DXLoadMeshFromX`（FTM 無視・頂点そのまま） |
| 読み込み経路 | `LoadCustomXFrameHierarchyFromText` (CustomXLoader.cpp:2790) | `LoadBlenderOfficialCollisionMesh` (PhysicsLib.cpp:1942) or `D3DXLoadMeshFromX` |

- 物理側の `UsesBlenderOfficialAxisTransform`（PhysicsLib.cpp:1840）は .x 先頭 64KB の**空白除去文字列**を、2 つの FTM 行列パターン（`1,0,0/0,0,-1/0,1,0` と X ミラー `-1,0,0/0,0,1/0,1,0`）と**文字列一致**で判定するハック。

### 3. 全 334 個の `.x` ファイル実走査結果

| 分類 | 個数 | レンダー | 物理 | 整合 |
|---|---|---|---|---|
| Blender 標準 FTM | 91 | 変換適用 | 変換適用 | ✅ |
| X ミラー FTM | 4 | 変換適用 | 変換適用 | ⚠️ 実質 X 対称で無害（既知） |
| **非標準 FTM** | **232** | 変換適用 | **素の D3DX（FTM 無視）** | ⚠️ 頂点が既に Y-up ベーク済みであることが暗黙前提 |
| FTM なし | 7 | 変換適用 | 素の D3DX | ⚠️ 同上 |
| 計 | 334 | | | 233 個（約 7 割）が「レンダーは変換、物理は素読み」の**暗黙依存** |

### 4. 最重要の制約（B の実現可能性に直結）
- レンダラー（CustomXLoader.cpp:1067-1071）は**頂点をそのまま読み、軸変換は Frame 行列（+アニメーションキー）に対してのみ**行う。
- `D3DXLoadMeshFromX` は**アニメーション非対応**（ボーン/スキン/Frame 階層アニメを読まない）。
- よって「**すべてのメッシュを素の D3DX 直続に一本化**」は**アニメーション付きモデルでは不可能**。
- 素の D3DX 直続にできるのは「静止メッシュ（衝突用・静的な描画メッシュ）」のみ。

---

## 提案アプローチ（B を現実適用範囲に絞ったもの）

### 方針
「頂点をワールド Y-up にベーク統一」を**Blender 側（アセット生成）で行い**、その結果:
1. **衝突メッシュ**（`XFileListPhysics.csv` 系）は素の `D3DXLoadMeshFromX` 直続に一本化 → 軸変換コード・パターン文字列ハックを物理から撤去。
2. **描画メッシュ**（MeshMix2）はカスタムローダー維持（アニメ対応のため）だが、頂点が既に Y-up なら **FTM に補正を掛ける必要がなくなる** → `CorrectBlenderOfficialAxisTransforms` の適用を「必要時のみ」または撤去できる。
3. 重複する `CorrectBlenderOfficialAxisTransforms` は共通化 or 撤去。

### 適用範囲の 3 分類
- **分類 A（静止・素直に一本化できる）**: 非標準 FTM 232 + FTM なし 7 = 239 個。すでに頂点が Y-up ベーク済みの前提。**検証が必要**（後述フェーズ 1）。
- **分類 B（Blender 標準 FTM 91 個）**: 頂点が Z-up で書かれている可能性が高い。Blender 側で頂点を Y-up にベーク（既存 `tools/PrepareEnemyModels.py` の `apply_mesh_world_transform` 設定を流用）し、FTM を恒等に。
- **分類 C（アニメーション付き）**: wolf・player 等。素の D3DX 直続は**不可**。Frame 階層 + アニメーションキーの軸補正はレンダー側に残す（現状維持 or 変換を「頂点ベーク前提の簡略形」に置換）。

### 重要な注意（AGENTS.md との整合）
- AGENTS.md: 「公式 Blender エクスポーターの出力をゲーム側ローダー/レンダラーで吸収せよ。カスタムエクスポーター・変換・後処理書き換えは作るな」。
- よって**ゲーム側の .x ファイルを後処理で書き換えるのは NG**。
- 正しい進め方: **Blender 側のエクスポート設定（axis_forward/axis_up）と、モデル側のベーク（transform_apply）で頂点を Y-up にしておく**。これは既存の `PrepareEnemyModels.py` が既にやっている方向（`apply_mesh_world_transform`）。
- ただし `stage_ground.x` 等の**再生成は別 AI が作業中**。今回の計画では**アセット再生成はフェーズ 2 以降・ユーザー確認後に**。

---

## フェーズ分割

### フェーズ 0: 現状記録（作業なし・確認のみ）
- 現行の全 `.x` FTM パターンと頂点軸のスナップショットを `tools/` に記録（スクリプトは新規作成可、実行のみ）。
- 目的: リファクタ前後の回帰比較の基準を作る。
- 成果物: `tools/audit_x_axis_snapshot.py`（リポジトリに残す）。

### フェーズ 1: 分類 A（239 個）の頂点軸を実データ検証
- 非標準 FTM / FTM なしファイルの頂点 XYZ 範囲を実走査し、「既に Y-up（Y が高さ、Z が奥行き）」を確認。
- もし Z-up のままのファイルが見つかったら、それを分類 B に移す。
- 成果物: 分類リスト（A/B/C）確定。`scripts/audit_collision_x_files.py` を拡張 or 新規 `tools/verify_x_vertex_axis.py`。

### フェーズ 2: 物理側の一本化（ユーザー確認後・アセット再生成の前に可能な分のみ）
- `PhysicsLib::LoadMesh`（PhysicsLib.cpp:2074）の分岐を整理:
  - 分類 A のファイル → `D3DXLoadMeshFromX` 直続（現状と同じ結果）。
  - 分類 B/C のファイル → 変換経路維持 or 段階移行。
- `UsesBlenderOfficialAxisTransform` の文字列ハックを「ファイル分類の明示リスト（CSV or 定数）」に置換する案を設計。**即座のコード変更はしない**（計画段階）。

### フェーズ 3: Blender 側ベーク（別 AI の作業完了後・ユーザー確認後）
- 分類 B の 91 個: `PrepareEnemyModels.py` / 各 `_build_*.py` の設定で頂点をワールド Y-up にベーク。
- 分類 C（アニメ）: アニメーションキーと Frame の軸補正が正しく共存する形を維持（現状のレンダー側処理を「頂点ベーク前提」に単純化できるかを検証）。
- **再生成差分は `scripts/compare_x_frames.py` でフレーム単位検証**（redfortress-dev スキル参照）。

### フェーズ 4: レンダー側の簡略化
- 頂点が Y-up ベーク済みになれば、MeshMix2 の `CorrectBlenderOfficialAxisTransforms` 適用が「恒等 or 不要」になるかを確認し、必要なら撤去/条件化。
- アニメーション付きは現状維持（安全側）。

### フェーズ 5: 重複排除
- `CorrectBlenderOfficialAxisTransforms`（MeshMix2.cpp:1142 / PhysicsLib.cpp:1886）の同一実装を共通ヘッダ or 共通ユーティリティに抽出（**物理→レンダーが同一の変換を共有**）。

### フェーズ 6: 検証・回帰
- MSBuild Debug/x64 で 0 エラー。
- `scripts/audit_collision_x_files.py`・`audit_visual_x_files.py` で面数・FVF 回帰なし。
- 衝突位置が描画位置と一致することの静的確認（CSV 座標 vs メッシュ AABB）。
- ランタイム挙動は手動プレイ確認（自動化不可、redfortress-dev スキルに従う）。

---

## 変更対象ファイル（実行時）

| ファイル | 変更内容 |
|---|---|
| `PhysicsLib/PhysicsLib/PhysicsLib.cpp` (:1840, :1886, :2074) | `UsesBlenderOfficialAxisTransform` 撤去/置換、`CorrectBlenderOfficialAxisTransforms` 共通化、`LoadMesh` 分岐整理 |
| `RedFortressRender/Render/MeshMix2.cpp` (:1142) | `CorrectBlenderOfficialAxisTransforms` 共通化・条件化 |
| `RedFortressRender/Render/MeshMix2.h` (:127) | 共通化に伴う宣言変更 |
| `tools/PrepareEnemyModels.py` ほか `_build_*.py` | 頂点 Y-up ベーク設定（分類 B） |
| `scripts/audit_collision_x_files.py` ほか検証スクリプト | 頂点軸検証の追加 |
| 新規 `tools/audit_x_axis_snapshot.py` | リファクタ前後比較スナップショット |

## テスト / 検証
- ビルド: MSBuild Debug/x64 0 エラー 0 警告。
- 静的検証: 頂点軸（Y が高さ）の全数チェック、CSV 配置と AABB の整合。
- 手動: 全ステージの描画/衝突一致、移動床・敵・プレイヤー挙動。

## リスク・トレードオフ・未解決点
1. **アニメーション付きモデルは素の D3DX 直続不可** → 分類 C はレンダー側の Frame/アニメ経路を維持する必要があり、「完全一本化」は達成不能。妥協点として「静止メッシュのみ一本化 + アニメは変換を明示化」。
2. **非標準 232 個が本当に Y-up かは未実測**（フェーズ 1 で確定）。もし Z-up が混在していたら、そのファイルは現在も「物理がずれている」可能性があり、修正対象が増える。
3. **アセット再生成は別 AI 作業中** → フェーズ 2/3 の実行タイミングはユーザーの合図を待つ。
4. AGENTS.md の「カスタム変換禁止」→ Blender 側ベークは許容（公式エクスポーター + transform_apply）だが、.x 後処理は不可。計画はそれに従う。
5. 物理の `LoadMesh` は衝突メッシュ（低ポリ）が主。`D3DXLoadMeshFromX` 直続にすると FVF 統一（CloneMeshFVF）経路が失われる → 混在 FVF クラッシュの再発リスク（gotcha #7）。**必ず FVF 統一処理を残す**。
6. `OutPassThroughIds`/`outSolidIds` 等の既知の死んだ出力は今回のスコープ外（別リファクタ候補）。

## オープンクエスチョン（ユーザーに確認）
- Q1: フェーズ 1 の検証結果待ちで「B をどこまで適用するか」（静止のみ / アニメも変換簡略化）を確定したい。先にフェーズ 0/1（読み取り専用・スクリプト新規のみ）まで進めてよいか?
- Q2: 別 AI の作業完了の合図を待つ形でよいか（現在は計画のみで停止）?
- Q3: 物理の文字列ハック撤去は「分類リスト（CSV）方式」でよいか、それとも「常に素の D3DX + FVF 統一」方式がよいか?

---

## 実行開始条件
- ユーザーが「実行してよい」と言う
- 別 AI のアセット作業が完了している（または競合しないファイルのみ）
- 本計画の承認
