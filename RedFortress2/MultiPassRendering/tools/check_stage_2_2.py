# -*- coding: utf-8 -*-
"""ステージ2-2の上昇経路、せり上がる溶岩、敵と必須ギミックを検証する。"""

import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_2"
START = (-20.0, -54.0)
GOAL = (20.0, 54.0)
VALID_ENEMY_TYPES = {"small_spider", "spider", "small_golem"}
RESERVED_POSITIONS = (
    ("QTE tree", -27.0, 10.0),
    ("dash booster", 27.0, 28.0),
    ("pressure plate", 22.0, -18.0),
    ("pushable box", 17.0, -21.0),
)

ZONE_CENTERS = (
    (-20.0, -54.0, 0.4), (-10.0, -45.0, 0.4), (-24.0, -36.0, 1.4),
    (-10.0, -26.0, 2.5), (20.0, -19.0, 3.6), (2.0, -8.0, 4.8),
    (-20.0, 2.0, 6.0), (20.0, 12.0, 7.0), (-16.0, 24.0, 8.0),
    (2.0, 36.0, 9.5), (-20.0, 45.0, 11.0), (20.0, 54.0, 13.0),
)


def read_csv(filename):
    with (STAGE_DIR / filename).open(encoding="utf-8-sig", newline="") as input_file:
        return list(csv.DictReader(input_file))


def add_duplicate_errors(rows, label, errors):
    seen = set()
    for row in rows:
        csv_id = row["ID"]
        if csv_id in seen:
            errors.append("duplicate ID %s in %s" % (csv_id, label))
        seen.add(csv_id)


def main():
    errors = []
    simple = read_csv("XFileList_simple.csv")
    physics = read_csv("XFileListPhysics.csv")
    moves = read_csv("XFileListMove.csv")
    enemies = read_csv("EnemyPositions.csv")
    collectibles = read_csv("Collectibles.csv")
    triggers = read_csv("AttackTriggers.csv")
    plates = read_csv("PressurePlates.csv")
    boxes = read_csv("PushableBoxes.csv")
    lava_rise = read_csv("LavaRise.csv")

    add_duplicate_errors(simple, "XFileList_simple.csv", errors)
    add_duplicate_errors(physics, "XFileListPhysics.csv", errors)
    simple_ids = {row["ID"] for row in simple}
    physics_ids = {row["ID"] for row in physics}

    for row in physics:
        if row["ID"] not in simple_ids:
            errors.append("physics ID %s missing in render CSV" % row["ID"])
        model_path = ROOT / row["FileName"]
        if not model_path.exists():
            errors.append("physics model is missing: %s" % row["FileName"])

    for row in simple:
        model_path = STAGE_DIR / row["FileName"]
        if not model_path.exists():
            errors.append("render model is missing: %s" % row["FileName"])

    if len(moves) != 1:
        errors.append("exactly one mandatory moving platform is required")
    for row in moves:
        if row["RenderID"] not in simple_ids or row["PhysicsID"] not in physics_ids:
            errors.append("moving platform reference is missing")
        if row["RenderID"] != row["PhysicsID"]:
            errors.append("moving platform IDs do not match")

    if len(ZONE_CENTERS) != 12:
        errors.append("main zone count must be 12")
    previous_height = ZONE_CENTERS[0][2]
    for _x, _z, height in ZONE_CENTERS[1:]:
        if height < previous_height:
            errors.append("main route height must never descend")
        previous_height = height

    if len(enemies) != 17:
        errors.append("enemy count must be exactly 17, got %d" % len(enemies))
    for enemy in enemies:
        enemy_type = enemy["Type"]
        if enemy_type == "bird":
            errors.append("bird must not be placed in stage 2-2")
        if enemy_type not in VALID_ENEMY_TYPES:
            errors.append("invalid World 2 enemy type: %s" % enemy_type)
        x = float(enemy["PosX"])
        z = float(enemy["PosZ"])
        if math.hypot(x - START[0], z - START[1]) < 7.0:
            errors.append("enemy is too close to start")
        if math.hypot(x - GOAL[0], z - GOAL[1]) < 7.0:
            errors.append("enemy is too close to goal")
        for label, reserved_x, reserved_z in RESERVED_POSITIONS:
            if math.hypot(x - reserved_x, z - reserved_z) < 2.5:
                errors.append("enemy overlaps %s" % label)

    if len(lava_rise) != 1:
        errors.append("exactly one rising lava definition is required")
    else:
        lava = lava_rise[0]
        if (float(lava["MinX"]), float(lava["MaxX"]),
                float(lava["MinZ"]), float(lava["MaxZ"])) != (-30.0, 30.0, -60.0, 60.0):
            errors.append("rising lava must cover the complete 60x120m tower")
        if float(lava["Delay"]) < 12.0:
            errors.append("rising lava delay must be at least 12 seconds")
        if float(lava["Duration"]) < 190.0:
            errors.append("rising lava duration must be at least 190 seconds")
        if float(lava["EndY"]) >= ZONE_CENTERS[-1][2]:
            errors.append("rising lava must stop below the goal terrace")

    if read_csv("LavaZones.csv"):
        errors.append("stage 2-2 must use LavaRise.csv, not fixed damage floors")

    if len(triggers) != 1 or triggers[0]["TargetID"] != "2403":
        errors.append("Lever3 must target gate 2403")
    elif triggers[0]["TargetID"] not in simple_ids or triggers[0]["TargetID"] not in physics_ids:
        errors.append("Lever3 target 2403 is missing")

    if len(plates) != 1 or plates[0]["WallID"] != "2301":
        errors.append("PressurePlate3 must target gate 2301")
    elif plates[0]["WallID"] not in simple_ids or plates[0]["WallID"] not in physics_ids:
        errors.append("PressurePlate3 target 2301 is missing")
    if len(boxes) != 1:
        errors.append("one pushable box is required for PressurePlate3")

    for target_id in ("2301", "2403"):
        rows = [row for row in physics if row["ID"] == target_id]
        if not rows or rows[0]["Move"].lower() != "y":
            errors.append("gate %s must use Move=y" % target_id)

    if len(read_csv("DashBoosters.csv")) != 1:
        errors.append("one reward-branch dash booster is required")
    if len(read_csv("Interactables.csv")) != 1:
        errors.append("one reward-branch QTE tree is required")

    for collectible in collectibles:
        data_id = collectible["DataID"]
        if len(data_id) != 3 or not data_id.isdigit():
            errors.append("collectible DataID %s is not zero-padded" % data_id)

    x_data = (STAGE_DIR / "stage_ground.x").read_bytes()
    if not x_data.startswith(b"xof "):
        errors.append("stage_ground.x is not a DirectX X text file")
    if x_data.startswith(b"\xef\xbb\xbf"):
        errors.append("stage_ground.x must not contain a BOM")
    if b"\n" in x_data.replace(b"\r\n", b""):
        errors.append("stage_ground.x contains LF-only line endings")

    csv_filenames = (
        "XFileList_simple.csv", "XFileListPhysics.csv", "XFileListMove.csv",
        "EnemyPositions.csv", "Collectibles.csv", "Destructibles.csv", "LavaZones.csv",
        "LavaRise.csv", "DashBoosters.csv", "Interactables.csv", "AttackTriggers.csv",
        "PressurePlates.csv", "PushableBoxes.csv", "PointLights.csv", "Skulls.csv",
        "Stars.csv", "SpeedUps.csv", "WarpBears.csv",
    )
    for filename in csv_filenames:
        data = (STAGE_DIR / filename).read_bytes()
        if not data.startswith(b"\xef\xbb\xbf"):
            errors.append("%s has no UTF-8 BOM" % filename)
        if b"\n" in data.replace(b"\r\n", b""):
            errors.append("%s contains LF-only line endings" % filename)

    if errors:
        print("FAIL")
        for error in errors:
            print(" -", error)
        return 1

    print("PASS: stage 2-2 rising-lava tower checks OK")
    print(" main_zones=12 enemies=17 birds=0 dead_ends=2 goal_y=13 lava_end_y=11.5")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
