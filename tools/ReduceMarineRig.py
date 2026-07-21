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

# These are the original MMD armature indices. Keeping the original number in
# the exported Bone_NNN name preserves compatibility with the existing X
# animation files, while the unused animation channels are ignored by the
# renderer.
KEEP_BONE_INDICES = {
    0,    # 全ての親
    1,    # センター
    2,    # グルーブ
    3,    # 上半身
    4,    # 上半身2
    5,    # 首
    6,    # 頭
    152,  # 肩.L
    153,  # 腕.L
    154,  # 腕捩.L
    155,  # ひじ.L
    156,  # 手捩.L
    157,  # 手首.L
    237,  # 肩.R
    238,  # 腕.R
    239,  # 腕捩.R
    240,  # ひじ.R
    241,  # 手捩.R
    242,  # 手首.R
    444,  # 下半身
    445,  # 足.L
    446,  # ひざ.L
    447,  # 足首.L
    448,  # つま先.L
    457,  # 足.R
    458,  # ひざ.R
    459,  # 足首.R
    460,  # つま先.R
}


def require_object(name, object_type):
    obj = bpy.data.objects.get(name)
    if obj is None:
        raise RuntimeError("Required Blender object is missing: " + name)
    if obj.type != object_type:
        raise RuntimeError("Unexpected object type for " + name + ": " + obj.type)
    return obj


def get_bone_index_maps(armature):
    bones = list(armature.data.bones)
    if len(bones) != 549:
        raise RuntimeError("Expected 549 source bones, found " + str(len(bones)))
    if max(KEEP_BONE_INDICES) >= len(bones):
        raise RuntimeError("A retained bone index is outside the armature")

    name_to_index = {}
    index_to_name = {}
    for index, bone in enumerate(bones):
        name_to_index[bone.name] = index
        index_to_name[index] = bone.name
    return name_to_index, index_to_name


def find_weight_targets(armature, name_to_index):
    targets = {}
    for bone in armature.data.bones:
        bone_index = name_to_index[bone.name]
        if bone_index in KEEP_BONE_INDICES:
            continue

        parent = bone.parent
        while parent is not None:
            parent_index = name_to_index[parent.name]
            if parent_index in KEEP_BONE_INDICES:
                targets[bone.name] = parent.name
                break
            parent = parent.parent

        if bone.name not in targets:
            targets[bone.name] = index_to_name_checked(name_to_index, 0)
    return targets


def index_to_name_checked(name_to_index, requested_index):
    for name, index in name_to_index.items():
        if index == requested_index:
            return name
    raise RuntimeError("Bone index was not found: " + str(requested_index))


def snapshot_vertex_totals(mesh):
    totals = []
    for vertex in mesh.data.vertices:
        total = 0.0
        for group_element in vertex.groups:
            total += group_element.weight
        totals.append(total)
    return totals


def transfer_removed_weights(mesh, weight_targets):
    group_names = {}
    for vertex_group in mesh.vertex_groups:
        group_names[vertex_group.index] = vertex_group.name

    transfers = {}
    for vertex in mesh.data.vertices:
        for group_element in vertex.groups:
            source_name = group_names[group_element.group]
            target_name = weight_targets.get(source_name)
            if target_name is None:
                continue
            target_transfers = transfers.setdefault(target_name, [])
            target_transfers.append((vertex.index, group_element.weight))

    for target_name, target_transfers in transfers.items():
        target_group = mesh.vertex_groups.get(target_name)
        if target_group is None:
            target_group = mesh.vertex_groups.new(name=target_name)
        for vertex_index, weight in target_transfers:
            target_group.add([vertex_index], weight, "ADD")

    groups_to_remove = []
    for vertex_group in mesh.vertex_groups:
        if vertex_group.name in weight_targets:
            groups_to_remove.append(vertex_group)
    for vertex_group in groups_to_remove:
        mesh.vertex_groups.remove(vertex_group)


def remove_unused_mmd_objects(armature, mesh):
    objects_to_keep = {armature, mesh}
    parent = armature.parent
    while parent is not None:
        objects_to_keep.add(parent)
        parent = parent.parent
    for obj in bpy.data.objects:
        if obj.type in {"CAMERA", "LIGHT"}:
            objects_to_keep.add(obj)

    objects_to_remove = []
    for obj in bpy.data.objects:
        if obj not in objects_to_keep:
            objects_to_remove.append(obj)
    for obj in objects_to_remove:
        bpy.data.objects.remove(obj, do_unlink=True)
    return len(objects_to_remove)


def remove_pose_constraints(armature):
    removed_count = 0
    for pose_bone in armature.pose.bones:
        constraints_to_remove = list(pose_bone.constraints)
        for constraint in constraints_to_remove:
            pose_bone.constraints.remove(constraint)
            removed_count += 1
    return removed_count


def delete_removed_bones(armature, name_to_index):
    bpy.context.view_layer.objects.active = armature
    armature.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")

    names_to_remove = []
    for edit_bone in armature.data.edit_bones:
        bone_index = name_to_index[edit_bone.name]
        if bone_index not in KEEP_BONE_INDICES:
            names_to_remove.append(edit_bone.name)

    for name in reversed(names_to_remove):
        edit_bone = armature.data.edit_bones.get(name)
        if edit_bone is not None:
            armature.data.edit_bones.remove(edit_bone)

    bpy.ops.object.mode_set(mode="OBJECT")


def validate_reduced_rig(armature, mesh, totals_before):
    if len(armature.data.bones) != len(KEEP_BONE_INDICES):
        raise RuntimeError(
            "Reduced armature has "
            + str(len(armature.data.bones))
            + " bones; expected "
            + str(len(KEEP_BONE_INDICES))
        )

    retained_names = {bone.name for bone in armature.data.bones}
    weighted_names = set()
    for vertex_group in mesh.vertex_groups:
        has_weight = False
        for vertex in mesh.data.vertices:
            try:
                if vertex_group.weight(vertex.index) > 0.0:
                    has_weight = True
                    break
            except RuntimeError:
                continue
        if has_weight:
            weighted_names.add(vertex_group.name)

    bone_weight_names = weighted_names & retained_names
    unknown_weight_groups = {
        name
        for name in weighted_names - retained_names
        if not name.startswith("mmd_")
    }
    if unknown_weight_groups:
        raise RuntimeError(
            "Weighted vertex groups without retained bones: "
            + ", ".join(sorted(unknown_weight_groups))
        )

    totals_after = snapshot_vertex_totals(mesh)
    if len(totals_before) != len(totals_after):
        raise RuntimeError("Vertex count changed while reducing the rig")

    maximum_error = 0.0
    unweighted_vertices = 0
    for index, total_after in enumerate(totals_after):
        maximum_error = max(maximum_error, abs(totals_before[index] - total_after))
        if total_after <= 0.000001:
            unweighted_vertices += 1
    if maximum_error > 0.0001:
        raise RuntimeError("Vertex weight totals changed by " + str(maximum_error))
    if unweighted_vertices != 0:
        raise RuntimeError("Unweighted vertices found: " + str(unweighted_vertices))

    return len(bone_weight_names), maximum_error


def rename_for_export(armature, index_to_name):
    original_to_export = {}
    for index in sorted(KEEP_BONE_INDICES):
        original_name = index_to_name[index]
        export_name = "Bone_" + str(index).zfill(3)
        original_to_export[original_name] = export_name

    for original_name, export_name in original_to_export.items():
        bone = armature.data.bones.get(original_name)
        if bone is None:
            raise RuntimeError("Retained bone is missing before export: " + original_name)
        bone.name = export_name
    return original_to_export


def restore_names_after_export(armature, original_to_export):
    for original_name, export_name in original_to_export.items():
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
    name_to_index, index_to_name = get_bone_index_maps(armature)
    totals_before = snapshot_vertex_totals(mesh)
    weight_targets = find_weight_targets(armature, name_to_index)

    transfer_removed_weights(mesh, weight_targets)
    removed_object_count = remove_unused_mmd_objects(armature, mesh)
    removed_constraint_count = remove_pose_constraints(armature)
    delete_removed_bones(armature, name_to_index)
    weighted_bone_count, maximum_weight_error = validate_reduced_rig(
        armature, mesh, totals_before
    )

    original_action = None
    if armature.animation_data is not None:
        original_action = armature.animation_data.action
        armature.animation_data.action = None
    original_pose_position = armature.data.pose_position
    armature.data.pose_position = "REST"
    bpy.context.scene.frame_set(0)

    original_to_export = rename_for_export(armature, index_to_name)
    export_model(MODEL_X_PATH, armature, mesh)
    export_model(NON_ANIM_X_PATH, armature, mesh)
    restore_names_after_export(armature, original_to_export)

    armature.data.pose_position = original_pose_position
    if armature.animation_data is not None:
        armature.animation_data.action = original_action
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))

    print("MARINE_RIG_SOURCE_BONES", len(name_to_index))
    print("MARINE_RIG_RETAINED_BONES", len(armature.data.bones))
    print("MARINE_RIG_WEIGHTED_BONES", weighted_bone_count)
    print("MARINE_RIG_REMOVED_OBJECTS", removed_object_count)
    print("MARINE_RIG_REMOVED_CONSTRAINTS", removed_constraint_count)
    print("MARINE_RIG_MAX_WEIGHT_ERROR", format(maximum_weight_error, ".9f"))
    print("MARINE_RIG_MODEL_X", MODEL_X_PATH)
    print("MARINE_RIG_NON_ANIM_X", NON_ANIM_X_PATH)


if __name__ == "__main__":
    main()
