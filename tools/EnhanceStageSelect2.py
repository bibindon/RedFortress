import bmesh
import bpy
import math
import os
import sys

from mathutils import Matrix, Vector


DETAIL_COLLECTION_NAME = "RF2_StageDetail"
REMOVED_BASE_OBJECTS = {
    "cave_ceiling.001",
    "cave_high_ceiling_extension",
    "warm_entrance_light_panel.001",
}

DUMMY_BASE_PREFIXES = (
    "blue_cave_crystal_",
    "cave_moss_patch_",
    "cave_path_marker_slab_",
    "cave_rock_",
    "cave_stalactite_",
)

PORTALS = [
    (-17.5, 11.0),
    (-14.0, 14.0),
    (-3.0, 15.0),
    (8.0, 12.0),
    (13.0, 7.0),
    (8.0, 3.0),
    (-3.0, 2.0),
    (-10.0, 0.0),
    (-9.0, -5.0),
    (-3.0, -8.0),
    (9.0, -7.0),
]

TEXTURES = {
    "RF2_CaveSky": "../SkySphere_cave/Skydome.png",
    "stageSelectCaveWall": "stageSelectCaveWall.png",
    "stageSelectCaveDeepDark": "stageSelectCaveDeepDark.png",
    "stageSelectCavePath": "stageSelectCavePath.png",
    "stageSelectCaveMoss": "stageSelectCaveMoss.png",
    "stageSelectCaveCrystal": "stageSelectCaveCrystal.png",
    "stageSelectCaveShrine": "stageSelectCaveShrine.png",
    "stageSelectCaveFloor": "stageSelectCaveFloor.png",
}


def get_script_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Output .x path and preview path are required.")
    arguments = sys.argv[sys.argv.index("--") + 1:]
    if len(arguments) != 2:
        raise RuntimeError("Expected: <output-x-path> <preview-png-path>")
    return os.path.abspath(arguments[0]), os.path.abspath(arguments[1])


def remove_collection(name):
    collection = bpy.data.collections.get(name)
    if collection is None:
        return
    for obj in list(collection.all_objects):
        bpy.data.objects.remove(obj, do_unlink=True)
    bpy.data.collections.remove(collection)


def remove_obsolete_base_objects():
    for name in REMOVED_BASE_OBJECTS:
        obj = bpy.data.objects.get(name)
        if obj is not None:
            bpy.data.objects.remove(obj, do_unlink=True)


def remove_dummy_base_objects():
    for obj in list(bpy.data.objects):
        if obj.name.startswith(DUMMY_BASE_PREFIXES):
            bpy.data.objects.remove(obj, do_unlink=True)


def create_detail_collection():
    collection = bpy.data.collections.new(DETAIL_COLLECTION_NAME)
    bpy.context.scene.collection.children.link(collection)
    return collection


def find_material(prefix):
    for material in bpy.data.materials:
        if material.name.startswith(prefix):
            material["rf2_texture"] = TEXTURES[prefix]
            return material

    texture_path = TEXTURES.get(prefix)
    if texture_path is None:
        raise RuntimeError("Required material texture is not registered: " + prefix)

    material = bpy.data.materials.new(prefix)
    material["rf2_texture"] = texture_path
    material.diffuse_color = (1.0, 1.0, 1.0, 1.0)
    material.roughness = 0.85
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
    shader_node.inputs["Roughness"].default_value = 0.85
    links.new(shader_node.outputs["BSDF"], output_node.inputs["Surface"])
    return material


def create_cave_sky_material():
    material = bpy.data.materials.get("RF2_CaveSky")
    if material is None:
        material = bpy.data.materials.new("RF2_CaveSky")
    material["rf2_texture"] = TEXTURES["RF2_CaveSky"]
    material.diffuse_color = (0.12, 0.09, 0.055, 1.0)
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()
    output_node = nodes.new("ShaderNodeOutputMaterial")
    shader_node = nodes.new("ShaderNodeBsdfPrincipled")
    texture_node = nodes.new("ShaderNodeTexImage")
    image_path = os.path.abspath(os.path.join(
        os.path.dirname(bpy.data.filepath),
        TEXTURES["RF2_CaveSky"],
    ))
    texture_node.image = bpy.data.images.load(image_path, check_existing=True)
    links.new(texture_node.outputs["Color"], shader_node.inputs["Base Color"])
    if "Roughness" in shader_node.inputs:
        shader_node.inputs["Roughness"].default_value = 1.0
    if "Emission Color" in shader_node.inputs:
        links.new(texture_node.outputs["Color"], shader_node.inputs["Emission Color"])
    if "Emission Strength" in shader_node.inputs:
        shader_node.inputs["Emission Strength"].default_value = 0.28
    links.new(shader_node.outputs["BSDF"], output_node.inputs["Surface"])
    return material


def create_portal_mountain_material():
    material = bpy.data.materials.get("RF2_PortalMountainBrown")
    if material is None:
        material = bpy.data.materials.new("RF2_PortalMountainBrown")
    material["rf2_texture"] = ""
    material.diffuse_color = (0.54, 0.27, 0.085, 1.0)
    material.roughness = 0.82
    material.metallic = 0.0
    material.use_nodes = True
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    nodes.clear()
    output_node = nodes.new("ShaderNodeOutputMaterial")
    shader_node = nodes.new("ShaderNodeBsdfPrincipled")
    shader_node.inputs["Base Color"].default_value = (0.54, 0.27, 0.085, 1.0)
    shader_node.inputs["Roughness"].default_value = 0.82
    shader_node.inputs["Metallic"].default_value = 0.0
    links.new(shader_node.outputs["BSDF"], output_node.inputs["Surface"])
    return material


def create_mesh_object(collection, name, mesh, material):
    obj = bpy.data.objects.new(name, mesh)
    collection.objects.link(obj)
    obj.data.materials.append(material)
    obj["rf2_export"] = True
    return obj


def ensure_generated_uv_map(mesh):
    if len(mesh.uv_layers) > 0 or len(mesh.vertices) == 0:
        return

    coordinate_ranges = []
    for axis_index in range(3):
        values = [vertex.co[axis_index] for vertex in mesh.vertices]
        minimum = min(values)
        maximum = max(values)
        coordinate_ranges.append((maximum - minimum, axis_index, minimum))

    coordinate_ranges.sort(reverse=True)
    first_range, first_axis, first_minimum = coordinate_ranges[0]
    second_range, second_axis, second_minimum = coordinate_ranges[1]
    if first_range <= 0.000001:
        first_range = 1.0
    if second_range <= 0.000001:
        second_range = 1.0

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex = mesh.vertices[mesh.loops[loop_index].vertex_index]
            u = (vertex.co[first_axis] - first_minimum) / first_range
            v = (vertex.co[second_axis] - second_minimum) / second_range
            uv_layer.data[loop_index].uv = (u, v)


def create_cylinder(collection, name, location, radius, depth, segments, material):
    mesh = bpy.data.meshes.new(name + "Mesh")
    bm = bmesh.new()
    bmesh.ops.create_cone(
        bm,
        cap_ends=True,
        cap_tris=True,
        segments=segments,
        radius1=radius,
        radius2=radius,
        depth=depth,
    )
    bm.to_mesh(mesh)
    bm.free()
    obj = create_mesh_object(collection, name, mesh, material)
    obj.location = location
    return obj


def create_box(collection, name, location, scale, rotation_z, material):
    mesh = bpy.data.meshes.new(name + "Mesh")
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    bm.to_mesh(mesh)
    bm.free()
    obj = create_mesh_object(collection, name, mesh, material)
    obj.location = location
    obj.scale = scale
    obj.rotation_euler.z = rotation_z
    return obj


def create_rock(collection, name, location, scale, rotation_z, material, seed):
    mesh = bpy.data.meshes.new(name + "Mesh")
    bm = bmesh.new()
    bmesh.ops.create_icosphere(bm, subdivisions=1, radius=1.0)
    for index, vertex in enumerate(bm.verts):
        variation = 0.84 + 0.22 * math.sin(seed * 1.73 + index * 2.31)
        vertex.co.x *= scale[0] * variation
        vertex.co.y *= scale[1] * (1.03 - (variation - 0.84) * 0.35)
        vertex.co.z *= scale[2] * (0.94 + 0.10 * math.cos(seed + index * 1.17))
    bm.to_mesh(mesh)
    bm.free()
    obj = create_mesh_object(collection, name, mesh, material)
    obj.location = location
    obj.rotation_euler.z = rotation_z
    return obj


def create_portal_mountain(collection, name, location, material, seed):
    segment_count = 7
    top_height = -0.11
    shoulder_height = -0.30
    base_height = -1.58
    vertices = []

    for segment_index in range(segment_count):
        angle = math.tau * segment_index / segment_count + seed * 0.071
        radius = 1.52 + 0.13 * math.sin(seed * 1.31 + segment_index * 2.17)
        vertices.append((radius * math.cos(angle), radius * math.sin(angle), top_height))

    for segment_index in range(segment_count):
        angle = math.tau * segment_index / segment_count + seed * 0.071 + 0.10
        radius = 2.38 + 0.25 * math.sin(seed * 1.73 + segment_index * 1.91)
        height = shoulder_height + 0.11 * math.cos(seed * 0.83 + segment_index * 2.43)
        vertices.append((radius * math.cos(angle), radius * math.sin(angle), height))

    for segment_index in range(segment_count):
        angle = math.tau * segment_index / segment_count + seed * 0.071 - 0.08
        radius = 3.42 + 0.28 * math.sin(seed * 2.11 + segment_index * 1.47)
        height = base_height + 0.18 * math.cos(seed * 1.19 + segment_index * 2.07)
        vertices.append((radius * math.cos(angle), radius * math.sin(angle), height))

    faces = [tuple(range(segment_count))]
    for segment_index in range(segment_count):
        next_index = (segment_index + 1) % segment_count
        top_a = segment_index
        top_b = next_index
        shoulder_a = segment_count + segment_index
        shoulder_b = segment_count + next_index
        base_a = segment_count * 2 + segment_index
        base_b = segment_count * 2 + next_index
        if segment_index % 2 == 0:
            faces.append((top_a, shoulder_a, shoulder_b))
            faces.append((top_a, shoulder_b, top_b))
            faces.append((shoulder_a, base_a, base_b))
            faces.append((shoulder_a, base_b, shoulder_b))
        else:
            faces.append((top_a, shoulder_a, top_b))
            faces.append((top_b, shoulder_a, shoulder_b))
            faces.append((shoulder_a, base_a, shoulder_b))
            faces.append((shoulder_b, base_a, base_b))

    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    for polygon in mesh.polygons:
        polygon.use_smooth = False
    obj = create_mesh_object(collection, name, mesh, material)
    obj.location = location
    return obj


def cut_portal_mountain_openings():
    terrain = bpy.data.objects.get("RF2_CaveTerrain")
    if terrain is None or terrain.type != "MESH":
        raise RuntimeError("RF2_CaveTerrain mesh was not found.")

    mesh = terrain.data
    bm = bmesh.new()
    bm.from_mesh(mesh)
    faces_to_remove = []
    opening_radius_squared = 2.95 * 2.95
    for face in bm.faces:
        center = face.calc_center_median()
        for portal_x, portal_y in PORTALS:
            difference_x = center.x - portal_x
            difference_y = center.y - portal_y
            if difference_x * difference_x + difference_y * difference_y <= opening_radius_squared:
                faces_to_remove.append(face)
                break

    if faces_to_remove:
        bmesh.ops.delete(bm, geom=faces_to_remove, context="FACES")
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()


def create_crystal(collection, name, location, radius, height, rotation, material):
    mesh = bpy.data.meshes.new(name + "Mesh")
    bm = bmesh.new()
    bmesh.ops.create_cone(
        bm,
        cap_ends=True,
        cap_tris=True,
        segments=6,
        radius1=radius,
        radius2=0.0,
        depth=height,
    )
    bm.to_mesh(mesh)
    bm.free()
    obj = create_mesh_object(collection, name, mesh, material)
    obj.location = location
    obj.rotation_euler = rotation
    return obj


def create_torus(collection, name, location, major_radius, minor_radius, material):
    major_segments = 16
    minor_segments = 4
    vertices = []
    faces = []
    uvs = []
    for major_index in range(major_segments):
        major_angle = math.tau * major_index / major_segments
        for minor_index in range(minor_segments):
            minor_angle = math.tau * minor_index / minor_segments
            radial = major_radius + minor_radius * math.cos(minor_angle)
            vertices.append((
                radial * math.cos(major_angle),
                radial * math.sin(major_angle),
                minor_radius * math.sin(minor_angle),
            ))
            uvs.append((major_index / major_segments, minor_index / minor_segments))
    for major_index in range(major_segments):
        next_major = (major_index + 1) % major_segments
        for minor_index in range(minor_segments):
            next_minor = (minor_index + 1) % minor_segments
            a = major_index * minor_segments + minor_index
            b = next_major * minor_segments + minor_index
            c = next_major * minor_segments + next_minor
            d = major_index * minor_segments + next_minor
            faces.append((a, b, c, d))
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    obj = create_mesh_object(collection, name, mesh, material)
    obj.location = location
    return obj


def create_path_ribbon(collection, material):
    points = [Vector((x, y, 0.055)) for x, y in PORTALS]
    half_width = 1.05
    vertices = []
    faces = []
    for index, point in enumerate(points):
        if index == 0:
            tangent = points[1] - point
        elif index + 1 == len(points):
            tangent = point - points[index - 1]
        else:
            tangent = points[index + 1] - points[index - 1]
        tangent.z = 0.0
        tangent.normalize()
        perpendicular = Vector((-tangent.y, tangent.x, 0.0))
        vertices.append(tuple(point + perpendicular * half_width))
        vertices.append(tuple(point - perpendicular * half_width))
    for index in range(len(points) - 1):
        base = index * 2
        faces.append((base, base + 1, base + 3, base + 2))
    mesh = bpy.data.meshes.new("RF2_PilgrimRoadMesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    create_mesh_object(collection, "RF2_PilgrimRoad", mesh, material)


def create_irregular_pool(collection, material):
    boundary = [
        (-19.5, -7.0, -0.12),
        (-16.0, -10.5, -0.12),
        (-10.5, -11.0, -0.12),
        (-7.0, -8.0, -0.12),
        (-8.7, -5.5, -0.12),
        (-14.0, -4.8, -0.12),
    ]
    center = tuple(sum((Vector(point) for point in boundary), Vector()) / len(boundary))
    vertices = [center] + boundary
    faces = []
    for index in range(len(boundary)):
        next_index = (index + 1) % len(boundary)
        faces.append((0, index + 1, next_index + 1))
    mesh = bpy.data.meshes.new("RF2_ObsidianPoolMesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    create_mesh_object(collection, "RF2_ObsidianPool", mesh, material)


def create_cave_sky(collection, material):
    horizontal_radius = 72.0
    vertical_radius = 38.0
    center_height = 4.0
    longitude_segments = 32
    latitude_segments = 14
    vertices = []
    texture_coordinates = []
    for latitude_index in range(latitude_segments + 1):
        latitude = math.pi * latitude_index / latitude_segments
        ring_radius = math.sin(latitude) * horizontal_radius
        height = math.cos(latitude) * vertical_radius + center_height
        for longitude_index in range(longitude_segments + 1):
            longitude = math.tau * longitude_index / longitude_segments
            vertices.append((
                ring_radius * math.cos(longitude),
                ring_radius * math.sin(longitude),
                height,
            ))
            texture_coordinates.append((
                longitude_index / longitude_segments,
                latitude_index / latitude_segments,
            ))
    faces = []
    row_size = longitude_segments + 1
    for latitude_index in range(latitude_segments):
        for longitude_index in range(longitude_segments):
            a = latitude_index * row_size + longitude_index
            b = a + 1
            c = a + row_size + 1
            d = a + row_size
            faces.append((a, d, c, b))
    mesh = bpy.data.meshes.new("RF2_CaveSkyMesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = texture_coordinates[vertex_index]
    create_mesh_object(collection, "RF2_CaveSky", mesh, material)


def create_gateway(collection, prefix, center, direction, shrine_material, wall_material):
    direction_vector = Vector(direction)
    direction_vector.normalize()
    side = Vector((-direction_vector.y, direction_vector.x))
    rotation = math.atan2(direction_vector.y, direction_vector.x)
    for side_index, sign in enumerate((-1.0, 1.0)):
        position = Vector(center) + side * 1.75 * sign
        create_rock(
            collection,
            prefix + "PillarRock" + str(side_index),
            (position.x, position.y, 1.55),
            (0.75, 0.9, 1.85),
            rotation + sign * 0.12,
            wall_material,
            40 + side_index,
        )
        create_box(
            collection,
            prefix + "RunePillar" + str(side_index),
            (position.x, position.y, 1.65),
            (0.42, 0.42, 1.55),
            rotation,
            shrine_material,
        )
    create_box(
        collection,
        prefix + "Lintel",
        (center[0], center[1], 3.25),
        (2.35, 0.5, 0.38),
        rotation,
        shrine_material,
    )


def add_crystal_cluster(collection, prefix, center, crystal_material, wall_material, seed):
    x, y = center
    create_rock(
        collection,
        prefix + "Base",
        (x, y, 0.28),
        (1.15, 0.9, 0.45),
        seed * 0.31,
        wall_material,
        seed,
    )
    specifications = [
        (-0.38, 0.08, 0.24, 1.8, -0.12),
        (0.12, 0.15, 0.32, 2.45, 0.05),
        (0.48, -0.12, 0.22, 1.55, 0.14),
    ]
    for index, (offset_x, offset_y, radius, height, tilt) in enumerate(specifications):
        create_crystal(
            collection,
            prefix + "Crystal" + str(index),
            (x + offset_x, y + offset_y, 0.38 + height * 0.5),
            radius,
            height,
            (tilt, -tilt * 0.7, seed * 0.23 + index * 0.8),
            crystal_material,
        )


def build_stage_details(collection, materials):
    create_cave_sky(collection, materials["sky"])
    create_path_ribbon(collection, materials["path"])
    for index, (x, y) in enumerate(PORTALS):
        create_portal_mountain(
            collection,
            "RF2_PortalMountain_%02d" % index,
            (x, y, 0.0),
            materials["mountain"],
            150 + index,
        )
        create_cylinder(
            collection,
            "RF2_PortalBase_%02d" % index,
            (x, y, 0.02),
            1.38,
            0.24,
            12,
            materials["wall"],
        )
        create_cylinder(
            collection,
            "RF2_PortalInset_%02d" % index,
            (x, y, 0.17),
            1.06,
            0.16,
            12,
            materials["shrine"],
        )
        ring_material = materials["path"]
        if index in (0, 1, 10):
            ring_material = materials["crystal"]
        create_torus(
            collection,
            "RF2_PortalRing_%02d" % index,
            (x, y, 0.30),
            1.12,
            0.09,
            ring_material,
        )

    create_gateway(
        collection,
        "RF2_WestGate_",
        (-14.0, 14.0),
        (2.0, -1.0),
        materials["shrine"],
        materials["wall"],
    )
    create_gateway(
        collection,
        "RF2_EastGate_",
        (9.0, -7.0),
        (2.0, -1.0),
        materials["shrine"],
        materials["wall"],
    )

    crystal_centers = [
        (-14.5, 3.0),
        (-8.5, 10.0),
        (1.5, 7.5),
        (7.0, 16.5),
        (14.5, -7.5),
    ]
    for index, center in enumerate(crystal_centers):
        add_crystal_cluster(
            collection,
            "RF2_CrystalCluster_%02d_" % index,
            center,
            materials["crystal"],
            materials["wall"],
            60 + index,
        )

    wall_rocks = [
        (-20.2, -7.0, 2.0, 1.6, 2.6),
        (-20.4, 2.0, 2.2, 1.5, 3.0),
        (-20.0, 9.0, 2.0, 1.6, 2.7),
        (20.0, -6.0, 2.3, 1.8, 2.9),
        (20.2, 2.0, 2.0, 1.6, 2.5),
        (20.0, 10.0, 2.2, 1.7, 3.1),
        (-10.0, 18.2, 2.6, 1.4, 2.4),
        (0.0, 18.3, 2.4, 1.5, 2.8),
        (10.0, 18.2, 2.7, 1.4, 2.5),
    ]
    for index, (x, y, sx, sy, sz) in enumerate(wall_rocks):
        create_rock(
            collection,
            "RF2_WallButtress_%02d" % index,
            (x, y, sz * 0.62),
            (sx, sy, sz),
            index * 0.47,
            materials["wall"],
            90 + index,
        )

    create_irregular_pool(collection, materials["dark"])
    pool_edge_rocks = [
        (-18.3, -7.0),
        (-15.5, -10.0),
        (-11.5, -10.2),
        (-8.5, -8.0),
        (-10.0, -5.8),
        (-15.2, -5.4),
    ]
    for index, (x, y) in enumerate(pool_edge_rocks):
        create_rock(
            collection,
            "RF2_PoolEdge_%02d" % index,
            (x, y, 0.22),
            (1.15, 0.65, 0.48),
            index * 0.71,
            materials["wall"],
            120 + index,
        )

    moss_locations = [(-12.0, 11.5), (2.0, 13.5), (3.0, 3.5), (-6.5, -2.0)]
    for index, (x, y) in enumerate(moss_locations):
        create_cylinder(
            collection,
            "RF2_MossMedallion_%02d" % index,
            (x, y, 0.08),
            0.72,
            0.06,
            10,
            materials["moss"],
        )


def export_directx_meshes(collection, output_path):
    bpy.ops.object.select_all(action="DESELECT")
    selected_count = 0
    for scene_object in collection.all_objects:
        if scene_object.type != "MESH":
            continue
        if not scene_object.get("rf2_export"):
            continue
        ensure_generated_uv_map(scene_object.data)
        scene_object.select_set(True)
        selected_count += 1

    if selected_count == 0:
        raise RuntimeError("No meshes were selected for DirectX export.")

    result = bpy.ops.export_scene.directx_x(
        filepath=output_path,
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="-Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        use_original_material_data=False,
        export_format="TEXT_X",
        triangulate=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX export failed: " + output_path)

    print("EXPORTED_DETAIL_OBJECTS", selected_count)


def add_preview_light(name, light_type, location, energy, color, size=5.0):
    light_data = bpy.data.lights.new(name + "Data", type=light_type)
    light_data.energy = energy
    light_data.color = color
    if light_type == "AREA":
        light_data.shape = "DISK"
        light_data.size = size
    light_object = bpy.data.objects.new(name, light_data)
    bpy.context.scene.collection.objects.link(light_object)
    light_object.location = location
    return light_object


def aim_object(obj, target):
    direction = Vector(target) - obj.location
    obj.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def render_preview(preview_path):
    for obj in list(bpy.data.objects):
        if obj.name.startswith("RF2_PREVIEW_"):
            bpy.data.objects.remove(obj, do_unlink=True)
    camera_data = bpy.data.cameras.new("RF2_PREVIEW_CameraData")
    camera = bpy.data.objects.new("RF2_PREVIEW_Camera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    camera.location = (0.0, -32.0, 15.0)
    camera_data.lens = 42.0
    aim_object(camera, (0.0, 5.5, 0.8))
    bpy.context.scene.camera = camera

    key = add_preview_light(
        "RF2_PREVIEW_Key",
        "AREA",
        (-8.0, -12.0, 18.0),
        1700.0,
        (0.70, 0.80, 1.0),
        12.0,
    )
    aim_object(key, (0.0, 2.0, 0.0))
    fill = add_preview_light(
        "RF2_PREVIEW_Fill",
        "AREA",
        (14.0, 4.0, 10.0),
        1100.0,
        (0.35, 0.55, 1.0),
        9.0,
    )
    aim_object(fill, (3.0, 0.0, 0.0))
    warm = add_preview_light(
        "RF2_PREVIEW_Warm",
        "AREA",
        (-16.0, 13.0, 7.0),
        900.0,
        (1.0, 0.48, 0.20),
        6.0,
    )
    aim_object(warm, (-12.0, 9.0, 0.0))

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1200
    scene.render.resolution_y = 675
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = preview_path
    scene.render.film_transparent = False
    scene.world.color = (0.012, 0.016, 0.028)
    os.makedirs(os.path.dirname(preview_path), exist_ok=True)
    bpy.ops.render.render(write_still=True)


def main():
    output_x_path, preview_path = get_script_arguments()
    remove_collection(DETAIL_COLLECTION_NAME)
    remove_obsolete_base_objects()
    remove_dummy_base_objects()
    cut_portal_mountain_openings()
    collection = create_detail_collection()
    materials = {
        "sky": create_cave_sky_material(),
        "wall": find_material("stageSelectCaveWall"),
        "dark": find_material("stageSelectCaveDeepDark"),
        "path": find_material("stageSelectCavePath"),
        "moss": find_material("stageSelectCaveMoss"),
        "crystal": find_material("stageSelectCaveCrystal"),
        "shrine": find_material("stageSelectCaveShrine"),
        "mountain": create_portal_mountain_material(),
    }
    build_stage_details(collection, materials)
    for obj in bpy.context.scene.collection.all_objects:
        if obj.type == "MESH":
            obj["rf2_export"] = True
    export_directx_meshes(bpy.context.scene.collection, output_x_path)
    render_preview(preview_path)
    bpy.ops.wm.save_as_mainfile(filepath=bpy.data.filepath)
    print("SAVED_BLEND", bpy.data.filepath)
    print("PREVIEW", preview_path)


if __name__ == "__main__":
    main()
