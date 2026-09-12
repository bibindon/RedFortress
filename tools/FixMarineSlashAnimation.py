"""Repair Marine's overhead slash in Blender and export with the official add-on."""
import math
from pathlib import Path

import bpy
from mathutils import Quaternion, Vector

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / 'RedFortress2/MultiPassRendering/res/model2/marine_512_low'
SOURCE_ACTION = 'slash_before_arm_fix'


def limit_angle(angle, degrees):
    bound = math.radians(degrees)
    return max(-bound, min(bound, angle))


def limit_swing(rotation, degrees):
    rotation = rotation.normalized()
    if rotation.w < 0.0:
        rotation.negate()
    if rotation.angle > math.radians(degrees):
        return Quaternion(rotation.axis, math.radians(degrees))
    return rotation


def influence(frame):
    amount = min(frame / 6.0, (55.0 - frame) / 10.0, 1.0)
    amount = max(0.0, amount)
    return amount * amount * (3.0 - 2.0 * amount)


def correct_grip(armature):
    """Aim the club along an overhead arc using forearm roll and wrist swing.

    The game attaches the club along wrist-local -X. Targets are in Blender
    armature space: -Y is forward and +Z is up. Keep the original endpoints.
    """
    scene = bpy.context.scene
    targets = [(0, (0, -.7, .7)), (10, (0, .5, .866)),
               (16, (0, .94, .342)), (20, (0, .2, .98)),
               (24, (0, -.866, -.5)), (30, (0, -.8, -.6)),
               (40, (0, -.8, -.6)), (55, (0, -.7, .7))]
    samples = []
    directions = []
    rolls = []
    for frame in range(56):
        scene.frame_set(frame)
        samples.append({bone.name: bone.rotation_quaternion.copy()
                        for bone in armature.pose.bones})
        for index in range(len(targets) - 1):
            start, first = targets[index]
            end, last = targets[index + 1]
            if start <= frame <= end:
                t = (frame - start) / (end - start)
                t = t * t * (3.0 - 2.0 * t)
                desired = Vector(first).lerp(Vector(last), t).normalized()
                break
        directions.append(desired)
        forearm = armature.pose.bones['Bone_241']
        wrist = armature.pose.bones['Bone_242']
        axis = (forearm.matrix.to_3x3() @ Vector((0, 1, 0))).normalized()
        current = (wrist.matrix.to_3x3() @ Vector((-1, 0, 0))).normalized()
        first = current - axis * current.dot(axis)
        last = desired - axis * desired.dot(axis)
        angle = 0.0
        if rolls:
            angle = rolls[-1]
        if first.length > .0001 and last.length > .0001:
            first.normalize()
            last.normalize()
            angle = math.atan2(axis.dot(first.cross(last)), first.dot(last))
        if rolls:
            while angle - rolls[-1] > math.pi:
                angle -= 2.0 * math.pi
            while angle - rolls[-1] < -math.pi:
                angle += 2.0 * math.pi
        rolls.append(angle)

    previous = {}
    for frame, pose in enumerate(samples):
        scene.frame_set(frame)
        for name, rotation in pose.items():
            armature.pose.bones[name].rotation_quaternion = rotation
        amount = max(0.0, min(frame / 10.0, (55 - frame) / 15.0, 1.0))
        amount = amount * amount * (3.0 - 2.0 * amount)
        forearm = armature.pose.bones['Bone_241']
        wrist = armature.pose.bones['Bone_242']
        # Projection becomes unstable when the club aligns with the forearm.
        # Smooth the unwrapped roll to prevent a sudden grip flip there.
        window = rolls[max(0, frame - 4):min(56, frame + 5)]
        angle = sum(window) / len(window)
        forearm.rotation_quaternion = (forearm.rotation_quaternion
                                       @ Quaternion((0, 1, 0), angle * amount))
        bpy.context.view_layer.update()
        current = (wrist.matrix.to_3x3() @ Vector((-1, 0, 0))).normalized()
        correction = Quaternion().slerp(
            current.rotation_difference(directions[frame]), amount)
        matrix = correction.to_matrix().to_4x4() @ wrist.matrix
        matrix.translation = wrist.matrix.translation
        wrist.matrix = matrix
        bpy.context.view_layer.update()
        for bone in (forearm, wrist):
            rotation = bone.rotation_quaternion.copy()
            if bone.name in previous and previous[bone.name].dot(rotation) < 0.0:
                rotation.negate()
            bone.rotation_quaternion = rotation
            previous[bone.name] = rotation.copy()
            bone.keyframe_insert(data_path='rotation_quaternion', frame=frame,
                                 group=bone.name)

def main():
    armature = next(obj for obj in bpy.data.objects if obj.type == 'ARMATURE')
    if SOURCE_ACTION not in bpy.data.actions:
        source = bpy.data.actions['slash'].copy()
        source.name = SOURCE_ACTION
        source.use_fake_user = True
    source = bpy.data.actions[SOURCE_ACTION]
    armature.animation_data.action = None
    for bone in armature.pose.bones:
        bone.location = (0.0, 0.0, 0.0)
        bone.rotation_mode = 'QUATERNION'
        bone.rotation_quaternion = Quaternion()
        bone.scale = (1.0, 1.0, 1.0)
    armature.animation_data.action = source
    armature.animation_data.action_slot = source.slots[0]
    samples = []
    for frame in range(56):
        bpy.context.scene.frame_set(frame)
        samples.append({bone.name: (bone.location.copy(), bone.rotation_quaternion.copy(),
                                    bone.scale.copy()) for bone in armature.pose.bones})

    old = bpy.data.actions.get('slash')
    if old is not None:
        bpy.data.actions.remove(old)
    action = bpy.data.actions.new('slash')
    action.use_fake_user = True
    armature.animation_data.action = action
    previous = {}
    for frame, pose in enumerate(samples):
        amount = influence(frame)
        rotations = {name: values[1].normalized() for name, values in pose.items()}
        upper_swing, upper_twist = rotations['Bone_239'].to_swing_twist('Y')
        elbow_swing, elbow_twist = rotations['Bone_240'].to_swing_twist('Y')
        forearm_swing, forearm_twist = rotations['Bone_241'].to_swing_twist('Y')
        wrist_swing, wrist_twist = rotations['Bone_242'].to_swing_twist('Y')
        limited_wrist_twist = limit_angle(wrist_twist, 15.0)
        corrected = {
            'Bone_239': Quaternion((0, 1, 0), limit_angle(upper_twist, 35.0)),
            'Bone_240': elbow_swing,
            'Bone_241': Quaternion((0, 1, 0), limit_angle(
                forearm_twist + elbow_twist + wrist_twist - limited_wrist_twist, 60.0)),
            'Bone_242': limit_swing(wrist_swing, 25.0)
                        @ Quaternion((0, 1, 0), limited_wrist_twist),
        }
        for name, rotation in corrected.items():
            rotations[name] = rotations[name].slerp(rotation, amount)
        for bone in armature.pose.bones:
            bone.rotation_mode = 'QUATERNION'
            bone.location = pose[bone.name][0]
            rotation = rotations[bone.name]
            if bone.name in previous and previous[bone.name].dot(rotation) < 0.0:
                rotation.negate()
            previous[bone.name] = rotation.copy()
            bone.rotation_quaternion = rotation
            bone.scale = pose[bone.name][2]
            for channel in ('location', 'rotation_quaternion', 'scale'):
                bone.keyframe_insert(data_path=channel, frame=frame, group=bone.name)
    correct_grip(armature)
    for layer in action.layers:
        for strip in layer.strips:
            for bag in strip.channelbags:
                for curve in bag.fcurves:
                    for key in curve.keyframe_points:
                        key.interpolation = 'LINEAR'

    bpy.ops.object.select_all(action='DESELECT')
    armature.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.context.scene.frame_start = 0
    bpy.context.scene.frame_end = 55
    bpy.context.scene.frame_set(0)
    result = bpy.ops.export_scene.directx_x(
        filepath=str(ASSETS / 'marine.slash.x'), check_existing=False,
        use_selection=True, axis_forward='Z', axis_up='Y', global_scale=1.0,
        use_mesh_modifiers=True, export_normals=True, export_uvs=True,
        export_materials=True, export_textures=False, export_armature=True,
        export_weights=True, export_animation=True, anim_key_format='TRS',
        pz_compat=False, anim_fps=30.0, anim_frame_start=0, anim_frame_end=55,
        unweld_on_export=False, use_original_material_data=False,
        export_format='TEXT_X', triangulate=False)
    if 'FINISHED' not in result:
        raise RuntimeError('Official DirectX animation export failed')
    path = ASSETS / 'marine.slash.x'
    # Only normalize text encoding/newlines, never rewrite exported X structures.
    text = path.read_text(encoding='utf-8-sig')
    path.write_bytes(text.replace('\r\n', '\n').replace('\n', '\r\n').encode('utf-8'))
    bpy.context.scene.frame_set(0)
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(ASSETS / 'marine.blend'))
    print('MARINE_SLASH_FIXED: 56 frames; original action retained as ' + SOURCE_ACTION)


if __name__ == '__main__':
    main()