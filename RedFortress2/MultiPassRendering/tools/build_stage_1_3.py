# -*- coding: utf-8 -*-
"""ステージ1-3「ガレキでふさがれた道」CSV一式を生成するスクリプト。

STAGE_GENERATION_MEMO.md の設計規則と stage_1_7（W1完成参照）のCSV形式に従う。
出力: res/model/stage_1_3/ 配下の各CSV（BOM付きUTF-8, CRLF改行）。
"""
import csv
import io
import os
import shutil

BASE = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "res", "model", "stage_1_3")
BASE = os.path.normpath(BASE)
SRC_17 = os.path.normpath(os.path.join(BASE, "..", "stage_1_7"))


def write_csv(filename, rows):
    path = os.path.join(BASE, filename)
    buf = io.StringIO()
    writer = csv.writer(buf, lineterminator="\r\n")
    for row in rows:
        writer.writerow(row)
    with open(path, "w", encoding="utf-8-sig", newline="") as f:
        f.write(buf.getvalue())
    print("WROTE", os.path.relpath(path), len(rows), "rows")


# ---------------------------------------------------------------- XFileList_simple.csv
SIMPLE = [
    ["ID", "FileName", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "loadType"],
    [1, "../ground/stage_visual_ground_world1.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"],
    [2, "stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2"],
    # 壁（ゾーン境界・遮蔽）
    [3101, "../collision_wall/collision_wall.x", -9, 1.5, 18, 0, 0, 0, 1, "meshmix2"],
    [3102, "../collision_wall/collision_wall.x", 9, 1.5, 12, 0, 90, 0, 1, "meshmix2"],
    [3103, "../collision_wall/collision_wall.x", -10, 1.5, -18, 0, 0, 0, 1, "meshmix2"],
    [3105, "../collision_wall/collision_wall.x", -7, 1.5, -18, 0, 0, 0, 1, "meshmix2"],
    # 感圧板連動扉（Move=y で上下に開く）
    [3106, "../collision_wall/collision_wall.x", 0, 1.5, -22, 0, 90, 0, 1, "meshmix2"],
    # 木箱（足場・遮蔽・I02の台）
    [3111, "../cubeWoodSmall/cube_wood_small.x", -6, 0, 20, 0, 0, 0, 1, "normal"],
    [3112, "../cubeWoodSmall/cube_wood_small.x", -6, 0.95, 20, 0, 20, 0, 1, "normal"],
    [3113, "../cubeWoodSmall/cube_wood_small.x", 5, 0, 9, 0, 0, 0, 1, "normal"],
    [3114, "../cubeWoodSmall/cube_wood_small.x", -8.5, 0, 24, 0, 0, 0, 1, "normal"],
    [3121, "../cubeWoodSmall/cube_wood_small.x", -8, 0, -24, 0, 0, 0, 1, "normal"],
    [3122, "../cubeWoodSmall/cube_wood_small.x", 10, 0, -15, 0, 0, 0, 1, "normal"],
    # 岩（遮蔽・装飾）
    [3131, "../base/base_rock1.x", -12, 0, 14, 0, 30, 0, 0.7, "normal"],
    [3132, "../base/base_rock2.x", 12, 0, 20, 0, 120, 0, 0.7, "normal"],
    [3133, "../base/base_rock1.x", -12, 0, -26, 0, 200, 0, 0.8, "normal"],
    [3134, "../base/base_rock2.x", 13, 0, -6, 0, 310, 0, 0.8, "normal"],
    # 移動床フェリー（堀の横断）
    [3511, "../collision_moving_platform/collision_moving_platform.x", -6, 0.4, -10, 0, 0, 0, 2, "meshmix2"],
    # 高所台座（Y>=3 の動かない床。ダッシュ床で到達）
    [3620, "../static_platform/static_platform_2x2.x", 10, 3.4, -10, 0, 0, 0, 1, "normal"],
    # QTE木
    [3801, "../tree2/lemonTree.x", -11.5, 0, 22, 0, 0, 0, 1, "normal"],
    # レバー連動壁（Y軸回転）
    [11001, "../attack_block/attack_wall.x", -11.5, 1.5, 14, 0, 0, 0, 1, "normal"],
    # 柵（外周の目印）
    [8001, "../fence.x", -11, 0.5, -30, 0, 0, 0, 1, "normal"],
    [8002, "../fence.x", -3, 0.5, -30, 0, 0, 0, 1, "normal"],
    [8003, "../fence.x", 5, 0.5, -30, 0, 0, 0, 1, "normal"],
    [8004, "../fence.x", 13, 0.5, -30, 0, 0, 0, 1, "normal"],
    [8005, "../fence.x", -11, 0.5, 30, 0, 0, 0, 1, "normal"],
    [8006, "../fence.x", -3, 0.5, 30, 0, 0, 0, 1, "normal"],
    [8007, "../fence.x", 5, 0.5, 30, 0, 0, 0, 1, "normal"],
    [8008, "../fence.x", 13, 0.5, 30, 0, 0, 0, 1, "normal"],
    [8009, "../fence.x", -15, 0.5, -26, 0, 90, 0, 1, "normal"],
    [8010, "../fence.x", -15, 0.5, -18, 0, 90, 0, 1, "normal"],
    [8011, "../fence.x", -15, 0.5, -10, 0, 90, 0, 1, "normal"],
    [8012, "../fence.x", -15, 0.5, -2, 0, 90, 0, 1, "normal"],
    [8013, "../fence.x", -15, 0.5, 6, 0, 90, 0, 1, "normal"],
    [8014, "../fence.x", -15, 0.5, 14, 0, 90, 0, 1, "normal"],
    [8015, "../fence.x", -15, 0.5, 22, 0, 90, 0, 1, "normal"],
    [8016, "../fence.x", -15, 0.5, 30, 0, 90, 0, 1, "normal"],
    [8017, "../fence.x", 15, 0.5, -26, 0, 90, 0, 1, "normal"],
    [8018, "../fence.x", 15, 0.5, -18, 0, 90, 0, 1, "normal"],
    [8019, "../fence.x", 15, 0.5, -10, 0, 90, 0, 1, "normal"],
    [8020, "../fence.x", 15, 0.5, -2, 0, 90, 0, 1, "normal"],
    [8021, "../fence.x", 15, 0.5, 6, 0, 90, 0, 1, "normal"],
    [8022, "../fence.x", 15, 0.5, 14, 0, 90, 0, 1, "normal"],
    [8023, "../fence.x", 15, 0.5, 22, 0, 90, 0, 1, "normal"],
    [8024, "../fence.x", 15, 0.5, 30, 0, 90, 0, 1, "normal"],
    [9290, "../SkySphere/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 0, 1, "normal"],
    [9200, "../grass/grass.x", 0, 0, 0, 0, 0, 0, 1, "instancing", "../grass/grass1-3.csv"],
    [9201, "../tree2/lemonTree.Instancing.x", 0, 0, 0, 0, 0, 0, 1, "instancing", "../tree2/lemonTree.Instancing.1-3.csv"],
]

# ---------------------------------------------------------------- XFileListPhysics.csv
PHYSICS = [
    ["ID", "FileName", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "Type", "Move", "Instancing"],
    [1, "res/model/cubeNormalInverse30x60.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n"],
    [2, "res/model/stage_1_3/stage_ground.x", 0, 0.01, 0, 0, 0, 0, 1, "Collision", "n"],
    [3101, "res/model/collision_wall/collision_wall_collision.x", -9, 1.5, 18, 0, 0, 0, 1, "Collision", "n"],
    [3102, "res/model/collision_wall/collision_wall_collision.x", 9, 1.5, 12, 0, 90, 0, 1, "Collision", "n"],
    [3103, "res/model/collision_wall/collision_wall_collision.x", -10, 1.5, -18, 0, 0, 0, 1, "Collision", "n"],
    [3105, "res/model/collision_wall/collision_wall_collision.x", -7, 1.5, -18, 0, 0, 0, 1, "Collision", "n"],
    [3106, "res/model/collision_wall/collision_wall_collision.x", 0, 1.5, -22, 0, 90, 0, 1, "Collision", "y"],
    [3111, "res/model/cubeWoodSmall/cube_wood_small_collision.x", -6, 0, 20, 0, 0, 0, 1, "Collision", "n"],
    [3112, "res/model/cubeWoodSmall/cube_wood_small_collision.x", -6, 0.95, 20, 0, 20, 0, 1, "Collision", "n"],
    [3113, "res/model/cubeWoodSmall/cube_wood_small_collision.x", 5, 0, 9, 0, 0, 0, 1, "Collision", "n"],
    [3114, "res/model/cubeWoodSmall/cube_wood_small_collision.x", -8.5, 0, 24, 0, 0, 0, 1, "Collision", "n"],
    [3121, "res/model/cubeWoodSmall/cube_wood_small_collision.x", -8, 0, -24, 0, 0, 0, 1, "Collision", "n"],
    [3122, "res/model/cubeWoodSmall/cube_wood_small_collision.x", 10, 0, -15, 0, 0, 0, 1, "Collision", "n"],
    [3131, "res/model/base/base_rock1_collision.x", -12, 0, 14, 0, 30, 0, 0.7, "Collision", "n"],
    [3132, "res/model/base/base_rock2_collision.x", 12, 0, 20, 0, 120, 0, 0.7, "Collision", "n"],
    [3133, "res/model/base/base_rock1_collision.x", -12, 0, -26, 0, 200, 0, 0.8, "Collision", "n"],
    [3134, "res/model/base/base_rock2_collision.x", 13, 0, -6, 0, 310, 0, 0.8, "Collision", "n"],
    [3511, "res/model/collision_moving_platform.x", -6, 0.4, -10, 0, 0, 0, 2, "Collision", "y"],
    [3620, "res/model/static_platform/static_platform_2x2_collision.x", 10, 3.4, -10, 0, 0, 0, 1, "Collision", "n"],
    [3801, "res/model/tree2Physics/tree_cylinder_collision.x", -11.5, 0, 22, 0, 0, 0, 1, "Collision", "n"],
    [11001, "res/model/attack_block/attack_wall.x", -11.5, 1.5, 14, 0, 0, 0, 1, "Collision", "y"],
]

# ---------------------------------------------------------------- XFileListMove.csv
MOVE = [
    ["ID", "RenderID", "PhysicsID", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale",
     "StartX", "StartY", "StartZ", "EndX", "EndY", "EndZ", "Duration"],
    # 堀のフェリー: X=-6〜+6 を ping-pong 往復（Scale2 → 6x6m 床、スイープが堀帯全域を覆う）
    [1, 3511, 3511, -6, 0.4, -10, 0, 0, 0, 2, -6, 0.4, -10, 6, 0.4, -10, 6.0],
]

# ---------------------------------------------------------------- EnemyPositions.csv
ENEMIES = [
    ["Type", "PosX", "PosY", "PosZ", "RotY"],
    ["wolf", -6, 0.2, 24, 180],
    ["frog", 8, 0.2, 20, 180],
    ["small_mushroom", -3, 0.2, 14, 0],
    ["crab", 0, 0.2, 10, 0],
    ["wolf", -9, 0.2, 6, 180],
    ["frog", -7, 0.2, 2, 90],
    ["crab", 6, 0.2, -16, 180],
    ["small_mushroom", -8, 0.2, -20, 0],
    ["frog", 11, 0.2, -14, 180],
    ["wolf", 7, 0.2, -26, 0],
    ["small_mushroom", -8, 0.2, -25, 90],
]

# ---------------------------------------------------------------- Destructibles.csv（ガレキ）
DESTRUCTIBLES = [
    ["PosX", "PosY", "PosZ", "HP", "DropItemId"],
    # メイン道を塞ぐガレキの山（壊して道を開く）
    [0, 0.45, 14, 3, "016"],
    [0, 0.45, 11.5, 2, "None"],
    [-2, 0.45, 13, 2, "None"],
    [2, 0.45, 13, 3, "014"],
    # QTE木のアルコーブを塞ぐガレキ
    [-9, 0.45, 22, 2, "None"],
    [-9.5, 0.45, 24, 1, "None"],
    # 南区画のガレキ
    [7, 0.45, -20, 2, "None"],
    [-7, 0.45, -17, 2, "None"],
    [9, 0.45, -24, 3, "016"],
]

# ---------------------------------------------------------------- Collectibles.csv
COLLECTIBLES = [
    ["CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"],
    ["stage13-I01", "Item", 1, 10, 0.45, 26, 1],       # 北東端（端ギミック）
    ["stage13-I02", "Item", 3, -8.5, 1.45, 24, 1],     # QTE木そばの木箱の上
    ["stage13-I03", "Item", 4, 13, 0.45, -25, 1],      # 南東端（端ギミック）
    ["stage13-I04", "Item", 5, 10, 3.85, -10, 1],      # 高所台座3620（ダッシュ床で）
    ["stage13-I05", "Item", 6, -13, 0.45, 17.5, 1],    # レバーで開く西側隠し部屋
    ["stage13-I06", "Item", 9, -4, 0.45, 4, 1],        # 北区画 西
]

# ---------------------------------------------------------------- その他
SPEEDUPS = [
    ["PosX", "PosY", "PosZ"],
    [0, 0.45, 18],
]

BOOSTERS = [
    ["DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ", "Speed", "Duration", "Radius", "Scale"],
    # 堀北岸 → 高所台座3620（I04）へ射出
    ["stage13-booster-01", 10, 0.5, -6, 0, 0.93, -0.37, 16, 0.9, 1.25, 0.55],
]

SKULLS = [
    ["ID", "PosX", "PosY", "PosZ", "RotY"],
    [1, 2.5, 0.2, -19, 0],    # 感圧板そば（板に載せて扉を開いたままにする）
    [2, -13, 0.2, 10, 0],     # 西側隠し部屋
]

STARS = [
    ["PosX", "PosY", "PosZ"],
    [0, 0.45, -24],           # 扉の先・ゴール前
]

INTERACTABLES = [
    ["InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"],
    ["stage13-tree-01", "Tree", -11.5, 0, 22, 2.5],
]

ATTACK_TRIGGERS = [
    ["ID", "Type", "TriggerX", "TriggerY", "TriggerZ", "TargetID", "Axis", "BaseRotX", "BaseRotY", "BaseRotZ", "Scale"],
    [1, "Lever", -11.5, 0, 11, 11001, "Y", 0, 0, 0, 1],
]

PRESSURE_PLATES = [
    ["ID", "PlatePosX", "PlatePosY", "PlatePosZ", "WallID", "WallRotX", "WallRotY", "WallRotZ", "WallScale"],
    [1, 0, 0.01, -19, 3106, 0, 90, 0, 1],
]


def copy_instancing_files():
    """1-7 の柵外景観インスタンシング配置を流用（ワールド共通の見た目）。"""
    pairs = [
        (os.path.join(SRC_17, "..", "grass", "grass1-7.csv"),
         os.path.join(BASE, "..", "grass", "grass1-3.csv")),
        (os.path.join(SRC_17, "..", "tree2", "lemonTree.Instancing.1-7.csv"),
         os.path.join(BASE, "..", "tree2", "lemonTree.Instancing.1-3.csv")),
    ]
    for src, dst in pairs:
        src = os.path.normpath(src)
        dst = os.path.normpath(dst)
        shutil.copyfile(src, dst)
        print("COPIED", os.path.relpath(dst), "<-", os.path.relpath(src))


def main():
    os.makedirs(BASE, exist_ok=True)
    write_csv("XFileList_simple.csv", SIMPLE)
    write_csv("XFileListPhysics.csv", PHYSICS)
    write_csv("XFileListMove.csv", MOVE)
    write_csv("EnemyPositions.csv", ENEMIES)
    write_csv("Destructibles.csv", DESTRUCTIBLES)
    write_csv("Collectibles.csv", COLLECTIBLES)
    write_csv("SpeedUps.csv", SPEEDUPS)
    write_csv("DashBoosters.csv", BOOSTERS)
    write_csv("Skulls.csv", SKULLS)
    write_csv("Stars.csv", STARS)
    write_csv("Interactables.csv", INTERACTABLES)
    write_csv("AttackTriggers.csv", ATTACK_TRIGGERS)
    write_csv("PressurePlates.csv", PRESSURE_PLATES)
    copy_instancing_files()
    print("DONE")


if __name__ == "__main__":
    main()
