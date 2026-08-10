# -*- coding: utf-8 -*-
"""ステージ2-6「あつあつ飛び石ロード」のCSVを一括生成する。"""

import csv
import io
import math
from pathlib import Path


BASE = Path(__file__).resolve().parents[1] / "res" / "model" / "stage_2_6"

SIMPLE_HEADER = ["ID", "FileName", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "loadType"]
PHYSICS_HEADER = ["ID", "FileName", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "Type", "Move", "Instancing"]
MOVE_HEADER = ["ID", "RenderID", "PhysicsID", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale",
               "StartX", "StartY", "StartZ", "EndX", "EndY", "EndZ", "Duration"]


def write_csv(filename, rows):
    buffer = io.StringIO()
    writer = csv.writer(buffer, lineterminator="\r\n")
    writer.writerows(rows)
    path = BASE / filename
    with path.open("w", encoding="utf-8-sig", newline="") as output:
        output.write(buffer.getvalue())
    print("WROTE", path.relative_to(BASE.parent.parent.parent), len(rows) - 1, "rows")


def add_pair(simple, physics, csv_id, render_path, physics_path, x, y, z,
             rotation_y=0, scale=1, load_type="normal", move="n", physics_type="Collision"):
    simple.append([csv_id, render_path, x, y, z, 0, rotation_y, 0, scale, load_type])
    physics.append([csv_id, physics_path, x, y, z, 0, rotation_y, 0, scale, physics_type, move, ""])


def add_static(simple, physics, csv_id, model, x, y, z):
    add_pair(simple, physics, csv_id,
             "../static_platform/" + model + ".x",
             "res/model/static_platform/" + model + "_collision.x",
             x, y, z)


def build_render_and_physics():
    simple = [SIMPLE_HEADER]
    physics = [PHYSICS_HEADER]

    simple.append([1, "../ground/stage_visual_ground_world2.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"])
    simple.append([2, "stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"])
    physics.append([1, "res/model/cubeNormalInverse120x120.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""])
    physics.append([2, "res/model/stage_2_6/stage_ground.x", 0, 0.01, 0, 0, 0, 0, 1, "Collision", "n", ""])

    # 120x120mを20m四方の溶岩36枚で隙間なく覆う。
    lava_ids = []
    lava_id = 4001
    for z in (-50, -30, -10, 10, 30, 50):
        for x in (-50, -30, -10, 10, 30, 50):
            simple.append([lava_id, "../plateLava.x", x, 0.02, z, 0, 0, 0, 2.5, "meshmix2"])
            physics.append([lava_id, "res/model/plateLava.x", x, 0.02, z, 0, 0, 0, 2.5, "NonCollision", "n", ""])
            lava_ids.append(lava_id)
            lava_id += 1

    platform_id = 5001

    # 4つの24x24m正方形陸地。陸地BだけY=3.2mの高台。
    lands = (
        ((-52, 0.35, -48), (-40, 0.35, -48), (-52, 0.35, -36), (-40, 0.35, -36)),
        ((-31, 3.2, -1), (-19, 3.2, -1), (-31, 3.2, 11), (-19, 3.2, 11)),
        ((4, 0.35, -44), (16, 0.35, -44), (4, 0.35, -32), (16, 0.35, -32)),
        ((39, 0.35, 34), (51, 0.35, 34), (39, 0.35, 46), (51, 0.35, 46)),
    )
    for land in lands:
        for x, y, z in land:
            add_static(simple, physics, platform_id, "static_platform_4x4", x, y, z)
            platform_id += 1

    # 陸地Aから陸地Bへ北上する上り石。
    main_platforms = (
        ("static_platform_1x1", -42, 0.8, -25.5),
        ("static_platform_2x2", -39, 1.2, -19),
        ("static_platform_2x2", -35, 2.0, -12.5),
        ("static_platform_2x2", -30, 2.8, -6),
        # 陸地Bから陸地Cへ南下する下り石。
        ("static_platform_2x2", -15, 2.4, -12),
        ("static_platform_2x2", -9, 1.6, -19),
        ("static_platform_2x2", -3, 0.8, -26),
        # レバー3門の先にある移動床待機場所。
        ("static_platform_1x2", 30, 0.35, -23),
        # 移動床から隣へ飛び移る東の高台。
        ("static_platform_2x2", 38, 1.9, -13),
        # 東から北西へ折り返してY=3.2mへ登る。
        ("static_platform_2x2", 32, 2.3, -6),
        ("static_platform_2x2", 24, 2.7, 1),
        ("static_platform_4x4", 17, 3.2, 7),
        # 北東の陸地Dへ下る。
        ("static_platform_1x1", 20, 2.4, 16),
        ("static_platform_2x2", 26, 1.6, 22),
        ("static_platform_2x2", 34, 0.8, 28),
    )
    for model, x, y, z in main_platforms:
        add_static(simple, physics, platform_id, model, x, y, z)
        platform_id += 1

    # 分岐1: 陸地Aから西端のQTE木へ。
    qte_branch = ((-52, 0.35, -25), (-54, 0.35, -19))
    for x, y, z in qte_branch:
        add_static(simple, physics, platform_id, "static_platform_1x1", x, y, z)
        platform_id += 1
    add_static(simple, physics, platform_id, "static_platform_4x4", -54, 0.35, -10)
    platform_id += 1

    # 分岐2: 北端の報酬。陸地Bと同じ高さを保つ。
    north_branch = ((-27, 3.2, 23), (-28, 3.2, 29), (-28, 3.2, 35), (-28, 3.2, 41))
    for x, y, z in north_branch:
        add_static(simple, physics, platform_id, "static_platform_1x2", x, y, z)
        platform_id += 1
    add_static(simple, physics, platform_id, "static_platform_4x4", -28, 3.2, 51)
    platform_id += 1

    # 分岐3: 陸地Cから南端のLever2へ登る階段。
    south_high_branch = ((-5, 0.8, -51), (-10, 1.6, -52), (-15, 2.4, -53))
    for x, y, z in south_high_branch:
        add_static(simple, physics, platform_id, "static_platform_2x2", x, y, z)
        platform_id += 1
    add_static(simple, physics, platform_id, "static_platform_4x4", -22, 3.2, -54)
    platform_id += 1

    # 分岐4: 区画9から西端へ延びる長い高所分岐。
    west_branch = ((8, 12), (0, 16), (-8, 19), (-16, 22), (-24, 24), (-32, 26), (-40, 27))
    for x, z in west_branch:
        add_static(simple, physics, platform_id, "static_platform_2x2", x, 3.2, z)
        platform_id += 1
    add_static(simple, physics, platform_id, "static_platform_4x4", -52, 3.2, 27)
    platform_id += 1

    # 区画7→8の斜め昇降床。上端で高台と重ならず、0.5mの隙間を空ける。
    add_pair(simple, physics, 6001, "../collision_moving_platform/collision_moving_platform.x",
             "res/model/collision_moving_platform.x", 34, 0.65, -22,
             scale=1, load_type="meshmix2", move="y")

    # QTE木。
    add_pair(simple, physics, 7001, "../tree2/lemonTree.x",
             "res/model/tree2Physics/tree_cylinder_collision.x", -54, 0.85, -10)

    # 南端高台のレバー2報酬箱。
    add_pair(simple, physics, 9001, "../attack_trigger/lever_box_floor.x",
             "res/model/attack_trigger/lever_box_floor.x", -22, 3.7, -54)
    add_pair(simple, physics, 9002, "../attack_trigger/lever_box.x",
             "res/model/attack_trigger/lever_box.x", -22, 3.8, -54)
    add_pair(simple, physics, 9003, "../attack_trigger/lever_box_door.x",
             "res/model/attack_trigger/lever_box_door.x", -22, 3.8, -57,
             scale=0.98, move="y")

    # 陸地Cから東側へ出る唯一のレバー3門。開口部をX方向へ向ける。
    add_pair(simple, physics, 9101, "../attack_trigger/lever_box3_floor.x",
             "res/model/attack_trigger/lever_box3_floor.x", 27, 0.85, -27,
             rotation_y=90)
    add_pair(simple, physics, 9102, "../attack_trigger/lever_box3.x",
             "res/model/attack_trigger/lever_box3.x", 27, 0.95, -27,
             rotation_y=90)
    add_pair(simple, physics, 9103, "../attack_trigger/lever_box3_door.x",
             "res/model/attack_trigger/lever_box3_door.x", 27, 0.95, -27,
             rotation_y=90, scale=0.98, move="y")

    # World 2の外周景観は岩だけにする。
    rocks = ((-58, -32, 30, 0.9, 1), (-58, 8, 110, 1.1, 2), (-58, 44, 210, 0.9, 1),
             (58, -45, 170, 1.0, 2), (58, -10, 260, 0.9, 1), (58, 12, 330, 1.1, 2),
             (-5, -58, 70, 0.9, 2), (22, -58, 210, 1.0, 1), (8, 58, 130, 0.9, 2))
    rock_id = 7201
    for x, z, rotation_y, scale, variant in rocks:
        model = "../base/base_rock1.x"
        if variant == 2:
            model = "../base/base_rock2.x"
        simple.append([rock_id, model, x, 0, z, 0, rotation_y, 0, scale, "normal"])
        rock_id += 1

    simple.append([9200, "../SkySphere_cave/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 0, 1, "normal"])
    return simple, physics, lava_ids


def main():
    BASE.mkdir(parents=True, exist_ok=True)
    simple, physics, lava_ids = build_render_and_physics()

    move = [MOVE_HEADER,
            [1, 6001, 6001, 34, 0.65, -22, 0, 0, 0, 1,
             34, 0.65, -22, 33, 2.2, -14, 6.0]]

    enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
               ["small_spider", -45, 0.9, -34, 30], ["spider", -50, 0.9, -35, 180],
               ["small_golem", -38, 0.9, -40, 270],
               ["small_spider", -31, 3.75, 2, 45], ["spider", -22, 3.75, 1, 210],
               ["small_golem", -30, 3.75, 11, 120], ["small_spider", -20, 3.75, 10, 300],
               ["small_golem", 4, 0.9, -44, 30], ["small_spider", 11, 0.9, -45, 180],
               ["spider", 17, 0.9, -43, 270], ["small_spider", 5, 0.9, -34, 120],
               ["small_golem", 15, 0.9, -33, 300],
               ["small_golem", 37, 0.9, 32, 30], ["small_spider", 44, 0.9, 32, 180],
               ["spider", 51, 0.9, 33, 270], ["small_spider", 38, 0.9, 39, 120],
               ["small_golem", 45, 0.9, 39, 300],
               ["small_spider", -31, 3.75, 50, 45], ["spider", -25, 3.75, 52, 225],
               ["small_golem", -54, 3.75, 25, 60], ["small_spider", -49, 3.75, 29, 240]]

    collectibles = [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
                    ["stage26-I01", "Item", "006", -22, 4.35, -54, 1],
                    ["stage26-I02", "Item", "009", -28, 4.0, 51, 1],
                    ["stage26-I03", "Item", "010", -52, 4.0, 27, 1],
                    ["stage26-I04", "Item", "014", 17, 4.0, 7, 1],
                    ["stage26-I05", "Item", "016", 38, 2.7, -13, 1],
                    ["stage26-I06", "Item", "005", 55, 1.1, 47, 1]]

    destructibles = [["PosX", "PosY", "PosZ", "HP", "DropItemId"],
                     [-43, 1.05, -39, 2, "None"], [-36, 1.05, -35, 2, "014"],
                     [-28, 3.95, 6, 3, "None"], [8, 1.05, -39, 2, "016"],
                     [42, 1.05, 35, 3, "None"], [-50, 3.95, 27, 2, "010"]]

    lava = [["ID", "PhysicsID", "Damage"]]
    for index, physics_id in enumerate(lava_ids, start=1):
        lava.append(["stage26-lava-%02d" % index, physics_id, 20])

    direction_x = -25.0
    direction_z = 12.0
    direction_length = math.hypot(direction_x, direction_z)
    boosters = [["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ",
                 "Speed", "Duration", "Radius", "Scale", "ChargeEnabled"],
                ["stage26-booster-01", 10, 4.0, 10,
                 round(direction_x / direction_length, 4), 0.04, round(direction_z / direction_length, 4),
                 20, 1.0, 1.2, 0.6, "n"]]

    interactables = [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"],
                     ["stage26-tree-01", "Tree", -54, 0.85, -10, 2.5]]

    triggers = [["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis",
                 "BaseRotX", "BaseRotY", "BaseRotZ", "Scale", "LiftHeight"],
                [1, "LeverLift", -22, 4.4, -58.2, 9003, "Y", 0, 0, 0, 0.98, 6],
                [2, "LeverLift", 21.8, 1.55, -27, 9103, "Y", 0, 90, 0, 0.98, 6],
                [3, "LeverLift", 32.2, 1.55, -27, 9103, "Y", 0, 90, 0, 0.98, 6]]

    point_lights = [["PosX", "PosY", "PosZ", "Brightness", "ColorR", "ColorG", "ColorB", "ColorA",
                     "Shape", "LineLength", "SquareWidth", "SquareHeight", "RotX", "RotY", "RotZ", "Range", "OwnerTag"],
                    [-52, 3.0, -49, 1.0, 0.20, 0.55, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 11, "stage26-start"],
                    [-25, 7.0, 5, 0.9, 0.35, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 13, "stage26-high-land"],
                    [10, 3.0, -38, 0.9, 1.0, 0.30, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 13, "stage26-south-land"],
                    [27, 3.2, -27, 0.9, 0.70, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage26-gate"],
                    [38, 5.0, -13, 0.9, 0.20, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage26-lift"],
                    [17, 7.0, 7, 0.9, 0.25, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage26-turn"],
                    [-52, 7.0, 27, 0.8, 1.0, 0.30, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 11, "stage26-west-edge"],
                    [45, 3.0, 40, 1.0, 0.15, 0.70, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 13, "stage26-final"],
                    [52, 3.0, 49, 1.0, 0.15, 0.70, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 11, "stage26-goal"]]

    empty_files = {
        "PressurePlates.csv": [["ID", "PlatePosX", "PlatePosY", "PlatePosZ", "WallID", "WallRotX", "WallRotY", "WallRotZ", "WallScale", "TravelDistance"]],
        "PushableBoxes.csv": [["ID", "PosX", "PosY", "PosZ", "RotY", "Scale"]],
        "Skulls.csv": [["ID", "PosX", "PosY", "PosZ", "RotY"]],
        "Stars.csv": [["PosX", "PosY", "PosZ"]],
        "SpeedUps.csv": [["PosX", "PosY", "PosZ"]],
        "WarpBears.csv": [["WarpID", "PairID", "PosX", "PosY", "PosZ", "RotY"]],
    }

    write_csv("XFileList_simple.csv", simple)
    write_csv("XFileListPhysics.csv", physics)
    write_csv("XFileListMove.csv", move)
    write_csv("EnemyPositions.csv", enemies)
    write_csv("Collectibles.csv", collectibles)
    write_csv("Destructibles.csv", destructibles)
    write_csv("LavaZones.csv", lava)
    write_csv("DashBoosters.csv", boosters)
    write_csv("Interactables.csv", interactables)
    write_csv("AttackTriggers.csv", triggers)
    write_csv("PointLights.csv", point_lights)
    for filename, rows in empty_files.items():
        write_csv(filename, rows)


if __name__ == "__main__":
    main()
