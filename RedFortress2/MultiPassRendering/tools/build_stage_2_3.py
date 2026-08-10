# -*- coding: utf-8 -*-
"""ステージ2-3「追ってくるマグマの道」のS字コース配置CSVを生成する。"""

import csv
import io
from pathlib import Path


BASE = Path(__file__).resolve().parents[1] / "res" / "model" / "stage_2_3"

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


def build_render_and_physics():
    if not (BASE / "stage_ground.x").exists():
        raise RuntimeError("stage_ground.x is missing; run build_stage_2_3_ground.py with Blender first")

    simple = [SIMPLE_HEADER,
              [1, "../ground/stage_visual_ground_world2.x", 0, -8, 0, 0, 0, 0, 1, "meshmix2"],
              [2, "stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"]]
    physics = [PHYSICS_HEADER,
               [2, "res/model/stage_2_3/stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""]]

    rocks = (
        (-28, -47, 20, 1.0, 1), (-12, -35, 160, 1.1, 2),
        (-10, -33, 250, 1.0, 1), (10, -17, 70, 1.1, 2),
        (12, 1, 190, 1.0, 1), (10, 18, 300, 1.1, 2),
        (-12, 26, 35, 1.0, 2), (-28, 42, 215, 1.1, 1),
    )
    rock_id = 3201
    for x, z, rotation_y, scale, variant in rocks:
        model = "../base/base_rock1.x"
        if variant == 2:
            model = "../base/base_rock2.x"
        simple.append([rock_id, model, x, -7.6, z, 0, rotation_y, 0, scale, "normal"])
        rock_id += 1
    simple.append([3299, "../SkySphere_cave/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 0, 1, "normal"])
    return simple, physics


def main():
    BASE.mkdir(parents=True, exist_ok=True)
    simple, physics = build_render_and_physics()

    enemies = [["Type", "PosX", "PosY", "PosZ", "RotY"],
               ["small_spider", -22, 0.9, -44, 0],
               ["spider", -18, 0.9, -34, 180],
               ["small_golem", -10, 0.9, -25, 90],
               ["small_spider", 2, 0.9, -27, 270],
               ["spider", 13, 0.9, -23, 90],
               ["small_spider", 20, 0.9, -15, 180],
               ["small_golem", 18, 0.9, -4, 0],
               ["small_spider", 13, 0.9, 10, 270],
               ["spider", 2, 0.9, 12, 90],
               ["small_golem", -10, 0.9, 8, 270],
               ["small_spider", -20, 0.9, 21, 180],
               ["spider", -22, 0.9, 33, 0],
               ["small_golem", -18, 0.9, 44, 180]]

    collectibles = [["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
                    ["stage23-I01", "Item", "004", -20, 1.0, -30, 1],
                    ["stage23-I02", "Item", "009", 20, 1.0, -8, 1],
                    ["stage23-I03", "Item", "010", -2, 1.0, 10, 1],
                    ["stage23-I04", "Item", "014", -20, 1.0, 39, 1]]

    destructibles = [["PosX", "PosY", "PosZ", "HP", "DropItemId"],
                     [-23, 1.1, -38, 2, "None"], [-17, 1.1, -38, 2, "None"],
                     [-3, 1.1, -27, 2, "006"], [7, 1.1, -23, 2, "None"],
                     [17, 1.1, 4, 2, "None"], [3, 1.1, 8, 2, "009"],
                     [-23, 1.1, 27, 2, "None"], [-17, 1.1, 40, 2, "None"]]

    # 各区間は直前の区間が角へ到達した瞬間に起動する。
    lava_flood = [["ID", "Damage", "AnchorX", "AnchorY", "AnchorZ",
                   "DirectionX", "DirectionZ", "StartWidth", "StartLength",
                   "EndWidth", "EndLength", "Delay", "Duration"],
                  ["stage23-flood-01", 20, -20, 0.42, -59, 0, 1, 14, 2, 14, 36, 8, 32],
                  ["stage23-flood-02", 20, -22, 0.42, -25, 1, 0, 14, 2, 14, 44, 40, 37],
                  ["stage23-flood-03", 20, 20, 0.42, -27, 0, 1, 14, 2, 14, 39, 77, 32],
                  ["stage23-flood-04", 20, 22, 0.42, 10, -1, 0, 14, 2, 14, 44, 109, 37],
                  ["stage23-flood-05", 20, -20, 0.42, 8, 0, 1, 14, 2, 14, 51, 146, 46]]

    point_lights = [["PosX", "PosY", "PosZ", "Brightness", "ColorR", "ColorG", "ColorB", "ColorA",
                     "Shape", "LineLength", "SquareWidth", "SquareHeight", "RotX", "RotY", "RotZ", "Range", "OwnerTag"],
                    [-20, 3.0, -54, 1.0, 0.2, 0.55, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage23-start"],
                    [-20, 3.0, -25, 0.9, 1.0, 0.25, 0.08, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage23-turn1"],
                    [20, 3.0, -25, 0.9, 1.0, 0.25, 0.08, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage23-turn2"],
                    [20, 3.0, 10, 0.9, 1.0, 0.25, 0.08, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage23-turn3"],
                    [-20, 3.0, 10, 0.9, 1.0, 0.25, 0.08, 1.0, "Point", 12, 10, 10, 0, 0, 0, 12, "stage23-turn4"],
                    [-20, 3.0, 54, 1.0, 0.15, 0.7, 1.0, 1.0, "Point", 12, 10, 10, 0, 0, 0, 10, "stage23-goal"]]

    empty_files = {
        "XFileListMove.csv": [MOVE_HEADER],
        "LavaZones.csv": [["ID", "PhysicsID", "Damage"]],
        "LavaRise.csv": [["ID", "Damage", "MinX", "MaxX", "MinZ", "MaxZ", "StartY", "EndY", "Delay", "Duration"]],
        "DashBoosters.csv": [["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ", "Speed", "Duration", "Radius", "Scale"]],
        "Interactables.csv": [["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"]],
        "AttackTriggers.csv": [["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis", "BaseRotX", "BaseRotY", "BaseRotZ", "Scale"]],
        "PressurePlates.csv": [["ID", "PlatePosX", "PlatePosY", "PlatePosZ", "WallID", "WallRotX", "WallRotY", "WallRotZ", "WallScale"]],
        "PushableBoxes.csv": [["ID", "PosX", "PosY", "PosZ", "RotY", "Scale"]],
        "Skulls.csv": [["ID", "PosX", "PosY", "PosZ", "RotY"]],
        "Stars.csv": [["PosX", "PosY", "PosZ"]],
        "SpeedUps.csv": [["PosX", "PosY", "PosZ"]],
        "WarpBears.csv": [["WarpID", "PairID", "PosX", "PosY", "PosZ", "RotY"]],
    }

    write_csv("XFileList_simple.csv", simple)
    write_csv("XFileListPhysics.csv", physics)
    write_csv("EnemyPositions.csv", enemies)
    write_csv("Collectibles.csv", collectibles)
    write_csv("Destructibles.csv", destructibles)
    write_csv("LavaFlood.csv", lava_flood)
    write_csv("PointLights.csv", point_lights)
    for filename, rows in empty_files.items():
        write_csv(filename, rows)


if __name__ == "__main__":
    main()
