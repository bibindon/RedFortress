import bpy
import math
import os


ARMATURE_NAME = "Wolf_Skeleton"
ACTION_EXPORTS = (
    ("000", "wolfAnim.000.x"),
    ("Wolf_Idle_", "wolfAnim.idle.x"),
    ("Wolf_Run_Cycle_", "wolfAnim.run.x"),
    ("Wolf_creep_cycle", "wolfAnim.creep.x"),
    ("Wolf_Walk_cycle_", "wolfAnim.walk.x"),
    ("Wolf_seat_", "wolfAnim.seat.x"),
)


def get_channel_bags(action):
    bags = []
    for layer in action.layers:
        for strip in layer.strips:
            for channel_bag in strip.channelbags:
                bags.append(channel_bag)
    if len(bags) != 1:
        raise RuntimeError(
            f"Action {action.name} must contain exactly one channel bag; "
            f"found {len(bags)}"
        )
    return bags


def collect_rotation_samples(armature, actions):
    all_samples = {}
    original_frame = bpy.context.scene.frame_current

    for action in actions:
        armature.animation_data.action = action
        channel_bag = get_channel_bags(action)[0]
        frames_by_bone = {}

        for curve in channel_bag.fcurves:
            suffix = ".rotation_euler"
            if not curve.data_path.endswith(suffix):
                continue
            prefix = 'pose.bones["'
            if not curve.data_path.startswith(prefix):
                continue
            bone_name = curve.data_path[len(prefix):-len('"].rotation_euler')]
            frames = frames_by_bone.setdefault(bone_name, set())
            for point in curve.keyframe_points:
                frames.add(float(point.co[0]))

        action_samples = {}
        frames_to_bones = {}
        for bone_name, frames in frames_by_bone.items():
            for frame in frames:
                bone_names = frames_to_bones.setdefault(frame, [])
                bone_names.append(bone_name)

        previous_by_bone = {}
        for frame in sorted(frames_to_bones):
            bpy.context.scene.frame_set(int(frame), subframe=frame - int(frame))
            for bone_name in frames_to_bones[frame]:
                bone = armature.pose.bones.get(bone_name)
                if bone is None:
                    raise RuntimeError(
                        f"Action {action.name} references missing bone {bone_name}"
                    )

                rotation = bone.matrix_basis.to_quaternion().normalized()
                previous = previous_by_bone.get(bone_name)
                if previous is not None and previous.dot(rotation) < 0.0:
                    rotation.negate()
                previous_by_bone[bone_name] = rotation.copy()

                bone_samples = action_samples.setdefault(bone_name, [])
                bone_samples.append((frame, rotation.copy()))

        all_samples[action.name] = action_samples

    bpy.context.scene.frame_set(original_frame)
    return all_samples


def replace_euler_curves(actions, all_samples):
    for action in actions:
        channel_bag = get_channel_bags(action)[0]
        euler_curves = [
            curve
            for curve in channel_bag.fcurves
            if curve.data_path.endswith(".rotation_euler")
        ]
        for curve in euler_curves:
            channel_bag.fcurves.remove(curve)

        action_samples = all_samples[action.name]
        for bone_name, samples in action_samples.items():
            data_path = f'pose.bones["{bone_name}"].rotation_quaternion'
            for component in range(4):
                curve = channel_bag.fcurves.new(
                    data_path,
                    index=component,
                    group_name=bone_name,
                )
                for frame, rotation in samples:
                    point = curve.keyframe_points.insert(
                        frame,
                        rotation[component],
                        options={"FAST"},
                    )
                    point.interpolation = "LINEAR"
                curve.update()


def validate_baked_rotations(armature, actions, all_samples):
    maximum_angle = 0.0
    original_frame = bpy.context.scene.frame_current

    for action in actions:
        armature.animation_data.action = action
        for bone_name, samples in all_samples[action.name].items():
            bone = armature.pose.bones[bone_name]
            for frame, expected in samples:
                bpy.context.scene.frame_set(int(frame), subframe=frame - int(frame))
                actual = bone.matrix_basis.to_quaternion().normalized()
                dot = abs(expected.dot(actual))
                if dot > 1.0:
                    dot = 1.0
                angle = 2.0 * math.acos(dot)
                if angle > maximum_angle:
                    maximum_angle = angle

    bpy.context.scene.frame_set(original_frame)
    if maximum_angle > 0.002:
        raise RuntimeError(
            f"Quaternion bake changed a keyed pose by {maximum_angle} radians"
        )
    print(f"WOLF_BAKE_MAX_ROTATION_ERROR {maximum_angle:.12f}")


def normalize_x_file(path):
    with open(path, "rb") as source:
        data = source.read()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    with open(path, "wb") as destination:
        destination.write(data)


def export_actions(armature, action_by_name, asset_directory):
    for scene_object in bpy.context.scene.objects:
        scene_object.select_set(False)
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature

    for action_name, filename in ACTION_EXPORTS:
        action = action_by_name[action_name]
        armature.animation_data.action = action
        frame_start = int(action.frame_range[0])
        frame_end = int(action.frame_range[1])
        bpy.context.scene.frame_start = frame_start
        bpy.context.scene.frame_end = frame_end
        bpy.context.scene.frame_set(frame_start)

        output_path = os.path.join(asset_directory, filename)
        result = bpy.ops.export_scene.directx_x(
            filepath=output_path,
            check_existing=False,
            use_selection=True,
            export_animation=True,
            anim_fps=30.0,
            anim_frame_start=frame_start,
            anim_frame_end=frame_end,
        )
        if "FINISHED" not in result:
            raise RuntimeError(f"DirectX X export failed: {output_path}")
        normalize_x_file(output_path)
        print(f"WOLF_EXPORTED {filename} {frame_start} {frame_end}")


def main():
    blend_path = bpy.data.filepath
    if os.path.basename(blend_path).lower() != "wolf.blend":
        raise RuntimeError("Open wolf.blend before running this script")

    armature = bpy.data.objects.get(ARMATURE_NAME)
    if armature is None or armature.type != "ARMATURE":
        raise RuntimeError(f"Armature {ARMATURE_NAME} was not found")
    if armature.animation_data is None:
        raise RuntimeError("Wolf armature has no animation data")

    action_by_name = {}
    for action_name, unused_filename in ACTION_EXPORTS:
        action = bpy.data.actions.get(action_name)
        if action is None:
            raise RuntimeError(f"Action {action_name} was not found")
        action_by_name[action_name] = action

    actions = list(bpy.data.actions)
    all_samples = collect_rotation_samples(armature, actions)
    replace_euler_curves(actions, all_samples)

    for bone in armature.pose.bones:
        bone.rotation_mode = "QUATERNION"

    validate_baked_rotations(armature, actions, all_samples)
    bpy.data.use_autopack = False
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    export_actions(armature, action_by_name, os.path.dirname(blend_path))


main()
