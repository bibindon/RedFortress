# -*- coding: utf-8 -*-
"""ステージ2-4のCSV間整合性と主要な設計条件を検査する。"""

import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_4"
START_XZ = (-50.0, -50.0)
ALLOWED_ENEMIES = {"small_spider", "spider", "small_golem", "bird"}


def read_csv(filename):
    with (STAGE_DIR / filename).open("r", encoding="utf-8-sig", newline="") as source:
        return list(csv.DictReader(source))


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


def distance_xz(a, b):
    return math.hypot(float(a[0]) - float(b[0]), float(a[1]) - float(b[1]))


def main():
    simple = read_csv("XFileList_simple.csv")
    physics = read_csv("XFileListPhysics.csv")
    moving = read_csv("XFileListMove.csv")
    enemies = read_csv("EnemyPositions.csv")
    collectibles = read_csv("Collectibles.csv")
    triggers = read_csv("AttackTriggers.csv")

    simple_ids = {int(row["ID"]) for row in simple}
    physics_ids = {int(row["ID"]) for row in physics}
    require(len(simple_ids) == len(simple), "描画CSVに重複IDがあります")
    require(len(physics_ids) == len(physics), "物理CSVに重複IDがあります")
    require(1 in physics_ids, "外周衝突がありません")
    require(simple_ids - {9000} == physics_ids - {1}, "描画・物理CSVの対応IDが一致しません")
    require(all("stage_ground" not in row["FileName"] for row in simple), "地面描画が残っています")
    require(all("stage_ground" not in row["FileName"] for row in physics), "地面衝突が残っています")
    for row in simple:
        require((STAGE_DIR / row["FileName"]).resolve().is_file(),
                "描画モデルがありません: " + row["FileName"])
    for row in physics:
        require((ROOT / row["FileName"]).resolve().is_file(),
                "物理モデルがありません: " + row["FileName"])

    require(len(moving) == 17, "移動床は17台必要です")
    for row in moving:
        render_id = int(row["RenderID"])
        physics_id = int(row["PhysicsID"])
        require(render_id in simple_ids, "移動床の描画IDがありません: " + str(render_id))
        require(physics_id in physics_ids, "移動床の物理IDがありません: " + str(physics_id))
        require(abs(float(row["StartY"]) - float(row["EndY"])) < 0.0001,
                "上下移動する床があります: " + row["ID"])

    require(len(enemies) >= 15, "World 2の最低敵数を満たしていません")
    require(all(row["Type"] in ALLOWED_ENEMIES for row in enemies), "World 2で使用できない敵がいます")
    for row in enemies:
        enemy_xz = (row["PosX"], row["PosZ"])
        require(distance_xz(enemy_xz, START_XZ) >= 7.0, "開始・ゴール付近に敵がいます")

    require(len(collectibles) >= 5, "分岐報酬が不足しています")
    require(len(triggers) >= 1, "レバー2がありません")
    for row in triggers:
        require(row["Type"] == "LeverLift", "レバー2以外の必須トリガーが混在しています")
        target_id = int(row["TargetID"])
        require(target_id in simple_ids and target_id in physics_ids,
                "レバー対象が描画・物理CSVの両方にありません")

    platform_rows = [row for row in simple if "static_platform" in row["FileName"]]
    platform_points = [(float(row["PosX"]), float(row["PosY"]), float(row["PosZ"]))
                       for row in platform_rows]
    require(any(distance_xz((x, z), START_XZ) < 0.1 and abs(y - 0.35) < 0.01
                for x, y, z in platform_points), "南西の開始足場がありません")
    require(any(distance_xz((x, z), START_XZ) < 0.1 and y >= 29.0
                for x, y, z in platform_points), "南西上空のゴール足場がありません")

    for row in moving:
        for prefix in ("Start", "End"):
            endpoint = (float(row[prefix + "X"]), float(row[prefix + "Z"]))
            endpoint_y = float(row[prefix + "Y"]) - 0.3
            near_platform = any(distance_xz(endpoint, (x, z)) <= 8.0 and abs(endpoint_y - y) < 0.01
                                for x, y, z in platform_points)
            require(near_platform, "移動床の端に乗降足場がありません: " + row["ID"] + " " + prefix)

    print("stage_2_4 validation passed")
    print("moving platforms:", len(moving))
    print("enemies:", len(enemies))
    print("collectibles:", len(collectibles))
    print("static platforms:", len(platform_rows))


if __name__ == "__main__":
    main()
