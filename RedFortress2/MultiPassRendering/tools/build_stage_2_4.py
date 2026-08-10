# -*- coding: utf-8 -*-
"""ステージ2-4「空飛ぶ足場を乗りつごう」のCSVを一括生成する。"""

import csv
import io
import math
from pathlib import Path


BASE = Path(__file__).resolve().parents[1] / "res" / "model" / "stage_2_4"

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


def add_island(simple, physics, csv_id, x, y, z, large=False):
    if not large:
        add_static(simple, physics, csv_id, "static_platform_4x4", x, y, z)
        return csv_id + 1

    for offset_x, offset_z in ((-6, -6), (6, -6), (-6, 6), (6, 6)):
        add_static(simple, physics, csv_id, "static_platform_4x4", x + offset_x, y, z + offset_z)
        csv_id += 1
    return csv_id


def add_stairway(simple, physics, csv_id, steps):
    for x, y, z in steps:
        add_static(simple, physics, csv_id, "static_platform_2x2", x, y, z)
        csv_id += 1
    return csv_id


def add_moving_platform(simple, physics, move, csv_id, start, end, duration):
    start_x, level_y, start_z = start
    end_x, end_y, end_z = end
    if level_y != end_y:
        raise ValueError("ステージ2-4では上下移動床を使用できません")
    platform_y = level_y + 0.3
    add_pair(simple, physics, csv_id,
             "../collision_moving_platform/collision_moving_platform.x",
             "res/model/collision_moving_platform.x",
             start_x, platform_y, start_z,
             scale=1.5, load_type="meshmix2", move="y")
    move.append([csv_id - 6000, csv_id, csv_id,
                 start_x, platform_y, start_z, 0, 0, 0, 1.5,
                 start_x, platform_y, start_z, end_x, platform_y, end_z, duration])


def build_render_physics_and_move():
    simple = [SIMPLE_HEADER]
    physics = [PHYSICS_HEADER]
    move = [MOVE_HEADER]

    # 外周衝突と洞窟背景だけを残し、地面は読み込まない。足場外はY=-10mまで落下して死亡する。
    physics.append([1, "res/model/cubeNormalInverse120x120.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""])
    simple.append([9000, "../SkySphere_cave/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 0, 1, "normal"])

    platform_id = 5001

    # 本線16区画。開始足場とゴール足場は同じXZ座標だが29m離れている。
    main_islands = (
        (-50, 0.35, -50, False), (-50, 0.35, -28, False), (-44, 0.35, -4, False),
        (-48, 8.0, 45, False), (-15, 8.0, 50, False), (24, 8.0, 46, False),
        (50, 16.0, 36, False), (51, 16.0, 5, False), (45, 16.0, -28, False),
        (15, 16.0, -50, False), (-7, 24.0, -31, False), (5, 24.0, 5, False),
        (25, 24.0, 28, False), (-15, 24.0, 30, False), (-30, 24.0, 0, False),
        (-30, 24.0, -30, False), (-50, 29.35, -50, False),
    )
    for x, y, z, large in main_islands:
        platform_id = add_island(simple, physics, platform_id, x, y, z, large)

    # ③→④、⑥→⑦、⑩→⑪、⑯→ゴール。高さは固定階段だけで上げる。
    stairways = (
        ((-46, 1.2, 4), (-47, 2.1, 9), (-48, 3.0, 14), (-49, 3.9, 19),
         (-50, 4.8, 24), (-50, 5.7, 29), (-49, 6.6, 34), (-48, 7.5, 39)),
        ((29, 8.9, 44), (33, 9.8, 43), (37, 10.7, 41), (41, 11.6, 40),
         (44, 12.5, 39), (47, 13.4, 38), (49, 14.3, 37), (50, 15.2, 36)),
        ((12, 16.9, -46), (10, 17.8, -44), (7, 18.7, -42), (5, 19.6, -40),
         (2, 20.5, -38), (0, 21.4, -36), (-2, 22.3, -34), (-5, 23.2, -32)),
        ((-34, 24.8, -34), (-37, 25.6, -37), (-40, 26.4, -40),
         (-43, 27.2, -43), (-46, 28.0, -46), (-48, 28.7, -48)),
    )
    for steps in stairways:
        platform_id = add_stairway(simple, physics, platform_id, steps)

    # 5本の往復分岐。分岐先は他ルートへ接続しない。
    branch_islands = (
        (-54, 0.35, 12, False),
        (5, 8.0, 54, False),
        (47, 16.0, -18, True),
        (18, 24.0, -38, False),
        (-52, 24.0, -10, False),
    )
    for x, y, z, large in branch_islands:
        platform_id = add_island(simple, physics, platform_id, x, y, z, large)

    moving_routes = (
        ((-50, 0.35, -43), (-50, 0.35, -35), 4.0),
        ((-49, 0.35, -21), (-45, 0.35, -11), 5.0),
        ((-41, 8.0, 46), (-22, 8.0, 49), 7.0),
        ((-8, 8.0, 49), (17, 8.0, 47), 8.0),
        ((50, 16.0, 29), (51, 16.0, 12), 7.0),
        ((50, 16.0, -2), (46, 16.0, -21), 7.0),
        ((39, 16.0, -33), (21, 16.0, -46), 8.0),
        ((-5, 24.0, -24), (3, 24.0, -2), 8.0),
        ((10, 24.0, 10), (21, 24.0, 23), 6.0),
        ((18, 24.0, 29), (-8, 24.0, 30), 8.0),
        ((-18, 24.0, 24), (-28, 24.0, 7), 7.0),
        ((-30, 24.0, -7), (-30, 24.0, -23), 6.0),
        # 枝A～E。
        ((-48, 0.35, 0), (-53, 0.35, 6), 4.0),
        ((-9, 8.0, 51), (-2, 8.0, 53), 4.0),
        ((52, 16.0, -2), (50, 16.0, -8), 4.0),
        ((0, 24.0, -34), (11, 24.0, -37), 5.0),
        ((-36, 24.0, -3), (-46, 24.0, -8), 5.0),
    )
    for index, (start, end, duration) in enumerate(moving_routes, start=1):
        add_moving_platform(simple, physics, move, 6000 + index, start, end, duration)

    # 枝AのQTE木。
    add_pair(simple, physics, 7001, "../tree2/lemonTree.x",
             "res/model/tree2Physics/tree_cylinder_collision.x", -54, 0.85, 12)

    # 枝Cのレバー2報酬箱。大きな足場の東半分に置く。
    add_pair(simple, physics, 7101, "../attack_trigger/lever_box_floor.x",
             "res/model/attack_trigger/lever_box_floor.x", 53, 16.05, -18)
    add_pair(simple, physics, 7102, "../attack_trigger/lever_box.x",
             "res/model/attack_trigger/lever_box.x", 53, 16.15, -18)
    add_pair(simple, physics, 7103, "../attack_trigger/lever_box_door.x",
             "res/model/attack_trigger/lever_box_door.x", 53, 16.15, -21,
             scale=0.98, move="y")

    return simple, physics, move


def main():
    BASE.mkdir(parents=True, exist_ok=True)
    simple, physics, move = build_render_physics_and_move()

    enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
               ["small_spider", -47, 0.9, -1, 30], ["spider", -42, 0.9, -5, 180],
               ["small_golem", -46, 0.9, -8, 270],
               ["small_spider", -18, 8.55, 48, 40], ["spider", -13, 8.55, 52, 200],
               ["small_golem", -10, 8.55, 47, 300],
               ["bird", 48, 17.5, 2, 180], ["small_spider", 53, 16.55, 5, 20],
               ["small_golem", 49, 16.55, 8, 250],
               ["small_spider", -10, 24.55, -29, 45], ["spider", -5, 24.55, -33, 210],
               ["small_golem", -3, 24.55, -28, 310],
               # 分岐B、C、Eは敵を倒して戻る必要がある。
               ["bird", 2, 9.5, 52, 90], ["small_spider", 7, 8.55, 55, 220],
               ["small_golem", 8, 8.55, 51, 300],
               ["small_spider", 40, 16.55, -14, 20], ["spider", 41, 16.55, -20, 180],
               ["small_golem", 46, 16.55, -24, 270],
               ["bird", -52, 25.5, -12, 0], ["small_spider", -49, 24.55, -7, 150],
               ["small_golem", -55, 24.55, -8, 260]]

    collectibles = [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
                    ["stage24-I01", "Item", "006", -54, 1.1, 12, 1],
                    ["stage24-I02", "Item", "009", 5, 8.8, 54, 1],
                    ["stage24-I03", "Item", "014", 53, 16.7, -18, 1],
                    ["stage24-I04", "Item", "016", 18, 24.8, -38, 1],
                    ["stage24-I05", "Item", "010", -52, 24.8, -10, 1],
                    ["stage24-I06", "Item", "005", 5, 24.8, 5, 1]]

    destructibles = [["PosX", "PosY", "PosZ", "HP", "DropItemId"],
                     [-48, 1.05, -26, 2, "None"], [22, 8.7, 44, 3, "014"],
                     [48, 16.7, 34, 2, "None"], [42, 16.7, -29, 3, "016"],
                     [22, 24.7, 28, 3, "None"], [-27, 24.7, 2, 2, "010"]]

    direction_x = -6.0
    direction_z = -19.0
    direction_length = math.hypot(direction_x, direction_z)
    boosters = [["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ",
                 "Speed", "Duration", "Radius", "Scale", "ChargeEnabled"],
                ["stage24-booster-01", 49, 16.8, 4,
                 round(direction_x / direction_length, 4), 0.02,
                 round(direction_z / direction_length, 4), 20, 1.0, 1.2, 0.6, "n"]]

    interactables = [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"],
                     ["stage24-tree-01", "Tree", -54, 0.85, 12, 2.5]]

    triggers = [["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis",
                 "BaseRotX", "BaseRotY", "BaseRotZ", "Scale", "LiftHeight"],
                [1, "LeverLift", 48.5, 16.75, -22.5, 7103, "Y", 0, 0, 0, 0.98, 6]]

    point_lights = [["PosX", "PosY", "PosZ", "Brightness", "ColorR", "ColorG", "ColorB", "ColorA",
                     "Shape", "LineLength", "SquareWidth", "SquareHeight", "RotX", "RotY", "RotZ", "Range", "OwnerTag"],
                    [-50, 4.0, -50, 1.0, 0.15, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 13, "stage24-start"],
                    [-46, 6.0, 16, 0.9, 0.25, 0.55, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 14, "stage24-west-stairs"],
                    [-15, 12.0, 50, 0.9, 0.35, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 14, "stage24-north"],
                    [50, 20.0, 15, 0.9, 1.0, 0.35, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 15, "stage24-east"],
                    [15, 20.0, -45, 0.9, 1.0, 0.25, 0.12, 1.0, "Point", 12, 10, 10, 0, 0, 0, 14, "stage24-south"],
                    [5, 28.0, 5, 0.9, 0.45, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 15, "stage24-high"],
                    [-50, 33.0, -50, 1.1, 1.0, 0.20, 0.45, 1.0, "Point", 12, 10, 10, 0, 0, 0, 13, "stage24-goal"]]

    empty_files = {
        "LavaZones.csv": [["ID", "PhysicsID", "Damage"]],
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
    write_csv("DashBoosters.csv", boosters)
    write_csv("Interactables.csv", interactables)
    write_csv("AttackTriggers.csv", triggers)
    write_csv("PointLights.csv", point_lights)
    for filename, rows in empty_files.items():
        write_csv(filename, rows)


if __name__ == "__main__":
    main()
