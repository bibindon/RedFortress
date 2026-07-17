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
        grass_index = central_mountain.data.materials.find(grass_material.name)
        for polygon in central_mountain.data.polygons:
            polygon.material_index = grass_index
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
create_portal_path(portal_layout)

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
