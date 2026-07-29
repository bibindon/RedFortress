import math
import os
import re
import shutil
import sys

import bpy
from mathutils import Euler, Matrix


AXIS_FORWARD = "Z"
AXIS_UP = "Y"
ANIMATION_START_FRAME = 0
ANIMATION_MIDDLE_FRAME = 30
ANIMATION_END_FRAME = 60
ARMATURE_NAME = "Armature"
BASE_COLOR_NODE_NAME = "Mtoon1BaseColorTexture.Image"
PROTOTYPE_SCALE = 1.0


def parse_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected arguments after --")

    arguments = sys.argv[sys.argv.index("--") + 1 :]
    if len(arguments) != 2:
        raise RuntimeError(
            "Usage: blender --background --python PrepareKanataPrototype.py "
            "-- <imported.blend> <output-directory>"
        )

    blend_path = os.path.abspath(arguments[0])
    output_directory = os.path.abspath(arguments[1])
    if not os.path.isfile(blend_path):
        raise RuntimeError(f"Imported Blender file was not found: {blend_path}")
    if not os.path.isdir(output_directory):
        raise RuntimeError(f"Output directory was not found: {output_directory}")

    return blend_path, output_directory


def require_armature():
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    if len(armatures) != 1:
        raise RuntimeError(f"Expected one armature, found {len(armatures)}")

    armature = armatures[0]
    armature.name = ARMATURE_NAME
    armature.data.name = ARMATURE_NAME
    return armature


def enable_directx_exporter():
    result = bpy.ops.preferences.addon_enable(
        module="bl_ext.user_default.io_directx_x"
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"Failed to enable the DirectX X exporter: {result}")


def require_meshes():
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(meshes) == 0:
        raise RuntimeError("No meshes were found in the imported Blender file")
    return meshes


def sanitize_name(name):
    sanitized = re.sub(r"[^A-Za-z0-9_]", "_", name)
    sanitized = re.sub(r"_+", "_", sanitized).strip("_")
    if len(sanitized) == 0:
        sanitized = "Unnamed"
    if sanitized[0].isdigit():
        sanitized = "N_" + sanitized
    return sanitized


def find_base_color_image(material):
    if material is None or material.node_tree is None:
        return None

    node = material.node_tree.nodes.get(BASE_COLOR_NODE_NAME)
    if node is not None and node.type == "TEX_IMAGE" and node.image is not None:
        return node.image

    for candidate in material.node_tree.nodes:
        if candidate.type == "TEX_IMAGE" and candidate.image is not None:
            return candidate.image
    return None


def copy_material_textures(meshes, output_directory):
    used_materials = []
    seen_material_ids = set()
    for mesh in meshes:
        for slot in mesh.material_slots:
            material = slot.material
            if material is None:
                continue
            material_id = material.as_pointer()
            if material_id in seen_material_ids:
                continue
            seen_material_ids.add(material_id)
            used_materials.append(material)

    texture_directory = os.path.join(output_directory, "textures")
    os.makedirs(texture_directory, exist_ok=True)

    copied_textures = []
    for material_index, material in enumerate(used_materials):
        image = find_base_color_image(material)
        if image is None:
            raise RuntimeError(f"Base-color texture was not found: {material.name}")

        source_path = bpy.path.abspath(image.filepath)
        if not os.path.isfile(source_path):
            raise RuntimeError(
                f"Base-color texture file was not found for {material.name}: "
                f"{source_path}"
            )

        extension = os.path.splitext(source_path)[1].lower()
        if extension != ".png":
            extension = ".png"
        texture_filename = f"kanata_{material_index:02d}{extension}"
        destination_path = os.path.join(texture_directory, texture_filename)
        shutil.copy2(source_path, destination_path)

        material.name = f"KanataMat_{material_index:02d}"
        material["_x_face_color"] = [1.0, 1.0, 1.0, 1.0]
        material["_x_power"] = 24.0
        material["_x_specular"] = [0.15, 0.15, 0.15]
        material["_x_emissive"] = [0.0, 0.0, 0.0]
        material["_x_texture_filename"] = (
            "textures\\" + texture_filename
        )
        image.filepath = destination_path
        copied_textures.append(destination_path)

    if len(copied_textures) == 0:
        raise RuntimeError("No base-color textures were copied")

    return copied_textures


def prepare_object_names(meshes):
    used_names = set()
    for index, mesh in enumerate(meshes):
        base_name = sanitize_name(mesh.name)
        candidate_name = base_name
        suffix = 1
        while candidate_name in used_names:
            candidate_name = f"{base_name}_{suffix:02d}"
            suffix += 1
        used_names.add(candidate_name)
        mesh.name = candidate_name
        mesh.data.name = candidate_name + "_Mesh"

        mesh.scale = (
            mesh.scale.x * PROTOTYPE_SCALE,
            mesh.scale.y * PROTOTYPE_SCALE,
            mesh.scale.z * PROTOTYPE_SCALE,
        )

        for polygon in mesh.data.polygons:
            polygon.use_smooth = True


def require_pose_bone(armature, bone_name):
    pose_bone = armature.pose.bones.get(bone_name)
    if pose_bone is None:
        raise RuntimeError(f"Required VRM humanoid bone was not found: {bone_name}")
    return pose_bone


def set_pose_rotation_value(pose_bone, rotation_x, rotation_y, rotation_z):
    pose_bone.rotation_mode = "QUATERNION"
    pose_bone.rotation_quaternion = Euler(
        (
            math.radians(rotation_x),
            math.radians(rotation_y),
            math.radians(rotation_z),
        ),
        "XYZ",
    ).to_quaternion()


def apply_bake_pose(armature):
    for action in list(bpy.data.actions):
        bpy.data.actions.remove(action)
    armature.animation_data_clear()

    set_pose_rotation_value(
        require_pose_bone(armature, "J_Bip_L_UpperArm"),
        -68.0,
        0.0,
        -4.0,
    )
    set_pose_rotation_value(
        require_pose_bone(armature, "J_Bip_R_UpperArm"),
        -68.0,
        0.0,
        4.0,
    )
    set_pose_rotation_value(
        require_pose_bone(armature, "J_Bip_L_LowerArm"),
        0.0,
        0.0,
        -8.0,
    )
    set_pose_rotation_value(
        require_pose_bone(armature, "J_Bip_R_LowerArm"),
        0.0,
        0.0,
        8.0,
    )
    set_pose_rotation_value(
        require_pose_bone(armature, "J_Bip_C_Chest"),
        0.0,
        0.0,
        0.0,
    )
    set_pose_rotation_value(
        require_pose_bone(armature, "J_Bip_C_Head"),
        0.0,
        0.0,
        0.0,
    )
    bpy.context.view_layer.update()


def bake_deformed_meshes(meshes):
    dependency_graph = bpy.context.evaluated_depsgraph_get()
    baked_meshes = []
    for source_object in meshes:
        evaluated_object = source_object.evaluated_get(dependency_graph)
        baked_mesh_data = bpy.data.meshes.new_from_object(
            evaluated_object,
            preserve_all_data_layers=True,
            depsgraph=dependency_graph,
        )
        source_world_matrix = source_object.matrix_world.copy()
        for vertex in baked_mesh_data.vertices:
            vertex.co = source_world_matrix @ vertex.co

        baked_object = bpy.data.objects.new(source_object.name, baked_mesh_data)
        bpy.context.scene.collection.objects.link(baked_object)
        baked_object.matrix_world = Matrix.Identity(4)
        for polygon in baked_mesh_data.polygons:
            polygon.use_smooth = True
        baked_meshes.append(baked_object)
    return baked_meshes


def combine_baked_meshes(baked_meshes):
    if not baked_meshes:
        raise RuntimeError("No baked meshes were created")

    bpy.ops.object.select_all(action="DESELECT")
    for mesh in baked_meshes:
        mesh.select_set(True)
    bpy.context.view_layer.objects.active = baked_meshes[0]
    result = bpy.ops.object.join()
    if "FINISHED" not in result:
        raise RuntimeError(f"Failed to combine baked meshes: {result}")

    combined_mesh = bpy.context.view_layer.objects.active
    combined_mesh.name = "KanataRigidMesh"
    combined_mesh.data.name = "KanataRigidMesh_Data"
    combined_mesh.matrix_world = Matrix.Identity(4)
    return [combined_mesh]


def create_rigid_root_armature(source_armature, source_meshes):
    apply_bake_pose(source_armature)
    baked_meshes = bake_deformed_meshes(source_meshes)
    baked_meshes = combine_baked_meshes(baked_meshes)

    for source_mesh in source_meshes:
        bpy.data.objects.remove(source_mesh, do_unlink=True)
    bpy.data.objects.remove(source_armature, do_unlink=True)

    armature_data = bpy.data.armatures.new("GameRootArmature")
    armature = bpy.data.objects.new(ARMATURE_NAME, armature_data)
    bpy.context.scene.collection.objects.link(armature)

    bpy.context.view_layer.objects.active = armature
    armature.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    root_bone = armature_data.edit_bones.new("GameRoot")
    root_bone.head = (0.0, 0.0, 0.0)
    root_bone.tail = (0.0, 0.0, 1.0)
    bpy.ops.object.mode_set(mode="OBJECT")

    for mesh in baked_meshes:
        vertex_group = mesh.vertex_groups.new(name="GameRoot")
        vertex_group.add(range(len(mesh.data.vertices)), 1.0, "REPLACE")
        modifier = mesh.modifiers.new(name="GameRootArmature", type="ARMATURE")
        modifier.object = armature

    bpy.context.view_layer.update()
    return armature, baked_meshes


def set_root_location(root_bone, height, frame):
    root_bone.location = (0.0, 0.0, height)
    root_bone.keyframe_insert(data_path="location", frame=frame)


def create_idle_animation(armature):
    for action in list(bpy.data.actions):
        bpy.data.actions.remove(action)

    armature.animation_data_clear()
    armature.animation_data_create()
    action = bpy.data.actions.new("idle")
    action.use_fake_user = True
    armature.animation_data.action = action
    root_bone = require_pose_bone(armature, "GameRoot")

    set_root_location(root_bone, 0.0, ANIMATION_START_FRAME)
    set_root_location(root_bone, 0.015, ANIMATION_MIDDLE_FRAME)
    set_root_location(root_bone, 0.0, ANIMATION_END_FRAME)

    for fcurve in action.fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "SINE"

    bpy.context.scene.render.fps = 30
    bpy.context.scene.frame_start = ANIMATION_START_FRAME
    bpy.context.scene.frame_end = ANIMATION_END_FRAME
    bpy.context.scene.frame_set(ANIMATION_START_FRAME)
    bpy.context.view_layer.update()
    return action

def validate_prepared_pose(armature, meshes):
    bpy.context.scene.frame_set(ANIMATION_MIDDLE_FRAME)
    bpy.context.view_layer.update()

    minimum_z = None
    maximum_z = None
    dependency_graph = bpy.context.evaluated_depsgraph_get()
    for mesh in meshes:
        evaluated_object = mesh.evaluated_get(dependency_graph)
        evaluated_mesh = evaluated_object.to_mesh()
        try:
            for vertex in evaluated_mesh.vertices:
                world_position = evaluated_object.matrix_world @ vertex.co
                if minimum_z is None or world_position.z < minimum_z:
                    minimum_z = world_position.z
                if maximum_z is None or world_position.z > maximum_z:
                    maximum_z = world_position.z
        finally:
            evaluated_object.to_mesh_clear()

    if minimum_z is None or maximum_z is None:
        raise RuntimeError("Prepared model has no evaluated vertices")
    if minimum_z < -0.15 or minimum_z > 0.15:
        raise RuntimeError(
            f"Prepared model feet are not near ground level: minimum_z={minimum_z}"
        )
    model_height = maximum_z - minimum_z
    if model_height < 1.4 or model_height > 1.8:
        raise RuntimeError(f"Unexpected prepared model height: {model_height}")

    armature_rotation = armature.rotation_euler
    if max(abs(value) for value in armature_rotation) > 0.0001:
        raise RuntimeError("Armature object rotation must remain zero before X export")
    if AXIS_FORWARD != "Z" or AXIS_UP != "Y":
        raise RuntimeError("DirectX X export axes must remain forward Z and up Y")


def select_export_objects(armature, meshes):
    bpy.ops.object.select_all(action="DESELECT")
    armature.hide_set(False)
    armature.hide_viewport = False
    armature.hide_render = False
    armature.select_set(True)
    for mesh in meshes:
        mesh.hide_set(False)
        mesh.hide_viewport = False
        mesh.hide_render = False
        mesh.select_set(True)
    bpy.context.view_layer.objects.active = armature


def export_x(path, armature, meshes, export_animation):
    select_export_objects(armature, meshes)
    result = bpy.ops.export_scene.directx_x(
        filepath=path,
        check_existing=False,
        use_selection=True,
        axis_forward=AXIS_FORWARD,
        axis_up=AXIS_UP,
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=True,
        export_weights=True,
        export_animation=export_animation,
        anim_key_format="MATRIX",
        anim_fps=30.0,
        anim_frame_start=ANIMATION_START_FRAME,
        anim_frame_end=ANIMATION_END_FRAME,
        export_format="TEXT_X",
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"DirectX X export failed: {path}")


def validate_exported_mesh_ground_origin(text, path):
    mesh_match = re.search(
        r"^\s*Mesh\s+\w+\s*\{\s*(\d+)\s*;\s*(.*?)\s*;\s*\d+\s*;",
        text,
        re.MULTILINE | re.DOTALL,
    )
    if mesh_match is None:
        raise RuntimeError(f"Exported rigid mesh vertices were not found: {path}")

    vertex_count = int(mesh_match.group(1))
    coordinate_values = [
        float(value)
        for value in re.findall(
            r"[-+]?\d+(?:\.\d*)?(?:[eE][-+]?\d+)?",
            mesh_match.group(2),
        )
    ]
    expected_coordinate_count = vertex_count * 3
    if len(coordinate_values) != expected_coordinate_count:
        raise RuntimeError(
            "Unexpected exported rigid mesh coordinate count: "
            f"expected={expected_coordinate_count} actual={len(coordinate_values)} "
            f"path={path}"
        )

    y_values = coordinate_values[1::3]
    minimum_y = min(y_values)
    maximum_y = max(y_values)
    if abs(minimum_y) > 0.001:
        raise RuntimeError(
            "Exported Kanata mesh must keep its feet at local Y=0: "
            f"minimum_y={minimum_y} path={path}"
        )
    model_height = maximum_y - minimum_y
    if model_height < 1.4 or model_height > 1.8:
        raise RuntimeError(
            f"Unexpected exported Kanata mesh height: {model_height} path={path}"
        )


def validate_x(path, expect_animation):
    with open(path, "r", encoding="utf-8-sig") as exported_file:
        text = exported_file.read()

    if not text.startswith("xof 0303txt 0032"):
        raise RuntimeError(f"Unexpected DirectX X header: {path}")
    if "SkinWeights" not in text:
        raise RuntimeError(f"Skin weights were not exported: {path}")
    skin_weight_bones = set(
        re.findall(r'SkinWeights\s*\{\s*"([^"]+)"', text, re.DOTALL)
    )
    if skin_weight_bones != {"GameRoot"}:
        raise RuntimeError(
            f"Only the rigid GameRoot bone may be exported: {skin_weight_bones}"
        )
    exported_meshes = re.findall(r"^\s*Mesh\s+\w+\s*\{", text, re.MULTILINE)
    if len(exported_meshes) != 1:
        raise RuntimeError(
            f"The rigid prototype must contain exactly one mesh: {len(exported_meshes)}"
        )
    validate_exported_mesh_ground_origin(text, path)
    if "TextureFileName" not in text:
        raise RuntimeError(f"Texture references were not exported: {path}")
    if expect_animation:
        if "AnimationSet" not in text:
            raise RuntimeError(f"AnimationSet was not exported: {path}")
        animation_key_blocks = re.findall(r"AnimationKey\s*\{(.*?)\}", text, re.DOTALL)
        if not animation_key_blocks:
            raise RuntimeError(f"AnimationKey was not exported: {path}")
        for animation_key_block in animation_key_blocks:
            matrix_key_at_frame_zero = re.match(
                r"\s*4;\s*\d+;\s*0;16;",
                animation_key_block,
            )
            if matrix_key_at_frame_zero is None:
                raise RuntimeError(
                    "Animations must use matrix keys beginning at frame 0: "
                    f"{path}"
                )

    with open(path, "rb") as exported_file:
        first_three_bytes = exported_file.read(3)
    if first_three_bytes == b"\xef\xbb\xbf":
        raise RuntimeError(f"DirectX X file must not contain a UTF-8 BOM: {path}")


def write_animation_csv(output_directory):
    rows = [
        'Anim, "000", "enemy.idle.x", default',
        'Anim, "idle", "enemy.idle.x", loop',
        'Anim, "walk", "enemy.idle.x", loop',
        'Anim, "creep", "enemy.idle.x", loop',
        'Anim, "run", "enemy.idle.x", loop',
        'Anim, "attack", "enemy.idle.x", stopWhenEnd',
        'Anim, "hit", "enemy.idle.x", stopWhenEnd',
        'Anim, "death", "enemy.idle.x", stopWhenEnd',
    ]
    csv_path = os.path.join(output_directory, "enemy.csv")
    with open(csv_path, "wb") as csv_file:
        csv_file.write(("\r\n".join(rows) + "\r\n").encode("utf-8"))


def main():
    blend_path, output_directory = parse_arguments()
    bpy.ops.wm.open_mainfile(filepath=blend_path)
    enable_directx_exporter()

    armature = require_armature()
    meshes = require_meshes()
    prepare_object_names(meshes)
    copied_textures = copy_material_textures(meshes, output_directory)
    armature, meshes = create_rigid_root_armature(armature, meshes)
    create_idle_animation(armature)
    validate_prepared_pose(armature, meshes)

    prepared_blend_path = os.path.join(
        output_directory,
        "kanata_prototype.blend",
    )
    save_result = bpy.ops.wm.save_as_mainfile(filepath=prepared_blend_path)
    if "FINISHED" not in save_result:
        raise RuntimeError(f"Failed to save prepared Blender file: {prepared_blend_path}")

    base_x_path = os.path.join(output_directory, "enemy.x")
    export_x(base_x_path, armature, meshes, False)
    validate_x(base_x_path, False)

    idle_x_path = os.path.join(output_directory, "enemy.idle.x")
    export_x(idle_x_path, armature, meshes, True)
    validate_x(idle_x_path, True)
    write_animation_csv(output_directory)

    print(
        "KANATA_PROTOTYPE_PREPARED "
        f"meshes={len(meshes)} "
        f"textures={len(copied_textures)} "
        f"blend={prepared_blend_path} "
        f"base_x={base_x_path} "
        f"idle_x={idle_x_path}"
    )


main()
