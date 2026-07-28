"""Remove IK control bones from the Spider model and rebake FK animation.

Root cause of the in-game display bug:
The Spider's `.x` exports 39 bones, including 10 IK control bones
(FrontFoot.L/R, MidFrontFoot.L/R, BackFoot.L/R, MidBackFoot.L/R,
PoleTarget.L/R) that are parented to `Root` and disconnected from the
leg chains (FrontLeg.L -> FrontLeg2.L -> FrontLeg3.L). Those IK bones
are registered in `SkinWeights` and deform ~800 leg-tip vertices each
via location-only keys, so the leg-tip vertices detach from the chain
rotation and jitter on their own.

This script:
1. Migrates each IK bone's vertex weights into the matching leg tip
   bone's vertex group (FrontFoot.L -> FrontLeg3.L, etc.), normalizes,
   then removes the IK vertex groups.
2. Deletes the 10 IK bones (and their `*_end` Empty helpers) from the
   armature and scene.
3. Rebakes every action with visual_keying so the leg chain rotations
   become clean FK keyframes with no IK residue.
4. Saves the cleaned model as `Spider_clean.blend` (source.blend is
   never modified).

The game-side C++ loader is not touched. The exported `.x` matches the
Skeleton/Golem structure (no IK bones), so MeshMixSkinAnim2's
Blender512Custom loader works unchanged.
"""
import os

import bpy


# IK control bones to remove, mapped to the leg-tip bone that absorbs
# their vertex weights. Order does not matter.
IK_BONE_TO_LEG_TIP = {
    "FrontFoot.L":    "FrontLeg3.L",
    "FrontFoot2.R":   "FrontLeg3.R",
    "MidFrontFoot.L": "MidFrontLeg3.L",
    "MidFrontFoot.R": "MidFrontLeg3.R",
    "BackFoot.L":     "BackLeg3.L",
    "BackFoot.R":     "BackLeg3.R",
    "MidBackFoot.L":  "MidBackLeg3.L",
    "MidBackFoot.R":  "MidBackLeg3.R",
}

# Pole targets carry no vertex weight but are still removed as bones.
IK_BONES_NO_WEIGHT = ["PoleTarget.L", "PoleTarget.R"]

ARMATURE_NAME = "SpiderArmature"
MESH_NAME = "Cube"


def migrate_vertex_weights(mesh_obj, ik_to_tip):
    """Add each IK group's weight into the leg tip group, then remove IK groups."""
    if mesh_obj.type != "MESH":
        return

    groups = {vg.name: vg for vg in mesh_obj.vertex_groups}

    for ik_name, tip_name in ik_to_tip.items():
        ik_group = groups.get(ik_name)
        tip_group = groups.get(tip_name)
        if ik_group is None:
            continue

        # Build a per-vertex weight map for the IK group (only vertices
        # that have a non-zero weight in the IK group).
        ik_weights = {}
        for v in mesh_obj.data.vertices:
            for el in v.groups:
                if el.group == ik_group.index and el.weight > 0.0:
                    ik_weights[v.index] = el.weight
                    break

        if ik_weights and tip_group is not None:
            # Add the IK weight into the tip group. add(weight=0) would
            # replace, so we must read existing first then re-add the sum.
            for v in mesh_obj.data.vertices:
                added = ik_weights.get(v.index)
                if added is None:
                    continue
                existing = 0.0
                for el in v.groups:
                    if el.group == tip_group.index:
                        existing = el.weight
                        break
                tip_group.add([v.index], existing + added, "REPLACE")

        migrated = len(ik_weights)
        print(f"  weight: {ik_name} -> {tip_name}  ({migrated} verts migrated)")

    # Remove all IK vertex groups (including the no-weight pole targets).
    all_ik = list(ik_to_tip.keys()) + IK_BONES_NO_WEIGHT
    for ik_name in all_ik:
        vg = groups.get(ik_name)
        if vg is not None:
            mesh_obj.vertex_groups.remove(vg)
            print(f"  removed vertex group: {ik_name}")

    # Rebuild group-index -> group-object map (indices shifted after removal).
    group_by_index = {vg.index: vg for vg in mesh_obj.vertex_groups}

    # Normalize each vertex's remaining weights so they sum to 1.0.
    for v in mesh_obj.data.vertices:
        total = 0.0
        contrib = []
        for el in v.groups:
            vg = group_by_index.get(el.group)
            if vg is None:
                continue
            if el.weight > 0.0:
                total += el.weight
                contrib.append((el, vg, el.weight))
        if total > 0.0:
            scale = 1.0 / total
            for el, vg, w in contrib:
                vg.add([v.index], w * scale, "REPLACE")


def delete_ik_bones_and_empties(armature_obj, ik_to_tip):
    """Remove IK bones from edit mode and delete matching _end Empties."""
    all_ik = list(ik_to_tip.keys()) + IK_BONES_NO_WEIGHT

    # Delete the *_end Empty helpers that are children of the armature.
    end_names = {f"{name}_end" for name in all_ik}
    bpy.ops.object.select_all(action="DESELECT")
    deleted_empties = 0
    for obj in list(bpy.data.objects):
        if obj.type == "EMPTY" and obj.name in end_names:
            bpy.data.objects.remove(obj, do_unlink=True)
            deleted_empties += 1
    print(f"  deleted {deleted_empties} _end empties")

    # Delete the bones in edit mode.
    bpy.context.view_layer.objects.active = armature_obj
    armature_obj.select_set(True)
    bpy.ops.object.mode_set(mode="EDIT")
    edit_bones = armature_obj.data.edit_bones
    removed = 0
    for ik_name in all_ik:
        eb = edit_bones.get(ik_name)
        if eb is not None:
            edit_bones.remove(eb)
            removed += 1
    bpy.ops.object.mode_set(mode="OBJECT")
    print(f"  deleted {removed} IK bones")


def rebake_actions(armature_obj):
    """Bake every action with visual keying so FK chains are clean."""
    if armature_obj.animation_data is None:
        armature_obj.animation_data_create()

    baked = 0
    for action in list(bpy.data.actions):
        frame_start = int(action.frame_range[0])
        frame_end = int(action.frame_range[1])
        if frame_end <= frame_start:
            continue

        armature_obj.animation_data.action = action
        bpy.context.scene.frame_set(frame_start)
        bpy.context.view_layer.update()

        bpy.ops.object.select_all(action="DESELECT")
        armature_obj.select_set(True)
        bpy.context.view_layer.objects.active = armature_obj

        bpy.ops.object.mode_set(mode="POSE")
        bpy.ops.pose.select_all(action="SELECT")

        bpy.ops.nla.bake(
            frame_start=frame_start,
            frame_end=frame_end,
            only_selected=True,
            visual_keying=True,
            clear_constraints=False,
            use_current_action=True,
            bake_types={"POSE"},
        )

        bpy.ops.object.mode_set(mode="OBJECT")
        baked += 1
        print(f"  baked: {action.name} ({frame_start}-{frame_end})")

    return baked


def main():
    spider_dir = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        "..",
        "RedFortress2",
        "MultiPassRendering",
        "res",
        "model2",
        "Spider",
    )
    source_path = os.path.join(spider_dir, "source.blend")
    clean_path = os.path.join(spider_dir, "Spider_clean.blend")

    print(f"Loading: {source_path}")
    bpy.ops.wm.open_mainfile(filepath=source_path)

    armature_obj = bpy.data.objects.get(ARMATURE_NAME)
    if armature_obj is None or armature_obj.type != "ARMATURE":
        raise RuntimeError(f"Armature not found: {ARMATURE_NAME}")
    mesh_obj = bpy.data.objects.get(MESH_NAME)
    if mesh_obj is None:
        raise RuntimeError(f"Mesh not found: {MESH_NAME}")

    bone_names_before = [b.name for b in armature_obj.data.bones]

    print("Step 1: migrate IK vertex weights into leg-tip groups...")
    migrate_vertex_weights(mesh_obj, IK_BONE_TO_LEG_TIP)

    print("Step 2: delete IK bones and _end empties...")
    delete_ik_bones_and_empties(armature_obj, IK_BONE_TO_LEG_TIP)

    bone_names_after = [b.name for b in armature_obj.data.bones]
    print(f"  bones: {len(bone_names_before)} -> {len(bone_names_after)}")

    print("Step 3: rebake all actions as clean FK...")
    rebake_actions(armature_obj)

    print(f"Step 4: saving cleaned model -> {clean_path}")
    bpy.ops.wm.save_as_mainfile(filepath=clean_path)
    print("Done.")


if __name__ == "__main__":
    main()
