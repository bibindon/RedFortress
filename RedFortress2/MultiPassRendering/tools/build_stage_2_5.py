# -*- coding: utf-8 -*-
"""ステージ2-5「チクチク床の飛び石」のCSVを一括生成する。"""

import csv
import io
import math
from pathlib import Path


BASE = Path(__file__).resolve().parents[1] / "res" / "model" / "stage_2_5"

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


def add_static_platform(simple, physics, csv_id, model, x, y, z):
    add_pair(simple, physics, csv_id,
             "../static_platform/" + model + ".x",
             "res/model/static_platform/" + model + "_collision.x",
             x, y, z)


def add_island(simple, physics, next_id, pieces):
    for model, x, y, z in pieces:
        add_static_platform(simple, physics, next_id, model, x, y, z)
        next_id += 1
    return next_id


def build_render_and_physics():
    simple = [SIMPLE_HEADER]
    physics = [PHYSICS_HEADER]

    simple.append([1, "../ground/stage_visual_ground_world2.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"])
    simple.append([2, "stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"])
    physics.append([1, "res/model/cubeNormalInverse120x120.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""])
    physics.append([2, "res/model/stage_2_5/stage_ground.x", 0, 0.01, 0, 0, 0, 0, 1, "Collision", "n", ""])

    # 20m四方のplateLavaを6x6枚、隙間なく並べて120x120m全体を覆う。
    lava_ids = []
    lava_id = 4001
    for z in (-50, -30, -10, 10, 30, 50):
        for x in (-50, -30, -10, 10, 30, 50):
            simple.append([lava_id, "../plateLava.x", x, 0.02, z, 0, 0, 0, 2.5, "meshmix2"])
            physics.append([lava_id, "res/model/plateLava.x", x, 0.02, z, 0, 0, 0, 2.5, "NonCollision", "n", ""])
            lava_ids.append(lava_id)
            lava_id += 1

    # 12区画。大区画は12x12mと6x6mを継ぎ目で接続して戦闘面積を確保する。
    platform_id = 5001
    islands = (
        (("static_platform_4x4", -50, 0.35, -50),),
        (("static_platform_4x4", -37, 0.35, -39),),
        (("static_platform_4x4", -18, 0.35, -43),
         ("static_platform_2x2", -9, 0.35, -46),
         ("static_platform_2x2", -9, 0.35, -40)),
        (("static_platform_4x4", 8, 0.35, -31),),
        (("static_platform_4x4", 31, 3.2, -44),),
        (("static_platform_4x4", 39, 0.35, -17),
         ("static_platform_2x2", 48, 0.35, -20),
         ("static_platform_2x2", 48, 0.35, -14)),
        (("static_platform_4x4", 32, 0.35, 4),),
        (("static_platform_4x4", 14, 0.35, 20),),
        (("static_platform_4x4", -11, 0.35, 17),
         ("static_platform_2x2", -2, 0.35, 14),
         ("static_platform_2x2", -2, 0.35, 20)),
        (),
        (("static_platform_4x4", -42, 0.35, 40),
         ("static_platform_2x2", -33, 0.35, 40)),
        (("static_platform_4x4", -51, 0.35, 53),),
    )
    for pieces in islands:
        platform_id = add_island(simple, physics, platform_id, pieces)

    # 通常ジャンプ用の固定飛び石。全て3x3mで、縁から縁の間隔を4.1m以下にする。
    stones = (
        (-27.5, 0.35, -41),
        (-3, 0.35, -36), (0, 0.35, -36),
        (18, 0.35, -27), (23, 0.35, -23), (28, 0.35, -20),
        (35, 0.35, -6.5),
        (23.5, 0.35, 11.5),
        (4, 0.35, 20),
        (-29.5, 0.35, 33),
        (0, 0.35, 28.5), (3, 0.35, 33.5), (6, 0.35, 38.5), (8, 0.35, 44),
        (49, 0.35, -7), (52, 0.35, -4.5),
    )
    for x, y, z in stones:
        add_static_platform(simple, physics, platform_id, "static_platform_1x1", x, y, z)
        platform_id += 1

    # 区画5への往復階段。0.8mずつ上がり、終端の足場はY=3.2m。
    staircase = (
        (17, 0.8, -35),
        (21.5, 1.6, -38),
        (26, 2.4, -41),
    )
    for x, y, z in staircase:
        add_static_platform(simple, physics, platform_id, "static_platform_2x2", x, y, z)
        platform_id += 1

    # 東端の報酬足場と北端のQTE木足場。どちらも行き止まりで同じ道を戻る。
    add_static_platform(simple, physics, platform_id, "static_platform_2x2", 54, 0.35, -1)
    platform_id += 1
    add_static_platform(simple, physics, platform_id, "static_platform_4x4", 8, 0.35, 52)
    platform_id += 1

    # 区画3→4の移動床は、固定飛び石と並行する任意の近道。
    add_pair(simple, physics, 6001, "../collision_moving_platform/collision_moving_platform.x",
             "res/model/collision_moving_platform.x", -5, 0.65, -40,
             scale=1, load_type="meshmix2", move="y")

    # QTE木は北端の専用足場に置く。
    add_pair(simple, physics, 7001, "../tree2/lemonTree.x",
             "res/model/tree2Physics/tree_cylinder_collision.x", 8, 0.85, 52)

    # 高所のレバー2。箱の中のアイテムは扉を上げないと取得できない。
    add_pair(simple, physics, 9001, "../attack_trigger/lever_box_floor.x",
             "res/model/attack_trigger/lever_box_floor.x", 31, 3.7, -44)
    add_pair(simple, physics, 9002, "../attack_trigger/lever_box.x",
             "res/model/attack_trigger/lever_box.x", 31, 3.8, -44)
    add_pair(simple, physics, 9003, "../attack_trigger/lever_box_door.x",
             "res/model/attack_trigger/lever_box_door.x", 31, 3.8, -47,
             scale=0.98, move="y")

    # 区画9と11の間をつなぐレバー3門。床そのものが区画10の橋になる。
    gate_rotation = -53.13
    add_pair(simple, physics, 9101, "../attack_trigger/lever_box3_floor.x",
             "res/model/attack_trigger/lever_box3_floor.x", -24, 0.85, 27,
             rotation_y=gate_rotation)
    add_pair(simple, physics, 9102, "../attack_trigger/lever_box3.x",
             "res/model/attack_trigger/lever_box3.x", -24, 0.95, 27,
             rotation_y=gate_rotation)
    add_pair(simple, physics, 9103, "../attack_trigger/lever_box3_door.x",
             "res/model/attack_trigger/lever_box3_door.x", -24, 0.95, 27,
             rotation_y=gate_rotation, scale=0.98, move="y")

    # 洞窟の外周装飾。攻略足場と誤認しないよう、触れない外周だけに置く。
    rock_positions = (
        (-57, -34, 80, 1.0, 1), (-57, 2, 150, 0.9, 2), (-57, 28, 230, 1.1, 1),
        (57, -42, 190, 1.0, 2), (57, 20, 310, 0.9, 1), (57, 42, 40, 1.1, 2),
        (-24, -57, 120, 0.9, 2), (18, -57, 250, 1.0, 1), (34, 57, 145, 1.0, 2),
    )
    rock_id = 7201
    for x, z, rotation_y, scale, variant in rock_positions:
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
            [1, 6001, 6001, -5, 0.65, -40, 0, 0, 0, 1,
             -5, 0.65, -40, 1, 0.65, -34, 5.0]]

    enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
               ["small_spider", -20, 0.9, -46, 35], ["spider", -14, 0.9, -46, 210],
               ["small_golem", -20, 0.9, -40, 120], ["small_spider", -14, 0.9, -40, 300],
               ["small_golem", 38, 0.9, -20, 30], ["small_spider", 44, 0.9, -20, 200],
               ["spider", 38, 0.9, -14, 110], ["small_spider", 44, 0.9, -14, 290],
               ["small_golem", -13, 0.9, 14, 20], ["small_spider", -7, 0.9, 14, 190],
               ["spider", -13, 0.9, 20, 100], ["small_spider", -7, 0.9, 20, 280],
               ["small_golem", -43, 0.9, 38, 20], ["spider", -37, 0.9, 40, 200],
               ["small_spider", -43, 0.9, 44, 300]]

    collectibles = [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
                    ["stage25-I01", "Item", "006", 31, 4.35, -44, 1],
                    ["stage25-I02", "Item", "009", 54, 1.1, -1, 1],
                    ["stage25-I03", "Item", "010", 8, 1.1, -29, 1],
                    ["stage25-I04", "Item", "014", 32, 1.1, 4, 1],
                    ["stage25-I05", "Item", "016", 14, 1.1, 20, 1],
                    ["stage25-I06", "Item", "005", -55, 1.1, 55, 1]]

    destructibles = [["PosX", "PosY", "PosZ", "HP", "DropItemId"],
                     [-21, 1.05, -43, 2, "None"], [-12, 1.05, -43, 2, "014"],
                     [39, 1.05, -20, 3, "None"], [45, 1.05, -14, 2, "016"],
                     [-14, 1.05, 17, 2, "None"], [-39, 1.05, 40, 3, "010"]]

    lava = [["ID", "PhysicsID", "Damage"]]
    for index, physics_id in enumerate(lava_ids, start=1):
        lava.append(["stage25-lava-%02d" % index, physics_id, 20])

    direction_x = -12.0
    direction_z = 13.0
    direction_length = math.hypot(direction_x, direction_z)
    boosters = [["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ",
                 "Speed", "Duration", "Radius", "Scale", "ChargeEnabled"],
                ["stage25-booster-01", 44, 1.0, -10,
                 round(direction_x / direction_length, 4), 0.08, round(direction_z / direction_length, 4),
                 20, 0.85, 1.2, 0.6, "n"]]

    interactables = [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"],
                     ["stage25-tree-01", "Tree", 8, 0.85, 52, 2.5]]

    gate_direction_x = -0.8
    gate_direction_z = 0.6
    triggers = [["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis",
                 "BaseRotX", "BaseRotY", "BaseRotZ", "Scale", "LiftHeight"],
                [1, "LeverLift", 31, 4.4, -48.2, 9003, "Y", 0, 0, 0, 0.98, 6],
                [2, "LeverLift", -24 - gate_direction_x * 4.2, 1.55,
                 27 - gate_direction_z * 4.2, 9103, "Y", 0, -53.13, 0, 0.98, 6],
                [3, "LeverLift", -24 + gate_direction_x * 4.2, 1.55,
                 27 + gate_direction_z * 4.2, 9103, "Y", 0, -53.13, 0, 0.98, 6]]

    point_lights = [["PosX", "PosY", "PosZ", "Brightness", "ColorR", "ColorG", "ColorB", "ColorA",
                     "Shape", "LineLength", "SquareWidth", "SquareHeight", "RotX", "RotY", "RotZ", "Range", "OwnerTag"],
                    [-50, 3.0, -50, 1.0, 0.20, 0.55, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 11, "stage25-start"],
                    [-15, 3.0, -43, 0.9, 0.55, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage25-battle-a"],
                    [31, 7.0, -44, 0.9, 0.25, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage25-high"],
                    [42, 3.0, -17, 0.9, 0.50, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage25-battle-b"],
                    [32, 3.0, 4, 1.0, 1.0, 0.30, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage25-booster"],
                    [-8, 3.0, 17, 0.9, 0.20, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage25-battle-c"],
                    [-24, 3.2, 27, 1.0, 0.70, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage25-gate"],
                    [8, 3.0, 52, 0.9, 0.25, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage25-qte"],
                    [-51, 3.0, 53, 1.0, 0.15, 0.70, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 11, "stage25-goal"]]

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
