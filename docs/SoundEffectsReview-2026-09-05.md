# 効果音の割り当てレビュー（2026-09-05）

更新：優先度「高」の3件は2026-09-05に修正済み。死亡とボス撃破、爆弾設置とキャンセル、食品と飲料の使用音を分離した。食品は効果（HP回復／残機増加）を示す短い音に変更し、ジュースだけ飲用音を使う。QTE失敗も独立させた。下表は修正前の調査記録。扉の移動・停止音も前の作業で修正済み。再生成手順と素材出典は tools/audio/priority/README.md および tools/audio/door/README.md を参照。

対象は現行開発版 RedFortress2/MultiPassRendering。通常BGMは対象外とし、スター無敵中の差し替え曲と環境音は併記した。ソースコードの再生箇所、WAVの再生時間、PCMデータの一致を確認した。ゲームを起動した実聴・音色の聴取は実施できていない。したがって、以下は割り当てのレビューであり、怖さ・声質・音楽性を聴き取った評価ではない。実行中のexeが別のresを使っている場合、その音はこの一覧と異なる可能性がある。

判断基準は「子供向けの、明るく親しみやすいホロライブ二次創作ゲーム」。攻撃の手応えは残しつつ、苦痛や威圧よりもコミカルな反応、成功の嬉しさ、操作のわかりやすさを優先する。二次創作だからといって全てを歓声やキャラクターボイスにする必要はなく、頻出音は短く控えめにするのが望ましい。

優先して見直したい割り当て

| 優先 | 対象 | 確認できた事実と懸念 | 提案する方向 |
|---|---|---|---|
| 高 | death.wav：プレイヤー死亡／ボス撃破 | 同じファイルを敗北と勝利に使用。勝った瞬間にも自分が倒れたときの音が鳴る。怖い音かどうかとは別に、結果の意味が逆になる | プレイヤーは短いコミカルな失敗音、ボスは消滅の「ぽふっ」と勝利の短い上昇音に分ける |
| 高 | bombDrop.wav／menu_cursor_cancel.wav | PCMが一致。爆弾設置・ドクロ着地と、戻る操作・QTE失敗が同じ音 | 爆弾は軽い物体の接地音、戻る操作は短い下降UI音、QTE失敗は軽い空振り音に分ける |
| 高 | drink.wav：食品の使用 | メニューからのスパゲッティ・ポテチ・ジュース全てで同じPlayDrinkを呼ぶ。飲用音という割り当てが食品の違いを表現していない。実際に飲む音かは未聴取 | ポテチは軽い「さくっ」、スパゲッティは短い食事音、ジュースは「ごくっ」。回復を示す共通の小さな音を添えてもよい |
| 中 | ammoMax.wav／qte_best.wav | PCMが一致。弾薬満タン・QTE大成功・ステージクリアの規模が同じになる | 弾薬満タンは短い充填完了音、QTEは小さな祝福、クリアは独立した達成ジングル |
| 中 | arrow.wav | ゴール案内の出現とドクロ投擲で同じ音。前者は案内、後者は物体の運動 | ゴール案内は発見チャイム、投擲は短い風切り音。ファイル名から弓矢の発射音とは断定しない |
| 中 | enemyAttack.wav＋damage01.wav | 通常の接触被弾・特殊攻撃被弾で両方を続けて再生。素材が適切でも、重ね方で必要以上に強い被弾印象になる可能性 | 接触と攻撃を区別し、主音を一つに決める。重ねるなら一方を短い補助音にする |
| 中・試聴 | pullOar.wav／pullOar2.wav | レバー・ボタン操作／仕掛けの移動ループに使用。名前はオール用途を示唆するが、水音の有無は未確認 | 水音を含むなら交換。レバーは「かちっ」、扉は素材に合う穏やかな摩擦音。実際に乾いた機械音なら維持できる |
| 中・試聴 | qte.wav | 3.773秒の開始音。PlayQteStopは開始音を停止せず別の決定音を追加するため、早い入力では重なる可能性 | 開始は短い合図にするか、停止時に開始音を止める。実際の残響・無音区間とQTE所要時間を合わせて確認する |
| 中・試聴 | hyperMode.wav | 207.052秒の音源を音量設定78でループし通常BGMを0にする。効果音API扱いだが実質的には楽曲差し替え | スター取得直後の明るい合図と、快活な無敵曲として試聴。冒頭の盛り上がりと通常曲への戻り方を確認する |
| 低 | itemGet.wav／powerup.wav／save_complete.wav | 3ファイルのPCMが一致。全て肯定的なので意味の逆転はないが、セーブの完了や強化の特別感が弱い | 取得音を共通モチーフにし、強化は上昇、セーブは落ち着いた終止形に変える |
| 低・試聴 | slashHit.wav／stomp_impact.wav／weaponChange.wav | 斬撃とロープ切断、踏みつけと機構停止、武器切替とドクロ取得をそれぞれ共用 | 物理的な手触りが合えば共用可。ロープは繊維が切れる短音、機構停止は材質別の軽い止まり音が望ましい |

音色を聴いてから判断すべき候補

- death.wav、damage01.wav、enemyAttack.wav：生々しい苦痛の声や威圧的な鳴き声を含むかを確認する。名前だけで暗い・怖いと判定しない。
- explosion.wav（2.997秒）、buster_hit.wav（1.505秒）、stomp_impact.wav（1.257秒）：連戦で低音や余韻が蓄積しないかを確認する。子供向けでも爆発や打撃そのものは不自然ではない。
- menu_cursor_move.wav（0.937秒）、cursor_move.wav（1.064秒）、jump_action.wav（1.104秒）：頻出するため、連続再生で邪魔にならないかを確認する。ファイル長には無音も含まれるので、この長さだけで冗長とは判定しない。
- menu_open.wavと各決定音：開閉・移動・決定を通して音の材質と音量感が揃うかを確認する。ゲームの世界観より汎用PCの通知音を強く連想させるなら調整する。

使用中の音源一覧

「使用中」は現行ソースに再生経路があるという意味。全ステージで実際に発生することまで実走確認したものではない。秒数はWAVヘッダーから算出したファイル全長。音量はソースの設定値で、聴感上の音量やdBではない。環境音とループ音の音量は下に注記する。

効果音・特殊ループは **40ファイル**（PCMが異なるものは36種類）、使用経路のある環境音は2ファイル。海環境音は定数定義のみで、再生経路がないため参考行として記載する。

| ファイル | 鳴る場面 | 秒 | 音量設定 | 定義／呼び出し関数 |
|---|---|---:|---|---|
| ENV_forest.wav | 森林環境音。読み込み中・多くのステージの初期値 | 37.888 | 環境音・下記参照 | [音源定義](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:30>) |
| ENV_sea.wav | 海環境音（定数定義のみ・現行再生経路なし） | 31.591 | 環境音・下記参照 | [音源定義](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:31>) |
| ENV_rain.wav | 雨設定時の環境音 | 30.080 | 環境音・下記参照 | [音源定義](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:32>) |
| menu_cursor_move.wav | メニュー移動、会話送り、各リスト選択 | 0.937 | 70 | [PlayMenuMove](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:603>) |
| menu_cursor_confirm.wav | メニュー決定、クラフト成功、QTE通常成功 | 0.912 | 78 / 76 | [PlayMenuConfirm](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:604>)、[PlayQteNormal](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:791>) |
| menu_cursor_cancel.wav | メニューを戻る、クラフト不可、一部の使用不可、QTE失敗 | 1.013 | 72 / 76 | [PlayMenuCancel](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:605>)、[PlayQteFailure](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:792>) |
| item_unavailable.wav | アイテムを今は使用できない | 0.679 | 78 | [PlayItemUnavailable](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:606>) |
| menu_open.wav | ポーズメニューを開く | 0.192 | 78 | [PlayMenuOpen](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:607>) |
| craft_open.wav | クラフトを開く | 0.272 | 78 | [PlayCraftOpen](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:608>) |
| explanation_open.wav | 説明を開く | 0.625 | 76 | [PlayExplanationOpen](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:609>) |
| save_complete.wav | セーブ完了 | 1.007 | 78 | [PlaySaveComplete](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:610>) |
| cursor_move.wav | ステージ選択の移動 | 1.064 | 72 | [PlayStageSelectMove](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:614>) |
| cursor_confirm.wav | ステージ選択決定、QTEの停止入力 | 1.010 | 78 / 70 | [PlayStageSelectConfirm](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:615>)、[PlayQteStop](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:789>) |
| attack01.wav | 剣以外の近接攻撃を振る | 0.445 | 82 | [PlayPlayerAttack](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:616>) |
| sword_swing.wav | 剣を振る | 0.613 | 80 | [PlaySwordSwing](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:617>) |
| slashHit.wav | 剣の命中、ロープ切断系トリガー | 0.406 | 82 | [PlaySlashHit](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:618>)、[PlayRopeCut](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:622>) |
| pullOar.wav | レバー・リフトレバー・ボタンの操作 | 0.819 | 80 | [PlayLeverToggle](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:621>) |
| stomp_impact.wav | 敵の踏みつけ、仕掛けの移動停止 | 1.257 | 80 / 82 | [PlayMechanismStop](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:623>)、[PlayStomp](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:778>) |
| pullOar2.wav | 扉・仕掛けの移動中（ループ） | 1.520 | 52（ループ） | [音源定義](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:49>) |
| pushable_box_scrape.wav | 箱を押している間（ループ） | 1.285 | 46（ループ） | [音源定義](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:50>) |
| club_hit.wav | 打撃命中、敵への一部ダメージ、投げたドクロの命中 | 1.003 | 82 / 78 | [PlayAttackHit](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:619>)、[PlaySkullHit](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:782>) |
| sword_deflected.wav | 剣攻撃を弾かれる | 0.313 | 82 | [PlaySwordDeflected](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:620>) |
| buster_hit.wav | バスター弾の命中 | 1.505 | 76 | [PlayBusterHit](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:712>) |
| enemyAttack.wav | 敵の接触・攻撃がプレイヤーに当たる | 1.251 | 72 | [PlayEnemyAttack](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:713>) |
| damage01.wav | プレイヤーのHPが減る | 0.968 | 88 | [PlayPlayerDamage](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:714>) |
| death.wav | プレイヤー死亡、ボス撃破 | 1.915 | 92 / 88 | [PlayBossDefeat](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:613>)、[PlayPlayerDeath](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:715>) |
| itemGet.wav | 収集アイテム・ドロップ素材の取得 | 1.007 | 82 | [PlayItemGet](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:716>) |
| ammoMax.wav | 弾薬が満タンになる／すでに満タン | 2.377 | 78 | [PlayAmmoMax](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:717>) |
| ammoGet.wav | 弾薬の補充（満タン未満） | 0.539 | 80 | [PlayAmmoGet](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:718>) |
| jump_action.wav | ジャンプ成立、復帰演出のジャンプ | 1.104 | 62 | [PlayJump](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:719>) |
| powerup.wav | 移動速度アップ取得 | 1.007 | 82 | [PlayPowerUp](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:720>) |
| drink.wav | メニューからスパゲッティ・ポテチ・ジュースを使用 | 2.413 | 80 | [PlayDrink](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:721>) |
| hyperMode.wav | スター無敵中（ループ、通常BGMを消音） | 207.052 | 78（ループ） | [音源定義](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:63>) |
| dash.wav | ダッシュ | 1.238 | 72 | [PlayDash](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:774>) |
| dashBooster2.wav | ダッシュブースター発動 | 2.554 | 78 | [PlayDashBooster](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:775>) |
| explosion.wav | 爆弾の爆発 | 2.997 | 75 | [PlayExplosion](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:776>) |
| bombDrop.wav | 爆弾設置、ドクロの着地 | 1.013 | 78 / 62 | [PlayBombPlace](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:777>)、[PlaySkullLand](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:783>) |
| buster.wav | バスター発射 | 1.384 | 55 | [PlayBuster](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:779>) |
| arrow.wav | ゴール案内矢印の出現、ドクロ投擲、F2キーの試聴用呼び出し | 0.912 | 68 / 100 | [PlaySkullThrow](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:781>)、[PlayArrow](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:785>) |
| weaponChange.wav | 武器切替、ドクロを持ち上げる | 1.097 | 72 | [PlaySkullGrab](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:780>)、[PlayWeaponChange](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:784>) |
| qte.wav | QTE開始（単発） | 3.773 | 70 | [PlayQteStart](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:788>) |
| qte_best.wav | QTE大成功、ステージクリア | 2.377 | 86 / 82 | [PlayStageClear](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:787>)、[PlayQteSuccess](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:790>) |
| warp.wav | ワープ | 0.467 | 80 | [PlayWarp](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:786>) |

環境音は森林が初期値。雨設定で雨に切り替わる。海環境音は定数定義だけで、切り替える処理がない。設定音量は初期18、セレクト・拠点・未クリアのボス戦は14、要塞は16など。洞窟・遺跡でも初期値の森林が残る経路があるため、もし森林音に目立つ鳥の声などが入っているなら、背景と合うか確認したい。実際の音の内容は未確認。

同一PCMの補足

上記の現役音源同士の一致に加えて、sword_swing.wavは現在未参照のjump.wavとPCMが一致する。これだけでは剣の音として不適切とは言えないが、ジャンプ用の音をそのまま転用した可能性があるため試聴対象に含める。名称と音色を同一視しない。

再生方法に関する根拠

- [ボス撃破に死亡音を使用](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:612>)、[ボス演出から再生](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameApp.cpp:7286>)。
- [メニュー決定音に飲用音を重ねる](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/PauseMenu.cpp:951>)、[食べ物・飲み物の実際の処理](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameApp.cpp:5691>)。ショートカットからのポテチ／ジュース使用はこのメニュー側の飲用音を経由しないため、使用経路による通知の差もある。
- [敵の攻撃音からHP減少処理を呼ぶ](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameApp.cpp:7773>)、[HP減少時の被弾音](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameApp.cpp:7813>)。
- [ゴール案内矢印の出現音](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameApp.cpp:4689>)、[F2キーでも同音を再生](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameApp.cpp:2278>)。
- [単発音は位置指定なしで再生](<C:/Users/bibindon/source/repos/bibindon/RedFortress/RedFortress2/MultiPassRendering/GameAudio.cpp:343>)。単発SEは同一ファイルでも独立したバッファで重ねて再生される（SoundLib.cpp:1109以降）。離れた物体でも単発音の距離減衰はこの経路では指定していない。

現在のGameAudio.cppから参照されていない同梱WAV

`ENV_torch.wav`、`battle1.wav`、`battle2.wav`、`bgm_select1.wav`、`bgm_select4.wav`、`collideRaft.wav`、`cursor_cancel.wav`、`darkHit.wav`、`darkSet.wav`、`dead.wav`、`doukutsu.wav`、`dying.wav`、`enemyHanen.wav`、`enemyOrange.wav`、`enemyStep.wav`、`field1.wav`、`field2.wav`、`field3.wav`、`fireHit.wav`、`fireSet.wav`、`get_item.wav`、`haikyo.wav`、`iceHit.wav`、`iceSet.wav`、`jinja.wav`、`jump.wav`、`jump2.wav`、`message1.wav`、`minatoato.wav`、`opening.wav`、`stomp.wav`、`windy.wav`、`world1.wav`。

これらは通常BGM候補も含む同梱ファイルであり、「現在鳴っている不自然な効果音」には数えていない。

今回は調査レポートのみ作成。ゲームコード・音源・設定の変更、ビルドは行っていない。
