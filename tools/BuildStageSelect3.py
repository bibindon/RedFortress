import math
import os
import random
import sys

import bmesh
import bpy
from mathutils import Vector

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import EnhanceStageSelect2 as common


COLLECTION_NAME = "RF3_MoonlessMountain"

PORTALS = [
    ("select2", -16.0, -10.0, 0.7),
    ("3-1", -11.0, -8.0, 0.9),
    ("3-2", -5.0, -3.0, 1.7),
    ("3-3", 4.0, -1.0, 2.5),
    ("3-4", 11.0, 3.0, 3.3),
    ("3-5", 7.0, 8.0, 4.3),
    ("3-6", 0.0, 10.5, 5.1),
    ("3-7", -7.0, 14.0, 6.1),
    ("3-8", 0.0, 17.5, 7.4),
    ("select4", 15.0, -8.0, 1.1),
    ("base", -19.0, -4.0, 1.1),
]


def get_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected output .x and preview paths.")
    arguments = sys.argv[sys.argv.index("--") + 1:]
    if len(arguments) != 2:
        raise RuntimeError("Expected: <output-x-path> <preview-png-path>")
    return os.path.abspath(arguments[0]), os.path.abspath(arguments[1])


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in list(bpy.data.collections):
        if collection.name != "Collection":
            bpy.data.collections.remove(collection)
    root_collection = bpy.data.collections.get("Collection")
    if root_collection is not None:
        root_collection.name = COLLECTION_NAME
    else:
        root_collection = bpy.data.collections.new(COLLECTION_NAME)
        bpy.context.scene.collection.children.link(root_collection)
    return root_collection


def create_material(name, texture_path, base_color, emission_strength=0.0):
    material = bpy.data.materials.get(name)
    if material is None:
        material = bpy.data.materials.new(name)
    material["rf2_texture"] = texture_path
    material.diffuse_color = base_color
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()
    output_node = nodes.new("ShaderNodeOutputMaterial")
    shader_node = nodes.new("ShaderNodeBsdfPrincipled")
    texture_node = nodes.new("ShaderNodeTexImage")
    image_path = os.path.abspath(os.path.join(os.path.dirname(bpy.data.filepath), texture_path))
    texture_node.image = bpy.data.images.load(image_path, check_existing=True)
    links.new(texture_node.outputs["Color"], shader_node.inputs["Base Color"])
    if "Roughness" in shader_node.inputs:
        shader_node.inputs["Roughness"].default_value = 0.92
    if emission_strength > 0.0:
        if "Emission Color" in shader_node.inputs:
            links.new(texture_node.outputs["Color"], shader_node.inputs["Emission Color"])
        if "Emission Strength" in shader_node.inputs:
            shader_node.inputs["Emission Strength"].default_value = emission_strength
    links.new(shader_node.outputs["BSDF"], output_node.inputs["Surface"])
    return material


def create_polygon_prism(collection, name, boundary, bottom_height, top_height, material):
    vertex_count = len(boundary)
    vertices = [(x, y, bottom_height) for x, y in boundary]
    vertices.extend((x, y, top_height) for x, y in boundary)
    faces = []
    faces.append(tuple(range(vertex_count, vertex_count * 2)))
    faces.append(tuple(reversed(range(vertex_count))))
    for index in range(vertex_count):
        next_index = (index + 1) % vertex_count
        faces.append((index, next_index, next_index + vertex_count, index + vertex_count))
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    return common.create_mesh_object(collection, name, mesh, material)


def create_sloped_patch(collection, name, boundary, bottom_height, material):
    center_x = sum(vertex[0] for vertex in boundary) / len(boundary)
    center_y = sum(vertex[1] for vertex in boundary) / len(boundary)
    center_z = sum(vertex[2] for vertex in boundary) / len(boundary) + 0.18
    vertices = list(boundary)
    center_index = len(vertices)
    vertices.append((center_x, center_y, center_z))
    bottom_start = len(vertices)
    vertices.extend((x, y, bottom_height) for x, y, z in boundary)
    faces = []
    for index in range(len(boundary)):
        next_index = (index + 1) % len(boundary)
        faces.append((center_index, index, next_index))
        faces.append((index, bottom_start + index, bottom_start + next_index, next_index))
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    return common.create_mesh_object(collection, name, mesh, material)


def create_path_ribbon(collection, name, points, half_width, material):
    vertices = []
    faces = []
    vectors = [Vector((x, y, z)) for x, y, z in points]
    for index, point in enumerate(vectors):
        if index == 0:
            tangent = vectors[1] - point
        elif index + 1 == len(vectors):
            tangent = point - vectors[index - 1]
        else:
            tangent = vectors[index + 1] - vectors[index - 1]
        tangent.z = 0.0
        tangent.normalize()
        perpendicular = Vector((-tangent.y, tangent.x, 0.0))
        vertices.append(tuple(point + perpendicular * half_width))
        vertices.append(tuple(point - perpendicular * half_width))
    for index in range(len(vectors) - 1):
        base = index * 2
        faces.append((base, base + 1, base + 3, base + 2))
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    return common.create_mesh_object(collection, name, mesh, material)


def create_stair_run(collection, prefix, start, end, material):
    start_vector = Vector(start)
    end_vector = Vector(end)
    horizontal = end_vector - start_vector
    horizontal.z = 0.0
    distance = horizontal.length
    step_count = max(2, int(distance / 2.1))
    direction = horizontal.normalized()
    rotation = math.atan2(direction.y, direction.x)
    for index in range(step_count):
        amount = (index + 0.5) / step_count
        location = start_vector.lerp(end_vector, amount)
        common.create_box(
            collection,
            prefix + "%02d" % index,
            (location.x, location.y, location.z - 0.05),
            (distance / step_count * 0.58, 1.12, 0.10),
            rotation,
            material,
        )


def create_torii(collection, prefix, center, height, rotation, shrine_material, dark_material):
    direction = Vector((math.cos(rotation), math.sin(rotation)))
    side = Vector((-direction.y, direction.x))
    for index, sign in enumerate((-1.0, 1.0)):
        position = Vector(center) + side * 1.65 * sign
        common.create_box(
            collection,
            prefix + "Post" + str(index),
            (position.x, position.y, height + 1.65),
            (0.30, 0.36, 1.65),
            rotation,
            shrine_material,
        )
        common.create_rock(
            collection,
            prefix + "Foot" + str(index),
            (position.x, position.y, height + 0.20),
            (0.55, 0.55, 0.28),
            rotation,
            dark_material,
            170 + index,
        )
    common.create_box(
        collection,
        prefix + "Lintel",
        (center[0], center[1], height + 3.25),
        (2.25, 0.38, 0.28),
        rotation,
        shrine_material,
    )
    common.create_box(
        collection,
        prefix + "Cap",
        (center[0], center[1], height + 3.62),
        (2.65, 0.28, 0.14),
        rotation,
        dark_material,
    )


def create_lantern(collection, prefix, location, stone_material, glow_material):
    x, y, z = location
    common.create_cylinder(collection, prefix + "Base", (x, y, z + 0.15), 0.38, 0.30, 8, stone_material)
    common.create_box(collection, prefix + "Stem", (x, y, z + 0.82), (0.18, 0.18, 0.62), 0.0, stone_material)
    common.create_box(collection, prefix + "Light", (x, y, z + 1.48), (0.34, 0.34, 0.38), 0.0, glow_material)
    common.create_box(collection, prefix + "Roof", (x, y, z + 1.91), (0.55, 0.55, 0.12), math.radians(45.0), stone_material)


def create_pine(collection, prefix, location, scale, trunk_material, foliage_material):
    x, y, z = location
    common.create_cylinder(collection, prefix + "Trunk", (x, y, z + scale * 1.1), 0.18 * scale, 2.2 * scale, 7, trunk_material)
    for index in range(3):
        height = scale * (1.55 + index * 0.62)
        radius = scale * (1.25 - index * 0.22)
        common.create_crystal(
            collection,
            prefix + "Crown" + str(index),
            (x, y, z + scale * (2.0 + index * 0.56)),
            radius,
            height,
            (0.0, 0.0, index * 0.7),
            foliage_material,
        )


def create_night_sky(collection, material):
    old_textures = dict(common.TEXTURES)
    common.TEXTURES["RF2_CaveSky"] = "../SkySphere_night/Skydome.png"
    common.create_cave_sky(collection, material)
    common.TEXTURES.clear()
    common.TEXTURES.update(old_textures)
    sky_object = bpy.data.objects.get("RF2_CaveSky")
    if sky_object is not None:
        sky_object.name = "RF3_MoonlessSky"


def create_star_field(collection, material):
    random_generator = random.Random(31027)
    vertices = []
    faces = []
    camera_position = Vector((0.0, -38.0, 23.0))
    camera_target = Vector((0.0, 6.0, 4.2))
    camera_forward = (camera_target - camera_position).normalized()
    camera_right = Vector((1.0, 0.0, 0.0))
    camera_up = camera_right.cross(camera_forward).normalized()
    horizontal_half_tangent = 1.0
    vertical_half_tangent = horizontal_half_tangent * 9.0 / 16.0
    sky_center = Vector((0.0, 0.0, 4.0))
    horizontal_sky_radius = 72.0
    vertical_sky_radius = 38.0
    row_count = 21
    column_count = 38
    star_index = 0
    for row_index in range(row_count):
        for column_index in range(column_count):
            jitter_x = random_generator.uniform(-0.48, 0.48)
            jitter_y = random_generator.uniform(-0.48, 0.48)
            screen_x = ((column_index + 0.5 + jitter_x) / column_count) * 2.0 - 1.0
            screen_y = ((row_index + 0.5 + jitter_y) / row_count) * 2.0 - 1.0
            density = 0.46
            density += 0.13 * math.sin(screen_x * 3.1 + screen_y * 1.7)
            density += 0.10 * math.cos(screen_y * 4.3 - screen_x * 1.2)
            cluster_offset_x = screen_x + 0.58
            cluster_offset_y = screen_y - 0.24
            density += 0.20 * math.exp(
                -(cluster_offset_x * cluster_offset_x + cluster_offset_y * cluster_offset_y) / 0.10
            )
            cluster_offset_x = screen_x - 0.52
            cluster_offset_y = screen_y + 0.42
            density += 0.17 * math.exp(
                -(cluster_offset_x * cluster_offset_x + cluster_offset_y * cluster_offset_y) / 0.08
            )
            density = max(0.22, min(density, 0.76))
            if random_generator.random() > density:
                continue
            ray_direction = (
                camera_forward +
                camera_right * (screen_x * horizontal_half_tangent) +
                camera_up * (screen_y * vertical_half_tangent)
            ).normalized()
            relative_camera = camera_position - sky_center
            ray_a = (
                (ray_direction.x * ray_direction.x + ray_direction.y * ray_direction.y) /
                (horizontal_sky_radius * horizontal_sky_radius) +
                ray_direction.z * ray_direction.z / (vertical_sky_radius * vertical_sky_radius)
            )
            ray_b = 2.0 * (
                (relative_camera.x * ray_direction.x + relative_camera.y * ray_direction.y) /
                (horizontal_sky_radius * horizontal_sky_radius) +
                relative_camera.z * ray_direction.z / (vertical_sky_radius * vertical_sky_radius)
            )
            ray_c = (
                (relative_camera.x * relative_camera.x + relative_camera.y * relative_camera.y) /
                (horizontal_sky_radius * horizontal_sky_radius) +
                relative_camera.z * relative_camera.z / (vertical_sky_radius * vertical_sky_radius) - 1.0
            )
            discriminant = ray_b * ray_b - 4.0 * ray_a * ray_c
            if discriminant <= 0.0:
                raise RuntimeError("Stage-select 3 star ray did not intersect the sky sphere.")
            distance = (-ray_b + math.sqrt(discriminant)) / (2.0 * ray_a)
            star_position = camera_position + ray_direction * (distance - 0.8)
            x = star_position.x
            y = star_position.y
            z = star_position.z
            size_roll = random_generator.random()
            if size_roll < 0.70:
                radius = distance * random_generator.uniform(0.00075, 0.00110)
            elif size_roll < 0.95:
                radius = distance * random_generator.uniform(0.00115, 0.00165)
            else:
                radius = distance * random_generator.uniform(0.0019, 0.0023)
            vertical_radius = radius * random_generator.uniform(1.05, 1.55)
            base_index = len(vertices)
            vertices.extend([
                (x, y, z + vertical_radius),
                (x, y, z - vertical_radius),
                (x + radius, y, z),
                (x - radius, y, z),
                (x, y + radius, z),
                (x, y - radius, z),
            ])
            faces.extend([
                (base_index, base_index + 2, base_index + 4),
                (base_index, base_index + 4, base_index + 3),
                (base_index, base_index + 3, base_index + 5),
                (base_index, base_index + 5, base_index + 2),
                (base_index + 1, base_index + 4, base_index + 2),
                (base_index + 1, base_index + 3, base_index + 4),
                (base_index + 1, base_index + 5, base_index + 3),
                (base_index + 1, base_index + 2, base_index + 5),
            ])
            star_index += 1
    mesh = bpy.data.meshes.new("RF3_StarFieldMesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    star_object = common.create_mesh_object(collection, "RF3_StarField", mesh, material)
    if hasattr(star_object, "visible_shadow"):
        star_object.visible_shadow = False


def build_scene(collection, materials):
    create_night_sky(collection, materials["sky"])
    create_star_field(collection, materials["star"])

    create_polygon_prism(
        collection,
        "RF3_LowerMountain",
        [(-25.0, -15.0), (25.0, -15.0), (24.0, 8.0), (16.0, 15.0), (-18.0, 16.0), (-25.0, 7.0)],
        -2.8,
        0.0,
        materials["dark"],
    )
    create_sloped_patch(
        collection,
        "RF3_LeftFoothill",
        [
            (-42.0, -10.0, -2.2),
            (-38.0, 6.0, -1.2),
            (-26.0, 9.0, 0.0),
            (-24.0, -8.0, 0.15),
            (-15.0, -16.0, -0.25),
            (-18.0, -23.0, -1.3),
            (-34.0, -24.0, -2.0),
        ],
        -3.2,
        materials["rock"],
    )
    create_sloped_patch(
        collection,
        "RF3_RightFoothill",
        [
            (42.0, -10.0, -2.2),
            (38.0, 6.0, -1.2),
            (26.0, 9.0, 0.0),
            (24.0, -8.0, 0.15),
            (15.0, -16.0, -0.25),
            (18.0, -23.0, -1.3),
            (34.0, -24.0, -2.0),
        ],
        -3.2,
        materials["rock"],
    )
    create_polygon_prism(
        collection,
        "RF3_MiddleTerrace",
        [(-17.0, -2.0), (16.0, -1.0), (18.0, 10.0), (10.0, 16.0), (-13.0, 18.0), (-18.0, 9.0)],
        -0.2,
        2.25,
        materials["rock"],
    )
    create_polygon_prism(
        collection,
        "RF3_UpperTerrace",
        [(-12.5, 8.0), (12.5, 8.0), (13.0, 20.5), (8.0, 25.0), (-9.0, 25.0), (-14.0, 18.0)],
        1.9,
        4.65,
        materials["dark"],
    )

    main_route = [(x, y, z + 0.05) for _, x, y, z in PORTALS[0:9]]
    create_path_ribbon(collection, "RF3_PilgrimTrail", main_route, 1.22, materials["path"])
    for index in range(len(main_route) - 1):
        create_stair_run(
            collection,
            "RF3_Step_%02d_" % index,
            main_route[index],
            main_route[index + 1],
            materials["shrine"],
        )

    for index, (destination, x, y, z) in enumerate(PORTALS):
        ring_material = materials["shrine"]
        if destination in ("select2", "select4", "base"):
            ring_material = materials["blue"]
        common.create_cylinder(collection, "RF3_PortalPlinth_%02d" % index, (x, y, z - 0.20), 1.42, 0.40, 14, materials["rock"])
        common.create_cylinder(collection, "RF3_PortalDisk_%02d" % index, (x, y, z + 0.02), 1.02, 0.10, 14, materials["dark"])
        common.create_torus(collection, "RF3_PortalRing_%02d" % index, (x, y, z + 0.13), 1.12, 0.09, ring_material)

    create_polygon_prism(
        collection,
        "RF3_SummitAltar",
        [(-5.8, 15.0), (-3.0, 12.8), (3.2, 12.8), (6.0, 15.2), (6.0, 22.0), (3.0, 24.5), (-3.2, 24.5), (-6.0, 22.0)],
        4.4,
        7.05,
        materials["shrine"],
    )
    common.create_cylinder(collection, "RF3_SealOuter", (0.0, 21.0, 7.18), 3.55, 0.26, 20, materials["dark"])
    common.create_torus(collection, "RF3_SealGoldRing", (0.0, 21.0, 7.37), 2.75, 0.13, materials["gold"])
    common.create_cylinder(collection, "RF3_SealCenter", (0.0, 21.0, 7.31), 1.55, 0.22, 18, materials["gold"])
    common.create_box(collection, "RF3_GoldenCrossVertical", (0.0, 21.0, 8.35), (0.24, 0.24, 1.05), 0.0, materials["gold"])
    common.create_box(collection, "RF3_GoldenCrossHorizontal", (0.0, 21.0, 8.62), (0.85, 0.24, 0.22), 0.0, materials["gold"])

    for index in range(8):
        angle = math.tau * index / 8.0
        radius = 4.65
        height = 2.6
        if index in (2, 5):
            height = 1.45
        x = math.cos(angle) * radius
        y = 21.0 + math.sin(angle) * radius
        common.create_box(
            collection,
            "RF3_AltarPillar_%02d" % index,
            (x, y, 7.05 + height * 0.5),
            (0.42, 0.42, height * 0.5),
            angle + 0.12 * math.sin(index),
            materials["shrine"],
        )

    create_torii(collection, "RF3_LowerTorii_", (-7.5, -5.0), 1.25, math.radians(40.0), materials["shrine"], materials["dark"])
    create_torii(collection, "RF3_UpperTorii_", (4.0, 9.3), 4.25, math.radians(160.0), materials["shrine"], materials["dark"])

    lanterns = [
        (-8.5, -4.0, 1.3),
        (1.5, -1.4, 2.2),
        (9.2, 4.8, 3.5),
        (3.8, 9.5, 4.7),
        (-5.0, 13.2, 5.7),
        (-3.8, 19.2, 7.1),
        (3.8, 19.2, 7.1),
    ]
    for index, location in enumerate(lanterns):
        create_lantern(collection, "RF3_Lantern_%02d_" % index, location, materials["shrine"], materials["white"])

    pines = [
        (-21.0, -9.0, 0.0, 1.3),
        (-32.0, -7.0, -0.8, 1.8),
        (32.0, -7.0, -0.8, 1.7),
        (-21.0, 4.0, 0.0, 1.7),
        (20.0, 2.0, 0.0, 1.5),
        (17.0, 12.0, 2.3, 1.2),
        (-14.0, 20.0, 4.7, 1.4),
        (12.0, 22.0, 4.7, 1.1),
    ]
    for index, (x, y, z, scale) in enumerate(pines):
        create_pine(collection, "RF3_Pine_%02d_" % index, (x, y, z), scale, materials["path"], materials["foliage"])

    cliff_rocks = [
        (-23.0, 17.0, 5.0, 4.5, 4.0, 7.0),
        (-17.0, 25.0, 6.0, 5.0, 4.0, 8.0),
        (-9.0, 28.0, 7.0, 5.5, 4.0, 9.0),
        (8.0, 29.0, 7.2, 5.5, 4.0, 9.0),
        (17.0, 25.0, 6.2, 5.0, 4.0, 8.0),
        (23.0, 17.0, 5.0, 4.5, 4.0, 7.0),
        (-25.0, -2.0, 3.0, 3.5, 4.0, 5.0),
        (25.0, -2.0, 3.0, 3.5, 4.0, 5.0),
        (-33.0, -13.0, -0.7, 4.5, 3.4, 3.1),
        (33.0, -13.0, -0.7, 4.5, 3.4, 3.1),
        (-29.0, 1.0, 0.0, 3.6, 3.0, 3.8),
        (29.0, 1.0, 0.0, 3.6, 3.0, 3.8),
    ]
    for index, (x, y, z, sx, sy, sz) in enumerate(cliff_rocks):
        common.create_rock(collection, "RF3_Cliff_%02d" % index, (x, y, z), (sx, sy, sz), index * 0.43, materials["rock"], 210 + index)

    create_polygon_prism(
        collection,
        "RF3_River",
        [(11.5, -15.0), (17.0, -15.0), (18.5, -3.0), (14.0, 0.0), (12.3, -4.0)],
        0.02,
        0.08,
        materials["blue"],
    )
    create_path_ribbon(
        collection,
        "RF3_ForegroundRiver",
        [
            (-17.0, -16.0, 0.10),
            (-10.0, -13.0, 0.11),
            (-2.0, -14.0, 0.12),
            (4.0, -11.0, 0.13),
            (3.0, -8.0, 0.14),
            (9.0, -5.5, 0.15),
            (14.5, -2.0, 0.15),
        ],
        1.85,
        materials["river"],
    )
    foreground_rocks = [
        (-22.5, -13.5, 0.8, 4.0, 2.6, 1.8),
        (-16.0, -12.0, 0.6, 3.3, 2.0, 1.4),
        (-9.0, -14.0, 0.5, 2.8, 2.1, 1.2),
        (8.5, -14.5, 0.5, 2.6, 2.0, 1.1),
        (16.5, -12.5, 0.7, 3.4, 2.3, 1.5),
        (23.0, -13.5, 0.9, 4.2, 2.8, 2.0),
    ]
    for index, (x, y, z, sx, sy, sz) in enumerate(foreground_rocks):
        common.create_rock(
            collection,
            "RF3_ForegroundCliff_%02d" % index,
            (x, y, z),
            (sx, sy, sz),
            index * 0.63,
            materials["dark"],
            320 + index,
        )
    foreground_pines = [(-18.5, -11.5, 0.3, 1.15), (19.5, -10.5, 0.3, 1.25)]
    for index, (x, y, z, scale) in enumerate(foreground_pines):
        create_pine(
            collection,
            "RF3_ForegroundPine_%02d_" % index,
            (x, y, z),
            scale,
            materials["path"],
            materials["foliage"],
        )
    common.create_box(collection, "RF3_BrokenBridgeA", (11.6, -5.0, 1.0), (2.6, 0.55, 0.14), math.radians(18.0), materials["path"])
    common.create_box(collection, "RF3_BrokenBridgeB", (17.7, -3.8, 1.25), (2.0, 0.55, 0.14), math.radians(-22.0), materials["path"])

    spirit_locations = [(-5.0, -11.5, 2.2), (6.0, -9.0, 1.9), (-2.0, 5.0, 4.4), (12.5, 13.5, 7.0), (-10.0, 21.0, 8.5)]
    for index, location in enumerate(spirit_locations):
        common.create_rock(collection, "RF3_WhiteWisp_%02d" % index, location, (0.35, 0.35, 0.55), index * 0.7, materials["white"], 260 + index)
    mist_locations = [(2.0, 4.5, 2.6), (-4.5, 11.0, 5.2), (4.0, 16.0, 7.6), (0.0, 23.0, 8.2)]
    for index, location in enumerate(mist_locations):
        common.create_rock(collection, "RF3_BlackMist_%02d" % index, location, (1.6, 1.0, 0.45), index * 0.9, materials["dark"], 280 + index)


def add_preview_light(name, light_type, location, energy, color, size):
    light = common.add_preview_light(name, light_type, location, energy, color, size)
    return light


def render_preview(preview_path):
    camera_data = bpy.data.cameras.new("RF3_PreviewCameraData")
    camera = bpy.data.objects.new("RF3_PreviewCamera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    camera.location = (0.0, -38.0, 23.0)
    camera.data.lens = 18.0
    common.aim_object(camera, (0.0, 6.0, 4.2))
    bpy.context.scene.camera = camera

    moon_light = add_preview_light("RF3_Starlight", "AREA", (-8.0, -6.0, 25.0), 2400.0, (0.18, 0.30, 0.72), 14.0)
    common.aim_object(moon_light, (0.0, 8.0, 3.0))
    altar_light = add_preview_light("RF3_AltarLight", "POINT", (0.0, 21.0, 10.2), 900.0, (1.0, 0.42, 0.08), 4.0)
    altar_light.data.shadow_soft_size = 3.0
    fill_light = add_preview_light("RF3_Fill", "AREA", (17.0, -12.0, 12.0), 1500.0, (0.12, 0.28, 0.58), 11.0)
    common.aim_object(fill_light, (4.0, 4.0, 2.0))

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1200
    scene.render.resolution_y = 675
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = preview_path
    scene.render.film_transparent = False
    scene.world.color = (0.002, 0.004, 0.012)
    os.makedirs(os.path.dirname(preview_path), exist_ok=True)
    bpy.ops.render.render(write_still=True)


def main():
    output_x_path, preview_path = get_arguments()
    collection = clear_scene()
    materials = {
        "sky": create_material("RF2_CaveSky_World3Night", "../SkySphere_night/Skydome.png", (0.025, 0.035, 0.075, 1.0), 0.45),
        "star": create_material("RF3_StarWhite", "../cube_white.png", (0.90, 0.96, 1.0, 1.0), 2.4),
        "rock": create_material("RF3_MountainStone", "../stage-select2/stageSelectCaveWall.png", (0.22, 0.25, 0.32, 1.0)),
        "dark": create_material("RF3_ObsidianStone", "../stage-select2/stageSelectCaveDeepDark.png", (0.035, 0.04, 0.08, 1.0)),
        "path": create_material("RF3_PilgrimPath", "../stage-select2/stageSelectCavePath.png", (0.24, 0.19, 0.15, 1.0)),
        "shrine": create_material("RF3_AncientShrine", "../stage-select2/stageSelectCaveShrine.png", (0.34, 0.34, 0.37, 1.0)),
        "foliage": create_material("RF3_NightFoliage", "../stage-select2/stageSelectCaveMoss.png", (0.025, 0.08, 0.07, 1.0)),
        "gold": create_material("RF3_SealGold", "../sphereOrange/sphere_orange.png", (1.0, 0.42, 0.04, 1.0), 1.8),
        "white": create_material("RF3_SpiritWhite", "../cube_white.png", (0.72, 0.90, 1.0, 1.0), 1.1),
        "blue": create_material("RF3_SpiritBlue", "../cubeBlue/cube_blue.png", (0.05, 0.28, 1.0, 1.0), 0.7),
        "river": create_material("RF2_CaveSky_World3River", "../cubeBlue/cube_blue.png", (0.025, 0.12, 0.42, 1.0), 0.65),
    }
    build_scene(collection, materials)
    star_object = bpy.data.objects.get("RF3_StarField")
    if star_object is None:
        raise RuntimeError("RF3_StarField was not created.")
    star_object["rf2_export"] = False
    common.export_directx_meshes(bpy.context.scene.collection, output_x_path)
    for scene_object in bpy.context.scene.collection.all_objects:
        if scene_object.type == "MESH":
            scene_object["rf2_export"] = False
    star_object["rf2_export"] = True
    star_output_path = os.path.join(os.path.dirname(output_x_path), "stageSelectMoonMountainStars.x")
    common.export_directx_meshes(bpy.context.scene.collection, star_output_path)
    for scene_object in bpy.context.scene.collection.all_objects:
        if scene_object.type == "MESH":
            scene_object["rf2_export"] = True
    render_preview(preview_path)
    bpy.ops.wm.save_as_mainfile(filepath=bpy.data.filepath)
    print("SAVED_BLEND", bpy.data.filepath)
    print("PREVIEW", preview_path)


if __name__ == "__main__":
    main()
