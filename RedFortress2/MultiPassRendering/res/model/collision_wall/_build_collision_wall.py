import math
import os

import bpy
from mathutils import Vector


OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
BLEND_PATH = os.path.join(OUTPUT_DIR, "collision_wall.blend")
X_PATH = os.path.join(OUTPUT_DIR, "collision_wall.x")
PREVIEW_PATH = os.path.join(OUTPUT_DIR, "collision_wall_preview.png")
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


def add_box(name, location, dimensions, material, bevel=0.025, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_cube_add(location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    cube_project_uv(obj, cube_size=0.7)
    apply_material(obj, material)

    if bevel > 0.0:
        modifier = obj.modifiers.new(name="Crafted_Edges", type="BEVEL")
        modifier.width = bevel
        modifier.segments = 3
        modifier.limit_method = "ANGLE"
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.modifier_apply(modifier=modifier.name)
        obj.select_set(False)
    return obj


def add_rivet(name, location, material, radius=0.044):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=16,
        ring_count=8,
        radius=radius,
        location=location,
    )
    obj = bpy.context.object
    obj.name = name
    apply_material(obj, material)
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    return obj


def add_torus(name, location, material):
    bpy.ops.mesh.primitive_torus_add(
        major_segments=40,
        minor_segments=10,
        location=location,
        rotation=(0.0, math.radians(90.0), 0.0),
        major_radius=0.34,
        minor_radius=0.042,
    )
    obj = bpy.context.object
    obj.name = name
    apply_material(obj, material)
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    return obj


def add_emblem_disc(name, x, material):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=12,
        radius=0.255,
        depth=0.065,
        location=(x, 0.0, 0.0),
        rotation=(0.0, math.radians(90.0), 0.0),
    )
    obj = bpy.context.object
    obj.name = name
    apply_material(obj, material)
    bevel = obj.modifiers.new(name="Forged_Rim", type="BEVEL")
    bevel.width = 0.018
    bevel.segments = 2
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=bevel.name)
    return obj


def add_camera_and_lights():
    bpy.ops.object.camera_add(location=(6.2, -9.4, 4.7))
    camera = bpy.context.object
    camera.name = "Presentation_Camera"
    camera.data.lens = 55.0
    camera.data.sensor_width = 36.0
    bpy.context.scene.camera = camera

    target = Vector((0.0, 0.0, 0.0))
    direction = target - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()

    bpy.ops.object.light_add(type="AREA", location=(4.5, -4.0, 5.5))
    key = bpy.context.object
    key.name = "Warm_Key"
    key.data.energy = 1100.0
    key.data.shape = "DISK"
    key.data.size = 4.0
    key.data.color = (1.0, 0.78, 0.58)
    key.rotation_euler = ((target - key.location).to_track_quat("-Z", "Y").to_euler())

    bpy.ops.object.light_add(type="AREA", location=(3.0, 5.5, 2.2))
    fill = bpy.context.object
    fill.name = "Cool_Fill"
    fill.data.energy = 750.0
    fill.data.size = 5.0
    fill.data.color = (0.55, 0.72, 1.0)
    fill.rotation_euler = ((target - fill.location).to_track_quat("-Z", "Y").to_euler())

    bpy.ops.object.light_add(type="AREA", location=(-3.0, 0.0, 4.0))
    rim = bpy.context.object
    rim.name = "Edge_Light"
    rim.data.energy = 900.0
    rim.data.size = 3.0
    rim.data.color = (1.0, 0.45, 0.22)
    rim.rotation_euler = ((target - rim.location).to_track_quat("-Z", "Y").to_euler())


def build_wall():
    wood = make_material("Aged_Walnut_Planks", (0.56, 0.31, 0.18), 0.42, 0.0, TEXTURE_PATH)
    dark_wood = make_material("Dark_Oak_Bracing", (0.24, 0.095, 0.045), 0.34, 0.0, TEXTURE_PATH)
    iron = make_material("Blackened_Iron", (0.055, 0.065, 0.072), 0.16, 0.88)
    edge_iron = make_material("Worn_Steel_Edges", (0.20, 0.23, 0.24), 0.12, 0.92)
    brass = make_material("Brass_Rivets", (0.58, 0.28, 0.055), 0.10, 0.90)

    plank_offsets = (-0.025, 0.018, -0.010, 0.030, -0.018, 0.012, -0.030, 0.022, -0.008)
    plank_depths = (0.69, 0.72, 0.70, 0.73, 0.71, 0.69, 0.72, 0.70, 0.71)
    for index in range(9):
        z = -1.28 + (index * 0.32)
        length = 7.72 - ((index % 3) * 0.035)
        add_box(
            f"Hand_Hewn_Plank_{index + 1:02d}",
            (plank_offsets[index], 0.0, z),
            (plank_depths[index], length, 0.292),
            wood,
            bevel=0.038,
        )

    for index, y in enumerate((-3.73, -1.86, 0.0, 1.86, 3.73)):
        width = 0.31
        if index in (0, 4):
            width = 0.38
        add_box(
            f"Massive_Oak_Post_{index + 1:02d}",
            (0.0, y, 0.0),
            (0.89, width, 2.86),
            dark_wood,
            bevel=0.055,
        )

    brace_angle = math.radians(36.0)
    brace_specs = (
        (-1.86, brace_angle),
        (1.86, -brace_angle),
    )
    for face_index, x in enumerate((-0.405, 0.405)):
        for brace_index, (y, angle) in enumerate(brace_specs):
            if x < 0.0:
                angle = -angle
            add_box(
                f"Diagonal_Brace_{face_index + 1}_{brace_index + 1}",
                (x, y, 0.0),
                (0.135, 4.05, 0.205),
                dark_wood,
                bevel=0.035,
                rotation=(angle, 0.0, 0.0),
            )

    for z_index, z in enumerate((-0.88, 0.88)):
        for face_index, x in enumerate((-0.455, 0.455)):
            add_box(
                f"Iron_Band_{z_index + 1}_{face_index + 1}",
                (x, 0.0, z),
                (0.065, 7.48, 0.125),
                iron,
                bevel=0.018,
            )

    for z, label in ((1.425, "Top"), (-1.425, "Bottom")):
        add_box(
            f"{label}_Forged_Cap",
            (0.0, 0.0, z),
            (0.97, 7.90, 0.105),
            edge_iron,
            bevel=0.024,
        )

    rivet_y_positions = (-3.52, -2.80, -2.08, -1.36, -0.68, 0.0, 0.68, 1.36, 2.08, 2.80, 3.52)
    for face_index, x in enumerate((-0.450, 0.450)):
        for band_index, z in enumerate((-0.88, 0.88)):
            for rivet_index, y in enumerate(rivet_y_positions):
                add_rivet(
                    f"Band_Rivet_{face_index + 1}_{band_index + 1}_{rivet_index + 1:02d}",
                    (x, y, z),
                    brass,
                )

    for face_index, x in enumerate((-0.446, 0.446)):
        add_torus(f"Central_Emblem_Ring_{face_index + 1}", (x, 0.0, 0.0), edge_iron)
        add_emblem_disc(f"Central_Emblem_Disc_{face_index + 1}", x, iron)
        add_box(
            f"Emblem_Vertical_{face_index + 1}",
            (x, 0.0, 0.0),
            (0.075, 0.10, 0.34),
            brass,
            bevel=0.022,
        )
        add_box(
            f"Emblem_Horizontal_{face_index + 1}",
            (x, 0.0, 0.0),
            (0.075, 0.34, 0.10),
            brass,
            bevel=0.022,
        )

    for face_index, x in enumerate((-0.450, 0.450)):
        for post_index, y in enumerate((-3.73, -1.86, 1.86, 3.73)):
            for z_index, z in enumerate((-1.17, -0.58, 0.0, 0.58, 1.17)):
                add_rivet(
                    f"Post_Rivet_{face_index + 1}_{post_index + 1}_{z_index + 1}",
                    (x, y, z),
                    edge_iron,
                    radius=0.041,
                )


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


def bake_mesh_transforms_for_directx():
    identity_matrix = (
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    )
    bpy.ops.object.select_all(action="DESELECT")
    for obj in [item for item in bpy.context.scene.objects if item.type == "MESH"]:
        bpy.context.view_layer.objects.active = obj
        obj.select_set(True)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        obj["_x_frame_ftm"] = identity_matrix
        obj.select_set(False)


def join_visual_meshes():
    meshes = [item for item in bpy.context.scene.objects if item.type == "MESH"]
    bpy.ops.object.select_all(action="DESELECT")
    for obj in meshes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    wall = bpy.context.object
    wall.name = "Collision_Wall_Visual"
    bpy.ops.object.material_slot_remove_unused()
    wall["_x_frame_name"] = "Collision_Wall_Visual"
    wall["_x_mesh_name"] = "Collision_Wall_VisualGeo"
    wall["_x_frame_ftm"] = (
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0,
    )
    wall.select_set(False)


def main():
    clear_scene()
    build_wall()
    bake_mesh_transforms_for_directx()
    join_visual_meshes()
    add_camera_and_lights()
    configure_scene()

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    bpy.ops.render.render(write_still=True)
    result = bpy.ops.export_scene.directx_x(
        filepath=X_PATH,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
        triangulate=True,
        unweld_on_export=False,
    )
    print("EXPORT_RESULT", result)
    print("BLEND_PATH", BLEND_PATH)
    print("X_PATH", X_PATH)
    print("PREVIEW_PATH", PREVIEW_PATH)
    print("MESH_OBJECT_COUNT", len([obj for obj in bpy.context.scene.objects if obj.type == "MESH"]))
    print("MATERIAL_POWERS", [(material.name, material.get("_x_power")) for material in bpy.data.materials])


main()
