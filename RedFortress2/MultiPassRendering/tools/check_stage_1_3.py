# -*- coding: utf-8 -*-
"""ステージ1-3 の静的チェック（1-7設計書の13項目チェック相当を手動実装）。
重複ID / 描画↔物理整合 / 移動床3CSV連携 / AttackTriggers TargetID /
PressurePlates WallID / 敵数・型 / 収集物DataID / ギミック最小数 / 開始・ゴール7m以内に敵なし。"""
import csv
import math
import os

DIR = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "res", "model", "stage_1_3"))


def read(name):
    with open(os.path.join(DIR, name), encoding="utf-8-sig", newline="") as f:
        return list(csv.DictReader(f))


def main():
    errors = []
    simple = read("XFileList_simple.csv")
    physics = read("XFileListPhysics.csv")
    move = read("XFileListMove.csv")
    plates = read("PressurePlates.csv")

    # 1) 重複ID
    def dup(rows, label):
        ids = [r["ID"] for r in rows]
        seen = set()
        for i in ids:
            if i in seen:
                errors.append("duplicate ID %s in %s" % (i, label))
            seen.add(i)
    dup(simple, "simple")
    dup(physics, "physics")

    # 2) 描画↔物理整合（物理IDは描画に存在。描画専用ID帯 5xxx/8xxx/9xxx と地面・空・移動床は除外）
    simple_ids = {r["ID"] for r in simple}
    for r in physics:
        if r["ID"] not in simple_ids:
            errors.append("physics ID %s missing in simple" % r["ID"])
    for r in simple:
        i = r["ID"]
        if i in {s["ID"] for s in physics}:
            continue
        fn = r["FileName"].lower()
        if "ground" in fn or "skysphere" in fn or "collision_moving_platform" in fn:
            continue
        if r.get("loadType", "").strip().lower() == "instancing":
            continue
        if i.startswith(("5", "8", "9")):
            continue
        errors.append("render-only ID %s not allowed (%s)" % (i, r["FileName"]))

    # 3) 移動床3CSV連携
    for r in move:
        rid, pid = r["RenderID"], r["PhysicsID"]
        if rid not in simple_ids:
            errors.append("move RenderID %s missing in simple" % rid)
        if pid not in {s["ID"] for s in physics}:
            errors.append("move PhysicsID %s missing in physics" % pid)
        same = [s for s in simple if s["ID"] == rid]
        if same and same[0]["FileName"].lower().find("collision_moving_platform") < 0:
            errors.append("move RenderID %s is not a moving platform" % rid)
    if len(move) < 1:
        errors.append("no moving platform rows")

    # 4) AttackTriggers TargetID が描画・物理両方に存在（-1はボタン限定）
    for r in read("AttackTriggers.csv"):
        if r["TargetID"] == "-1":
            if r["Type"] != "Button":
                errors.append("TargetID=-1 only allowed for Button")
            continue
        if r["TargetID"] not in simple_ids:
            errors.append("trigger TargetID %s missing in simple" % r["TargetID"])
        if r["TargetID"] not in {s["ID"] for s in physics}:
            errors.append("trigger TargetID %s missing in physics" % r["TargetID"])
        phys = [s for s in physics if s["ID"] == r["TargetID"]]
        if phys and phys[0].get("Move", "").lower() != "y":
            errors.append("trigger target %s physics Move != y" % r["TargetID"])

    # 5) PressurePlates WallID が描画・物理両方に存在
    for r in plates:
        wid = r["WallID"]
        if wid not in simple_ids:
            errors.append("plate WallID %s missing in simple" % wid)
        if wid not in {s["ID"] for s in physics}:
            errors.append("plate WallID %s missing in physics" % wid)
        phys = [s for s in physics if s["ID"] == wid]
        if phys and phys[0].get("Move", "").lower() != "y":
            errors.append("plate wall %s physics Move != y" % wid)

    # 感圧板3は各扉の外側2枚と箱内中央1枚を同じ扉へ接続する。
    expected_plate_positions = {
        (-10.0, 0.71, -14.0),
        (-10.0, 0.71, -18.0),
        (-10.0, 0.71, -22.0),
    }
    actual_plate_positions = {
        (float(r["PlatePosX"]), float(r["PlatePosY"]), float(r["PlatePosZ"]))
        for r in plates
        if r["WallID"] == "9114"
    }
    if len(plates) != 3:
        errors.append("PressurePlate3 must have exactly three plates")
    if any(r["WallID"] != "9114" for r in plates):
        errors.append("all PressurePlate3 plates must target wall 9114")
    if actual_plate_positions != expected_plate_positions:
        errors.append("PressurePlate3 must have two outside plates and one plate at the box center")

    # 6) 敵数・型・開始/ゴール7m以内に敵なし
    enemies = read("EnemyPositions.csv")
    valid_types = {"wolf", "small_mushroom", "crab", "frog", "bird", "ghost", "spider", "skeleton", "golem"}
    for e in enemies:
        if e["Type"] not in valid_types:
            errors.append("unknown enemy type %s" % e["Type"])
    if len(enemies) < 10:
        errors.append("enemy count %d < 10" % len(enemies))
    for label, (sx, sz) in (("start", (0.0, 28.0)), ("goal", (0.0, -28.0))):
        for e in enemies:
            d = math.hypot(float(e["PosX"]) - sx, float(e["PosZ"]) - sz)
            if d < 7.0:
                errors.append("enemy %s at (%s,%s) within 7m of %s (%.1fm)" % (e["Type"], e["PosX"], e["PosZ"], label, d))

    # 7) 収集物DataID（007/008禁止）
    for c in read("Collectibles.csv"):
        if c["DataID"] in ("7", "8", "007", "008"):
            errors.append("forbidden DataID %s" % c["DataID"])

    # 8) ギミック最小数
    checks = [
        ("QTE tree", len(read("Interactables.csv")) >= 1, "Interactables.csv"),
        ("dash booster", len(read("DashBoosters.csv")) >= 1, "DashBoosters.csv"),
        ("lever/rope/button", len(read("AttackTriggers.csv")) >= 1, "AttackTriggers.csv"),
        ("pressure plate", len(plates) >= 1, "PressurePlates.csv"),
        ("destructibles", len(read("Destructibles.csv")) >= 1, "Destructibles.csv"),
        ("star", len(read("Stars.csv")) >= 1, "Stars.csv"),
    ]
    for label, ok, src in checks:
        if not ok:
            errors.append("missing gimmick: %s (%s)" % (label, src))

    # 9) Y>=3 の動かない床（描画 PosY >= 3 の static_platform）
    high_platforms = [
        r for r in simple
        if float(r.get("PosY", "0")) >= 3.0 and "static_platform" in r["FileName"].lower()
    ]
    if not high_platforms:
        errors.append("no static platform at Y>=3")

    if errors:
        print("FAIL:")
        for e in errors:
            print(" -", e)
        return 1
    print("PASS: all static checks OK (enemies=%d, collectibles=%d, destructibles=%d)" % (
        len(enemies), len(read("Collectibles.csv")), len(read("Destructibles.csv"))))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
