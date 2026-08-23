import csv
import math
import os
from collections import deque
from pathlib import Path

import bpy


GROUND_DIR = Path(__file__).resolve().parent
MODEL_DIR = GROUND_DIR.parent
BLEND_PATH = GROUND_DIR / "stage_grounds.blend"
SIDE_TEXTURE_PATH = MODEL_DIR / "whiteWall.png"
SLAB_BOTTOM = -20.0
PIT_BOTTOM = -18.0
VISUAL_GROUND_HALF_SIZE = 500.0

# Per-world top-surface textures (CC0 from Poly Haven, see tex/POLYHAVEN_CC0.md).
# stage_ground.x lives in res/model/<folder>/ and references the texture as
# "../ground/tex/<file>", which resolves to res/model/ground/tex/<file>.
TEX_DIR = GROUND_DIR / "tex"
WORLD_TOP_TEXTURES = {
    1: TEX_DIR / "world1.png",
    2: TEX_DIR / "world2.png",
    3: TEX_DIR / "world3.png",
    4: TEX_DIR / "world4.png",
}

# Map a stage folder name (e.g. "stage_1_5") to its world number (1..4).
# Folder names use the new "stage_<world>_<stage>" convention (e.g. stage_3_5 -> World 3).
def world_for_folder(folder):
    digits = "".join(ch for ch in folder if ch.isdigit())
    if not digits:
        return 1
    if folder.startswith("stage_"):
        world_digits = "".join(ch for ch in folder.split("_")[1] if ch.isdigit())
        if world_digits:
            world = int(world_digits)
            if 1 <= world <= 4:
                return world
    n = int(digits)
    if n in (1, 2, 3, 4, 17, 18, 19, 20):
        return 1
    if n in (5, 6, 7, 8, 21, 22, 23, 24):
        return 2
    if n in (9, 10, 11, 12, 25, 26, 27, 28):
        return 3
    return 4


STAGES = (
    {"display": "1-1", "folder": "stage_1_1", "size": (16.0, 32.0), "start": (0.0, -28.0), "goal": (0.0, 28.0),
     "pits": ((4.0, 7.0, -32.0, -24.0), (-16.0, 16.0, 0.0, 6.0), (-16.0, -12.0, 7.5, 31.0), (-6.0, 16.0, 7.5, 18.0)),
     "jump_links": (((3.0, -27.0), (8.0, -27.0)),),
     "static_platforms": ((13.0, -4.0, 1.5),)},
    {"display": "1-2", "folder": "stage_1_2", "size": (16.0, 32.0), "start": (0.0, -28.0), "goal": (0.0, 28.0),
     "pits": ((-8.0, 8.0, -24.0, -22.0), (-16.0, 16.0, 6.0, 12.0), (-16.0, 7.0, -17.0, -11.0), (13.0, 16.0, -17.0, -11.0)),
     "jump_links": (((0.0, -25.0), (0.0, -21.0)),),
     "static_platforms": ((11.0, -24.0, 1.5), (-12.0, -1.0, 3.0))},
    {"display": "1-3", "folder": "stage_1_3", "size": (16.0, 32.0), "start": (0.0, 28.0), "goal": (0.0, -28.0),
     "pits": ((-14.5, 14.5, -30.5, 30.5),),
     "elevated_route": True,
     "jump_links": (((0.0, 26.0), (4.0, 25.0)), ((4.0, 25.0), (10.0, 24.0)),
                    ((10.0, 22.0), (10.0, 20.0)), ((10.0, 16.0), (10.0, 14.0)),
                    ((10.0, -5.0), (4.0, -6.0)), ((-4.0, -8.0), (-10.0, -10.0)),
                    ((-10.0, -26.0), (-4.0, -24.0)), ((-4.0, -24.0), (0.0, -26.0)),
                    ((-10.0, -10.0), (-10.0, -4.0)), ((-10.0, -4.0), (-10.0, 2.0)),
                    ((-10.0, 2.0), (-10.0, 8.0)), ((-10.0, 8.0), (-10.0, 14.0)),
                    ((-10.0, 14.0), (-10.0, 20.0)),
                    ((10.0, -2.0), (10.0, -8.0)), ((10.0, -8.0), (10.0, -14.0)),
                    ((10.0, -14.0), (10.0, -20.0))),
     "static_platforms": ((0.0, 26.0, 3.0), (4.0, 25.0, 1.5), (10.0, 24.0, 3.0), (10.0, 18.0, 3.0),
                          (10.0, 12.0, 3.0), (10.0, 6.0, 5.0), (10.0, -2.0, 3.0),
                          (4.0, -6.0, 1.5), (-4.0, -8.0, 1.5), (-10.0, -10.0, 3.0),
                          (-10.0, -18.0, 7.0), (-10.0, -26.0, 3.0), (-4.0, -24.0, 1.5),
                          (0.0, -26.0, 3.0),
                          (-10.0, -4.0, 1.5), (-10.0, 2.0, 1.5), (-10.0, 8.0, 1.5),
                          (-10.0, 14.0, 1.5), (-10.0, 20.0, 1.5),
                          (10.0, -8.0, 1.5), (10.0, -14.0, 1.5), (10.0, -20.0, 1.5),
                          (14.0, -16.0, 1.5))},
    {"display": "1-4", "folder": "stage_1_4", "size": (16.0, 32.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": (),
     "static_platforms": ((0.0, -16.0, 2.25), (-4.0, -15.0, 2.25), (-8.0, -14.0, 2.25), (-4.0, -11.0, 2.25),
                          (0.0, -8.0, 2.25), (4.0, -5.0, 2.25), (8.0, -2.0, 2.25), (8.0, 2.0, 2.25),
                          (4.0, 5.0, 2.25), (0.0, 8.0, 2.25), (-4.0, 11.0, 2.25), (-8.0, 14.0, 2.25),
                          (-4.0, 17.0, 2.25), (0.0, 20.0, 2.25), (0.0, 24.0, 2.25), (13.0, 2.0, 2.25),
                          (13.0, 9.0, 2.25), (13.0, 16.0, 2.25), (0.0, 2.0, 2.25), (0.0, 28.0, 3.0),
                          (-14.0, -4.0, 3.0))},
    {"display": "1-5", "folder": "stage_1_5", "size": (16.0, 32.0), "start": (0.0, -28.0), "goal": (0.0, 28.0),
     "pits": (),
     "static_platforms": ((-10.0, -19.0, 1.0), (4.0, -15.0, 3.0), (-6.0, -9.0, 5.0),
                          (10.0, -3.0, 7.0), (-4.0, 3.0, 9.0), (6.0, 10.0, 11.0),
                          (-8.0, 17.0, 13.0), (10.0, 20.0, 15.0), (-5.0, 27.0, 17.0),
                          (0.0, 28.0, 18.6), (16.0, 8.0, 11.0), (-6.0, 21.0, 15.0)),
     "booster_links": (((0.0, -20.0), (-10.0, -19.0)),
                       ((-13.5, -20.5), (4.0, -15.0)),
                       ((1.0, -14.5), (-6.0, -9.0)),
                       ((-3.0, -8.5), (10.0, -3.0)),
                       ((7.0, -2.5), (-4.0, 3.0)),
                       ((-1.0, 3.5), (6.0, 10.0)),
                       ((3.0, 10.5), (-8.0, 17.0)),
                       ((-5.0, 17.5), (10.0, 20.0)),
                       ((7.0, 20.5), (-5.0, 27.0)),
                       ((-5.0, 26.5), (0.0, 28.0)),
                       ((9.0, 10.5), (16.0, 8.0)),
                       ((7.0, 17.5), (-6.0, 21.0))),
     "jump_links": (((-14.0, -26.0), (-14.0, -14.0)),
                    ((-6.0, -27.0), (-6.0, 21.0)))},
    {"display": "1-6", "folder": "stage_1_6", "size": (16.0, 32.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0),
     "pits": ((-9.0, -4.5, -3.0, 3.0),
              (-4.5, 5.5, -30.0, -8.0),
              (-4.5, 10.5, 8.0, 30.0),
              (-1.5, -0.5, -8.0, 8.0),
              (1.5, 3.0, -8.0, 8.0),
              (5.5, 10.5, -8.0, 8.0),
              (5.5, 14.8, -30.0, -26.0),
              (5.5, 14.8, -20.0, -8.0),
              (10.5, 14.8, 8.0, 10.5),
              (10.5, 14.8, 20.0, 30.0),
              (-14.8, -4.5, 8.0, 30.0)),
     "static_platforms": ((7.0, 0.0, 1.5), (9.0, 0.0, 1.5), (9.5, -5.0, 3.0), (-10.0, 14.0, 3.0)),
     "jump_links": (((-2.0, 0.0), (0.0, 0.0)),
                    ((1.0, 0.0), (4.0, 0.0)),
                    ((-2.0, 0.0), (8.0, -24.0))),
     "booster_links": (((4.0, -5.0), (9.5, -5.0)),
                       ((4.0, 3.0), (12.4, 3.0)),
                       ((-10.0, 7.0), (-10.0, 14.0)))},
    {"display": "1-7", "folder": "stage_1_7", "size": (16.0, 32.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-14.8, 14.8, -5.0, 5.0),), "static_platforms": ((5.0, 3.0, 3.0),)},
    {"display": "1-8", "folder": "stage_1_8", "size": (16.0, 32.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ((-14.0, -11.0, -4.0, 4.0), (11.0, 14.0, -4.0, 4.0))},

    {"display": "2-5", "folder": "stage_2_5", "size": (60.0, 60.0), "start": (-50.0, -50.0), "goal": (-51.0, 53.0),
     "pits": (), "elevated_route": True,
     "static_platforms": ((-50.0, -50.0, 6.0), (-37.0, -39.0, 6.0),
                          (-18.0, -43.0, 6.0), (-9.0, -46.0, 3.0), (-9.0, -40.0, 3.0),
                          (8.0, -31.0, 6.0), (31.0, -44.0, 6.0),
                          (39.0, -17.0, 6.0), (48.0, -20.0, 3.0), (48.0, -14.0, 3.0),
                          (32.0, 4.0, 6.0), (14.0, 20.0, 6.0),
                          (-11.0, 17.0, 6.0), (-2.0, 14.0, 3.0), (-2.0, 20.0, 3.0),
                          (-24.0, 27.0, 5.0),
                          (-42.0, 40.0, 6.0), (-33.0, 40.0, 3.0), (-51.0, 53.0, 6.0),
                          (-27.5, -41.0, 1.5), (-3.0, -36.0, 1.5), (0.0, -36.0, 1.5),
                          (18.0, -27.0, 1.5), (23.0, -23.0, 1.5), (28.0, -20.0, 1.5),
                          (35.0, -6.5, 1.5), (23.5, 11.5, 1.5), (4.0, 20.0, 1.5),
                          (-29.5, 33.0, 1.5),
                          (0.0, 28.5, 1.5), (3.0, 33.5, 1.5), (6.0, 38.5, 1.5), (8.0, 44.0, 1.5),
                          (49.0, -7.0, 1.5), (52.0, -4.5, 1.5),
                          (17.0, -35.0, 3.0), (21.5, -38.0, 3.0), (26.0, -41.0, 3.0),
                          (54.0, -1.0, 3.0), (8.0, 52.0, 6.0)),
     "jump_links": (((-50.0, -50.0), (-37.0, -39.0)),
                    ((-37.0, -39.0), (-27.5, -41.0)),
                    ((-27.5, -41.0), (-18.0, -43.0)),
                    ((-18.0, -43.0), (-9.0, -40.0)),
                    ((-9.0, -40.0), (-3.0, -36.0)),
                    ((-3.0, -36.0), (0.0, -36.0)),
                    ((0.0, -36.0), (8.0, -31.0)),
                    ((8.0, -31.0), (18.0, -27.0)),
                    ((18.0, -27.0), (23.0, -23.0)),
                    ((23.0, -23.0), (28.0, -20.0)),
                    ((28.0, -20.0), (39.0, -17.0)),
                    ((39.0, -17.0), (35.0, -6.5)),
                    ((35.0, -6.5), (32.0, 4.0)),
                    ((32.0, 4.0), (23.5, 11.5)),
                    ((23.5, 11.5), (14.0, 20.0)),
                    ((14.0, 20.0), (4.0, 20.0)),
                    ((4.0, 20.0), (-2.0, 20.0)),
                    ((-2.0, 20.0), (-11.0, 17.0)),
                    ((-11.0, 17.0), (-24.0, 27.0)),
                    ((-24.0, 27.0), (-29.5, 33.0)),
                    ((-29.5, 33.0), (-33.0, 40.0)),
                    ((-33.0, 40.0), (-42.0, 40.0)),
                    ((-42.0, 40.0), (-51.0, 53.0))),
     "booster_links": (((44.0, -10.0), (32.0, 4.0)),)},
    {"display": "2-6", "folder": "stage_2_6", "size": (60.0, 60.0), "start": (-52.0, -49.0), "goal": (52.0, 49.0),
     "pits": (), "elevated_route": True,
     "static_platforms": ((-52.0, -48.0, 6.0), (-40.0, -48.0, 6.0), (-52.0, -36.0, 6.0), (-40.0, -36.0, 6.0),
                          (-31.0, -1.0, 6.0), (-19.0, -1.0, 6.0), (-31.0, 11.0, 6.0), (-19.0, 11.0, 6.0),
                          (4.0, -44.0, 6.0), (16.0, -44.0, 6.0), (4.0, -32.0, 6.0), (16.0, -32.0, 6.0),
                          (39.0, 34.0, 6.0), (51.0, 34.0, 6.0), (39.0, 46.0, 6.0), (51.0, 46.0, 6.0),
                          (-42.0, -25.5, 1.5), (-39.0, -19.0, 3.0), (-35.0, -12.5, 3.0), (-30.0, -6.0, 3.0),
                          (-15.0, -12.0, 3.0), (-9.0, -19.0, 3.0), (-3.0, -26.0, 3.0),
                          (30.0, -23.0, 3.0), (38.0, -13.0, 3.0),
                          (32.0, -6.0, 3.0), (24.0, 1.0, 3.0), (17.0, 7.0, 6.0),
                          (20.0, 16.0, 1.5), (26.0, 22.0, 3.0), (34.0, 28.0, 3.0),
                          (27.0, -27.0, 5.0),
                          (-52.0, -25.0, 1.5), (-54.0, -19.0, 1.5), (-54.0, -10.0, 6.0),
                          (-27.0, 23.0, 3.0), (-28.0, 29.0, 3.0), (-28.0, 35.0, 3.0), (-28.0, 41.0, 3.0), (-28.0, 51.0, 6.0),
                          (-5.0, -51.0, 3.0), (-10.0, -52.0, 3.0), (-15.0, -53.0, 3.0), (-22.0, -54.0, 6.0),
                          (8.0, 12.0, 3.0), (0.0, 16.0, 3.0), (-8.0, 19.0, 3.0), (-16.0, 22.0, 3.0),
                          (-24.0, 24.0, 3.0), (-32.0, 26.0, 3.0), (-40.0, 27.0, 3.0), (-52.0, 27.0, 6.0)),
     "jump_links": (((-52.0, -49.0), (-42.0, -25.5)),
                    ((-42.0, -25.5), (-39.0, -19.0)),
                    ((-39.0, -19.0), (-35.0, -12.5)),
                    ((-35.0, -12.5), (-30.0, -6.0)),
                    ((-30.0, -6.0), (-31.0, -1.0)),
                    ((-31.0, -1.0), (-19.0, -1.0)),
                    ((-19.0, -1.0), (-15.0, -12.0)),
                    ((-15.0, -12.0), (-9.0, -19.0)),
                    ((-9.0, -19.0), (-3.0, -26.0)),
                    ((-3.0, -26.0), (4.0, -32.0)),
                    ((4.0, -32.0), (16.0, -32.0)),
                    ((16.0, -32.0), (27.0, -27.0)),
                    ((27.0, -27.0), (30.0, -23.0)),
                    ((30.0, -23.0), (38.0, -13.0)),
                    ((38.0, -13.0), (32.0, -6.0)),
                    ((32.0, -6.0), (24.0, 1.0)),
                    ((24.0, 1.0), (17.0, 7.0)),
                    ((17.0, 7.0), (20.0, 16.0)),
                    ((20.0, 16.0), (26.0, 22.0)),
                    ((26.0, 22.0), (34.0, 28.0)),
                    ((34.0, 28.0), (39.0, 34.0)),
                    ((39.0, 34.0), (51.0, 46.0)),
                    ((39.0, 34.0), (52.0, 49.0))),
     "booster_links": (((10.0, 10.0), (-15.0, 21.0)),)},
    {"display": "2-7", "folder": "stage_2_7", "size": (60.0, 60.0), "start": (-52.0, -50.0), "goal": (52.0, 52.0),
     "pits": ((-60.0, -1.0, -60.0, -58.0),
              (11.0, 60.0, -60.0, -36.0),
              (50.0, 60.0, -36.0, 2.0),
              (-46.0, -1.0, -58.0, 14.0),
              (11.0, 38.0, -24.0, 14.0),
              (-46.0, 38.0, 14.0, 41.0),
              (-60.0, -58.0, -58.0, 53.0),
              (50.0, 60.0, 14.0, 53.0),
              (-60.0, -34.0, 53.0, 60.0),
              (-22.0, 60.0, 53.0, 60.0)),
     "elevated_route": True,
     "static_platforms": ((-52.0, -49.0, 6.0), (-52.0, -34.0, 6.0),
                          (-52.0, 28.0, 6.0), (44.0, 38.0, 6.0),
                          (44.0, -30.0, 6.0), (5.0, -30.0, 6.0),
                          (-28.0, 56.0, 6.0), (56.0, 8.0, 6.0),
                          (5.0, -54.0, 6.0),
                          (5.0, -18.0, 3.0), (5.0, -13.0, 3.0),
                          (5.0, -8.0, 3.0), (5.0, -3.0, 3.0),
                          (5.0, 2.0, 3.0), (5.0, 7.0, 3.0),
                          (5.0, 8.0, 3.0), (5.0, 14.0, 3.0),
                          (5.0, 20.0, 3.0), (5.0, 26.0, 3.0),
                          (5.0, 32.0, 3.0), (11.0, 34.0, 3.0),
                          (17.0, 34.0, 3.0), (23.0, 34.0, 3.0),
                          (29.0, 34.0, 3.0), (35.0, 34.0, 3.0),
                          (41.0, 34.0, 3.0), (47.0, 34.0, 3.0),
                          (52.0, 34.0, 3.0), (52.0, 40.0, 3.0),
                          (52.0, 46.0, 3.0), (52.0, 52.0, 3.0)),
     "jump_links": (((-52.0, -49.0), (-52.0, -34.0)),
                    ((-52.0, -34.0), (-52.0, 28.0)),
                    ((-52.0, 28.0), (44.0, 38.0)),
                    ((44.0, 38.0), (44.0, -30.0)),
                    ((44.0, -30.0), (5.0, -30.0)),
                    ((5.0, -30.0), (5.0, -18.0)),
                    ((5.0, -18.0), (5.0, -13.0)),
                    ((5.0, -13.0), (5.0, -8.0)),
                    ((5.0, -8.0), (5.0, -3.0)),
                    ((5.0, -3.0), (5.0, 2.0)),
                    ((5.0, 2.0), (5.0, 7.0)),
                    ((5.0, 7.0), (5.0, 8.0)),
                    ((5.0, 8.0), (5.0, 14.0)),
                    ((5.0, 14.0), (5.0, 20.0)),
                    ((5.0, 20.0), (5.0, 26.0)),
                    ((5.0, 26.0), (5.0, 32.0)),
                    ((5.0, 32.0), (11.0, 34.0)),
                    ((11.0, 34.0), (17.0, 34.0)),
                    ((17.0, 34.0), (23.0, 34.0)),
                    ((23.0, 34.0), (29.0, 34.0)),
                    ((29.0, 34.0), (35.0, 34.0)),
                    ((35.0, 34.0), (41.0, 34.0)),
                    ((41.0, 34.0), (47.0, 34.0)),
                    ((47.0, 34.0), (52.0, 34.0)),
                    ((52.0, 34.0), (52.0, 40.0)),
                    ((52.0, 40.0), (52.0, 46.0)),
                    ((52.0, 46.0), (52.0, 52.0)))},
    {"display": "2-4", "folder": "stage_2_4", "size": (60.0, 60.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},
    {"display": "2-1", "folder": "stage_2_1", "size": (30.0, 60.0), "start": (-20.0, -54.0), "goal": (20.0, 54.0),
     "pits": (), "dedicated_ground": True},
    {"display": "2-2", "folder": "stage_2_2", "size": (30.0, 60.0), "start": (-20.0, -54.0), "goal": (20.0, 54.0),
     "pits": (), "dedicated_ground": True},
    {"display": "2-3", "folder": "stage_2_3", "size": (30.0, 60.0), "start": (-20.0, -54.0), "goal": (-20.0, 54.0),
     "pits": (), "dedicated_ground": True},
    {"display": "2-8", "folder": "stage_2_8", "size": (60.0, 60.0), "start": (0.0, -42.0), "goal": (0.0, 44.0),
     "pits": (), "dedicated_ground": True},

    {"display": "3-1", "folder": "stage_3_1", "size": (60.0, 120.0), "start": (0.0, -28.0), "goal": (0.0, 28.0),
     "pits": ((-10.0, -4.0, -21.0, -16.0), (4.0, 10.0, -21.0, -16.0),
              (-10.0, 10.0, -10.0, 2.0),
              (-12.0, -1.2, 4.0, 20.0), (1.2, 12.0, 4.0, 20.0),
              (-10.0, -4.0, 22.0, 26.0), (4.0, 10.0, 22.0, 26.0))},
    {"display": "3-2", "folder": "stage_3_2", "size": (60.0, 120.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((-32.0, -6.0, -52.0, -44.0), (6.0, 32.0, -52.0, -44.0), (-32.0, -6.0, 44.0, 52.0), (6.0, 32.0, 44.0, 52.0))},
    {"display": "3-3", "folder": "stage_3_3", "size": (60.0, 120.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-35.0, -25.0, -50.0, 50.0),)},
    {"display": "3-4", "folder": "stage_3_4", "size": (60.0, 120.0), "start": (0.0, -116.0), "goal": (0.0, 114.0),
     "pits": ((-60.0, 60.0, -112.0, 112.0),),
     "static_platforms": ((0.0, -100.0, 6.0), (-16.0, -88.0, 6.0), (-32.0, -76.0, 6.0), (-44.0, -64.0, 6.0),
                          (-28.0, -52.0, 6.0), (-12.0, -40.0, 6.0), (4.0, -28.0, 6.0), (20.0, -16.0, 6.0),
                          (36.0, -4.0, 6.0), (44.0, 10.0, 6.0), (28.0, 22.0, 6.0), (12.0, 34.0, 6.0),
                          (-4.0, 46.0, 6.0), (-8.0, 62.0, 6.0), (0.0, 80.0, 6.0), (0.0, 96.0, 6.0)),
     "booster_links": (((0.0, -113.0), (0.0, -100.0)),
                       ((0.0, -100.0), (-16.0, -88.0)),
                       ((-16.0, -88.0), (-32.0, -76.0)),
                       ((-32.0, -76.0), (-44.0, -64.0)),
                       ((-44.0, -64.0), (-28.0, -52.0)),
                       ((-28.0, -52.0), (-12.0, -40.0)),
                       ((-12.0, -40.0), (4.0, -28.0)),
                       ((4.0, -28.0), (20.0, -16.0)),
                       ((20.0, -16.0), (36.0, -4.0)),
                       ((36.0, -4.0), (44.0, 10.0)),
                       ((44.0, 10.0), (28.0, 22.0)),
                       ((28.0, 22.0), (12.0, 34.0)),
                       ((12.0, 34.0), (-4.0, 46.0)),
                       ((-4.0, 46.0), (-8.0, 62.0)),
                       ((-8.0, 62.0), (0.0, 80.0)),
                       ((0.0, 80.0), (0.0, 96.0)),
                       ((0.0, 96.0), (0.0, 114.0)))},
    {"display": "3-5", "folder": "stage_3_5", "size": (60.0, 120.0), "start": (0.0, -112.0), "goal": (0.0, 112.0),
     "pits": ((-42.0, -28.0, -106.0, -96.0), (28.0, 42.0, -106.0, -96.0),
              (-18.0, 18.0, -72.0, -64.0), (-50.0, -30.0, -35.0, -25.0),
              (-14.0, 14.0, 8.0, 18.0), (-34.0, -14.0, 95.0, 108.0)),
     "jump_links": (((0.0, -100.0), (-30.0, -60.0)),
                    ((30.0, -70.0), (-25.0, -30.0)),
                    ((30.0, -15.0), (-30.0, 15.0)),
                    ((30.0, 35.0), (-30.0, 55.0)),
                    ((30.0, 75.0), (0.0, 100.0)),
                    ((30.0, -88.0), (30.0, -35.0)),
                    ((-30.0, -45.0), (30.0, 5.0)),
                    ((0.0, -5.0), (0.0, 60.0)),
                    ((-40.0, -108.0), (-40.0, 105.0)),
                    ((-30.0, 30.0), (30.0, 90.0)))},
    {"display": "3-6", "folder": "stage_3_6", "size": (60.0, 120.0), "start": (-36.8, -36.8), "goal": (0.0, 0.0), "pits": (),
     "ground_shape": "spiral", "spiral_center": (0.0, 0.0),
     "spiral_start_radius": 52.0, "spiral_end_radius": 2.5,
     "spiral_start_angle": -2.35619449, "spiral_turns": 2.25,
     "spiral_width": 5.5, "spiral_core_radius": 5.0,
     "spiral_samples": 260,
     "static_platforms": ((-4.8, 9.5, 3.2),)},
    {"display": "3-7", "folder": "stage_3_7", "size": (60.0, 120.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-40.0, -30.0, -12.0, 12.0), (30.0, 40.0, -12.0, 12.0), (-18.0, -8.0, 48.0, 64.0), (8.0, 18.0, -64.0, -48.0))},
    {"display": "3-8", "folder": "stage_3_8", "size": (60.0, 120.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},

    {"display": "4-1", "folder": "stage_4_1", "size": (60.0, 120.0), "start": (0.0, -80.0), "goal": (0.0, 80.0), "pits": ()},
    {"display": "4-2", "folder": "stage_4_2", "size": (60.0, 120.0), "start": (-40, -114), "goal": (16, 110), "pits": [(-57.0, -55.0, -6.0, 0.0), (55.0, 57.0, -60.0, -52.0), (-57.0, -55.0, 52.0, 58.0), (55.0, 57.0, 60.0, 66.0)], "static_platforms": ((-40, -114, 3.0), (-45.515, -111.636, 3.0), (-51.03, -109.273, 3.0), (-51.959, -106.129, 3.0), (-47.536, -102.075, 3.0), (-43.113, -98.021, 3.0), (-38.69, -93.966, 3.0), (-13.092, -92.763, 3.0), (-18.663, -90.535, 3.0), (-34.268, -89.912, 3.0), (-24.233, -88.307, 3.0), (-29.804, -86.078, 3.0), (-12.742, -85.775, 3.0), (-19, -81, 3.0), (-14.639, -80.083, 3.0), (-26, -78, 3.0), (-16.536, -74.391, 3.0), (-33, -70, 3.0), (-18.434, -68.698, 3.0), (-20.852, -63.391, 3.0), (-30, -62, 3.0), (-25.735, -59.904, 3.0), (-30.617, -56.416, 3.0), (-27, -53, 3.0), (-32.584, -52.82, 3.0), (-27.975, -48.979, 3.0), (-23.366, -45.138, 3.0), (-18.08, -42.432, 3.0), (-12.509, -40.204, 3.0), (-6.938, -37.975, 3.0), (-1.455, -35.591, 3.0), (3.345, -31.991, 3.0), (8.145, -28.391, 3.0), (12.945, -24.791, 3.0), (17.523, -20.917, 3.0), (22.039, -16.966, 3.0), (26.554, -13.015, 3.0), (36, -10, 3.0), (31.079, -9.075, 3.0), (35.635, -5.17, 3.0), (40.19, -1.266, 3.0), (43.305, 2.695, 3.0), (39.063, 6.937, 3.0), (34.82, 11.18, 3.0), (30.578, 15.422, 3.0), (25.555, 18.667, 3.0), (20.41, 21.754, 3.0), (15.266, 24.841, 3.0), (10.121, 27.928, 3.0), (5.057, 31.146, 3.0), (-0.005, 34.367, 3.0), (-5.067, 37.588, 3.0), (-10.129, 40.81, 3.0), (-14.827, 44.513, 3.0), (-19.312, 48.499, 3.0), (-23.796, 52.485, 3.0), (-28.281, 56.472, 3.0), (-26.632, 59.531, 3.0), (-21.17, 62.014, 3.0), (-15.708, 64.496, 3.0), (-10.246, 66.979, 3.0), (-5.673, 70.659, 3.0), (-1.722, 75.175, 3.0), (2.229, 79.69, 3.0), (5.878, 84.244, 3.0), (3.195, 89.611, 3.0), (0.511, 94.977, 3.0), (-1.635, 100.122, 3.0), (4.057, 102.019, 3.0), (9.749, 103.916, 3.0), (15.441, 105.814, 3.0),), "jump_links": (((-40, -114), (-45.515, -111.636)), ((-45.515, -111.636), (-51.03, -109.273)), ((-51.03, -109.273), (-51.959, -106.129)), ((-51.959, -106.129), (-47.536, -102.075)), ((-47.536, -102.075), (-43.113, -98.021)), ((-43.113, -98.021), (-38.69, -93.966)), ((-38.69, -93.966), (-34.268, -89.912)), ((-34.268, -89.912), (-29.804, -86.078)), ((-29.804, -86.078), (-24.233, -88.307)), ((-24.233, -88.307), (-18.663, -90.535)), ((-18.663, -90.535), (-13.092, -92.763)), ((-13.092, -92.763), (-12.742, -85.775)), ((-12.742, -85.775), (-14.639, -80.083)), ((-14.639, -80.083), (-16.536, -74.391)), ((-16.536, -74.391), (-18.434, -68.698)), ((-18.434, -68.698), (-20.852, -63.391)), ((-20.852, -63.391), (-25.735, -59.904)), ((-25.735, -59.904), (-30.617, -56.416)), ((-30.617, -56.416), (-32.584, -52.82)), ((-32.584, -52.82), (-27.975, -48.979)), ((-27.975, -48.979), (-23.366, -45.138)), ((-23.366, -45.138), (-18.08, -42.432)), ((-18.08, -42.432), (-12.509, -40.204)), ((-12.509, -40.204), (-6.938, -37.975)), ((-6.938, -37.975), (-1.455, -35.591)), ((-1.455, -35.591), (3.345, -31.991)), ((3.345, -31.991), (8.145, -28.391)), ((8.145, -28.391), (12.945, -24.791)), ((12.945, -24.791), (17.523, -20.917)), ((17.523, -20.917), (22.039, -16.966)), ((22.039, -16.966), (26.554, -13.015)), ((26.554, -13.015), (31.079, -9.075)), ((31.079, -9.075), (35.635, -5.17)), ((35.635, -5.17), (40.19, -1.266)), ((40.19, -1.266), (43.305, 2.695)), ((43.305, 2.695), (39.063, 6.937)), ((39.063, 6.937), (34.82, 11.18)), ((34.82, 11.18), (30.578, 15.422)), ((30.578, 15.422), (25.555, 18.667)), ((25.555, 18.667), (20.41, 21.754)), ((20.41, 21.754), (15.266, 24.841)), ((15.266, 24.841), (10.121, 27.928)), ((10.121, 27.928), (5.057, 31.146)), ((5.057, 31.146), (-0.005, 34.367)), ((-0.005, 34.367), (-5.067, 37.588)), ((-5.067, 37.588), (-10.129, 40.81)), ((-10.129, 40.81), (-14.827, 44.513)), ((-14.827, 44.513), (-19.312, 48.499)), ((-19.312, 48.499), (-23.796, 52.485)), ((-23.796, 52.485), (-28.281, 56.472)), ((-28.281, 56.472), (-26.632, 59.531)), ((-26.632, 59.531), (-21.17, 62.014)), ((-21.17, 62.014), (-15.708, 64.496)), ((-15.708, 64.496), (-10.246, 66.979)), ((-10.246, 66.979), (-5.673, 70.659)), ((-5.673, 70.659), (-1.722, 75.175)), ((-1.722, 75.175), (2.229, 79.69)), ((2.229, 79.69), (5.878, 84.244)), ((5.878, 84.244), (3.195, 89.611)), ((3.195, 89.611), (0.511, 94.977)), ((0.511, 94.977), (-1.635, 100.122)), ((-1.635, 100.122), (4.057, 102.019)), ((4.057, 102.019), (9.749, 103.916)), ((9.749, 103.916), (15.441, 105.814)), ((-14.639, -80.083), (-19, -81)), ((-19, -81), (-26, -78)), ((-26, -78), (-33, -70)), ((-33, -70), (-30, -62)), ((-30, -62), (-27, -53)), ((-27, -53), (-27.975, -48.979)), ((36, -10), (35.635, -5.17)),), "booster_links": (((31.079, -9.075), (36, -10)),)},
    {"display": "4-3", "folder": "stage_4_3", "size": (60.0, 120.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-48.0, -42.0, -12.0, 12.0), (42.0, 48.0, -12.0, 12.0))},
    {"display": "4-4", "folder": "stage_4_4", "size": (60.0, 120.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},
    {"display": "4-5", "folder": "stage_4_5", "size": (60.0, 120.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ((-30.0, -20.0, -12.0, 12.0), (20.0, 30.0, -12.0, 12.0), (-12.0, 12.0, 42.0, 52.0), (-12.0, 12.0, -52.0, -42.0))},
    {"display": "4-6", "folder": "stage_4_6", "size": (60.0, 120.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((-42.0, -28.0, 36.0, 50.0), (28.0, 42.0, 36.0, 50.0), (-42.0, -28.0, -50.0, -36.0), (28.0, 42.0, -50.0, -36.0))},
    {"display": "4-7", "folder": "stage_4_7", "size": (60.0, 120.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-28.0, -18.0, 20.0, 36.0), (16.0, 28.0, 4.0, 18.0), (-26.0, -14.0, -18.0, -4.0), (14.0, 24.0, -36.0, -20.0))},
    {"display": "4-8", "folder": "stage_4_8", "size": (60.0, 120.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},
)


POSITION_CSV_FILES = (
    "EnemyPositions.csv",
    "Destructibles.csv",
    "Collectibles.csv",
    "SpeedUps.csv",
    "DashBoosters.csv",
)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials, bpy.data.images):
        for data_block in list(collection):
            collection.remove(data_block)


def create_material(name, texture_path, texture_filename, color):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = color
    material.roughness = 1.0
    material.metallic = 0.0
    material["_x_power"] = 0.0
    material["_x_specular"] = (0.0, 0.0, 0.0)
    material["_x_emissive"] = (0.0, 0.0, 0.0)
    material["_x_texture_filename"] = texture_filename

    principled = next(
        node for node in material.node_tree.nodes
        if node.type == "BSDF_PRINCIPLED"
    )
    principled.inputs["Base Color"].default_value = color
    principled.inputs["Roughness"].default_value = 1.0

    image = bpy.data.images.load(str(texture_path), check_existing=True)
    texture = material.node_tree.nodes.new("ShaderNodeTexImage")
    texture.name = name + "_Texture"
    texture.image = image
    texture.extension = "REPEAT"
    material.node_tree.links.new(texture.outputs["Color"], principled.inputs["Base Color"])
    return material


def point_in_pit(x, y, pits, margin=0.0):
    for pit in pits:
        x_min, x_max, y_min, y_max = pit
        if x >= x_min - margin and x <= x_max + margin:
            if y >= y_min - margin and y <= y_max + margin:
                return True
    return False


def point_on_spiral_ground(x, y, stage, margin=0.0):
    center_x, center_y = stage.get("spiral_center", (0.0, 0.0))
    delta_x = x - center_x
    delta_y = y - center_y
    radius_from_center = math.sqrt(delta_x * delta_x + delta_y * delta_y)
    core_radius = stage["spiral_core_radius"]
    if radius_from_center <= core_radius + margin:
        return True

    start_radius = stage["spiral_start_radius"]
    end_radius = stage["spiral_end_radius"]
    start_angle = stage["spiral_start_angle"]
    total_angle = stage["spiral_turns"] * math.pi * 2.0
    half_width = stage["spiral_width"] * 0.5 + margin
    if total_angle <= 0.0:
        raise RuntimeError(stage["display"] + " has an invalid spiral angle")

    point_angle = math.atan2(delta_y, delta_x)
    for revolution in range(-4, 5):
        angle = point_angle + revolution * math.pi * 2.0
        progress = (angle - start_angle) / total_angle
        if progress < -0.0001 or progress > 1.0001:
            continue
        if progress < 0.0:
            progress = 0.0
        if progress > 1.0:
            progress = 1.0
        expected_radius = start_radius + (end_radius - start_radius) * progress
        expected_x = center_x + expected_radius * math.cos(angle)
        expected_y = center_y + expected_radius * math.sin(angle)
        distance_x = x - expected_x
        distance_y = y - expected_y
        if distance_x * distance_x + distance_y * distance_y <= half_width * half_width:
            return True
    return False


def point_on_stage_ground(x, y, stage, margin=0.0):
    if stage.get("ground_shape") == "spiral":
        return point_on_spiral_ground(x, y, stage, margin)
    return not point_in_pit(x, y, stage["pits"], margin)


def point_on_static_platform(x, y, stage, margin=0.0):
    for platform in stage.get("static_platforms", ()):
        center_x, center_y, half_size = platform
        if x >= center_x - half_size - margin and x <= center_x + half_size + margin:
            if y >= center_y - half_size - margin and y <= center_y + half_size + margin:
                return True
    return False


def validate_stage(stage):
    half_width, half_depth = stage["size"]
    pits = stage["pits"]

    for pit_index, pit in enumerate(pits):
        x_min, x_max, y_min, y_max = pit
        if x_min >= x_max or y_min >= y_max:
            raise RuntimeError(stage["display"] + " has an invalid pit rectangle")
        for other_index in range(pit_index):
            other = pits[other_index]
            separated = x_max <= other[0] or x_min >= other[1]
            if not separated:
                separated = y_max <= other[2] or y_min >= other[3]
            if not separated:
                raise RuntimeError(stage["display"] + " has overlapping pits")

    for label in ("start", "goal"):
        point = stage[label]
        # 地面なしステージ（全面ピット）では start/goal が静的プラットフォームの上に来るため、
        # 「ピット外の地面」または「静的プラットフォームの上」のどちらかで許容する。
        on_ground = point_on_stage_ground(point[0], point[1], stage, margin=1.0)
        on_platform = point_on_static_platform(point[0], point[1], stage, margin=1.0)
        if not on_ground and not on_platform:
            raise RuntimeError(stage["display"] + " " + label + " is outside the playable ground")

    stage_dir = MODEL_DIR / stage["folder"]
    conflicts = []
    for csv_name in POSITION_CSV_FILES:
        csv_path = stage_dir / csv_name
        if not csv_path.exists():
            continue
        with csv_path.open("r", encoding="utf-8-sig", newline="") as file:
            for row_index, row in enumerate(csv.DictReader(file), start=2):
                if "PosX" not in row or "PosZ" not in row:
                    continue
                if row["PosX"] is None or row["PosZ"] is None:
                    continue
                if row["PosX"].strip() == "" or row["PosZ"].strip() == "":
                    continue
                x = float(row["PosX"])
                y = float(row["PosZ"])
                position_y = float(row.get("PosY", "0"))
                # ガレキ（Destructibles）はピット上に浮かせて「道からはみ出す壁」を作れる。
                # 破壊可能で壊すと消えるためピット上でも問題ない（敵・アイテム等は従来どおり NG）。
                if csv_name == "Destructibles.csv":
                    continue
                # 地面なしステージ（全面ピット）ではオブジェクトが静的プラットフォームの上に置かれるため、
                # 「ピット外の地面」または「静的プラットフォームの上」のどちらかで許容する（PosY不問）。
                elevated_and_supported = point_on_static_platform(x, y, stage, margin=0.8)
                if not point_on_stage_ground(x, y, stage, margin=0.8) and not elevated_and_supported:
                    conflicts.append(csv_name + ":" + str(row_index))

    render_path = stage_dir / "XFileList_simple.csv"
    if render_path.exists():
        with render_path.open("r", encoding="utf-8-sig", newline="") as file:
            for row_index, row in enumerate(csv.DictReader(file), start=2):
                filename = row.get("FileName", "")
                lowered = filename.lower()
                if "ground" in lowered or "platefield" in lowered:
                    continue
                if "static_platform" in lowered:
                    continue
                if "skysphere" in lowered or "fence.x" in lowered:
                    continue
                if "collision_moving_platform" in lowered:
                    continue
                if row.get("loadType", "").strip().lower() == "instancing":
                    continue
                if row.get("PosX") is None or row.get("PosZ") is None:
                    continue
                x = float(row["PosX"])
                y = float(row["PosZ"])
                position_y = float(row.get("PosY", "0"))
                elevated_and_supported = point_on_static_platform(x, y, stage, margin=0.8)
                if not point_on_stage_ground(x, y, stage, margin=0.8) and not elevated_and_supported:
                    conflicts.append("XFileList_simple.csv:" + str(row_index))

    collision_rectangles = load_collision_rectangles(stage)
    for rectangle in collision_rectangles:
        for pit in pits:
            center_x = (rectangle[0] + rectangle[1]) * 0.5
            center_y = (rectangle[2] + rectangle[3]) * 0.5
            elevated_and_supported = point_on_static_platform(center_x, center_y, stage, margin=0.8)
            if rectangle_intersects_pit(rectangle, pit, margin=0.2) and not elevated_and_supported:
                conflicts.append("XFileListPhysics.csv:" + str(rectangle[4]))
                break

    if not has_safe_route(stage, collision_rectangles):
        conflicts.append("no safe route from start to goal")

    warnings = []
    platforms = stage.get("static_platforms", ())
    if len(platforms) >= 2:
        platform_xs = [platform[0] for platform in platforms]
        spread = max(platform_xs) - min(platform_xs)
        half_width = stage["size"][0]
        if spread < half_width * 0.5:
            warnings.append(
                "sections are concentrated in X "
                "(spread %.1fm of %.1fm half-width); place sections across more of the X range"
                % (spread, half_width)
            )

    return conflicts, warnings


def load_collision_rectangles(stage):
    rectangles = []
    physics_path = MODEL_DIR / stage["folder"] / "XFileListPhysics.csv"
    with physics_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row_index, row in enumerate(csv.DictReader(file), start=2):
            filename = row.get("FileName", "").lower()
            if row.get("Move", "").lower() == "y":
                continue
            half_x = None
            half_y = None
            is_climbable = False
            if "collision_wall" in filename:
                rotation = int(float(row.get("RotY", "0"))) % 180
                if rotation == 0:
                    half_x = 0.9
                    half_y = 4.4
                else:
                    half_x = 4.4
                    half_y = 0.9
            elif "cube_wood" in filename:
                half_x = 1.25
                half_y = 1.25
                is_climbable = True
            elif "tree_cylinder" in filename:
                half_x = 1.2
                half_y = 1.2
            if half_x is None:
                continue
            center_x = float(row["PosX"])
            center_y = float(row["PosZ"])
            rectangles.append((
                center_x - half_x,
                center_x + half_x,
                center_y - half_y,
                center_y + half_y,
                row_index,
                is_climbable,
                float(row.get("PosY", "0")),
            ))
    return rectangles


def rectangle_intersects_pit(rectangle, pit, margin=0.0):
    if rectangle[1] <= pit[0] - margin or rectangle[0] >= pit[1] + margin:
        return False
    if rectangle[3] <= pit[2] - margin or rectangle[2] >= pit[3] + margin:
        return False
    return True


def load_lava_zones(stage):
    zones = []
    stage_dir = MODEL_DIR / stage["folder"]
    lava_path = stage_dir / "LavaZones.csv"
    physics_path = stage_dir / "XFileListPhysics.csv"
    if not lava_path.exists() or not physics_path.exists():
        return zones

    plate_bounds = {}
    with physics_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row in csv.DictReader(file):
            filename = row.get("FileName", "").lower()
            if "platelava" not in filename:
                continue
            csv_id = int(row["ID"])
            scale = float(row["Scale"])
            plate_bounds[csv_id] = (float(row["PosX"]), float(row["PosZ"]), 4.0 * scale)

    with lava_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row in csv.DictReader(file):
            physics_id = int(row["PhysicsID"])
            if physics_id in plate_bounds:
                x, z, half_size = plate_bounds[physics_id]
                zones.append((x, z, half_size))
    return zones


def load_moving_platform_sweeps(stage):
    sweeps = []
    move_path = MODEL_DIR / stage["folder"] / "XFileListMove.csv"
    if not move_path.exists():
        return sweeps

    with move_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row in csv.DictReader(file):
            scale = float(row["Scale"])
            half_size = 1.5 * scale
            start_x = float(row["StartX"])
            start_z = float(row["StartZ"])
            end_x = float(row["EndX"])
            end_z = float(row["EndZ"])
            sweeps.append((
                min(start_x, end_x) - half_size,
                max(start_x, end_x) + half_size,
                min(start_z, end_z) - half_size,
                max(start_z, end_z) + half_size,
            ))
    return sweeps


def load_static_platform_footprints(stage):
    footprints = []
    physics_path = MODEL_DIR / stage["folder"] / "XFileListPhysics.csv"
    if not physics_path.exists():
        return footprints

    with physics_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row in csv.DictReader(file):
            filename = row.get("FileName", "").lower()
            if "collision_moving_platform" not in filename:
                continue
            if row.get("Move", "").lower() == "y":
                continue
            scale = float(row["Scale"])
            half_size = 1.5 * scale
            center_x = float(row["PosX"])
            center_z = float(row["PosZ"])
            footprints.append((
                center_x - half_size,
                center_x + half_size,
                center_z - half_size,
                center_z + half_size,
            ))
    return footprints


def has_safe_route(stage, rectangles):
    half_width, half_depth = stage["size"]
    pits = stage["pits"]
    lava_zones = load_lava_zones(stage)
    moving_platform_sweeps = load_moving_platform_sweeps(stage)
    static_platform_footprints = load_static_platform_footprints(stage)
    jump_links = {}
    for jump_link in stage.get("jump_links", ()):
        first = jump_link[0]
        second = jump_link[1]
        first_key = (round(first[0], 3), round(first[1], 3))
        second_key = (round(second[0], 3), round(second[1], 3))
        jump_links.setdefault(first_key, []).append(second)
        jump_links.setdefault(second_key, []).append(first)
    booster_links = {}
    for booster_link in stage.get("booster_links", ()):
        first = booster_link[0]
        second = booster_link[1]
        first_key = (round(first[0], 3), round(first[1], 3))
        booster_links.setdefault(first_key, []).append(second)
    grid_step = 1.0
    player_margin = 0.45

    def supported_by_moving_platform(x, y):
        for sweep in moving_platform_sweeps:
            if x >= sweep[0] - player_margin and x <= sweep[1] + player_margin:
                if y >= sweep[2] - player_margin and y <= sweep[3] + player_margin:
                    return True
        return False

    def supported_by_static_platform(x, y):
        if point_on_static_platform(x, y, stage, margin=-player_margin):
            return True
        for footprint in static_platform_footprints:
            if x >= footprint[0] + player_margin and x <= footprint[1] - player_margin:
                if y >= footprint[2] + player_margin and y <= footprint[3] - player_margin:
                    return True
        return False

    def blocked(x, y):
        if x < -half_width + player_margin or x > half_width - player_margin:
            return True
        if y < -half_depth + player_margin or y > half_depth - player_margin:
            return True
        if not point_on_stage_ground(x, y, stage, margin=player_margin):
            if not supported_by_moving_platform(x, y) and not supported_by_static_platform(x, y):
                return True
        for zone_x, zone_y, radius in lava_zones:
            delta_x = x - zone_x
            delta_y = y - zone_y
            safe_radius = radius + player_margin
            if delta_x * delta_x + delta_y * delta_y <= safe_radius * safe_radius:
                if not supported_by_static_platform(x, y):
                    return True
        for rectangle in rectangles:
            if rectangle[5]:
                continue
            if x >= rectangle[0] - player_margin and x <= rectangle[1] + player_margin:
                if y >= rectangle[2] - player_margin and y <= rectangle[3] + player_margin:
                    if stage.get("elevated_route", False) and supported_by_static_platform(x, y):
                        continue
                    return True
        return False

    start = stage["start"]
    goal = stage["goal"]
    if blocked(start[0], start[1]) or blocked(goal[0], goal[1]):
        return False

    queue = deque((start,))
    visited = {(round(start[0], 3), round(start[1], 3))}
    directions = ((grid_step, 0.0), (-grid_step, 0.0), (0.0, grid_step), (0.0, -grid_step))
    while queue:
        x, y = queue.popleft()
        if abs(x - goal[0]) <= grid_step * 0.5 and abs(y - goal[1]) <= grid_step * 0.5:
            return True
        for delta_x, delta_y in directions:
            next_x = x + delta_x
            next_y = y + delta_y
            key = (round(next_x, 3), round(next_y, 3))
            if key in visited:
                continue
            if blocked(next_x, next_y):
                continue
            visited.add(key)
            queue.append((next_x, next_y))
        current_key = (round(x, 3), round(y, 3))
        for destination in jump_links.get(current_key, ()):
            next_x = destination[0]
            next_y = destination[1]
            key = (round(next_x, 3), round(next_y, 3))
            if key in visited:
                continue
            if blocked(next_x, next_y):
                continue
            visited.add(key)
            queue.append((next_x, next_y))
        for destination in booster_links.get(current_key, ()):
            next_x = destination[0]
            next_y = destination[1]
            key = (round(next_x, 3), round(next_y, 3))
            if key in visited:
                continue
            if blocked(next_x, next_y):
                continue
            visited.add(key)
            queue.append((next_x, next_y))
    return False

def add_quad(vertices, faces, uvs, material_indices, coordinates, quad_uvs, material_index):
    start = len(vertices)
    vertices.extend(coordinates)
    uvs.extend(quad_uvs)
    faces.append((start, start + 1, start + 2, start + 3))
    material_indices.append(material_index)


def create_visual_ground_extension(world, top_material, play_half_size):
    play_half_width, play_half_depth = play_half_size
    outer_half_size = VISUAL_GROUND_HALF_SIZE
    vertices = []
    faces = []
    uvs = []
    material_indices = []

    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((-outer_half_size, play_half_depth, 0.0),
         (outer_half_size, play_half_depth, 0.0),
         (outer_half_size, outer_half_size, 0.0),
         (-outer_half_size, outer_half_size, 0.0)),
        ((-outer_half_size / 8.0, play_half_depth / 8.0),
         (outer_half_size / 8.0, play_half_depth / 8.0),
         (outer_half_size / 8.0, outer_half_size / 8.0),
         (-outer_half_size / 8.0, outer_half_size / 8.0)),
        0,
    )
    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((-outer_half_size, -outer_half_size, 0.0),
         (outer_half_size, -outer_half_size, 0.0),
         (outer_half_size, -play_half_depth, 0.0),
         (-outer_half_size, -play_half_depth, 0.0)),
        ((-outer_half_size / 8.0, -outer_half_size / 8.0),
         (outer_half_size / 8.0, -outer_half_size / 8.0),
         (outer_half_size / 8.0, -play_half_depth / 8.0),
         (-outer_half_size / 8.0, -play_half_depth / 8.0)),
        0,
    )
    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((-outer_half_size, -play_half_depth, 0.0),
         (-play_half_width, -play_half_depth, 0.0),
         (-play_half_width, play_half_depth, 0.0),
         (-outer_half_size, play_half_depth, 0.0)),
        ((-outer_half_size / 8.0, -play_half_depth / 8.0),
         (-play_half_width / 8.0, -play_half_depth / 8.0),
         (-play_half_width / 8.0, play_half_depth / 8.0),
         (-outer_half_size / 8.0, play_half_depth / 8.0)),
        0,
    )
    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((play_half_width, -play_half_depth, 0.0),
         (outer_half_size, -play_half_depth, 0.0),
         (outer_half_size, play_half_depth, 0.0),
         (play_half_width, play_half_depth, 0.0)),
        ((play_half_width / 8.0, -play_half_depth / 8.0),
         (outer_half_size / 8.0, -play_half_depth / 8.0),
         (outer_half_size / 8.0, play_half_depth / 8.0),
         (play_half_width / 8.0, play_half_depth / 8.0)),
        0,
    )

    object_name = "StageVisualGroundWorld" + str(world)
    mesh = bpy.data.meshes.new(object_name + "Geo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(top_material)

    for polygon_index, polygon in enumerate(mesh.polygons):
        polygon.use_smooth = False
        polygon.material_index = material_indices[polygon_index]

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]

    obj = bpy.data.objects.new(object_name, mesh)
    bpy.context.collection.objects.link(obj)
    obj["_x_frame_name"] = object_name
    obj["_x_mesh_name"] = object_name + "Geo"
    return obj

def add_triangle(vertices, faces, uvs, material_indices, coordinates, triangle_uvs, material_index):
    start = len(vertices)
    vertices.extend(coordinates)
    uvs.extend(triangle_uvs)
    faces.append((start, start + 1, start + 2))
    material_indices.append(material_index)


def create_spiral_stage_ground(stage, top_material, side_material):
    center_x, center_y = stage.get("spiral_center", (0.0, 0.0))
    start_radius = stage["spiral_start_radius"]
    end_radius = stage["spiral_end_radius"]
    start_angle = stage["spiral_start_angle"]
    turns = stage["spiral_turns"]
    strip_width = stage["spiral_width"]
    sample_count = int(stage["spiral_samples"])
    if sample_count < 8 or turns <= 0.0 or strip_width <= 0.0:
        raise RuntimeError(stage["display"] + " has invalid spiral geometry settings")

    vertices = []
    faces = []
    uvs = []
    material_indices = []
    centerline = []
    total_angle = turns * math.pi * 2.0
    radius_per_angle = (end_radius - start_radius) / total_angle

    for sample_index in range(sample_count):
        progress = sample_index / float(sample_count - 1)
        angle = start_angle + total_angle * progress
        radius = start_radius + (end_radius - start_radius) * progress
        position_x = center_x + radius * math.cos(angle)
        position_y = center_y + radius * math.sin(angle)
        derivative_x = radius_per_angle * math.cos(angle) - radius * math.sin(angle)
        derivative_y = radius_per_angle * math.sin(angle) + radius * math.cos(angle)
        derivative_length = math.sqrt(derivative_x * derivative_x + derivative_y * derivative_y)
        if derivative_length <= 0.0001:
            raise RuntimeError(stage["display"] + " has a zero-length spiral tangent")
        tangent_x = derivative_x / derivative_length
        tangent_y = derivative_y / derivative_length
        normal_x = -tangent_y
        normal_y = tangent_x
        half_width = strip_width * 0.5
        left = (position_x + normal_x * half_width, position_y + normal_y * half_width)
        right = (position_x - normal_x * half_width, position_y - normal_y * half_width)
        centerline.append((left, right))

    for sample_index in range(sample_count - 1):
        left_a, right_a = centerline[sample_index]
        left_b, right_b = centerline[sample_index + 1]
        distance_u = sample_index / 8.0
        segment_length = math.sqrt(
            (left_b[0] - left_a[0]) * (left_b[0] - left_a[0])
            + (left_b[1] - left_a[1]) * (left_b[1] - left_a[1])
        )
        add_quad(
            vertices,
            faces,
            uvs,
            material_indices,
            ((left_a[0], left_a[1], 0.0),
             (right_a[0], right_a[1], 0.0),
             (right_b[0], right_b[1], 0.0),
             (left_b[0], left_b[1], 0.0)),
            ((distance_u, 0.0),
             (distance_u, 0.7),
             (distance_u + segment_length / 8.0, 0.7),
             (distance_u + segment_length / 8.0, 0.0)),
            0,
        )
        add_quad(
            vertices,
            faces,
            uvs,
            material_indices,
            ((left_a[0], left_a[1], 0.0),
             (left_b[0], left_b[1], 0.0),
             (left_b[0], left_b[1], SLAB_BOTTOM),
             (left_a[0], left_a[1], SLAB_BOTTOM)),
            ((0.0, 0.0),
             (segment_length / 4.0, 0.0),
             (segment_length / 4.0, 2.25),
             (0.0, 2.25)),
            1,
        )
        add_quad(
            vertices,
            faces,
            uvs,
            material_indices,
            ((right_b[0], right_b[1], 0.0),
             (right_a[0], right_a[1], 0.0),
             (right_a[0], right_a[1], SLAB_BOTTOM),
             (right_b[0], right_b[1], SLAB_BOTTOM)),
            ((0.0, 0.0),
             (segment_length / 4.0, 0.0),
             (segment_length / 4.0, 2.25),
             (0.0, 2.25)),
            1,
        )

    first_left, first_right = centerline[0]
    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((first_right[0], first_right[1], 0.0),
         (first_left[0], first_left[1], 0.0),
         (first_left[0], first_left[1], SLAB_BOTTOM),
         (first_right[0], first_right[1], SLAB_BOTTOM)),
        ((0.0, 0.0), (0.7, 0.0), (0.7, 2.25), (0.0, 2.25)),
        1,
    )

    core_radius = stage["spiral_core_radius"]
    core_segments = 48
    for segment_index in range(core_segments):
        angle_a = math.pi * 2.0 * segment_index / float(core_segments)
        angle_b = math.pi * 2.0 * (segment_index + 1) / float(core_segments)
        core_a = (center_x + core_radius * math.cos(angle_a), center_y + core_radius * math.sin(angle_a))
        core_b = (center_x + core_radius * math.cos(angle_b), center_y + core_radius * math.sin(angle_b))
        add_triangle(
            vertices,
            faces,
            uvs,
            material_indices,
            ((center_x, center_y, 0.0),
             (core_a[0], core_a[1], 0.0),
             (core_b[0], core_b[1], 0.0)),
            ((center_x / 8.0, center_y / 8.0),
             (core_a[0] / 8.0, core_a[1] / 8.0),
             (core_b[0] / 8.0, core_b[1] / 8.0)),
            0,
        )
        add_quad(
            vertices,
            faces,
            uvs,
            material_indices,
            ((core_a[0], core_a[1], 0.0),
             (core_b[0], core_b[1], 0.0),
             (core_b[0], core_b[1], SLAB_BOTTOM),
             (core_a[0], core_a[1], SLAB_BOTTOM)),
            ((0.0, 0.0), (0.7, 0.0), (0.7, 2.25), (0.0, 2.25)),
            1,
        )

    object_name = "Stage" + stage["display"].replace("-", "_") + "Ground"
    mesh = bpy.data.meshes.new(object_name + "Geo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(top_material)
    mesh.materials.append(side_material)

    for polygon_index, polygon in enumerate(mesh.polygons):
        polygon.use_smooth = False
        polygon.material_index = material_indices[polygon_index]

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]

    obj = bpy.data.objects.new(object_name, mesh)
    bpy.context.collection.objects.link(obj)
    obj["_x_frame_name"] = object_name
    obj["_x_mesh_name"] = object_name + "Geo"
    return obj


def create_stage_ground(stage, top_material, side_material):
    if stage.get("ground_shape") == "spiral":
        return create_spiral_stage_ground(stage, top_material, side_material)

    half_width, half_depth = stage["size"]
    pits = stage["pits"]
    vertices = []
    faces = []
    uvs = []
    material_indices = []

    x_values = {-half_width, half_width}
    y_values = {-half_depth, half_depth}
    for pit in pits:
        x_values.add(pit[0])
        x_values.add(pit[1])
        y_values.add(pit[2])
        y_values.add(pit[3])
    x_values = sorted(x_values)
    y_values = sorted(y_values)

    for x_index in range(len(x_values) - 1):
        for y_index in range(len(y_values) - 1):
            x_min = x_values[x_index]
            x_max = x_values[x_index + 1]
            y_min = y_values[y_index]
            y_max = y_values[y_index + 1]
            center_x = (x_min + x_max) * 0.5
            center_y = (y_min + y_max) * 0.5
            if point_in_pit(center_x, center_y, pits):
                continue
            add_quad(
                vertices,
                faces,
                uvs,
                material_indices,
                ((x_min, y_min, 0.0), (x_max, y_min, 0.0), (x_max, y_max, 0.0), (x_min, y_max, 0.0)),
                ((x_min / 8.0, y_min / 8.0), (x_max / 8.0, y_min / 8.0), (x_max / 8.0, y_max / 8.0), (x_min / 8.0, y_max / 8.0)),
                0,
            )

    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((-half_width, -half_depth, SLAB_BOTTOM), (-half_width, half_depth, SLAB_BOTTOM), (half_width, half_depth, SLAB_BOTTOM), (half_width, -half_depth, SLAB_BOTTOM)),
        ((0.0, 0.0), (0.0, half_depth / 8.0), (half_width / 8.0, half_depth / 8.0), (half_width / 8.0, 0.0)),
        1,
    )

    add_quad(vertices, faces, uvs, material_indices,
             ((-half_width, -half_depth, 0.0), (-half_width, -half_depth, SLAB_BOTTOM), (half_width, -half_depth, SLAB_BOTTOM), (half_width, -half_depth, 0.0)),
             ((0.0, 0.0), (0.0, 2.5), (half_width / 4.0, 2.5), (half_width / 4.0, 0.0)), 1)
    add_quad(vertices, faces, uvs, material_indices,
             ((half_width, half_depth, 0.0), (half_width, half_depth, SLAB_BOTTOM), (-half_width, half_depth, SLAB_BOTTOM), (-half_width, half_depth, 0.0)),
             ((0.0, 0.0), (0.0, 2.5), (half_width / 4.0, 2.5), (half_width / 4.0, 0.0)), 1)
    add_quad(vertices, faces, uvs, material_indices,
             ((-half_width, half_depth, 0.0), (-half_width, half_depth, SLAB_BOTTOM), (-half_width, -half_depth, SLAB_BOTTOM), (-half_width, -half_depth, 0.0)),
             ((0.0, 0.0), (0.0, 2.5), (half_depth / 4.0, 2.5), (half_depth / 4.0, 0.0)), 1)
    add_quad(vertices, faces, uvs, material_indices,
             ((half_width, -half_depth, 0.0), (half_width, -half_depth, SLAB_BOTTOM), (half_width, half_depth, SLAB_BOTTOM), (half_width, half_depth, 0.0)),
             ((0.0, 0.0), (0.0, 2.5), (half_depth / 4.0, 2.5), (half_depth / 4.0, 0.0)), 1)

    # Pit walls and bottoms use the same world texture as the ground surface.
    for pit in pits:
        x_min, x_max, y_min, y_max = pit
        # 外周に接する辺は既存の外周壁に任せ、同一面の穴壁を重ねない。
        if x_min > -half_width:
            add_quad(vertices, faces, uvs, material_indices,
                     ((x_min, y_min, 0.0), (x_min, y_min, PIT_BOTTOM), (x_min, y_max, PIT_BOTTOM), (x_min, y_max, 0.0)),
                     ((0.0, 0.0), (0.0, 2.25), ((y_max - y_min) / 4.0, 2.25), ((y_max - y_min) / 4.0, 0.0)), 0)
        if x_max < half_width:
            add_quad(vertices, faces, uvs, material_indices,
                     ((x_max, y_max, 0.0), (x_max, y_max, PIT_BOTTOM), (x_max, y_min, PIT_BOTTOM), (x_max, y_min, 0.0)),
                     ((0.0, 0.0), (0.0, 2.25), ((y_max - y_min) / 4.0, 2.25), ((y_max - y_min) / 4.0, 0.0)), 0)
        if y_min > -half_depth:
            add_quad(vertices, faces, uvs, material_indices,
                     ((x_max, y_min, 0.0), (x_max, y_min, PIT_BOTTOM), (x_min, y_min, PIT_BOTTOM), (x_min, y_min, 0.0)),
                     ((0.0, 0.0), (0.0, 2.25), ((x_max - x_min) / 4.0, 2.25), ((x_max - x_min) / 4.0, 0.0)), 0)
        if y_max < half_depth:
            add_quad(vertices, faces, uvs, material_indices,
                     ((x_min, y_max, 0.0), (x_min, y_max, PIT_BOTTOM), (x_max, y_max, PIT_BOTTOM), (x_max, y_max, 0.0)),
                     ((0.0, 0.0), (0.0, 2.25), ((x_max - x_min) / 4.0, 2.25), ((x_max - x_min) / 4.0, 0.0)), 0)
        add_quad(vertices, faces, uvs, material_indices,
                 ((x_min, y_min, PIT_BOTTOM), (x_max, y_min, PIT_BOTTOM), (x_max, y_max, PIT_BOTTOM), (x_min, y_max, PIT_BOTTOM)),
                 ((0.0, 0.0), ((x_max - x_min) / 4.0, 0.0), ((x_max - x_min) / 4.0, (y_max - y_min) / 4.0), (0.0, (y_max - y_min) / 4.0)), 0)

    object_name = "Stage" + stage["display"].replace("-", "_") + "Ground"
    mesh = bpy.data.meshes.new(object_name + "Geo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(top_material)
    mesh.materials.append(side_material)

    for polygon_index, polygon in enumerate(mesh.polygons):
        polygon.use_smooth = False
        polygon.material_index = material_indices[polygon_index]

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]

    obj = bpy.data.objects.new(object_name, mesh)
    bpy.context.collection.objects.link(obj)
    obj["_x_frame_name"] = object_name
    obj["_x_mesh_name"] = object_name + "Geo"
    return obj


def export_object(obj, output_path):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    result = bpy.ops.export_scene.directx_x(
        filepath=str(output_path),
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        export_format="TEXT_X",
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(output_path))


def main():
    if len(STAGES) != 32:
        raise RuntimeError("Exactly 32 stage ground definitions are required")

    folders = {stage["folder"] for stage in STAGES}
    if len(folders) != 32:
        raise RuntimeError("Stage ground folder names must be unique")

    selected_display = os.environ.get("RED_FORTRESS_STAGE_GROUND", "").strip()
    stages_to_build = tuple(stage for stage in STAGES if not stage.get("dedicated_ground", False))
    if selected_display != "":
        stages_to_build = tuple(stage for stage in STAGES if stage["display"] == selected_display)
        if len(stages_to_build) != 1:
            raise RuntimeError("Unknown stage ground selection: " + selected_display)
        if stages_to_build[0].get("dedicated_ground", False):
            raise RuntimeError("Stage " + selected_display + " uses tools/BuildStage21Ground.py")

    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    clear_scene()
    side_material = create_material(
        "StageGroundSide",
        SIDE_TEXTURE_PATH,
        "../whiteWall.png",
        (0.42, 0.42, 0.42, 1.0),
    )

    # One top-surface material per world (texture differs per world).
    top_materials = {}
    for world, texture_path in WORLD_TOP_TEXTURES.items():
        if not texture_path.exists():
            raise RuntimeError("Missing world top texture: " + str(texture_path))
        relative_name = ".." / texture_path.relative_to(MODEL_DIR)
        top_materials[world] = create_material(
            "StageGroundTopWorld" + str(world),
            texture_path,
            str(relative_name).replace("\\", "/"),
            (0.64, 0.64, 0.64, 1.0),
        )

    visual_ground_objects = []
    world_play_half_sizes = {
        1: (16.0, 32.0),
        2: (60.0, 60.0),
        3: (60.0, 120.0),
        4: (60.0, 120.0),
    }
    for world, play_half_size in world_play_half_sizes.items():
        visual_ground = create_visual_ground_extension(
            world,
            top_materials[world],
            play_half_size,
        )
        visual_ground_objects.append((world, visual_ground))
    validation_errors = []
    for stage in stages_to_build:
        conflicts, warnings = validate_stage(stage)
        for warning in warnings:
            print("WARNING", stage["display"] + ": " + warning)
        if conflicts:
            validation_errors.append(stage["display"] + ": " + ", ".join(conflicts))
    if validation_errors:
        raise RuntimeError("Stage ground conflicts:\n" + "\n".join(validation_errors))

    objects = []
    for stage in stages_to_build:
        world = world_for_folder(stage["folder"])
        obj = create_stage_ground(stage, top_materials[world], side_material)
        objects.append((stage, obj))

    if selected_display == "":
        bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    for stage, obj in objects:
        output_path = MODEL_DIR / stage["folder"] / "stage_ground.x"
        export_object(obj, output_path)
        print("EXPORTED", stage["display"], output_path)

    if selected_display == "":
        for world, obj in visual_ground_objects:
            output_path = GROUND_DIR / ("stage_visual_ground_world" + str(world) + ".x")
            export_object(obj, output_path)
            print("EXPORTED_VISUAL", world, output_path)
        print("BLEND_PATH", BLEND_PATH)
    print("EXPORTED_STAGE_GROUNDS", len(objects))


main()
