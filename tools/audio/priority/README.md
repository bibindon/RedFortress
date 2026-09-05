# Priority audio replacements (2026-09-05)

Rebuild: `python tools/PreparePriorityAudio.py` (Python and ffmpeg on Path).

- player_defeat.wav: original synthesized fixed-pitch descending fifth, 0.86 s, peak 0.38, game volume 68.
- boss_defeat.wav: original synthesized ascending major resolution, 1.04 s, peak 0.42, volume 72.
- ui_back.wav: original short descending two-note UI cue, 0.20 s, peak 0.30, volume 62.
- qte_failure.wav: original subdued single-note result cue, 0.25 s, peak 0.28, volume 62.
- item_heal.wav: original two-note recovery cue, 0.48 s, peak 0.34, volume 68; potato chips.
- item_life_up.wav: original three-note life-gain cue, 0.68 s, peak 0.34, volume 68; spaghetti.
- bomb_place.wav: filtered/trimmed Kenney dropLeather.ogg, 0.35 s, peak 0.38, volume 62.

Kenney source: https://kenney.nl/assets/rpg-audio
Download: https://kenney.nl/media/pages/assets/rpg-audio/8e99002d76-1677590336/kenney_rpg-audio.zip
License: CC0. Original OGG and original license notice are included here.
The six tonal cues are synthesized entirely by the checked-in script; no external music samples.
All outputs are mono 44.1 kHz, 16-bit PCM WAV.

Design: understated fixed pitches and smooth decays, without cartoon pitch swoops or pain voices.
Food cues indicate the gameplay benefit rather than imitating chewing. Existing drink.wav is reserved
for juice, at volume 65. Success audio is emitted by HandleInventoryItemUse so menus and shortcuts
share the same behavior; the menu no longer overlays confirmation and drinking sounds.
Skull landing retains bombDrop.wav, explicitly preloaded after bomb placement stops using it.
QTE failure has its own cue instead of sharing the menu-back cue.

Selection is based on source metadata and signal inspection; no listening or in-game auditory test.
