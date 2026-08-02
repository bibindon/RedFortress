import math
import os

import bpy
from mathutils import Vector


OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
BLEND_PATH = os.path.join(OUTPUT_DIR, "collision_wall_tall.blend")
X_PATH = os.path.join(OUTPUT_DIR, "collision_wall_tall.x")
COLLISION_X_PATH = os.path.join(OUTPUT_DIR, "collision_wall_tall_collision.x")
PREVIEW_PATH = os.path.join(OUTPUT_DIR, "collision_wall_tall_preview.png")
TEXTURE_PATH = os.path.join(OUTPUT_DIR, "wood.png")
SPECULAR_POWER_MAX = 500.0


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


def make_material(name, base_color, roughness, metallic, texture_path=None):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = (*base_color, 1.0)
    material.roughness = roughness
    material.metallic = metallic
    material["_x_power"] = SPECULAR_POWER_MAX
    material["_x_specular"] = (1.0, 1.0, 1.0)

    principled = next(
        (node for node in material.node_tree.nodes if node.type == "BSDF_PRINCIPLED"),
        None,
    )
    if principled is None:
        principled = material.node_tree.nodes.new("ShaderNodeBsdfPrincipled")
        output = next(
            (node for node in material.node_tree.nodes if node.type == "OUTPUT_MATERIAL"),
            None,
        )
        if output is None:
            output = material.node_tree.nodes.new("ShaderNodeOutputMaterial")
        material.node_tree.links.new(principled.outputs["BSDF"], output.inputs["Surface"])

    principled.inputs["Base Color"].default_value = (*base_color, 1.0)
    principled.inputs["Roughness"].default_value = roughness
    principled.inputs["Metallic"].default_value = metallic
    specular_input = principled.inputs.get("Specular IOR Level")
    if specular_input is not None:
        specular_input.default_value = 1.0

    if texture_path is not None:
        image = bpy.data.images.load(texture_path, check_existing=True)
        image.colorspace_settings.name = "sRGB"
        texture = material.node_tree.nodes.new("ShaderNodeTexImage")
        texture.name = f"{name}_Texture"
        texture.image = image
        texture.extension = "REPEAT"
        material.node_tree.links.new(texture.outputs["Color"], principled.inputs["Base Color"])
        material["_x_texture_filename"] = "wood.png"

    return material


def apply_material(obj, material):
    obj.data.materials.append(material)


def cube_project_uv(obj, cube_size=1.0):
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.cube_project(cube_size=cube_size, correct_aspect=True)
    bpy.ops.object.mode_set(mode="OBJECT")
    obj.select_set(False)


def add_box(name, location, dimensions, material, rotation=(0.0, 0.0, 0.0), uv_project=True):
    bpy.ops.mesh.primitive_cube_add(location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if uv_project:
        cube_project_uv(obj, cube_size=0.7)
    apply_material(obj, material)
    return obj


def add_rivet(name, location, material, radius=0.044):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=6,
        radius=radius,
        depth=0.026,
        location=location,
        rotation=(0.0, math.radians(90.0), 0.0),
    )
    obj = bpy.context.object
    obj.name = name
    apply_material(obj, material)
    return obj


def build_tall_wall():
    wood = make_material("Tall_Wall_Aged_Walnut", (0.56, 0.31, 0.18), 0.42, 0.0, TEXTURE_PATH)
    dark_wood = make_material("Tall_Wall_Dark_Oak", (0.24, 0.095, 0.045), 0.34, 0.0, TEXTURE_PATH)
    iron = make_material("Tall_Wall_Forged_Iron", (0.12, 0.14, 0.15), 0.14, 0.90)
    brass = make_material("Tall_Wall_Brass_Rivets", (0.58, 0.28, 0.055), 0.10, 0.90)

    plank_offsets = (-0.026, 0.018, -0.012, 0.028, -0.020, 0.010, -0.030, 0.022, -0.008, 0.016, -0.024, 0.012, -0.018, 0.026, -0.010, 0.020, -0.014)
    plank_depths = (0.69, 0.72, 0.70, 0.73, 0.71, 0.69, 0.72, 0.70, 0.71, 0.70, 0.73, 0.69, 0.72, 0.71, 0.70, 0.73, 0.71)
    for index in range(17):
        z = -2.56 + (index * 0.32)
        length = 7.72 - ((index % 4) * 0.035)
        add_box(
            f"Tall_Hand_Hewn_Plank_{index + 1:02d}",
            (plank_offsets[index], 0.0, z),
            (plank_depths[index], length, 0.292),
            wood,
        )

    post_positions = (-3.73, -1.86, 0.0, 1.86, 3.73)
    for index, y in enumerate(post_positions):
        width = 0.31
        if index == 0 or index == 4:
            width = 0.38
        add_box(
            f"Tall_Massive_Oak_Post_{index + 1:02d}",
            (0.0, y, 0.0),
            (0.89, width, 5.74),
            dark_wood,
        )

    brace_angle = math.radians(36.0)
    brace_specs = (
        (-1.90, -1.35, brace_angle),
        (-1.90, 1.35, -brace_angle),
        (1.90, -1.35, -brace_angle),
        (1.90, 1.35, brace_angle),
    )
    for face_index, x in enumerate((-0.405, 0.405)):
        for brace_index, (y, z, angle) in enumerate(brace_specs):
            if x < 0.0:
                angle = -angle
            add_box(
                f"Tall_Diagonal_Brace_{face_index + 1}_{brace_index + 1}",
                (x, y, z),
                (0.135, 3.95, 0.205),
                dark_wood,
                rotation=(angle, 0.0, 0.0),
            )

    for band_index, z in enumerate((-1.92, 0.0, 1.92)):
        for face_index, x in enumerate((-0.455, 0.455)):
            add_box(
                f"Tall_Iron_Band_{band_index + 1}_{face_index + 1}",
                (x, 0.0, z),
                (0.065, 7.48, 0.125),
                iron,
            )

    for z, label in ((2.85, "Top"), (-2.85, "Bottom")):
        add_box(
            f"Tall_{label}_Forged_Cap",
            (0.0, 0.0, z),
            (0.97, 7.90, 0.105),
            iron,
        )

    for face_index, x in enumerate((-0.450, 0.450)):
        for band_index, z in enumerate((-1.92, 0.0, 1.92)):
            for rivet_index, y in enumerate((-2.80, 0.0, 2.80)):
                add_rivet(
                    f"Tall_Band_Rivet_{face_index + 1}_{band_index + 1}_{rivet_index + 1:02d}",
                    (x, y, z),
                    brass,
                )


def join_visual_meshes():
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    wall = bpy.context.object
    wall.name = "Collision_Wall_Tall_Visual"
    bpy.ops.object.material_slot_remove_unused()
    wall["_x_frame_name"] = "Collision_Wall_Tall_Visual"
    wall["_x_mesh_name"] = "Collision_Wall_Tall_VisualGeo"
    wall.select_set(False)
    return wall


def build_collision_mesh(material):
    collision = add_box(
        "Collision_Wall_Tall_Collision",
        (0.0, 0.0, 0.0),
        (0.90, 7.92, 5.86),
        material,
        uv_project=False,
    )
    collision.display_type = "WIRE"
    collision.hide_render = True
    collision["_x_frame_name"] = "Collision_Wall_Tall_Collision"
    collision["_x_mesh_name"] = "Collision_Wall_Tall_CollisionGeo"
    return collision


def add_camera_and_lights():
    bpy.ops.object.camera_add(location=(8.0, -11.8, 7.0))
    camera = bpy.context.object
    camera.name = "Tall_Wall_Presentation_Camera"
    camera.data.lens = 58.0
    camera.data.sensor_width = 36.0
    bpy.context.scene.camera = camera

    target = Vector((0.0, 0.0, 0.0))
    direction = target - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()

    bpy.ops.object.light_add(type="AREA", location=(4.5, -5.0, 8.0))
    key = bpy.context.object
    key.name = "Tall_Wall_Warm_Key"
    key.data.energy = 1350.0
    key.data.shape = "DISK"
    key.data.size = 4.5
    key.data.color = (1.0, 0.78, 0.58)
    key.rotation_euler = ((target - key.location).to_track_quat("-Z", "Y").to_euler())

    bpy.ops.object.light_add(type="AREA", location=(3.0, 6.0, 3.5))
    fill = bpy.context.object
    fill.name = "Tall_Wall_Cool_Fill"
    fill.data.energy = 900.0
    fill.data.size = 5.0
    fill.data.color = (0.55, 0.72, 1.0)
    fill.rotation_euler = ((target - fill.location).to_track_quat("-Z", "Y").to_euler())

    bpy.ops.object.light_add(type="AREA", location=(-4.0, 0.0, 5.0))
    rim = bpy.context.object
    rim.name = "Tall_Wall_Edge_Light"
    rim.data.energy = 1050.0
    rim.data.size = 3.0
    rim.data.color = (1.0, 0.45, 0.22)
    rim.rotation_euler = ((target - rim.location).to_track_quat("-Z", "Y").to_euler())


def configure_scene():
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 960
    scene.render.resolution_y = 640
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = PREVIEW_PATH
    scene.render.film_transparent = False
    scene.world.color = (0.012, 0.016, 0.022)
    scene.view_settings.look = "AgX - Medium High Contrast"
    scene.render.fps = 30
    scene.frame_start = 1
    scene.frame_end = 1


def bake_mesh_transforms():
    bpy.ops.object.select_all(action="DESELECT")
    for obj in [item for item in bpy.context.scene.objects if item.type == "MESH"]:
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        obj.select_set(False)


def export_selected(filepath):
    result = bpy.ops.export_scene.directx_x(
        filepath=filepath,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
        triangulate=True,
        unweld_on_export=False,
    )
    print("EXPORT_RESULT", filepath, result)


def select_only(obj):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj


def main():
    clear_scene()
    build_tall_wall()
    visual = join_visual_meshes()
    collision_material = make_material("Tall_Wall_Collision", (0.35, 0.35, 0.35), 1.0, 0.0)
    collision = build_collision_mesh(collision_material)
    bake_mesh_transforms()
    add_camera_and_lights()
    configure_scene()

    select_only(visual)
    export_selected(X_PATH)
    select_only(collision)
    export_selected(COLLISION_X_PATH)

    bpy.ops.object.select_all(action="DESELECT")
    bpy.ops.file.pack_all()
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    bpy.context.scene.render.filepath = PREVIEW_PATH
    bpy.ops.render.render(write_still=True)
    print("BLEND_PATH", BLEND_PATH)
    print("X_PATH", X_PATH)
    print("COLLISION_X_PATH", COLLISION_X_PATH)
    print("PREVIEW_PATH", PREVIEW_PATH)
    print("MESH_OBJECT_COUNT", len([obj for obj in bpy.context.scene.objects if obj.type == "MESH"]))
    print("VISUAL_DIMENSIONS", tuple(round(value, 3) for value in visual.dimensions))
    print("COLLISION_DIMENSIONS", tuple(round(value, 3) for value in collision.dimensions))


main()