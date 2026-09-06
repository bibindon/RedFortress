import math
import os

import bpy
from mathutils import Vector


OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
STONE_TEX = os.path.join(OUTPUT_DIR, "stone_steps.png")
IRON_TEX = os.path.join(OUTPUT_DIR, "forged_metal.png")
COBBLE_TEX = os.path.join(OUTPUT_DIR, "lever_cobble.png")
WOOD_TEX = os.path.join(OUTPUT_DIR, "lever_wood.png")


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for data_collection in (
        bpy.data.meshes,
        bpy.data.curves,
        bpy.data.materials,
        bpy.data.cameras,
        bpy.data.lights,
    ):
        for data_block in list(data_collection):
            data_collection.remove(data_block)


def fresh_materials():
    stone = make_material("LeverStone", (0.55, 0.55, 0.6), 0.85, 0.0, STONE_TEX)
    stone_dark = make_material("LeverStoneDark", (0.22, 0.22, 0.26), 0.9, 0.0)
    iron = make_material("LeverIron", (0.45, 0.46, 0.5), 0.55, 0.85, IRON_TEX)
    iron_dark = make_material("LeverIronDark", (0.16, 0.16, 0.18), 0.6, 0.8)
    gold = make_material("LeverGold", (0.85, 0.62, 0.2), 0.35, 1.0)
    cobble = make_material("LeverCobble", (0.9, 0.88, 0.85), 0.9, 0.0, COBBLE_TEX)
    woodtex = make_material("LeverWood", (0.95, 0.9, 0.85), 0.8, 0.0, WOOD_TEX)
    return stone, stone_dark, iron, iron_dark, gold, cobble, woodtex


def make_material(name, base_color, roughness, metallic, texture_path=None):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = (*base_color, 1.0)
    material.roughness = roughness
    material.metallic = metallic
    material["_x_power"] = 500.0
    material["_x_specular"] = (0.5, 0.5, 0.5)
    principled = next(
        (node for node in material.node_tree.nodes if node.type == "BSDF_PRINCIPLED"),
        None,
    )
    if principled is not None:
        principled.inputs["Base Color"].default_value = (*base_color, 1.0)
        principled.inputs["Roughness"].default_value = roughness
        principled.inputs["Metallic"].default_value = metallic
    if texture_path is not None:
        image = bpy.data.images.load(texture_path, check_existing=True)
        image.colorspace_settings.name = "sRGB"
        texture = material.node_tree.nodes.new("ShaderNodeTexImage")
        texture.image = image
        texture.extension = "REPEAT"
        material.node_tree.links.new(texture.outputs["Color"], principled.inputs["Base Color"])
        material["_x_texture_filename"] = os.path.basename(texture_path)
    else:
        material["_x_face_color"] = (*base_color, 1.0)
    return material


def apply_material(obj, material):
    obj.data.materials.append(material)


def uv_project(obj, cube_size):
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.cube_project(cube_size=cube_size, correct_aspect=True)
    bpy.ops.object.mode_set(mode="OBJECT")
    obj.select_set(False)


def add_box(name, bloc, bdims, material, uv_size, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_cube_add(location=bloc, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = bdims
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    uv_project(obj, uv_size)
    apply_material(obj, material)
    return obj


def add_rivet(name, bloc, radius, material):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=8, ring_count=4, radius=radius, location=bloc
    )
    obj = bpy.context.object
    obj.name = name
    obj.scale = (1.0, 0.45, 1.0)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    apply_material(obj, material)
    return obj


def bake_transforms():
    bpy.ops.object.select_all(action="DESELECT")
    for obj in [item for item in bpy.context.scene.objects if item.type == "MESH"]:
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        obj.select_set(False)


def join_named(frame_name, mesh_name):
    meshes = [item for item in bpy.context.scene.objects if item.type == "MESH"]
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    joined = bpy.context.object
    joined.name = frame_name
    bpy.ops.object.material_slot_remove_unused()
    joined["_x_frame_name"] = frame_name
    joined["_x_mesh_name"] = mesh_name
    joined.select_set(False)


def setup_preview(cam_loc, target, preview_path):
    bpy.ops.object.camera_add(location=cam_loc)
    camera = bpy.context.object
    camera.data.lens = 50.0
    bpy.context.scene.camera = camera
    direction = Vector(target) - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    bpy.ops.object.light_add(type="AREA", location=(cam_loc[0], cam_loc[1], cam_loc[2] + 3.0))
    key = bpy.context.object
    key.data.energy = 2500.0
    key.data.size = 5.0
    bpy.ops.object.light_add(type="SUN", location=(0.0, 0.0, 10.0))
    fill = bpy.context.object
    fill.data.energy = 1.2
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 900
    scene.render.resolution_y = 900
    scene.render.film_transparent = True
    scene.render.filepath = preview_path
    bpy.ops.render.render(write_still=True)
    print("PREVIEW", preview_path)


def remove_helpers():
    bpy.ops.object.select_all(action="DESELECT")
    for obj in list(bpy.context.scene.objects):
        if obj.type in ("CAMERA", "LIGHT"):
            obj.select_set(True)
    bpy.ops.object.delete(use_global=False)


def export_current(blend_name, x_name, frame_name, mesh_name):
    bake_transforms()
    join_named(frame_name, mesh_name)
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUTPUT_DIR, blend_name))
    result = bpy.ops.export_scene.directx_x(
        filepath=os.path.join(OUTPUT_DIR, x_name),
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
        triangulate=True,
        unweld_on_export=False,
    )
    print("EXPORT_RESULT", x_name, result)


# File coords: opening file -Z, bottom origin. Blender = (fx, -fz, fy).
def B(fx, fy, fz):
    return (fx, -fz, fy)


def D(dx, dy, dz):
    return (dx, dz, dy)


def build_box(stone, stone_dark, iron, cobble):
    for sx in (-1.0, 1.0):
        for sz in (-1.0, 1.0):
            add_box("Pillar", B(sx * 2.65, 3.0, sz * 2.65), D(0.7, 6.0, 0.7), cobble, 3.0)
    for sx in (-1.0, 1.0):
        add_box("WallSide", B(sx * 2.68, 3.0, 0.0), D(0.56, 6.0, 4.6), cobble, 3.0)
    add_box("WallBack", B(0.0, 3.0, 2.68), D(4.6, 6.0, 0.56), cobble, 3.0)
    for y0 in (1.9, 4.0):
        for sx in (-1.0, 1.0):
            add_box("BandSide", B(sx * 2.96, y0 + 0.15, 0.0), D(0.12, 0.3, 4.6), iron, 1.0)
        add_box("BandBack", B(0.0, y0 + 0.15, 2.96), D(4.6, 0.3, 0.12), iron, 1.0)
    for sx in (-1.0, 1.0):
        add_box("Jamb", B(sx * 2.62, 2.99, -2.72), D(0.56, 5.98, 0.6), iron, 1.0)
    add_box("Lintel", B(0.0, 5.63, -2.70), D(5.9, 0.7, 0.56), iron, 1.0)
    add_box("Slab", B(0.0, 0.25, 0.0), D(6.0, 0.5, 6.0), cobble, 3.0)
    add_box("Medallion", B(0.0, 0.495, 0.0), D(2.4, 0.02, 2.4), cobble, 5.0)


def build_door(woodtex, wood_dark, iron, gold):
    add_box("DoorCore", B(0.0, 3.0, 0.125), D(6.0, 6.0, 0.51), wood_dark, 1.5)
    for fz in (0.5, -0.25):
        for i in range(5):
            px = -2.28 + i * 1.14
            add_box("Plank", B(px, 3.0, fz - (0.06 if fz > 0.0 else -0.06)),
                    D(1.04, 6.0, 0.12), woodtex, 1.5)
    for fz in (0.5, -0.25):
        cz = 0.50 if fz > 0.0 else -0.25
        add_box("Rail", B(0.0, 1.2, cz), D(4.8, 0.35, 0.06), iron, 1.0)
        add_box("Rail", B(0.0, 4.8, cz), D(4.8, 0.35, 0.06), iron, 1.0)
    for fz in (0.53, -0.28):
        for px in (-2.0, -1.0, 0.0, 1.0, 2.0):
            for py in (1.2, 4.8):
                add_rivet("RivetRail", (px, -fz, py), 0.055, gold)


def build_floor_cobble(cobble):
    add_box("FloorSlab", B(0.0, 0.5, 0.0), D(10.0, 1.0, 10.0), cobble, 3.0)


def build_lever_base(stone_dark, iron):
    add_box("Pedestal", (0.0, 0.0, -0.05), (0.7, 0.5, 0.14), stone_dark, 0.7)
    add_box("Collar", (0.0, 0.0, 0.07), (0.4, 0.3, 0.1), iron, 0.7)


def build_lever_handle(iron, gold):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=10, radius=0.055, depth=1.02, location=(0.0, 0.0, 0.0)
    )
    rod = bpy.context.object
    rod.name = "Rod"
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    apply_material(rod, iron)
    for ez in (-0.51, 0.51):
        bpy.ops.mesh.primitive_uv_sphere_add(
            segments=10, ring_count=6, radius=0.11, location=(0.0, 0.0, ez)
        )
        knob = bpy.context.object
        knob.name = "Knob"
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
        apply_material(knob, gold)


def delete_by_name(names):
    for obj in list(bpy.context.scene.objects):
        if obj.name in names or obj.name.startswith("Knob"):
            bpy.data.objects.remove(obj, do_unlink=True)


def main():
    clear_scene()
    stone, stone_dark, iron, iron_dark, gold, cobble, woodtex = fresh_materials()
    build_box(stone, stone_dark, iron, cobble)
    setup_preview((9.0, 10.0, 7.0), (0.0, 0.0, 3.0),
                  os.path.join(OUTPUT_DIR, "lever2_box_preview.png"))
    remove_helpers()
    export_current("lever2_box.blend", "lever_box_new.x", "LeverBox", "LeverBoxGeo")

    clear_scene()
    stone, stone_dark, iron, iron_dark, gold, cobble, woodtex = fresh_materials()
    wood_dark = make_material("LeverWoodDark", (0.30, 0.20, 0.12), 0.85, 0.0)
    build_door(woodtex, wood_dark, iron, gold)
    setup_preview((7.0, -8.0, 5.0), (0.0, 0.0, 3.0),
                  os.path.join(OUTPUT_DIR, "lever2_door_preview.png"))
    remove_helpers()
    export_current("lever2_door.blend", "lever_box_door_new.x", "LeverBoxDoor", "LeverBoxDoorGeo")

    clear_scene()
    stone, stone_dark, iron, iron_dark, gold, cobble, woodtex = fresh_materials()
    build_lever_base(stone_dark, iron)
    build_lever_handle(iron, gold)
    setup_preview((2.2, -2.8, 1.8), (0.0, 0.0, 0.3),
                  os.path.join(OUTPUT_DIR, "lever2_lever_preview.png"))
    remove_helpers()
    delete_by_name(("Rod", "Knob"))
    export_current("lever2_lever_base.blend", "lever_base_new.x", "LeverBase", "LeverBaseGeo")

    clear_scene()
    stone, stone_dark, iron, iron_dark, gold, cobble, woodtex = fresh_materials()
    build_lever_handle(iron, gold)
    remove_helpers()
    export_current("lever2_lever_handle.blend", "lever_handle_new.x", "LeverHandle", "LeverHandleGeo")

    clear_scene()
    stone, stone_dark, iron, iron_dark, gold, cobble, woodtex = fresh_materials()
    build_floor_cobble(cobble)
    setup_preview((9.0, -9.0, 7.0), (0.0, 0.0, 0.5),
                  os.path.join(OUTPUT_DIR, "lever2_floor_preview.png"))
    remove_helpers()
    export_current("lever2_floor.blend", "lever_box_floor_new.x", "LeverBoxFloor", "LeverBoxFloorGeo")


main()
