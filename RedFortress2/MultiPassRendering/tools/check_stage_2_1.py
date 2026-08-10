# -*- coding: utf-8 -*-
"""ステージ2-1の配置要件とCSV参照整合性を検証する。"""

import csv
import math
from pathlib import Path


STAGE_DIR = Path(__file__).resolve().parents[1] / "res" / "model" / "stage_2_1"
START = (-48.0, -52.0)
GOAL = (-48.0, 52.0)


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

    valid_enemy_types = {"small_spider", "spider", "small_golem"}
    if len(enemies) < 15:
        errors.append("enemy count %d is below World 2 minimum 15" % len(enemies))
    for enemy in enemies:
        if enemy["Type"] not in valid_enemy_types:
            errors.append("invalid World 2 enemy type %s" % enemy["Type"])
        enemy_x = float(enemy["PosX"])
        enemy_z = float(enemy["PosZ"])
        for label, position in (("start", START), ("goal", GOAL)):
            distance = math.hypot(enemy_x - position[0], enemy_z - position[1])
            if distance < 7.0:
                errors.append("enemy at (%s,%s) is %.1fm from %s" %
                              (enemy["PosX"], enemy["PosZ"], distance, label))

    for collectible in collectibles:
        data_id = collectible["DataID"]
        if len(data_id) != 3 or not data_id.isdigit():
            errors.append("collectible DataID %s is not zero-padded" % data_id)

    if len(read_csv("LavaZones.csv")) < 4:
        errors.append("damage-floor bands are missing")
    if len(moves) < 1:
        errors.append("moving platform is missing")
    if len(read_csv("DashBoosters.csv")) < 1:
        errors.append("dash booster is missing")
    if len(read_csv("Interactables.csv")) < 1:
        errors.append("QTE tree is missing")
    if len(triggers) < 3:
        errors.append("lever2/lever3 triggers are missing")

    high_platforms = []
    for row in simple:
        if "static_platform" not in row["FileName"].lower():
            continue
        if float(row["PosY"]) >= 3.0:
            high_platforms.append(row)
    if len(high_platforms) < 1:
        errors.append("static platform at Y>=3 is missing")

    edge_gimmicks = 0
    for row in read_csv("Interactables.csv") + read_csv("Collectibles.csv"):
        x = float(row["PosX"])
        z = float(row["PosZ"])
        if abs(x) >= 50.0 or abs(z) >= 50.0:
            edge_gimmicks += 1
    if edge_gimmicks < 2:
        errors.append("fewer than two edge gimmicks")

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

    print("PASS: stage 2-1 static checks OK")
    print(" enemies=%d lava=%d collectibles=%d edge_gimmicks=%d" %
          (len(enemies), len(read_csv("LavaZones.csv")), len(collectibles), edge_gimmicks))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
