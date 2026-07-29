# HOSHIGIRL_PLAN.md — ステージ3-8 ボス「ホシガール」実装計画

> **このファイルの目的**: トークン使用制限で作業が中断されても、次のセッションが
> このファイルを読めば即座に作業を再開できるようにするための、単一の情報源
> （Single Source of Truth）です。作業を進めるたびに **「## 進捗状況」** を更新してください。

---

## 📋 全体サマリー（何を作っているか）

ステージ3-8（`stage28`）に配置するボス敵 **ホシガール（Hoshigirl）** を実装する。
黒い布ゴースト型のデザイン。4種類の攻撃パターンを持つ本格的なボス。

**現状**: モデル・配置・基本ボスクラス・4攻撃AIすべて実装完了。ビルド成功済み。残作業は実機テスト（挙動・バランス確認）のみ。

---

## ✅ 進捗状況（作業ごとにここを更新する）

### フェーズ1: モデル・配置・基本クラス（100% 完了）
- [x] `Hoshigirl.blend` + `hoshigirl.png` 作成済み（ユーザー作成）
- [x] Xファイル生成（`PrepareEnemyModels.py` asset=hoshigirl を実行）
  - 生成物: `enemy.x`, `enemy.idle.x`, `enemy.move.x`, `enemy.fast_move.x`,
    `enemy.attack.x`, `enemy.hit.x`, `enemy.death.x`, `enemy.default.x`, `enemy.csv`
  - 場所: `RedFortress2/MultiPassRendering/res/model2/Hoshigirl/`
- [x] `EnemyHoshigirl.h` / `.cpp` 新規作成（基本ボスクラス）
- [x] `EnemyManager.cpp` に `#include "EnemyHoshigirl.h"` とファクトリ登録追加
- [x] `simple-directx9.vcxproj` / `.filters` に `EnemyHoshigirl.cpp/.h` 追加
- [x] `stage28/EnemyPositions.csv`（3-8ステージ）を `hoshigirl,0.0,0.2,0.0,180` に編集
- [x] ビルド成功（MSBuild Debug x64）
- [x] 地面埋まりバグ修正: `GetMeshVerticalOffset() { return 0.25f; }` を追加
  - 原因: モデル足元が原点より-0.1m下に作られていた。scale(2.5)×0.1=0.25m押し上げ

### フェーズ2: 4攻撃パターン実装（100% 完了）
- [x] **STEP 1**: `EnemyHoshigirl.h` を4攻撃パターン対応に書き換え
- [x] **STEP 2**: `EnemyHoshigirl.cpp` に4攻撃パターンを実装
  - [x] ②STEP 2a: ファイル骨格（インクルード、無名空間、定数、列挙型）
  - [x] ②STEP 2b: コンストラクタと公開メソッド（UsesSpecialAttacks, UpdateSpecialAttack）
  - [x] ②STEP 2c: 攻撃選択ロジック（SelectAttack, IsAttackAllowed, BeginAttack）
  - [x] ②STEP 2d: フェーズ遷移（BeginActivePhase, BeginRecovery, EndAttack）
  - [x] ②STEP 2e: UpdateActivePhase（各攻撃の発動フレーム処理）
  - [x] ②STEP 2f: 持続エフェクト更新（UpdateSoulBoltProjectile, UpdateCurseMire）
- [x] **STEP 3**: ビルド検証（MSBuild Debug x64）— 2026-07-29 成功

> **中断時の状況メモ（2026-07-29 解消済み）**:
> 前回セッションは STEP 2 実装完了直後にトークン制限で中断されたため、
> このファイルのチェックボックス更新だけが残っていた。
> コードは EnemySkeleton/EnemySpider 準拠で全4攻撃とも実装済み。
> ビルド時に `vc145.pdb` の C2471 エラーが出たが、原因は実行中の
> `simple-directx9.exe` と stale な `mspdbsrv.exe` の残存プロセス。
> 両方終了させて PDB を削除したところ正常にビルド成功した。

### フェーズ3: 死亡アニメーション追加（100% 完了 — 2026-07-29）
- [x] `Hoshigirl.blend` に `death` アクションをPython（blender --background）で作成
  - 内容: f1 直立 → f7 後退り（リコイル）→ f18 前方くずおれ → f24/30 崩れきって保持
  - 全9ボーンに回転キー、Root に位置キー（チャンネル欠落なし）
  - デザイン: 布ゴーストが「しぼんで前方に崩れる」死亡表現。顔が地面を向く
- [x] `PrepareEnemyModels.py` の hoshigirl 設定: `"death": "idle"` → `"death": "death"`
- [x] 再エクスポート実行（`blender --background --python tools/PrepareEnemyModels.py -- hoshigirl ...`）
  - `enemy.death.x` が `AnimationSet death`（キー時刻 1,7,18,24,30）を含むことを検証済み
  - idle/attack/hit 等は従来どおり idle アクションのまま（ユーザー指定の制約は維持）
- [x] 出力先 `RedFortress2/x64/Debug/res/model2/Hoshigirl/` に手動コピー済み
  - ※ C++ コード変更なしのためビルド不要。次回ビルド時の xcopy でも同期される
- [ ] **残**: 実機で死亡演出を確認（45フレーム後に消去される点に注意）

---

## 🔧 確定仕様

| 項目 | 値 | 理由 |
|---|---|---|
| ボス名 | ホシガール | ユーザー指定 |
| 動作モード | `MovementMode::Ground`（着地・歩行追尾・重力あり） | ユーザー指定 |
| HP | 120 | ユーザー指定 |
| アーマー | `HitReactionMode::SuperArmor`（ノックバック無効） | ユーザー指定 |
| スケール | 2.5f（モデル1.6m × 2.5 ≒ 4m） | — |
| メッシュ垂直オフセット | 0.25f（地面埋まり補正） | モデル足元が原点-0.1m |
| アニメーション | idle（待機・移動・攻撃・被弾）+ death（死亡のみ専用） | ユーザー指定＋フェーズ3でdeath追加 |

---

## ⚔️ 4つの攻撃パターン（実装仕様）

### 共通アーキテクチャ（EnemySkeleton / EnemySpider 準拠）
- 4フェーズ状態機械: `Windup`（予備動作）→ `Active`（発動）→ `Recovery`（硬直）→ クールダウン
- 攻撃選択: ラウンドロビン（`m_nextAttackIndex`）+ 距離ゲート（`IsAttackAllowed`）
- 全パターン idle アニメ再生（アニメ差別化不可なので予備動作フレーム長＋パーティクルで判別）
- ベースクラスのヘルパーを再利用:
  - `IsSpecialAttackReady()` → Chase状態時のみ攻撃可
  - `FaceSpecialAttackTarget()` → 予備動作でプレイヤー方向へ旋回
  - `MoveForSpecialAttack()` → 突進の本体移動
  - `MoveSpecialProjectile()` → 飛翔体の移動＋壁判定
  - `PlaySpecialAttackAnimation()` → アニメ再生
  - `FinishSpecialAttack()` → 攻撃終了後に通常アニメへ復帰
  - `EmitAttackHit(damage, 座標, ノックバック, スロー)` → プレイヤーへダメージ通知

### ① 霊弾連射（Soul Bolt）— 遠距離・飛翔体
テンプレート: Skeleton BoneShot / Spider WebShot
- 十字の目から黒い霊弾を発射。高速・大ダメージ・強ノックバック。
- `MoveSpecialProjectile` で壁判定、`PlaceParticleEffect(Damage)` を6fおきに出して軌跡。

| 項目 | 値 |
|---|---|
| Windup | 24f |
| Active | 1f |
| Recovery | 18f |
| ダメージ | 12 |
| ノックバック | 30f |
| 範囲ゲート | 2.5〜9.0m |
| 弾速 | 6.0 |
| 弾寿命 | 90f |
| 弾判定半径 | 0.3 |
| 命中判定 | 水平距離≤0.7, 垂直差≤1.5 |

### ② 幽体突進（Wraith Charge）— 中距離・突進
テンプレート: Skeleton Charge / Spider Pounce
- 予備動作後に方向ロック、直線突進。当たると大きく吹き飛ぶ。

| 項目 | 値 |
|---|---|
| Windup | 20f |
| Active | 30f |
| Recovery | 18f |
| ダメージ | 15 |
| ノックバック | 45f |
| 範囲ゲート | 1.5〜6.0m |
| 突進速度 | 6.0 |
| 命中判定 | 水平距離≤1.1 |

### ③ 呪いの沼（Curse Mire）— 設置・持続AoE
テンプレート: Spider PoisonPool（唯一の持続ダメージ源）
- プレイヤー足元に黒い霧の沼を設置。長時間残留し継続ダメージ＋スロー。

| 項目 | 値 |
|---|---|
| Windup | 30f |
| Active | 1f |
| Recovery | 18f |
| ダメージ | 5/回 |
| スロー | 60f |
| 範囲ゲート | ≤5.0m |
| 沼寿命 | 180f |
| 再ダメ間隔 | 45f |
| 沼判定半径 | 1.8 |

### ④ 引き裂き（Soul Reap）— 近接・前方円錐
テンプレート: Skeleton Slash（前方円錐判定）
- 爪のある両手で前方を薙ぎ払う。張り付かれた時の拒否技。
- `Dot(forward, toPlayer) >= 0.35` の前方円錐判定。

| 項目 | 値 |
|---|---|
| Windup | 14f |
| Active | 4f |
| Recovery | 18f |
| ダメージ | 14 |
| ノックバック | 28f |
| 範囲ゲート | ≤2.2m |
| 命中判定 | 水平距離≤2.0, 前方円錐Dot≥0.35 |

### 攻撃選択ルール（距離で自動切替）
- 遠距離 → ①霊弾連射
- 中距離 → ②幽体突進 / ③呪いの沼（交替）
- 近距離 → ④引き裂き + ②突進
- ラウンドロビンカーソル `m_nextAttackIndex` は実際に攻撃開始した時だけ進む

---

## 📝 実装に必要なメンバ変数（EnemyHoshigirl.h 用）

EnemySpider.h / EnemySkeleton.h 準拠の構造:

```cpp
// 攻撃種別
enum class AttackType { None, SoulBolt, WraithCharge, CurseMire, SoulReap };

// 攻撃フェーズ
enum class AttackPhase { None, Windup, Active, Recovery };

// 共通攻撃状態
AttackType m_attackType = AttackType::None;
AttackPhase m_attackPhase = AttackPhase::None;
int m_phaseFrames = 0;                 // 現在フェーズの残りフレーム
int m_attackCooldownFrames = 0;        // 攻撃間のクールダウン
int m_nextAttackIndex = 0;             // ラウンドロビン用カーソル (0-3)
bool m_attackHitApplied = false;       // 1攻撃1ヒット保証用
D3DXVECTOR3 m_lockedDirection = D3DXVECTOR3(0.0f, 0.0f, -1.0f);  // 突進方向ロック

// ①霊弾連射（SoulBolt）用
bool m_soulBoltActive = false;
D3DXVECTOR3 m_soulBoltPosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
D3DXVECTOR3 m_soulBoltDirection = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
int m_soulBoltFrames = 0;

// ③呪いの沼（CurseMire）用
bool m_curseMireActive = false;
D3DXVECTOR3 m_curseMirePosition = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
int m_curseMireFrames = 0;
int m_curseMireDamageCooldownFrames = 0;
```

---

## 📂 関連ファイル・場所

### 実装対象ファイル
- **`RedFortress2/MultiPassRendering/EnemyHoshigirl.h`** — 4攻撃対応に書き換え
- **`RedFortress2/MultiPassRendering/EnemyHoshigirl.cpp`** — 4攻撃AI実装
- **`RedFortress2/MultiPassRendering/EnemyBase.h`** — 基底クラス（攻撃ヘルパー参照）
- **`RedFortress2/MultiPassRendering/EnemyBase.cpp`** — 基底クラス実装

### テンプレート参照ファイル（実装時に読むこと）
- **`EnemySkeleton.h` / `.cpp`** — 4フェーズ状態機械、BoneShot(飛翔体)、Slash(前方円錐)、Charge(突進)
- **`EnemySpider.h` / `.cpp`** — PoisonPool(持続AoE)の実装がここにしかない。必ず読む
- **`EnemyKanata.cpp`** — ボスの最小構成テンプレート

### コード規約（AGENTS.md より）
- `.cpp`/`.h` は **BOM付きUTF-8 / CRLF**
- 三項演算子禁止（`? :`）。if/else を使う
- PascalCase（クラス・メソッド）、`m_`（メンバ）、`k`（ファイルスコープ定数）
- 単位: 1 unit = 1 m、Y up、プレイヤー高さ1.7

### ビルドコマンド（Git Bash用、スイッチは `-` 形式）
```bash
MSYS_NO_PATHCONV=1 "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" \
  "RedFortress2/MultiPassRendering.sln" -p:Configuration=Debug -p:Platform=x64 -m -v:minimal
```
注意: Git Bash は `/p:` をパスと誤認するので `-p:` 形式 + `MSYS_NO_PATHCONV=1` が必須。

---

## 🔄 中断からの再開手順（次のセッション向け）

### 前提チェック（必ず最初に実行）
```bash
# 1. 現在の進捗（上記「進捗状況」）を確認
cat HOSHIGIRL_PLAN.md | grep -A 20 "進捗状況"

# 2. EnemyHoshigirl.cpp の現在の行数と内容を確認（実装段階を把握）
wc -l RedFortress2/MultiPassRendering/EnemyHoshigirl.cpp
```

### 作業再開のフロー
1. 上記「## ⚔️ 4つの攻撃パターン」を仕様として読む
2. 上記「## 📝 実装に必要なメンバ変数」を `.h` に反映済みか確認
3. `EnemySpider.cpp` と `EnemySkeleton.cpp` をテンプレートとして読む
4. 「進捗状況」の未完了 `[ ]` のうち、最も若い番号の STEP から再開
5. 各 STEP 完了ごとに「進捗状況」の `[ ]` を `[x]` に更新
6. 全 STEP 完了後、ビルドを実行

### 重要な注意
- **`UsesSpecialAttacks()` が true の敵は接触ダメージゼロになる**（GameApp.cpp:1606 のゲート）。
  全ダメージは明示的に `EmitAttackHit` を呼ぶ必要がある。
- **`EmitAttackHit` は1回の攻撃で1ヒットのみ**（`m_hasPendingAttackHit` ガード）。
  持続AoE(③)は別途クールダウン管理で複数回ヒットさせる。
- **全攻撃で idle アニメ**。アニメによる攻撃の差別化は不可。
  予備動作フレーム長 + パーティクル種類（Damage/Explosion）で視覚的に判別させる。
- **`PlaceParticleEffect` は最大8個同時**（MAX_EFFECT_INSTANCES=8）。超過すると古いものが消える。
  ②の突進軌跡などで連発しないよう間引く。

---

## 📐 実装スケルトン（EnemyHoshigirl.cpp の構造）

実装時のメソッド構成。EnemySpider.cpp と同じ構造にする:

```
namespace {
    // 無名空間のヘルパー（Spider/Skeleton からコピー）
    float HorizontalDistance(a, b);
    D3DXVECTOR3 HorizontalDirection(from, to);
    const int kAttackCooldownFrames = 42;
    // 各攻撃のWindup/Active/Recoveryフレーム定数
}

// コンストラクタ（既存のまま、SuperArmor/Ground/HP120）
// UsesSpecialAttacks() → true をオーバーライド
// UpdateSpecialAttack() の実装:
//   1. 持続エフェクト更新（UpdateSoulBoltProjectile, UpdateCurseMire）
//   2. m_attackCooldownFrames デクリメント
//   3. m_attackPhase==None で攻撃開始判定
//   4. Windup/Active/Recovery のフェーズ遷移
//   5. return m_attackPhase != None

// private メソッド群:
//   SelectAttack(playerPos, playerInvincible)
//   IsAttackAllowed(type, distance)
//   BeginAttack(render, type, playerPos)
//   BeginActivePhase()
//   UpdateActivePhase(render, playerPos, playerInvincible)
//   BeginRecovery()
//   EndAttack()
//   UpdateSoulBoltProjectile()      // ①の飛翔体
//   UpdateCurseMire()               // ③の持続AoE
```

---

## 📚 実装の詳細手順（STEP 2 のサブステップ）

### STEP 2a: ファイル骨格
- `#include "EnemyHoshigirl.h"` と Render.h のインクルード
- 無名 namespace に `HorizontalDistance`, `HorizontalDirection` ヘルパー
- 無名 namespace に定数群（kAttackCooldownFrames + 各攻撃のフレーム長）

### STEP 2b: コンストラクタと公開メソッド
- コンストラクタは既存のまま（HP120, Ground, SuperArmor）
- `UsesSpecialAttacks()` をオーバーライドして `true` を返す
- `UpdateSpecialAttack()` の骨格（5ステップのドライバ）

### STEP 2c: 攻撃選択ロジック
- `SelectAttack`: ラウンドロビン（offset 0-3 ループ）+ IsAttackAllowed でゲート
- `IsAttackAllowed`: 距離ゲート（遠/中/近）
- `BeginAttack`: type/phase 設定、FaceSpecialAttackTarget、方向ロック、
  PlaySpecialAttackAnimation（全idle）、Windupフレーム設定

### STEP 2d: フェーズ遷移
- `BeginActivePhase`: Activeフレーム設定 + 飛翔体/AoEスポーン
- `BeginRecovery`: Recoveryフレーム設定
- `EndAttack`: type/phase を None、cooldown 設定、FinishSpecialAttack 呼び出し

### STEP 2e: UpdateActivePhase
- 各攻撃の発動フレーム処理:
  - SoulBolt: 発生フレームで EmitAttackHit（飛翔体は別途 UpdateSoulBoltProjectile）
  - WraithCharge: MoveForSpecialAttack で突進 + 接触判定 EmitAttackHit
  - CurseMire: 設置（持続は別途 UpdateCurseMire）
  - SoulReap: 前方円錐判定（Dot≥0.35）+ EmitAttackHit

### STEP 2f: 持続エフェクト更新
- `UpdateSoulBoltProjectile`: MoveSpecialProjectile で移動、6fおきに Damage パーティクル、命中判定
- `UpdateCurseMire`: 30fおきに Damage パーティクル、45fおきに EmitAttackHit（再ダメ）、寿命管理
