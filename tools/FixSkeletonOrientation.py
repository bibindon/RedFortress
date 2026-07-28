"""Fix the Skeleton enemy model orientation and origin (safe approach).

Problems in source.blend:
  1. SIDEWAYS IN GAME: the top-level `Hips` bone carries a rest rotation,
     so the .x top-level frame is rotated 90deg. Spider/Golem/Mushroom
     export an identity top-level frame.
  2. BURIED IN GROUND: SkeletonArmature origin is at Z=1.355 (abdomen);
     feet sit at Z=-1.629. The game treats the origin as ground contact.

Safe fix (does NOT use Apply-Pose-as-Rest, which would scramble the
existing animation F-curves and detach limbs from the body):

  Step 1: Add a `Root` bone at the world origin with identity rest, and
          reparent `Hips` under it. The Hips rest rotation is left as-is;
          now the top-level frame is Root (identity), exactly like Spider.
          Hips' rotation becomes equivalent to Spider's `Body` rotation.
  Step 2: Translate every bone rest head/tail (except Root, which stays
          pinned at origin) and every mesh vertex by +delta in Z so the
          lowest foot vertex lands at Z=0. Zero the object locations so
          the exported top-level frame is at the origin.

Because the bone rest ROTATIONS and relative positions are unchanged,
the existing walk/attack/idle F-curves keep working unchanged. No
re-baking is needed.

Saves Skeleton_clean.blend (source.blend is never modified).
"""
import os

import bpy


ARMATURE_NAME = "SkeletonArmature"


def add_root_bone(armature_obj):
    """Add a Root bone at the origin (identity rest) and reparent Hips under it."""
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

    a. transform_apply each mesh so its local space aligns with armature
       local space (collapses matrix_parent_inverse into vertices).
    b. Find the lowest mesh vertex in world space.
    c. Translate mesh vertices + every bone rest head/tail (except Root,
       pinned at origin) by +delta in Z.
    d. Zero the object locations so the exported top-level frame is at
       the origin with identity rotation.
    """
    # a. Collapse parent_inverse into mesh vertices.
    bpy.ops.object.select_all(action="DESELECT")
    for mesh_obj in mesh_objects:
        mesh_obj.select_set(True)
        bpy.context.view_layer.objects.active = mesh_obj
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bpy.ops.object.select_all(action="DESELECT")

    # b. World-space lowest vertex.
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

    print("Step 1: add Root bone (identity rest), reparent Hips under it...")
    add_root_bone(armature_obj)

    print("Step 2: move origin to feet (Z=0), Root pinned at origin...")
    delta = move_origin_to_feet(armature_obj, mesh_objects)
    print(f"  translated by Z={delta:.3f}")

    print(f"Step 3: saving -> {clean_path}")
    bpy.ops.wm.save_as_mainfile(filepath=clean_path)
    print("Done. (Hips rest rotation and all animation F-curves preserved.)")


if __name__ == "__main__":
    main()
