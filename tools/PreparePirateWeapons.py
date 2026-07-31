from pathlib import Path

import bpy


REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
ASSET_DIRECTORY = REPOSITORY_ROOT / "RedFortress2" / "MultiPassRendering" / "res" / "model" / "piratekit"
TEXTURE_NAME = "Atlas_Pirate.png"
# The grip orientation fix (180-degree roll around the weapon's long axis) is
# baked into the .blend sources by tools/BakePirateWeaponGripFix.py and marked
# with the custom property below. Export refuses unbaked sources.
GRIP_FIX_FLAG = "grip_fix_baked"
WEAPONS = (
    ("Weapon_Pistol.blend", "pistol.x", "Weapon_Pistol"),
    ("Weapon_Cutlass.blend", "cutlass.x", "Weapon_Cutlass"),
)


def export_weapon(source_name, output_name, expected_object_name):
    source_path = ASSET_DIRECTORY / source_name
    output_path = ASSET_DIRECTORY / output_name
    if not source_path.exists():
        raise FileNotFoundError(source_path)

    bpy.ops.wm.open_mainfile(filepath=str(source_path))
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")

    mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(mesh_objects) != 1:
        raise RuntimeError(source_name + " must contain exactly one mesh object")

    mesh_object = mesh_objects[0]
    if mesh_object.name != expected_object_name:
        raise RuntimeError("Unexpected weapon object: " + mesh_object.name)

    if not mesh_object.get(GRIP_FIX_FLAG):
        raise RuntimeError(source_name + " is missing the baked grip fix. "
                           "Run tools/BakePirateWeaponGripFix.py first.")

    texture_found = False
    for image in bpy.data.images:
        if image.name == TEXTURE_NAME or image.filepath.lower().endswith(TEXTURE_NAME.lower()):
            if image.packed_file is not None:
                image.unpack(method="REMOVE")
            image.filepath_raw = str(ASSET_DIRECTORY / TEXTURE_NAME)
            image.reload()
            image.filepath_raw = "//" + TEXTURE_NAME
            texture_found = True
    if not texture_found:
        raise RuntimeError("Pirate Kit texture was not found in " + source_name)

    bpy.ops.object.select_all(action="DESELECT")
    mesh_object.select_set(True)
    bpy.context.view_layer.objects.active = mesh_object

    result = bpy.ops.export_scene.directx_x(
        filepath=str(output_path),
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        use_mesh_modifiers=True,
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        use_original_material_data=False,
        export_format="TEXT_X",
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(output_path))

    print("PIRATE_WEAPON_EXPORTED", output_path)


def main():
    if not (ASSET_DIRECTORY / TEXTURE_NAME).exists():
        raise FileNotFoundError(ASSET_DIRECTORY / TEXTURE_NAME)

    for source_name, output_name, expected_object_name in WEAPONS:
        export_weapon(source_name, output_name, expected_object_name)


if __name__ == "__main__":
    main()