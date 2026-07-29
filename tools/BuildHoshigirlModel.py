"""Builds the Hoshigirl boss model (black sheet ghost) as a Blender file.

The Hoshigirl is the floating ghost boss for stage 3-8.  This script rebuilds
the source asset (``Hoshigirl.blend``) from scratch based on the reference
image ``hoshigaaru.png``:

    * a black draped sheet body with a wavy hem and a trailing back
    * white plus-sign eyes and a white smiling mouth on the face
    * asymmetric arms: the right arm reaches out to the side with claws,
      the left arm curls in front of the chest with claws

The resulting blend is handed to ``PrepareEnemyModels.py`` (asset
``hoshigirl``) which exports the DirectX X files and ``enemy.csv``.
Only an idle action is authored; every logical animation maps to it.

Coordinate convention (matches every other enemy asset):
    * +Z is up
    * the character faces Blender's -Y axis
    * origin is at the feet (z = 0)

Model height is about 1.6 m.  The game scales it to about 4 m via
``EnemyHoshigirl::GetScale()``.

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

TEXTURE_SIZE = 512
BODY_DARK_VALUE = 0.02

# Lathe profile (z, radius) describing the sheet silhouette from hem to neck.
BODY_PROFILE = [
    (0.00, 0.55),
    (0.12, 0.48),
    (0.30, 0.43),
    (0.50, 0.40),
    (0.70, 0.39),
    (0.90, 0.375),
    (1.05, 0.36),
    (1.20, 0.33),
    (1.35, 0.28),
    (1.47, 0.20),
    (1.55, 0.11),
]
BODY_TOP_Z = 1.60

RING_SEGMENTS = 32
HEM_LOBES = 7
HEM_RADIUS_WOBBLE = 0.16
HEM_DROOP = 0.10
# The hem is stretched backwards (+Y) so the sheet trails like in the image.
HEM_TRAIN_STRETCH = 1.45

# Dark UV strip (everything except the face samples this black area).
BLACK_U_MIN = 0.03
BLACK_U_MAX = 0.18
BLACK_V_MIN = 0.05
BLACK_V_MAX = 0.95

# Face decal UV box (matches the markings drawn in generate_texture).
FACE_U_MIN = 0.30
FACE_U_MAX = 0.90
FACE_V_MIN = 0.22
FACE_V_MAX = 0.86

# The face markings are mapped directly onto the front body quads so that no
# decal seam is visible.  Quad rows FACE_RING_START..FACE_RING_END-1 and quad
# columns FACE_SEGMENT_START..FACE_SEGMENT_END-1 sample the face UV box.
# Ring 5 is z=0.90 and ring 9 is z=1.47; segments 20..28 cover the -Y front
# quarter (theta 225..315 degrees).
FACE_RING_START = 5
FACE_RING_END = 9
FACE_SEGMENT_START = 20
FACE_SEGMENT_END = 28

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

# Right arm (character right = -X): reaches out to the side like the image.
RIGHT_ARM_CHAIN = [
    (-0.24, -0.06, 1.00, 0.095),
    (-0.40, -0.12, 0.90, 0.085),
    (-0.55, -0.18, 0.83, 0.070),
    (-0.68, -0.25, 0.78, 0.055),
    (-0.76, -0.31, 0.74, 0.040),
]
RIGHT_CLAW_DIRECTIONS = [
    (-0.60, -0.50, -0.35),
    (-0.80, -0.30, -0.15),
    (-0.45, -0.70, -0.10),
]

# Left arm (character left = +X): curls in front of the chest.
LEFT_ARM_CHAIN = [
    (0.24, -0.06, 1.00, 0.095),
    (0.38, -0.16, 0.93, 0.085),
    (0.42, -0.30, 0.88, 0.070),
    (0.33, -0.40, 0.83, 0.055),
    (0.20, -0.45, 0.79, 0.040),
]
LEFT_CLAW_DIRECTIONS = [
    (-0.20, -0.50, -0.80),
    (0.15, -0.60, -0.75),
    (-0.45, -0.40, -0.75),
]

ARM_RING_COUNT = 12
ARM_RING_SEGMENTS = 12
CLAW_LENGTH = 0.14
CLAW_RADIUS = 0.028
CLAW_SEGMENTS = 8


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


def lerp(a, b, t):
    return a + (b - a) * t


def black_uv(u_ratio, v_ratio):
    u = lerp(BLACK_U_MIN, BLACK_U_MAX, u_ratio)
    v = lerp(BLACK_V_MIN, BLACK_V_MAX, v_ratio)
    return (u, v)


# --------------------------------------------------------------------------
# Geometry
# --------------------------------------------------------------------------

def body_weight_for_z(z):
    if z <= 0.50:
        return {BONE_ROOT: 1.0}
    if z <= 0.75:
        blend = (z - 0.50) / 0.25
        return {BONE_ROOT: 1.0 - blend, BONE_SPINE: blend}
    if z <= 1.00:
        return {BONE_SPINE: 1.0}
    if z <= 1.20:
        blend = (z - 1.00) / 0.20
        return {BONE_SPINE: 1.0 - blend, BONE_HEAD: blend}
    return {BONE_HEAD: 1.0}


def build_body_mesh():
    mesh = bpy.data.meshes.new("Hoshigirl_Body")
    bm = bmesh.new()
    uv_layer = bm.loops.layers.uv.new("UV")
    weights = []

    ring_count = len(BODY_PROFILE)
    rings = []
    for ring_index, (z, radius) in enumerate(BODY_PROFILE):
        ring = []
        for segment in range(RING_SEGMENTS):
            theta = (segment / RING_SEGMENTS) * 2.0 * math.pi
            point_x = radius * math.cos(theta)
            point_y = radius * math.sin(theta)
            point_z = z
            if ring_index == 0:
                # Wavy hem lobes plus a droop between the lobes.
                lobe = 0.5 + 0.5 * math.cos(HEM_LOBES * theta)
                point_x *= 1.0 + HEM_RADIUS_WOBBLE * lobe
                point_y *= 1.0 + HEM_RADIUS_WOBBLE * lobe
                point_z = z - HEM_DROOP * (1.0 - lobe)
                # Stretch the back of the hem into a trailing train.
                if point_y > 0.0:
                    point_y *= HEM_TRAIN_STRETCH
            elif ring_index == 1:
                if point_y > 0.0:
                    point_y *= 1.0 + (HEM_TRAIN_STRETCH - 1.0) * 0.4
            else:
                # Subtle vertical cloth folds on the sheet.
                fold = 0.012 * math.sin(5.0 * theta + z * 2.0)
                scale = 1.0 + fold * (1.0 - z / BODY_TOP_Z)
                point_x *= scale
                point_y *= scale
            vertex = bm.verts.new((point_x, point_y, point_z))
            weights.append(body_weight_for_z(z))
            ring.append(vertex)
        rings.append(ring)

    for ring_index in range(ring_count - 1):
        lower = rings[ring_index]
        upper = rings[ring_index + 1]
        v_lower = ring_index / float(ring_count)
        v_upper = (ring_index + 1) / float(ring_count)
        is_face_row = FACE_RING_START <= ring_index < FACE_RING_END - 1
        face_v_lower = lerp(
            FACE_V_MIN, FACE_V_MAX,
            (ring_index - FACE_RING_START) / float(FACE_RING_END - 1 - FACE_RING_START))
        face_v_upper = lerp(
            FACE_V_MIN, FACE_V_MAX,
            (ring_index + 1 - FACE_RING_START) / float(FACE_RING_END - 1 - FACE_RING_START))
        for segment in range(RING_SEGMENTS):
            next_segment = (segment + 1) % RING_SEGMENTS
            face = bm.faces.new((
                lower[segment], lower[next_segment],
                upper[next_segment], upper[segment],
            ))
            if is_face_row and FACE_SEGMENT_START <= segment < FACE_SEGMENT_END:
                span = float(FACE_SEGMENT_END - FACE_SEGMENT_START)
                face_u_a = lerp(
                    FACE_U_MAX, FACE_U_MIN,
                    (segment - FACE_SEGMENT_START) / span)
                face_u_b = lerp(
                    FACE_U_MAX, FACE_U_MIN,
                    (segment + 1 - FACE_SEGMENT_START) / span)
                face.loops[0][uv_layer].uv = (face_u_a, face_v_lower)
                face.loops[1][uv_layer].uv = (face_u_b, face_v_lower)
                face.loops[2][uv_layer].uv = (face_u_b, face_v_upper)
                face.loops[3][uv_layer].uv = (face_u_a, face_v_upper)
            else:
                u_a = segment / float(RING_SEGMENTS)
                u_b = next_segment / float(RING_SEGMENTS)
                face.loops[0][uv_layer].uv = black_uv(u_a, v_lower)
                face.loops[1][uv_layer].uv = black_uv(u_b, v_lower)
                face.loops[2][uv_layer].uv = black_uv(u_b, v_upper)
                face.loops[3][uv_layer].uv = black_uv(u_a, v_upper)
            face.smooth = True

    # Rounded head cap.
    apex = bm.verts.new((0.0, 0.0, BODY_TOP_Z))
    weights.append({BONE_HEAD: 1.0})
    top_ring = rings[-1]
    for segment in range(RING_SEGMENTS):
        next_segment = (segment + 1) % RING_SEGMENTS
        face = bm.faces.new((top_ring[segment], top_ring[next_segment], apex))
        u_a = segment / float(RING_SEGMENTS)
        u_b = next_segment / float(RING_SEGMENTS)
        face.loops[0][uv_layer].uv = black_uv(u_a, 0.96)
        face.loops[1][uv_layer].uv = black_uv(u_b, 0.96)
        face.loops[2][uv_layer].uv = black_uv(0.5, 1.0)
        face.smooth = True

    # Bottom cap so the hem is not open when seen from below.
    bottom_center = bm.verts.new((0.0, 0.05, 0.02))
    weights.append({BONE_ROOT: 1.0})
    bottom_ring = rings[0]
    for segment in range(RING_SEGMENTS):
        next_segment = (segment + 1) % RING_SEGMENTS
        face = bm.faces.new((
            bottom_ring[next_segment], bottom_ring[segment], bottom_center,
        ))
        face.loops[0][uv_layer].uv = black_uv(0.4, 0.0)
        face.loops[1][uv_layer].uv = black_uv(0.6, 0.0)
        face.loops[2][uv_layer].uv = black_uv(0.5, 0.05)
        face.smooth = True

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    return mesh, weights


def arm_weight_for_parameter(side, parameter):
    if side == "L":
        upper_name = BONE_L_UPPER_ARM
        lower_name = BONE_L_LOWER_ARM
        hand_name = BONE_L_HAND
    else:
        upper_name = BONE_R_UPPER_ARM
        lower_name = BONE_R_LOWER_ARM
        hand_name = BONE_R_HAND
    if parameter <= 0.45:
        return {upper_name: 1.0}
    if parameter <= 0.75:
        return {lower_name: 1.0}
    return {hand_name: 1.0}


def sample_chain(chain, parameter):
    span = len(chain) - 1
    scaled = parameter * span
    lower_index = min(int(scaled), span - 1)
    local = scaled - lower_index
    x0, y0, z0, r0 = chain[lower_index]
    x1, y1, z1, r1 = chain[lower_index + 1]
    center = Vector((lerp(x0, x1, local), lerp(y0, y1, local), lerp(z0, z1, local)))
    radius = lerp(r0, r1, local)
    direction = Vector((x1 - x0, y1 - y0, z1 - z0))
    if direction.length < 1e-6:
        direction = Vector((0.0, 0.0, -1.0))
    direction.normalize()
    return center, radius, direction


def ring_basis(direction):
    normal_a = direction.cross(Vector((0.0, 0.0, 1.0)))
    if normal_a.length < 1e-6:
        normal_a = direction.cross(Vector((1.0, 0.0, 0.0)))
    normal_a.normalize()
    normal_b = direction.cross(normal_a).normalized()
    return normal_a, normal_b


def add_claw(bm, uv_layer, weights, base, direction, hand_weight):
    claw_direction = Vector(direction).normalized()
    normal_a, normal_b = ring_basis(claw_direction)
    base_ring = []
    for index in range(CLAW_SEGMENTS):
        theta = (index / CLAW_SEGMENTS) * 2.0 * math.pi
        offset = (normal_a * math.cos(theta) + normal_b * math.sin(theta))
        vertex = bm.verts.new(base + offset * CLAW_RADIUS)
        weights.append(dict(hand_weight))
        base_ring.append(vertex)
    tip = bm.verts.new(base + claw_direction * CLAW_LENGTH)
    weights.append(dict(hand_weight))

    for index in range(CLAW_SEGMENTS):
        next_index = (index + 1) % CLAW_SEGMENTS
        face = bm.faces.new((base_ring[index], base_ring[next_index], tip))
        face.loops[0][uv_layer].uv = black_uv(0.2, 0.2)
        face.loops[1][uv_layer].uv = black_uv(0.3, 0.2)
        face.loops[2][uv_layer].uv = black_uv(0.25, 0.35)
        face.smooth = True
    for index in range(1, CLAW_SEGMENTS - 1):
        face = bm.faces.new((
            base_ring[0], base_ring[index + 1], base_ring[index],
        ))
        face.loops[0][uv_layer].uv = black_uv(0.2, 0.2)
        face.loops[1][uv_layer].uv = black_uv(0.3, 0.2)
        face.loops[2][uv_layer].uv = black_uv(0.25, 0.3)
        face.smooth = True


def build_arm_mesh(side):
    if side == "L":
        chain = LEFT_ARM_CHAIN
        claw_directions = LEFT_CLAW_DIRECTIONS
    else:
        chain = RIGHT_ARM_CHAIN
        claw_directions = RIGHT_CLAW_DIRECTIONS

    mesh = bpy.data.meshes.new("Hoshigirl_Arm_" + side)
    bm = bmesh.new()
    uv_layer = bm.loops.layers.uv.new("UV")
    weights = []

    rings = []
    for ring_index in range(ARM_RING_COUNT + 1):
        parameter = ring_index / float(ARM_RING_COUNT)
        center, radius, direction = sample_chain(chain, parameter)
        normal_a, normal_b = ring_basis(direction)
        ring = []
        for index in range(ARM_RING_SEGMENTS):
            theta = (index / ARM_RING_SEGMENTS) * 2.0 * math.pi
            offset = (normal_a * math.cos(theta) + normal_b * math.sin(theta))
            vertex = bm.verts.new(center + offset * radius)
            weights.append(arm_weight_for_parameter(side, parameter))
            ring.append(vertex)
        rings.append((ring, parameter))

    for ring_index in range(len(rings) - 1):
        lower, param_lower = rings[ring_index]
        upper, param_upper = rings[ring_index + 1]
        for index in range(ARM_RING_SEGMENTS):
            next_index = (index + 1) % ARM_RING_SEGMENTS
            face = bm.faces.new((
                lower[index], lower[next_index],
                upper[next_index], upper[index],
            ))
            u_a = index / float(ARM_RING_SEGMENTS)
            u_b = next_index / float(ARM_RING_SEGMENTS)
            face.loops[0][uv_layer].uv = black_uv(u_a, 0.3 + 0.4 * param_lower)
            face.loops[1][uv_layer].uv = black_uv(u_b, 0.3 + 0.4 * param_lower)
            face.loops[2][uv_layer].uv = black_uv(u_b, 0.3 + 0.4 * param_upper)
            face.loops[3][uv_layer].uv = black_uv(u_a, 0.3 + 0.4 * param_upper)
            face.smooth = True

    # Cap the wrist end, then grow claws out of it.
    hand_center, hand_radius, hand_direction = sample_chain(chain, 1.0)
    end_ring = rings[-1][0]
    end_vertex = bm.verts.new(hand_center + hand_direction * (hand_radius * 0.8))
    hand_weight = arm_weight_for_parameter(side, 1.0)
    weights.append(dict(hand_weight))
    for index in range(ARM_RING_SEGMENTS):
        next_index = (index + 1) % ARM_RING_SEGMENTS
        face = bm.faces.new((end_ring[index], end_ring[next_index], end_vertex))
        face.loops[0][uv_layer].uv = black_uv(0.6, 0.7)
        face.loops[1][uv_layer].uv = black_uv(0.7, 0.7)
        face.loops[2][uv_layer].uv = black_uv(0.65, 0.8)
        face.smooth = True

    for claw_direction in claw_directions:
        claw_base = hand_center + Vector(claw_direction).normalized() * (hand_radius * 0.5)
        add_claw(bm, uv_layer, weights, claw_base, claw_direction, hand_weight)

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
    return mesh, weights


def body_radius_at(z):
    """Interpolates the lathe profile radius at the given height."""
    if z <= BODY_PROFILE[0][0]:
        return BODY_PROFILE[0][1]
    for index in range(len(BODY_PROFILE) - 1):
        z0, r0 = BODY_PROFILE[index]
        z1, r1 = BODY_PROFILE[index + 1]
        if z <= z1:
            return lerp(r0, r1, (z - z0) / (z1 - z0))
    # Above the last ring the head cap tapers linearly to the apex.
    z_last, r_last = BODY_PROFILE[-1]
    if z >= BODY_TOP_Z:
        return 0.0
    return r_last * (BODY_TOP_Z - z) / (BODY_TOP_Z - z_last)


def build_face_mesh():
    """Curved decal grid wrapped around the head front (-Y side)."""
    mesh = bpy.data.meshes.new("Hoshigirl_Face")
    bm = bmesh.new()
    uv_layer = bm.loops.layers.uv.new("UV")
    weights = []

    half_angle = math.radians(FACE_HALF_ANGLE_DEG)
    grid = []
    for row in range(FACE_GRID_Z + 1):
        v_ratio = row / float(FACE_GRID_Z)
        z = FACE_CENTER_Z - FACE_HALF_HEIGHT + 2.0 * FACE_HALF_HEIGHT * v_ratio
        radius = body_radius_at(z) + FACE_SURFACE_OFFSET
        row_vertices = []
        for column in range(FACE_GRID_X + 1):
            u_ratio = column / float(FACE_GRID_X)
            angle = -half_angle + 2.0 * half_angle * u_ratio
            x = radius * math.sin(angle)
            y = -radius * math.cos(angle)
            vertex = bm.verts.new((x, y, z))
            weights.append({BONE_HEAD: 1.0})
            row_vertices.append(vertex)
        grid.append(row_vertices)

    for row in range(FACE_GRID_Z):
        for column in range(FACE_GRID_X):
            v0 = row / float(FACE_GRID_Z)
            v1 = (row + 1) / float(FACE_GRID_Z)
            u0 = column / float(FACE_GRID_X)
            u1 = (column + 1) / float(FACE_GRID_X)
            face = bm.faces.new((
                grid[row][column], grid[row][column + 1],
                grid[row + 1][column + 1], grid[row + 1][column],
            ))
            # Mirror U so the face reads correctly when viewed from -Y.
            face.loops[0][uv_layer].uv = (
                lerp(FACE_U_MAX, FACE_U_MIN, u0), lerp(FACE_V_MIN, FACE_V_MAX, v0))
            face.loops[1][uv_layer].uv = (
                lerp(FACE_U_MAX, FACE_U_MIN, u1), lerp(FACE_V_MIN, FACE_V_MAX, v0))
            face.loops[2][uv_layer].uv = (
                lerp(FACE_U_MAX, FACE_U_MIN, u1), lerp(FACE_V_MIN, FACE_V_MAX, v1))
            face.loops[3][uv_layer].uv = (
                lerp(FACE_U_MAX, FACE_U_MIN, u0), lerp(FACE_V_MIN, FACE_V_MAX, v1))
            face.smooth = True

    bm.normal_update()
    bm.to_mesh(mesh)
    bm.free()
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
    root.head = (0.0, 0.0, 0.40)
    root.tail = (0.0, 0.0, 0.55)

    spine = edit_bones.new(BONE_SPINE)
    spine.head = (0.0, 0.0, 0.55)
    spine.tail = (0.0, 0.0, 1.00)
    spine.parent = root
    spine.use_connect = True

    head = edit_bones.new(BONE_HEAD)
    head.head = (0.0, 0.0, 1.00)
    head.tail = (0.0, 0.0, 1.45)
    head.parent = spine
    head.use_connect = True

    for chain, upper_name, lower_name, hand_name in (
        (LEFT_ARM_CHAIN, BONE_L_UPPER_ARM, BONE_L_LOWER_ARM, BONE_L_HAND),
        (RIGHT_ARM_CHAIN, BONE_R_UPPER_ARM, BONE_R_LOWER_ARM, BONE_R_HAND),
    ):
        shoulder = Vector(chain[0][:3])
        elbow = Vector(chain[2][:3])
        wrist = Vector(chain[3][:3])
        tip = Vector(chain[4][:3])

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
    if len(weights) != len(mesh.vertices):
        raise RuntimeError(
            "Weight count mismatch for {}: {} weights, {} vertices".format(
                name, len(weights), len(mesh.vertices))
        )
    for vertex_index, weight_map in enumerate(weights):
        for bone_name, weight_value in weight_map.items():
            obj.vertex_groups[bone_name].add([vertex_index], weight_value, "REPLACE")
    return obj


# --------------------------------------------------------------------------
# Texture + material
# --------------------------------------------------------------------------

def generate_texture(path):
    size = TEXTURE_SIZE
    base = BODY_DARK_VALUE
    pixels = [base, base, base, 1.0] * (size * size)

    def write_pixel(x, y, value):
        if 0 <= x < size and 0 <= y < size:
            index = (y * size + x) * 4
            if pixels[index] < value:
                pixels[index] = value
                pixels[index + 1] = value
                pixels[index + 2] = value

    def draw_plus(center_x, center_y, arm, thickness, value):
        for delta in range(-arm, arm + 1):
            for spread in range(-thickness, thickness + 1):
                write_pixel(center_x + delta, center_y + spread, value)
                write_pixel(center_x + spread, center_y + delta, value)

    def draw_arc(center_x, center_y, radius, start_deg, end_deg, thickness, value):
        for tenth_degree in range(start_deg * 10, end_deg * 10 + 1):
            rad = math.radians(tenth_degree / 10.0)
            px = int(round(center_x + radius * math.cos(rad)))
            py = int(round(center_y + radius * math.sin(rad)))
            for dx in range(-thickness, thickness + 1):
                for dy in range(-thickness, thickness + 1):
                    if dx * dx + dy * dy <= thickness * thickness:
                        write_pixel(px + dx, py + dy, value)

    face_left = int(FACE_U_MIN * size)
    face_right = int(FACE_U_MAX * size)
    face_bottom = int(FACE_V_MIN * size)
    face_top = int(FACE_V_MAX * size)
    face_width = face_right - face_left
    face_height = face_top - face_bottom
    center_x = (face_left + face_right) // 2

    eye_y = face_top - int(face_height * 0.32)
    eye_offset = int(face_width * 0.17)
    # Soft halo first, then the bright core, so the markings glow slightly.
    draw_plus(center_x - eye_offset, eye_y, 19, 7, 0.30)
    draw_plus(center_x + eye_offset, eye_y, 19, 7, 0.30)
    draw_plus(center_x - eye_offset, eye_y, 16, 4, 1.0)
    draw_plus(center_x + eye_offset, eye_y, 16, 4, 1.0)

    mouth_y = face_bottom + int(face_height * 0.30)
    mouth_radius = int(face_width * 0.24)
    draw_arc(center_x, mouth_y, mouth_radius, 205, 335, 8, 0.30)
    draw_arc(center_x, mouth_y, mouth_radius, 205, 335, 5, 1.0)

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
    principled.inputs["Roughness"].default_value = 0.45
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
    material["_x_specular"] = (0.15, 0.15, 0.15)
    material["_x_emissive"] = (0.0, 0.0, 0.0)
    material["_x_texture_filename"] = TEXTURE_FILENAME
    return material


# --------------------------------------------------------------------------
# Animation (idle only)
# --------------------------------------------------------------------------

def create_idle_action(armature_obj):
    for pose_bone in armature_obj.pose.bones:
        pose_bone.rotation_mode = "XYZ"

    action = bpy.data.actions.new("idle")
    action.use_fake_user = True
    armature_obj.animation_data_create().action = action

    def key_rotation(bone_name, rotation_degrees, frame):
        pose_bone = armature_obj.pose.bones[bone_name]
        pose_bone.rotation_euler = (
            math.radians(rotation_degrees[0]),
            math.radians(rotation_degrees[1]),
            math.radians(rotation_degrees[2]),
        )
        pose_bone.keyframe_insert(data_path="rotation_euler", frame=frame)

    def key_location(bone_name, location, frame):
        pose_bone = armature_obj.pose.bones[bone_name]
        pose_bone.location = location
        pose_bone.keyframe_insert(data_path="location", frame=frame)

    # 60 frames at 30 fps = a 2 second floating loop.
    # Pose bone locations are in bone space: local +Y runs along the bone,
    # and the Root bone points world-up, so +Y lifts the whole body.
    for frame, phase in ((1, 0.0), (30, 1.0), (60, 0.0)):
        lift = 0.05 * phase
        key_location(BONE_ROOT, (0.0, lift, 0.0), frame)
        key_rotation(BONE_SPINE, (-1.5 + 3.0 * phase, 0.0, 1.0 - 2.0 * phase), frame)
        key_rotation(BONE_HEAD, (2.0 - 4.0 * phase, 0.0, 0.0), frame)
        key_rotation(BONE_R_UPPER_ARM, (0.0, 0.0, -4.0 + 8.0 * phase), frame)
        key_rotation(BONE_L_UPPER_ARM, (3.0 - 6.0 * phase, 0.0, 0.0), frame)
        key_rotation(BONE_L_HAND, (4.0 - 8.0 * phase, 0.0, 0.0), frame)

    for fcurve in action.fcurves:
        for keyframe in fcurve.keyframe_points:
            keyframe.interpolation = "SINE"

    armature_obj.animation_data.action = None
    bpy.context.scene.render.fps = 30
    return action


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
    face_mesh, face_weights = build_face_mesh()

    armature_obj = create_armature()

    mesh_objects = (
        link_mesh("Hoshigirl_Body", body_mesh, body_weights, armature_obj),
        link_mesh("Hoshigirl_Arm_L", left_mesh, left_weights, armature_obj),
        link_mesh("Hoshigirl_Arm_R", right_mesh, right_weights, armature_obj),
        link_mesh("Hoshigirl_Face", face_mesh, face_weights, armature_obj),
    )
    for obj in mesh_objects:
        obj.data.materials.clear()
        obj.data.materials.append(material)

    create_idle_action(armature_obj)

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
