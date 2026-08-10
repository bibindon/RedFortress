# -*- coding: utf-8 -*-
"""ステージ2-3のS字経路と区間追従型の迫る溶岩を検証する。"""

import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_3"
START = (-20.0, -54.0)
GOAL = (-20.0, 54.0)
VALID_ENEMY_TYPES = {"small_spider", "spider", "small_golem"}
EXPECTED_DIRECTIONS = ((0.0, 1.0), (1.0, 0.0), (0.0, 1.0), (-1.0, 0.0), (0.0, 1.0))
COURSE_RECTS = (
    (-20.0, -39.5, 12.0, 29.0), (0.0, -25.0, 40.0, 12.0),
    (20.0, -7.5, 12.0, 35.0), (0.0, 10.0, 40.0, 12.0),
    (-20.0, 32.0, 12.0, 44.0), (-20.0, -54.0, 16.0, 10.0),
    (-20.0, 54.0, 16.0, 10.0),
)


def read_csv(filename):
    with (STAGE_DIR / filename).open(encoding="utf-8-sig", newline="") as input_file:
        return list(csv.DictReader(input_file))


def is_on_course(x, z, padding=0.0):
    for center_x, center_z, width_x, depth_z in COURSE_RECTS:
        if (abs(x - center_x) <= width_x * 0.5 + padding and
                abs(z - center_z) <= depth_z * 0.5 + padding):
            return True
    return False


def check_text_format(path, bom_required, errors):
    data = path.read_bytes()
    if bom_required and not data.startswith(b"\xef\xbb\xbf"):
        errors.append("UTF-8 BOM is missing: %s" % path.name)
    if not bom_required and data.startswith(b"\xef\xbb\xbf"):
        errors.append("unexpected UTF-8 BOM: %s" % path.name)
    if b"\n" in data.replace(b"\r\n", b""):
        errors.append("LF-only line ending: %s" % path.name)


def main():
    errors = []
    simple = read_csv("XFileList_simple.csv")
    physics = read_csv("XFileListPhysics.csv")
    floods = read_csv("LavaFlood.csv")
    enemies = read_csv("EnemyPositions.csv")

    simple_ids = {row["ID"] for row in simple}
    physics_ids = {row["ID"] for row in physics}
    if "2" not in simple_ids or "2" not in physics_ids:
        errors.append("dedicated stage ground must use render/physics ID 2")
    backdrop_rows = [row for row in simple if row["ID"] == "1"]
    if not backdrop_rows or float(backdrop_rows[0]["PosY"]) > -5.0:
        errors.append("the visual backdrop must stay well below the S-curve")

    for row in simple:
        if not (STAGE_DIR / row["FileName"]).exists():
            errors.append("render model is missing: %s" % row["FileName"])
    for row in physics:
        if not (ROOT / row["FileName"]).exists():
            errors.append("physics model is missing: %s" % row["FileName"])

    if len(floods) != 5:
        errors.append("S-curve requires exactly five lava flood segments")
    previous_end_time = None
    previous_end = None
    for index, flood in enumerate(floods):
        direction_x = float(flood["DirectionX"])
        direction_z = float(flood["DirectionZ"])
        direction_length = math.hypot(direction_x, direction_z)
        if abs(direction_length - 1.0) > 0.0001:
            errors.append("lava direction must be normalized: %s" % flood["ID"])
        if index < len(EXPECTED_DIRECTIONS):
            if (direction_x, direction_z) != EXPECTED_DIRECTIONS[index]:
                errors.append("lava segment direction order does not form the S-curve")

        delay = float(flood["Delay"])
        duration = float(flood["Duration"])
        if duration <= 0.0:
            errors.append("lava duration must be positive")
        if previous_end_time is not None and abs(delay - previous_end_time) > 0.001:
            errors.append("lava segments must activate without a timing gap")
        previous_end_time = delay + duration

        anchor_x = float(flood["AnchorX"])
        anchor_z = float(flood["AnchorZ"])
        end_length = float(flood["EndLength"])
        end = (anchor_x + direction_x * end_length,
               anchor_z + direction_z * end_length)
        if previous_end is not None:
            if math.hypot(anchor_x - previous_end[0], anchor_z - previous_end[1]) > 4.0:
                errors.append("adjacent lava segments do not overlap at a corner")
        previous_end = end
        if float(flood["EndWidth"]) < 12.0:
            errors.append("lava must cover the complete corridor width")

    if floods:
        if float(floods[0]["Delay"]) < 8.0:
            errors.append("the player needs at least eight seconds before the chase starts")
        final_end_time = float(floods[-1]["Delay"]) + float(floods[-1]["Duration"])
        if final_end_time < 180.0:
            errors.append("the complete chase is too short")

    if len(enemies) != 13:
        errors.append("enemy count must be exactly 13")
    for enemy in enemies:
        enemy_type = enemy["Type"]
        if enemy_type == "bird":
            errors.append("bird must not be placed in stage 2-3")
        if enemy_type not in VALID_ENEMY_TYPES:
            errors.append("invalid World 2 enemy type: %s" % enemy_type)
        x = float(enemy["PosX"])
        z = float(enemy["PosZ"])
        if not is_on_course(x, z):
            errors.append("enemy is outside the S-curve")
        if math.hypot(x - START[0], z - START[1]) < 7.0:
            errors.append("enemy is too close to start")
        if math.hypot(x - GOAL[0], z - GOAL[1]) < 7.0:
            errors.append("enemy is too close to goal")

    for filename in ("Collectibles.csv", "Destructibles.csv"):
        for row in read_csv(filename):
            x = float(row["PosX"])
            z = float(row["PosZ"])
            if not is_on_course(x, z, 0.5):
                errors.append("%s object is outside the S-curve" % filename)

    for collectible in read_csv("Collectibles.csv"):
        data_id = collectible["DataID"]
        if len(data_id) != 3 or not data_id.isdigit():
            errors.append("collectible DataID must be zero-padded: %s" % data_id)

    if read_csv("LavaZones.csv") or read_csv("LavaRise.csv"):
        errors.append("stage 2-3 must use only LavaFlood.csv")

    ground_x = STAGE_DIR / "stage_ground.x"
    if not ground_x.exists():
        errors.append("stage_ground.x is missing")
    else:
        check_text_format(ground_x, False, errors)
        if not ground_x.read_bytes().startswith(b"xof "):
            errors.append("stage_ground.x is not a DirectX X text file")

    csv_filenames = (
        "XFileList_simple.csv", "XFileListPhysics.csv", "XFileListMove.csv",
        "EnemyPositions.csv", "Collectibles.csv", "Destructibles.csv", "LavaZones.csv",
        "LavaFlood.csv", "LavaRise.csv", "DashBoosters.csv", "Interactables.csv",
        "AttackTriggers.csv", "PressurePlates.csv", "PushableBoxes.csv", "PointLights.csv",
        "Skulls.csv", "Stars.csv", "SpeedUps.csv", "WarpBears.csv",
    )
    for filename in csv_filenames:
        path = STAGE_DIR / filename
        if not path.exists():
            errors.append("required stage file is missing: %s" % filename)
            continue
        check_text_format(path, True, errors)

    if errors:
        print("FAIL")
        for error in errors:
            print(" -", error)
        return 1

    print("PASS: stage 2-3 S-curve lava chase checks OK")
    print(" route=188m flood_segments=5 enemies=13 birds=0 chase_end=192s")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
