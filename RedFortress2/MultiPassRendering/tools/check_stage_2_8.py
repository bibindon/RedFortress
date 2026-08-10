# -*- coding: utf-8 -*-
"""ステージ2-8のボスあり版・クリア後版の分離と配置を検証する。"""

import csv
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_8"
VALID_CLEARED_ENEMIES = {"small_spider", "spider", "small_golem"}
CLEARED_FLOORS = (
    (0.0, -42.0, 30.0, 20.0), (0.0, -27.0, 70.0, 14.0),
    (-26.0, 0.0, 18.0, 40.0), (26.0, 0.0, 18.0, 40.0),
    (0.0, 0.0, 52.0, 8.0), (0.0, 27.0, 70.0, 14.0),
    (0.0, 42.0, 30.0, 20.0), (-43.5, 0.0, 17.0, 8.0),
    (-52.0, 0.0, 8.0, 14.0), (43.5, 0.0, 17.0, 8.0),
    (52.0, 0.0, 8.0, 14.0),
)


def read_csv(filename):
    with (STAGE_DIR / filename).open(encoding="utf-8-sig", newline="") as input_file:
        return list(csv.DictReader(input_file))


def check_model_references(simple_name, physics_name, errors):
    simple = read_csv(simple_name)
    physics = read_csv(physics_name)
    simple_ids = {row["ID"] for row in simple}
    physics_ids = {row["ID"] for row in physics}
    if "2" not in simple_ids or "2" not in physics_ids:
        errors.append("%s/%s must contain dedicated ground ID 2" % (simple_name, physics_name))
    for row in simple:
        if not (STAGE_DIR / row["FileName"]).exists():
            errors.append("render model is missing in %s: %s" % (simple_name, row["FileName"]))
    for row in physics:
        if not (ROOT / row["FileName"]).exists():
            errors.append("physics model is missing in %s: %s" % (physics_name, row["FileName"]))
    for row in physics:
        if row["ID"] not in simple_ids:
            errors.append("physics ID %s has no render pair in %s" % (row["ID"], simple_name))


def check_text_format(path, bom_required, errors):
    data = path.read_bytes()
    if bom_required and not data.startswith(b"\xef\xbb\xbf"):
        errors.append("UTF-8 BOM is missing: %s" % path.name)
    if not bom_required and data.startswith(b"\xef\xbb\xbf"):
        errors.append("unexpected UTF-8 BOM: %s" % path.name)
    if b"\n" in data.replace(b"\r\n", b""):
        errors.append("LF-only line ending: %s" % path.name)


def floors_connect(first, second):
    first_x, first_z, first_width, first_depth = first
    second_x, second_z, second_width, second_depth = second
    overlap_x = min(first_x + first_width * 0.5, second_x + second_width * 0.5) - \
        max(first_x - first_width * 0.5, second_x - second_width * 0.5)
    overlap_z = min(first_z + first_depth * 0.5, second_z + second_depth * 0.5) - \
        max(first_z - first_depth * 0.5, second_z - second_depth * 0.5)
    if overlap_x >= 0.0 and overlap_z >= 1.0:
        return True
    if overlap_z >= 0.0 and overlap_x >= 1.0:
        return True
    return False


def cleared_floor_is_connected():
    reached = {0}
    pending = [0]
    while pending:
        current = pending.pop()
        for index, floor in enumerate(CLEARED_FLOORS):
            if index in reached:
                continue
            if floors_connect(CLEARED_FLOORS[current], floor):
                reached.add(index)
                pending.append(index)
    return len(reached) == len(CLEARED_FLOORS)


def main():
    errors = []
    if not cleared_floor_is_connected():
        errors.append("cleared-version floor rectangles are not fully connected")
    boss_enemies = read_csv("EnemyPositions.csv")
    cleared_enemies = read_csv("EnemyPositionsCleared.csv")
    if len(boss_enemies) != 1 or boss_enemies[0]["Type"] != "boss_golem":
        errors.append("boss version must contain only one boss_golem")
    if len(cleared_enemies) != 12:
        errors.append("cleared version must contain exactly 12 normal enemies")
    for enemy in cleared_enemies:
        if enemy["Type"] not in VALID_CLEARED_ENEMIES:
            errors.append("invalid cleared-version enemy: %s" % enemy["Type"])

    boss_forbidden = (
        "Collectibles.csv", "Interactables.csv", "Destructibles.csv", "Stars.csv",
        "SpeedUps.csv", "DashBoosters.csv", "Skulls.csv", "PressurePlates.csv",
        "PushableBoxes.csv", "AttackTriggers.csv", "WarpBears.csv", "XFileListMove.csv",
    )
    for filename in boss_forbidden:
        if read_csv(filename):
            errors.append("boss version must keep %s empty" % filename)

    for filename in ("DashBoostersCleared.csv", "AttackTriggersCleared.csv"):
        if read_csv(filename):
            errors.append("%s is forbidden in both boss-stage versions" % filename)

    if len(read_csv("InteractablesCleared.csv")) != 1:
        errors.append("cleared version requires exactly one QTE tree")
    if len(read_csv("CollectiblesCleared.csv")) != 4:
        errors.append("cleared version requires exactly four collectibles")
    if len(read_csv("DestructiblesCleared.csv")) != 8:
        errors.append("cleared version requires exactly eight destructibles")

    boss_lava = read_csv("LavaZones.csv")
    cleared_lava = read_csv("LavaZonesCleared.csv")
    if len(boss_lava) != 5:
        errors.append("boss arena requires five damage-floor plates")
    if len(cleared_lava) != 4:
        errors.append("cleared ruins require four damage-floor plates")

    check_model_references("XFileList_simple.csv", "XFileListPhysics.csv", errors)
    check_model_references("XFileList_simpleCleared.csv", "XFileListPhysicsCleared.csv", errors)

    boss_ground = STAGE_DIR / "stage_ground.x"
    cleared_ground = STAGE_DIR / "stage_ground_cleared.x"
    for ground in (boss_ground, cleared_ground):
        if not ground.exists():
            errors.append("ground is missing: %s" % ground.name)
            continue
        check_text_format(ground, False, errors)
        if not ground.read_bytes().startswith(b"xof "):
            errors.append("invalid DirectX X ground: %s" % ground.name)
    if boss_ground.exists() and cleared_ground.exists():
        if boss_ground.read_bytes() == cleared_ground.read_bytes():
            errors.append("boss and cleared grounds must be different")

    required_base_files = (
        "XFileList_simple.csv", "XFileListPhysics.csv", "XFileListMove.csv",
        "EnemyPositions.csv", "Collectibles.csv", "Interactables.csv", "Stars.csv",
        "SpeedUps.csv", "Destructibles.csv", "DashBoosters.csv", "LavaZones.csv",
        "LavaFlood.csv", "LavaRise.csv", "Skulls.csv", "PressurePlates.csv",
        "PushableBoxes.csv", "AttackTriggers.csv", "WarpBears.csv", "PointLights.csv",
    )
    cleared_files = tuple(filename[:-4] + "Cleared.csv" for filename in required_base_files)
    required_files = required_base_files + cleared_files + ("EnemyPositionsCleared.csv",)
    for filename in required_files:
        path = STAGE_DIR / filename
        if not path.exists():
            errors.append("required version file is missing: %s" % filename)
            continue
        check_text_format(path, True, errors)

    for collectible in read_csv("CollectiblesCleared.csv"):
        data_id = collectible["DataID"]
        if len(data_id) != 3 or not data_id.isdigit():
            errors.append("collectible DataID must be zero-padded: %s" % data_id)

    if errors:
        print("FAIL")
        for error in errors:
            print(" -", error)
        return 1

    print("PASS: stage 2-8 boss/cleared variant checks OK")
    print(" boss=boss_golem only cleared_enemies=12 boss_lava=5 cleared_lava=4")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
