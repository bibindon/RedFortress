import math
import os
import sys

import bpy
from mathutils import Euler


TRANSFORM_EPSILON = 0.0001


def parse_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected arguments after --")

    arguments = sys.argv[sys.argv.index("--") + 1 :]
    if len(arguments) != 3:
        raise RuntimeError(
            "Usage: blender --background --python PrepareRavenSource.py "
            "-- <source.blend> <output.blend> <output.png>"
        )

    return (
        os.path.abspath(arguments[0]),
        os.path.abspath(arguments[1]),
        os.path.abspath(arguments[2]),
    )


def select_only(scene_object):
    for collection in scene_object.users_collection:
        collection.hide_viewport = False
        collection.hide_render = False
    scene_object.hide_viewport = False
    scene_object.hide_set(False)
    bpy.context.view_layer.objects.active = scene_object
    if scene_object.mode != "OBJECT":
        bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="DESELECT")
    scene_object.select_set(True)


def orient_raven(armature, mesh_object):
    armature.rotation_euler.z += math.pi
    select_only(armature)
    result = bpy.ops.object.transform_apply(
        location=False,
        rotation=True,
        scale=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("Failed to apply Raven orientation")

    select_only(mesh_object)
    result = bpy.ops.object.transform_apply(
        location=False,
        rotation=True,
        scale=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("Failed to apply Raven mesh orientation")
    bpy.context.view_layer.update()


def get_channel_bag(action):
    if len(action.layers) != 1:
        raise RuntimeError(f"Unexpected action layer count: {action.name}")

    layer = action.layers[0]
    if len(layer.strips) != 1:
        raise RuntimeError(f"Unexpected action strip count: {action.name}")

    strip = layer.strips[0]
    if len(strip.channelbags) != 1:
        raise RuntimeError(f"Unexpected action channel bag count: {action.name}")

    return strip.channelbags[0]


def replace_curve_keys(action, data_path, keyframes):
    channel_bag = get_channel_bag(action)
    curves = {}
    for curve in channel_bag.fcurves:
        if curve.data_path == data_path:
            curves[curve.array_index] = curve

    value_count = len(keyframes[0][1])
    for value_index in range(value_count):
        curve = curves.get(value_index)
        if curve is None:
            curve = channel_bag.fcurves.new(
                data_path,
                index=value_index,
            )

        curve.keyframe_points.clear()
        for frame, values in keyframes:
            key = curve.keyframe_points.insert(frame, values[value_index])
            key.interpolation = "LINEAR"


def create_hit_action(armature, stand_action):
    armature.animation_data.action = stand_action
    bpy.context.scene.frame_set(0)
    bpy.context.view_layer.update()
    root_bone = armature.pose.bones["ROOT"]
    base_rotation = root_bone.rotation_quaternion.copy()

    hit_action = stand_action.copy()
    hit_action.name = "raven_hit"
    hit_action.use_fake_user = True
    hit_rotation = Euler((math.radians(22.0), 0.0, 0.0)).to_quaternion()
    keyframes = [
        (0.0, tuple(base_rotation)),
        (4.0, tuple(hit_rotation @ base_rotation)),
        (8.0, tuple(base_rotation)),
    ]
    replace_curve_keys(
        hit_action,
        'pose.bones["ROOT"].rotation_quaternion',
        keyframes,
    )
    return hit_action


def create_death_action(armature, stand_action):
    armature.animation_data.action = stand_action
    bpy.context.scene.frame_set(0)
    bpy.context.view_layer.update()
    root_bone = armature.pose.bones["ROOT"]
    base_rotation = root_bone.rotation_quaternion.copy()
    base_location = root_bone.location.copy()

    death_action = stand_action.copy()
    death_action.name = "raven_death"
    death_action.use_fake_user = True
    middle_rotation = Euler(
        (math.radians(55.0), 0.0, math.radians(100.0))
    ).to_quaternion()
    end_rotation = Euler(
        (math.radians(100.0), 0.0, math.radians(210.0))
    ).to_quaternion()
    rotation_keyframes = [
        (0.0, tuple(base_rotation)),
        (20.0, tuple(middle_rotation @ base_rotation)),
        (40.0, tuple(end_rotation @ base_rotation)),
    ]
    replace_curve_keys(
        death_action,
        'pose.bones["ROOT"].rotation_quaternion',
        rotation_keyframes,
    )

    middle_location = base_location.copy()
    middle_location.z -= 0.4
    end_location = base_location.copy()
    end_location.z -= 1.4
    location_keyframes = [
        (0.0, tuple(base_location)),
        (20.0, tuple(middle_location)),
        (40.0, tuple(end_location)),
    ]
    replace_curve_keys(
        death_action,
        'pose.bones["ROOT"].location',
        location_keyframes,
    )
    return death_action


def save_texture(output_texture_path):
    image = bpy.data.images.get("raven")
    if image is None:
        raise RuntimeError("Packed Raven texture was not found")

    os.makedirs(os.path.dirname(output_texture_path), exist_ok=True)
    if image.packed_file is not None:
        with open(output_texture_path, "wb") as output_file:
            output_file.write(image.packed_file.data)
    else:
        image.filepath_raw = output_texture_path
        image.file_format = "PNG"
        image.save()
    if not os.path.isfile(output_texture_path):
        raise RuntimeError(f"Failed to save Raven texture: {output_texture_path}")


def configure_materials(mesh_object, output_texture_path):
    image = bpy.data.images.get("raven")
    if image is None:
        raise RuntimeError("Raven texture was not found")

    image.filepath = f"//{os.path.basename(output_texture_path)}"
    for material in mesh_object.data.materials:
        if material is None:
            continue

        material["_x_texture_filename"] = os.path.basename(output_texture_path)
        material["_x_face_color"] = [1.0, 1.0, 1.0, 1.0]
        material.use_nodes = True

        node_tree = material.node_tree
        node_tree.nodes.clear()
        texture_node = node_tree.nodes.new("ShaderNodeTexImage")
        texture_node.name = "Raven Texture"
        texture_node.image = image
        texture_node.location = (-420.0, 40.0)

        shader_node = node_tree.nodes.new("ShaderNodeBsdfPrincipled")
        shader_node.location = (-100.0, 40.0)
        shader_node.inputs["Roughness"].default_value = 0.7

        output_node = node_tree.nodes.new("ShaderNodeOutputMaterial")
        output_node.location = (220.0, 40.0)

        node_tree.links.new(
            texture_node.outputs["Color"],
            shader_node.inputs["Base Color"],
        )
        node_tree.links.new(
            texture_node.outputs["Alpha"],
            shader_node.inputs["Alpha"],
        )
        node_tree.links.new(
            shader_node.outputs["BSDF"],
            output_node.inputs["Surface"],
        )

def validate_raven(armature, mesh_object):
    for scene_object in [armature, mesh_object]:
        for rotation in scene_object.rotation_euler:
            if abs(rotation) > TRANSFORM_EPSILON:
                raise RuntimeError(
                    f"Object rotation was not applied: {scene_object.name}"
                )

    beak_bone = armature.data.bones.get("beak")
    if beak_bone is None:
        raise RuntimeError("Raven beak bone was not found")
    if beak_bone.tail_local.y >= beak_bone.head_local.y:
        raise RuntimeError("Raven must face Blender -Y")

    required_actions = ["fly", "stand", "raven_hit", "raven_death"]
    for action_name in required_actions:
        if bpy.data.actions.get(action_name) is None:
            raise RuntimeError(f"Required Raven action was not found: {action_name}")


def main():
    source_path, output_blend_path, output_texture_path = parse_arguments()
    bpy.ops.wm.open_mainfile(filepath=source_path)

    armature = bpy.data.objects.get("raven_armature")
    mesh_object = bpy.data.objects.get("raven_mesh")
    if armature is None or armature.type != "ARMATURE":
        raise RuntimeError("Raven armature was not found")
    if mesh_object is None or mesh_object.type != "MESH":
        raise RuntimeError("Raven mesh was not found")
    if armature.animation_data is None:
        armature.animation_data_create()

    fly_action = bpy.data.actions.get("fly")
    stand_action = bpy.data.actions.get("stand")
    if fly_action is None or stand_action is None:
        raise RuntimeError("Raven source actions were not found")

    orient_raven(armature, mesh_object)
    create_hit_action(armature, stand_action)
    create_death_action(armature, stand_action)
    save_texture(output_texture_path)
    configure_materials(mesh_object, output_texture_path)

    armature.animation_data.action = fly_action
    bpy.context.scene.frame_start = int(fly_action.frame_range[0])
    bpy.context.scene.frame_end = int(fly_action.frame_range[1])
    bpy.context.scene.frame_set(bpy.context.scene.frame_start)
    bpy.context.view_layer.update()
    validate_raven(armature, mesh_object)

    os.makedirs(os.path.dirname(output_blend_path), exist_ok=True)
    result = bpy.ops.wm.save_as_mainfile(filepath=output_blend_path)
    if "FINISHED" not in result:
        raise RuntimeError(f"Failed to save Raven blend: {output_blend_path}")

    print(
        f"PREPARED raven output={output_blend_path} "
        f"texture={output_texture_path}"
    )


main()
