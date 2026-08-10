# -*- coding: utf-8 -*-
"""ステージ2-1の全面ダメージ床、鳥配置、必須ギミックを検証する。"""

import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_1"
START = (-20.0, -54.0)
GOAL = (20.0, 54.0)

PATH_SEGMENTS = (
    ((-20.0, -54.0), (-20.0, -43.0)),
    ((-20.0, -43.0), (-23.0, -34.0)),
    ((-23.0, -34.0), (-18.5, -29.5)),
    ((-12.0, -25.0), (-5.0, -15.0)),
    ((-5.0, -15.0), (-22.0, -5.0)),
    ((-22.0, -5.0), (0.0, 0.0)),
    ((0.0, 0.0), (19.0, 5.0)),
    ((19.0, 5.0), (22.0, 16.0)),
    ((22.0, 16.0), (4.0, 25.0)),
    ((4.0, 25.0), (-18.0, 35.0)),
    ((-18.0, 35.0), (5.0, 45.0)),
    ((5.0, 45.0), (20.0, 54.0)),
    ((-5.0, -15.0), (26.0, -18.0)),
    ((4.0, 25.0), (-24.0, 20.0)),
)

SAFE_PLATFORMS = (
    (-20.0, -54.0, 4.0, 4.0), (-20.0, -43.0, 2.5, 2.5),
    (-23.0, -34.0, 4.0, 4.0), (-12.0, -25.0, 2.5, 2.5),
    (-5.0, -15.0, 4.5, 4.0), (-22.0, -5.0, 4.5, 4.0),
    (19.0, 5.0, 4.5, 4.0), (22.0, 16.0, 4.0, 4.0),
    (4.0, 25.0, 4.0, 3.5), (-18.0, 35.0, 5.0, 4.5),
    (5.0, 45.0, 3.0, 3.0), (20.0, 54.0, 4.0, 4.0),
    (26.0, -18.0, 4.0, 4.0), (-24.0, 20.0, 3.5, 3.5),
    (-26.0, 13.0, 3.0, 3.0),
)


def read_csv(filename):
    with (STAGE_DIR / filename).open(encoding="utf-8-sig", newline="") as input_file:
        return list(csv.DictReader(input_file))


def distance_to_segment(point, start, end):
    point_x, point_z = point
    start_x, start_z = start
    end_x, end_z = end
    delta_x = end_x - start_x
    delta_z = end_z - start_z
    squared_length = delta_x * delta_x + delta_z * delta_z
    if squared_length <= 0.0:
        return math.hypot(point_x - start_x, point_z - start_z)
    projection = ((point_x - start_x) * delta_x + (point_z - start_z) * delta_z) / squared_length
    projection = max(0.0, min(1.0, projection))
    closest_x = start_x + delta_x * projection
    closest_z = start_z + delta_z * projection
    return math.hypot(point_x - closest_x, point_z - closest_z)


def on_safe_platform(x, z):
    for center_x, center_z, half_x, half_z in SAFE_PLATFORMS:
        if abs(x - center_x) <= half_x and abs(z - center_z) <= half_z:
            return True
    return False


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
    lava_rows = read_csv("LavaZones.csv")

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

    for row in moves:
        if row["RenderID"] not in simple_ids or row["PhysicsID"] not in physics_ids:
            errors.append("moving platform reference is missing")
        if row["RenderID"] != row["PhysicsID"]:
            errors.append("moving platform IDs do not match")
    if len(moves) != 1:
        errors.append("exactly one moving platform is required")

    # 60x120mを、20m四方のplateLava 3x6枚で完全に覆う。
    expected_centers = {(x, z) for x in (-20, 0, 20) for z in (-50, -30, -10, 10, 30, 50)}
    lava_physics = [row for row in physics if "platelava" in row["FileName"].lower()]
    actual_centers = {(int(float(row["PosX"])), int(float(row["PosZ"]))) for row in lava_physics}
    if len(lava_physics) != 18 or actual_centers != expected_centers:
        errors.append("damage floor must be a complete 3x6 grid of 18 plates")
    lava_ids = {row["ID"] for row in lava_physics}
    for row in lava_physics:
        if abs(float(row["Scale"]) - 2.5) > 0.001:
            errors.append("lava plate %s must use Scale=2.5" % row["ID"])
        if row["Type"] != "NonCollision":
            errors.append("lava plate %s must use NonCollision" % row["ID"])
    if len(lava_rows) != 18:
        errors.append("LavaZones.csv must reference all 18 damage plates")
    for row in lava_rows:
        if row["PhysicsID"] not in lava_ids:
            errors.append("lava PhysicsID %s is missing" % row["PhysicsID"])

    # 鳥は安全島ではなく、細道から索敵距離14m以内のダメージ床上に置く。
    if len(enemies) != 15:
        errors.append("enemy count must be exactly 15, got %d" % len(enemies))
    for enemy in enemies:
        if enemy["Type"] != "bird":
            errors.append("stage 2-1 enemy must be bird: %s" % enemy["Type"])
        x = float(enemy["PosX"])
        y = float(enemy["PosY"])
        z = float(enemy["PosZ"])
        if x <= -30.0 or x >= 30.0 or z <= -60.0 or z >= 60.0:
            errors.append("bird is outside the 60x120m play area: (%s,%s)" % (enemy["PosX"], enemy["PosZ"]))
        if y < 3.0:
            errors.append("bird must start above the damage floor: (%s,%s,%s)" %
                          (enemy["PosX"], enemy["PosY"], enemy["PosZ"]))
        if on_safe_platform(x, z):
            errors.append("bird must not start on a safe island: (%s,%s)" % (enemy["PosX"], enemy["PosZ"]))
        nearest_path = min(distance_to_segment((x, z), start, end) for start, end in PATH_SEGMENTS)
        if nearest_path > 10.0:
            errors.append("bird is too far from every narrow path: (%s,%s) %.1fm" %
                          (enemy["PosX"], enemy["PosZ"], nearest_path))
        if math.hypot(x - START[0], z - START[1]) < 7.0:
            errors.append("bird is too close to start")
        if math.hypot(x - GOAL[0], z - GOAL[1]) < 7.0:
            errors.append("bird is too close to goal")

    if len(triggers) != 1 or triggers[0]["TargetID"] != "2303":
        errors.append("the high Lever3 must target gate 2303")
    elif triggers[0]["TargetID"] not in simple_ids or triggers[0]["TargetID"] not in physics_ids:
        errors.append("Lever3 target 2303 is missing")

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
        errors.append("stage_ground.x is not an official DirectX X text file")
    if x_data.startswith(b"\xef\xbb\xbf"):
        errors.append("stage_ground.x must not contain a BOM")
    if b"\n" in x_data.replace(b"\r\n", b""):
        errors.append("stage_ground.x contains LF-only line endings")

    csv_filenames = (
        "XFileList_simple.csv", "XFileListPhysics.csv", "XFileListMove.csv",
        "EnemyPositions.csv", "Collectibles.csv", "Destructibles.csv", "LavaZones.csv",
        "DashBoosters.csv", "Interactables.csv", "AttackTriggers.csv", "PointLights.csv",
        "PressurePlates.csv", "PushableBoxes.csv", "Skulls.csv", "Stars.csv",
        "SpeedUps.csv", "WarpBears.csv",
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

    print("PASS: stage 2-1 narrow-path bird checks OK")
    print(" main_zones=12 birds=15 lava=18 dead_ends=2 moving_platforms=1")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
