import math
import os

import bpy


OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))


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


def make_material(name, base_color):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = (*base_color, 1.0)
    material.roughness = 0.9
    material.metallic = 0.0
    principled = next(
        (node for node in material.node_tree.nodes if node.type == "BSDF_PRINCIPLED"),
        None,
    )
    if principled is not None:
        principled.inputs["Base Color"].default_value = (*base_color, 1.0)
        principled.inputs["Roughness"].default_value = 0.9
        principled.inputs["Metallic"].default_value = 0.0
    return material


def add_box(name, blender_location, blender_dimensions, material, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_cube_add(location=blender_location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = blender_dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    apply_material(obj, material)
    return obj


def apply_material(obj, material):
    obj.data.materials.append(material)


def bake_mesh_transforms_for_directx():
    bpy.ops.object.select_all(action="DESELECT")
    for obj in [item for item in bpy.context.scene.objects if item.type == "MESH"]:
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        obj.select_set(False)


def join_meshes(frame_name, mesh_name):
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
    return joined


def export_proxy(blend_name, x_name, frame_name, mesh_name):
    bake_mesh_transforms_for_directx()
    join_meshes(frame_name, mesh_name)
    blend_path = os.path.join(OUTPUT_DIR, blend_name)
    x_path = os.path.join(OUTPUT_DIR, x_name)
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    result = bpy.ops.export_scene.directx_x(
        filepath=x_path,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
        triangulate=True,
        unweld_on_export=False,
    )
    print("EXPORT_RESULT", x_name, result)
    print("MESH_OBJECT_COUNT", len([obj for obj in bpy.context.scene.objects if obj.type == "MESH"]))


def gx(blender_game_x, blender_game_y, blender_game_z):
    return (blender_game_x, blender_game_z, blender_game_y)


def gd(game_dims):
    return (game_dims[0], game_dims[2], game_dims[1])


def build_floor(material):
    add_box("FloorSlab", gx(0.0, 0.5, 0.0), gd((10.0, 1.0, 10.0)), material)


def build_box(material):
    add_box("WallWest", gx(-2.75, 3.0, 0.0), gd((0.5, 6.0, 6.0)), material)
    add_box("WallEast", gx(2.75, 3.0, 0.0), gd((0.5, 6.0, 6.0)), material)
    add_box("WallBack", gx(0.0, 3.0, 2.75), gd((6.0, 6.0, 0.5)), material)
    add_box("InnerFloor", gx(0.0, 0.45, 0.0), gd((5.0, 0.1, 5.0)), material)


def build_door(material):
    add_box("DoorSlab", gx(0.0, 3.0, -0.125), gd((6.0, 6.0, 0.75)), material)


def main():
    material = None

    clear_scene()
    material = make_material("LeverCollision", (0.5, 0.5, 0.55))
    build_floor(material)
    export_proxy(
        "lever_box_floor_collision.blend",
        "lever_box_floor_collision.x",
        "LeverBoxFloorCollision",
        "LeverBoxFloorCollisionGeo",
    )

    clear_scene()
    material = make_material("LeverCollision", (0.5, 0.5, 0.55))
    build_box(material)
    export_proxy(
        "lever_box_collision.blend",
        "lever_box_collision.x",
        "LeverBoxCollision",
        "LeverBoxCollisionGeo",
    )

    clear_scene()
    material = make_material("LeverCollision", (0.5, 0.5, 0.55))
    build_door(material)
    export_proxy(
        "lever_box_door_collision.blend",
        "lever_box_door_collision.x",
        "LeverBoxDoorCollision",
        "LeverBoxDoorCollisionGeo",
    )


main()
