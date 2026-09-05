"""Build restrained mechanical door sounds from the bundled Kenney CC0 sources.

Requires ffmpeg on Path. Run from any working directory.
"""

import array
import math
from pathlib import Path
import subprocess
import wave


ROOT = Path(__file__).resolve().parent.parent
SOURCE = ROOT / "tools/audio/door"
OUTPUT = ROOT / "RedFortress2/MultiPassRendering/res/sound"
RATE = 44100


def decode(name, filters):
    result = subprocess.run(
        ["ffmpeg", "-v", "error", "-i", str(SOURCE / name),
         "-af", filters, "-ac", "1", "-ar", str(RATE),
         "-f", "s16le", "-"],
        capture_output=True, check=True)
    return list(array.array("h", result.stdout))


def write(name, samples, peak):
    scale = peak * 32767 / max(abs(value) for value in samples)
    pcm = array.array("h", (round(value * scale) for value in samples))
    with wave.open(str(OUTPUT / name), "wb") as sound:
        sound.setnchannels(1)
        sound.setsampwidth(2)
        sound.setframerate(RATE)
        sound.writeframes(pcm.tobytes())


def main():
    # Use the sustained interior of the sliding-door sound, excluding its
    # opening transient and closing tail. Overlap four windowed grains so
    # the loop has no repeated starting impact or silent gap.
    movement = decode("doorOpen_000.ogg", "highpass=f=100,lowpass=f=2400")
    grain = movement[round(0.04 * RATE):round(0.28 * RATE)]
    length = len(grain)
    loop = [0.0] * length
    for phase in range(4):
        offset = phase * length // 4
        for index, value in enumerate(grain):
            window = 0.5 - 0.5 * math.cos(2 * math.pi * index / length)
            loop[(index + offset) % length] += value * window
    # Remove the tiny residual wrap discontinuity without inserting silence.
    difference = loop[-1] - loop[0]
    loop = [value - difference * index / (length - 1)
            for index, value in enumerate(loop)]
    write("door_slide_loop.wav", loop, 0.36)

    lever = decode("impactMetal_light_000.ogg",
                   "highpass=f=150,lowpass=f=3200,atrim=duration=0.22,"
                   "afade=t=in:d=0.002,afade=t=out:st=0.12:d=0.10")
    write("lever_latch.wav", lever, 0.40)
    stop = decode("impactMetal_medium_000.ogg",
                  "highpass=f=100,lowpass=f=2600,atrim=duration=0.26,"
                  "afade=t=in:d=0.002,afade=t=out:st=0.14:d=0.12")
    write("door_stop.wav", stop, 0.46)


if __name__ == "__main__":
    main()
