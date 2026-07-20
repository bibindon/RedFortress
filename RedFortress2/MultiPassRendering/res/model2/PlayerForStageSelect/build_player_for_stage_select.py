import math
import os

import bpy
from mathutils import Vector


OUTPUT_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
BLEND_PATH = os.path.join(OUTPUT_DIRECTORY, "PlayerForStageSelect.blend")
BASE_X_PATH = os.path.join(OUTPUT_DIRECTORY, "PlayerForStageSelect.x")
IDLE_X_PATH = os.path.join(OUTPUT_DIRECTORY, "PlayerForStageSelect.000.x")
WALK_X_PATH = os.path.join(OUTPUT_DIRECTORY, "PlayerForStageSelect.walk.x")


def clear_scene():
    if bpy.context.object and bpy.context.object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for data_collection in (bpy.data.meshes, bpy.data.curves, bpy.data.armatures, bpy.data.materials):
        for data_block in list(data_collection):
            if data_block.users == 0:
                data_collection.remove(data_block)


def create_material(name, color, metallic=0.0, roughness=0.55):
    material = bpy.data.materials.new(name)
    material.diffuse_color = (*color, 1.0)
    material.use_nodes = True
    shader = None
    for node in material.node_tree.nodes:
        if node.type == "BSDF_PRINCIPLED":
            shader = node
            break
    if shader is None:
        raise RuntimeError("Principled BSDF node was not created for material: " + name)
    shader.inputs["Base Color"].default_value = (*color, 1.0)
    shader.inputs["Metallic"].default_value = metallic
    shader.inputs["Roughness"].default_value = roughness
    return material


def finish_part(obj, material, bone_name, smooth=True):
    obj.data.materials.append(material)
    group = obj.vertex_groups.new(name=bone_name)
    group.add(range(len(obj.data.vertices)), 1.0, "REPLACE")
    if smooth:
        for polygon in obj.data.polygons:
            polygon.use_smooth = True
    obj["stage_select_bone"] = bone_name
    return obj


def add_uv_sphere(name, location, scale, material, bone_name, segments=12, rings=8):
    bpy.ops.mesh.primitive_uv_sphere_add(segments=segments, ring_count=rings, location=location)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    return finish_part(obj, material, bone_name)


def add_cylinder(name, location, radius, depth, material, bone_name, vertices=10, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=depth, location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    return finish_part(obj, material, bone_name)


def add_cone(name, location, radius_top, radius_bottom, depth, material, bone_name, vertices=10, rotation=(0.0, 0.0, 0.0)):
    bpy.ops.mesh.primitive_cone_add(vertices=vertices, radius1=radius_bottom, radius2=radius_top, depth=depth, location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    return finish_part(obj, material, bone_name)


def add_box(name, location, scale, material, bone_name, rotation=(0.0, 0.0, 0.0), bevel=0.0):
    bpy.ops.mesh.primitive_cube_add(location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if bevel > 0.0:
        modifier = obj.modifiers.new(name="SmallBevel", type="BEVEL")
        modifier.width = bevel
        modifier.segments = 1
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.modifier_apply(modifier=modifier.name)
    return finish_part(obj, material, bone_name, smooth=False)


def add_curve(name, points, bevel_depth, material, bone_name, cyclic=False):
    curve = bpy.data.curves.new(name=name + "Curve", type="CURVE")
    curve.dimensions = "3D"
    curve.resolution_u = 1
    curve.bevel_depth = bevel_depth
    curve.bevel_resolution = 0
    spline = curve.splines.new("POLY")
    spline.points.add(len(points) - 1)
    for point, coordinate in zip(spline.points, points):
        point.co = (*coordinate, 1.0)
    spline.use_cyclic_u = cyclic
    obj = bpy.data.objects.new(name, curve)
    bpy.context.collection.objects.link(obj)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.convert(target="MESH")
    return finish_part(obj, material, bone_name)


def add_bicorne(material_black, material_gold):
    vertices = [
        (-0.34, -0.12, 1.705), (0.0, -0.16, 1.905), (0.34, -0.12, 1.705),
        (-0.31, 0.13, 1.720), (0.0, 0.16, 1.850), (0.31, 0.13, 1.720),
    ]
    faces = [(0, 1, 4, 3), (1, 2, 5, 4), (0, 3, 5, 2)]
    mesh = bpy.data.meshes.new("PirateHatMesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    hat = bpy.data.objects.new("PirateHat", mesh)
    bpy.context.collection.objects.link(hat)
    finish_part(hat, material_black, "Head", smooth=False)
    add_curve("HatGoldFront", [(-0.34, -0.165, 1.708), (0.0, -0.185, 1.908), (0.34, -0.165, 1.708)], 0.012, material_gold, "Head")
    add_curve("HatGoldBack", [(-0.31, 0.145, 1.722), (0.0, 0.175, 1.852), (0.31, 0.145, 1.722)], 0.010, material_gold, "Head")


def create_armature():
    armature_data = bpy.data.armatures.new("PlayerForStageSelectRig")
    armature = bpy.data.objects.new("PlayerForStageSelectRig", armature_data)
    bpy.context.collection.objects.link(armature)
    bpy.context.view_layer.objects.active = armature
    armature.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")

    bone_specs = [
        ("Root", (0.0, 0.0, 0.0), (0.0, 0.0, 0.18), None),
        ("Hips", (0.0, 0.0, 0.72), (0.0, 0.0, 0.91), "Root"),
        ("Spine", (0.0, 0.0, 0.91), (0.0, 0.0, 1.16), "Hips"),
        ("Chest", (0.0, 0.0, 1.16), (0.0, 0.0, 1.34), "Spine"),
        ("Neck", (0.0, 0.0, 1.34), (0.0, 0.0, 1.41), "Chest"),
        ("Head", (0.0, 0.0, 1.41), (0.0, 0.0, 1.65), "Neck"),
        ("UpperArm.L", (-0.15, 0.0, 1.27), (-0.33, 0.0, 1.12), "Chest"),
        ("LowerArm.L", (-0.33, 0.0, 1.12), (-0.42, -0.01, 0.91), "UpperArm.L"),
        ("Hand.L", (-0.42, -0.01, 0.91), (-0.43, -0.01, 0.82), "LowerArm.L"),
        ("UpperArm.R", (0.15, 0.0, 1.27), (0.33, 0.0, 1.12), "Chest"),
        ("LowerArm.R", (0.33, 0.0, 1.12), (0.42, -0.01, 0.91), "UpperArm.R"),
        ("Bone_242", (0.42, -0.01, 0.91), (0.43, -0.01, 0.82), "LowerArm.R"),
        ("Thigh.L", (-0.115, 0.0, 0.76), (-0.115, 0.0, 0.47), "Hips"),
        ("Shin.L", (-0.115, 0.0, 0.47), (-0.115, 0.0, 0.16), "Thigh.L"),
        ("Foot.L", (-0.115, 0.0, 0.16), (-0.115, -0.16, 0.08), "Shin.L"),
        ("Thigh.R", (0.115, 0.0, 0.76), (0.115, 0.0, 0.47), "Hips"),
        ("Shin.R", (0.115, 0.0, 0.47), (0.115, 0.0, 0.16), "Thigh.R"),
        ("Foot.R", (0.115, 0.0, 0.16), (0.115, -0.16, 0.08), "Shin.R"),
    ]
    bones = {}
    for name, head, tail, parent_name in bone_specs:
        bone = armature_data.edit_bones.new(name)
        bone.head = head
        bone.tail = tail
        if parent_name:
            bone.parent = bones[parent_name]
        bones[name] = bone

    bpy.ops.object.mode_set(mode="OBJECT")
    armature.show_in_front = True
    return armature


def create_model(armature):
    skin = create_material("Skin", (0.94, 0.63, 0.55), 0.0, 0.8)
    skin_shadow = create_material("SkinShadow", (0.72, 0.39, 0.35), 0.0, 0.8)
    hair = create_material("MarineHair", (0.42, 0.055, 0.13), 0.0, 0.65)
    hair_light = create_material("HairHighlight", (0.68, 0.14, 0.25), 0.0, 0.6)
    red = create_material("CaptainRed", (0.42, 0.035, 0.055), 0.0, 0.62)
    red_light = create_material("SkirtRed", (0.66, 0.10, 0.14), 0.0, 0.7)
    black = create_material("CoatBlack", (0.018, 0.022, 0.045), 0.0, 0.58)
    brown = create_material("StockingBrown", (0.16, 0.085, 0.065), 0.0, 0.72)
    boot = create_material("BootLeather", (0.085, 0.037, 0.026), 0.0, 0.5)
    white = create_material("SailorWhite", (0.86, 0.86, 0.92), 0.0, 0.75)
    gold = create_material("PirateGold", (0.92, 0.55, 0.08), 0.65, 0.3)
    eye = create_material("VisibleEye", (0.92, 0.12, 0.31), 0.1, 0.25)
    teal = create_material("NeckGem", (0.03, 0.72, 0.82), 0.35, 0.18)

    parts = []
    parts.append(add_uv_sphere("Head", (0.0, 0.0, 1.50), (0.235, 0.205, 0.25), skin, "Head", 16, 10))
    parts.append(add_uv_sphere("HairBack", (0.0, 0.055, 1.52), (0.245, 0.19, 0.255), hair, "Head", 14, 9))
    parts.append(add_uv_sphere("FaceFront", (0.0, -0.095, 1.49), (0.205, 0.12, 0.205), skin, "Head", 14, 9))

    for index, x_position in enumerate((-0.16, -0.08, 0.0, 0.08, 0.16)):
        z_position = 1.62
        if index == 2:
            z_position = 1.605
        parts.append(add_cone("Bang_%02d" % index, (x_position, -0.19, z_position), 0.055, 0.095, 0.24, hair_light, "Head", 7, (math.radians(82.0), 0.0, 0.0)))

    parts.append(add_uv_sphere("EyeVisible", (-0.077, -0.218, 1.525), (0.035, 0.016, 0.052), eye, "Head", 10, 6))
    parts.append(add_uv_sphere("EyeHighlight", (-0.087, -0.231, 1.545), (0.010, 0.006, 0.014), white, "Head", 8, 5))
    parts.append(add_uv_sphere("EyePatch", (0.082, -0.222, 1.53), (0.057, 0.018, 0.066), black, "Head", 10, 6))
    parts.append(add_curve("EyePatchStrap", [(-0.18, -0.216, 1.61), (0.08, -0.231, 1.58), (0.20, -0.205, 1.52)], 0.008, black, "Head"))
    parts.append(add_curve("Smile", [(-0.035, -0.223, 1.435), (0.0, -0.23, 1.425), (0.035, -0.223, 1.435)], 0.007, red, "Head"))

    parts.append(add_cone("SideTailLeft", (-0.24, 0.015, 1.34), 0.035, 0.075, 0.43, hair, "Head", 8, (0.0, math.radians(-8.0), math.radians(-4.0))))
    parts.append(add_cone("SideTailRight", (0.24, 0.015, 1.34), 0.035, 0.075, 0.43, hair, "Head", 8, (0.0, math.radians(8.0), math.radians(4.0))))
    parts.append(add_cone("PonytailUpper", (-0.27, 0.09, 1.62), 0.08, 0.12, 0.34, hair_light, "Head", 9, (0.0, math.radians(-55.0), 0.0)))
    parts.append(add_cone("PonytailLower", (-0.40, 0.09, 1.45), 0.035, 0.10, 0.38, hair_light, "Head", 9, (0.0, math.radians(-18.0), 0.0)))
    parts.append(add_box("HairRibbonLeft", (-0.225, -0.015, 1.56), (0.06, 0.025, 0.055), red_light, "Head", (0.0, math.radians(-15.0), math.radians(20.0)), 0.01))
    parts.append(add_box("HairRibbonRight", (0.225, -0.015, 1.56), (0.06, 0.025, 0.055), red_light, "Head", (0.0, math.radians(15.0), math.radians(-20.0)), 0.01))
    add_bicorne(black, gold)

    parts.append(add_uv_sphere("Chest", (0.0, 0.0, 1.17), (0.205, 0.145, 0.20), red, "Chest", 12, 8))
    parts.append(add_cone("Waist", (0.0, 0.0, 0.99), 0.17, 0.145, 0.22, skin_shadow, "Spine", 12))
    parts.append(add_cone("Skirt", (0.0, 0.0, 0.83), 0.18, 0.265, 0.22, red_light, "Hips", 12))
    parts.append(add_box("SailorCollar", (0.0, -0.135, 1.29), (0.15, 0.025, 0.055), white, "Chest", (math.radians(10.0), 0.0, 0.0), 0.012))
    parts.append(add_box("NeckBow", (0.0, -0.17, 1.315), (0.105, 0.025, 0.045), red_light, "Chest", (0.0, 0.0, math.radians(45.0)), 0.008))
    parts.append(add_uv_sphere("NeckGem", (0.0, -0.198, 1.33), (0.032, 0.018, 0.032), teal, "Chest", 10, 6))
    parts.append(add_cylinder("Belt", (0.0, 0.0, 0.925), 0.184, 0.055, boot, "Hips", 12))
    parts.append(add_uv_sphere("BeltBuckle", (0.17, -0.10, 0.93), (0.048, 0.018, 0.055), gold, "Hips", 10, 6))

    for x_position in (-0.16, -0.08, 0.0, 0.08, 0.16):
        parts.append(add_box("WhiteSkirtPleat_%s" % str(x_position), (x_position, -0.21, 0.84), (0.028, 0.012, 0.095), white, "Hips", (math.radians(4.0), 0.0, 0.0)))

    parts.append(add_box("CoatBackLeft", (-0.17, 0.12, 0.94), (0.18, 0.055, 0.40), black, "Chest", (0.0, math.radians(-6.0), math.radians(3.0)), 0.025))
    parts.append(add_box("CoatBackRight", (0.17, 0.12, 0.94), (0.18, 0.055, 0.40), black, "Chest", (0.0, math.radians(6.0), math.radians(-3.0)), 0.025))
    parts.append(add_box("CoatLapelLeft", (-0.215, -0.055, 1.17), (0.075, 0.035, 0.22), red, "Chest", (0.0, math.radians(-8.0), math.radians(-10.0)), 0.015))
    parts.append(add_box("CoatLapelRight", (0.215, -0.055, 1.17), (0.075, 0.035, 0.22), red, "Chest", (0.0, math.radians(8.0), math.radians(10.0)), 0.015))
    parts.append(add_curve("CoatGoldHem", [(-0.35, 0.055, 0.57), (0.0, 0.13, 0.50), (0.35, 0.055, 0.57)], 0.010, gold, "Chest"))

    limb_specs = [
        ("UpperArmLeft", (-0.245, 0.0, 1.19), 0.075, 0.25, skin, "UpperArm.L", (0.0, math.radians(-48.0), 0.0)),
        ("LowerArmLeft", (-0.375, -0.01, 1.015), 0.068, 0.24, black, "LowerArm.L", (0.0, math.radians(-24.0), 0.0)),
        ("UpperArmRight", (0.245, 0.0, 1.19), 0.075, 0.25, skin, "UpperArm.R", (0.0, math.radians(48.0), 0.0)),
        ("LowerArmRight", (0.375, -0.01, 1.015), 0.068, 0.24, black, "LowerArm.R", (0.0, math.radians(24.0), 0.0)),
    ]
    for name, location, radius, depth, material, bone_name, rotation in limb_specs:
        parts.append(add_cylinder(name, location, radius, depth, material, bone_name, 9, rotation))
    parts.append(add_uv_sphere("GloveLeft", (-0.43, -0.01, 0.86), (0.065, 0.055, 0.085), white, "Hand.L", 10, 6))
    parts.append(add_uv_sphere("GloveRight", (0.43, -0.01, 0.86), (0.065, 0.055, 0.085), white, "Bone_242", 10, 6))
    parts.append(add_box("EpauletteLeft", (-0.225, -0.005, 1.29), (0.10, 0.075, 0.035), gold, "Chest", (0.0, 0.0, math.radians(-10.0)), 0.012))
    parts.append(add_box("EpauletteRight", (0.225, -0.005, 1.29), (0.10, 0.075, 0.035), gold, "Chest", (0.0, 0.0, math.radians(10.0)), 0.012))

    for side, x_position, thigh_bone, shin_bone, foot_bone in (
        ("Left", -0.115, "Thigh.L", "Shin.L", "Foot.L"),
        ("Right", 0.115, "Thigh.R", "Shin.R", "Foot.R"),
    ):
        parts.append(add_cylinder("Thigh" + side, (x_position, 0.0, 0.61), 0.092, 0.31, brown, thigh_bone, 10))
        parts.append(add_cylinder("Shin" + side, (x_position, 0.0, 0.31), 0.078, 0.31, brown, shin_bone, 10))
        parts.append(add_box("Boot" + side, (x_position, -0.045, 0.12), (0.095, 0.14, 0.11), boot, foot_bone, (0.0, 0.0, 0.0), 0.025))
        parts.append(add_box("BootCuff" + side, (x_position, 0.0, 0.255), (0.11, 0.085, 0.045), red, shin_bone, (0.0, 0.0, 0.0), 0.012))
        parts.append(add_curve("BootGold" + side, [(x_position - 0.07, -0.14, 0.13), (x_position, -0.155, 0.08), (x_position + 0.07, -0.14, 0.13)], 0.008, gold, foot_bone))

    mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    bpy.ops.object.select_all(action="DESELECT")
    for obj in mesh_objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = mesh_objects[0]
    bpy.ops.object.join()
    model = bpy.context.object
    model.name = "PlayerForStageSelect"
    modifier = model.modifiers.new(name="Armature", type="ARMATURE")
    modifier.object = armature
    model.parent = armature

    for material in model.data.materials:
        material.diffuse_color[3] = 1.0
    return model


def key_pose(pose_bone, frame, rotation=(0.0, 0.0, 0.0), location=(0.0, 0.0, 0.0)):
    pose_bone.rotation_mode = "XYZ"
    pose_bone.rotation_euler = rotation
    pose_bone.location = location
    pose_bone.keyframe_insert(data_path="rotation_euler", frame=frame)
    pose_bone.keyframe_insert(data_path="location", frame=frame)


def create_actions(armature):
    animation_data = armature.animation_data_create()
    idle = bpy.data.actions.new("000")
    idle.slots.new(id_type="OBJECT", name=armature.name)
    animation_data.action = idle
    for frame, bob in ((1, 0.0), (16, 0.012), (31, 0.0)):
        key_pose(armature.pose.bones["Root"], frame, location=(0.0, 0.0, bob))
        key_pose(armature.pose.bones["Chest"], frame, rotation=(0.0, 0.0, math.radians(1.5 if frame == 16 else 0.0)))

    walk = bpy.data.actions.new("walk")
    walk.slots.new(id_type="OBJECT", name=armature.name)
    animation_data.action = walk
    frames = (1, 8, 16, 23, 31)
    phases = (0.0, 1.0, 0.0, -1.0, 0.0)
    for frame, phase in zip(frames, phases):
        bob = 0.018
        if phase == 0.0:
            bob = 0.0
        key_pose(armature.pose.bones["Root"], frame, location=(0.0, 0.0, bob))
        key_pose(armature.pose.bones["Hips"], frame, rotation=(0.0, 0.0, math.radians(-3.0 * phase)))
        key_pose(armature.pose.bones["Chest"], frame, rotation=(0.0, 0.0, math.radians(2.0 * phase)))
        key_pose(armature.pose.bones["Head"], frame, rotation=(0.0, 0.0, math.radians(-1.5 * phase)))
        key_pose(armature.pose.bones["Thigh.L"], frame, rotation=(math.radians(28.0 * phase), 0.0, 0.0))
        key_pose(armature.pose.bones["Thigh.R"], frame, rotation=(math.radians(-28.0 * phase), 0.0, 0.0))
        left_knee = 7.0
        right_knee = 7.0
        if phase > 0.0:
            right_knee = 30.0
        if phase < 0.0:
            left_knee = 30.0
        key_pose(armature.pose.bones["Shin.L"], frame, rotation=(math.radians(left_knee), 0.0, 0.0))
        key_pose(armature.pose.bones["Shin.R"], frame, rotation=(math.radians(right_knee), 0.0, 0.0))
        key_pose(armature.pose.bones["UpperArm.L"], frame, rotation=(math.radians(-18.0 * phase), 0.0, math.radians(-3.0)))
        key_pose(armature.pose.bones["UpperArm.R"], frame, rotation=(math.radians(18.0 * phase), 0.0, math.radians(3.0)))
        key_pose(armature.pose.bones["LowerArm.L"], frame, rotation=(math.radians(-8.0), 0.0, 0.0))
        key_pose(armature.pose.bones["LowerArm.R"], frame, rotation=(math.radians(-8.0), 0.0, 0.0))

    animation_data.action = idle
    return idle, walk


def configure_scene():
    scene = bpy.context.scene
    scene.frame_start = 1
    scene.frame_end = 31
    scene.render.fps = 30
    scene.unit_settings.system = "METRIC"
    scene.unit_settings.scale_length = 1.0
    scene["asset_purpose"] = "Low-poly stage-select-only player model"
    scene["reference_character"] = "Houshou Marine"
    scene["walk_cycle"] = "In-place, 30 fps, frames 1-31"


def export_x(filepath, armature, action, export_animation):
    armature.animation_data.action = action
    bpy.context.scene.frame_set(1)
    result = bpy.ops.export_scene.directx_x(
        filepath=filepath,
        use_selection=False,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=False,
        triangulate=True,
        unweld_on_export=False,
        export_materials=True,
        export_textures=False,
        export_armature=True,
        export_weights=True,
        export_animation=export_animation,
        anim_key_format="TRS",
        pz_compat=False,
        anim_fps=30.0,
        anim_frame_start=1,
        anim_frame_end=31,
        export_format="TEXT_X",
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + filepath)


def add_preview_camera_and_lights():
    bpy.ops.object.camera_add(location=(2.75, -5.3, 2.15))
    camera = bpy.context.object
    camera.name = "PreviewCamera"
    direction = Vector((0.0, 0.0, 1.0)) - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    bpy.context.scene.camera = camera
    camera.data.lens = 70.0

    bpy.ops.object.light_add(type="AREA", location=(-2.5, -3.0, 4.0))
    bpy.context.object.data.energy = 850.0
    bpy.context.object.data.shape = "DISK"
    bpy.context.object.data.size = 4.0
    bpy.ops.object.light_add(type="AREA", location=(2.0, 0.5, 3.0))
    bpy.context.object.data.energy = 550.0
    bpy.context.object.data.size = 3.0
    bpy.ops.object.light_add(type="POINT", location=(0.0, -2.0, 0.6))
    bpy.context.object.data.energy = 180.0


def main():
    clear_scene()
    configure_scene()
    armature = create_armature()
    model = create_model(armature)
    idle, walk = create_actions(armature)

    triangle_count = sum(len(polygon.vertices) - 2 for polygon in model.data.polygons)
    model["triangle_count"] = triangle_count
    model["design_note"] = "Recognizable silhouette and color blocking prioritized for small on-screen display"
    add_preview_camera_and_lights()

    armature.animation_data.action = idle
    bpy.context.scene.frame_set(1)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    export_x(BASE_X_PATH, armature, idle, False)
    export_x(IDLE_X_PATH, armature, idle, True)
    export_x(WALK_X_PATH, armature, walk, True)
    armature.animation_data.action = walk
    bpy.context.scene.frame_set(8)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    print("PLAYER_FOR_STAGE_SELECT_TRIANGLES", triangle_count)
    print("PLAYER_FOR_STAGE_SELECT_BLEND", BLEND_PATH)


if __name__ == "__main__":
    main()
