import math
import os

import bpy
from mathutils import Euler


ARMATURE_NAME = "宝鐘マリンV2_arm"
RUN_ACTION_NAME = "run"
RUN_FILE_NAME = "marine.run.x"

FIRST_FRAME = 2
LAST_FRAME = 79

# The adjustments are distributed across the upper body so the character
# leans forward instead of bending sharply at a single joint. The head gets
# an additional downward tilt so the face follows the sprint posture.
POSE_ADJUSTMENTS = {
    "Bone_003": (7.0, 0.0, 0.0),
    "Bone_004": (8.0, 0.0, 0.0),
    "Bone_005": (4.0, 0.0, 0.0),
    "Bone_006": (10.0, 0.0, 0.0),
}


def require_armature(name):
    armature = bpy.data.objects.get(name)
    if armature is None:
        raise RuntimeError("Player armature was not found: " + name)
    if armature.type != "ARMATURE":
        raise RuntimeError("Player armature has an unexpected object type: " + name)
    return armature


def require_bones(armature, bone_names):
    for bone_name in bone_names:
        if armature.pose.bones.get(bone_name) is None:
            raise RuntimeError("Required player bone was not found: " + bone_name)


def remove_existing_run_action(armature):
    if armature.animation_data is None:
        armature.animation_data_create()

    existing_action = bpy.data.actions.get(RUN_ACTION_NAME)
    if existing_action is None:
        return

    if armature.animation_data.action == existing_action:
        armature.animation_data.action = None
    bpy.data.actions.remove(existing_action)


def import_run_animation(input_path, existing_armatures):
    result = bpy.ops.import_scene.directx_x(
        filepath=input_path,
        axis_forward="Z",
        axis_up="Y",
        import_normals=False,
        import_uvs=False,
        import_materials=False,
        import_textures=False,
        import_armature=True,
        import_weights=False,
        import_animation=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X run animation import failed: " + input_path)

    imported_armatures = [
        obj
        for obj in bpy.data.objects
        if obj.type == "ARMATURE" and obj not in existing_armatures
    ]
    if len(imported_armatures) != 1:
        raise RuntimeError(
            "Expected one imported run armature, found "
            + str(len(imported_armatures))
        )

    imported_armature = imported_armatures[0]
    if imported_armature.animation_data is None:
        raise RuntimeError("Imported run armature has no animation data")
    if imported_armature.animation_data.action is None:
        raise RuntimeError("Imported run armature has no action")
    return imported_armature


def validate_hierarchy(target_armature, imported_armature):
    target_names = {bone.name for bone in target_armature.data.bones}
    imported_names = {bone.name for bone in imported_armature.data.bones}
    if target_names != imported_names:
        missing_names = sorted(target_names - imported_names)
        extra_names = sorted(imported_names - target_names)
        raise RuntimeError(
            "Imported run hierarchy does not match the player hierarchy. Missing: "
            + ", ".join(missing_names)
            + "; extra: "
            + ", ".join(extra_names)
        )


def capture_imported_poses(scene, imported_armature):
    poses = {}
    for frame in range(FIRST_FRAME, LAST_FRAME + 1):
        scene.frame_set(frame)
        frame_pose = {}
        for pose_bone in imported_armature.pose.bones:
            frame_pose[pose_bone.name] = pose_bone.matrix_basis.copy()
        poses[frame] = frame_pose
    return poses


def rotate_bone_local(armature, bone_name, rotation_degrees):
    pose_bone = armature.pose.bones[bone_name]
    x_degrees, y_degrees, z_degrees = rotation_degrees
    delta = Euler(
        (
            math.radians(x_degrees),
            math.radians(y_degrees),
            math.radians(z_degrees),
        ),
        "XYZ",
    ).to_quaternion()
    pose_bone.rotation_mode = "QUATERNION"
    pose_bone.rotation_quaternion = pose_bone.rotation_quaternion @ delta


def key_pose(armature, frame):
    for pose_bone in armature.pose.bones:
        pose_bone.keyframe_insert(data_path="location", frame=frame, group=pose_bone.name)
        pose_bone.keyframe_insert(
            data_path="rotation_quaternion",
            frame=frame,
            group=pose_bone.name,
        )
        pose_bone.keyframe_insert(data_path="scale", frame=frame, group=pose_bone.name)


def create_adjusted_action(scene, armature, imported_poses):
    action = bpy.data.actions.new(RUN_ACTION_NAME)
    action.use_fake_user = True
    armature.animation_data.action = action

    for frame in range(FIRST_FRAME, LAST_FRAME + 1):
        scene.frame_set(frame)
        for pose_bone in armature.pose.bones:
            pose_bone.rotation_mode = "QUATERNION"
            pose_bone.matrix_basis = imported_poses[frame][pose_bone.name]
        bpy.context.view_layer.update()

        for bone_name, rotation_degrees in POSE_ADJUSTMENTS.items():
            rotate_bone_local(armature, bone_name, rotation_degrees)
        bpy.context.view_layer.update()
        key_pose(armature, frame)

    return action


def export_action(armature, output_path):
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature

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
        anim_frame_start=FIRST_FRAME,
        anim_frame_end=LAST_FRAME,
        unweld_on_export=False,
        use_original_material_data=False,
        export_format="TEXT_X",
        triangulate=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X run animation export failed: " + output_path)


def main():
    blend_path = bpy.data.filepath
    if os.path.basename(blend_path).lower() != "marine.blend":
        raise RuntimeError("Open marine.blend before running this script")

    armature = require_armature(ARMATURE_NAME)
    require_bones(armature, POSE_ADJUSTMENTS.keys())
    remove_existing_run_action(armature)

    asset_directory = os.path.dirname(blend_path)
    run_path = os.path.join(asset_directory, RUN_FILE_NAME)
    if not os.path.isfile(run_path):
        raise RuntimeError("Run animation file was not found: " + run_path)

    existing_armatures = {
        obj for obj in bpy.data.objects if obj.type == "ARMATURE"
    }
    imported_armature = import_run_animation(run_path, existing_armatures)
    validate_hierarchy(armature, imported_armature)

    scene = bpy.context.scene
    imported_poses = capture_imported_poses(scene, imported_armature)
    imported_action = imported_armature.animation_data.action
    imported_armature.animation_data.action = None
    bpy.data.objects.remove(imported_armature, do_unlink=True)
    bpy.data.actions.remove(imported_action)

    action = create_adjusted_action(scene, armature, imported_poses)
    armature.animation_data.action = action
    scene.render.fps = 30
    scene.render.fps_base = 1.0
    scene.frame_start = FIRST_FRAME
    scene.frame_end = LAST_FRAME
    scene.frame_set(FIRST_FRAME)

    export_action(armature, run_path)

    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    bpy.ops.wm.save_as_mainfile(filepath=blend_path + "1", copy=True)

    print("MARINE_RUN_ACTION run 2 79")
    print("MARINE_RUN_ADJUSTMENT Bone_003 7")
    print("MARINE_RUN_ADJUSTMENT Bone_004 8")
    print("MARINE_RUN_ADJUSTMENT Bone_005 4")
    print("MARINE_RUN_ADJUSTMENT Bone_006 10")
    print("MARINE_EXPORTED marine.run.x")


if __name__ == "__main__":
    main()
