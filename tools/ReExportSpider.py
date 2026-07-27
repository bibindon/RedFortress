"""Re-export Spider X files with IK baked before export.

Bakes all animation actions so that IK/constraint-driven bone motion
is converted to actual bone keyframes. Then exports using the same
logic as PrepareEnemyModels.py.
"""
import bpy
import os
import re
import sys


def bake_all_actions(armature):
    """Bake all actions to convert IK/constraint motion to keyframes."""
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

        # Bake the action: converts IK/constraints to actual keyframes
        bpy.ops.nla.bake(
            frame_start=frame_start,
            frame_end=frame_end,
            only_selected=True,
            visual_keying=True,
            clear_constraints=True,
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

    # Bake IK/constraints for all actions
    print("Baking IK for all actions...")
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
