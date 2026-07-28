"""Fix the Skeleton enemy model orientation and origin.

Three problems identified in source.blend:

1. SIDEWAYS IN GAME: The top-level `Hips` bone carries a rest rotation of
   euler(90, 0, -90) degrees, while every other bone (Torso/Neck/Head)
   has identity rotation. This isolated rotation twists the whole
   hierarchy 90 degrees. Spider/Golem/Mushroom all have an identity
   top-level frame; only the Skeleton does not.

2. BURIED IN GROUND: SkeletonArmature's origin is at Z=1.355 (abdomen
   height), so the feet sit at Z=-1.629. In-game the origin is treated
   as the ground contact point, so the lower body sinks ~1.6m into the
   floor. Player/Spider keep the origin at the feet (Z=0).

3. NO ROOT BONE: Spider exports a top-level `Root` frame at the world
   origin with identity transform, then `Body` underneath. The Skeleton
   has no Root bone; `Hips` is the top level. Adding a Root bone (at the
   origin, identity rest, like Spider) and reparenting Hips under it
   makes the export structure match the working enemies.

This script performs (non-destructively, saving Skeleton_clean.blend):
  Step 1: Reset Hips rest rotation to identity via Apply-Pose-as-Rest.
  Step 2: Add a Root bone at the world origin, reparent Hips under it.
  Step 3: Translate bone rest matrices + mesh vertices up so the lowest
          foot vertex lands at Z=0; then ZERO the armature/mesh object
          locations so the exported top-level frame is at the origin
          with identity rotation (matching Spider).
  Step 4: Re-bake every action so the F-curves match the new rest pose.
  Step 5: Save as Skeleton_clean.blend (source.blend unchanged).

The game-side C++ is not touched.
"""
import os

import bpy


ARMATURE_NAME = "SkeletonArmature"


def reset_hips_rest(armature_obj):
    """Neutralize the Hips rest rotation by applying the inverse pose as rest."""
    hips = armature_obj.data.bones["Hips"]
    rest_rot = hips.matrix_local.decompose()[1]

    bpy.ops.object.select_all(action="DESELECT")
    armature_obj.select_set(True)
    bpy.context.view_layer.objects.active = armature_obj
    bpy.ops.object.mode_set(mode="POSE")

    pb = armature_obj.pose.bones["Hips"]
    pb.rotation_mode = "QUATERNION"
    pb.rotation_quaternion = rest_rot.inverted()
    bpy.context.view_layer.update()

    bpy.ops.pose.armature_apply(selected=False)
    bpy.ops.object.mode_set(mode="OBJECT")


def add_root_bone(armature_obj):
    """Add a Root bone at origin (identity rest) and reparent Hips under it."""
    bpy.ops.object.select_all(action="DESELECT")
    armature_obj.select_set(True)
    bpy.context.view_layer.objects.active = armature_obj
    bpy.ops.object.mode_set(mode="EDIT")

    edit_bones = armature_obj.data.edit_bones
    hips_eb = edit_bones["Hips"]

    root_eb = edit_bones.new("Root")
    root_eb.head = (0.0, 0.0, 0.0)
    root_eb.tail = (0.0, 0.0, 0.1)
    root_eb.roll = 0.0

    hips_eb.parent = root_eb

    bpy.ops.object.mode_set(mode="OBJECT")


def move_origin_to_feet(armature_obj, mesh_objects):
    """Lift the model so the lowest foot vertex is at Z=0.

    Steps:
      a. Apply each mesh's parent_inverse (transform_apply) so mesh local
         space aligns with armature local space - otherwise the .x exporter
         writes vertices with a stale offset baked into matrix_parent_inverse.
      b. Find the lowest mesh vertex in world space.
      c. Translate mesh vertices (now in armature-local space) and every
         bone rest head/tail by +delta in Z, EXCEPT the Root bone which
         must stay pinned at the origin.
      d. Zero the armature/mesh object locations so the exported top-level
         frame is at the origin with identity rotation (matching Spider).
    """
    # a. Collapse parent_inverse into vertices so local == armature-local.
    bpy.ops.object.select_all(action="DESELECT")
    for mesh_obj in mesh_objects:
        mesh_obj.select_set(True)
        bpy.context.view_layer.objects.active = mesh_obj
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bpy.ops.object.select_all(action="DESELECT")

    # b. World-space lowest vertex (armature is at origin after step a).
    lowest_z = None
    for mesh_obj in mesh_objects:
        for v in mesh_obj.data.vertices:
            wco = mesh_obj.matrix_world @ v.co
            if lowest_z is None or wco.z < lowest_z:
                lowest_z = wco.z

    if lowest_z is None or abs(lowest_z) < 1e-6:
        return 0.0

    delta = -lowest_z

    # c. Translate mesh vertices and bone rest by delta in Z (Root pinned).
    for mesh_obj in mesh_objects:
        for v in mesh_obj.data.vertices:
            v.co.z += delta
        mesh_obj.data.update()

    armature_obj.select_set(True)
    bpy.context.view_layer.objects.active = armature_obj
    bpy.ops.object.mode_set(mode="EDIT")
    for eb in armature_obj.data.edit_bones:
        if eb.name == "Root":
            continue
        eb.head.z += delta
        eb.tail.z += delta
    bpy.ops.object.mode_set(mode="OBJECT")

    # d. Zero object locations so the top-level .x frame is at origin.
    armature_obj.location = (0.0, 0.0, 0.0)
    for mesh_obj in mesh_objects:
        mesh_obj.location = (0.0, 0.0, 0.0)

    bpy.context.view_layer.update()
    return delta


def rebake_actions(armature_obj):
    """Rebake every action so F-curves match the new rest pose."""
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
    skeleton_dir = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        "..",
        "RedFortress2",
        "MultiPassRendering",
        "res",
        "model2",
        "Skeleton",
    )
    source_path = os.path.join(skeleton_dir, "source.blend")
    clean_path = os.path.join(skeleton_dir, "Skeleton_clean.blend")

    print(f"Loading: {source_path}")
    bpy.ops.wm.open_mainfile(filepath=source_path)

    armature_obj = bpy.data.objects.get(ARMATURE_NAME)
    if armature_obj is None or armature_obj.type != "ARMATURE":
        raise RuntimeError(f"Armature not found: {ARMATURE_NAME}")

    mesh_objects = [
        o for o in bpy.data.objects
        if o.type == "MESH" and o.parent == armature_obj
    ]
    if len(mesh_objects) == 0:
        raise RuntimeError("No mesh bound to armature")

    print("Step 1: reset Hips rest rotation to identity...")
    reset_hips_rest(armature_obj)

    print("Step 2: add Root bone and reparent Hips under it...")
    add_root_bone(armature_obj)

    print("Step 3: move origin to feet (Z=0), Root pinned at origin...")
    delta = move_origin_to_feet(armature_obj, mesh_objects)
    print(f"  translated by Z={delta:.3f}")

    print("Step 4: rebake all actions...")
    rebake_actions(armature_obj)

    print(f"Step 5: saving -> {clean_path}")
    bpy.ops.wm.save_as_mainfile(filepath=clean_path)
    print("Done.")


if __name__ == "__main__":
    main()
