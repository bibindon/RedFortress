import os
import sys

import bpy


ASSET_CONFIGS = {
    "crab": {
        "armature": "rig crab",
        "actions": {
            "idle": "idle",
            "move": "walk",
            "fast_move": "walk",
            "attack": "walk",
            "hit": "idle",
            "death": "idle",
        },
    },
    "frog": {
        "armature": "frog_armature",
        "actions": {
            "idle": "idle",
            "move": "walk",
            "fast_move": "jump",
            "attack": "jump",
            "hit": "land",
            "death": "land",
        },
    },
    "bird": {
        "armature": "bird-skeleton",
        "actions": {
            "idle": "idle",
            "move": "flap",
            "fast_move": "flap",
            "attack": "attack",
            "hit": "idle",
            "death": "loony",
        },
    },
    "ghost": {
        "armature": "CharacterArmature",
        "actions": {
            "idle": "CharacterArmature|Flying_Idle",
            "move": "CharacterArmature|Fast_Flying",
            "fast_move": "CharacterArmature|Fast_Flying",
            "attack": "CharacterArmature|Headbutt",
            "hit": "CharacterArmature|HitReact",
            "death": "CharacterArmature|Death",
        },
    },
}


def parse_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected arguments after --")

    arguments = sys.argv[sys.argv.index("--") + 1 :]
    if len(arguments) != 3:
        raise RuntimeError(
            "Usage: blender --background --python PrepareEnemyModels.py "
            "-- <asset> <source> <output>"
        )

    asset_name = arguments[0].lower()
    if asset_name not in ASSET_CONFIGS:
        raise RuntimeError(f"Unknown enemy asset: {asset_name}")

    source_path = os.path.abspath(arguments[1])
    output_directory = os.path.abspath(arguments[2])
    return asset_name, source_path, output_directory


def load_source(source_path):
    extension = os.path.splitext(source_path)[1].lower()
    if extension == ".blend":
        bpy.ops.wm.open_mainfile(filepath=source_path)
        return

    if extension == ".glb" or extension == ".gltf":
        bpy.ops.object.select_all(action="SELECT")
        bpy.ops.object.delete(use_global=False)
        result = bpy.ops.import_scene.gltf(filepath=source_path)
        if "FINISHED" not in result:
            raise RuntimeError(f"glTF import failed: {source_path}")
        return

    raise RuntimeError(f"Unsupported source format: {source_path}")


def find_export_objects(armature_name):
    armature = bpy.data.objects.get(armature_name)
    if armature is None or armature.type != "ARMATURE":
        raise RuntimeError(f"Armature was not found: {armature_name}")

    mesh_objects = []
    for scene_object in bpy.context.scene.objects:
        if scene_object.type != "MESH":
            continue

        is_bound = scene_object.parent == armature
        for modifier in scene_object.modifiers:
            if modifier.type == "ARMATURE" and modifier.object == armature:
                is_bound = True
                break

        parent = scene_object.parent
        while parent is not None:
            if parent == armature:
                is_bound = True
                break
            parent = parent.parent

        if is_bound:
            mesh_objects.append(scene_object)

    if len(mesh_objects) == 0:
        raise RuntimeError(f"No mesh is bound to armature: {armature_name}")

    if armature.animation_data is None:
        armature.animation_data_create()

    return armature, mesh_objects


def select_export_objects(armature, mesh_objects):
    bpy.ops.object.select_all(action="DESELECT")
    armature.hide_viewport = False
    armature.hide_set(False)
    armature.select_set(True)
    for mesh_object in mesh_objects:
        mesh_object.hide_viewport = False
        mesh_object.hide_set(False)
        mesh_object.select_set(True)
    bpy.context.view_layer.objects.active = armature


def normalize_x_file(path):
    with open(path, "rb") as source_file:
        data = source_file.read()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    with open(path, "wb") as destination_file:
        destination_file.write(data)


def set_action(armature, action):
    armature.animation_data.action = action
    frame_start = int(action.frame_range[0])
    frame_end = int(action.frame_range[1])
    bpy.context.scene.frame_start = frame_start
    bpy.context.scene.frame_end = frame_end
    bpy.context.scene.frame_set(frame_start)
    bpy.context.view_layer.update()
    return frame_start, frame_end


def export_x(path, armature, mesh_objects, export_animation, frame_start, frame_end):
    select_export_objects(armature, mesh_objects)

    arguments = {
        "filepath": path,
        "check_existing": False,
        "use_selection": True,
        "axis_forward": "-Z",
        "axis_up": "Y",
        "export_animation": export_animation,
    }
    if export_animation:
        arguments["anim_fps"] = 30.0
        arguments["anim_frame_start"] = frame_start
        arguments["anim_frame_end"] = frame_end

    result = bpy.ops.export_scene.directx_x(**arguments)
    if "FINISHED" not in result:
        raise RuntimeError(f"DirectX X export failed: {path}")
    normalize_x_file(path)


def write_animation_csv(output_directory):
    rows = [
        'Anim, "000", "enemy.default.x", default',
        'Anim, "idle", "enemy.idle.x", loop',
        'Anim, "walk", "enemy.move.x", loop',
        'Anim, "creep", "enemy.move.x", loop',
        'Anim, "run", "enemy.fast_move.x", loop',
        'Anim, "attack", "enemy.attack.x", stopWhenEnd',
        'Anim, "hit", "enemy.hit.x", stopWhenEnd',
        'Anim, "death", "enemy.death.x", stopWhenEnd',
    ]
    output_path = os.path.join(output_directory, "enemy.csv")
    data = ("\r\n".join(rows) + "\r\n").encode("utf-8")
    with open(output_path, "wb") as output_file:
        output_file.write(data)


def main():
    asset_name, source_path, output_directory = parse_arguments()
    config = ASSET_CONFIGS[asset_name]
    os.makedirs(output_directory, exist_ok=True)

    load_source(source_path)
    armature, mesh_objects = find_export_objects(config["armature"])

    action_map = {}
    for logical_name, source_name in config["actions"].items():
        action = bpy.data.actions.get(source_name)
        if action is None:
            raise RuntimeError(
                f"Action {source_name} for {asset_name}:{logical_name} was not found"
            )
        action_map[logical_name] = action

    idle_action = action_map["idle"]
    idle_start, idle_end = set_action(armature, idle_action)

    base_path = os.path.join(output_directory, "enemy.x")
    export_x(base_path, armature, mesh_objects, False, idle_start, idle_end)

    default_path = os.path.join(output_directory, "enemy.default.x")
    export_x(default_path, armature, mesh_objects, True, idle_start, idle_start)

    for logical_name, action in action_map.items():
        frame_start, frame_end = set_action(armature, action)
        filename = f"enemy.{logical_name}.x"
        output_path = os.path.join(output_directory, filename)
        export_x(
            output_path,
            armature,
            mesh_objects,
            True,
            frame_start,
            frame_end,
        )
        print(
            f"EXPORTED {asset_name} {logical_name} "
            f"{action.name} {frame_start} {frame_end}"
        )

    write_animation_csv(output_directory)
    print(
        f"PREPARED {asset_name} meshes={len(mesh_objects)} "
        f"output={output_directory}"
    )


main()
