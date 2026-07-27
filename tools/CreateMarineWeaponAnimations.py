import math
import os

import bpy
from mathutils import Euler, Matrix


ARMATURE_NAME = "宝鐘マリンV2_arm"
MESH_NAME = "宝鐘マリンV2_mesh_decimate50"
BASE_ACTION_NAME = "slash"

SHOOT_START_ACTION_NAME = "shoot_start"
SHOOT_RECOIL_ACTION_NAME = "shoot_recoil"
SHOOT_AIM_ACTION_NAME = "shoot_aim"
SHOOT_END_ACTION_NAME = "shoot_end"
PLACE_BOMB_ACTION_NAME = "place_bomb"

SHOOT_START_END_FRAME = 6
SHOOT_RECOIL_END_FRAME = 6
SHOOT_AIM_END_FRAME = 1
SHOOT_END_END_FRAME = 8
PLACE_BOMB_END_FRAME = 24


def capture_pose(armature):
    pose = {}
    for pose_bone in armature.pose.bones:
        pose[pose_bone.name] = (
            pose_bone.location.copy(),
            pose_bone.rotation_quaternion.copy(),
            pose_bone.scale.copy(),
        )
    return pose


def restore_pose(armature, pose):
    for pose_bone in armature.pose.bones:
        location, rotation, scale = pose[pose_bone.name]
        pose_bone.rotation_mode = "QUATERNION"
        pose_bone.location = location
        pose_bone.rotation_quaternion = rotation
        pose_bone.scale = scale
    bpy.context.view_layer.update()


def rotate_bone_local(armature, bone_name, x_degrees, y_degrees, z_degrees):
    pose_bone = armature.pose.bones[bone_name]
    delta = Euler(
        (
            math.radians(x_degrees),
            math.radians(y_degrees),
            math.radians(z_degrees),
        ),
        "XYZ",
    ).to_quaternion()
    pose_bone.rotation_quaternion = pose_bone.rotation_quaternion @ delta
    bpy.context.view_layer.update()


def point_bone_at(armature, bone_name, target):
    pose_bone = armature.pose.bones[bone_name]
    current_direction = (pose_bone.tail - pose_bone.head).normalized()
    target_direction = (target - pose_bone.head).normalized()
    rotation_delta = current_direction.rotation_difference(target_direction)
    current_matrix = pose_bone.matrix.copy()
    desired_rotation = rotation_delta @ current_matrix.to_quaternion()
    desired_matrix = Matrix.Translation(pose_bone.head) @ desired_rotation.to_matrix().to_4x4()
    pose_bone.matrix = desired_matrix
    bpy.context.view_layer.update()


def key_pose(armature, frame):
    for pose_bone in armature.pose.bones:
        pose_bone.keyframe_insert(data_path="location", frame=frame, group=pose_bone.name)
        pose_bone.keyframe_insert(
            data_path="rotation_quaternion",
            frame=frame,
            group=pose_bone.name,
        )
        pose_bone.keyframe_insert(data_path="scale", frame=frame, group=pose_bone.name)


def create_action(armature, action_name):
    old_action = bpy.data.actions.get(action_name)
    if old_action is not None:
        bpy.data.actions.remove(old_action)

    action = bpy.data.actions.new(action_name)
    action.use_fake_user = True
    armature.animation_data.action = action
    return action


def apply_shoot_pose(armature, recoil):
    rotate_bone_local(armature, "Bone_003", -4.0 * recoil, 0.0, 0.0)
    rotate_bone_local(armature, "Bone_004", -3.0 * recoil, 0.0, 0.0)

    right_elbow = armature.pose.bones["Bone_238"].head.copy()
    right_elbow.x -= 0.10
    right_elbow.y -= 0.24 - 0.04 * recoil
    right_elbow.z -= 0.05
    point_bone_at(armature, "Bone_238", right_elbow)

    gun_hand = armature.pose.bones["Bone_240"].head.copy()
    gun_hand.x += 0.03
    gun_hand.y -= 0.32 - 0.06 * recoil
    gun_hand.z += 0.02
    point_bone_at(armature, "Bone_240", gun_hand)
    rotate_bone_local(armature, "Bone_242", 0.0, -8.0, -4.0)

    left_elbow = armature.pose.bones["Bone_153"].head.copy()
    left_elbow.x += 0.11
    left_elbow.y -= 0.20 - 0.03 * recoil
    left_elbow.z -= 0.08
    point_bone_at(armature, "Bone_153", left_elbow)

    support_hand = armature.pose.bones["Bone_155"].head.copy()
    support_hand.x -= 0.06
    support_hand.y -= 0.25 - 0.05 * recoil
    support_hand.z += 0.08
    point_bone_at(armature, "Bone_155", support_hand)
    rotate_bone_local(armature, "Bone_157", 0.0, 8.0, 5.0)


def create_shoot_start_action(armature, base_pose):
    action = create_action(armature, SHOOT_START_ACTION_NAME)

    restore_pose(armature, base_pose)
    key_pose(armature, 0)

    restore_pose(armature, base_pose)
    apply_shoot_pose(armature, 0.0)
    key_pose(armature, 2)

    restore_pose(armature, base_pose)
    apply_shoot_pose(armature, 1.0)
    key_pose(armature, 3)

    restore_pose(armature, base_pose)
    apply_shoot_pose(armature, 0.0)
    key_pose(armature, SHOOT_START_END_FRAME)
    return action


def create_shoot_recoil_action(armature, base_pose):
    action = create_action(armature, SHOOT_RECOIL_ACTION_NAME)

    restore_pose(armature, base_pose)
    apply_shoot_pose(armature, 0.0)
    key_pose(armature, 0)

    restore_pose(armature, base_pose)
    apply_shoot_pose(armature, 1.0)
    key_pose(armature, 2)

    restore_pose(armature, base_pose)
    apply_shoot_pose(armature, 0.0)
    key_pose(armature, SHOOT_RECOIL_END_FRAME)
    return action


def create_shoot_aim_action(armature, base_pose):
    action = create_action(armature, SHOOT_AIM_ACTION_NAME)

    restore_pose(armature, base_pose)
    apply_shoot_pose(armature, 0.0)
    key_pose(armature, 0)
    key_pose(armature, SHOOT_AIM_END_FRAME)
    return action


def create_shoot_end_action(armature, base_pose):
    action = create_action(armature, SHOOT_END_ACTION_NAME)

    restore_pose(armature, base_pose)
    apply_shoot_pose(armature, 0.0)
    key_pose(armature, 0)

    restore_pose(armature, base_pose)
    key_pose(armature, SHOOT_END_END_FRAME)
    return action


def apply_place_bomb_pose(armature, amount):
    rotate_bone_local(armature, "Bone_003", 18.0 * amount, 0.0, 0.0)
    rotate_bone_local(armature, "Bone_004", 22.0 * amount, 0.0, 0.0)
    rotate_bone_local(armature, "Bone_005", -10.0 * amount, 0.0, 0.0)

    rotate_bone_local(armature, "Bone_445", 18.0 * amount, 0.0, 0.0)
    rotate_bone_local(armature, "Bone_446", 28.0 * amount, 0.0, 0.0)
    rotate_bone_local(armature, "Bone_457", -18.0 * amount, 0.0, 0.0)
    rotate_bone_local(armature, "Bone_458", -28.0 * amount, 0.0, 0.0)

    right_elbow = armature.pose.bones["Bone_238"].head.copy()
    right_elbow.x -= 0.10
    right_elbow.y -= 0.14
    right_elbow.z -= 0.20 * amount
    point_bone_at(armature, "Bone_238", right_elbow)

    bomb_hand = armature.pose.bones["Bone_240"].head.copy()
    bomb_hand.x += 0.06
    bomb_hand.y -= 0.23
    bomb_hand.z -= 0.23 * amount
    point_bone_at(armature, "Bone_240", bomb_hand)
    rotate_bone_local(armature, "Bone_242", -30.0 * amount, 0.0, 0.0)

    left_elbow = armature.pose.bones["Bone_153"].head.copy()
    left_elbow.x += 0.08
    left_elbow.y -= 0.10
    left_elbow.z -= 0.12 * amount
    point_bone_at(armature, "Bone_153", left_elbow)

    balance_hand = armature.pose.bones["Bone_155"].head.copy()
    balance_hand.x += 0.03
    balance_hand.y -= 0.18
    balance_hand.z -= 0.10 * amount
    point_bone_at(armature, "Bone_155", balance_hand)


def create_place_bomb_action(armature, base_pose):
    action = create_action(armature, PLACE_BOMB_ACTION_NAME)

    restore_pose(armature, base_pose)
    key_pose(armature, 0)

    restore_pose(armature, base_pose)
    apply_place_bomb_pose(armature, 0.55)
    key_pose(armature, 6)

    restore_pose(armature, base_pose)
    apply_place_bomb_pose(armature, 1.0)
    key_pose(armature, 12)

    restore_pose(armature, base_pose)
    apply_place_bomb_pose(armature, 0.65)
    key_pose(armature, 18)

    restore_pose(armature, base_pose)
    key_pose(armature, PLACE_BOMB_END_FRAME)
    return action


def export_action(armature, action, output_path, end_frame):
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature
    armature.animation_data.action = action

    scene = bpy.context.scene
    scene.frame_start = 0
    scene.frame_end = end_frame
    scene.frame_set(0)

    result = bpy.ops.export_scene.directx_x(
        filepath=output_path,
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        use_mesh_modifiers=True,
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=False,
        export_armature=True,
        export_weights=True,
        export_animation=True,
        anim_key_format="TRS",
        pz_compat=False,
        anim_fps=30.0,
        anim_frame_start=0,
        anim_frame_end=end_frame,
        unweld_on_export=False,
        use_original_material_data=False,
        export_format="TEXT_X",
        triangulate=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + output_path)


def main():
    blend_path = bpy.data.filepath
    if os.path.basename(blend_path).lower() != "marine.blend":
        raise RuntimeError("Open marine.blend before running this script")

    armature = bpy.data.objects.get(ARMATURE_NAME)
    mesh = bpy.data.objects.get(MESH_NAME)
    if armature is None or armature.type != "ARMATURE":
        raise RuntimeError("Player armature was not found")
    if mesh is None or mesh.type != "MESH":
        raise RuntimeError("Player mesh was not found")

    if armature.animation_data is None:
        armature.animation_data_create()

    base_action = bpy.data.actions.get(BASE_ACTION_NAME)
    if base_action is None:
        raise RuntimeError("Base action was not found: " + BASE_ACTION_NAME)

    armature.animation_data.action = base_action
    bpy.context.scene.frame_set(0)
    base_pose = capture_pose(armature)

    legacy_shoot_action = bpy.data.actions.get("shoot")
    if legacy_shoot_action is not None:
        bpy.data.actions.remove(legacy_shoot_action)

    shoot_start_action = create_shoot_start_action(armature, base_pose)
    shoot_recoil_action = create_shoot_recoil_action(armature, base_pose)
    shoot_aim_action = create_shoot_aim_action(armature, base_pose)
    shoot_end_action = create_shoot_end_action(armature, base_pose)
    place_bomb_action = create_place_bomb_action(armature, base_pose)

    asset_directory = os.path.dirname(blend_path)
    export_action(
        armature,
        shoot_start_action,
        os.path.join(asset_directory, "marine.shoot_start.x"),
        SHOOT_START_END_FRAME,
    )
    export_action(
        armature,
        shoot_recoil_action,
        os.path.join(asset_directory, "marine.shoot_recoil.x"),
        SHOOT_RECOIL_END_FRAME,
    )
    export_action(
        armature,
        shoot_aim_action,
        os.path.join(asset_directory, "marine.shoot_aim.x"),
        SHOOT_AIM_END_FRAME,
    )
    export_action(
        armature,
        shoot_end_action,
        os.path.join(asset_directory, "marine.shoot_end.x"),
        SHOOT_END_END_FRAME,
    )
    export_action(
        armature,
        place_bomb_action,
        os.path.join(asset_directory, "marine.place_bomb.x"),
        PLACE_BOMB_END_FRAME,
    )

    armature.animation_data.action = shoot_start_action
    bpy.context.scene.frame_start = 0
    bpy.context.scene.frame_end = SHOOT_START_END_FRAME
    bpy.context.scene.frame_set(0)
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    bpy.ops.wm.save_as_mainfile(filepath=blend_path + "1", copy=True)

    print("MARINE_ACTION shoot_start 0 6")
    print("MARINE_ACTION shoot_recoil 0 6")
    print("MARINE_ACTION shoot_aim 0 1")
    print("MARINE_ACTION shoot_end 0 8")
    print("MARINE_ACTION place_bomb 0 24")
    print("MARINE_EXPORTED marine.shoot_start.x")
    print("MARINE_EXPORTED marine.shoot_recoil.x")
    print("MARINE_EXPORTED marine.shoot_aim.x")
    print("MARINE_EXPORTED marine.shoot_end.x")
    print("MARINE_EXPORTED marine.place_bomb.x")


if __name__ == "__main__":
    main()
