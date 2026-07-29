import json
import os
import sys

import bpy


def parse_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected arguments after --")

    arguments = sys.argv[sys.argv.index("--") + 1 :]
    if len(arguments) != 2:
        raise RuntimeError(
            "Usage: blender --background --python ImportKanataPrototype.py "
            "-- <source.vrm> <output-directory>"
        )

    source_path = os.path.abspath(arguments[0])
    output_directory = os.path.abspath(arguments[1])
    if not os.path.isfile(source_path):
        raise RuntimeError(f"VRM source was not found: {source_path}")

    return source_path, output_directory


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)

    for data_collection in (
        bpy.data.actions,
        bpy.data.armatures,
        bpy.data.materials,
        bpy.data.meshes,
        bpy.data.objects,
    ):
        for data_block in list(data_collection):
            if data_block.users == 0:
                data_collection.remove(data_block)


def import_vrm(source_path):
    result = bpy.ops.import_scene.vrm(
        filepath=source_path,
        use_addon_preferences=False,
        extract_textures_into_folder=True,
        make_new_texture_folder=False,
        set_shading_type_to_material_on_import=False,
        set_view_transform_to_standard_on_import=True,
        set_armature_display_to_wire=True,
        set_armature_display_to_show_in_front=True,
        set_armature_bone_shape_to_default=True,
        enable_mtoon_outline_preview=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"VRM import failed: {result}")


def build_report(source_path):
    armatures = [obj for obj in bpy.context.scene.objects if obj.type == "ARMATURE"]
    meshes = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(armatures) == 0:
        raise RuntimeError("No armature was imported from the VRM")
    if len(meshes) == 0:
        raise RuntimeError("No mesh was imported from the VRM")

    report = {
        "source": source_path,
        "armatures": [],
        "meshes": [],
        "materials": sorted(material.name for material in bpy.data.materials),
        "images": sorted(image.name for image in bpy.data.images),
        "actions": sorted(action.name for action in bpy.data.actions),
    }

    for armature in armatures:
        report["armatures"].append(
            {
                "name": armature.name,
                "location": list(armature.location),
                "rotation_euler": list(armature.rotation_euler),
                "scale": list(armature.scale),
                "bones": [bone.name for bone in armature.data.bones],
            }
        )

    for mesh in meshes:
        report["meshes"].append(
            {
                "name": mesh.name,
                "location": list(mesh.location),
                "rotation_euler": list(mesh.rotation_euler),
                "scale": list(mesh.scale),
                "dimensions": list(mesh.dimensions),
                "vertices": len(mesh.data.vertices),
                "polygons": len(mesh.data.polygons),
                "materials": [
                    slot.material.name
                    for slot in mesh.material_slots
                    if slot.material is not None
                ],
                "armature_modifiers": [
                    modifier.object.name
                    for modifier in mesh.modifiers
                    if modifier.type == "ARMATURE" and modifier.object is not None
                ],
            }
        )

    return report


def main():
    source_path, output_directory = parse_arguments()
    os.makedirs(output_directory, exist_ok=True)

    clear_scene()
    import_vrm(source_path)
    bpy.context.view_layer.update()

    report = build_report(source_path)
    report_path = os.path.join(output_directory, "kanata_import_report.json")
    with open(report_path, "w", encoding="utf-8", newline="\n") as report_file:
        json.dump(report, report_file, ensure_ascii=False, indent=2)
        report_file.write("\n")

    blend_path = os.path.join(output_directory, "kanata_imported.blend")
    save_result = bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    if "FINISHED" not in save_result:
        raise RuntimeError(f"Failed to save Blender file: {blend_path}")

    print(
        "KANATA_VRM_IMPORTED "
        f"armatures={len(report['armatures'])} "
        f"meshes={len(report['meshes'])} "
        f"materials={len(report['materials'])} "
        f"blend={blend_path}"
    )


main()
