# -*- coding: utf-8 -*-
"""Blender公式DirectX Xエクスポーターでステージ2-3専用S字地形を生成する。"""

from pathlib import Path

import bpy


ROOT = Path(__file__).resolve().parents[1]
STAGE_DIR = ROOT / "res" / "model" / "stage_2_3"
OUTPUT_X = STAGE_DIR / "stage_ground.x"
OUTPUT_BLEND = STAGE_DIR / "stage_ground.blend"
SURFACE_TOP_Y = 0.4
SURFACE_THICKNESS = 0.4


COURSE_RECTS = (
    ("south_lane", -20.0, -39.5, 12.0, 29.0),
    ("south_crossing", 0.0, -25.0, 40.0, 12.0),
    ("east_lane", 20.0, -7.5, 12.0, 35.0),
    ("north_crossing", 0.0, 10.0, 40.0, 12.0),
    ("north_lane", -20.0, 32.0, 12.0, 44.0),
    ("start_terrace", -20.0, -54.0, 16.0, 10.0),
    ("goal_terrace", -20.0, 54.0, 16.0, 10.0),
)


CORNER_RECTS = (
    ("corner_01", -20.0, -25.0, 14.0, 14.0),
    ("corner_02", 20.0, -25.0, 14.0, 14.0),
    ("corner_03", 20.0, 10.0, 14.0, 14.0),
    ("corner_04", -20.0, 10.0, 14.0, 14.0),
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


def make_material():
    material = bpy.data.materials.new("Stage23Basalt")
    material.diffuse_color = (0.18, 0.20, 0.22, 1.0)
    material.specular_intensity = 0.12
    material.roughness = 0.92
    return material


def add_floor(name, x, z, width_x, depth_z, material):
    bpy.ops.mesh.primitive_cube_add(
        location=(x, -z, SURFACE_TOP_Y - SURFACE_THICKNESS * 0.5))
    obj = bpy.context.active_object
    obj.name = name
    obj.dimensions = (width_x, depth_z, SURFACE_THICKNESS)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material)
    return obj


def export_ground(ground):
    bpy.ops.object.select_all(action="DESELECT")
    ground.select_set(True)
    bpy.context.view_layer.objects.active = ground
    result = bpy.ops.export_scene.directx_x(
        filepath=str(OUTPUT_X),
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(OUTPUT_X))
    normalize_x_file(OUTPUT_X)
    data = OUTPUT_X.read_bytes()
    if not data.startswith(b"xof "):
        raise RuntimeError("Invalid DirectX X header")
    if data.startswith(b"\xef\xbb\xbf"):
        raise RuntimeError("DirectX X output must not contain a BOM")


def main():
    STAGE_DIR.mkdir(parents=True, exist_ok=True)
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    clear_scene()
    material = make_material()

    objects = []
    for name, x, z, width_x, depth_z in COURSE_RECTS + CORNER_RECTS:
        objects.append(add_floor(name, x, z, width_x, depth_z, material))

    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    ground = bpy.context.active_object
    ground.name = "Stage23Ground"
    ground.data.name = "Stage23GroundMesh"
    ground["stage"] = "2-3"
    ground["play_area_m"] = "60x120"
    ground["course_shape"] = "S"
    ground["route_length_m"] = 188.0
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)

    bpy.ops.wm.save_as_mainfile(filepath=str(OUTPUT_BLEND))
    export_ground(ground)
    print("CREATED", OUTPUT_BLEND)
    print("EXPORTED", OUTPUT_X)


main()
