# -*- coding: utf-8 -*-
"""ステージ2-1の配置要件、全面ダメージ床、通常ジャンプ経路を検証する。"""

import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_1"
START = (-50.0, -50.0)
GOAL = (-51.0, 53.0)
MAX_JUMP_DISTANCE = 4.1
MAX_JUMP_HEIGHT = 1.28


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


def make_surface(name, x, y, z, half_x, half_z):
    return {"name": name, "x": x, "y": y, "z": z, "half_x": half_x, "half_z": half_z}


def horizontal_gap(first, second):
    gap_x = abs(first["x"] - second["x"]) - first["half_x"] - second["half_x"]
    gap_z = abs(first["z"] - second["z"]) - first["half_z"] - second["half_z"]
    gap_x = max(0.0, gap_x)
    gap_z = max(0.0, gap_z)
    return math.hypot(gap_x, gap_z)


def check_route(route, errors, label):
    for first, second in zip(route, route[1:]):
        gap = horizontal_gap(first, second)
        height = abs(first["y"] - second["y"])
        if gap > MAX_JUMP_DISTANCE + 0.001:
            errors.append("%s gap %.2fm exceeds jump: %s -> %s" %
                          (label, gap, first["name"], second["name"]))
        if height > MAX_JUMP_HEIGHT + 0.001:
            errors.append("%s height %.2fm exceeds jump: %s -> %s" %
                          (label, height, first["name"], second["name"]))


def point_on_any_surface(x, z, surfaces, margin=0.0):
    for surface in surfaces:
        if abs(x - surface["x"]) <= surface["half_x"] - margin:
            if abs(z - surface["z"]) <= surface["half_z"] - margin:
                return True
    return False


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

    for row in moves:
        if row["RenderID"] not in simple_ids:
            errors.append("moving platform render ID %s is missing" % row["RenderID"])
        if row["PhysicsID"] not in physics_ids:
            errors.append("moving platform physics ID %s is missing" % row["PhysicsID"])
        if row["RenderID"] != row["PhysicsID"]:
            errors.append("moving platform IDs do not match")

    for row in triggers:
        target_id = row["TargetID"]
        if target_id not in simple_ids or target_id not in physics_ids:
            errors.append("trigger target %s is missing" % target_id)
            continue
        target_physics = [entry for entry in physics if entry["ID"] == target_id]
        if target_physics[0]["Move"].lower() != "y":
            errors.append("trigger target %s must use Move=y" % target_id)

    # plateLavaは半径4m×Scale。Scale=2.5を20m間隔で並べ、外周まで完全に覆う。
    expected_centers = {(x, z) for x in (-50, -30, -10, 10, 30, 50)
                        for z in (-50, -30, -10, 10, 30, 50)}
    lava_physics = [row for row in physics if "platelava" in row["FileName"].lower()]
    actual_centers = {(int(float(row["PosX"])), int(float(row["PosZ"]))) for row in lava_physics}
    if len(lava_physics) != 36 or actual_centers != expected_centers:
        errors.append("damage floor must be a complete 6x6 grid of 36 plates")
    for row in lava_physics:
        if abs(float(row["Scale"]) - 2.5) > 0.001:
            errors.append("lava plate %s must use Scale=2.5" % row["ID"])
        if row["Type"] != "NonCollision":
            errors.append("lava plate %s must use NonCollision" % row["ID"])
    if len(lava_rows) != 36:
        errors.append("LavaZones.csv must reference all 36 damage plates")
    lava_ids = {row["ID"] for row in lava_physics}
    for row in lava_rows:
        if row["PhysicsID"] not in lava_ids:
            errors.append("lava PhysicsID %s is missing" % row["PhysicsID"])

    p1 = make_surface("zone1", -50, 0.85, -50, 6, 6)
    p2 = make_surface("zone2", -37, 0.85, -39, 6, 6)
    p3 = make_surface("zone3-main", -18, 0.85, -43, 6, 6)
    p3_east = make_surface("zone3-east", -9, 0.85, -40, 3, 3)
    p4 = make_surface("zone4", 8, 0.85, -31, 6, 6)
    p5 = make_surface("zone5-high", 31, 3.7, -44, 6, 6)
    p6 = make_surface("zone6-main", 39, 0.85, -17, 6, 6)
    p6_east = make_surface("zone6-east", 48, 0.85, -14, 3, 3)
    p7 = make_surface("zone7", 32, 0.85, 4, 6, 6)
    p8 = make_surface("zone8", 14, 0.85, 20, 6, 6)
    p9 = make_surface("zone9-main", -11, 0.85, 17, 6, 6)
    p9_east = make_surface("zone9-east", -2, 0.85, 20, 3, 3)
    gate = make_surface("zone10-lever3", -24, 0.95, 27, 5.5, 5.0)
    p11 = make_surface("zone11-main", -42, 0.85, 40, 6, 6)
    p11_east = make_surface("zone11-east", -33, 0.85, 40, 3, 3)
    p12 = make_surface("zone12-goal", -51, 0.85, 53, 6, 6)

    stone = lambda name, x, z: make_surface(name, x, 0.85, z, 1.5, 1.5)
    main_route = [
        p1, p2, stone("south-stone", -27.5, -41), p3, p3_east,
        stone("crossing-a", -3, -36), stone("crossing-b", 0, -36), p4,
        stone("east-a", 18, -27), stone("east-b", 23, -23), stone("east-c", 28, -20), p6,
        stone("north-turn", 35, -6.5), p7, stone("zigzag", 23.5, 11.5), p8,
        stone("center", 4, 20), p9_east, p9, gate, stone("gate-exit", -29.5, 33),
        p11_east, p11, p12,
    ]
    check_route(main_route, errors, "main route")

    high_branch = [
        p4,
        make_surface("stair-1", 17, 1.3, -35, 3, 3),
        make_surface("stair-2", 21.5, 2.1, -38, 3, 3),
        make_surface("stair-3", 26, 2.9, -41, 3, 3),
        p5,
    ]
    east_reward = [
        p6_east, stone("edge-a", 49, -7), stone("edge-b", 52, -4.5),
        make_surface("east-reward", 54, 0.85, -1, 3, 3),
    ]
    qte_branch = [
        p9_east, stone("qte-a", 0, 28.5), stone("qte-b", 3, 33.5),
        stone("qte-c", 6, 38.5), stone("qte-d", 8, 44),
        make_surface("qte-end", 8, 0.85, 52, 6, 6),
    ]
    check_route(high_branch, errors, "high dead end")
    check_route(east_reward, errors, "east dead end")
    check_route(qte_branch, errors, "QTE dead end")

    combat_surfaces = [p3, p3_east, p6, p6_east, p9, p9_east, p11, p11_east]
    valid_enemy_types = {"small_spider", "spider", "small_golem"}
    if len(enemies) != 15:
        errors.append("enemy count must be exactly 15, got %d" % len(enemies))
    for enemy in enemies:
        if enemy["Type"] not in valid_enemy_types:
            errors.append("invalid World 2 enemy type %s" % enemy["Type"])
        enemy_x = float(enemy["PosX"])
        enemy_z = float(enemy["PosZ"])
        if not point_on_any_surface(enemy_x, enemy_z, combat_surfaces, margin=0.5):
            errors.append("enemy at (%s,%s) is not on a combat platform" %
                          (enemy["PosX"], enemy["PosZ"]))
        for label, position in (("start", START), ("goal", GOAL)):
            distance = math.hypot(enemy_x - position[0], enemy_z - position[1])
            if distance < 7.0:
                errors.append("enemy at (%s,%s) is %.1fm from %s" %
                              (enemy["PosX"], enemy["PosZ"], distance, label))

    for collectible in collectibles:
        data_id = collectible["DataID"]
        if len(data_id) != 3 or not data_id.isdigit():
            errors.append("collectible DataID %s is not zero-padded" % data_id)

    if len(moves) < 1:
        errors.append("moving platform is missing")
    if len(read_csv("DashBoosters.csv")) < 1:
        errors.append("dash booster is missing")
    if len(read_csv("Interactables.csv")) < 1:
        errors.append("QTE tree is missing")
    if len(triggers) != 3:
        errors.append("one Lever2 and two-sided Lever3 triggers are required")
    if len([row for row in triggers if row["TargetID"] == "9103"]) != 2:
        errors.append("Lever3 must have two triggers sharing target 9103")

    high_platforms = [row for row in simple
                      if "static_platform" in row["FileName"].lower() and float(row["PosY"]) >= 3.0]
    if len(high_platforms) < 1:
        errors.append("static platform at Y>=3 is missing")

    edge_gimmicks = 0
    for row in read_csv("Interactables.csv") + collectibles:
        x = float(row["PosX"])
        z = float(row["PosZ"])
        if abs(x) >= 50.0 or abs(z) >= 50.0:
            edge_gimmicks += 1
    if edge_gimmicks < 2:
        errors.append("fewer than two edge gimmicks")

    # スタートとゴールは12x12m足場の中央付近に限定する。
    if not point_on_any_surface(START[0], START[1], [p1], margin=3.0):
        errors.append("start is not safely inside zone1")
    if not point_on_any_surface(GOAL[0], GOAL[1], [p12], margin=3.0):
        errors.append("goal is not safely inside zone12")

    for filename in ("XFileList_simple.csv", "XFileListPhysics.csv", "XFileListMove.csv",
                     "EnemyPositions.csv", "Collectibles.csv", "Destructibles.csv",
                     "LavaZones.csv", "DashBoosters.csv", "Interactables.csv",
                     "AttackTriggers.csv", "PointLights.csv"):
        data = (STAGE_DIR / filename).read_bytes()
        if not data.startswith(b"\xef\xbb\xbf"):
            errors.append("%s has no UTF-8 BOM" % filename)
        normalized = data.replace(b"\r\n", b"")
        if b"\n" in normalized:
            errors.append("%s contains LF-only line endings" % filename)

    if errors:
        print("FAIL")
        for error in errors:
            print(" -", error)
        return 1

    print("PASS: stage 2-1 safe-island checks OK")
    print(" zones=12 enemies=%d lava=%d dead_ends=3 edge_gimmicks=%d" %
          (len(enemies), len(lava_rows), edge_gimmicks))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
