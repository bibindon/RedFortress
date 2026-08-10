# -*- coding: utf-8 -*-
"""ステージ2-2「上に逃げろ！」の配置CSVを一括生成する。"""

import csv
import io
from pathlib import Path


BASE = Path(__file__).resolve().parents[1] / "res" / "model" / "stage_2_2"

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
        raise RuntimeError("stage_ground.x is missing; run tools/BuildStage22Ground.py with Blender first")

    simple = [SIMPLE_HEADER]
    physics = [PHYSICS_HEADER]
    simple.append([1, "../ground/stage_visual_ground_world2.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"])
    add_pair(simple, physics, 2, "stage_ground.x", "res/model/stage_2_2/stage_ground.x",
             0, 0, 0, load_type="meshmix2")

    # Zone 3から4へ上昇しながら渡る必須移動床。
    add_pair(simple, physics, 2201, "../collision_moving_platform/collision_moving_platform.x",
             "res/model/collision_moving_platform.x", -20, 1.6, -32,
             scale=1.5, load_type="meshmix2", move="y")

    # Zone 5の箱＋感圧板3で開く門。
    add_pair(simple, physics, 2301, "../collision_wall/collision_wall_tall.x",
             "res/model/collision_wall/collision_wall_tall_collision.x", 8.5, 4.3, -12,
             rotation_y=90, move="y")

    # Zone 6のレバー3で開く中央門。
    add_pair(simple, physics, 2401, "../attack_trigger/lever_box3_floor.x",
             "res/model/attack_trigger/lever_box3_floor.x", -9, 5.25, -3)
    add_pair(simple, physics, 2402, "../attack_trigger/lever_box3.x",
             "res/model/attack_trigger/lever_box3.x", -9, 5.35, -3)
    add_pair(simple, physics, 2403, "../attack_trigger/lever_box3_door.x",
             "res/model/attack_trigger/lever_box3_door.x", -9, 5.35, -3,
             scale=0.98, move="y")

    # 西端の行き止まりにQTE木を置く。
    add_pair(simple, physics, 2501, "../tree2/lemonTree.x",
             "res/model/tree2Physics/tree_cylinder_collision.x", -27, 7.0, 10)

    rocks = (
        (-34, -48, 30, 1.2, 1), (-35, -18, 110, 1.0, 2), (-34, 16, 210, 1.2, 1),
        (-35, 48, 300, 1.0, 2), (34, -42, 150, 1.1, 2), (35, -8, 230, 1.2, 1),
        (34, 24, 30, 1.0, 2), (35, 50, 320, 1.2, 1), (-12, -64, 80, 1.0, 2),
        (12, 64, 260, 1.0, 1),
    )
    rock_id = 2601
    for x, z, rotation_y, scale, variant in rocks:
        model = "../base/base_rock1.x"
        if variant == 2:
            model = "../base/base_rock2.x"
        simple.append([rock_id, model, x, 0, z, 0, rotation_y, 0, scale, "normal"])
        rock_id += 1

    simple.append([2900, "../SkySphere_cave/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 0, 1, "normal"])
    return simple, physics


def main():
    BASE.mkdir(parents=True, exist_ok=True)
    simple, physics = build_render_and_physics()

    move = [MOVE_HEADER,
            [1, 2201, 2201, -20, 1.6, -32, 0, 0, 0, 1.5,
             -20, 1.6, -32, -14, 2.7, -28, 5.5]]

    enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
               ["small_spider", -27, 1.9, -38, 45], ["spider", -24, 1.9, -34, 180],
               ["small_golem", -20, 1.9, -37, 270], ["small_spider", -12, 3.0, -27, 30],
               ["small_golem", -8, 3.0, -24, 210], ["small_spider", 16, 4.1, -16, 45],
               ["spider", 20, 4.1, -22, 180], ["small_golem", 24, 4.1, -21, 270],
               ["small_spider", 0, 5.3, -10, 30], ["small_golem", 4, 5.3, -6, 210],
               ["small_spider", 17, 7.5, 10, 45], ["spider", 22, 7.5, 14, 225],
               ["small_spider", -19, 8.5, 22, 30], ["small_golem", -13, 8.5, 26, 210],
               ["spider", -20, 11.5, 45, 180], ["small_spider", -25, 7.0, 12, 90],
               ["small_golem", 25, 9.5, 30, 270]]

    collectibles = [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
                    ["stage22-I01", "Item", "006", -27, 7.25, 12, 1],
                    ["stage22-I02", "Item", "009", 27, 12.75, 34, 1],
                    ["stage22-I03", "Item", "010", 22, 7.75, 12, 1],
                    ["stage22-I04", "Item", "014", 2, 10.25, 36, 1]]

    destructibles = [["PosX", "PosY", "PosZ", "HP", "DropItemId"],
                     [-27, 2.1, -34, 2, "None"], [-21, 2.1, -34, 2, "014"],
                     [-18, 8.7, 24, 3, "None"]]

    boosters = [["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ",
                 "Speed", "Duration", "Radius", "Scale", "ChargeEnabled"],
                ["stage22-booster-01", 27, 9.5, 28, 0, 0.4472, 0.8944,
                 18, 0.5, 1.2, 0.6, "n"]]

    interactables = [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"],
                     ["stage22-tree-01", "Tree", -27, 7.0, 10, 2.5]]

    triggers = [["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis",
                 "BaseRotX", "BaseRotY", "BaseRotZ", "Scale", "LiftHeight"],
                [1, "LeverLift", 2, 5.4, -8, 2403, "Y", 0, 0, 0, 0.98, 6]]

    pressure_plates = [["ID", "PlatePosX", "PlatePosY", "PlatePosZ", "WallID",
                        "WallRotX", "WallRotY", "WallRotZ", "WallScale", "TravelDistance"],
                       [1, 22, 4.1, -18, 2301, 0, 90, 0, 1, 6]]
    pushable_boxes = [["ID", "PosX", "PosY", "PosZ", "RotY", "Scale"],
                      [1, 17, 4.1, -21, 0, 1]]

    lava_rise = [["ID", "Damage", "MinX", "MaxX", "MinZ", "MaxZ",
                  "StartY", "EndY", "Delay", "Duration"],
                 ["stage22-rise-01", 20, -30, 30, -60, 60, -2, 11.5, 12, 190]]

    point_lights = [["PosX", "PosY", "PosZ", "Brightness", "ColorR", "ColorG", "ColorB", "ColorA",
                     "Shape", "LineLength", "SquareWidth", "SquareHeight", "RotX", "RotY", "RotZ", "Range", "OwnerTag"],
                    [-20, 3.0, -54, 1.0, 0.20, 0.55, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage22-start"],
                    [-24, 4.0, -36, 0.9, 1.0, 0.25, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage22-lower"],
                    [20, 6.0, -19, 0.9, 0.70, 0.25, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage22-plate"],
                    [2, 7.0, -8, 0.9, 0.25, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage22-lever"],
                    [20, 9.0, 12, 0.9, 1.0, 0.30, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage22-east"],
                    [-16, 10.0, 24, 0.9, 0.25, 0.65, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage22-upper"],
                    [-20, 13.0, 45, 0.9, 1.0, 0.30, 0.10, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage22-final"],
                    [20, 15.0, 54, 1.0, 0.15, 0.70, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage22-goal"]]

    empty_files = {
        "LavaZones.csv": [["ID", "PhysicsID", "Damage"]],
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
    write_csv("PressurePlates.csv", pressure_plates)
    write_csv("PushableBoxes.csv", pushable_boxes)
    write_csv("LavaRise.csv", lava_rise)
    write_csv("PointLights.csv", point_lights)
    for filename, rows in empty_files.items():
        write_csv(filename, rows)


if __name__ == "__main__":
    main()
