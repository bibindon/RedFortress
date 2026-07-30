import bpy
import math
import re
from mathutils import Quaternion
from pathlib import Path

BLENDER_EXECUTABLE_NOTE = "Run with: blender.exe --background --python BuildPortalModels.py"

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
PORTAL_DIRECTORY = REPOSITORY_ROOT / "RedFortress2" / "MultiPassRendering" / "res" / "model" / "portal"

EXPORT_KWARGS = dict(
    check_existing=False,
    use_selection=True,
    use_mesh_modifiers=True,
    global_scale=1.0,
    axis_forward="-Z",
    axis_up="Y",
    export_normals=True,
    export_uvs=True,
    export_materials=True,
    export_textures=True,
    triangulate=True,
    unweld_on_export=False,
)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for mesh in list(bpy.data.meshes):
        bpy.data.meshes.remove(mesh)
    for material in list(bpy.data.materials):
        bpy.data.materials.remove(material)
    for image in list(bpy.data.images):
        if image.users == 0:
            bpy.data.images.remove(image)
    for armature in list(bpy.data.armatures):
        bpy.data.armatures.remove(armature)
    for action in list(bpy.data.actions):
        bpy.data.actions.remove(action)


def normalize_x_file(path):
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    data = data.replace(b"\n", b"\r\n")
    path.write_bytes(data)


def export_selected_x(x_path, export_armature, export_weights, export_animation):
    result = bpy.ops.export_scene.directx_x(
        filepath=str(x_path),
        export_armature=export_armature,
        export_weights=export_weights,
        export_animation=export_animation,
        **EXPORT_KWARGS,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(x_path))
    normalize_x_file(x_path)


def save_blend(blend_path):
    bpy.data.use_autopack = False
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))


def add_box(name, size_x, size_y, size_z, center_x=0.0, center_y=0.0, center_z=0.0):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(center_x, center_y, center_z))
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = (size_x, size_y, size_z)
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=True)
    return obj


def build_stone_steps_mesh():
    tiers = [
        (2.0, 2.0, 0.5, 0.00),
        (1.6, 1.6, 0.5, 0.50),
        (1.2, 1.2, 0.5, 1.00),
    ]
    boxes = []
    for index, (size_x, size_y, height, base_z) in enumerate(tiers):
        center_z = base_z + height * 0.5
        obj = add_box("stone_tier_{}".format(index), size_x, size_y, height, center_z=center_z)
        boxes.append(obj)
    bpy.ops.object.select_all(action="DESELECT")
    for obj in boxes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = boxes[0]
    bpy.ops.object.join()
    joined = bpy.context.active_object
    joined.name = "stone_steps"
    return joined


def build_stone_texture(path):
    width = 256
    height = 256
    image = bpy.data.images.new("stone_steps", width=width, height=height, alpha=False)
    pixels = []
    import random
    random.seed(1234)
    for _y in range(height):
        for _x in range(width):
            noise = random.random() * 0.18
            base = 0.42 + noise
            pixels.extend([base, base * 0.97, base * 0.90, 1.0])
    image.pixels = pixels
    image.filepath_raw = str(path)
    image.file_format = "PNG"
    image.save()
    return image


def create_textured_material(material_name, texture_filename, image):
    material = bpy.data.materials.new(material_name)
    material.use_nodes = True
    material.diffuse_color = (1.0, 1.0, 1.0, 1.0)
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    principled = None
    for node in nodes:
        if node.bl_idname == "ShaderNodeBsdfPrincipled":
            principled = node
            break
    if principled is None:
        raise RuntimeError("Principled BSDF node was not found")
    image_node = nodes.new("ShaderNodeTexImage")
    image_node.name = material_name
    image_node.image = image
    links.new(image_node.outputs["Color"], principled.inputs["Base Color"])
    principled.inputs["Roughness"].default_value = 0.95
    return material


def build_stone_steps():
    PORTAL_DIRECTORY.mkdir(parents=True, exist_ok=True)
    texture_path = PORTAL_DIRECTORY / "stone_steps.png"
    x_path = PORTAL_DIRECTORY / "stone_steps.x"
    collision_x_path = PORTAL_DIRECTORY / "stone_steps_collision.x"
    blend_path = PORTAL_DIRECTORY / "stone_steps.blend"

    clear_scene()
    image = build_stone_texture(texture_path)
    material = create_textured_material("stone_steps", "stone_steps.png", image)
    steps_obj = build_stone_steps_mesh()
    steps_obj.data.materials.append(material)

    bpy.context.view_layer.objects.active = steps_obj
    steps_obj.select_set(True)
    save_blend(blend_path)
    export_selected_x(x_path, export_armature=False, export_weights=False, export_animation=False)
    export_selected_x(collision_x_path, export_armature=False, export_weights=False, export_animation=False)

    print("STONE_STEPS_BLEND {}".format(blend_path))
    print("STONE_STEPS_X {}".format(x_path))
    print("STONE_STEPS_COLLISION_X {}".format(collision_x_path))
    print("STONE_STEPS_TEXTURE {}".format(texture_path))


def build_light_pillar_mesh(radius=0.4, height=20.0, segments=24):
    # Cylinder with base on Z=0 (Blender up axis). Origin stays at world origin.
    bpy.ops.mesh.primitive_cylinder_add(
        radius=radius,
        depth=height,
        vertices=segments,
        location=(0.0, 0.0, height * 0.5),
    )
    obj = bpy.context.active_object
    obj.name = "light_pillar"
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=True)
    return obj


def build_light_pillar_texture(path):
    width = 64
    height = 256
    image = bpy.data.images.new("light_pillar", width=width, height=height, alpha=True)
    pixels = []
    for y in range(height):
        # Vertical gradient: bright white at the bottom fading to soft blue at the top.
        t = y / (height - 1)
        r = 0.85 - 0.25 * t
        g = 0.92 - 0.20 * t
        b = 1.0
        a = 0.75 - 0.45 * t
        for _x in range(width):
            pixels.extend([r, g, b, a])
    image.pixels = pixels
    image.filepath_raw = str(path)
    image.file_format = "PNG"
    image.save()
    return image


def create_emissive_textured_material(material_name, image, emission_strength=2.0):
    material = bpy.data.materials.new(material_name)
    material.use_nodes = True
    material.blend_method = "BLEND"
    material.diffuse_color = (1.0, 1.0, 1.0, 1.0)
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    principled = None
    for node in nodes:
        if node.bl_idname == "ShaderNodeBsdfPrincipled":
            principled = node
            break
    if principled is None:
        raise RuntimeError("Principled BSDF node was not found")
    image_node = nodes.new("ShaderNodeTexImage")
    image_node.name = material_name
    image_node.image = image
    links.new(image_node.outputs["Color"], principled.inputs["Base Color"])
    links.new(image_node.outputs["Alpha"], principled.inputs["Alpha"])
    # Emission so the pillar looks self-lit even without strong lighting.
    if "Emission Color" in principled.inputs:
        links.new(image_node.outputs["Color"], principled.inputs["Emission Color"])
        principled.inputs["Emission Color"].default_value = (0.8, 0.9, 1.0, 1.0)
    if "Emission Strength" in principled.inputs:
        principled.inputs["Emission Strength"].default_value = emission_strength
    principled.inputs["Roughness"].default_value = 0.3
    return material


def write_light_pillar_csv(csv_path):
    # Same Emit format as res/model/Buster/buster.csv
    lines = [
        "meshtype,Emit",
        "EmitIntensity,0.5",
        "EmitColor,200,230,255",
    ]
    csv_path.write_text("\r\n".join(lines) + "\r\n", encoding="ascii")


def build_light_pillar():
    PORTAL_DIRECTORY.mkdir(parents=True, exist_ok=True)
    texture_path = PORTAL_DIRECTORY / "light_pillar.png"
    x_path = PORTAL_DIRECTORY / "light_pillar.x"
    csv_path = PORTAL_DIRECTORY / "light_pillar.csv"
    blend_path = PORTAL_DIRECTORY / "light_pillar.blend"

    clear_scene()
    image = build_light_pillar_texture(texture_path)
    material = create_emissive_textured_material("light_pillar", image)
    pillar_obj = build_light_pillar_mesh()
    pillar_obj.data.materials.append(material)

    bpy.context.view_layer.objects.active = pillar_obj
    pillar_obj.select_set(True)
    save_blend(blend_path)
    export_selected_x(x_path, export_armature=False, export_weights=False, export_animation=False)
    write_light_pillar_csv(csv_path)

    print("LIGHT_PILLAR_BLEND {}".format(blend_path))
    print("LIGHT_PILLAR_X {}".format(x_path))
    print("LIGHT_PILLAR_TEXTURE {}".format(texture_path))
    print("LIGHT_PILLAR_CSV {}".format(csv_path))


FLAG_BONE_NAMES = ["cloth_0", "cloth_1", "cloth_2", "cloth_3"]
FLAG_BONE_HEAD_X = [0.0, 0.3, 0.6, 0.9]
FLAG_BONE_TAIL_X = [0.3, 0.6, 0.9, 1.2]
FLAG_BONE_Z = 1.5
FLAG_WAVE_AMPLITUDE = [0.0, 0.10, 0.22, 0.35]
FLAG_WAVE_PHASE = [0.0, 0.9, 1.8, 2.7]
FLAG_WAVE_FRAMES = 60


def build_flag_texture(path):
    width = 64
    height = 64
    image = bpy.data.images.new("black_flag", width=width, height=height, alpha=False)
    import random
    random.seed(5678)
    pixels = []
    for _y in range(height):
        for x in range(width):
            noise = random.random() * 0.03
            if x < width // 2:
                # Cloth: near-black with a faint blue tint.
                pixels.extend([0.02 + noise, 0.02 + noise, 0.04 + noise, 1.0])
            else:
                # Pole: dark grey metal.
                base = 0.16 + noise
                pixels.extend([base, base, base * 1.1, 1.0])
    image.pixels = pixels
    image.filepath_raw = str(path)
    image.file_format = "PNG"
    image.save()
    return image


def shrink_uvs_to_u_range(obj, u_min, u_max):
    mesh = obj.data
    uv_layer = mesh.uv_layers.active
    if uv_layer is None:
        raise RuntimeError("UV layer was not found on " + obj.name)
    u_center = (u_min + u_max) * 0.5
    u_half = (u_max - u_min) * 0.5
    for loop_uv in uv_layer.data:
        u = loop_uv.uv.x
        loop_uv.uv = (u_center + (u - 0.5) * 2.0 * u_half, loop_uv.uv.y)


def build_flag_pole():
    bpy.ops.mesh.primitive_cylinder_add(
        radius=0.035,
        depth=2.0,
        vertices=12,
        location=(0.0, 0.0, 1.0),
    )
    pole = bpy.context.active_object
    pole.name = "flag_pole"
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=True)
    shrink_uvs_to_u_range(pole, 0.55, 0.95)
    return pole


def build_flag_cloth():
    # Grid is created on the XY plane, then rotated to stand in the XZ plane.
    bpy.ops.mesh.primitive_grid_add(
        x_subdivisions=9,
        y_subdivisions=5,
        size=1.0,
        location=(0.0, 0.0, 0.0),
        rotation=(math.radians(90.0), 0.0, 0.0),
    )
    cloth = bpy.context.active_object
    cloth.name = "flag_cloth"
    # Width 1.2 m along X starting at the pole surface, height 0.8 m centered at Z=1.5.
    cloth.scale = (1.2, 1.0, 0.8)
    cloth.location = (0.635, 0.0, FLAG_BONE_Z)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    shrink_uvs_to_u_range(cloth, 0.05, 0.45)

    # Make the cloth double-sided so it is visible from behind.
    solidify = cloth.modifiers.new("solidify", "SOLIDIFY")
    solidify.thickness = 0.004
    solidify.offset = 0.0
    bpy.context.view_layer.objects.active = cloth
    bpy.ops.object.modifier_apply(modifier=solidify.name)
    return cloth


def assign_cloth_weights(mesh_obj):
    groups = {}
    for bone_name in FLAG_BONE_NAMES:
        groups[bone_name] = mesh_obj.vertex_groups.new(name=bone_name)
    bone_centers = []
    for i in range(len(FLAG_BONE_NAMES)):
        bone_centers.append((FLAG_BONE_HEAD_X[i] + FLAG_BONE_TAIL_X[i]) * 0.5)
    segment = 0.3
    for vertex in mesh_obj.data.vertices:
        x = vertex.co.x
        if x <= 0.02:
            # Pole vertices stay on the root bone and never move.
            groups[FLAG_BONE_NAMES[0]].add([vertex.index], 1.0, "REPLACE")
            continue
        # Linear blend between the two nearest bone centers.
        index = int((x - bone_centers[0]) / segment + 0.5)
        if index < 0:
            index = 0
        if index > len(bone_centers) - 2:
            index = len(bone_centers) - 2
        left = bone_centers[index]
        right = bone_centers[index + 1]
        t = (x - left) / (right - left)
        if t < 0.0:
            t = 0.0
        if t > 1.0:
            t = 1.0
        groups[FLAG_BONE_NAMES[index]].add([vertex.index], 1.0 - t, "REPLACE")
        groups[FLAG_BONE_NAMES[index + 1]].add([vertex.index], t, "REPLACE")


def build_flag_armature():
    armature_data = bpy.data.armatures.new("black_flag_arm")
    armature_obj = bpy.data.objects.new("black_flag_arm", armature_data)
    bpy.context.scene.collection.objects.link(armature_obj)
    bpy.context.view_layer.objects.active = armature_obj
    bpy.ops.object.mode_set(mode="EDIT")
    edit_bones = armature_data.edit_bones
    bones = []
    for i, bone_name in enumerate(FLAG_BONE_NAMES):
        bone = edit_bones.new(bone_name)
        bone.head = (FLAG_BONE_HEAD_X[i], 0.0, FLAG_BONE_Z)
        bone.tail = (FLAG_BONE_TAIL_X[i], 0.0, FLAG_BONE_Z)
        if i > 0:
            bone.parent = bones[i - 1]
            bone.use_connect = True
        bones.append(bone)
    bpy.ops.object.mode_set(mode="OBJECT")
    return armature_obj


def build_wave_action(armature_obj):
    armature_obj.animation_data_create()
    action = bpy.data.actions.new("wave")
    armature_obj.animation_data.action = action
    pose_bones = armature_obj.pose.bones
    # The DirectX X exporter only samples rotation_quaternion f-curves, so
    # the pose bones must stay in quaternion mode and key rotation_quaternion.
    for bone_name in FLAG_BONE_NAMES:
        pose_bones[bone_name].rotation_mode = "QUATERNION"
    for frame in range(0, FLAG_WAVE_FRAMES + 1, 3):
        for i, bone_name in enumerate(FLAG_BONE_NAMES):
            pose_bone = pose_bones[bone_name]
            angle = FLAG_WAVE_AMPLITUDE[i] * math.sin(
                2.0 * math.pi * frame / FLAG_WAVE_FRAMES - FLAG_WAVE_PHASE[i]
            )
            pose_bone.rotation_quaternion = Quaternion((0.0, 0.0, 1.0), angle)
            pose_bone.keyframe_insert("rotation_quaternion", frame=frame)
    # Blender 5.1 slotted actions: the slot must be assigned explicitly or
    # the exporter cannot see the f-curves.
    if len(action.slots) > 0:
        armature_obj.animation_data.action_slot = action.slots[0]
    # Keep the scene at 30 fps so exported tick numbers match frame numbers
    # (the exporter converts keys to anim_fps ticks using the scene fps).
    bpy.context.scene.render.fps = 30
    bpy.context.scene.frame_start = 0
    bpy.context.scene.frame_end = FLAG_WAVE_FRAMES
    bpy.context.scene.frame_set(0)
    return action


def export_flag_x(x_path, armature_obj, mesh_obj, export_animation):
    bpy.ops.object.select_all(action="DESELECT")
    armature_obj.select_set(True)
    mesh_obj.select_set(True)
    bpy.context.view_layer.objects.active = armature_obj
    arguments = dict(
        filepath=str(x_path),
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        export_animation=export_animation,
    )
    if export_animation:
        arguments["anim_fps"] = 30.0
        arguments["anim_frame_start"] = 0
        arguments["anim_frame_end"] = FLAG_WAVE_FRAMES
    result = bpy.ops.export_scene.directx_x(**arguments)
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(x_path))
    normalize_x_file(x_path)


def validate_flag_x(x_path, expect_animation):
    text = x_path.read_text(encoding="utf-8")
    if not text.startswith("xof 0303txt"):
        raise RuntimeError("Unexpected X header: " + str(x_path))
    if "TextureFileName {\"black_flag.png\";}" not in text:
        raise RuntimeError("black_flag.png texture reference missing: " + str(x_path))
    if re.search(r"\bXSkinMeshHeader\b", text) is None:
        raise RuntimeError("XSkinMeshHeader missing: " + str(x_path))
    if re.search(r"\bSkinWeights\b", text) is None:
        raise RuntimeError("SkinWeights missing: " + str(x_path))
    has_animation = re.search(r"\bAnimationSet\b", text) is not None
    if expect_animation and not has_animation:
        raise RuntimeError("AnimationSet missing: " + str(x_path))
    if expect_animation and re.search(r"\bAnimationKey\b", text) is None:
        raise RuntimeError("AnimationKey missing: " + str(x_path))


def write_black_flag_csv(csv_path):
    lines = [
        'Anim, "wave", "black_flag.wave.x", loop',
    ]
    csv_path.write_text("\r\n".join(lines) + "\r\n", encoding="ascii")


def build_black_flag():
    PORTAL_DIRECTORY.mkdir(parents=True, exist_ok=True)
    texture_path = PORTAL_DIRECTORY / "black_flag.png"
    x_path = PORTAL_DIRECTORY / "black_flag.x"
    wave_x_path = PORTAL_DIRECTORY / "black_flag.wave.x"
    csv_path = PORTAL_DIRECTORY / "black_flag.csv"
    blend_path = PORTAL_DIRECTORY / "black_flag.blend"

    clear_scene()
    image = build_flag_texture(texture_path)
    material = create_textured_material("black_flag", "black_flag.png", image)

    pole_obj = build_flag_pole()
    cloth_obj = build_flag_cloth()
    pole_obj.data.materials.append(material)
    cloth_obj.data.materials.append(material)

    # Join pole and cloth into a single skinned mesh.
    bpy.ops.object.select_all(action="DESELECT")
    pole_obj.select_set(True)
    cloth_obj.select_set(True)
    bpy.context.view_layer.objects.active = cloth_obj
    bpy.ops.object.join()
    flag_mesh = bpy.context.active_object
    flag_mesh.name = "black_flag"

    assign_cloth_weights(flag_mesh)

    armature_obj = build_flag_armature()
    flag_mesh.parent = armature_obj
    armature_modifier = flag_mesh.modifiers.new("Armature", "ARMATURE")
    armature_modifier.object = armature_obj

    build_wave_action(armature_obj)

    save_blend(blend_path)

    # Base file: rest pose, no animation.
    armature_obj.animation_data.action = None
    export_flag_x(x_path, armature_obj, flag_mesh, export_animation=False)

    # Animation file: the wave action. Re-assigning the action clears the
    # action slot, so set it again explicitly.
    action = bpy.data.actions["wave"]
    armature_obj.animation_data.action = action
    if len(action.slots) > 0:
        armature_obj.animation_data.action_slot = action.slots[0]
    bpy.context.scene.frame_set(0)
    export_flag_x(wave_x_path, armature_obj, flag_mesh, export_animation=True)

    validate_flag_x(x_path, expect_animation=False)
    validate_flag_x(wave_x_path, expect_animation=True)
    write_black_flag_csv(csv_path)

    print("BLACK_FLAG_BLEND {}".format(blend_path))
    print("BLACK_FLAG_X {}".format(x_path))
    print("BLACK_FLAG_WAVE_X {}".format(wave_x_path))
    print("BLACK_FLAG_TEXTURE {}".format(texture_path))
    print("BLACK_FLAG_CSV {}".format(csv_path))


if __name__ == "__main__":
    build_black_flag()
