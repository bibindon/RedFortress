import os
import re
import sys

import bpy


DIRECTX_X_AXIS_FORWARD = "Z"
DIRECTX_X_AXIS_UP = "Y"
TRANSFORM_EPSILON = 0.0001


ASSET_CONFIGS = {
    "crab": {
        "armature": "rig crab",
        "material_power": 500.0,
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
        "material_power": 500.0,
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


def validate_export_objects(armature, mesh_objects):
    export_objects = [armature]
    export_objects.extend(mesh_objects)

    for export_object in export_objects:
        for rotation in export_object.rotation_euler:
            if abs(rotation) > TRANSFORM_EPSILON:
                raise RuntimeError(
                    f"Object rotation must be applied before export: "
                    f"{export_object.name}"
                )

        for scale in export_object.scale:
            if scale <= TRANSFORM_EPSILON:
                raise RuntimeError(
                    f"Object scale must be positive before export: "
                    f"{export_object.name}"
                )

        determinant = export_object.matrix_world.to_3x3().determinant()
        if determinant <= TRANSFORM_EPSILON:
            raise RuntimeError(
                f"Object transform is mirrored or degenerate: {export_object.name}"
            )


def set_material_power(mesh_objects, material_power):
    material_count = 0
    for mesh_object in mesh_objects:
        for material in mesh_object.data.materials:
            if material is None:
                continue
            material["_x_power"] = material_power
            material_count += 1

    if material_count == 0:
        raise RuntimeError("No material was found for the requested material power")


def normalize_x_file(path):
    with open(path, "rb") as source_file:
        data = source_file.read()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    with open(path, "wb") as destination_file:
        destination_file.write(data)


def validate_x_file(path, export_animation, material_power):
    with open(path, "rb") as source_file:
        data = source_file.read()

    if data.startswith(b"\xef\xbb\xbf"):
        raise RuntimeError(f"DirectX X file must not contain a UTF-8 BOM: {path}")
    if not data.startswith(b"xof "):
        raise RuntimeError(f"DirectX X header was not found: {path}")
    if b"\n" in data.replace(b"\r\n", b""):
        raise RuntimeError(f"DirectX X file must use CRLF line endings: {path}")

    text = data.decode("utf-8")
    skin_weights_pattern = (
        r'SkinWeights\s*\{\s*"[^"]+";\s*[1-9][0-9]*;'
    )
    if re.search(skin_weights_pattern, text) is None:
        raise RuntimeError(f"Non-empty SkinWeights block was not found: {path}")

    if material_power is not None:
        material_power_pattern = (
            r"Material\s+[^{]+\{\s+[^\r\n]+\s+"
            r"([+-]?[0-9]+(?:\.[0-9]+)?);"
        )
        exported_powers = re.findall(material_power_pattern, text)
        if len(exported_powers) == 0:
            raise RuntimeError(f"Material power was not found: {path}")
        for exported_power in exported_powers:
            if abs(float(exported_power) - material_power) > TRANSFORM_EPSILON:
                raise RuntimeError(
                    f"Unexpected material power in {path}: {exported_power}"
                )

    if export_animation:
        if re.search(r"\bAnimationSet\b", text) is None:
            raise RuntimeError(f"AnimationSet was not found: {path}")
        if re.search(r"\bAnimationKey\b", text) is None:
            raise RuntimeError(f"AnimationKey was not found: {path}")


def set_action(armature, action):
    armature.animation_data.action = action
    frame_start = int(action.frame_range[0])
    frame_end = int(action.frame_range[1])
    bpy.context.scene.frame_start = frame_start
    bpy.context.scene.frame_end = frame_end
    bpy.context.scene.frame_set(frame_start)
    bpy.context.view_layer.update()
    return frame_start, frame_end


def export_x(
    path,
    armature,
    mesh_objects,
    export_animation,
    frame_start,
    frame_end,
    material_power,
):
    select_export_objects(armature, mesh_objects)

    arguments = {
        "filepath": path,
        "check_existing": False,
        "use_selection": True,
        "axis_forward": DIRECTX_X_AXIS_FORWARD,
        "axis_up": DIRECTX_X_AXIS_UP,
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
    validate_x_file(path, export_animation, material_power)


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
    validate_export_objects(armature, mesh_objects)
    material_power = config.get("material_power")
    if material_power is not None:
        set_material_power(mesh_objects, material_power)

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
    export_x(
        base_path,
        armature,
        mesh_objects,
        False,
        idle_start,
        idle_end,
        material_power,
    )

    default_path = os.path.join(output_directory, "enemy.default.x")
    export_x(
        default_path,
        armature,
        mesh_objects,
        True,
        idle_start,
        idle_start,
        material_power,
    )

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
            material_power,
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
