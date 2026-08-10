# -*- coding: utf-8 -*-
"""ステージ2-2の4陸地、折り返し経路、高低差、CSV整合性を検証する。"""

import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_2"
START = (-52.0, -49.0)
GOAL = (52.0, 49.0)
MAX_JUMP_DISTANCE = 4.1
MAX_JUMP_HEIGHT = 1.28


def read_csv(filename):
    with (STAGE_DIR / filename).open(encoding="utf-8-sig", newline="") as input_file:
        return list(csv.DictReader(input_file))


def surface(name, x, y, z, half_x, half_z):
    return {"name": name, "x": x, "y": y, "z": z, "half_x": half_x, "half_z": half_z}


def gap(first, second):
    gap_x = max(0.0, abs(first["x"] - second["x"]) - first["half_x"] - second["half_x"])
    gap_z = max(0.0, abs(first["z"] - second["z"]) - first["half_z"] - second["half_z"])
    return math.hypot(gap_x, gap_z)


def check_route(route, errors, label):
    for first, second in zip(route, route[1:]):
        distance = gap(first, second)
        height = abs(first["y"] - second["y"])
        if distance > MAX_JUMP_DISTANCE + 0.001:
            errors.append("%s gap %.2fm exceeds jump: %s -> %s" %
                          (label, distance, first["name"], second["name"]))
        if height > MAX_JUMP_HEIGHT + 0.001:
            errors.append("%s height %.2fm exceeds jump: %s -> %s" %
                          (label, height, first["name"], second["name"]))


def point_on_surface(x, z, candidates, margin=0.0):
    for candidate in candidates:
        if abs(x - candidate["x"]) <= candidate["half_x"] - margin:
            if abs(z - candidate["z"]) <= candidate["half_z"] - margin:
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
    lava_zones = read_csv("LavaZones.csv")

    simple_ids = {row["ID"] for row in simple}
    physics_ids = {row["ID"] for row in physics}
    if len(simple_ids) != len(simple):
        errors.append("duplicate ID in XFileList_simple.csv")
    if len(physics_ids) != len(physics):
        errors.append("duplicate ID in XFileListPhysics.csv")
    for row in physics:
        if row["ID"] not in simple_ids:
            errors.append("physics ID %s missing in render CSV" % row["ID"])

    # 全面ダメージ床は20m四方の6x6グリッドで外周まで覆う。
    expected_lava = {(x, z) for x in (-50, -30, -10, 10, 30, 50)
                     for z in (-50, -30, -10, 10, 30, 50)}
    lava_physics = [row for row in physics if "platelava" in row["FileName"].lower()]
    actual_lava = {(int(float(row["PosX"])), int(float(row["PosZ"]))) for row in lava_physics}
    if len(lava_physics) != 36 or actual_lava != expected_lava:
        errors.append("damage floor must be a complete 6x6 grid of 36 plates")
    for row in lava_physics:
        if abs(float(row["Scale"]) - 2.5) > 0.001 or row["Type"] != "NonCollision":
            errors.append("lava plate %s has invalid scale or type" % row["ID"])
    if len(lava_zones) != 36:
        errors.append("LavaZones.csv must reference all 36 damage plates")
    lava_ids = {row["ID"] for row in lava_physics}
    for row in lava_zones:
        if row["PhysicsID"] not in lava_ids:
            errors.append("lava PhysicsID %s is missing" % row["PhysicsID"])

    # 4つの大きな正方形陸地。
    land_a = surface("land-A", -46, 0.85, -42, 12, 12)
    land_b = surface("land-B-high", -25, 3.7, 5, 12, 12)
    land_c = surface("land-C", 10, 0.85, -38, 12, 12)
    land_d = surface("land-D", 45, 0.85, 40, 12, 12)
    expected_land_tiles = {(-52, -48, 0.35), (-40, -48, 0.35), (-52, -36, 0.35), (-40, -36, 0.35),
                           (-31, -1, 3.2), (-19, -1, 3.2), (-31, 11, 3.2), (-19, 11, 3.2),
                           (4, -44, 0.35), (16, -44, 0.35), (4, -32, 0.35), (16, -32, 0.35),
                           (39, 34, 0.35), (51, 34, 0.35), (39, 46, 0.35), (51, 46, 0.35)}
    actual_land_tiles = set()
    for row in simple:
        if "static_platform_4x4" not in row["FileName"].lower():
            continue
        key = (int(float(row["PosX"])), int(float(row["PosZ"])), float(row["PosY"]))
        if key in expected_land_tiles:
            actual_land_tiles.add(key)
    if actual_land_tiles != expected_land_tiles:
        errors.append("the four 24x24m land masses are incomplete")

    a1 = surface("ascent-1", -42, 1.3, -25.5, 1.5, 1.5)
    a2 = surface("ascent-2", -39, 1.7, -19, 3, 3)
    a3 = surface("ascent-3", -35, 2.5, -12.5, 3, 3)
    a4 = surface("ascent-4", -30, 3.3, -6, 3, 3)
    d1 = surface("descent-1", -15, 2.9, -12, 3, 3)
    d2 = surface("descent-2", -9, 2.1, -19, 3, 3)
    d3 = surface("descent-3", -3, 1.3, -26, 3, 3)
    gate = surface("lever3-gate", 27, 0.95, -27, 5, 2.5)
    wait = surface("moving-wait", 30, 0.85, -23, 1.5, 3)
    moving_start = surface("moving-start", 34, 0.85, -22, 1.5, 1.5)
    moving_end = surface("moving-end", 33, 2.4, -14, 1.5, 1.5)
    landing = surface("east-landing", 38, 2.4, -13, 3, 3)
    rise2 = surface("east-rise-2", 32, 2.8, -6, 3, 3)
    rise3 = surface("east-rise-3", 24, 3.2, 1, 3, 3)
    turn = surface("northwest-turn", 17, 3.7, 7, 6, 6)
    down1 = surface("final-down-1", 20, 2.9, 16, 1.5, 1.5)
    down2 = surface("final-down-2", 26, 2.1, 22, 3, 3)
    down3 = surface("final-down-3", 34, 1.3, 28, 3, 3)

    first_route = [land_a, a1, a2, a3, a4, land_b, d1, d2, d3, land_c, gate, wait, moving_start]
    second_route = [moving_end, landing, rise2, rise3, turn, down1, down2, down3, land_d]
    check_route(first_route, errors, "main route before lift")
    check_route(second_route, errors, "main route after lift")

    # 移動床は高さ1.55mを運び、終点と高台を重ねず隣へジャンプできる。
    if len(moves) != 1:
        errors.append("exactly one moving platform is required")
    else:
        row = moves[0]
        if row["RenderID"] != "6001" or row["PhysicsID"] != "6001":
            errors.append("moving platform IDs must both be 6001")
        if "6001" not in simple_ids or "6001" not in physics_ids:
            errors.append("moving platform 6001 is missing from render or physics CSV")
        move_target_height = float(row["EndY"]) + 0.2
        if abs(move_target_height - landing["y"]) > 0.01:
            errors.append("moving platform top does not align with east landing")
        if gap(moving_end, landing) <= 0.0:
            errors.append("moving platform end overlaps elevated landing and may crush the player")
        if gap(moving_end, landing) > MAX_JUMP_DISTANCE:
            errors.append("moving platform end is too far from elevated landing")

    qte_branch = [land_a,
                  surface("qte-stone-1", -52, 0.85, -25, 1.5, 1.5),
                  surface("qte-stone-2", -54, 0.85, -19, 1.5, 1.5),
                  surface("qte-end", -54, 0.85, -10, 6, 6)]
    north_branch = [land_b,
                    surface("north-1", -27, 3.7, 23, 1.5, 3),
                    surface("north-2", -28, 3.7, 29, 1.5, 3),
                    surface("north-3", -28, 3.7, 35, 1.5, 3),
                    surface("north-4", -28, 3.7, 41, 1.5, 3),
                    surface("north-end", -28, 3.7, 51, 6, 6)]
    south_branch = [land_c,
                    surface("south-1", -5, 1.3, -51, 3, 3),
                    surface("south-2", -10, 2.1, -52, 3, 3),
                    surface("south-3", -15, 2.9, -53, 3, 3),
                    surface("south-end", -22, 3.7, -54, 6, 6)]
    west_branch = [turn,
                   surface("west-1", 8, 3.7, 12, 3, 3), surface("west-2", 0, 3.7, 16, 3, 3),
                   surface("west-3", -8, 3.7, 19, 3, 3), surface("west-4", -16, 3.7, 22, 3, 3),
                   surface("west-5", -24, 3.7, 24, 3, 3), surface("west-6", -32, 3.7, 26, 3, 3),
                   surface("west-7", -40, 3.7, 27, 3, 3), surface("west-end", -52, 3.7, 27, 6, 6)]
    check_route(qte_branch, errors, "QTE dead end")
    check_route(north_branch, errors, "north dead end")
    check_route(south_branch, errors, "Lever2 dead end")
    check_route(west_branch, errors, "west dead end")

    # 進行方向は北上、南下、東進、北西折り返し、北東ゴールの順。
    section_points = [START, (-25, 5), (10, -38), (38, -13), (17, 7), GOAL]
    if not section_points[1][1] > section_points[0][1]:
        errors.append("route does not first travel north")
    if not section_points[2][1] < section_points[1][1]:
        errors.append("route does not descend south after land B")
    if not section_points[3][0] > section_points[2][0]:
        errors.append("route does not travel east after land C")
    if not section_points[4][0] < section_points[3][0]:
        errors.append("route does not turn back northwest")
    if not (GOAL[0] > section_points[4][0] and GOAL[1] > section_points[4][1]):
        errors.append("goal is not northeast of the final turn")

    combat_surfaces = [land_a, land_b, land_c, land_d,
                       surface("north-end", -28, 3.7, 51, 6, 6),
                       surface("west-end", -52, 3.7, 27, 6, 6)]
    valid_enemy_types = {"small_spider", "spider", "small_golem"}
    if len(enemies) != 21:
        errors.append("enemy count must be exactly 21, got %d" % len(enemies))
    for enemy in enemies:
        enemy_x = float(enemy["PosX"])
        enemy_z = float(enemy["PosZ"])
        if enemy["Type"] not in valid_enemy_types:
            errors.append("invalid World 2 enemy type %s" % enemy["Type"])
        if not point_on_surface(enemy_x, enemy_z, combat_surfaces, margin=0.5):
            errors.append("enemy at (%s,%s) is not on a combat land" %
                          (enemy["PosX"], enemy["PosZ"]))
        for label, position in (("start", START), ("goal", GOAL)):
            distance = math.hypot(enemy_x - position[0], enemy_z - position[1])
            if distance < 7.0:
                errors.append("enemy at (%s,%s) is %.1fm from %s" %
                              (enemy["PosX"], enemy["PosZ"], distance, label))

    for collectible in collectibles:
        if len(collectible["DataID"]) != 3 or not collectible["DataID"].isdigit():
            errors.append("collectible DataID %s is not zero-padded" % collectible["DataID"])

    for row in triggers:
        target_id = row["TargetID"]
        if target_id not in simple_ids or target_id not in physics_ids:
            errors.append("trigger target %s is missing" % target_id)
            continue
        target = [entry for entry in physics if entry["ID"] == target_id][0]
        if target["Move"].lower() != "y":
            errors.append("trigger target %s must use Move=y" % target_id)
    if len([row for row in triggers if row["TargetID"] == "9103"]) != 2:
        errors.append("Lever3 must have two triggers sharing target 9103")
    if len([row for row in triggers if row["TargetID"] == "9003"]) != 1:
        errors.append("Lever2 trigger for target 9003 is missing")

    if len(read_csv("Interactables.csv")) != 1:
        errors.append("one QTE tree is required")
    if len(read_csv("DashBoosters.csv")) != 1:
        errors.append("one dash booster is required")

    edge_gimmicks = 0
    for row in read_csv("Interactables.csv") + collectibles:
        if abs(float(row["PosX"])) >= 50.0 or abs(float(row["PosZ"])) >= 50.0:
            edge_gimmicks += 1
    if edge_gimmicks < 2:
        errors.append("fewer than two edge gimmicks")

    if not point_on_surface(START[0], START[1], [land_a], margin=3.0):
        errors.append("start is not safely inside southwest land")
    if not point_on_surface(GOAL[0], GOAL[1], [land_d], margin=3.0):
        errors.append("goal is not safely inside northeast land")

    for filename in ("XFileList_simple.csv", "XFileListPhysics.csv", "XFileListMove.csv",
                     "EnemyPositions.csv", "Collectibles.csv", "Destructibles.csv",
                     "LavaZones.csv", "DashBoosters.csv", "Interactables.csv",
                     "AttackTriggers.csv", "PointLights.csv"):
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

    print("PASS: stage 2-2 folded-route checks OK")
    print(" zones=12 lands=4 enemies=%d lava=%d dead_ends=4 edge_gimmicks=%d" %
          (len(enemies), len(lava_zones), edge_gimmicks))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
