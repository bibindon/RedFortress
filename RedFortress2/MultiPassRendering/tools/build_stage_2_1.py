# -*- coding: utf-8 -*-
"""ステージ2-1「チクチク床の飛び石」のCSVを一括生成する。"""

import csv
import io
from pathlib import Path


BASE = Path(__file__).resolve().parents[1] / "res" / "model" / "stage_2_1"

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


def build_render_and_physics():
    simple = [SIMPLE_HEADER]
    physics = [PHYSICS_HEADER]

    simple.append([1, "../ground/stage_visual_ground_world2.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"])
    simple.append([2, "stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"])
    physics.append([1, "res/model/cubeNormalInverse120x120.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""])
    physics.append([2, "res/model/stage_2_1/stage_ground.x", 0, 0.01, 0, 0, 0, 0, 1, "Collision", "n", ""])

    # 外周の洞窟壁。内側の柵を越えても、この壁でステージ外へ出られない。
    wall_id = 3001
    wall_steps = (-48, -32, -16, 0, 16, 32, 48)
    for z in wall_steps:
        add_pair(simple, physics, wall_id, "../collision_wall/collision_wall_tall.x",
                 "res/model/collision_wall/collision_wall_tall_collision.x", -58, 3, z,
                 scale=2, load_type="meshmix2")
        wall_id += 1
        add_pair(simple, physics, wall_id, "../collision_wall/collision_wall_tall.x",
                 "res/model/collision_wall/collision_wall_tall_collision.x", 58, 3, z,
                 scale=2, load_type="meshmix2")
        wall_id += 1
    for x in wall_steps:
        add_pair(simple, physics, wall_id, "../collision_wall/collision_wall_tall.x",
                 "res/model/collision_wall/collision_wall_tall_collision.x", x, 3, -58,
                 rotation_y=90, scale=2, load_type="meshmix2")
        wall_id += 1
        add_pair(simple, physics, wall_id, "../collision_wall/collision_wall_tall.x",
                 "res/model/collision_wall/collision_wall_tall_collision.x", x, 3, 58,
                 rotation_y=90, scale=2, load_type="meshmix2")
        wall_id += 1

    # 区画10のレバー3門。左右をステージ幅いっぱいまで壁で塞ぐ。
    gate_wall_x = (-50, -34, -12, 5, 22, 39, 52)
    for x in gate_wall_x:
        add_pair(simple, physics, wall_id, "../collision_wall/collision_wall_tall.x",
                 "res/model/collision_wall/collision_wall_tall_collision.x", x, 3, 24,
                 rotation_y=90, scale=2, load_type="meshmix2")
        wall_id += 1

    # 4本のダメージ床帯。半径4mの円盤を連続させ、外周迂回を防ぐ。
    lava_ids = []
    lava_id = 4001
    for z in (-44, -25, 2, 36):
        for x in range(-56, 57, 8):
            simple.append([lava_id, "../plateLava.x", x, 0.02, z, 0, 0, 0, 1, "meshmix2"])
            physics.append([lava_id, "res/model/plateLava.x", x, 0.02, z, 0, 0, 0, 1, "NonCollision", "n", ""])
            lava_ids.append(lava_id)
            lava_id += 1

    # 通常ジャンプ用の安全な飛び石。3x3m、中心間隔は最大6.4mだが斜め区間は2石に分ける。
    stone_positions = (
        (-43, 0.35, -46), (-39, 0.35, -42),
        (-7, 0.35, -28), (-7, 0.35, -22),
        (26, 0.35, -2), (29, 0.35, 3), (32, 0.35, 8),
        (-29, 0.35, 32), (-35, 0.65, 36), (-41, 0.35, 40),
    )
    stone_id = 5001
    for x, y, z in stone_positions:
        add_pair(simple, physics, stone_id, "../static_platform/static_platform_1x1.x",
                 "res/model/static_platform/static_platform_1x1_collision.x", x, y, z)
        stone_id += 1

    # 区画4: 第2ダメージ帯を横断する6x6mの移動床。
    add_pair(simple, physics, 6001, "../collision_moving_platform/collision_moving_platform.x",
             "res/model/collision_moving_platform.x", 4, 0.4, -31,
             scale=2, load_type="meshmix2", move="y")

    # 区画5: 通常ジャンプで登れる0.8m刻みの高台。最上段はY=3.2m。
    high_platforms = (
        (6101, "static_platform_2x2", 18, 0.8, -40),
        (6102, "static_platform_2x2", 23, 1.6, -40),
        (6103, "static_platform_2x2", 28, 2.4, -40),
        (6104, "static_platform_4x4", 35, 3.2, -40),
    )
    for csv_id, model, x, y, z in high_platforms:
        add_pair(simple, physics, csv_id, "../static_platform/" + model + ".x",
                 "res/model/static_platform/" + model + "_collision.x", x, y, z)

    # QTE木。World 2の柵外装飾には木を使わず、これは攻略対象として柵内に置く。
    add_pair(simple, physics, 7001, "../tree2/lemonTree.x",
             "res/model/tree2Physics/tree_cylinder_collision.x", -52, 0, -31)

    # レバー2: 高台上の報酬箱。
    add_pair(simple, physics, 9001, "../attack_trigger/lever_box_floor.x",
             "res/model/attack_trigger/lever_box_floor.x", 35, 3.25, -40)
    add_pair(simple, physics, 9002, "../attack_trigger/lever_box.x",
             "res/model/attack_trigger/lever_box.x", 35, 3.35, -40)
    add_pair(simple, physics, 9003, "../attack_trigger/lever_box_door.x",
             "res/model/attack_trigger/lever_box_door.x", 35, 3.35, -43,
             scale=0.98, move="y")

    # レバー3: 南北を接続する両開き門。
    add_pair(simple, physics, 9101, "../attack_trigger/lever_box3_floor.x",
             "res/model/attack_trigger/lever_box3_floor.x", -24, 0.05, 24)
    add_pair(simple, physics, 9102, "../attack_trigger/lever_box3.x",
             "res/model/attack_trigger/lever_box3.x", -24, 0.15, 24)
    add_pair(simple, physics, 9103, "../attack_trigger/lever_box3_door.x",
             "res/model/attack_trigger/lever_box3_door.x", -24, 0.15, 24,
             scale=0.98, move="y")

    # 柵は境界の視認用。衝突は外周壁が担当する。
    fence_id = 8001
    fence_steps = range(-52, 53, 8)
    for value in fence_steps:
        simple.append([fence_id, "../fence.x", -55, 0.5, value, 0, 0, 0, 1, "normal"])
        fence_id += 1
        simple.append([fence_id, "../fence.x", 55, 0.5, value, 0, 0, 0, 1, "normal"])
        fence_id += 1
        simple.append([fence_id, "../fence.x", value, 0.5, -55, 0, 90, 0, 1, "normal"])
        fence_id += 1
        simple.append([fence_id, "../fence.x", value, 0.5, 55, 0, 90, 0, 1, "normal"])
        fence_id += 1

    # 柵外の洞窟景観は岩だけを使用する。
    rock_positions = (
        (-57, -49, 25, 0.8, 1), (-57, -34, 80, 1.1, 2), (-57, -15, 150, 0.9, 1),
        (-57, 5, 230, 1.2, 2), (-57, 26, 310, 0.8, 1), (-57, 47, 35, 1.0, 2),
        (57, -47, 190, 1.0, 2), (57, -28, 270, 0.8, 1), (57, -8, 340, 1.2, 2),
        (57, 13, 60, 0.9, 1), (57, 33, 130, 1.1, 2), (57, 49, 215, 0.8, 1),
        (-47, -57, 10, 1.0, 2), (-27, -57, 95, 0.8, 1), (-7, -57, 170, 1.1, 2),
        (14, -57, 250, 0.9, 1), (35, -57, 325, 1.2, 2), (49, -57, 45, 0.8, 1),
        (-49, 57, 205, 1.1, 1), (-31, 57, 285, 0.9, 2), (-10, 57, 355, 1.2, 1),
        (12, 57, 70, 0.8, 2), (33, 57, 145, 1.0, 1), (49, 57, 225, 1.1, 2),
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
            [1, 6001, 6001, 4, 0.4, -31, 0, 0, 0, 2, 4, 0.4, -31, 4, 0.4, -19, 6.0]]

    enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
               ["small_spider", -20, 0.2, -37, 30], ["spider", -14, 0.2, -34, 210],
               ["small_golem", -8, 0.2, -31, 180], ["small_spider", 11, 0.2, -36, 90],
               ["spider", 18, 0.2, -33, 270], ["small_golem", 42, 0.2, -17, 180],
               ["small_spider", 48, 0.2, -13, 240], ["spider", 35, 0.2, -15, 90],
               ["small_spider", 24, 0.2, -12, 0], ["small_golem", 26, 0.2, 12, 180],
               ["small_spider", 18, 0.2, 15, 90], ["spider", 8, 0.2, 17, 270],
               ["small_spider", -4, 0.2, 16, 0], ["small_golem", -12, 0.2, 20, 180],
               ["spider", -18, 0.2, 29, 180], ["small_spider", -30, 0.2, 30, 45],
               ["small_golem", -43, 0.2, 29, 315]]

    collectibles = [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
                    ["stage21-I01", "Item", "005", -54, 0.45, -31, 1],
                    ["stage21-I02", "Item", "006", 35, 3.9, -40, 1],
                    ["stage21-I03", "Item", "009", 53, 0.45, -13, 1],
                    ["stage21-I04", "Item", "010", 42, 0.45, 9, 1],
                    ["stage21-I05", "Item", "014", -24, 0.55, 24, 1],
                    ["stage21-I06", "Item", "016", -50, 0.45, 44, 1]]

    destructibles = [["PosX", "PosY", "PosZ", "HP", "DropItemId"],
                     [-25, 0.45, -35, 2, "None"], [-22, 0.45, -33, 2, "014"],
                     [50, 0.45, -18, 3, "None"], [47, 0.45, -20, 2, "016"],
                     [4, 0.45, 12, 2, "None"], [-16, 0.45, 31, 3, "010"]]

    lava = [["ID", "PhysicsID", "Damage"]]
    for index, physics_id in enumerate(lava_ids, start=1):
        lava.append(["stage21-lava-%02d" % index, physics_id, 20])

    boosters = [["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ",
                 "Speed", "Duration", "Radius", "Scale", "ChargeEnabled"],
                ["stage21-booster-01", 34, 0.5, -6, 0, 0.08, 1, 20, 0.8, 1.2, 0.6, "n"]]

    interactables = [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"],
                     ["stage21-tree-01", "Tree", -52, 0, -31, 2.5]]

    triggers = [["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis",
                 "BaseRotX", "BaseRotY", "BaseRotZ", "Scale", "LiftHeight"],
                [1, "LeverLift", 35, 3.95, -45, 9003, "Y", 0, 0, 0, 0.98, 6],
                [2, "LeverLift", -24, 0.75, 20.5, 9103, "Y", 0, 0, 0, 0.98, 6],
                [3, "LeverLift", -24, 0.75, 27.5, 9103, "Y", 0, 0, 0, 0.98, 6]]

    point_lights = [["PosX", "PosY", "PosZ", "Brightness", "ColorR", "ColorG", "ColorB", "ColorA",
                     "Shape", "LineLength", "SquareWidth", "SquareHeight", "RotX", "RotY", "RotZ", "Range", "OwnerTag"],
                    [-48, 3.0, -52, 1.0, 0.20, 0.55, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 11, "stage21-start"],
                    [-36, 2.5, -40, 0.9, 1.0, 0.35, 0.12, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage21-lava1"],
                    [-12, 2.5, -34, 0.8, 0.45, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage21-battle1"],
                    [4, 2.8, -25, 0.9, 1.0, 0.35, 0.12, 1.0, "Point", 12, 10, 10, 0, 0, 0, 11, "stage21-moving"],
                    [35, 6.8, -40, 0.9, 0.25, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-high"],
                    [44, 2.6, -15, 0.8, 0.50, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage21-east"],
                    [34, 2.5, 2, 1.0, 1.0, 0.30, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-booster"],
                    [15, 2.5, 15, 0.8, 0.20, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage21-zigzag"],
                    [-24, 3.0, 24, 1.0, 0.70, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-gate"],
                    [-35, 2.5, 36, 1.0, 1.0, 0.30, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-lava4"],
                    [-48, 3.0, 52, 1.0, 0.15, 0.70, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 11, "stage21-goal"]]

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
