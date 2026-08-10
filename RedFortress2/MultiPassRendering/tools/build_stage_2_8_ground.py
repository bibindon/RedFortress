# -*- coding: utf-8 -*-
"""ステージ2-8のボスあり・クリア後専用地形を公式DirectX X形式で生成する。"""

from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_8"
SURFACE_TOP_Y = 0.4
SURFACE_THICKNESS = 0.4


BOSS_FLOORS = (
    ("boss_arena", 0.0, 0.0, 80.0, 100.0),
)


BOSS_WALLS = (
    ("boss_wall_west", -40.0, 0.0, 1.0, 100.0),
    ("boss_wall_east", 40.0, 0.0, 1.0, 100.0),
    ("boss_wall_south", 0.0, -50.0, 80.0, 1.0),
    ("boss_wall_north", 0.0, 50.0, 80.0, 1.0),
)


CLEARED_FLOORS = (
    ("cleared_entry", 0.0, -42.0, 30.0, 20.0),
    ("cleared_south_hall", 0.0, -27.0, 70.0, 14.0),
    ("cleared_west_lane", -26.0, 0.0, 18.0, 40.0),
    ("cleared_east_lane", 26.0, 0.0, 18.0, 40.0),
    ("cleared_cross_bridge", 0.0, 0.0, 52.0, 8.0),
    ("cleared_north_hall", 0.0, 27.0, 70.0, 14.0),
    ("cleared_north_room", 0.0, 42.0, 30.0, 20.0),
    ("cleared_west_branch", -43.5, 0.0, 17.0, 8.0),
    ("cleared_west_reward", -52.0, 0.0, 8.0, 14.0),
    ("cleared_east_branch", 43.5, 0.0, 17.0, 8.0),
    ("cleared_east_reward", 52.0, 0.0, 8.0, 14.0),
)


def normalize_x_file(path):
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    path.write_bytes(data)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for material in list(bpy.data.materials):
        bpy.data.materials.remove(material)


def make_material(name, color):
    material = bpy.data.materials.new(name)
    material.diffuse_color = color
    material.specular_intensity = 0.15
    material.roughness = 0.9
    return material


def add_box_game(name, x, z, top_y, width_x, depth_z, height, material):
    bpy.ops.mesh.primitive_cube_add(location=(x, -z, top_y - height * 0.5))
    obj = bpy.context.active_object
    obj.name = name
    obj.dimensions = (width_x, depth_z, height)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material)
    return obj


def join_objects(objects, object_name, version):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    ground = bpy.context.active_object
    ground.name = object_name
    ground.data.name = object_name + "Mesh"
    ground["stage"] = "2-8"
    ground["version"] = version
    ground["play_area_m"] = "120x120"
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    return ground


def export_ground(ground, output_x):
    bpy.ops.object.select_all(action="DESELECT")
    ground.select_set(True)
    bpy.context.view_layer.objects.active = ground
    result = bpy.ops.export_scene.directx_x(
        filepath=str(output_x),
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(output_x))
    normalize_x_file(output_x)
    data = output_x.read_bytes()
    if not data.startswith(b"xof ") or data.startswith(b"\xef\xbb\xbf"):
        raise RuntimeError("Invalid DirectX X output: " + str(output_x))


def build_boss_version():
    clear_scene()
    floor_material = make_material("Stage28BossBasalt", (0.20, 0.18, 0.17, 1.0))
    wall_material = make_material("Stage28BossWall", (0.11, 0.09, 0.08, 1.0))
    objects = []
    for name, x, z, width_x, depth_z in BOSS_FLOORS:
        objects.append(add_box_game(name, x, z, SURFACE_TOP_Y, width_x, depth_z,
                                    SURFACE_THICKNESS, floor_material))
    for name, x, z, width_x, depth_z in BOSS_WALLS:
        objects.append(add_box_game(name, x, z, 3.0, width_x, depth_z, 3.0, wall_material))
    ground = join_objects(objects, "Stage28BossGround", "boss")
    blend_path = STAGE_DIR / "stage_ground.blend"
    output_x = STAGE_DIR / "stage_ground.x"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    export_ground(ground, output_x)
    print("CREATED", blend_path)
    print("EXPORTED", output_x)


def build_cleared_version():
    clear_scene()
    floor_material = make_material("Stage28ClearedBasalt", (0.24, 0.22, 0.20, 1.0))
    objects = []
    for name, x, z, width_x, depth_z in CLEARED_FLOORS:
        objects.append(add_box_game(name, x, z, SURFACE_TOP_Y, width_x, depth_z,
                                    SURFACE_THICKNESS, floor_material))
    ground = join_objects(objects, "Stage28ClearedGround", "cleared")
    blend_path = STAGE_DIR / "stage_ground_cleared.blend"
    output_x = STAGE_DIR / "stage_ground_cleared.x"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    export_ground(ground, output_x)
    print("CREATED", blend_path)
    print("EXPORTED", output_x)


def main():
    STAGE_DIR.mkdir(parents=True, exist_ok=True)
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    build_boss_version()
    build_cleared_version()


main()
