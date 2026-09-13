"""Validate stage 1-5 placement and no-input booster landing estimates.

The trajectory estimate follows CharacterMover's 60 Hz Legacy inertia settings
(18 m/s^2 braking, 9.8 m/s^2 gravity). It excludes enemy hits and mesh collision
response; it is a regression check, not a replacement for gameplay testing.
"""
import csv
import math
from pathlib import Path

STAGE = Path(__file__).resolve().parents[1] / "res/model/stage_1_5"


def read(name):
    with (STAGE / name).open(encoding="utf-8-sig", newline="") as stream:
        return list(csv.DictReader(stream))


def position(row):
    return [float(row["Pos" + axis]) for axis in "XYZ"]


def landing(booster, top):
    point = position(booster)
    direction = [float(booster["Dir" + axis]) for axis in "XYZ"]
    magnitude = math.sqrt(sum(value * value for value in direction))
    velocity = [value / magnitude * float(booster["Speed"]) for value in direction]
    remaining = float(booster["Duration"])
    dt = 1 / 60
    # Charging freezes the player, so start at the first launch frame.
    for _ in range(600):
        remaining -= dt
        if remaining <= 0:
            horizontal = math.hypot(velocity[0], velocity[2])
            if horizontal > 0:
                ratio = max(0, horizontal - 18 * dt) / horizontal
                velocity[0] *= ratio
                velocity[2] *= ratio
            velocity[1] -= 9.8 * dt
        previous = point[:]
        point = [value + speed * dt for value, speed in zip(point, velocity)]
        if velocity[1] < 0 and previous[1] >= top and point[1] <= top:
            ratio = (previous[1] - top) / (previous[1] - point[1])
            return [a + (b - a) * ratio for a, b in zip(previous, point)]
    raise AssertionError("No descending crossing: " + booster["DashBoosterID"])


def assert_coasts_away_from_next_booster(booster, landing_point, next_booster):
    direction = [float(booster["Dir" + axis]) for axis in "XYZ"]
    magnitude = math.sqrt(sum(value * value for value in direction))
    speed = float(booster["Speed"])
    velocity = [value / magnitude * speed for value in direction]
    next_position = position(next_booster)
    point = landing_point[:]
    minimum_distance = math.dist(point, next_position)
    dt = 1 / 60
    # With no input, Legacy inertia removes 18 m/s each second after landing.
    # The residual motion must carry the player away from the next trigger.
    for _ in range(60):
        horizontal = math.hypot(velocity[0], velocity[2])
        if horizontal <= 0:
            break
        ratio = max(0, horizontal - 18 * dt) / horizontal
        velocity[0] *= ratio
        velocity[2] *= ratio
        point[0] += velocity[0] * dt
        point[2] += velocity[2] * dt
        distance = math.dist(point, next_position)
        assert distance > float(next_booster["Radius"]) + 0.3, (
            "Coasting enters next booster", booster["DashBoosterID"], next_booster["DashBoosterID"])
        assert distance >= minimum_distance - 0.01, (
            "Coasting approaches next booster", booster["DashBoosterID"], next_booster["DashBoosterID"])
        minimum_distance = distance


def main():
    render = {row["ID"]: row for row in read("XFileList_simple.csv")}
    physics = {row["ID"]: row for row in read("XFileListPhysics.csv")}
    removed = {"9001", "9002", "9003"} | {str(i) for i in range(3101, 3107)}
    assert not removed.intersection(render), "Start barrier remains visible"
    assert not removed.intersection(physics), "Start barrier collision remains"
    assert not read("AttackTriggers.csv"), "Orphaned start levers remain"
    for name in ("XFileList_simple.csv", "XFileListPhysics.csv"):
        rows = read(name)
        assert len(rows) == len({row["ID"] for row in rows}), name
    for key, row in physics.items():
        assert key in render, "Missing render: " + key
        for field in ("PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale"):
            assert float(row[field]) == float(render[key][field]), (key, field)
    for row in read("XFileListMove.csv"):
        assert row["RenderID"] in render and row["PhysicsID"] in physics
        assert physics[row["PhysicsID"]]["Move"] == "y"
    pickups = read("Collectibles.csv") + read("Stars.csv") + read("SpeedUps.csv")
    for i, first in enumerate(pickups):
        for second in pickups[i + 1:]:
            assert math.dist(position(first), position(second)) >= 1.2, (first, second)
    for row in pickups + read("WarpBears.csv"):
        x, _, z = position(row)
        assert abs(x) <= 14.7 and abs(z) <= 29.7, "Outside boundary: " + str(row)
    for row in read("WarpBears.csv"):
        mesh = render[str(4999 + int(row["WarpID"]))]
        assert position(row) == position(mesh), "Warp visual mismatch"
    boosters = read("DashBoosters.csv")
    targets = [str(i) for i in range(3601, 3611)] + ["3612", "3613"]
    sources = [None] + [str(i) for i in range(3601, 3610)] + ["3606", "3608"]
    assert len(boosters) == len(targets)
    for booster, target_id, source_id in zip(boosters, targets, sources):
        assert booster["ChargeEnabled"].lower() in {"y", "yes", "true", "1", "on"}, (
            "Booster must keep its 0.5 second pre-launch charge", booster["DashBoosterID"])
        assert float(booster["Duration"]) <= 0.15, (
            "Booster control lock is too long", booster["DashBoosterID"])
        assert float(booster["Speed"]) >= 10.0, (
            "Booster launch is not cannon-fast", booster["DashBoosterID"])
        if source_id is not None:
            source = position(physics[source_id])
            start = position(booster)
            assert abs(start[0] - source[0]) <= 2.4
            assert abs(start[2] - source[2]) <= 2.4
        target = position(physics[target_id])
        # Platform rim top: 0.5 minus the mesh's 0.197 translation.
        result = landing(booster, target[1] + 0.303)
        assert abs(result[0] - target[0]) < 1.0, (booster["DashBoosterID"], result)
        assert abs(result[2] - target[2]) < 1.0, (booster["DashBoosterID"], result)
        assert abs(result[0]) <= 14.7 and abs(result[2]) <= 29.7
        for other in boosters:
            assert math.dist(result, position(other)) > float(other["Radius"]) + 0.2, (
                "Landing immediately triggers booster", booster["DashBoosterID"], other["DashBoosterID"])
        main_index = targets.index(target_id)
        if main_index + 1 < 10:
            assert_coasts_away_from_next_booster(booster, result, boosters[main_index + 1])
        for warp in read("WarpBears.csv"):
            wx, wy, wz = position(warp)
            if abs(result[1] - wy) < 1.7:
                assert math.hypot(result[0] - wx, result[2] - wz) > 0.9, "Landing on warp"
        print(booster["DashBoosterID"], "landing", tuple(round(v, 3) for v in result))
    print("PASS: placement, boundaries, <=0.15s control locks, and 12 impulse landings")


if __name__ == "__main__":
    main()
