#!/usr/bin/env python3
"""STAGE_PLAN.md に従って 2-1～4-8 のステージ CSV を生成する。"""

import csv
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
MODEL_DIR = ROOT / "RedFortress2" / "MultiPassRendering" / "res" / "model"

RENDER_HEADER = ("ID", "FileName", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "loadType")
PHYSICS_HEADER = ("ID", "FileName", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "Type", "Move", "Instancing")
MOVE_HEADER = ("ID", "RenderID", "PhysicsID", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale",
               "StartX", "StartY", "StartZ", "EndX", "EndY", "EndZ", "Duration")
ENEMY_HEADER = ("Type", "PosX", "PosY", "PosZ", "RotY")
DESTRUCTIBLE_HEADER = ("PosX", "PosY", "PosZ", "HP")
COLLECTIBLE_HEADER = ("CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale")
LAVA_HEADER = ("ID", "PhysicsID", "Damage")
BOOSTER_HEADER = ("DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ",
                  "Speed", "Duration", "Radius", "Scale")
SPEED_HEADER = ("PosX", "PosY", "PosZ")


def wall(x, z, rotation=0):
    return (x, 1.5, z, rotation)


def crate(x, z, y=0):
    return (x, y, z)


def enemy(kind, x, z, rotation=0, y=0.2):
    return (kind, x, y, z, rotation)


def item(data_id, x, z, y=0.45):
    return (data_id, x, y, z)


def lava(x, z, radius=4, damage=20):
    return (x, 0, z, radius, damage)


def platform(x, y, z, end_x, end_y, end_z, duration=8):
    return (x, y, z, end_x, end_y, end_z, duration)


def booster(x, z, direction_x, direction_y, direction_z, speed=20, duration=1.4):
    return (x, 0.5, z, direction_x, direction_y, direction_z, speed, duration, 1.1, 0.6)


STAGES = (
    # World 2: 洞窟と鉱山
    {"display": "2-1", "folder": "stage_2_1", "world": 2, "theme": "洞窟の入口",
     "start": (0, -28), "goal": (0, 28),
     "walls": (wall(-22, -12), wall(-22, 12), wall(22, -12), wall(22, 12),
               wall(-10, 0, 90), wall(10, 0, 90)),
     "crates": (crate(-30, -24), crate(30, -20), crate(-30, 22), crate(30, 26)),
     "enemies": (enemy("small_spider", -10, -12), enemy("spider", 12, -4, 180),
                 enemy("small_spider", -12, 10), enemy("spider", 14, 20, 180)),
     "items": (item("005", -34, -24), item("006", 34, 24), item("009", -30, 30))},
    {"display": "2-2", "folder": "stage_2_2", "world": 2, "theme": "分かれ道",
     "start": (-14, 0), "goal": (14, 0),
     "walls": (wall(0, -18, 90), wall(0, 18, 90), wall(-14, 10), wall(14, -10),
               wall(-28, -28, 90), wall(-28, 28, 90)),
     "crates": (crate(28, -22), crate(34, -22), crate(28, 22), crate(34, 22), crate(-36, 30)),
     "enemies": (enemy("small_spider", -34, -18, 90), enemy("skeleton", -30, 18, 90),
                 enemy("spider", 28, -14, 270), enemy("small_spider", 34, 14, 270),
                 enemy("skeleton", 2, 30, 180)),
     "items": (item("001", 32, -26), item("002", 38, 22), item("011", -40, 32))},
    {"display": "2-3", "folder": "stage_2_3", "world": 2, "theme": "昇降坑",
     "start": (0, 28), "goal": (0, -28),
     "walls": (wall(-18, 18), wall(18, 8), wall(-18, -8), wall(18, -18),
               wall(-34, 0, 90), wall(34, 0, 90)),
     "crates": (crate(-24, 28), crate(24, 20), crate(-24, -20), crate(24, -28)),
     "platforms": (platform(-8, 0.4, 12, -8, 3.0, 12, 6),
                   platform(8, 0.4, 0, 8, 3.0, 0, 6),
                   platform(-8, 0.4, -12, -8, 3.0, -12, 6)),
     "enemies": (enemy("small_spider", -22, 20), enemy("skeleton", 22, 14, 180),
                 enemy("spider", -22, 0), enemy("small_skeleton", 22, -12, 180),
                 enemy("spider", -14, -22)),
     "items": (item("005", -8, 12, y=3.6), item("009", 8, 0, y=3.6), item("006", 30, -30))},
    {"display": "2-4", "folder": "stage_2_4", "world": 2, "theme": "地下溶岩湖",
     "start": (14, 28), "goal": (-14, -28),
     "walls": (wall(-18, 18, 90), wall(18, 18, 90), wall(-18, -18, 90), wall(18, -18, 90)),
     "crates": (crate(32, 30), crate(-32, 28), crate(32, -28), crate(-32, -30)),
     "lava": (lava(-12, 8, 5), lava(0, 8, 5), lava(12, 8, 5),
              lava(-12, -8, 5), lava(0, -8, 5), lava(12, -8, 5)),
     "enemies": (enemy("small_golem", -26, 20), enemy("skeleton", 28, 14, 180),
                 enemy("spider", -28, -12), enemy("small_spider", 28, -18, 180),
                 enemy("ghost", 0, -24)),
     "items": (item("001", 38, 32), item("010", -38, 30), item("014", 38, -32))},
    {"display": "2-5", "folder": "stage_2_5", "world": 2, "theme": "木箱迷路",
     "start": (0, -28), "goal": (0, 28),
     "walls": (wall(-24, -16), wall(-8, -16), wall(16, -16), wall(28, 0, 90),
               wall(16, 16), wall(0, 16), wall(-24, 16), wall(-30, 0, 90)),
     "crates": (crate(-16, -24), crate(8, -24), crate(24, -8), crate(24, 8),
                crate(8, 24), crate(-16, 24), crate(-28, 28), crate(34, -28)),
     "enemies": (enemy("small_spider", -28, -20), enemy("skeleton", 12, -10, 180),
                 enemy("ghost", 30, 14, 270), enemy("spider", -14, 10),
                 enemy("ghost", 12, 24, 180)),
     "items": (item("005", -32, 30), item("006", 36, -30), item("011", 28, 24))},
    {"display": "2-6", "folder": "stage_2_6", "world": 2, "theme": "採掘砲台",
     "start": (-14, 0), "goal": (14, 0),
     "walls": (wall(-34, -22, 90), wall(-34, 22, 90), wall(34, -22, 90), wall(34, 22, 90),
               wall(0, -34), wall(0, 34)),
     "crates": (crate(-34, -14), crate(-34, 14), crate(34, -14), crate(34, 14)),
     "boosters": (booster(-38, -24, 1, 0.08, 0, 24, 1.8),
                  booster(38, 24, -1, 0.08, 0, 24, 1.8)),
     "enemies": (enemy("small_golem", -34, 28), enemy("skeleton", -32, -22),
                 enemy("spider", 32, 20, 180), enemy("ghost", 34, -24, 270),
                 enemy("small_spider", 36, 28, 180)),
     "items": (item("009", -46, -24), item("010", 46, 24), item("006", 0, 38))},
    {"display": "2-7", "folder": "stage_2_7", "world": 2, "theme": "三つの採掘区",
     "start": (0, 28), "goal": (0, -28),
     "walls": (wall(-32, 18, 90), wall(-8, 18, 90), wall(20, 18, 90),
               wall(-20, -18, 90), wall(8, -18, 90), wall(32, -18, 90)),
     "crates": (crate(-36, 30), crate(-28, 30), crate(32, 28), crate(-30, 0),
                crate(30, 0), crate(-30, -30), crate(30, -30)),
     "lava": (lava(-18, -30, 4), lava(18, -30, 4)),
     "enemies": (enemy("small_spider", -28, 28), enemy("spider", 26, 28, 180),
                 enemy("skeleton", -26, 0), enemy("ghost", 26, 0, 180),
                 enemy("small_golem", -24, -28), enemy("skeleton", 24, -26, 180)),
     "items": (item("001", -42, 34), item("006", 40, 32), item("011", 0, -36))},
    {"display": "2-8", "folder": "stage_2_8", "world": 2, "theme": "地底湖の主",
     "start": (14, 28), "goal": (-14, -28),
     "walls": (wall(-22, 22), wall(22, 22), wall(-22, -22), wall(22, -22)),
     "lava": (lava(-26, 0, 5), lava(26, 0, 5), lava(0, 26, 5), lava(0, -26, 5)),
     "enemies": (enemy("golem", 0, 0, 180),)},

    # World 3: 夕暮れの山岳遺跡
    {"display": "3-1", "folder": "stage_3_1", "world": 3, "theme": "山麓の遺跡",
     "start": (0, -28), "goal": (0, 28),
     "walls": (wall(-24, -10), wall(24, -10), wall(-24, 16), wall(24, 16),
               wall(0, -48, 90), wall(0, 48, 90)),
     "crates": (crate(-12, -36), crate(12, -36), crate(-12, 0), crate(12, 0), crate(-12, 36), crate(12, 36)),
     "enemies": (enemy("bird", -24, -42, 180, 3), enemy("skeleton", 22, -16),
                 enemy("bird", 24, 4, 180, 3), enemy("ghost", -22, 22),
                 enemy("bird", 0, 44, 180, 3)),
     "items": (item("005", -34, -52), item("009", 34, 8), item("011", -34, 52))},
    {"display": "3-2", "folder": "stage_3_2", "world": 3, "theme": "崩れた段丘",
     "start": (-14, 0), "goal": (14, 0),
     "walls": (wall(-22, -30, 90), wall(22, -30, 90), wall(-22, 30, 90), wall(22, 30, 90)),
     "crates": (crate(-36, -32), crate(36, -32), crate(-36, 0), crate(36, 0), crate(-36, 32), crate(36, 32)),
     "platforms": (platform(0, 0.4, -48, 0, 0.4, -42, 5), platform(0, 0.4, 48, 0, 0.4, 42, 5)),
     "enemies": (enemy("skeleton", -34, -30), enemy("bird", 34, -28, 180, 3),
                 enemy("ghost", -36, 0), enemy("skeleton", 36, 0, 180),
                 enemy("bird", -34, 30, 0, 3), enemy("small_golem", 34, 30, 180)),
     "items": (item("001", -42, -36), item("006", 42, 0), item("014", -42, 36))},
    {"display": "3-3", "folder": "stage_3_3", "world": 3, "theme": "大砲の峡谷",
     "start": (0, 28), "goal": (0, -28),
     "walls": (wall(-18, -54, 90), wall(18, -54, 90), wall(-18, 0, 90), wall(18, 0, 90),
               wall(-18, 54, 90), wall(18, 54, 90)),
     "crates": (crate(34, 58), crate(34, 42), crate(34, 8), crate(34, -8), crate(34, -42), crate(34, -58)),
     "boosters": (booster(-42, 48, 1, 0.12, 0, 26, 2.0), booster(42, -48, -1, 0.12, 0, 26, 2.0)),
     "enemies": (enemy("bird", -38, 62, 180, 3), enemy("skeleton", 34, 44),
                 enemy("ghost", -38, 18), enemy("bird", 36, -16, 0, 3),
                 enemy("skeleton", -38, -44), enemy("small_golem", 36, -64, 180)),
     "items": (item("009", -48, 48), item("010", 48, -48), item("011", 40, 68))},
    {"display": "3-4", "folder": "stage_3_4", "world": 3, "theme": "上層と下層",
     "start": (14, 28), "goal": (-14, -28),
     "walls": (wall(-22, -44), wall(22, -22), wall(-22, 0), wall(22, 22), wall(-22, 44)),
     "crates": (crate(-36, -42), crate(-36, -42, 1.8), crate(34, -18), crate(34, -18, 1.8),
                crate(-34, 8), crate(-34, 8, 1.8), crate(34, 42), crate(34, 42, 1.8)),
     "enemies": (enemy("skeleton", -30, -56), enemy("ghost", 18, -34, 180),
                 enemy("bird", -30, -10, 0, 3), enemy("skeleton", 30, 12, 180),
                 enemy("ghost", -18, 38), enemy("bird", 30, 60, 180, 3)),
     "items": (item("005", -36, -42, 4.0), item("009", 34, -18, 4.0), item("014", 34, 42, 4.0))},
    {"display": "3-5", "folder": "stage_3_5", "world": 3, "theme": "ワープめいろ",
     "start": (0, -112), "goal": (0, 112),
     "walls": (wall(-32, -100, 90), wall(32, -100, 90), wall(-32, 104, 90), wall(32, 104, 90)),
     "crates": (crate(-48, -108), crate(48, -108), crate(-48, 108), crate(48, 108)),
     "platforms": (platform(-18, 0.4, -30, 18, 0.4, -30, 10),
                   platform(0, 0.4, 40, 0, 3.5, 40, 8)),
     "enemies": (enemy("ghost", -30, -105), enemy("small_skeleton", 30, -105, 180),
                 enemy("skeleton", -38, -72), enemy("ghost", 20, -65, 180),
                 enemy("small_skeleton", -20, -70), enemy("skeleton", 38, -75, 180),
                 enemy("ghost", -20, -25), enemy("skeleton", 20, -25, 180),
                 enemy("small_skeleton", 0, -35), enemy("ghost", 38, -30, 180),
                 enemy("skeleton", -38, -15), enemy("ghost", 38, -15, 180),
                 enemy("small_skeleton", -19, -10), enemy("skeleton", 19, -10, 180),
                 enemy("ghost", 0, -5), enemy("skeleton", -38, 20),
                 enemy("ghost", 38, 20, 180), enemy("small_skeleton", -20, 25),
                 enemy("skeleton", 20, 25, 180), enemy("ghost", -38, 20),
                 enemy("skeleton", -20, 90), enemy("small_skeleton", 20, 90, 180),
                 enemy("ghost", 0, 100)),
     "items": (item("001", -38, -65), item("006", 0, 40), item("011", 38, -5))},
    {"display": "3-6", "folder": "stage_3_6", "world": 3, "theme": "亡霊の回廊",
     "start": (-14, 0), "goal": (14, 0),
     "walls": (wall(-24, -54), wall(8, -54), wall(24, -28), wall(-8, -28),
               wall(-24, 0), wall(8, 0), wall(24, 28), wall(-8, 28), wall(-24, 54), wall(8, 54)),
     "crates": (crate(-38, -62), crate(38, -48), crate(-38, -18), crate(38, 8), crate(-24, 36), crate(38, 62)),
     "enemies": (enemy("ghost", -34, -66), enemy("skeleton", 34, -52, 180),
                 enemy("ghost", -34, -22), enemy("skeleton", 34, 4, 180),
                 enemy("ghost", -24, 34), enemy("skeleton", 34, 64, 180)),
     "items": (item("005", -44, -68), item("009", 44, 8), item("014", -44, 38))},
    {"display": "3-7", "folder": "stage_3_7", "world": 3, "theme": "八の字遺跡",
     "start": (0, 28), "goal": (0, -28),
     "walls": (wall(-28, -46, 90), wall(28, -46, 90), wall(-18, -16), wall(18, -16),
               wall(-18, 16), wall(18, 16), wall(-28, 46, 90), wall(28, 46, 90)),
     "crates": (crate(-38, -60), crate(38, -60), crate(-26, 0), crate(26, 0), crate(-38, 60), crate(38, 60)),
     "platforms": (platform(-20, 0.4, 52, 20, 0.4, 52, 10),),
     "boosters": (booster(-42, -52, 1, 0.1, 0, 24, 1.9),),
     "enemies": (enemy("bird", -34, -64, 0, 3), enemy("skeleton", 34, -48, 180),
                 enemy("ghost", -34, -14), enemy("small_golem", 34, 14, 180),
                 enemy("bird", -34, 48, 0, 3), enemy("skeleton", 34, 66, 180),
                 enemy("ghost", 0, 0)),
     "items": (item("009", -48, -52), item("010", 0, 54), item("014", 44, 68))},
    {"display": "3-8", "folder": "stage_3_8", "world": 3, "theme": "山頂の守護者",
     "start": (14, 28), "goal": (-14, -28),
     "walls": (wall(-28, -22), wall(28, -22), wall(-28, 22), wall(28, 22),
               wall(-38, 0, 90), wall(38, 0, 90)),
     "enemies": (enemy("golem", 0, 0, 180),)},

    # World 4: 夜の要塞
    {"display": "4-1", "folder": "stage_4_1", "world": 4, "theme": "外城門",
     "start": (0, -28), "goal": (0, 28),
     "walls": (wall(-34, -40), wall(-18, -40), wall(18, -40), wall(34, -40),
               wall(-24, -4, 90), wall(24, -4, 90), wall(-34, 36), wall(34, 36)),
     "crates": (crate(-10, -42), crate(10, -42), crate(-32, -12), crate(32, -12),
                crate(-30, 34), crate(30, 34)),
     "enemies": (enemy("skeleton", -30, -52), enemy("spider", 30, -52, 180),
                 enemy("ghost", -32, -16), enemy("small_golem", 32, -16, 180),
                 enemy("skeleton", -30, 18), enemy("bird", 30, 20, 180, 3),
                 enemy("golem", 0, 42, 180)),
     "items": (item("005", -42, -56), item("009", 42, -18), item("011", -42, 40))},
    {"display": "4-2", "folder": "stage_4_2", "world": 4, "theme": "危険な堀",
     "start": (-14, 0), "goal": (14, 0),
     "walls": (wall(-32, -28, 90), wall(32, -28, 90), wall(-32, 28, 90), wall(32, 28, 90)),
     "crates": (crate(-42, -34), crate(42, -34), crate(-42, 34), crate(42, 34)),
     "platforms": (platform(-16, 0.4, 44, 16, 0.4, 44, 10),),
     "lava": (lava(-24, 44, 5, 25), lava(-10, 44, 5, 25), lava(10, 44, 5, 25), lava(24, 44, 5, 25)),
     "enemies": (enemy("ghost", -38, -52), enemy("skeleton", 38, -52, 180),
                 enemy("spider", -38, -18), enemy("small_golem", 38, -18, 180),
                 enemy("bird", -38, 18, 0, 3), enemy("skeleton", 38, 58, 180)),
     "items": (item("001", -46, -56), item("010", 46, 0), item("014", 0, 52))},
    {"display": "4-3", "folder": "stage_4_3", "world": 4, "theme": "二つの監視塔",
     "start": (0, 28), "goal": (0, -28),
     "walls": (wall(-34, -42), wall(-34, -26), wall(34, -42), wall(34, -26),
               wall(-34, 26), wall(-34, 42), wall(34, 26), wall(34, 42)),
     "crates": (crate(-40, -34), crate(-40, -34, 1.8), crate(40, -34), crate(40, -34, 1.8),
                crate(-40, 34), crate(-40, 34, 1.8), crate(40, 34), crate(40, 34, 1.8)),
     "enemies": (enemy("bird", -38, -52, 0, 4), enemy("skeleton", 38, -50, 180),
                 enemy("bird", -38, -10, 0, 4), enemy("golem", 0, 0, 180),
                 enemy("skeleton", 38, 14, 180), enemy("bird", -38, 54, 0, 4),
                 enemy("ghost", 38, 56, 180)),
     "items": (item("009", -40, -34, 4.0), item("010", 40, -34, 4.0), item("014", -40, 34, 4.0))},
    {"display": "4-4", "folder": "stage_4_4", "world": 4, "theme": "地下搬入口",
     "start": (14, 28), "goal": (-14, -28),
     "walls": (wall(-28, -54), wall(4, -54), wall(28, -26), wall(-4, -26),
               wall(-28, 2), wall(4, 2), wall(28, 30), wall(-4, 30), wall(-28, 58), wall(4, 58)),
     "crates": (crate(-40, -62), crate(-32, -62), crate(36, -42), crate(44, -42),
                crate(-40, -12), crate(-32, -12), crate(36, 18), crate(44, 18),
                crate(-40, 50), crate(-32, 50)),
     "enemies": (enemy("small_spider", -38, -70), enemy("skeleton", 38, -48, 180),
                 enemy("ghost", -38, -20), enemy("spider", 38, 8, 180),
                 enemy("small_golem", -38, 38), enemy("skeleton", 38, 66, 180)),
     "items": (item("005", -46, -64), item("006", 46, 18), item("009", -46, 52), item("014", 46, 70))},
    {"display": "4-5", "folder": "stage_4_5", "world": 4, "theme": "城壁砲台",
     "start": (0, -28), "goal": (0, 28),
     "walls": (wall(-38, -54, 90), wall(38, -54, 90), wall(-38, 0, 90), wall(38, 0, 90),
               wall(-38, 54, 90), wall(38, 54, 90)),
     "crates": (crate(-44, -66), crate(44, -66), crate(-44, -12), crate(44, -12), crate(-44, 42), crate(44, 42)),
     "boosters": (booster(-46, -58, 1, 0.12, 0, 26, 2.0),
                  booster(46, 0, -1, 0.12, 0, 26, 2.0),
                  booster(-46, 58, 1, 0.12, 0, 26, 2.0)),
     "enemies": (enemy("ghost", -38, -72), enemy("skeleton", 38, -60, 180),
                 enemy("bird", -38, -26, 0, 3), enemy("golem", 38, -4, 180),
                 enemy("spider", -38, 24), enemy("bird", 38, 48, 180, 3),
                 enemy("skeleton", 0, 72, 180)),
     "items": (item("009", -50, -58), item("010", 50, 0), item("014", -50, 58))},
    {"display": "4-6", "folder": "stage_4_6", "world": 4, "theme": "機関中庭",
     "start": (-14, 0), "goal": (14, 0),
     "walls": (wall(-22, -54), wall(22, -54), wall(-22, -18), wall(22, -18),
               wall(-22, 18), wall(22, 18), wall(-22, 54), wall(22, 54)),
     "crates": (crate(-40, -64), crate(40, -64), crate(-40, 0), crate(40, 0), crate(-40, 64), crate(40, 64)),
     "platforms": (platform(-12, 0.4, -43, 12, 0.4, -43, 8),
                   platform(12, 0.42, 43, -12, 0.42, 43, 8)),
     "lava": (lava(-12, -22, 4, 25), lava(12, -22, 4, 25), lava(-12, 22, 4, 25), lava(12, 22, 4, 25)),
     "enemies": (enemy("skeleton", -36, -68), enemy("bird", 36, -58, 180, 3),
                 enemy("ghost", -36, -16), enemy("small_golem", 36, -12, 180),
                 enemy("spider", -36, 34), enemy("bird", 36, 52, 180, 3),
                 enemy("golem", 0, 70, 180)),
     "items": (item("001", -46, -70), item("009", 46, 0), item("014", -46, 70))},
    {"display": "4-7", "folder": "stage_4_7", "world": 4, "theme": "四つの区画",
     "start": (0, 28), "goal": (0, -28),
     "walls": (wall(-36, -42, 90), wall(0, -42, 90), wall(36, -42, 90),
               wall(-36, 0, 90), wall(0, 0, 90), wall(36, 0, 90),
               wall(-36, 42, 90), wall(0, 42, 90), wall(36, 42, 90)),
     "crates": (crate(-42, -66), crate(42, -66), crate(-42, -22), crate(42, -22),
                crate(-42, 22), crate(42, 22), crate(-42, 66), crate(42, 66)),
     "platforms": (platform(-16, 0.4, 50, 16, 0.4, 50, 9),),
     "lava": (lava(-28, -24, 4, 25), lava(28, -24, 4, 25)),
     "boosters": (booster(-46, 24, 1, 0.1, 0, 24, 1.9),),
     "enemies": (enemy("wolf", -34, -70), enemy("bird", 34, -64, 180, 3),
                 enemy("spider", -34, -28), enemy("skeleton", 34, -20, 180),
                 enemy("ghost", -34, 18), enemy("small_golem", 34, 30, 180),
                 enemy("golem", 0, 70, 180), enemy("bird", -34, 66, 0, 3)),
     "items": (item("001", -48, -70), item("006", 48, -24), item("010", -48, 24), item("014", 48, 70))},
    {"display": "4-8", "folder": "stage_4_8", "world": 4, "theme": "最終決戦",
     "start": (14, 28), "goal": (-14, -28),
     "walls": (wall(-34, -28), wall(34, -28), wall(-34, 28), wall(34, 28),
               wall(-44, 0, 90), wall(44, 0, 90)),
     "enemies": (enemy("kanata", 0, 0, 180, 1.5),)},
)


def write_csv(path, header, rows):
    with path.open("w", encoding="utf-8-sig", newline="") as output:
        writer = csv.writer(output, lineterminator="\r\n")
        writer.writerow(header)
        writer.writerows(rows)


def add_boundaries(render_rows, half_width, half_depth):
    next_id = 8000
    x = -half_width + 4
    while x <= half_width - 4:
        render_rows.append((next_id, "../fence.x", x, 0.5, -half_depth, 0, 0, 0, 1, "normal"))
        next_id += 1
        render_rows.append((next_id, "../fence.x", x, 0.5, half_depth, 0, 0, 0, 1, "normal"))
        next_id += 1
        x += 8
    z = -half_depth + 4
    while z <= half_depth - 4:
        render_rows.append((next_id, "../fence.x", -half_width, 0.5, z, 0, 90, 0, 1, "normal"))
        next_id += 1
        render_rows.append((next_id, "../fence.x", half_width, 0.5, z, 0, 90, 0, 1, "normal"))
        next_id += 1
        z += 8


def additional_tall_walls(stage, half_depth):
    if stage["folder"] == "stage_4_3":
        return (wall(-42, -72), wall(0, -72, 90), wall(42, 72))
    if stage["folder"] == "stage_2_8":
        return (wall(-42, -42), wall(0, 42, 90), wall(42, 42))
    if stage["folder"] == "stage_3_8":
        return (wall(-42, -72), wall(0, 72, 90), wall(42, 72))
    if stage["folder"] in ("stage_1_1", "stage_1_2", "stage_1_3", "stage_1_4"):
        return (wall(-10, -20, 90), wall(10, 0, 90), wall(-10, 20, 90))
    depth_offset = 42
    if half_depth >= 120:
        depth_offset = 72
    return (wall(-42, -depth_offset), wall(0, 0, 90), wall(42, depth_offset))

def validate_stage(stage):
    world = stage["world"]
    enemy_count = len(stage["enemies"])
    if stage["display"].endswith("-8"):
        if enemy_count != 1:
            raise ValueError(stage["display"] + ": ボス戦はボス1体のみです")
    else:
        minimum = 4
        maximum = 6
        if world == 3:
            minimum = 5
            maximum = 7
        if world == 4:
            minimum = 6
            maximum = 8
        if enemy_count < minimum or enemy_count > maximum:
            raise ValueError(stage["display"] + ": 通常ステージの敵数が範囲外です")

    protected = (stage["start"], stage["goal"])
    for kind, x, unused_y, z, unused_rotation in stage["enemies"]:
        for protected_x, protected_z in protected:
            distance_squared = (x - protected_x) ** 2 + (z - protected_z) ** 2
            if distance_squared < 49:
                raise ValueError(stage["display"] + ": " + kind + " が開始地点またはポータルに近すぎます")


def build_stage(stage):
    validate_stage(stage)
    folder = stage["folder"]
    stage_dir = MODEL_DIR / folder
    stage_dir.mkdir(parents=True, exist_ok=True)
    world = stage["world"]
    half_width = 60
    half_depth = 60
    bounds_model = "cubeNormalInverse120x120.x"
    sky_model = "../SkySphere_cave/SkySphere.blend.x"
    if world >= 3:
        half_depth = 120
        bounds_model = "cubeNormalInverse120x240.x"
        sky_model = "../SkySphere_evening/SkySphere.blend.x"
    if world == 4:
        sky_model = "../SkySphere_night/SkySphere.blend.x"

    render_rows = [(2, "stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2")]
    physics_rows = [
        (1, "res/model/" + bounds_model, 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""),
        (2, "res/model/" + folder + "/stage_ground.x", 0, 0.01, 0, 0, 0, 0, 1, "Collision", "n", ""),
    ]
    move_rows = []
    next_id = world * 1000 + int(stage["display"][-1]) * 100

    for x, y, z, rotation in stage.get("walls", ()):
        next_id += 1
        render_rows.append((next_id, "../collision_wall/collision_wall.x", x, y, z, 0, rotation, 0, 1, "meshmix2"))
        physics_rows.append((next_id, "res/model/collision_wall/collision_wall_collision.x",
                             x, y, z, 0, rotation, 0, 1, "Collision", "n", ""))
    for x, y, z in stage.get("crates", ()):
        next_id += 1
        render_rows.append((next_id, "../cubeWood/cube_wood.x", x, y, z, 0, 0, 0, 1, "normal"))
        physics_rows.append((next_id, "res/model/cubeWood/cube_wood_collision.x",
                             x, y, z, 0, 0, 0, 1, "Collision", "n", ""))
    for platform_index, values in enumerate(stage.get("platforms", ()), start=1):
        x, y, z, end_x, end_y, end_z, duration = values
        next_id += 1
        render_rows.append((next_id, "../collision_moving_platform/collision_moving_platform.x",
                            x, y, z, 0, 0, 0, 1, "meshmix2"))
        physics_rows.append((next_id, "res/model/collision_moving_platform.x",
                             x, y, z, 0, 0, 0, 1, "Collision", "y", ""))
        move_rows.append((platform_index, next_id, next_id, x, y, z, 0, 0, 0, 1,
                          x, y, z, end_x, end_y, end_z, duration))
    lava_physics_ids = []
    for lava_index, values in enumerate(stage.get("lava", ()), start=1):
        x, y, z, radius, unused_damage = values
        next_id += 1
        lava_physics_ids.append(next_id)
        render_rows.append((next_id, "../plateLava.x", x, y + 0.02, z, 0, 0, 0,
                            radius / 4.0, "meshmix2"))
        physics_rows.append((next_id, "res/model/plateLava.x", x, y + 0.02, z, 0, 0, 0,
                             radius / 4.0, "NonCollision", "n", ""))

    next_id += 1
    render_rows.append((next_id, sky_model, 0, 0.01, 0, 0, 0, 0, 1, "normal"))
    add_boundaries(render_rows, half_width, half_depth)
    next_id = max(next_id, 9000)
    for x, y, z, rotation in additional_tall_walls(stage, half_depth):
        next_id += 1
        render_rows.append((next_id, "../collision_wall/collision_wall_tall.x", x, y, z, 0, rotation, 0, 1, "meshmix2"))
        physics_rows.append((next_id, "res/model/collision_wall/collision_wall_tall_collision.x",
                             x, y, z, 0, rotation, 0, 1, "Collision", "n", ""))
    enemy_rows = list(stage["enemies"])
    destructible_rows = []
    for index, values in enumerate(stage.get("crates", ())):
        if index % 2 == 0:
            x, unused_y, z = values
            destructible_rows.append((x + 3, 0, z, 3))
    collectible_rows = []
    for index, values in enumerate(stage.get("items", ()), start=1):
        data_id, x, y, z = values
        collectible_rows.append((folder + "-I" + str(index).zfill(2), "Item", data_id, x, y, z, 1))
    lava_rows = []
    for index, values in enumerate(stage.get("lava", ()), start=1):
        x, y, z, radius, damage = values
        lava_rows.append((folder + "-lava-" + str(index).zfill(2), lava_physics_ids[index - 1], damage))
    booster_rows = []
    for index, values in enumerate(stage.get("boosters", ()), start=1):
        booster_rows.append((folder + "-dashbooster-" + str(index).zfill(2),) + values)

    write_csv(stage_dir / "XFileList_simple.csv", RENDER_HEADER, render_rows)
    write_csv(stage_dir / "XFileListPhysics.csv", PHYSICS_HEADER, physics_rows)
    write_csv(stage_dir / "XFileListMove.csv", MOVE_HEADER, move_rows)
    write_csv(stage_dir / "EnemyPositions.csv", ENEMY_HEADER, enemy_rows)
    write_csv(stage_dir / "Destructibles.csv", DESTRUCTIBLE_HEADER, destructible_rows)
    write_csv(stage_dir / "Collectibles.csv", COLLECTIBLE_HEADER, collectible_rows)
    write_csv(stage_dir / "LavaZones.csv", LAVA_HEADER, lava_rows)
    write_csv(stage_dir / "DashBoosters.csv", BOOSTER_HEADER, booster_rows)
    write_csv(stage_dir / "SpeedUps.csv", SPEED_HEADER, ())


def main():
    for stage in STAGES:
        build_stage(stage)
    print("2-1～4-8 の24ステージを生成しました。")


if __name__ == "__main__":
    main()
