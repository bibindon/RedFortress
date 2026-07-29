"""Procedurally builds the Hoshigirl boss model (ghost-type) as a Blender file.

The Hoshigirl is a floating ghost-type boss for stage 3-8.  This script creates
the source asset (``Hoshigirl.blend``) entirely from primitives so that no
external mesh download is required.  The resulting blend is handed to
``PrepareEnemyModels.py`` (asset ``hoshigirl``) which exports the DirectX X
files and ``enemy.csv`` consumed by the game.

Coordinate convention (matches every other enemy asset):
    * +Z is up
    * the character faces Blender's -Y axis
    * origin is at the feet (z = 0)

Usage:
    blender --background --python BuildHoshigirlModel.py -- <output-directory>
"""

import math
import os
import sys

import bpy
import bmesh
from mathutils import Vector


ARMATURE_NAME = "HoshigirlArmature"
MATERIAL_NAME = "HoshigirlMat"
TEXTURE_FILENAME = "hoshigirl.png"
BLEND_FILENAME = "Hoshigirl.blend"
ENABLE_MODULE = "bl_ext.user_default.io_directx_x"

# Body height is ~1.5 m so the boss towers over the 1.7 m player once scaled.
BODY_TOP_Z = 1.48

# Lathe profile (z, radius) describing the ghost silhouette.
BODY_PROFILE = [
    (0.00, 0.34),
    (0.16, 0.40),
    (0.38, 0.45),
    (0.60, 0.465),
    (0.82, 0.44),
    (1.02, 0.38),
    (1.20, 0.30),
    (1.34, 0.21),
    (1.44, 0.11),
]

RING_SEGMENTS = 24
HEM_LOBES = 6
HEM_RADIUS_WOBBLE = 0.13
HEM_DROOP = 0.11

# Black UV strip on the left of the texture (face features live at U >= 0.25).
BLACK_U_MIN = 0.04
BLACK_U_MAX = 0.20

# Face decal UV box (matches the face drawn in generate_texture).
FACE_U_MIN = 0.25
FACE_U_MAX = 0.75
FACE_V_MIN = 0.26
FACE_V_MAX = 0.80

# Bone names (also used as vertex group names).
BONE_ROOT = "Root"
BONE_SPINE = "Spine"
BONE_HEAD = "Head"
BONE_L_UPPER_ARM = "L_UpperArm"
BONE_L_LOWER_ARM = "L_LowerArm"
BONE_L_HAND = "L_Hand"
BONE_R_UPPER_ARM = "R_UpperArm"
BONE_R_LOWER_ARM = "R_LowerArm"
BONE_R_HAND = "R_Hand"
ALL_BONES = (
    BONE_ROOT, BONE_SPINE, BONE_HEAD,
    BONE_L_UPPER_ARM, BONE_L_LOWER_ARM, BONE_L_HAND,
    BONE_R_UPPER_ARM, BONE_R_LOWER_ARM, BONE_R_HAND,
)

# Left arm chain (shoulder -> claw tip).  Right arm mirrors X.  The chain
# sweeps out to the side first, then forward and down so the claws read as
# arms from the front instead of merging with the body silhouette.
ARM_CHAIN = [
    (0.27, -0.02, 0.95, 0.105),
    (0.46, -0.12, 0.82, 0.092),
    (0.60, -0.26, 0.70, 0.078),
    (0.66, -0.40, 0.60, 0.062),
    (0.66, -0.52, 0.52, 0.044),
    (0.62, -0.62, 0.46, 0.016),
]
ARM_SEGMENTS = 10


def parse_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected arguments after --")
    arguments = sys.argv[sys.argv.index("--") + 1:]
    if len(arguments) != 1:
        raise RuntimeError(
            "Usage: blender --background --python BuildHoshigirlModel.py "
            "-- <output-directory>"
        )
    output_directory = os.path.abspath(arguments[0])
    os.makedirs(output_directory, exist_ok=True)
    return output_directory


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (
        bpy.data.actions, bpy.data.armatures, bpy.data.cameras,
        bpy.data.images, bpy.data.lights, bpy.data.materials,
        bpy.data.meshes, bpy.data.objects,
    ):
        for block in list(collection):
            if block.users == 0:
                collection.remove(block)


# --------------------------------------------------------------------------
# Geometry
# --------------------------------------------------------------------------

def lerp(a, b, t):
    return a + (b - a) * t


def build_body_mesh():
    mesh = bpy.data.meshes.new("Hoshigirl_Body")
    bm = bmesh.new()
    uv_layer = bm.loops.layers.uv.new("UV")

    rings = []
    for ring_index, (z, radius) in enumerate(BODY_PROFILE):
        ring = []
        for segment in range(RING_SEGMENTS):
            theta = (segment / RING_SEGMENTS) * 2.0 * math.pi
            point_x = radius * math.cos(theta)
            point_y = radius * math.sin(theta)
            point_z = z
            if ring_index == 0:
                lobe = 0.5 + 0.5 * math.cos(HEM_LOBES * theta)
                point_x *= 1.0 + HEM_RADIUS_WOBBLE * lobe
                point_y *= 1.0 + HEM_RADIUS_WOBBLE * lobe
                point_z = z - HEM_DROOP * lobe
            vertex = bm.verts.new((point_x, point_y, point_z))
            ring.append(vertex)
        rings.append(ring)

    for ring_index in range(len(rings) - 1):
        lower = rings[ring_index]
        upper = rings[ring_index + 1]
        v_factor_lower = 0.02 + 0.10 * (ring_index / float(len(BODY_PROFILE)))
        v_factor_upper = 0.02 + 0.10 * ((ring_index + 1) / float(len(BODY_PROFILE)))
        for segment in range(RING_SEGMENTS):
            next_segment = (segment + 1) % RING_SEGMENTS
            face = bm.faces.new((
                lower[segment], lower[next_segment],
                upper[next_segment], upper[segment],
            ))
            u_a = lerp(BLACK_U_MIN, BLACK_U_MAX, segment / float(RING_SEGMENTS))
            u_b = lerp(BLACK_U_MIN, BLACK_U_MAX, next_segment / float(RING_SEGMENTS))
            face.loops[0][uv_layer].uv = (u_a, v_factor_lower)
            face.loops[1][uv_layer].uv = (u_b, v_factor_lower)
            face.loops[2][uv_layer].uv = (u_b, v_factor_upper)
            face.loops[3][uv_layer].uv = (u_a, v_factor_upper)
            face.smooth = True

    apex = bm.verts.new((0.0, 0.0, BODY_TOP_Z))
    top_ring = rings[-1]
    v_top = 0.02 + 0.10
    for segment in range(RING_SEGMENTS):
        next_segment = (segment + 1) % RING_SEGMENTS
        face = bm.faces.new((top_ring[segment], top_ring[next_segment], apex))
        u_a = lerp(BLACK_U_MIN, BLACK_U_MAX, segment / float(RING_SEGMENTS))
        u_b = lerp(BLACK_U_MIN, BLACK_U_MAX, next_segment / float(RING_SEGMENTS))
        face.loops[0][uv_layer].uv = (u_a, v_top)
        face.loops[1][uv_layer].uv = (u_b, v_top)
        face.loops[2][uv_layer].uv = (0.12, 0.96)
        face.smooth = True

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()

    weights = []
    for vertex in mesh.vertices:
        coordinate = vertex.co
        if coordinate.z <= 0.55:
            weights.append({BONE_ROOT: 1.0})
        elif coordinate.z <= 0.85:
            blend = (coordinate.z - 0.55) / 0.30
            weights.append({BONE_SPINE: 1.0})
            if blend < 0.5:
                weights[-1] = {BONE_ROOT: 1.0 - blend * 2.0, BONE_SPINE: blend * 2.0}
        else:
            weights.append({BONE_HEAD: 1.0})
    return mesh, weights


def build_arm_mesh(side):
    mesh = bpy.data.meshes.new("Hoshigirl_Arm_" + side)
    bm = bmesh.new()
    uv_layer = bm.loops.layers.uv.new("UV")

    sign = 1.0 if side == "L" else -1.0
    points = [(sign * x, y, z, r) for (x, y, z, r) in ARM_CHAIN]

    rings = []
    for segment in range(ARM_SEGMENTS + 1):
        parameter = segment / float(ARM_SEGMENTS)
        lower_index = int(parameter * (len(points) - 1))
        upper_index = min(lower_index + 1, len(points) - 1)
        local = parameter * (len(points) - 1) - lower_index
        x0, y0, z0, r0 = points[lower_index]
        x1, y1, z1, r1 = points[upper_index]
        center = Vector((lerp(x0, x1, local), lerp(y0, y1, local), lerp(z0, z1, local)))
        radius = lerp(r0, r1, local)
        direction = (Vector((x1, y1, z1)) - Vector((x0, y0, z0)))
        if direction.length < 1e-6:
            direction = Vector((0.0, 0.0, 1.0))
        direction.normalize()
        normal_a = direction.cross(Vector((0.0, 0.0, 1.0)))
        if normal_a.length < 1e-6:
            normal_a = direction.cross(Vector((1.0, 0.0, 0.0)))
        normal_a.normalize()
        normal_b = direction.cross(normal_a).normalized()
        ring = []
        for index in range(RING_SEGMENTS):
            theta = (index / RING_SEGMENTS) * 2.0 * math.pi
            offset = (normal_a * math.cos(theta) + normal_b * math.sin(theta)) * radius
            vertex = bm.verts.new(center + offset)
            ring.append(vertex)
        rings.append((ring, parameter))

    for ring_index in range(len(rings) - 1):
        lower, param_lower = rings[ring_index]
        upper, param_upper = rings[ring_index + 1]
        for index in range(RING_SEGMENTS):
            next_index = (index + 1) % RING_SEGMENTS
            face = bm.faces.new((
                lower[index], lower[next_index],
                upper[next_index], upper[index],
            ))
            u_a = lerp(BLACK_U_MIN, BLACK_U_MAX, index / float(RING_SEGMENTS))
            u_b = lerp(BLACK_U_MIN, BLACK_U_MAX, next_index / float(RING_SEGMENTS))
            v_a = 0.02 + 0.08 * param_lower
            v_b = 0.02 + 0.08 * param_upper
            face.loops[0][uv_layer].uv = (u_a, v_a)
            face.loops[1][uv_layer].uv = (u_b, v_a)
            face.loops[2][uv_layer].uv = (u_b, v_b)
            face.loops[3][uv_layer].uv = (u_a, v_b)
            face.smooth = True

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()

    weights = []
    for info_index, (ring, parameter) in enumerate(rings):
        pass  # parameter is per-ring; vertex weights use vertex positions below

    weights = []
    for vertex in mesh.vertices:
        # Recover parameter from z against the chain span.
        z = vertex.co.z
        parameter = (z - ARM_CHAIN[0][2]) / (ARM_CHAIN[-1][2] - ARM_CHAIN[0][2])
        parameter = max(0.0, min(1.0, parameter))
        if parameter <= 0.4:
            group = {BONE_L_UPPER_ARM if side == "L" else BONE_R_UPPER_ARM: 1.0}
        elif parameter <= 0.78:
            group = {BONE_L_LOWER_ARM if side == "L" else BONE_R_LOWER_ARM: 1.0}
        else:
            group = {BONE_L_HAND if side == "L" else BONE_R_HAND: 1.0}
        weights.append(group)
    return mesh, weights


def build_face_decal_mesh():
    mesh = bpy.data.meshes.new("Hoshigirl_Face")
    bm = bmesh.new()
    uv_layer = bm.loops.layers.uv.new("UV")

    center_z = 1.14
    half_width = 0.20
    half_height = 0.22
    front_y = -0.42  # in front of the head surface (~-0.34) so the face reads
    bl = bm.verts.new((-half_width, front_y, center_z - half_height))
    br = bm.verts.new((half_width, front_y, center_z - half_height))
    tr = bm.verts.new((half_width, front_y, center_z + half_height))
    tl = bm.verts.new((-half_width, front_y, center_z + half_height))
    face = bm.faces.new((bl, br, tr, tl))
    face.loops[0][uv_layer].uv = (FACE_U_MIN, FACE_V_MIN)
    face.loops[1][uv_layer].uv = (FACE_U_MAX, FACE_V_MIN)
    face.loops[2][uv_layer].uv = (FACE_U_MAX, FACE_V_MAX)
    face.loops[3][uv_layer].uv = (FACE_U_MIN, FACE_V_MAX)
    face.normal_update()

    bm.to_mesh(mesh)
    bm.free()

    weights = [{BONE_HEAD: 1.0} for _ in mesh.vertices]
    return mesh, weights


# --------------------------------------------------------------------------
# Armature
# --------------------------------------------------------------------------

def create_armature():
    armature_data = bpy.data.armatures.new(ARMATURE_NAME)
    armature_obj = bpy.data.objects.new(ARMATURE_NAME, armature_data)
    bpy.context.collection.objects.link(armature_obj)
    armature_obj.location = (0.0, 0.0, 0.0)
    armature_obj.rotation_euler = (0.0, 0.0, 0.0)
    armature_obj.scale = (1.0, 1.0, 1.0)

    bpy.context.view_layer.objects.active = armature_obj
    bpy.ops.object.mode_set(mode="EDIT")

    edit_bones = armature_data.edit_bones
    root = edit_bones.new(BONE_ROOT)
    root.head = (0.0, 0.0, 0.45)
    root.tail = (0.0, 0.0, 0.58)

    spine = edit_bones.new(BONE_SPINE)
    spine.head = (0.0, 0.0, 0.58)
    spine.tail = (0.0, 0.0, 0.95)
    spine.parent = root
    spine.use_connect = True

    head = edit_bones.new(BONE_HEAD)
    head.head = (0.0, 0.0, 0.95)
    head.tail = (0.0, 0.0, 1.34)
    head.parent = spine
    head.use_connect = True

    for side, upper_name, lower_name, hand_name in (
        ("L", BONE_L_UPPER_ARM, BONE_L_LOWER_ARM, BONE_L_HAND),
        ("R", BONE_R_UPPER_ARM, BONE_R_LOWER_ARM, BONE_R_HAND),
    ):
        sign = 1.0 if side == "L" else -1.0
        shoulder = Vector((sign * ARM_CHAIN[0][0], ARM_CHAIN[0][1], ARM_CHAIN[0][2]))
        elbow = Vector((sign * ARM_CHAIN[2][0], ARM_CHAIN[2][1], ARM_CHAIN[2][2]))
        wrist = Vector((sign * ARM_CHAIN[4][0], ARM_CHAIN[4][1], ARM_CHAIN[4][2]))
        tip = Vector((sign * ARM_CHAIN[5][0], ARM_CHAIN[5][1], ARM_CHAIN[5][2]))

        upper = edit_bones.new(upper_name)
        upper.head = shoulder
        upper.tail = elbow
        upper.parent = spine

        lower = edit_bones.new(lower_name)
        lower.head = elbow
        lower.tail = wrist
        lower.parent = upper
        lower.use_connect = True

        hand = edit_bones.new(hand_name)
        hand.head = wrist
        hand.tail = tip
        hand.parent = lower
        hand.use_connect = True

    bpy.ops.object.mode_set(mode="OBJECT")
    return armature_obj


def link_mesh(name, mesh, weights, armature_obj):
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    obj.location = (0.0, 0.0, 0.0)
    obj.rotation_euler = (0.0, 0.0, 0.0)
    obj.scale = (1.0, 1.0, 1.0)
    obj.parent = armature_obj
    modifier = obj.modifiers.new(name="Armature", type="ARMATURE")
    modifier.object = armature_obj
    modifier.use_vertex_groups = True
    for bone_name in ALL_BONES:
        obj.vertex_groups.new(name=bone_name)
    for vertex_index, weight_map in enumerate(weights):
        for bone_name, weight_value in weight_map.items():
            obj.vertex_groups[bone_name].add([vertex_index], weight_value, "REPLACE")
    return obj


# --------------------------------------------------------------------------
# Texture + material
# --------------------------------------------------------------------------

def generate_texture(path):
    size = 256
    pixels = [0.0, 0.0, 0.0, 1.0] * (size * size)

    def write_pixel(x, y, red, green, blue):
        if 0 <= x < size and 0 <= y < size:
            index = (y * size + x) * 4
            pixels[index] = red
            pixels[index + 1] = green
            pixels[index + 2] = blue
            pixels[index + 3] = 1.0

    def draw_plus(center_x, center_y, arm, thickness, value=1.0):
        for delta in range(-arm, arm + 1):
            for spread in range(-thickness, thickness + 1):
                write_pixel(center_x + delta, center_y + spread, value, value, value)
                write_pixel(center_x + spread, center_y + delta, value, value, value)

    def draw_arc(center_x, center_y, radius, start_deg, end_deg, thickness, value=1.0):
        for degree in range(start_deg, end_deg + 1):
            rad = math.radians(degree)
            px = int(round(center_x + radius * math.cos(rad)))
            py = int(round(center_y + radius * math.sin(rad)))
            for dx in range(-thickness, thickness + 1):
                for dy in range(-thickness, thickness + 1):
                    write_pixel(px + dx, py + dy, value, value, value)

    # Face region inside the texture (matches FACE_U/V box).  The texture uses
    # bottom-left origin, so convert from UV space to pixels.
    face_left = int(FACE_U_MIN * size)
    face_right = int(FACE_U_MAX * size)
    face_bottom = int(FACE_V_MIN * size)
    face_top = int(FACE_V_MAX * size)
    center_x = (face_left + face_right) // 2
    eye_y = face_top - int((face_top - face_bottom) * 0.30)
    eye_offset = int((face_right - face_left) * 0.18)
    draw_plus(center_x - eye_offset, eye_y, arm=7, thickness=2)
    draw_plus(center_x + eye_offset, eye_y, arm=7, thickness=2)
    mouth_y = face_bottom + int((face_top - face_bottom) * 0.22)
    mouth_radius = int((face_right - face_left) * 0.26)
    # Smile: lower hemisphere arc (180..360 opens downward in image space).
    draw_arc(center_x, mouth_y, mouth_radius, 200, 340, thickness=2)

    image = bpy.data.images.new(TEXTURE_FILENAME, width=size, height=size, alpha=True)
    image.pixels = pixels
    image.filepath_raw = path
    image.file_format = "PNG"
    image.save()
    return image


def create_material(texture_path):
    material = bpy.data.materials.new(MATERIAL_NAME)
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    for node in list(nodes):
        nodes.remove(node)

    output = nodes.new("ShaderNodeOutputMaterial")
    output.location = (300, 0)
    principled = nodes.new("ShaderNodeBsdfPrincipled")
    principled.location = (0, 0)
    principled.inputs["Base Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    principled.inputs["Roughness"].default_value = 0.55
    texture_node = nodes.new("ShaderNodeTexImage")
    texture_node.location = (-350, 0)
    texture_node.name = "HoshigirlBaseColor.Image"
    texture_node.label = "HoshigirlBaseColor.Image"

    image = bpy.data.images.get(TEXTURE_FILENAME)
    if image is None:
        image = bpy.data.images.load(texture_path)
    else:
        image.filepath = texture_path
    texture_node.image = image

    links.new(texture_node.outputs["Color"], principled.inputs["Base Color"])
    links.new(principled.outputs["BSDF"], output.inputs["Surface"])

    # Custom properties consumed by the DirectX X exporter.
    material["_x_face_color"] = (1.0, 1.0, 1.0, 1.0)
    material["_x_power"] = 24.0
    material["_x_specular"] = (0.1, 0.1, 0.1)
    material["_x_emissive"] = (0.0, 0.0, 0.0)
    material["_x_texture_filename"] = TEXTURE_FILENAME
    return material


# --------------------------------------------------------------------------
# Animation
# --------------------------------------------------------------------------

def configure_pose_bones(armature_obj):
    for pose_bone in armature_obj.pose.bones:
        pose_bone.rotation_mode = "XYZ"


def key_rotation(pose_bone, rotation, frame):
    pose_bone.rotation_euler = rotation
    pose_bone.keyframe_insert(data_path="rotation_euler", frame=frame)


def key_location(pose_bone, location, frame):
    pose_bone.location = location
    pose_bone.keyframe_insert(data_path="location", frame=frame)


def reset_pose(armature_obj):
    for pose_bone in armature_obj.pose.bones:
        pose_bone.rotation_euler = (0.0, 0.0, 0.0)
        pose_bone.location = (0.0, 0.0, 0.0)


def make_action(armature_obj, name, frames, populate):
    action = bpy.data.actions.new(name)
    action.use_fake_user = True
    armature_obj.animation_data_create().action = action
    reset_pose(armature_obj)
    populate()
    for fcurve in action.fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "SINE"
    armature_obj.animation_data.action = None
    return action


def populate_idle(armature_obj):
    root = armature_obj.pose.bones[BONE_ROOT]
    head = armature_obj.pose.bones[BONE_HEAD]
    left_upper = armature_obj.pose.bones[BONE_L_UPPER_ARM]
    right_upper = armature_obj.pose.bones[BONE_R_UPPER_ARM]
    for frame, lift in ((1, 0.0), (20, 0.06), (40, 0.0)):
        key_location(root, (0.0, 0.0, lift), frame)
        key_rotation(head, (math.radians(-2.0 + 4.0 * (lift / 0.06)), 0.0, 0.0), frame)
        key_rotation(left_upper, (math.radians(-70.0), 0.0, math.radians(-6.0)), frame)
        key_rotation(right_upper, (math.radians(-70.0), 0.0, math.radians(6.0)), frame)


def populate_move(armature_obj):
    spine = armature_obj.pose.bones[BONE_SPINE]
    left_upper = armature_obj.pose.bones[BONE_L_UPPER_ARM]
    right_upper = armature_obj.pose.bones[BONE_R_UPPER_ARM]
    key_rotation(spine, (0.0, 0.0, math.radians(6.0)), 1)
    key_rotation(left_upper, (math.radians(-68.0), 0.0, math.radians(-12.0)), 1)
    key_rotation(right_upper, (math.radians(-72.0), 0.0, math.radians(12.0)), 1)
    key_rotation(spine, (0.0, 0.0, math.radians(-6.0)), 16)
    key_rotation(left_upper, (math.radians(-72.0), 0.0, math.radians(-12.0)), 16)
    key_rotation(right_upper, (math.radians(-68.0), 0.0, math.radians(12.0)), 16)
    key_rotation(spine, (0.0, 0.0, math.radians(6.0)), 30)


def populate_fast_move(armature_obj):
    spine = armature_obj.pose.bones[BONE_SPINE]
    left_upper = armature_obj.pose.bones[BONE_L_UPPER_ARM]
    right_upper = armature_obj.pose.bones[BONE_R_UPPER_ARM]
    key_rotation(spine, (math.radians(6.0), 0.0, math.radians(10.0)), 1)
    key_rotation(left_upper, (math.radians(-60.0), 0.0, math.radians(-18.0)), 1)
    key_rotation(right_upper, (math.radians(-60.0), 0.0, math.radians(18.0)), 1)
    key_rotation(spine, (math.radians(6.0), 0.0, math.radians(-10.0)), 10)
    key_rotation(left_upper, (math.radians(-80.0), 0.0, math.radians(-18.0)), 10)
    key_rotation(right_upper, (math.radians(-80.0), 0.0, math.radians(18.0)), 10)
    key_rotation(spine, (math.radians(6.0), 0.0, math.radians(10.0)), 20)


def populate_attack(armature_obj):
    spine = armature_obj.pose.bones[BONE_SPINE]
    head = armature_obj.pose.bones[BONE_HEAD]
    left_upper = armature_obj.pose.bones[BONE_L_UPPER_ARM]
    right_upper = armature_obj.pose.bones[BONE_R_UPPER_ARM]
    left_lower = armature_obj.pose.bones[BONE_L_LOWER_ARM]
    right_lower = armature_obj.pose.bones[BONE_R_LOWER_ARM]
    key_rotation(spine, (math.radians(-8.0), 0.0, 0.0), 1)
    key_rotation(left_upper, (math.radians(-110.0), 0.0, math.radians(-18.0)), 1)
    key_rotation(right_upper, (math.radians(-110.0), 0.0, math.radians(18.0)), 1)
    key_rotation(spine, (math.radians(12.0), 0.0, 0.0), 12)
    key_rotation(head, (math.radians(8.0), 0.0, 0.0), 12)
    key_rotation(left_upper, (math.radians(-40.0), 0.0, math.radians(-22.0)), 12)
    key_rotation(right_upper, (math.radians(-40.0), 0.0, math.radians(22.0)), 12)
    key_rotation(left_lower, (0.0, 0.0, math.radians(-30.0)), 12)
    key_rotation(right_lower, (0.0, 0.0, math.radians(30.0)), 12)
    key_rotation(spine, (math.radians(4.0), 0.0, 0.0), 26)
    key_rotation(left_upper, (math.radians(-70.0), 0.0, math.radians(-8.0)), 26)
    key_rotation(right_upper, (math.radians(-70.0), 0.0, math.radians(8.0)), 26)


def populate_hit(armature_obj):
    spine = armature_obj.pose.bones[BONE_SPINE]
    head = armature_obj.pose.bones[BONE_HEAD]
    root = armature_obj.pose.bones[BONE_ROOT]
    key_rotation(spine, (math.radians(18.0), 0.0, 0.0), 1)
    key_rotation(head, (math.radians(12.0), 0.0, 0.0), 1)
    key_location(root, (0.0, 0.10, 0.0), 1)
    key_rotation(spine, (math.radians(-2.0), 0.0, 0.0), 14)
    key_rotation(head, (math.radians(-2.0), 0.0, 0.0), 14)
    key_location(root, (0.0, 0.0, 0.0), 14)


def populate_death(armature_obj):
    root = armature_obj.pose.bones[BONE_ROOT]
    spine = armature_obj.pose.bones[BONE_SPINE]
    head = armature_obj.pose.bones[BONE_HEAD]
    left_upper = armature_obj.pose.bones[BONE_L_UPPER_ARM]
    right_upper = armature_obj.pose.bones[BONE_R_UPPER_ARM]
    key_rotation(spine, (0.0, 0.0, 0.0), 1)
    key_rotation(left_upper, (math.radians(-70.0), 0.0, math.radians(-6.0)), 1)
    key_rotation(right_upper, (math.radians(-70.0), 0.0, math.radians(6.0)), 1)
    key_location(root, (0.0, 0.0, 0.0), 1)
    key_rotation(spine, (math.radians(40.0), 0.0, 0.0), 25)
    key_rotation(head, (math.radians(30.0), 0.0, 0.0), 25)
    key_rotation(left_upper, (math.radians(-30.0), 0.0, math.radians(-30.0)), 25)
    key_rotation(right_upper, (math.radians(-30.0), 0.0, math.radians(30.0)), 25)
    key_location(root, (0.0, 0.0, -0.40), 25)
    key_location(root, (0.0, 0.0, -0.70), 50)
    key_rotation(spine, (math.radians(70.0), 0.0, 0.0), 50)
    key_rotation(head, (math.radians(50.0), 0.0, 0.0), 50)


def create_animations(armature_obj):
    configure_pose_bones(armature_obj)
    make_action(armature_obj, "idle", (1, 40), lambda: populate_idle(armature_obj))
    make_action(armature_obj, "move", (1, 30), lambda: populate_move(armature_obj))
    make_action(armature_obj, "fast_move", (1, 20), lambda: populate_fast_move(armature_obj))
    make_action(armature_obj, "attack", (1, 30), lambda: populate_attack(armature_obj))
    make_action(armature_obj, "hit", (1, 20), lambda: populate_hit(armature_obj))
    make_action(armature_obj, "death", (1, 50), lambda: populate_death(armature_obj))
    bpy.context.scene.render.fps = 30


# --------------------------------------------------------------------------
# Main
# --------------------------------------------------------------------------

def main():
    output_directory = parse_arguments()
    clear_scene()

    texture_path = os.path.join(output_directory, TEXTURE_FILENAME)
    generate_texture(texture_path)
    material = create_material(texture_path)

    body_mesh, body_weights = build_body_mesh()
    left_mesh, left_weights = build_arm_mesh("L")
    right_mesh, right_weights = build_arm_mesh("R")
    face_mesh, face_weights = build_face_decal_mesh()

    armature_obj = create_armature()

    body_obj = link_mesh("Hoshigirl_Body", body_mesh, body_weights, armature_obj)
    left_obj = link_mesh("Hoshigirl_Arm_L", left_mesh, left_weights, armature_obj)
    right_obj = link_mesh("Hoshigirl_Arm_R", right_mesh, right_weights, armature_obj)
    face_obj = link_mesh("Hoshigirl_Face", face_mesh, face_weights, armature_obj)

    for obj in (body_obj, left_obj, right_obj, face_obj):
        obj.data.materials.clear()
        obj.data.materials.append(material)

    create_animations(armature_obj)

    bpy.context.view_layer.update()

    blend_path = os.path.join(output_directory, BLEND_FILENAME)
    save_result = bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    if "FINISHED" not in save_result:
        raise RuntimeError("Failed to save Blender file: " + blend_path)

    print(
        "HOSHIGIRL_MODEL_BUILT "
        "bones={} "
        "texture={} "
        "blend={}".format(len(ALL_BONES), texture_path, blend_path)
    )


main()
