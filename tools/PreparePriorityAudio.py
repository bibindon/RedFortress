"""Build distinct, restrained feedback cues and a CC0 object-placement sound.

Run with Python and ffmpeg on Path. Tonal cues are synthesized here, with
fixed pitches and smooth envelopes, not cartoon pitch bends or vocal sounds.
"""

import array
import math
from pathlib import Path
import subprocess
import wave


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "tools/audio/priority"
OUTPUT = ROOT / "RedFortress2/MultiPassRendering/res/sound"
RATE = 44100


def write(name, samples, peak):
    scale = peak * 32767 / max(abs(value) for value in samples)
    pcm = array.array("h", (round(value * scale) for value in samples))
    with wave.open(str(OUTPUT / name), "wb") as sound:
        sound.setnchannels(1)
        sound.setsampwidth(2)
        sound.setframerate(RATE)
        sound.writeframes(pcm.tobytes())


def cue(name, notes, duration, peak):
    samples = [0.0] * round(duration * RATE)
    for start, frequency, length, level in notes:
        count = round(length * RATE)
        offset = round(start * RATE)
        for index in range(count):
            time = index / RATE
            attack = min(1.0, time / 0.012)
            release = min(1.0, (count - 1 - index) / (0.06 * RATE))
            envelope = attack * release * math.exp(-4.5 * time / length)
            phase = 2 * math.pi * frequency * time
            tone = math.sin(phase) + 0.18 * math.sin(2 * phase)
            tone += 0.045 * math.sin(3 * phase)
            samples[offset + index] += level * envelope * tone
    write(name, samples, peak)


def main():
    # A quiet descending perfect fifth acknowledges defeat without a scream.
    cue("player_defeat.wav", [(0, 392, 0.42, 1), (0.22, 261.6256, 0.60, 0.8)],
        0.86, 0.38)
    # Short major resolution fits inside the existing boss-defeat sequence.
    cue("boss_defeat.wav", [(0, 523.2511, 0.42, 1),
                           (0.16, 659.2551, 0.45, 0.85),
                           (0.32, 783.9909, 0.68, 0.8),
                           (0.32, 523.2511, 0.68, 0.30)], 1.04, 0.42)
    cue("ui_back.wav", [(0, 587.3295, 0.10, 1),
                       (0.065, 440, 0.12, 0.65)], 0.20, 0.30)
    cue("qte_failure.wav", [(0, 293.6648, 0.23, 1)], 0.25, 0.28)
    cue("item_heal.wav", [(0, 523.2511, 0.25, 1),
                         (0.11, 659.2551, 0.33, 0.8)], 0.48, 0.34)
    cue("item_life_up.wav", [(0, 659.2551, 0.26, 1),
                            (0.12, 783.9909, 0.28, 0.85),
                            (0.24, 1046.5023, 0.40, 0.7)], 0.68, 0.34)

    decoded = subprocess.run(
        ["ffmpeg", "-v", "error", "-i", str(SOURCE / "dropLeather.ogg"),
         "-af", "highpass=f=90,lowpass=f=2200,atrim=duration=0.35,"
         "afade=t=in:d=0.003,afade=t=out:st=0.23:d=0.12",
         "-ac", "1", "-ar", str(RATE), "-f", "s16le", "-"],
        capture_output=True, check=True)
    write("bomb_place.wav", list(array.array("h", decoded.stdout)), 0.38)


if __name__ == "__main__":
    main()
