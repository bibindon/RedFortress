# ステージセレクト・ボス戦BGMのクレジット

## ステージセレクト

### OpenGameArt.org / CC0

下記2曲は OpenGameArt.org の **CC0(パブリックドメイン)作品** です。クレジット表記は必須ではありませんが、出典を記録しておきます。

| 画面 | ファイル | 曲名 | 作者 | ライセンス | 出典 |
|---|---|---|---|---|---|
| select1 (W1: 草原と湿地) | `bgm_select1.wav` | GrassLands Theme | DST | CC0 | https://opengameart.org/content/grasslands-theme |
| select2 (W2: 洞窟と鉱山) | `stageselect2.wav` | (リポジトリ既存ファイル) | - | - | 元々 `res/sound/` にあった未使用ファイル |
| select4 (W4: 夜の要塞) | `bgm_select2.wav` | Cave Theme | Brandon75689 (HaelDB 投稿) | CC0 / OGA-BY 3.0 | https://opengameart.org/content/cave-theme |

### DOVA-SYNDROME

| 画面/用途 | ファイル | 曲名 | 作曲者 | ライセンス | 出典 |
|---|---|---|---|---|---|
| select3 (W3: 夕暮れの山岳遺跡) | `bgm_select3_ronri.wav` | 論理的思考 | Phalene | DOVA-SYNDROME 音源利用ライセンス + 追加条件 | https://dova-s.jp/bgm/detail/9090 |
| ボス戦 (1-8, 3-8) | `bgm_boss_crazyhill.wav` | CrazyHill | もっぴーさうんど | DOVA-SYNDROME 音源利用ライセンス | https://dova-s.jp/bgm/detail/1395 |

### DOVA-SYNDROME ライセンスの要点 (2026-08-01 確認)

https://dova-s.jp/help/articles/license/ より:

- 商用・非商用問わず無料。**クレジット表記は不要**(作曲者の追加条件が無い限り)。
- 加工可(ファイル形式の変換、イントロカット、ループ加工、エフェクト等)。
- 有償・無償のゲーム製品への組み込み利用可。
- **禁止**: 音源単体の配布・販売(ゲームに組み込んだ形での配布は可)。
  エンドユーザーが音源ファイルを容易に取り出せる状態での利用は、配布形態によっては
  ライセンス上の注意点になるため、配布時はご確認ください。
- 作曲者ごとの追加条件:
  - **Phalene**(論理的思考): **R-18コンテンツでの使用禁止**。それ以外はサイト準拠。
  - もっぴーさうんど(CrazyHill): サイト準拠(追加条件なし)。

## 履歴・未使用ファイル

- `bgm_select3.wav`(The Field Of Dreams / pauliuw / CC0 / https://opengameart.org/content/the-field-of-dreams) は
  2026-08-01 に「論理的思考」へ差し替えのため削除。
- `bgm_select4.wav` — Dark Shrine Loop (qubodup) / CC0 / https://opengameart.org/content/dark-shrine-loop
  (未使用。差し替え候補として残置)

## 変換処理

- OpenGameArt の曲: ffmpeg で 44.1kHz / 16bit / ステレオ / PCM WAV に変換。
- DOVA-SYNDROME の曲: 公式サイト配布音源(MP3)を ffmpeg で
  44.1kHz / 16bit / ステレオ / PCM WAV に変換(SoundLib は非圧縮 PCM WAV のみ再生可能)。
