import os
import math
import random

from PIL import Image, ImageDraw, ImageFilter

OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
SIZE = 1024
random.seed(20260907)


def speckle(base, amount):
    noise = Image.effect_noise((SIZE, SIZE), 32).convert("L")
    gray = Image.new("L", (SIZE, SIZE), 128)
    diff = Image.blend(gray, noise, amount)
    return diff


def apply_speckle(img, amount=0.10):
    n = speckle(img, 64).convert("RGB")
    return Image.blend(img, Image.composite(img, n, img.convert("L").point(lambda v: 128)), amount)


def make_cobble():
    W, H = SIZE, SIZE
    mortar = Image.new("RGB", (W, H), (66, 60, 54))
    stones = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    shadow = Image.new("RGBA", (W, H), (0, 0, 0, 0))
    ds = ImageDraw.Draw(stones)
    dh = ImageDraw.Draw(shadow)
    rows = 8
    rh = H // rows
    palette = [
        (150, 138, 118), (136, 126, 110), (146, 130, 106),
        (128, 120, 108), (140, 118, 98), (134, 128, 116),
    ]
    for r in range(rows):
        y0 = r * rh
        x = -random.randint(20, 90) if (r % 2 == 1) else -random.randint(0, 30)
        while x < W + 20:
            sw = random.randint(88, 150)
            gap = 10
            box = [x + gap, y0 + gap, x + sw - gap, y0 + rh - gap]
            rad = random.randint(26, 40)
            dh.rounded_rectangle(
                [box[0] + 3, box[1] + 4, box[2] + 3, box[3] + 4],
                radius=rad, fill=(0, 0, 0, 130))
            base = list(random.choice(palette))
            jitter = random.randint(-9, 9)
            base = tuple(max(0, min(255, v + jitter)) for v in base)
            ds.rounded_rectangle(box, radius=rad, fill=base + (255,))
            w, h = box[2] - box[0], box[3] - box[1]
            grad = Image.new("L", (1, h))
            for yy in range(h):
                t = yy / max(1, h - 1)
                grad.putpixel((0, yy), int(200 - 160 * t))
            grad = grad.resize((w, h))
            mask = Image.new("L", (w, h), 0)
            ImageDraw.Draw(mask).rounded_rectangle([0, 0, w, h], radius=rad, fill=255)
            stone_crop = stones.crop((box[0], box[1], box[2], box[3]))
            lit = Image.composite(
                Image.new("RGB", (w, h), (255, 250, 240)),
                Image.new("RGB", (w, h), (10, 8, 14)),
                grad)
            stone_crop = Image.blend(
                Image.new("RGB", (w, h), base),
                Image.composite(stone_crop, lit, mask), 0.65)
            stones.paste(stone_crop, (box[0], box[1]))
            for _ in range(random.randint(3, 7)):
                px = random.randint(box[0] + 6, box[2] - 6)
                py = random.randint(box[1] + 6, box[3] - 6)
                ds.ellipse([px - 2, py - 2, px + 2, py + 2],
                           fill=(0, 0, 0, random.randint(15, 35)))
            x += sw
    shadow = shadow.filter(ImageFilter.GaussianBlur(3.0))
    img = Image.alpha_composite(mortar.convert("RGBA"), shadow)
    img = Image.alpha_composite(img, stones)
    img = img.convert("RGB")
    img = img.filter(ImageFilter.GaussianBlur(0.4))
    img = apply_speckle(img, 0.10)
    img.save(os.path.join(OUTPUT_DIR, "lever_cobble.png"))
    print("WROTE lever_cobble.png")


def make_wood():
    img = Image.new("RGB", (SIZE, SIZE), (138, 95, 54))
    draw = ImageDraw.Draw(img, "RGBA")
    for _ in range(46):
        y_base = random.randint(0, SIZE)
        amp = random.uniform(6.0, 26.0)
        period = random.uniform(180.0, 520.0)
        phase = random.uniform(0.0, 6.28)
        width = random.randint(2, 6)
        dark = random.random() < 0.72
        color = (70, 44, 22, 70) if dark else (178, 132, 84, 60)
        pts = []
        x = -20
        while x < SIZE + 20:
            y = y_base + amp * math.sin(x / period * 6.28 + phase)
            pts.append((x, y))
            x += 16
        draw.line(pts, fill=color, width=width, joint="curve")
    for _ in range(7):
        kx = random.randint(60, SIZE - 60)
        ky = random.randint(60, SIZE - 60)
        for ring in range(4, 0, -1):
            draw.ellipse(
                [kx - ring * 9, ky - ring * 6, kx + ring * 9, ky + ring * 6],
                outline=(80, 52, 28, 110),
                width=3,
            )
    img = img.filter(ImageFilter.GaussianBlur(0.5))
    img = apply_speckle(img, 0.10)
    img.save(os.path.join(OUTPUT_DIR, "lever_wood.png"))
    print("WROTE lever_wood.png")


make_cobble()
make_wood()
