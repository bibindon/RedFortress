"""
Import a GLB/glTF source model into Blender, extract embedded textures to PNG
files next to the output, repoint materials to those PNGs, apply transforms,
and save as source.blend.

Usage:
    blender --background --python ImportAndSaveBlend.py -- <source.glb> <output_dir> <texture_basename>

After this, run PrepareEnemyModels.py against the produced source.blend.
"""
import os
import sys

import bpy


def parse_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected arguments after --")

    arguments = sys.argv[sys.argv.index("--") + 1:]
    if len(arguments) != 3:
        raise RuntimeError(
            "Usage: blender --background --python ImportAndSaveBlend.py "
            "-- <source.glb> <output_dir> <texture_basename>"
        )

    source_path = os.path.abspath(arguments[0])
    output_directory = os.path.abspath(arguments[1])
    texture_basename = arguments[2]
    return source_path, output_directory, texture_basename


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for block in list(bpy.data.meshes):
        bpy.data.meshes.remove(block)
    for block in list(bpy.data.materials):
        bpy.data.materials.remove(block)
    for block in list(bpy.data.images):
        bpy.data.images.remove(block)
    for block in list(bpy.data.armatures):
        bpy.data.armatures.remove(block)
    for block in list(bpy.data.actions):
        bpy.data.actions.remove(block)


def import_source(source_path):
    extension = os.path.splitext(source_path)[1].lower()
    if extension == ".glb":
        result = bpy.ops.import_scene.gltf(filepath=source_path)
    elif extension == ".gltf":
        result = bpy.ops.import_scene.gltf(filepath=source_path)
    else:
        raise RuntimeError(f"Unsupported source format: {source_path}")

    if "FINISHED" not in result:
        raise RuntimeError(f"glTF import failed: {source_path}")


def find_armature():
    for obj in bpy.context.scene.objects:
        if obj.type == "ARMATURE":
            return obj
    return None


def find_mesh_objects(armature):
    meshes = []
    for scene_object in bpy.context.scene.objects:
        if scene_object.type != "MESH":
            continue
        is_bound = False
        if scene_object.parent == armature:
            is_bound = True
        for modifier in scene_object.modifiers:
            if modifier.type == "ARMATURE" and modifier.object == armature:
                is_bound = True
                break
        if is_bound:
            meshes.append(scene_object)
    return meshes


def extract_textures(mesh_objects, output_directory, texture_basename):
    texture_path = os.path.join(output_directory, texture_basename)
    extracted_image = None

    for mesh_object in mesh_objects:
        for material_slot in mesh_object.material_slots:
            material = material_slot.material
            if material is None:
                continue
            tree_nodes = material.node_tree.nodes
            for node in tree_nodes:
                if node.type != "TEX_IMAGE":
                    continue
                image = node.image
                if image is None:
                    continue
                if extracted_image is None:
                    image.filepath_raw = texture_path
                    image.file_format = "PNG"
                    image.save()
                    extracted_image = image
                else:
                    node.image = extracted_image

    return texture_path if extracted_image is not None else None


def apply_transforms(armature, mesh_objects):
    export_objects = [armature]
    export_objects.extend(mesh_objects)
    bpy.ops.object.select_all(action="DESELECT")
    for export_object in export_objects:
        export_object.hide_viewport = False
        export_object.hide_set(False)
        export_object.select_set(True)
    bpy.context.view_layer.objects.active = armature
    bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)


def main():
    source_path, output_directory, texture_basename = parse_arguments()
    os.makedirs(output_directory, exist_ok=True)

    clear_scene()
    import_source(source_path)

    armature = find_armature()
    if armature is None:
        raise RuntimeError("No armature was found in the imported source.")
    armature_name = armature.name

    mesh_objects = find_mesh_objects(armature)
    if len(mesh_objects) == 0:
        raise RuntimeError("No mesh is bound to the armature.")

    texture_path = extract_textures(mesh_objects, output_directory, texture_basename)
    apply_transforms(armature, mesh_objects)

    blend_path = os.path.join(output_directory, "source.blend")
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)

    print(f"IMPORTED source={source_path}")
    print(f"  armature={armature_name}")
    print(f"  meshes={len(mesh_objects)} names={[m.name for m in mesh_objects]}")
    print(f"  texture={'saved: ' + texture_path if texture_path else 'none (no embedded texture)'}")
    print(f"  blend={blend_path}")


main()
