import math
import os
import csv

import bpy


DEEP_SEA_OBJECT_NAME = "StageSelect_DeepSea.001"
SHALLOW_WATER_OBJECT_NAME = "StageSelect_ShallowWaterRing.001"
GRASS_APRON_OBJECT_NAME = "StageSelect_CentralMountainContactApron"
GRASS_SKIRT_OBJECT_NAME = "RF1_GrassToBeachSeamSkirt"
SEA_RADIUS_METERS = 500.0
SEA_SEGMENT_COUNT = 128
SEA_HEIGHT = -2.35
SEA_TEXTURE_TILE_METERS = 12.0

SEA_MATERIAL_NAMES = {
    "DeepSea_TropicalBlue.001",
    "ShallowWater_Cyan",
    "WaveFoam_White",
}

MATERIAL_SETTINGS = {
    "CoastalRock": (0.65, 0.18),
    "DeepSea_TropicalBlue.001": (0.48, 0.20),
    "IslandGrass": (0.62, 0.14),
    "PalmLeaves": (0.68, 0.12),
    "PalmLeaves_LitTips": (0.65, 0.14),
    "PalmTrunk": (0.65, 0.16),
    "ShallowWater_Cyan": (0.42, 0.18),
    "ShellAccent": (0.62, 0.15),
    "WarmSand": (0.58, 0.16),
    "WaveFoam_White": (0.70, 0.12),
    "WeatheredWood": (0.72, 0.14),
}


def get_required_object(object_name):
    scene_object = bpy.data.objects.get(object_name)
    if scene_object is None:
        raise RuntimeError("Stage-select 1 object was not found: " + object_name)
    if scene_object.type != "MESH":
        raise RuntimeError("Stage-select 1 object is not a mesh: " + object_name)
    return scene_object


def get_required_material(material_name):
    material = bpy.data.materials.get(material_name)
    if material is None:
        raise RuntimeError("Stage-select 1 material was not found: " + material_name)
    return material


def get_principled_bsdf(material):
    if not material.use_nodes or material.node_tree is None:
        raise RuntimeError("Stage-select 1 material does not use nodes: " + material.name)

    for node in material.node_tree.nodes:
        if node.type == "BSDF_PRINCIPLED":
            return node

    raise RuntimeError("Stage-select 1 material has no Principled BSDF: " + material.name)


def apply_material_settings():
    for material_name, values in MATERIAL_SETTINGS.items():
        material = get_required_material(material_name)
        roughness, specular_intensity = values
        shader = get_principled_bsdf(material)
        roughness_input = shader.inputs.get("Roughness")
        specular_input = shader.inputs.get("Specular IOR Level")
        if roughness_input is None or specular_input is None:
            raise RuntimeError("Stage-select 1 material inputs were not found: " + material_name)

        roughness_input.default_value = roughness
        specular_input.default_value = specular_intensity
        material.roughness = roughness
        material.specular_intensity = specular_intensity
        material["_x_face_color"] = tuple(material.diffuse_color)


def assign_planar_uv(mesh, tile_size):
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            vertex = mesh.vertices[vertex_index].co
            uv_layer.data[loop_index].uv = (
                vertex.x / tile_size + 0.5,
                vertex.y / tile_size + 0.5,
            )


def replace_deep_sea_with_disk():
    sea_object = get_required_object(DEEP_SEA_OBJECT_NAME)
    sea_material = get_required_material("DeepSea_TropicalBlue.001")

    vertices = [(0.0, 0.0, SEA_HEIGHT)]
    for segment_index in range(SEA_SEGMENT_COUNT):
        angle = math.tau * float(segment_index) / float(SEA_SEGMENT_COUNT)
        vertices.append((
            math.cos(angle) * SEA_RADIUS_METERS,
            math.sin(angle) * SEA_RADIUS_METERS,
            SEA_HEIGHT,
        ))

    faces = []
    for segment_index in range(SEA_SEGMENT_COUNT):
        current_index = segment_index + 1
        next_index = ((segment_index + 1) % SEA_SEGMENT_COUNT) + 1
        faces.append((0, current_index, next_index))

    new_mesh = bpy.data.meshes.new("StageSelect_DeepSea_OneKilometerDisk")
    new_mesh.from_pydata(vertices, [], faces)
    new_mesh.materials.append(sea_material)
    assign_planar_uv(new_mesh, SEA_TEXTURE_TILE_METERS)
    new_mesh.update()

    old_mesh = sea_object.data
    sea_object.data = new_mesh
    sea_object.location = (0.0, 0.0, 0.0)
    sea_object.rotation_euler = (0.0, 0.0, 0.0)
    sea_object.scale = (1.0, 1.0, 1.0)
    if old_mesh.users == 0:
        bpy.data.meshes.remove(old_mesh)


def widen_shallow_water_overlap():
    shallow_object = get_required_object(SHALLOW_WATER_OBJECT_NAME)
    if shallow_object.get("rf1_overlap_expanded", False):
        return

    vertex_count = len(shallow_object.data.vertices)
    if vertex_count % 2 != 0:
        raise RuntimeError("Stage-select 1 shallow-water ring has an unexpected vertex count.")

    outer_vertex_count = vertex_count // 2
    for vertex_index, vertex in enumerate(shallow_object.data.vertices):
        scale = 0.88
        if vertex_index < outer_vertex_count:
            scale = 1.25
        vertex.co.x *= scale
        vertex.co.y *= scale

    shallow_object["rf1_overlap_expanded"] = True
    shallow_object.data.update()


def expand_grass_apron_overlap():
    apron_object = get_required_object(GRASS_APRON_OBJECT_NAME)
    if apron_object.get("rf1_overlap_expanded", False):
        return

    for vertex_index, vertex in enumerate(apron_object.data.vertices):
        scale = 0.98
        if vertex_index % 2 == 1:
            scale = 1.06
        vertex.co.x *= scale
        vertex.co.y *= scale

    apron_object["rf1_overlap_expanded"] = True
    apron_object.data.update()


def remove_existing_object(object_name):
    scene_object = bpy.data.objects.get(object_name)
    if scene_object is None:
        return
    mesh = scene_object.data
    bpy.data.objects.remove(scene_object, do_unlink=True)
    if mesh.users == 0:
        bpy.data.meshes.remove(mesh)


def create_grass_to_beach_seam_skirt():
    remove_existing_object(GRASS_SKIRT_OBJECT_NAME)
    apron_object = get_required_object(GRASS_APRON_OBJECT_NAME)
    grass_material = get_required_material("IslandGrass")

    outer_vertices = []
    for vertex_index, vertex in enumerate(apron_object.data.vertices):
        if vertex_index % 2 == 1:
            outer_vertices.append(vertex.co.copy())

    if len(outer_vertices) < 3:
        raise RuntimeError("Stage-select 1 grass apron has no usable outer boundary.")

    vertices = []
    for vertex in outer_vertices:
        vertices.append((vertex.x, vertex.y, vertex.z + 0.02))
    for vertex in outer_vertices:
        vertices.append((vertex.x * 1.015, vertex.y * 1.015, -1.18))

    boundary_count = len(outer_vertices)
    faces = []
    for boundary_index in range(boundary_count):
        next_index = (boundary_index + 1) % boundary_count
        faces.append((
            boundary_index,
            next_index,
            boundary_count + next_index,
            boundary_count + boundary_index,
        ))

    mesh = bpy.data.meshes.new("RF1_GrassToBeachSeamSkirtMesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.materials.append(grass_material)
    assign_planar_uv(mesh, 8.0)
    mesh.update()

    skirt_object = bpy.data.objects.new(GRASS_SKIRT_OBJECT_NAME, mesh)
    bpy.context.scene.collection.objects.link(skirt_object)


def align_portal_pedestals(output_directory):
    interactables_path = os.path.join(output_directory, "Interactables.csv")
    portal_positions = []
    with open(interactables_path, "r", encoding="utf-8-sig", newline="") as interactables_file:
        reader = csv.DictReader(interactables_file)
        for row in reader:
            if row["Type"] == "StagePortal":
                portal_positions.append((float(row["PosX"]), float(row["PosZ"])))

    if len(portal_positions) != 10:
        raise RuntimeError("Stage-select 1 must define exactly 10 stage portals.")

    object_suffixes = ("Base", "Inset", "Ring")
    for portal_index, portal_position in enumerate(portal_positions):
        game_x, game_z = portal_position
        for object_suffix in object_suffixes:
            object_name = "RF1_Portal_{:02d}_{}".format(portal_index, object_suffix)
            pedestal_object = get_required_object(object_name)
            pedestal_object.location.x = -game_x
            pedestal_object.location.y = game_z


def is_sea_object(scene_object):
    if scene_object.type != "MESH":
        return False
    if len(scene_object.data.materials) == 0 or scene_object.data.materials[0] is None:
        return False
    return scene_object.data.materials[0].name in SEA_MATERIAL_NAMES


def export_selected_meshes(output_path, export_sea):
    bpy.ops.object.select_all(action="DESELECT")
    selected_count = 0
    for scene_object in bpy.context.scene.objects:
        if scene_object.type != "MESH":
            continue

        object_is_sea = is_sea_object(scene_object)
        should_select = object_is_sea == export_sea
        if should_select:
            scene_object.select_set(True)
            selected_count += 1

    if selected_count == 0:
        raise RuntimeError("No stage-select 1 meshes were selected for export.")

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


def main():
    blend_path = bpy.data.filepath
    if not blend_path:
        raise RuntimeError("Open world1.blend before running this script.")

    output_directory = os.path.dirname(blend_path)
    apply_material_settings()
    replace_deep_sea_with_disk()
    widen_shallow_water_overlap()
    expand_grass_apron_overlap()
    create_grass_to_beach_seam_skirt()
    align_portal_pedestals(output_directory)

    export_selected_meshes(os.path.join(output_directory, "stageSelectIsland.x"), False)
    export_selected_meshes(os.path.join(output_directory, "stageSelectSea.x"), True)
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    print("FIXED_STAGE_SELECT_1_ENVIRONMENT", output_directory)


if __name__ == "__main__":
    main()
