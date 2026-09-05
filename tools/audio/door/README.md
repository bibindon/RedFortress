# Door mechanism audio

Added 2026-09-05. Sources by Kenney, licensed CC0:
- https://kenney.nl/assets/sci-fi-sounds (Sci-Fi Sounds 1.0)
- https://kenney.nl/assets/impact-sounds (Impact Sounds 1.0)

Original OGG files and the two original license notices are in this directory.
Rebuild with `python tools/PrepareDoorAudio.py` (requires ffmpeg on Path).

Outputs in RedFortress2/MultiPassRendering/res/sound:
- lever_latch.wav: impactMetal_light_000.ogg, filtered and trimmed to 0.22 s; peak 0.40; game volume 55.
- door_slide_loop.wav: doorOpen_000.ogg, filtered 100-2400 Hz; 0.04-0.28 s interior, four Hann-windowed overlapping grains, matched endpoints; 0.24 s loop; peak 0.36; game volume 38.
- door_stop.wav: impactMetal_medium_000.ogg, filtered and trimmed to 0.26 s; peak 0.46; game volume 58.

All outputs are mono 44.1 kHz 16-bit PCM WAV. Existing mechanism timing is preserved.
The common lever/button and mechanism-stop functions also serve lifts and other attack-trigger mechanisms.
Player stomp and rope-cut sounds are unchanged. Existing unrelated working-tree edits were preserved.

Selection is based on the source labels and signal inspection, not a listening test.
Verified PCM format, absence of sample clipping, loop endpoint continuity, preload registration,
Debug x64 build, and byte-identical deployment beside the executable.
