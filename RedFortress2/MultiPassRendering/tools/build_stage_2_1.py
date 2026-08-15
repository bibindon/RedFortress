# -*- coding: utf-8 -*-
"""ステージ2-1「鳥だらけ」の配置CSVを一括生成する。"""

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
    if not (BASE / "stage_ground.x").exists():
        raise RuntimeError("stage_ground.x is missing; run tools/BuildStage21Ground.py with Blender first")

    simple = [SIMPLE_HEADER]
    physics = [PHYSICS_HEADER]

    simple.append([1, "../ground/stage_visual_ground_world2.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"])
    add_pair(simple, physics, 2, "stage_ground.x", "res/model/stage_2_1/stage_ground.x",
             0, 0, 0, load_type="meshmix2")

    # 20m四方のダメージ床を3x6枚並べ、60x120m全体を覆う。
    lava_ids = []
    lava_id = 2101
    for z in (-50, -30, -10, 10, 30, 50):
        for x in (-20, 0, 20):
            add_pair(simple, physics, lava_id, "../plateLava.x", "res/model/plateLava.x",
                     x, 0.02, z, scale=2.5, load_type="meshmix2", physics_type="NonCollision")
            lava_ids.append(lava_id)
            lava_id += 1

    # 区画3から4へ渡る必須移動床。南側の岩壁の開口部を往復する。
    add_pair(simple, physics, 2201, "../collision_moving_platform/collision_moving_platform.x",
             "res/model/collision_moving_platform.x", -18.5, 0.65, -29.5,
             scale=1.5, load_type="meshmix2", move="y")

    # 中央のレバー3門。高台のレバーを操作しないと南北を移動できない。
    add_pair(simple, physics, 2301, "../attack_trigger/lever_box3_floor.x",
             "res/model/attack_trigger/lever_box3_floor.x", 0, -0.05, 0)
    add_pair(simple, physics, 2302, "../attack_trigger/lever_box3.x",
             "res/model/attack_trigger/lever_box3.x", 0, 0.95, 0)
    add_pair(simple, physics, 2303, "../attack_trigger/lever_box3_door.x",
             "res/model/attack_trigger/lever_box3_door.x", 0, 0.95, 0,
             scale=0.98, move="y")

    # 東端の行き止まりにQTE木を置く。
    add_pair(simple, physics, 2401, "../tree2/lemonTree.x",
             "res/model/tree2Physics/tree_cylinder_collision.x", 26, 0.9, -18)

    # 触れない外周に洞窟用の岩を置き、60x120mの細長い空間を視覚化する。
    rocks = (
        (-34, -48, 40, 1.2, 1), (-35, -18, 120, 1.0, 2), (-34, 18, 210, 1.2, 1),
        (-35, 48, 300, 1.0, 2), (34, -42, 150, 1.1, 2), (35, -8, 230, 1.2, 1),
        (34, 25, 30, 1.0, 2), (35, 50, 320, 1.2, 1), (-12, -64, 80, 1.0, 2),
        (12, 64, 260, 1.0, 1),
    )
    rock_id = 2501
    for x, z, rotation_y, scale, variant in rocks:
        model = "../base/base_rock1.x"
        if variant == 2:
            model = "../base/base_rock2.x"
        simple.append([rock_id, model, x, 0, z, 0, rotation_y, 0, scale, "normal"])
        rock_id += 1

    simple.append([2900, "../SkySphere_cave/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 0, 1, "normal"])
    return simple, physics, lava_ids


def main():
    BASE.mkdir(parents=True, exist_ok=True)
    simple, physics, lava_ids = build_render_and_physics()

    move = [MOVE_HEADER,
            [1, 2201, 2201, -18.5, 0.65, -29.5, 0, 0, 0, 1.5,
             -18.5, 0.65, -29.5, -13.0, 0.65, -26.5, 4.5]]

    # 全15体を安全な島ではなく細道脇のダメージ床上へ置く。
    enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
               ["bird", -27, 3.0, -39, 90], ["bird", -13, 4.0, -41, 270],
               ["bird", -29, 5.0, -35, 90], ["bird", -3, 3.0, -24, 180],
               ["bird", -17, 4.0, -20, 0], ["bird", -12, 3.0, -13, 90],
               ["bird", -16, 5.0, -12, 180], ["bird", -8, 4.0, 3, 270],
               ["bird", 7, 3.0, -3, 90], ["bird", 13, 5.0, 2, 270],
               ["bird", 14, 4.0, 22, 180], ["bird", 7, 3.0, 13, 0],
               ["bird", -6, 4.0, 43, 180], ["bird", 10, 3.0, -21, 0],
               ["bird", -14, 5.0, 19, 180]]

    collectibles = [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
                    ["stage21-I01", "Item", "006", 26, 1.15, -15, 1],
                    ["stage21-I02", "Item", "009", -26, 4.75, 13, 1],
                    ["stage21-I03", "Item", "010", 22, 4.35, 16, 1],
                    ["stage21-I04", "Item", "014", -18, 1.15, 35, 1]]

    destructibles = [["PosX", "PosY", "PosZ", "HP", "DropItemId"],
                     [-25, 1.1, -34, 2, "None"], [-21, 1.1, -34, 2, "014"],
                     [17, 1.1, 5, 2, "None"], [21, 1.1, 5, 2, "010"],
                     [-20, 1.1, 35, 3, "None"]]

    lava = [["ID", "PhysicsID", "Damage"]]
    for index, physics_id in enumerate(lava_ids, start=1):
        lava.append(["stage21-lava-%02d" % index, physics_id, 20])

    boosters = [["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ",
                 "Speed", "Duration", "Radius", "Scale", "ChargeEnabled"],
                ["stage21-booster-01", -24, 1.0, 20, -0.247, 0.445, -0.861,
                 18, 0.58, 1.2, 0.6, "n"]]

    interactables = [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"],
                     ["stage21-tree-01", "Tree", 26, 0.9, -18, 2.5]]

    triggers = [["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis",
                 "BaseRotX", "BaseRotY", "BaseRotZ", "Scale", "LiftHeight"],
                [1, "LeverLift", -22, 4.2, -5, 2303, "Y", 0, 0, 0, 0.98, 6]]

    point_lights = [["PosX", "PosY", "PosZ", "Brightness", "ColorR", "ColorG", "ColorB", "ColorA",
                     "Shape", "LineLength", "SquareWidth", "SquareHeight", "RotX", "RotY", "RotZ", "Range", "OwnerTag"],
                    [-20, 3.0, -54, 1.0, 0.20, 0.55, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage21-start"],
                    [-23, 4.0, -34, 0.8, 0.50, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-south"],
                    [-22, 6.0, -5, 0.9, 0.25, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-lever"],
                    [19, 4.0, 5, 0.8, 0.55, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-east"],
                    [22, 7.0, 16, 0.9, 0.25, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-high"],
                    [-18, 4.0, 35, 0.8, 0.55, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage21-north"],
                    [20, 3.0, 54, 1.0, 0.15, 0.70, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage21-goal"]]

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
