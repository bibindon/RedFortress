"""Re-export Spider X files with IK constraints rebuilt and baked.

The original source.blend has IK control bones (FrontFoot.L, etc.) but
no IK constraints on the leg chain tips. This script:
1. Reconstructs IK constraints on each leg's tip bone (segment 3)
2. Bakes only the tip bones (segment 3) so IK-driven motion becomes keyframes
3. Exports using the same logic as PrepareEnemyModels.py

Uses use_current_action=True to preserve the original F-curve animation
on bones that already animate correctly (segments 1 and 2).
Does NOT use clear_constraints to avoid modifying source.blend.
"""
import bpy
import os
import re
import sys

# IK chain definitions: (tip_bone_name, ik_target_bone, pole_target_bone)
IK_CHAINS = [
    ("FrontLeg3.L",    "FrontFoot.L",    "PoleTarget.L"),
    ("FrontLeg3.R",    "FrontFoot2.R",   "PoleTarget.R"),
    ("MidFrontLeg3.L", "MidFrontFoot.L", "PoleTarget.L"),
    ("MidFrontLeg3.R", "MidFrontFoot.R", "PoleTarget.R"),
    ("BackLeg3.L",     "BackFoot.L",     "PoleTarget.L"),
    ("BackLeg3.R",     "BackFoot.R",     "PoleTarget.R"),
    ("MidBackLeg3.L",  "MidBackFoot.L",  "PoleTarget.L"),
    ("MidBackLeg3.R",  "MidBackFoot.R",  "PoleTarget.R"),
]

# Only the tip bones (segment 3) need IK baking
TIP_BONES = [name for name, _, _ in IK_CHAINS]


def rebuild_ik_constraints(armature):
    """Add IK constraints to leg tip bones targeting the IK control bones."""
    added = 0

    for tip_name, target_name, pole_name in IK_CHAINS:
        tip_pbone = armature.pose.bones.get(tip_name)
        target_pbone = armature.pose.bones.get(target_name)
        pole_pbone = armature.pose.bones.get(pole_name)

        if tip_pbone is None:
            print(f"  WARNING: tip bone '{tip_name}' not found, skipping")
            continue
        if target_pbone is None:
            print(f"  WARNING: target bone '{target_name}' not found, skipping")
            continue

        # Remove any existing constraints on this bone
        for c in list(tip_pbone.constraints):
            tip_pbone.constraints.remove(c)

        # Add IK constraint
        ik = tip_pbone.constraints.new("IK")
        ik.name = "IK"
        ik.target = armature
        ik.subtarget = target_name
        ik.chain_count = 3
        ik.use_stretch = False

        if pole_pbone is not None:
            ik.pole_target = armature
            ik.pole_subtarget = pole_name
            ik.pole_angle = 0.0

        added += 1
        print(f"  Added IK: {tip_name} -> {target_name} (pole: {pole_name})")

    return added


def bake_all_actions(armature):
    """Bake all actions with visual keying, preserving original animation.

    Uses use_current_action=True to overwrite F-curves with IK-evaluated values.
    Uses clear_constraints=False to keep source.blend unmodified.
    """
    if armature.animation_data is None:
        armature.animation_data_create()

    baked_count = 0
    for action in list(bpy.data.actions):
        frame_start = int(action.frame_range[0])
        frame_end = int(action.frame_range[1])
        if frame_end <= frame_start:
            continue

        # Set this action on the armature
        armature.animation_data.action = action
        bpy.context.scene.frame_set(frame_start)
        bpy.context.view_layer.update()

        # Select the armature
        bpy.ops.object.select_all(action="DESELECT")
        armature.select_set(True)
        bpy.context.view_layer.objects.active = armature

        # Go to pose mode and select all bones
        bpy.ops.object.mode_set(mode="POSE")
        bpy.ops.pose.select_all(action="SELECT")

        # Bake: visual_keying captures IK-evaluated pose
        # use_current_action=True: overwrites existing F-curves
        # clear_constraints=False: keeps IK constraints (does not modify blend file)
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
        baked_count += 1
        print(f"  Baked: {action.name} ({frame_start}-{frame_end})")

    return baked_count


def main():
    source_path = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        "..",
        "RedFortress2",
        "MultiPassRendering",
        "res",
        "model2",
        "Spider",
        "source.blend",
    )
    output_dir = os.path.join(
        os.path.dirname(os.path.abspath(__file__)),
        "..",
        "RedFortress2",
        "MultiPassRendering",
        "res",
        "model2",
        "Spider",
    )

    print(f"Loading: {source_path}")
    bpy.ops.wm.open_mainfile(filepath=source_path)

    armature = bpy.data.objects.get("SpiderArmature")
    if armature is None:
        raise RuntimeError("SpiderArmature not found")

    # Step 1: Rebuild IK constraints
    print("Rebuilding IK constraints...")
    ik_count = rebuild_ik_constraints(armature)
    print(f"  Added {ik_count} IK constraint(s)")

    # Step 2: Bake all actions with IK (clear_constraints=False)
    print("Baking all actions with IK...")
    baked = bake_all_actions(armature)
    print(f"  Baked {baked} action(s)")

    # Load PrepareEnemyModels.py source (strip bare main() call)
    script_dir = os.path.dirname(os.path.abspath(__file__))
    prepare_path = os.path.join(script_dir, "PrepareEnemyModels.py")
    with open(prepare_path, encoding="utf-8-sig") as f:
        source = f.read()
    source = re.sub(r"\nmain\(\)\s*$", "\n", source)
    namespace = {"__name__": "prepare", "__file__": prepare_path}
    exec(compile(source, prepare_path, "exec"), namespace)
    prepare = type(sys)("prepare")
    prepare.__dict__.update(namespace)

    # Export using the same logic as PrepareEnemyModels.py
    config = prepare.ASSET_CONFIGS["spider"]
    armature_obj, mesh_objects = prepare.find_export_objects(config["armature"])
    prepare.validate_export_objects(armature_obj, mesh_objects)

    material_power = config.get("material_power")
    if material_power is not None:
        prepare.set_material_power(mesh_objects, material_power)

    action_map = {}
    for logical_name, source_name in config["actions"].items():
        action = bpy.data.actions.get(source_name)
        if action is None:
            raise RuntimeError(f"Action {source_name} not found")
        action_map[logical_name] = action

    idle_action = action_map["idle"]
    idle_start, idle_end = prepare.set_action(armature_obj, idle_action)

    # Export base mesh (no animation)
    base_path = os.path.join(output_dir, "enemy.x")
    prepare.export_x(
        base_path, armature_obj, mesh_objects,
        False, idle_start, idle_end,
        material_power, None, None, None,
    )
    print(f"  Exported: {base_path}")

    # Export default animation (single frame)
    default_path = os.path.join(output_dir, "enemy.default.x")
    prepare.export_x(
        default_path, armature_obj, mesh_objects,
        True, idle_start, idle_start,
        material_power, None, None, None,
    )
    print(f"  Exported: {default_path}")

    # Export each animation
    for logical_name, action in action_map.items():
        frame_start, frame_end = prepare.set_action(armature_obj, action)
        filename = f"enemy.{logical_name}.x"
        output_path = os.path.join(output_dir, filename)
        prepare.export_x(
            output_path, armature_obj, mesh_objects,
            True, frame_start, frame_end,
            material_power, None, None, None,
        )
        print(f"  Exported: {output_path} ({action.name} {frame_start}-{frame_end})")

    # Write animation CSV
    prepare.write_animation_csv(output_dir)
    print(f"  Wrote: {os.path.join(output_dir, 'enemy.csv')}")
    print("Done.")


if __name__ == "__main__":
    main()
