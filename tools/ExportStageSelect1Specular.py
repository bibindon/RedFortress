import os

import bpy


MATERIAL_SETTINGS = {
    "CoastalRock": (0.10, 0.90),
    "DeepSea_TropicalBlue.001": (0.02, 1.00),
    "IslandGrass": (0.14, 0.75),
    "PalmLeaves": (0.20, 0.65),
    "PalmLeaves_LitTips": (0.16, 0.75),
    "PalmTrunk": (0.22, 0.70),
    "ShallowWater_Cyan": (0.01, 1.00),
    "ShellAccent": (0.08, 1.00),
    "WarmSand": (0.12, 0.85),
    "WaveFoam_White": (0.12, 0.85),
    "WeatheredWood": (0.24, 0.65),
}

SEA_MATERIAL_NAMES = {
    "DeepSea_TropicalBlue.001",
    "ShallowWater_Cyan",
    "WaveFoam_White",
}


def get_principled_bsdf(material):
    if not material.use_nodes or material.node_tree is None:
        raise RuntimeError("Stage-select 1 material does not use nodes: " + material.name)

    for node in material.node_tree.nodes:
        if node.type == "BSDF_PRINCIPLED":
            return node

    raise RuntimeError("Stage-select 1 material has no Principled BSDF: " + material.name)


def apply_specular_settings():
    for material_name, values in MATERIAL_SETTINGS.items():
        material = bpy.data.materials.get(material_name)
        if material is None:
            raise RuntimeError("Stage-select 1 material was not found: " + material_name)

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
    apply_specular_settings()
    export_selected_meshes(os.path.join(output_directory, "stageSelectIsland.x"), False)
    export_selected_meshes(os.path.join(output_directory, "stageSelectSea.x"), True)
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    print("EXPORTED_STAGE_SELECT_1_SPECULAR", output_directory)


if __name__ == "__main__":
    main()
