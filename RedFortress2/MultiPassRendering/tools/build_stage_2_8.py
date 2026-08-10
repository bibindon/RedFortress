# -*- coding: utf-8 -*-
"""ステージ2-8のボスあり版とクリア後版の配置CSVを一括生成する。"""

import csv
import io
from pathlib import Path


BASE = Path(__file__).resolve().parents[1] / "res" / "model" / "stage_2_8"
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
             rotation_y=0, scale=1, load_type="normal", physics_type="Collision"):
    simple.append([csv_id, render_path, x, y, z, 0, rotation_y, 0, scale, load_type])
    physics.append([csv_id, physics_path, x, y, z, 0, rotation_y, 0, scale,
                    physics_type, "n", ""])


def add_lava_pair(simple, physics, lava_rows, csv_id, x, z, scale):
    add_pair(simple, physics, csv_id, "../plateLava.x", "res/model/plateLava.x",
             x, 0.42, z, scale=scale, load_type="meshmix2", physics_type="NonCollision")
    lava_rows.append(["stage28-lava-%02d" % (csv_id - 2809), csv_id, 20])


def build_boss_version():
    if not (BASE / "stage_ground.x").exists():
        raise RuntimeError("stage_ground.x is missing; run build_stage_2_8_ground.py with Blender first")

    simple = [SIMPLE_HEADER,
              [1, "../ground/stage_visual_ground_world2.x", 0, -8, 0, 0, 0, 0, 1, "meshmix2"]]
    physics = [PHYSICS_HEADER]
    add_pair(simple, physics, 2, "stage_ground.x", "res/model/stage_2_8/stage_ground.x",
             0, 0, 0, load_type="meshmix2")

    rocks = ((-31, -33, 25, 1.1, 1), (31, -31, 145, 1.0, 2),
             (-32, 31, 220, 1.0, 2), (31, 33, 315, 1.1, 1))
    rock_id = 2801
    for x, z, rotation_y, scale, variant in rocks:
        render_model = "../base/base_rock1.x"
        physics_model = "res/model/base/base_rock1_collision.x"
        if variant == 2:
            render_model = "../base/base_rock2.x"
            physics_model = "res/model/base/base_rock2_collision.x"
        add_pair(simple, physics, rock_id, render_model, physics_model,
                 x, 0.4, z, rotation_y=rotation_y, scale=scale)
        rock_id += 1

    lava_rows = [["ID", "PhysicsID", "Damage"]]
    for csv_id, x, z in ((2809, -24, -18), (2810, 24, -18),
                         (2811, -24, 14), (2812, 24, 14), (2813, 0, 34)):
        add_lava_pair(simple, physics, lava_rows, csv_id, x, z, 1.5)
    simple.append([2899, "../SkySphere_cave/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 0, 1, "normal"])
    return simple, physics, lava_rows


def build_cleared_version():
    if not (BASE / "stage_ground_cleared.x").exists():
        raise RuntimeError("stage_ground_cleared.x is missing; run build_stage_2_8_ground.py with Blender first")

    simple = [SIMPLE_HEADER,
              [1, "../ground/stage_visual_ground_world2.x", 0, -8, 0, 0, 0, 0, 1, "meshmix2"]]
    physics = [PHYSICS_HEADER]
    add_pair(simple, physics, 2, "stage_ground_cleared.x",
             "res/model/stage_2_8/stage_ground_cleared.x", 0, 0, 0, load_type="meshmix2")
    add_pair(simple, physics, 2850, "../tree2/lemonTree.x",
             "res/model/tree2Physics/tree_cylinder_collision.x", -52, 0.9, 0)

    lava_rows = [["ID", "PhysicsID", "Damage"]]
    for csv_id, x, z in ((2861, -26, -27), (2862, 26, -27),
                         (2863, -26, 27), (2864, 26, 27)):
        add_lava_pair(simple, physics, lava_rows, csv_id, x, z, 1.0)
    simple.append([2899, "../SkySphere_cave/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 1, "normal"])
    return simple, physics, lava_rows


def empty_rows():
    return {
        "XFileListMove.csv": [MOVE_HEADER],
        "Collectibles.csv": [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"]],
        "Interactables.csv": [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"]],
        "Stars.csv": [["PosX", "PosY", "PosZ"]],
        "SpeedUps.csv": [["PosX", "PosY", "PosZ"]],
        "Destructibles.csv": [["PosX", "PosY", "PosZ", "HP", "DropItemId"]],
        "DashBoosters.csv": [["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ", "Speed", "Duration", "Radius", "Scale"]],
        "LavaFlood.csv": [["ID", "Damage", "AnchorX", "AnchorY", "AnchorZ", "DirectionZ", "StartWidth", "StartLength", "EndWidth", "EndLength", "Duration"]],
        "LavaRise.csv": [["ID", "Damage", "MinX", "MaxX", "MinZ", "MaxZ", "StartY", "EndY", "Delay", "Duration"]],
        "Skulls.csv": [["ID", "PosX", "PosY", "PosZ", "RotY"]],
        "PressurePlates.csv": [["ID", "PlatePosX", "PlatePosY", "PlatePosZ", "WallID", "WallRotX", "WallRotY", "WallRotZ", "WallScale"]],
        "PushableBoxes.csv": [["ID", "PosX", "PosY", "PosZ", "RotY", "Scale"]],
        "AttackTriggers.csv": [["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis", "BaseRotX", "BaseRotY", "BaseRotZ", "Scale"]],
        "WarpBears.csv": [["WarpID", "PairID", "PosX", "PosY", "PosZ", "RotY"]],
    }


def main():
    BASE.mkdir(parents=True, exist_ok=True)
    boss_simple, boss_physics, boss_lava = build_boss_version()
    cleared_simple, cleared_physics, cleared_lava = build_cleared_version()

    boss_enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
                    ["boss_golem", 0, 0.9, 5, 180]]
    cleared_enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
                       ["small_spider", -12, 0.9, -29, 30],
                       ["spider", 12, 0.9, -25, 210],
                       ["small_golem", -27, 0.9, -12, 0],
                       ["small_spider", -24, 0.9, 2, 180],
                       ["spider", 27, 0.9, -10, 0],
                       ["small_golem", 24, 0.9, 8, 180],
                       ["small_spider", -10, 0.9, 0, 90],
                       ["spider", 10, 0.9, 0, 270],
                       ["small_golem", -26, 0.9, 25, 0],
                       ["small_spider", 25, 0.9, 29, 180],
                       ["spider", -8, 0.9, 43, 0],
                       ["small_golem", 8, 0.9, 45, 180]]

    cleared_collectibles = [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
                            ["stage28-I01", "Item", "006", -52, 1.0, 4, 1],
                            ["stage28-I02", "Item", "009", 52, 1.0, 0, 1],
                            ["stage28-I03", "Item", "010", 0, 1.0, 0, 1],
                            ["stage28-I04", "Item", "014", 0, 1.0, 44, 1]]
    cleared_interactables = [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"],
                             ["stage28-tree-01", "Tree", -52, 0.9, 0, 2.5]]
    cleared_destructibles = [["PosX", "PosY", "PosZ", "HP", "DropItemId"],
                             [-6, 1.1, -28, 2, "None"], [6, 1.1, -28, 2, "006"],
                             [-28, 1.1, -5, 2, "None"], [28, 1.1, 5, 2, "None"],
                             [-8, 1.1, 0, 2, "009"], [8, 1.1, 0, 2, "None"],
                             [-5, 1.1, 28, 2, "None"], [5, 1.1, 43, 3, "014"]]

    boss_lights = [["PosX", "PosY", "PosZ", "Brightness", "ColorR", "ColorG", "ColorB", "ColorA",
                    "Shape", "LineLength", "SquareWidth", "SquareHeight", "RotX", "RotY", "RotZ", "Range", "OwnerTag"],
                   [0, 4, -42, 1.0, 0.2, 0.55, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage28-start"],
                   [-28, 4, -20, 1.1, 1.0, 0.18, 0.05, 1.0, "Point", 12, 10, 10, 0, 0, 0, 15, "stage28-lava1"],
                   [28, 4, -20, 1.1, 1.0, 0.18, 0.05, 1.0, "Point", 12, 10, 10, 0, 0, 0, 15, "stage28-lava2"],
                   [0, 6, 5, 1.3, 1.0, 0.08, 0.02, 1.0, "Point", 12, 10, 10, 0, 0, 0, 20, "stage28-boss"],
                   [0, 4, 36, 1.0, 1.0, 0.18, 0.05, 1.0, "Point", 12, 10, 10, 0, 0, 0, 15, "stage28-north"]]
    cleared_lights = [boss_lights[0],
                      [-26, 4, 0, 1.0, 1.0, 0.20, 0.05, 1.0, "Point", 12, 10, 10, 0, 0, 0, 14, "stage28-cleared-west"],
                      [26, 4, 0, 1.0, 1.0, 0.20, 0.05, 1.0, "Point", 12, 10, 10, 0, 0, 0, 14, "stage28-cleared-east"],
                      [-52, 4, 0, 1.0, 0.25, 0.7, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage28-cleared-qte"],
                      [0, 4, 44, 1.0, 0.15, 0.75, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage28-cleared-north"]]

    write_csv("XFileList_simple.csv", boss_simple)
    write_csv("XFileListPhysics.csv", boss_physics)
    write_csv("EnemyPositions.csv", boss_enemies)
    write_csv("LavaZones.csv", boss_lava)
    write_csv("PointLights.csv", boss_lights)
    for filename, rows in empty_rows().items():
        write_csv(filename, rows)

    write_csv("XFileList_simpleCleared.csv", cleared_simple)
    write_csv("XFileListPhysicsCleared.csv", cleared_physics)
    write_csv("EnemyPositionsCleared.csv", cleared_enemies)
    write_csv("CollectiblesCleared.csv", cleared_collectibles)
    write_csv("InteractablesCleared.csv", cleared_interactables)
    write_csv("DestructiblesCleared.csv", cleared_destructibles)
    write_csv("LavaZonesCleared.csv", cleared_lava)
    write_csv("PointLightsCleared.csv", cleared_lights)
    for filename, rows in empty_rows().items():
        if filename in {"Collectibles.csv", "Interactables.csv", "Destructibles.csv"}:
            continue
        cleared_filename = filename[:-4] + "Cleared.csv"
        if cleared_filename in {"LavaZonesCleared.csv", "PointLightsCleared.csv"}:
            continue
        write_csv(cleared_filename, rows)


if __name__ == "__main__":
    main()
