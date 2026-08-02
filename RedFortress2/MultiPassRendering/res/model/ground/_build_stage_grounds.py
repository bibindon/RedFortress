import csv
import os
from collections import deque
from pathlib import Path

import bpy


GROUND_DIR = Path(__file__).resolve().parent
MODEL_DIR = GROUND_DIR.parent
BLEND_PATH = GROUND_DIR / "stage_grounds.blend"
SIDE_TEXTURE_PATH = MODEL_DIR / "whiteWall.png"
SLAB_BOTTOM = -20.0
PIT_BOTTOM = -18.0
VISUAL_GROUND_HALF_SIZE = 500.0

# Per-world top-surface textures (CC0 from Poly Haven, see tex/POLYHAVEN_CC0.md).
# stage_ground.x lives in res/model/<folder>/ and references the texture as
# "../ground/tex/<file>", which resolves to res/model/ground/tex/<file>.
TEX_DIR = GROUND_DIR / "tex"
WORLD_TOP_TEXTURES = {
    1: TEX_DIR / "world1.png",
    2: TEX_DIR / "world2.png",
    3: TEX_DIR / "world3.png",
    4: TEX_DIR / "world4.png",
}

# Map a stage folder name (e.g. "stage17") to its world number (1..4).
# folders: W1=stage1-4,17-20  W2=stage5-8,21-24  W3=stage9-12,25-28  W4=stage13-16,29-32
def world_for_folder(folder):
    digits = "".join(ch for ch in folder if ch.isdigit())
    if not digits:
        return 1
    n = int(digits)
    if n in (1, 2, 3, 4, 17, 18, 19, 20):
        return 1
    if n in (5, 6, 7, 8, 21, 22, 23, 24):
        return 2
    if n in (9, 10, 11, 12, 25, 26, 27, 28):
        return 3
    return 4


STAGES = (
    {"display": "1-1", "folder": "stage1", "size": (16.0, 32.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ((-13.0, -10.0, -3.0, 7.0),)},
    {"display": "1-2", "folder": "stage2", "size": (16.0, 32.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((3.0, 7.0, -17.0, -11.0),)},
    {"display": "1-3", "folder": "stage3", "size": (16.0, 32.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-2.0, 2.0, -6.0, 6.0),)},
    {"display": "1-4", "folder": "stage4", "size": (16.0, 32.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ((-13.0, -9.0, -15.0, -11.0), (8.0, 12.0, -20.0, -12.0))},
    {"display": "1-5", "folder": "stage17", "size": (16.0, 32.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ((-13.0, -10.0, 4.0, 9.0), (10.0, 13.0, -8.0, -2.0))},
    {"display": "1-6", "folder": "stage18", "size": (16.0, 32.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((-13.0, -10.0, -12.0, -4.0), (10.0, 13.0, 4.0, 12.0))},
    {"display": "1-7", "folder": "stage19", "size": (16.0, 32.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-14.8, 14.8, -5.0, 5.0),)},
    {"display": "1-8", "folder": "stage20", "size": (16.0, 32.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ((-14.0, -11.0, -4.0, 4.0), (11.0, 14.0, -4.0, 4.0))},

    {"display": "2-1", "folder": "stage5", "size": (60.0, 60.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ()},
    {"display": "2-2", "folder": "stage6", "size": (60.0, 60.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((-38.0, -28.0, -12.0, 12.0),)},
    {"display": "2-3", "folder": "stage7", "size": (60.0, 60.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-38.0, -30.0, -30.0, -10.0), (30.0, 38.0, 10.0, 30.0), (-8.0, 8.0, 40.0, 48.0))},
    {"display": "2-4", "folder": "stage8", "size": (60.0, 60.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},
    {"display": "2-5", "folder": "stage21", "size": (60.0, 60.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ()},
    {"display": "2-6", "folder": "stage22", "size": (60.0, 60.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((-24.0, 24.0, -28.0, -20.0), (-24.0, 24.0, 20.0, 28.0))},
    {"display": "2-7", "folder": "stage23", "size": (60.0, 60.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-48.0, -40.0, 8.0, 22.0), (40.0, 48.0, -22.0, -8.0))},
    {"display": "2-8", "folder": "stage24", "size": (60.0, 60.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},

    {"display": "3-1", "folder": "stage9", "size": (60.0, 120.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ((-50.0, -44.0, -22.0, 22.0), (44.0, 50.0, -22.0, 22.0))},
    {"display": "3-2", "folder": "stage10", "size": (60.0, 120.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((-32.0, -6.0, -52.0, -44.0), (6.0, 32.0, -52.0, -44.0), (-32.0, -6.0, 44.0, 52.0), (6.0, 32.0, 44.0, 52.0))},
    {"display": "3-3", "folder": "stage11", "size": (60.0, 120.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-35.0, -25.0, -50.0, 50.0),)},
    {"display": "3-4", "folder": "stage12", "size": (60.0, 120.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ((-38.0, -26.0, 30.0, 50.0), (-6.0, 6.0, 55.0, 70.0), (26.0, 38.0, -50.0, -30.0))},
    {"display": "3-5", "folder": "stage25", "size": (60.0, 120.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ((-30.0, 30.0, 45.0, 55.0),)},
    {"display": "3-6", "folder": "stage26", "size": (60.0, 120.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((-40.0, -30.0, 20.0, 45.0), (30.0, 40.0, -45.0, -20.0), (-5.0, 5.0, 60.0, 75.0))},
    {"display": "3-7", "folder": "stage27", "size": (60.0, 120.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-40.0, -30.0, -12.0, 12.0), (30.0, 40.0, -12.0, 12.0), (-18.0, -8.0, 48.0, 64.0), (8.0, 18.0, -64.0, -48.0))},
    {"display": "3-8", "folder": "stage28", "size": (60.0, 120.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},

    {"display": "4-1", "folder": "stage13", "size": (60.0, 120.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ()},
    {"display": "4-2", "folder": "stage14", "size": (60.0, 120.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ()},
    {"display": "4-3", "folder": "stage15", "size": (60.0, 120.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-48.0, -42.0, -12.0, 12.0), (42.0, 48.0, -12.0, 12.0))},
    {"display": "4-4", "folder": "stage16", "size": (60.0, 120.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},
    {"display": "4-5", "folder": "stage29", "size": (60.0, 120.0), "start": (0.0, -28.0), "goal": (0.0, 28.0), "pits": ((-30.0, -20.0, -12.0, 12.0), (20.0, 30.0, -12.0, 12.0), (-12.0, 12.0, 42.0, 52.0), (-12.0, 12.0, -52.0, -42.0))},
    {"display": "4-6", "folder": "stage30", "size": (60.0, 120.0), "start": (-14.0, 0.0), "goal": (14.0, 0.0), "pits": ((-42.0, -28.0, 36.0, 50.0), (28.0, 42.0, 36.0, 50.0), (-42.0, -28.0, -50.0, -36.0), (28.0, 42.0, -50.0, -36.0))},
    {"display": "4-7", "folder": "stage31", "size": (60.0, 120.0), "start": (0.0, 28.0), "goal": (0.0, -28.0), "pits": ((-28.0, -18.0, 20.0, 36.0), (16.0, 28.0, 4.0, 18.0), (-26.0, -14.0, -18.0, -4.0), (14.0, 24.0, -36.0, -20.0))},
    {"display": "4-8", "folder": "stage32", "size": (60.0, 120.0), "start": (14.0, 28.0), "goal": (-14.0, -28.0), "pits": ()},
)


POSITION_CSV_FILES = (
    "EnemyPositions.csv",
    "Destructibles.csv",
    "Collectibles.csv",
    "SpeedUps.csv",
    "DashBoosters.csv",
)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials, bpy.data.images):
        for data_block in list(collection):
            collection.remove(data_block)


def create_material(name, texture_path, texture_filename, color):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = color
    material.roughness = 1.0
    material.metallic = 0.0
    material["_x_power"] = 0.0
    material["_x_specular"] = (0.0, 0.0, 0.0)
    material["_x_emissive"] = (0.0, 0.0, 0.0)
    material["_x_texture_filename"] = texture_filename

    principled = next(
        node for node in material.node_tree.nodes
        if node.type == "BSDF_PRINCIPLED"
    )
    principled.inputs["Base Color"].default_value = color
    principled.inputs["Roughness"].default_value = 1.0

    image = bpy.data.images.load(str(texture_path), check_existing=True)
    texture = material.node_tree.nodes.new("ShaderNodeTexImage")
    texture.name = name + "_Texture"
    texture.image = image
    texture.extension = "REPEAT"
    material.node_tree.links.new(texture.outputs["Color"], principled.inputs["Base Color"])
    return material


def point_in_pit(x, y, pits, margin=0.0):
    for pit in pits:
        x_min, x_max, y_min, y_max = pit
        if x >= x_min - margin and x <= x_max + margin:
            if y >= y_min - margin and y <= y_max + margin:
                return True
    return False


def validate_stage(stage):
    half_width, half_depth = stage["size"]
    pits = stage["pits"]

    for pit_index, pit in enumerate(pits):
        x_min, x_max, y_min, y_max = pit
        if x_min >= x_max or y_min >= y_max:
            raise RuntimeError(stage["display"] + " has an invalid pit rectangle")
        if x_min <= -half_width or x_max >= half_width:
            raise RuntimeError(stage["display"] + " pit touches the outer X wall")
        if y_min <= -half_depth or y_max >= half_depth:
            raise RuntimeError(stage["display"] + " pit touches the outer Z wall")
        for other_index in range(pit_index):
            other = pits[other_index]
            separated = x_max <= other[0] or x_min >= other[1]
            if not separated:
                separated = y_max <= other[2] or y_min >= other[3]
            if not separated:
                raise RuntimeError(stage["display"] + " has overlapping pits")

    for label in ("start", "goal"):
        point = stage[label]
        if point_in_pit(point[0], point[1], pits, margin=1.0):
            raise RuntimeError(stage["display"] + " " + label + " overlaps a pit")

    stage_dir = MODEL_DIR / stage["folder"]
    conflicts = []
    for csv_name in POSITION_CSV_FILES:
        csv_path = stage_dir / csv_name
        if not csv_path.exists():
            continue
        with csv_path.open("r", encoding="utf-8-sig", newline="") as file:
            for row_index, row in enumerate(csv.DictReader(file), start=2):
                if "PosX" not in row or "PosZ" not in row:
                    continue
                if row["PosX"] is None or row["PosZ"] is None:
                    continue
                if row["PosX"].strip() == "" or row["PosZ"].strip() == "":
                    continue
                x = float(row["PosX"])
                y = float(row["PosZ"])
                if point_in_pit(x, y, pits, margin=0.8):
                    conflicts.append(csv_name + ":" + str(row_index))

    render_path = stage_dir / "XFileList_simple.csv"
    if render_path.exists():
        with render_path.open("r", encoding="utf-8-sig", newline="") as file:
            for row_index, row in enumerate(csv.DictReader(file), start=2):
                filename = row.get("FileName", "")
                lowered = filename.lower()
                if "ground" in lowered or "platefield" in lowered:
                    continue
                if "skysphere" in lowered or "fence.x" in lowered:
                    continue
                if "collision_moving_platform" in lowered:
                    continue
                if row.get("PosX") is None or row.get("PosZ") is None:
                    continue
                x = float(row["PosX"])
                y = float(row["PosZ"])
                if point_in_pit(x, y, pits, margin=0.8):
                    conflicts.append("XFileList_simple.csv:" + str(row_index))

    collision_rectangles = load_collision_rectangles(stage)
    for rectangle in collision_rectangles:
        for pit in pits:
            if rectangle_intersects_pit(rectangle, pit, margin=0.2):
                conflicts.append("XFileListPhysics.csv:" + str(rectangle[4]))
                break

    if not has_safe_route(stage, collision_rectangles):
        conflicts.append("no safe route from start to goal")

    return conflicts


def load_collision_rectangles(stage):
    rectangles = []
    physics_path = MODEL_DIR / stage["folder"] / "XFileListPhysics.csv"
    with physics_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row_index, row in enumerate(csv.DictReader(file), start=2):
            filename = row.get("FileName", "").lower()
            if row.get("Move", "").lower() == "y":
                continue
            half_x = None
            half_y = None
            is_climbable = False
            if "collision_wall" in filename:
                rotation = int(float(row.get("RotY", "0"))) % 180
                if rotation == 0:
                    half_x = 0.9
                    half_y = 4.4
                else:
                    half_x = 4.4
                    half_y = 0.9
            elif "cube_wood" in filename:
                half_x = 1.25
                half_y = 1.25
                is_climbable = True
            elif "tree_cylinder" in filename:
                half_x = 1.2
                half_y = 1.2
            if half_x is None:
                continue
            center_x = float(row["PosX"])
            center_y = float(row["PosZ"])
            rectangles.append((
                center_x - half_x,
                center_x + half_x,
                center_y - half_y,
                center_y + half_y,
                row_index,
                is_climbable,
            ))
    return rectangles


def rectangle_intersects_pit(rectangle, pit, margin=0.0):
    if rectangle[1] <= pit[0] - margin or rectangle[0] >= pit[1] + margin:
        return False
    if rectangle[3] <= pit[2] - margin or rectangle[2] >= pit[3] + margin:
        return False
    return True


def load_lava_zones(stage):
    zones = []
    stage_dir = MODEL_DIR / stage["folder"]
    lava_path = stage_dir / "LavaZones.csv"
    physics_path = stage_dir / "XFileListPhysics.csv"
    if not lava_path.exists() or not physics_path.exists():
        return zones

    plate_bounds = {}
    with physics_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row in csv.DictReader(file):
            filename = row.get("FileName", "").lower()
            if "platelava" not in filename:
                continue
            csv_id = int(row["ID"])
            scale = float(row["Scale"])
            plate_bounds[csv_id] = (float(row["PosX"]), float(row["PosZ"]), 4.0 * scale)

    with lava_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row in csv.DictReader(file):
            physics_id = int(row["PhysicsID"])
            if physics_id in plate_bounds:
                x, z, half_size = plate_bounds[physics_id]
                zones.append((x, z, half_size))
    return zones


def load_moving_platform_sweeps(stage):
    sweeps = []
    move_path = MODEL_DIR / stage["folder"] / "XFileListMove.csv"
    if not move_path.exists():
        return sweeps

    with move_path.open("r", encoding="utf-8-sig", newline="") as file:
        for row in csv.DictReader(file):
            scale = float(row["Scale"])
            half_size = 1.5 * scale
            start_x = float(row["StartX"])
            start_z = float(row["StartZ"])
            end_x = float(row["EndX"])
            end_z = float(row["EndZ"])
            sweeps.append((
                min(start_x, end_x) - half_size,
                max(start_x, end_x) + half_size,
                min(start_z, end_z) - half_size,
                max(start_z, end_z) + half_size,
            ))
    return sweeps


def has_safe_route(stage, rectangles):
    half_width, half_depth = stage["size"]
    pits = stage["pits"]
    lava_zones = load_lava_zones(stage)
    moving_platform_sweeps = load_moving_platform_sweeps(stage)
    grid_step = 1.0
    player_margin = 0.45

    def supported_by_moving_platform(x, y):
        for sweep in moving_platform_sweeps:
            if x >= sweep[0] - player_margin and x <= sweep[1] + player_margin:
                if y >= sweep[2] - player_margin and y <= sweep[3] + player_margin:
                    return True
        return False

    def blocked(x, y):
        if x < -half_width + player_margin or x > half_width - player_margin:
            return True
        if y < -half_depth + player_margin or y > half_depth - player_margin:
            return True
        if point_in_pit(x, y, pits, margin=player_margin):
            if not supported_by_moving_platform(x, y):
                return True
        for zone_x, zone_y, radius in lava_zones:
            delta_x = x - zone_x
            delta_y = y - zone_y
            safe_radius = radius + player_margin
            if delta_x * delta_x + delta_y * delta_y <= safe_radius * safe_radius:
                return True
        for rectangle in rectangles:
            if rectangle[5]:
                continue
            if x >= rectangle[0] - player_margin and x <= rectangle[1] + player_margin:
                if y >= rectangle[2] - player_margin and y <= rectangle[3] + player_margin:
                    return True
        return False

    start = stage["start"]
    goal = stage["goal"]
    if blocked(start[0], start[1]) or blocked(goal[0], goal[1]):
        return False

    queue = deque((start,))
    visited = {(round(start[0], 3), round(start[1], 3))}
    directions = ((grid_step, 0.0), (-grid_step, 0.0), (0.0, grid_step), (0.0, -grid_step))
    while queue:
        x, y = queue.popleft()
        if abs(x - goal[0]) <= grid_step * 0.5 and abs(y - goal[1]) <= grid_step * 0.5:
            return True
        for delta_x, delta_y in directions:
            next_x = x + delta_x
            next_y = y + delta_y
            key = (round(next_x, 3), round(next_y, 3))
            if key in visited:
                continue
            if blocked(next_x, next_y):
                continue
            visited.add(key)
            queue.append((next_x, next_y))
    return False

def add_quad(vertices, faces, uvs, material_indices, coordinates, quad_uvs, material_index):
    start = len(vertices)
    vertices.extend(coordinates)
    uvs.extend(quad_uvs)
    faces.append((start, start + 1, start + 2, start + 3))
    material_indices.append(material_index)


def create_visual_ground_extension(world, top_material, play_half_size):
    play_half_width, play_half_depth = play_half_size
    outer_half_size = VISUAL_GROUND_HALF_SIZE
    vertices = []
    faces = []
    uvs = []
    material_indices = []

    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((-outer_half_size, play_half_depth, 0.0),
         (outer_half_size, play_half_depth, 0.0),
         (outer_half_size, outer_half_size, 0.0),
         (-outer_half_size, outer_half_size, 0.0)),
        ((-outer_half_size / 8.0, play_half_depth / 8.0),
         (outer_half_size / 8.0, play_half_depth / 8.0),
         (outer_half_size / 8.0, outer_half_size / 8.0),
         (-outer_half_size / 8.0, outer_half_size / 8.0)),
        0,
    )
    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((-outer_half_size, -outer_half_size, 0.0),
         (outer_half_size, -outer_half_size, 0.0),
         (outer_half_size, -play_half_depth, 0.0),
         (-outer_half_size, -play_half_depth, 0.0)),
        ((-outer_half_size / 8.0, -outer_half_size / 8.0),
         (outer_half_size / 8.0, -outer_half_size / 8.0),
         (outer_half_size / 8.0, -play_half_depth / 8.0),
         (-outer_half_size / 8.0, -play_half_depth / 8.0)),
        0,
    )
    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((-outer_half_size, -play_half_depth, 0.0),
         (-play_half_width, -play_half_depth, 0.0),
         (-play_half_width, play_half_depth, 0.0),
         (-outer_half_size, play_half_depth, 0.0)),
        ((-outer_half_size / 8.0, -play_half_depth / 8.0),
         (-play_half_width / 8.0, -play_half_depth / 8.0),
         (-play_half_width / 8.0, play_half_depth / 8.0),
         (-outer_half_size / 8.0, play_half_depth / 8.0)),
        0,
    )
    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((play_half_width, -play_half_depth, 0.0),
         (outer_half_size, -play_half_depth, 0.0),
         (outer_half_size, play_half_depth, 0.0),
         (play_half_width, play_half_depth, 0.0)),
        ((play_half_width / 8.0, -play_half_depth / 8.0),
         (outer_half_size / 8.0, -play_half_depth / 8.0),
         (outer_half_size / 8.0, play_half_depth / 8.0),
         (play_half_width / 8.0, play_half_depth / 8.0)),
        0,
    )

    object_name = "StageVisualGroundWorld" + str(world)
    mesh = bpy.data.meshes.new(object_name + "Geo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(top_material)

    for polygon_index, polygon in enumerate(mesh.polygons):
        polygon.use_smooth = False
        polygon.material_index = material_indices[polygon_index]

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]

    obj = bpy.data.objects.new(object_name, mesh)
    bpy.context.collection.objects.link(obj)
    obj["_x_frame_name"] = object_name
    obj["_x_mesh_name"] = object_name + "Geo"
    return obj

def create_stage_ground(stage, top_material, side_material):
    half_width, half_depth = stage["size"]
    pits = stage["pits"]
    vertices = []
    faces = []
    uvs = []
    material_indices = []

    x_values = {-half_width, half_width}
    y_values = {-half_depth, half_depth}
    for pit in pits:
        x_values.add(pit[0])
        x_values.add(pit[1])
        y_values.add(pit[2])
        y_values.add(pit[3])
    x_values = sorted(x_values)
    y_values = sorted(y_values)

    for x_index in range(len(x_values) - 1):
        for y_index in range(len(y_values) - 1):
            x_min = x_values[x_index]
            x_max = x_values[x_index + 1]
            y_min = y_values[y_index]
            y_max = y_values[y_index + 1]
            center_x = (x_min + x_max) * 0.5
            center_y = (y_min + y_max) * 0.5
            if point_in_pit(center_x, center_y, pits):
                continue
            add_quad(
                vertices,
                faces,
                uvs,
                material_indices,
                ((x_min, y_min, 0.0), (x_max, y_min, 0.0), (x_max, y_max, 0.0), (x_min, y_max, 0.0)),
                ((x_min / 8.0, y_min / 8.0), (x_max / 8.0, y_min / 8.0), (x_max / 8.0, y_max / 8.0), (x_min / 8.0, y_max / 8.0)),
                0,
            )

    add_quad(
        vertices,
        faces,
        uvs,
        material_indices,
        ((-half_width, -half_depth, SLAB_BOTTOM), (-half_width, half_depth, SLAB_BOTTOM), (half_width, half_depth, SLAB_BOTTOM), (half_width, -half_depth, SLAB_BOTTOM)),
        ((0.0, 0.0), (0.0, half_depth / 8.0), (half_width / 8.0, half_depth / 8.0), (half_width / 8.0, 0.0)),
        1,
    )

    add_quad(vertices, faces, uvs, material_indices,
             ((-half_width, -half_depth, 0.0), (-half_width, -half_depth, SLAB_BOTTOM), (half_width, -half_depth, SLAB_BOTTOM), (half_width, -half_depth, 0.0)),
             ((0.0, 0.0), (0.0, 2.5), (half_width / 4.0, 2.5), (half_width / 4.0, 0.0)), 1)
    add_quad(vertices, faces, uvs, material_indices,
             ((half_width, half_depth, 0.0), (half_width, half_depth, SLAB_BOTTOM), (-half_width, half_depth, SLAB_BOTTOM), (-half_width, half_depth, 0.0)),
             ((0.0, 0.0), (0.0, 2.5), (half_width / 4.0, 2.5), (half_width / 4.0, 0.0)), 1)
    add_quad(vertices, faces, uvs, material_indices,
             ((-half_width, half_depth, 0.0), (-half_width, half_depth, SLAB_BOTTOM), (-half_width, -half_depth, SLAB_BOTTOM), (-half_width, -half_depth, 0.0)),
             ((0.0, 0.0), (0.0, 2.5), (half_depth / 4.0, 2.5), (half_depth / 4.0, 0.0)), 1)
    add_quad(vertices, faces, uvs, material_indices,
             ((half_width, -half_depth, 0.0), (half_width, -half_depth, SLAB_BOTTOM), (half_width, half_depth, SLAB_BOTTOM), (half_width, half_depth, 0.0)),
             ((0.0, 0.0), (0.0, 2.5), (half_depth / 4.0, 2.5), (half_depth / 4.0, 0.0)), 1)

    # Pit walls and bottoms use the same world texture as the ground surface.
    for pit in pits:
        x_min, x_max, y_min, y_max = pit
        add_quad(vertices, faces, uvs, material_indices,
                 ((x_min, y_min, 0.0), (x_min, y_min, PIT_BOTTOM), (x_min, y_max, PIT_BOTTOM), (x_min, y_max, 0.0)),
                 ((0.0, 0.0), (0.0, 2.25), ((y_max - y_min) / 4.0, 2.25), ((y_max - y_min) / 4.0, 0.0)), 0)
        add_quad(vertices, faces, uvs, material_indices,
                 ((x_max, y_max, 0.0), (x_max, y_max, PIT_BOTTOM), (x_max, y_min, PIT_BOTTOM), (x_max, y_min, 0.0)),
                 ((0.0, 0.0), (0.0, 2.25), ((y_max - y_min) / 4.0, 2.25), ((y_max - y_min) / 4.0, 0.0)), 0)
        add_quad(vertices, faces, uvs, material_indices,
                 ((x_max, y_min, 0.0), (x_max, y_min, PIT_BOTTOM), (x_min, y_min, PIT_BOTTOM), (x_min, y_min, 0.0)),
                 ((0.0, 0.0), (0.0, 2.25), ((x_max - x_min) / 4.0, 2.25), ((x_max - x_min) / 4.0, 0.0)), 0)
        add_quad(vertices, faces, uvs, material_indices,
                 ((x_min, y_max, 0.0), (x_min, y_max, PIT_BOTTOM), (x_max, y_max, PIT_BOTTOM), (x_max, y_max, 0.0)),
                 ((0.0, 0.0), (0.0, 2.25), ((x_max - x_min) / 4.0, 2.25), ((x_max - x_min) / 4.0, 0.0)), 0)
        add_quad(vertices, faces, uvs, material_indices,
                 ((x_min, y_min, PIT_BOTTOM), (x_max, y_min, PIT_BOTTOM), (x_max, y_max, PIT_BOTTOM), (x_min, y_max, PIT_BOTTOM)),
                 ((0.0, 0.0), ((x_max - x_min) / 4.0, 0.0), ((x_max - x_min) / 4.0, (y_max - y_min) / 4.0), (0.0, (y_max - y_min) / 4.0)), 0)

    object_name = "Stage" + stage["display"].replace("-", "_") + "Ground"
    mesh = bpy.data.meshes.new(object_name + "Geo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(top_material)
    mesh.materials.append(side_material)

    for polygon_index, polygon in enumerate(mesh.polygons):
        polygon.use_smooth = False
        polygon.material_index = material_indices[polygon_index]

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]

    obj = bpy.data.objects.new(object_name, mesh)
    bpy.context.collection.objects.link(obj)
    obj["_x_frame_name"] = object_name
    obj["_x_mesh_name"] = object_name + "Geo"
    return obj


def export_object(obj, output_path):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    result = bpy.ops.export_scene.directx_x(
        filepath=str(output_path),
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        export_format="TEXT_X",
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(output_path))


def main():
    if len(STAGES) != 32:
        raise RuntimeError("Exactly 32 stage ground definitions are required")

    folders = {stage["folder"] for stage in STAGES}
    if len(folders) != 32:
        raise RuntimeError("Stage ground folder names must be unique")

    selected_display = os.environ.get("RED_FORTRESS_STAGE_GROUND", "").strip()
    stages_to_build = STAGES
    if selected_display != "":
        stages_to_build = tuple(stage for stage in STAGES if stage["display"] == selected_display)
        if len(stages_to_build) != 1:
            raise RuntimeError("Unknown stage ground selection: " + selected_display)

    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    clear_scene()
    side_material = create_material(
        "StageGroundSide",
        SIDE_TEXTURE_PATH,
        "../whiteWall.png",
        (0.42, 0.42, 0.42, 1.0),
    )

    # One top-surface material per world (texture differs per world).
    top_materials = {}
    for world, texture_path in WORLD_TOP_TEXTURES.items():
        if not texture_path.exists():
            raise RuntimeError("Missing world top texture: " + str(texture_path))
        relative_name = ".." / texture_path.relative_to(MODEL_DIR)
        top_materials[world] = create_material(
            "StageGroundTopWorld" + str(world),
            texture_path,
            str(relative_name).replace("\\", "/"),
            (0.64, 0.64, 0.64, 1.0),
        )

    visual_ground_objects = []
    world_play_half_sizes = {
        1: (16.0, 32.0),
        2: (60.0, 60.0),
        3: (60.0, 120.0),
        4: (60.0, 120.0),
    }
    for world, play_half_size in world_play_half_sizes.items():
        visual_ground = create_visual_ground_extension(
            world,
            top_materials[world],
            play_half_size,
        )
        visual_ground_objects.append((world, visual_ground))
    validation_errors = []
    for stage in stages_to_build:
        conflicts = validate_stage(stage)
        if conflicts:
            validation_errors.append(stage["display"] + ": " + ", ".join(conflicts))
    if validation_errors:
        raise RuntimeError("Stage ground conflicts:\n" + "\n".join(validation_errors))

    objects = []
    for stage in stages_to_build:
        world = world_for_folder(stage["folder"])
        obj = create_stage_ground(stage, top_materials[world], side_material)
        objects.append((stage, obj))

    if selected_display == "":
        bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    for stage, obj in objects:
        output_path = MODEL_DIR / stage["folder"] / "stage_ground.x"
        export_object(obj, output_path)
        print("EXPORTED", stage["display"], output_path)

    if selected_display == "":
        for world, obj in visual_ground_objects:
            output_path = GROUND_DIR / ("stage_visual_ground_world" + str(world) + ".x")
            export_object(obj, output_path)
            print("EXPORTED_VISUAL", world, output_path)
        print("BLEND_PATH", BLEND_PATH)
    print("EXPORTED_STAGE_GROUNDS", len(objects))


main()