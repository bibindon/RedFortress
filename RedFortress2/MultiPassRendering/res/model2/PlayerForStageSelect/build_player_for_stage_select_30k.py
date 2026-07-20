import math
import os
import sys

import bpy
from mathutils import Vector


SCRIPT_DIRECTORY = os.path.dirname(os.path.abspath(__file__))
sys.dont_write_bytecode = True
if SCRIPT_DIRECTORY not in sys.path:
    sys.path.insert(0, SCRIPT_DIRECTORY)

import build_player_for_stage_select as base


BLEND_PATH = os.path.join(SCRIPT_DIRECTORY, "PlayerForStageSelect_30K.blend")
BASE_X_PATH = os.path.join(SCRIPT_DIRECTORY, "PlayerForStageSelect_30K.x")
IDLE_X_PATH = os.path.join(SCRIPT_DIRECTORY, "PlayerForStageSelect_30K.000.x")
WALK_X_PATH = os.path.join(SCRIPT_DIRECTORY, "PlayerForStageSelect_30K.walk.x")
PREVIEW_PATH = os.path.join(SCRIPT_DIRECTORY, "PlayerForStageSelect_30K_preview.png")


def add_sphere(name, location, scale, material, bone_name, segments=32, rings=20, rotation=None):
    obj = base.add_uv_sphere(name, location, scale, material, bone_name, segments, rings)
    if rotation is not None:
        obj.rotation_euler = rotation
        bpy.context.view_layer.objects.active = obj
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
    return obj


def add_cylinder(name, location, radius, depth, material, bone_name, vertices=24, rotation=(0.0, 0.0, 0.0)):
    return base.add_cylinder(name, location, radius, depth, material, bone_name, vertices, rotation)


def add_cone(name, location, radius_top, radius_bottom, depth, material, bone_name, vertices=24, rotation=(0.0, 0.0, 0.0)):
    return base.add_cone(name, location, radius_top, radius_bottom, depth, material, bone_name, vertices, rotation)


def add_rounded_box(name, location, scale, material, bone_name, rotation=(0.0, 0.0, 0.0), bevel=0.02, segments=3):
    bpy.ops.mesh.primitive_cube_add(location=location, rotation=rotation)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    modifier = obj.modifiers.new(name="RoundedEdges", type="BEVEL")
    modifier.width = bevel
    modifier.segments = segments
    bpy.context.view_layer.objects.active = obj
    bpy.ops.object.modifier_apply(modifier=modifier.name)
    return base.finish_part(obj, material, bone_name, smooth=True)


def add_smooth_curve(name, points, bevel_depth, material, bone_name, cyclic=False, bevel_resolution=2):
    curve = bpy.data.curves.new(name=name + "Curve", type="CURVE")
    curve.dimensions = "3D"
    curve.resolution_u = 2
    curve.bevel_depth = bevel_depth
    curve.bevel_resolution = bevel_resolution
    curve.resolution_u = 2
    spline = curve.splines.new("BEZIER")
    spline.bezier_points.add(len(points) - 1)
    for point, coordinate in zip(spline.bezier_points, points):
        point.co = coordinate
        point.handle_left_type = "AUTO"
        point.handle_right_type = "AUTO"
    spline.use_cyclic_u = cyclic
    obj = bpy.data.objects.new(name, curve)
    bpy.context.collection.objects.link(obj)
    bpy.context.view_layer.objects.active = obj
    obj.select_set(True)
    bpy.ops.object.convert(target="MESH")
    return base.finish_part(obj, material, bone_name)


def create_materials():
    return {
        "skin": base.create_material("30K_Skin", (0.95, 0.66, 0.58), 0.0, 0.74),
        "skin_shadow": base.create_material("30K_SkinShadow", (0.73, 0.42, 0.37), 0.0, 0.76),
        "hair": base.create_material("30K_MarineHair", (0.34, 0.035, 0.095), 0.0, 0.52),
        "hair_light": base.create_material("30K_HairHighlight", (0.66, 0.09, 0.20), 0.0, 0.48),
        "red": base.create_material("30K_CaptainRed", (0.39, 0.025, 0.045), 0.0, 0.52),
        "red_light": base.create_material("30K_SkirtRed", (0.63, 0.07, 0.11), 0.0, 0.6),
        "black": base.create_material("30K_CoatBlack", (0.012, 0.016, 0.032), 0.0, 0.44),
        "brown": base.create_material("30K_StockingBrown", (0.135, 0.066, 0.052), 0.0, 0.62),
        "boot": base.create_material("30K_BootLeather", (0.067, 0.026, 0.018), 0.0, 0.42),
        "white": base.create_material("30K_SailorWhite", (0.90, 0.90, 0.96), 0.0, 0.68),
        "gold": base.create_material("30K_PirateGold", (0.94, 0.58, 0.09), 0.72, 0.24),
        "eye": base.create_material("30K_VisibleEye", (0.92, 0.06, 0.28), 0.1, 0.2),
        "eye_dark": base.create_material("30K_EyeDark", (0.09, 0.012, 0.035), 0.0, 0.25),
        "teal": base.create_material("30K_NeckGem", (0.02, 0.66, 0.84), 0.42, 0.16),
    }


def create_hat(materials):
    black = materials["black"]
    gold = materials["gold"]
    red = materials["red"]

    add_sphere("HatCrown", (0.0, 0.035, 1.765), (0.285, 0.205, 0.17), black, "Head", 28, 16)
    add_sphere("HatRedBack", (0.0, 0.145, 1.76), (0.255, 0.07, 0.145), red, "Head", 24, 14)

    front_points = []
    back_points = []
    for index in range(17):
        fraction = float(index) / 16.0
        x = -0.36 + fraction * 0.72
        arch = 1.0 - abs(fraction * 2.0 - 1.0)
        front_points.append((x, -0.185, 1.715 + arch * 0.205))
        back_points.append((x, 0.165, 1.72 + arch * 0.14))

    vertices = front_points + back_points
    faces = []
    for index in range(16):
        faces.append((index, index + 1, 17 + index, 16 + index))
    mesh = bpy.data.meshes.new("BicorneBrimMesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    brim = bpy.data.objects.new("BicorneBrim", mesh)
    bpy.context.collection.objects.link(brim)
    base.finish_part(brim, black, "Head", smooth=True)
    solidify = brim.modifiers.new(name="BrimThickness", type="SOLIDIFY")
    solidify.thickness = 0.025
    solidify.offset = 0.0
    bpy.context.view_layer.objects.active = brim
    bpy.ops.object.modifier_apply(modifier=solidify.name)

    add_smooth_curve("HatGoldFront", front_points, 0.012, gold, "Head", False, 3)
    add_smooth_curve("HatGoldBack", back_points, 0.010, gold, "Head", False, 2)
    add_smooth_curve("HatGoldCrown", [(-0.25, -0.15, 1.77), (0.0, -0.19, 1.91), (0.25, -0.15, 1.77)], 0.009, gold, "Head", False, 2)
    add_rounded_box("HatFeatherPin", (0.235, -0.185, 1.80), (0.025, 0.012, 0.065), gold, "Head", (0.0, math.radians(18.0), math.radians(-25.0)), 0.008, 3)


def create_face_and_hair(materials):
    skin = materials["skin"]
    hair = materials["hair"]
    hair_light = materials["hair_light"]
    black = materials["black"]
    white = materials["white"]
    eye = materials["eye"]
    eye_dark = materials["eye_dark"]
    red = materials["red"]

    add_sphere("HairBack", (0.0, 0.055, 1.51), (0.255, 0.21, 0.275), hair, "Head", 34, 22)
    add_sphere("Face", (0.0, -0.075, 1.495), (0.225, 0.155, 0.235), skin, "Head", 34, 22)

    bang_data = [
        (-0.18, 1.62, -17.0), (-0.12, 1.635, -11.0), (-0.06, 1.625, -5.0),
        (0.0, 1.61, 0.0), (0.06, 1.625, 5.0), (0.12, 1.635, 11.0), (0.18, 1.62, 17.0),
    ]
    for index, (x, z, angle) in enumerate(bang_data):
        add_sphere("Bang_%02d" % index, (x, -0.19, z), (0.055, 0.038, 0.14), hair_light, "Head", 18, 12, (0.0, math.radians(angle), 0.0))

    for side_name, x, material in (("Left", -0.083, eye), ("Right", 0.083, black)):
        add_sphere("EyeWhite" + side_name, (x, -0.218, 1.525), (0.055, 0.018, 0.067), white, "Head", 20, 12)
        if side_name == "Left":
            add_sphere("Iris" + side_name, (x, -0.238, 1.522), (0.033, 0.010, 0.047), material, "Head", 20, 12)
            add_sphere("Pupil" + side_name, (x, -0.247, 1.522), (0.014, 0.006, 0.025), eye_dark, "Head", 16, 10)
            add_sphere("EyeHighlight" + side_name, (x - 0.011, -0.252, 1.542), (0.008, 0.004, 0.012), white, "Head", 12, 8)
        else:
            add_sphere("EyePatch", (x, -0.239, 1.525), (0.062, 0.012, 0.072), black, "Head", 20, 12)

    add_smooth_curve("EyePatchStrap", [(-0.19, -0.225, 1.61), (0.08, -0.253, 1.59), (0.205, -0.218, 1.53)], 0.007, black, "Head", False, 2)
    add_smooth_curve("LeftBrow", [(-0.14, -0.234, 1.60), (-0.08, -0.245, 1.61), (-0.025, -0.235, 1.60)], 0.006, hair, "Head", False, 2)
    add_smooth_curve("Smile", [(-0.045, -0.235, 1.435), (0.0, -0.246, 1.423), (0.045, -0.235, 1.435)], 0.006, red, "Head", False, 2)

    for side_name, x, angle in (("Left", -0.245, -6.0), ("Right", 0.245, 6.0)):
        add_sphere("SideHairUpper" + side_name, (x, 0.005, 1.43), (0.06, 0.055, 0.20), hair, "Head", 20, 12, (0.0, math.radians(angle), 0.0))
        add_sphere("SideHairLower" + side_name, (x, 0.012, 1.20), (0.045, 0.045, 0.17), hair_light, "Head", 18, 12, (0.0, math.radians(angle * 1.8), 0.0))

    ponytail_parts = [
        ((-0.27, 0.10, 1.61), (0.10, 0.09, 0.15), -40.0),
        ((-0.39, 0.11, 1.53), (0.105, 0.085, 0.17), -52.0),
        ((-0.46, 0.105, 1.38), (0.09, 0.075, 0.18), -20.0),
        ((-0.46, 0.095, 1.22), (0.07, 0.06, 0.15), 8.0),
    ]
    for index, (location, scale, angle) in enumerate(ponytail_parts):
        material = hair
        if index >= 2:
            material = hair_light
        add_sphere("Ponytail_%02d" % index, location, scale, material, "Head", 22, 14, (0.0, math.radians(angle), 0.0))

    add_rounded_box("RibbonLeft", (-0.235, -0.01, 1.56), (0.07, 0.03, 0.06), red, "Head", (0.0, math.radians(-15.0), math.radians(22.0)), 0.014, 3)
    add_rounded_box("RibbonRight", (0.235, -0.01, 1.56), (0.07, 0.03, 0.06), red, "Head", (0.0, math.radians(15.0), math.radians(-22.0)), 0.014, 3)


def create_body_and_costume(materials):
    skin = materials["skin"]
    skin_shadow = materials["skin_shadow"]
    red = materials["red"]
    red_light = materials["red_light"]
    black = materials["black"]
    brown = materials["brown"]
    boot = materials["boot"]
    white = materials["white"]
    gold = materials["gold"]
    teal = materials["teal"]

    add_sphere("Bust", (0.0, -0.015, 1.185), (0.215, 0.16, 0.20), red, "Chest", 34, 22)
    add_cone("Midriff", (0.0, 0.0, 1.00), 0.17, 0.145, 0.25, skin_shadow, "Spine", 28)
    add_cone("Skirt", (0.0, 0.0, 0.83), 0.18, 0.275, 0.23, red_light, "Hips", 32)
    add_cylinder("Belt", (0.0, 0.0, 0.94), 0.185, 0.052, boot, "Hips", 32)

    add_rounded_box("CollarLeft", (-0.083, -0.154, 1.285), (0.085, 0.026, 0.065), white, "Chest", (math.radians(8.0), math.radians(-7.0), math.radians(-8.0)), 0.012, 3)
    add_rounded_box("CollarRight", (0.083, -0.154, 1.285), (0.085, 0.026, 0.065), white, "Chest", (math.radians(8.0), math.radians(7.0), math.radians(8.0)), 0.012, 3)
    add_rounded_box("NeckBowLeft", (-0.055, -0.19, 1.315), (0.075, 0.025, 0.038), red_light, "Chest", (0.0, math.radians(-5.0), math.radians(24.0)), 0.012, 3)
    add_rounded_box("NeckBowRight", (0.055, -0.19, 1.315), (0.075, 0.025, 0.038), red_light, "Chest", (0.0, math.radians(5.0), math.radians(-24.0)), 0.012, 3)
    add_sphere("NeckGem", (0.0, -0.212, 1.33), (0.035, 0.018, 0.035), teal, "Chest", 24, 16)
    add_sphere("BeltBuckle", (0.17, -0.105, 0.94), (0.052, 0.018, 0.06), gold, "Hips", 24, 16)
    add_smooth_curve("VestGoldTrim", [(-0.18, -0.15, 1.23), (0.0, -0.18, 1.06), (0.18, -0.15, 1.23)], 0.007, gold, "Chest", False, 2)

    for index in range(7):
        x = -0.18 + index * 0.06
        material = red
        if index % 2 == 1:
            material = white
        add_rounded_box("SkirtPleat_%02d" % index, (x, -0.225, 0.835), (0.032, 0.014, 0.10), material, "Hips", (math.radians(3.0), 0.0, 0.0), 0.007, 2)

    add_rounded_box("CoatBackLeft", (-0.19, 0.13, 0.93), (0.19, 0.065, 0.42), black, "Chest", (0.0, math.radians(-7.0), math.radians(4.0)), 0.035, 4)
    add_rounded_box("CoatBackRight", (0.19, 0.13, 0.93), (0.19, 0.065, 0.42), black, "Chest", (0.0, math.radians(7.0), math.radians(-4.0)), 0.035, 4)
    add_rounded_box("CoatLiningLeft", (-0.195, 0.058, 0.92), (0.17, 0.018, 0.37), red, "Chest", (0.0, math.radians(-7.0), math.radians(4.0)), 0.018, 3)
    add_rounded_box("CoatLiningRight", (0.195, 0.058, 0.92), (0.17, 0.018, 0.37), red, "Chest", (0.0, math.radians(7.0), math.radians(-4.0)), 0.018, 3)
    add_smooth_curve("CoatGoldHem", [(-0.37, 0.055, 0.58), (0.0, 0.14, 0.49), (0.37, 0.055, 0.58)], 0.011, gold, "Chest", False, 3)

    arm_specs = [
        ("UpperArmLeft", (-0.245, 0.0, 1.19), (0.078, 0.078, 0.15), skin, "UpperArm.L", -48.0),
        ("LowerArmLeft", (-0.375, -0.005, 1.02), (0.075, 0.075, 0.145), black, "LowerArm.L", -24.0),
        ("UpperArmRight", (0.245, 0.0, 1.19), (0.078, 0.078, 0.15), skin, "UpperArm.R", 48.0),
        ("LowerArmRight", (0.375, -0.005, 1.02), (0.075, 0.075, 0.145), black, "LowerArm.R", 24.0),
    ]
    for name, location, scale, material, bone_name, angle in arm_specs:
        add_sphere(name, location, scale, material, bone_name, 22, 14, (0.0, math.radians(angle), 0.0))

    add_sphere("GloveLeft", (-0.43, -0.01, 0.86), (0.068, 0.057, 0.087), white, "Hand.L", 22, 14)
    add_sphere("GloveRight", (0.43, -0.01, 0.86), (0.068, 0.057, 0.087), white, "Bone_242", 22, 14)
    add_rounded_box("EpauletteLeft", (-0.23, -0.005, 1.30), (0.105, 0.078, 0.038), gold, "Chest", (0.0, 0.0, math.radians(-10.0)), 0.015, 3)
    add_rounded_box("EpauletteRight", (0.23, -0.005, 1.30), (0.105, 0.078, 0.038), gold, "Chest", (0.0, 0.0, math.radians(10.0)), 0.015, 3)
    for side, x, direction in (("Left", -0.23, -1.0), ("Right", 0.23, 1.0)):
        for fringe_index in range(5):
            fringe_x = x + direction * (fringe_index - 2) * 0.018
            add_cylinder("EpauletteFringe%s_%02d" % (side, fringe_index), (fringe_x, -0.035, 1.245), 0.007, 0.075, gold, "Chest", 12)

    for side, x, thigh_bone, shin_bone, foot_bone in (
        ("Left", -0.115, "Thigh.L", "Shin.L", "Foot.L"),
        ("Right", 0.115, "Thigh.R", "Shin.R", "Foot.R"),
    ):
        add_sphere("Thigh" + side, (x, 0.0, 0.61), (0.10, 0.09, 0.18), brown, thigh_bone, 24, 16)
        add_sphere("Shin" + side, (x, 0.0, 0.31), (0.083, 0.075, 0.18), brown, shin_bone, 24, 16)
        add_rounded_box("Boot" + side, (x, -0.05, 0.12), (0.10, 0.145, 0.115), boot, foot_bone, bevel=0.03, segments=4)
        add_rounded_box("BootCuff" + side, (x, 0.0, 0.255), (0.115, 0.09, 0.048), red, shin_bone, bevel=0.018, segments=3)
        add_smooth_curve("BootLaces" + side, [(x - 0.045, -0.195, 0.17), (x, -0.205, 0.12), (x + 0.045, -0.195, 0.17)], 0.006, gold, foot_bone, False, 2)


def join_model(armature):
    mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    bpy.ops.object.select_all(action="DESELECT")
    for obj in mesh_objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = mesh_objects[0]
    bpy.ops.object.join()
    model = bpy.context.object
    model.name = "PlayerForStageSelect_30K"
    modifier = model.modifiers.new(name="Armature", type="ARMATURE")
    modifier.object = armature
    model.parent = armature
    return model


def add_preview_scene():
    bpy.ops.object.camera_add(location=(2.7, -5.4, 2.25))
    camera = bpy.context.object
    camera.name = "PreviewCamera30K"
    target = Vector((0.0, 0.0, 1.0))
    camera.rotation_euler = (target - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera.data.lens = 72.0
    bpy.context.scene.camera = camera

    for location, energy, size in (((-2.5, -3.0, 4.0), 950.0, 4.0), ((2.5, -0.5, 3.0), 650.0, 3.0)):
        bpy.ops.object.light_add(type="AREA", location=location)
        bpy.context.object.data.energy = energy
        bpy.context.object.data.shape = "DISK"
        bpy.context.object.data.size = size
    bpy.ops.object.light_add(type="POINT", location=(0.0, -2.0, 0.7))
    bpy.context.object.data.energy = 220.0


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


def render_preview():
    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 800
    scene.render.resolution_y = 800
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = PREVIEW_PATH
    scene.world.color = (0.025, 0.035, 0.06)
    bpy.ops.render.render(write_still=True)


def main():
    base.clear_scene()
    base.configure_scene()
    scene = bpy.context.scene
    scene["asset_purpose"] = "Detailed stage-select player model, approximately 30K triangles"
    scene["generation"] = "Built from an empty Blender scene"

    armature = base.create_armature()
    materials = create_materials()
    create_face_and_hair(materials)
    create_hat(materials)
    create_body_and_costume(materials)
    model = join_model(armature)
    idle, walk = base.create_actions(armature)

    triangle_count = sum(len(polygon.vertices) - 2 for polygon in model.data.polygons)
    model["triangle_count"] = triangle_count
    model["target_triangle_count"] = 30000
    add_preview_scene()

    armature.animation_data.action = idle
    bpy.context.scene.frame_set(1)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    export_x(BASE_X_PATH, armature, idle, False)
    export_x(IDLE_X_PATH, armature, idle, True)
    export_x(WALK_X_PATH, armature, walk, True)
    armature.animation_data.action = walk
    bpy.context.scene.frame_set(8)
    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    render_preview()
    print("PLAYER_FOR_STAGE_SELECT_30K_TRIANGLES", triangle_count)
    print("PLAYER_FOR_STAGE_SELECT_30K_BLEND", BLEND_PATH)


if __name__ == "__main__":
    main()
