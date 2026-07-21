from collections import OrderedDict
from pathlib import Path

import bpy


REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
MODEL_DIRECTORY = (
    REPOSITORY_ROOT
    / "RedFortress2"
    / "MultiPassRendering"
    / "res"
    / "model2"
    / "marine_512_low"
)
BLEND_PATH = MODEL_DIRECTORY / "marine.blend"
MODEL_X_PATH = MODEL_DIRECTORY / "marine.x"
NON_ANIM_X_PATH = MODEL_DIRECTORY / "marine.nonAnim.x"

ARMATURE_NAME = "宝鐘マリンV2_arm"
MESH_NAME = "宝鐘マリンV2_mesh_decimate50"
EXPECTED_MATERIAL_COUNT = 40
EXPECTED_MERGED_MATERIAL_COUNT = 10
EXPECTED_POLYGON_COUNT = 15490
EXPECTED_VERTEX_COUNT = 14739

# The reduced rig retains these original MMD bone indices. The animation X
# files address the bones with these stable Bone_NNN names.
EXPORT_BONE_NAMES = {
    "全ての親": "Bone_000",
    "センター": "Bone_001",
    "グルーブ": "Bone_002",
    "上半身": "Bone_003",
    "上半身2": "Bone_004",
    "首": "Bone_005",
    "頭": "Bone_006",
    "肩.L": "Bone_152",
    "腕.L": "Bone_153",
    "腕捩.L": "Bone_154",
    "ひじ.L": "Bone_155",
    "手捩.L": "Bone_156",
    "手首.L": "Bone_157",
    "肩.R": "Bone_237",
    "腕.R": "Bone_238",
    "腕捩.R": "Bone_239",
    "ひじ.R": "Bone_240",
    "手捩.R": "Bone_241",
    "手首.R": "Bone_242",
    "下半身": "Bone_444",
    "足.L": "Bone_445",
    "ひざ.L": "Bone_446",
    "足首.L": "Bone_447",
    "つま先.L": "Bone_448",
    "足.R": "Bone_457",
    "ひざ.R": "Bone_458",
    "足首.R": "Bone_459",
    "つま先.R": "Bone_460",
}


def require_object(name, object_type):
    obj = bpy.data.objects.get(name)
    if obj is None:
        raise RuntimeError("Required Blender object is missing: " + name)
    if obj.type != object_type:
        raise RuntimeError("Unexpected object type for " + name + ": " + obj.type)
    return obj


def get_base_texture_name(material):
    if material is None:
        raise RuntimeError("A mesh material slot is empty")
    if not material.use_nodes or material.node_tree is None:
        raise RuntimeError("Material does not use nodes: " + material.name)

    base_node = material.node_tree.nodes.get("mmd_base_tex")
    if base_node is None or base_node.type != "TEX_IMAGE":
        raise RuntimeError("Material has no mmd_base_tex image node: " + material.name)
    if base_node.image is None:
        raise RuntimeError("Material has no base texture image: " + material.name)
    return base_node.image.name


def snapshot_faces_by_texture(mesh):
    slot_textures = []
    for material in mesh.data.materials:
        slot_textures.append(get_base_texture_name(material))

    faces_by_texture = {}
    for polygon in mesh.data.polygons:
        if polygon.material_index >= len(slot_textures):
            raise RuntimeError("Polygon has an invalid material index")
        texture_name = slot_textures[polygon.material_index]
        faces_by_texture[texture_name] = faces_by_texture.get(texture_name, 0) + 1
    return faces_by_texture


def merge_materials_by_base_texture(mesh):
    original_materials = list(mesh.data.materials)
    if len(original_materials) != EXPECTED_MATERIAL_COUNT:
        raise RuntimeError(
            "Expected "
            + str(EXPECTED_MATERIAL_COUNT)
            + " source materials, found "
            + str(len(original_materials))
        )

    groups = OrderedDict()
    old_slot_to_new_slot = {}
    for old_slot, material in enumerate(original_materials):
        texture_name = get_base_texture_name(material)
        if texture_name not in groups:
            groups[texture_name] = material
        old_slot_to_new_slot[old_slot] = list(groups.keys()).index(texture_name)

    if len(groups) != EXPECTED_MERGED_MATERIAL_COUNT:
        raise RuntimeError(
            "Expected "
            + str(EXPECTED_MERGED_MATERIAL_COUNT)
            + " base textures, found "
            + str(len(groups))
        )

    new_polygon_indices = []
    for polygon in mesh.data.polygons:
        new_polygon_indices.append(old_slot_to_new_slot[polygon.material_index])

    mesh.data.materials.clear()
    for material in groups.values():
        mesh.data.materials.append(material)
    for polygon, new_index in zip(mesh.data.polygons, new_polygon_indices):
        polygon.material_index = new_index

    retained_materials = set(groups.values())
    for material in original_materials:
        if material not in retained_materials and material.users == 0:
            bpy.data.materials.remove(material)

    return groups


def validate_merged_model(armature, mesh, faces_before):
    if len(armature.data.bones) != len(EXPORT_BONE_NAMES):
        raise RuntimeError("Unexpected reduced-rig bone count")
    if len(mesh.data.vertices) != EXPECTED_VERTEX_COUNT:
        raise RuntimeError("Vertex count changed while merging materials")
    if len(mesh.data.polygons) != EXPECTED_POLYGON_COUNT:
        raise RuntimeError("Polygon count changed while merging materials")
    if len(mesh.data.materials) != EXPECTED_MERGED_MATERIAL_COUNT:
        raise RuntimeError("Merged material count is incorrect")

    used_indices = {polygon.material_index for polygon in mesh.data.polygons}
    expected_indices = set(range(EXPECTED_MERGED_MATERIAL_COUNT))
    if used_indices != expected_indices:
        raise RuntimeError("One or more merged materials have no faces")

    faces_after = snapshot_faces_by_texture(mesh)
    if faces_before != faces_after:
        raise RuntimeError("Face-to-texture assignments changed during material merge")


def rename_bones_for_export(armature):
    for original_name, export_name in EXPORT_BONE_NAMES.items():
        bone = armature.data.bones.get(original_name)
        if bone is None:
            raise RuntimeError("Retained bone is missing before export: " + original_name)
        bone.name = export_name


def restore_bone_names(armature):
    for original_name, export_name in EXPORT_BONE_NAMES.items():
        bone = armature.data.bones.get(export_name)
        if bone is None:
            raise RuntimeError("Export bone is missing while restoring names: " + export_name)
        bone.name = original_name


def export_model(filepath, armature, mesh):
    bpy.ops.object.mode_set(mode="OBJECT")
    bpy.ops.object.select_all(action="DESELECT")
    armature.select_set(True)
    mesh.select_set(True)
    bpy.context.view_layer.objects.active = mesh

    result = bpy.ops.export_scene.directx_x(
        filepath=str(filepath),
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=True,
        export_weights=True,
        export_animation=False,
        anim_key_format="TRS",
        unweld_on_export=False,
        use_original_material_data=False,
        export_format="TEXT_X",
        pz_compat=False,
        anim_fps=30.0,
        anim_frame_start=0,
        anim_frame_end=0,
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(filepath))


def main():
    bpy.context.preferences.filepaths.save_version = 0
    armature = require_object(ARMATURE_NAME, "ARMATURE")
    mesh = require_object(MESH_NAME, "MESH")
    faces_before = snapshot_faces_by_texture(mesh)

    groups = merge_materials_by_base_texture(mesh)
    validate_merged_model(armature, mesh, faces_before)

    original_action = None
    if armature.animation_data is not None:
        original_action = armature.animation_data.action
        armature.animation_data.action = None
    original_pose_position = armature.data.pose_position
    armature.data.pose_position = "REST"
    bpy.context.scene.frame_set(0)

    rename_bones_for_export(armature)
    export_model(MODEL_X_PATH, armature, mesh)
    export_model(NON_ANIM_X_PATH, armature, mesh)
    restore_bone_names(armature)

    armature.data.pose_position = original_pose_position
    if armature.animation_data is not None:
        armature.animation_data.action = original_action
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))

    print("MARINE_MATERIALS_BEFORE", EXPECTED_MATERIAL_COUNT)
    print("MARINE_MATERIALS_AFTER", len(groups))
    print("MARINE_POLYGONS", len(mesh.data.polygons))
    print("MARINE_VERTICES", len(mesh.data.vertices))
    for texture_name, material in groups.items():
        print("MARINE_MATERIAL", texture_name, material.name)
    print("MARINE_MODEL_X", MODEL_X_PATH)
    print("MARINE_NON_ANIM_X", NON_ANIM_X_PATH)


if __name__ == "__main__":
    main()
