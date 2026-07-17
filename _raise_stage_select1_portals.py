import bpy
import math
import os
import random
from mathutils import Vector


model_dir = os.path.dirname(bpy.data.filepath)
island_export_path = os.path.join(model_dir, "stageSelectIsland.x")
sea_export_path = os.path.join(model_dir, "stageSelectSea.x")
preview_path = os.path.join(model_dir, "world1_preview.png")
bpy.context.preferences.filepaths.save_version = 0


def remove_object(object_name):
    target = bpy.data.objects.get(object_name)
    if target is not None:
        bpy.data.objects.remove(target, do_unlink=True)


def get_material(material_name):
    material = bpy.data.materials.get(material_name)
    if material is None:
        raise RuntimeError(f"Material was not found: {material_name}")
    return material


def assign_material(target, material):
    target.data.materials.clear()
    target.data.materials.append(material)


def create_portal_path(portal_positions):
    for target in list(bpy.data.objects):
        if target.name.startswith("RF1_PathStone_") or target.name.startswith("RF1_PathLog_"):
            bpy.data.objects.remove(target, do_unlink=True)

    stone_material = get_material("CoastalRock")
    wood_material = get_material("WeatheredWood")
    path_piece_index = 0

    for segment_index in range(len(portal_positions) - 1):
        start_x, start_y, start_height = portal_positions[segment_index]
        end_x, end_y, end_height = portal_positions[segment_index + 1]
        delta_x = end_x - start_x
        delta_y = end_y - start_y
        distance = math.sqrt(delta_x * delta_x + delta_y * delta_y)
        piece_count = max(1, int(distance / 2.0))
        cross_direction = Vector((-delta_y, delta_x, 0.0)).normalized()

        for piece_index in range(piece_count):
            interpolation = float(piece_index + 1) / float(piece_count + 1)
            position_x = start_x + delta_x * interpolation
            position_y = start_y + delta_y * interpolation
            ground_height = start_height + (end_height - start_height) * interpolation

            if path_piece_index % 3 == 2:
                bpy.ops.mesh.primitive_cylinder_add(
                    vertices=8,
                    radius=0.18,
                    depth=1.25,
                    location=(position_x, position_y, ground_height + 0.18),
                )
                path_object = bpy.context.object
                path_object.name = f"RF1_PathLog_{path_piece_index:02d}"
                path_object.rotation_euler = cross_direction.to_track_quat("Z", "Y").to_euler()
                assign_material(path_object, wood_material)
            else:
                bpy.ops.mesh.primitive_ico_sphere_add(
                    subdivisions=1,
                    radius=1.0,
                    location=(position_x, position_y, ground_height + 0.08),
                )
                path_object = bpy.context.object
                path_object.name = f"RF1_PathStone_{path_piece_index:02d}"
                path_object.scale = (0.60, 0.42, 0.12)
                path_object.rotation_euler.z = math.atan2(delta_y, delta_x)
                assign_material(path_object, stone_material)
                for polygon in path_object.data.polygons:
                    polygon.use_smooth = False

            path_piece_index += 1


def remove_objects_with_prefixes(prefixes):
    for target in list(bpy.data.objects):
        if target.name.startswith(prefixes):
            bpy.data.objects.remove(target, do_unlink=True)


def create_golden_mountain_trail(portal_positions):
    remove_objects_with_prefixes((
        "RF1_PathStone_",
        "RF1_PathLog_",
        "RF1_GoldenTrail",
        "RF1_TrailStep_",
    ))

    sand_material = get_material("WarmSand")
    wood_material = get_material("WeatheredWood")
    half_width = 1.18
    vertices = []
    texture_coordinates = []
    cumulative_distance = 0.0

    for point_index, point in enumerate(portal_positions):
        point_vector = Vector(point)
        if point_index == 0:
            tangent = Vector(portal_positions[1]) - point_vector
        elif point_index + 1 == len(portal_positions):
            tangent = point_vector - Vector(portal_positions[point_index - 1])
        else:
            tangent = Vector(portal_positions[point_index + 1]) - Vector(portal_positions[point_index - 1])
        tangent.z = 0.0
        tangent.normalize()
        perpendicular = Vector((-tangent.y, tangent.x, 0.0))

        if point_index > 0:
            previous = Vector(portal_positions[point_index - 1])
            planar_difference = point_vector - previous
            planar_difference.z = 0.0
            cumulative_distance += planar_difference.length

        trail_height = point_vector.z + 0.18
        left = Vector((point_vector.x, point_vector.y, trail_height)) + perpendicular * half_width
        right = Vector((point_vector.x, point_vector.y, trail_height)) - perpendicular * half_width
        vertices.append(tuple(left))
        vertices.append(tuple(right))
        texture_u = cumulative_distance / 3.5
        texture_coordinates.append((texture_u, 0.0))
        texture_coordinates.append((texture_u, 1.0))

    faces = []
    for point_index in range(len(portal_positions) - 1):
        base_index = point_index * 2
        faces.append((base_index, base_index + 1, base_index + 3, base_index + 2))

    mesh = bpy.data.meshes.new("RF1_GoldenTrailMesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.materials.append(sand_material)
    mesh.update()
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = texture_coordinates[vertex_index]

    trail_object = bpy.data.objects.new("RF1_GoldenTrail", mesh)
    bpy.context.scene.collection.objects.link(trail_object)

    step_index = 0
    for segment_index in range(len(portal_positions) - 1):
        start = Vector(portal_positions[segment_index])
        end = Vector(portal_positions[segment_index + 1])
        height_difference = end.z - start.z
        if abs(height_difference) < 0.75:
            continue

        direction = end - start
        direction.z = 0.0
        direction.normalize()
        cross_direction = Vector((-direction.y, direction.x, 0.0))
        step_count = max(3, int(abs(height_difference) * 3.0))
        for local_step_index in range(step_count):
            interpolation = float(local_step_index + 1) / float(step_count + 1)
            position = start.lerp(end, interpolation)
            bpy.ops.mesh.primitive_cube_add(
                size=1.0,
                location=(position.x, position.y, position.z + 0.27),
            )
            step_object = bpy.context.object
            step_object.name = f"RF1_TrailStep_{step_index:02d}"
            step_object.scale = (half_width * 0.92, 0.14, 0.09)
            step_object.rotation_euler.z = math.atan2(cross_direction.y, cross_direction.x)
            assign_material(step_object, wood_material)
            step_index += 1


def apply_mountain_cliff_materials():
    mountain = bpy.data.objects.get("StageSelect_CentralMountain")
    if mountain is None or mountain.type != "MESH":
        raise RuntimeError("StageSelect_CentralMountain was not found.")

    grass_material = get_material("IslandGrass")
    rock_material = get_material("CoastalRock")
    if mountain.data.materials.get(grass_material.name) is None:
        mountain.data.materials.append(grass_material)
    if mountain.data.materials.get(rock_material.name) is None:
        mountain.data.materials.append(rock_material)
    grass_index = mountain.data.materials.find(grass_material.name)
    rock_index = mountain.data.materials.find(rock_material.name)

    for polygon in mountain.data.polygons:
        is_low_cliff = polygon.center.z < 1.35 and polygon.normal.z < 0.72
        if is_low_cliff:
            polygon.material_index = rock_index
            polygon.use_smooth = False
        else:
            polygon.material_index = grass_index
            polygon.use_smooth = True


def create_tropical_palm(prefix, location, height, rotation):
    trunk_material = get_material("PalmTrunk")
    leaf_material = get_material("PalmLeaves")
    bpy.ops.mesh.primitive_cone_add(
        vertices=7,
        radius1=0.22,
        radius2=0.13,
        depth=height,
        location=(location[0], location[1], location[2] + height * 0.5),
    )
    trunk = bpy.context.object
    trunk.name = prefix + "Trunk"
    trunk.rotation_euler.z = rotation
    assign_material(trunk, trunk_material)
    for polygon in trunk.data.polygons:
        polygon.use_smooth = False

    crown_height = location[2] + height
    leaf_vertices = []
    leaf_faces = []
    leaf_count = 7
    for leaf_index in range(leaf_count):
        angle = rotation + math.tau * leaf_index / leaf_count
        direction = Vector((math.cos(angle), math.sin(angle), 0.0))
        side = Vector((-direction.y, direction.x, 0.0))
        center = Vector((location[0], location[1], crown_height + 0.08))
        middle = center + direction * 0.75 + Vector((0.0, 0.0, 0.18))
        tip = center + direction * 1.75 + Vector((0.0, 0.0, -0.38))
        base_index = len(leaf_vertices)
        leaf_vertices.extend((
            tuple(center - side * 0.16),
            tuple(center + side * 0.16),
            tuple(middle + side * 0.24),
            tuple(tip),
        ))
        leaf_faces.append((base_index, base_index + 1, base_index + 2, base_index + 3))
        leaf_faces.append((base_index + 3, base_index + 2, base_index + 1, base_index))

    leaf_mesh = bpy.data.meshes.new(prefix + "LeavesMesh")
    leaf_mesh.from_pydata(leaf_vertices, [], leaf_faces)
    leaf_mesh.materials.append(leaf_material)
    leaf_mesh.update()
    leaves = bpy.data.objects.new(prefix + "Leaves", leaf_mesh)
    bpy.context.scene.collection.objects.link(leaves)


def create_tropical_details():
    remove_objects_with_prefixes(("RF1_NewPalm_", "RF1_TropicalBush_", "RF1_NewCliff_"))
    palm_specs = [
        (-10.5, -5.0, 1.0, 2.0, 0.2),
        (-8.3, -1.8, 1.8, 2.3, 1.1),
        (5.8, -8.0, 0.8, 2.1, 2.0),
        (10.0, -2.7, 1.0, 2.4, 0.6),
        (-7.0, 0.7, 2.7, 1.9, 1.8),
        (4.8, 1.2, 4.7, 1.8, 2.6),
        (-0.5, 1.8, 5.7, 1.7, 0.9),
    ]
    for palm_index, (x, y, z, height, rotation) in enumerate(palm_specs):
        create_tropical_palm(
            f"RF1_NewPalm_{palm_index:02d}_",
            (x, y, z),
            height,
            rotation,
        )

    grass_material = get_material("IslandGrass")
    bush_specs = [
        (-9.0, -7.0, 1.0),
        (-5.0, -7.2, 1.2),
        (3.7, -7.0, 1.4),
        (8.8, -5.0, 1.3),
        (4.5, -2.5, 3.4),
        (-5.5, -2.2, 3.5),
        (0.0, 0.8, 5.8),
    ]
    for bush_index, (x, y, z) in enumerate(bush_specs):
        bpy.ops.mesh.primitive_ico_sphere_add(
            subdivisions=1,
            radius=1.0,
            location=(x, y, z + 0.28),
        )
        bush = bpy.context.object
        bush.name = f"RF1_TropicalBush_{bush_index:02d}"
        bush.scale = (0.72, 0.55, 0.38)
        bush.rotation_euler.z = bush_index * 0.73
        assign_material(bush, grass_material)
        for polygon in bush.data.polygons:
            polygon.use_smooth = False

    rock_material = get_material("CoastalRock")
    cliff_specs = [
        (-4.8, -7.2, 1.1, 1.4, 0.9, 1.5),
        (3.7, -7.4, 1.0, 1.2, 0.8, 1.7),
        (-6.2, -1.3, 2.0, 1.0, 0.8, 1.8),
        (5.7, -1.0, 2.6, 1.2, 0.9, 2.0),
        (-0.5, 1.2, 4.5, 1.0, 0.8, 1.6),
    ]
    for cliff_index, (x, y, z, scale_x, scale_y, scale_z) in enumerate(cliff_specs):
        bpy.ops.mesh.primitive_ico_sphere_add(
            subdivisions=1,
            radius=1.0,
            location=(x, y, z + scale_z * 0.5),
        )
        cliff = bpy.context.object
        cliff.name = f"RF1_NewCliff_{cliff_index:02d}"
        cliff.scale = (scale_x, scale_y, scale_z)
        cliff.rotation_euler.z = cliff_index * 0.91
        assign_material(cliff, rock_material)
        for polygon in cliff.data.polygons:
            polygon.use_smooth = False


def make_rocks_more_rugged():
    rugged_prefixes = (
        "RF1_MountainOutcrop_",
        "RF1_CoastFormation_",
        "RF1_ReefRock_",
    )
    for target in bpy.data.objects:
        if target.type != "MESH" or not target.name.startswith(rugged_prefixes):
            continue
        if not target.get("rf1_extra_rugged", False):
            generator = random.Random(target.name)
            center = sum((vertex.co for vertex in target.data.vertices), Vector()) / len(target.data.vertices)
            for vertex_index, vertex in enumerate(target.data.vertices):
                relative = vertex.co - center
                variation = 0.76 + generator.random() * 0.52
                if vertex_index % 3 == 0:
                    variation += 0.18
                vertex.co = center + relative * variation
            target["rf1_extra_rugged"] = True
        for polygon in target.data.polygons:
            polygon.use_smooth = False


def close_mountain_grass_gap():
    grass_material = get_material("IslandGrass")
    fill_material = bpy.data.materials.get("RF1_GrassGapFillGreen")
    if fill_material is None:
        fill_material = bpy.data.materials.new("RF1_GrassGapFillGreen")
    fill_material.diffuse_color = (0.10, 0.46, 0.08, 1.0)
    fill_material.use_nodes = True
    principled = fill_material.node_tree.nodes.get("Principled BSDF")
    if principled is None:
        principled = fill_material.node_tree.nodes.get("プリンシプルBSDF")
    if principled is not None:
        principled.inputs["Base Color"].default_value = (0.10, 0.46, 0.08, 1.0)
        principled.inputs["Roughness"].default_value = 0.92
    fill_material["_x_power"] = 500.0
    remove_object("RF1_GrassGapFillSurface")
    remove_object("StageSelect_CentralMountainContactApron")
    remove_object("RF1_GrassToBeachSeamSkirt")

    sandy_island = bpy.data.objects.get("StageSelect_SandyIslandBase.001")
    if sandy_island is not None:
        sand_material = get_material("WarmSand")
        if sandy_island.data.materials.get(sand_material.name) is None:
            sandy_island.data.materials.append(sand_material)
        if sandy_island.data.materials.get(fill_material.name) is None:
            sandy_island.data.materials.append(fill_material)
        sand_index = sandy_island.data.materials.find(sand_material.name)
        fill_index = sandy_island.data.materials.find(fill_material.name)
        for polygon in sandy_island.data.polygons:
            center = polygon.center
            ellipse_distance = (center.x / 19.0) ** 2 + (center.y / 13.5) ** 2
            if polygon.normal.z > 0.45 and ellipse_distance < 1.0:
                polygon.material_index = fill_index
            else:
                polygon.material_index = sand_index

    central_mountain = bpy.data.objects.get("StageSelect_CentralMountain")
    if central_mountain is not None:
        if central_mountain.data.materials.get(grass_material.name) is None:
            central_mountain.data.materials.append(grass_material)
        central_mountain.scale.x = 1.02
        central_mountain.scale.y = 1.02

    side_band = bpy.data.objects.get("StageSelect_CentralMountainSideBand")
    if side_band is not None:
        assign_material(side_band, grass_material)
        side_band.scale.x = 1.03
        side_band.scale.y = 1.03

objects_to_remove = [
    "StageSelect_CentralMountainRockCap",
]
objects_to_remove.extend(
    f"StageSelect_DriftwoodPlank_{index:02d}.001"
    for index in range(5)
)
objects_to_remove.extend(
    f"StageSelect_FoamArc_{index:02d}.001"
    for index in range(3)
)
objects_to_remove.extend(
    f"RF1_FoamBreak_{index:02d}"
    for index in range(7)
)
objects_to_remove.extend(
    f"StageSelect_CoastalRock_{index:02d}.001"
    for index in range(5)
)
objects_to_remove.extend(
    f"StageSelect_MountainRock_{index:02d}"
    for index in range(4)
)
objects_to_remove.extend(
    f"RF1_MountainStrata_{index:02d}"
    for index in range(5)
)
for object_name in objects_to_remove:
    remove_object(object_name)

portal_layout = [
    (-15.0, -11.5, 0.8),
    (-12.8, -9.4, 0.7),
    (-7.0, -6.0, 1.1),
    (0.0, -6.0, 2.9),
    (7.0, -6.0, 1.3),
    (7.0, -4.0, 2.6),
    (1.0, -4.0, 4.6),
    (-4.0, -4.0, 4.1),
    (-3.0, 0.0, 5.7),
    (2.0, 0.0, 6.5),
]
close_mountain_grass_gap()
make_rocks_more_rugged()
apply_mountain_cliff_materials()
create_golden_mountain_trail(portal_layout)
create_tropical_details()

for index, portal_position in enumerate(portal_layout):
    position_x, position_y, ground_height = portal_position
    prefix = f"RF1_Portal_{index:02d}"
    base = bpy.data.objects[f"{prefix}_Base"]
    inset = bpy.data.objects[f"{prefix}_Inset"]
    ring = bpy.data.objects[f"{prefix}_Ring"]

    base.location.x = position_x
    base.location.y = position_y
    base.location.z = ground_height + 0.45
    inset.location.x = position_x
    inset.location.y = position_y
    inset.location.z = ground_height + 0.625
    ring.location.x = position_x
    ring.location.y = position_y
    ring.location.z = ground_height + 0.77

    base.scale.x = 0.5
    base.scale.y = 0.5
    inset.scale.x = 0.5
    inset.scale.y = 0.5
    ring.scale.x = 0.5
    ring.scale.y = 0.5

for material in bpy.data.materials:
    material["_x_power"] = 500.0

for texture_name in (
    "stageSelectSea.png",
    "stageSelectShallowWater.png",
    "stageSelectSand.png",
):
    stage_select_texture = bpy.data.images.get(texture_name)
    if stage_select_texture is not None:
        stage_select_texture.reload()

preview_camera = bpy.context.scene.camera
if preview_camera is not None:
    preview_camera.location = Vector((0.0, -26.0, 18.0))
    preview_target = Vector((0.0, -2.0, 2.0))
    preview_camera.rotation_euler = (preview_target - preview_camera.location).to_track_quat("-Z", "Y").to_euler()
    preview_camera.data.lens = 28.0

sea_object_names = {
    "StageSelect_DeepSea.001",
    "StageSelect_ShallowWaterRing.001",
    "RF1_Portal_00_Ring",
    "RF1_Portal_09_Ring",
}


def export_objects(filepath, object_names):
    bpy.ops.object.select_all(action="DESELECT")
    selected_objects = []
    for object_name in object_names:
        export_object = bpy.data.objects.get(object_name)
        if export_object is None or export_object.type != "MESH":
            raise RuntimeError(f"Export mesh was not found: {object_name}")
        export_object.select_set(True)
        selected_objects.append(export_object)

    bpy.context.view_layer.objects.active = selected_objects[0]
    result = bpy.ops.export_scene.directx_x(
        filepath=filepath,
        use_selection=True,
        axis_forward="-Z",
        axis_up="Y",
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"Official DirectX X export failed: {filepath}")


island_object_names = {
    obj.name
    for obj in bpy.data.objects
    if obj.type == "MESH" and obj.name not in sea_object_names
}

bpy.ops.wm.save_as_mainfile(filepath=bpy.data.filepath)
export_objects(island_export_path, island_object_names)
export_objects(sea_export_path, sea_object_names)

bpy.context.scene.render.filepath = preview_path
bpy.ops.render.render(write_still=True)

print("Arranged portals:", len(portal_layout))
print("Preview camera:", tuple(preview_camera.location) if preview_camera is not None else None)
