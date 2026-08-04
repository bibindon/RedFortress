"""Create static platform Blender and DirectX X assets from the moving platform."""

from pathlib import Path

import bpy
from mathutils import Vector


SOURCE_BLEND = (
    Path(__file__).resolve().parents[1]
    / "RedFortress2"
    / "MultiPassRendering"
    / "res"
    / "model"
    / "collision_moving_platform"
    / "collision_moving_platform.blend"
)
OUTPUT_DIRECTORY = SOURCE_BLEND.parent.parent / "static_platform"
SOURCE_METAL_TEXTURE = SOURCE_BLEND.parent / "forged_metal.png"
SOURCE_WOOD_TEXTURE = SOURCE_BLEND.parent / "wood.png"

BASE_WIDTH = 3.0
BASE_DEPTH = 3.0
BASE_HEIGHT = 0.406

VARIANTS = (
    ("static_platform_1x1", 1.0, 1.0),
    ("static_platform_1x2", 1.0, 2.0),
    ("static_platform_2x1", 2.0, 1.0),
    ("static_platform_2x2", 2.0, 2.0),
)


def normalize_x_file(path):
    with path.open("rb") as source_file:
        data = source_file.read()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    with path.open("wb") as destination_file:
        destination_file.write(data)


def configure_relative_textures():
    for image in bpy.data.images:
        if image.name == "forged_metal.png":
            image.filepath = "//forged_metal.png"
            image.reload()
        if image.name == "wood.png":
            image.filepath = "//wood.png"
            image.reload()


def get_platform_object():
    platform = bpy.data.objects.get("Moving_Platform_Visual")
    if platform is None:
        raise RuntimeError("Moving_Platform_Visual was not found in the source blend file.")
    if platform.type != "MESH":
        raise RuntimeError("Moving_Platform_Visual is not a mesh object.")
    return platform


def configure_preview_camera(width_multiplier, depth_multiplier):
    camera = bpy.data.objects.get("Preview camera")
    if camera is None:
        return
    distance_multiplier = max(width_multiplier, depth_multiplier)
    camera.location = (4.4 * distance_multiplier, -4.7 * distance_multiplier,
                       3.5 * distance_multiplier)
    camera.data.lens = 52.0
    target = Vector((0.0, 0.0, 0.0))
    direction = target - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    bpy.context.scene.camera = camera


def configure_variant(name, width_multiplier, depth_multiplier):
    platform = get_platform_object()
    platform.name = name
    platform.data.name = f"{name}_Mesh"
    platform.scale.x = width_multiplier
    platform.scale.y = depth_multiplier
    platform.scale.z = 1.0
    bpy.context.view_layer.objects.active = platform
    platform.select_set(True)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    expected_dimensions = (
        BASE_WIDTH * width_multiplier,
        BASE_DEPTH * depth_multiplier,
        BASE_HEIGHT,
    )
    actual_dimensions = tuple(platform.dimensions)
    for index in range(3):
        if abs(actual_dimensions[index] - expected_dimensions[index]) > 0.001:
            raise RuntimeError(
                f"Unexpected dimensions for {name}: "
                f"{actual_dimensions}; expected {expected_dimensions}"
            )

    platform["asset_role"] = "static_platform"
    platform["footprint_width_m"] = expected_dimensions[0]
    platform["footprint_depth_m"] = expected_dimensions[1]
    platform["height_m"] = expected_dimensions[2]
    platform["width_multiplier"] = width_multiplier
    platform["depth_multiplier"] = depth_multiplier
    configure_preview_camera(width_multiplier, depth_multiplier)


def export_platform(platform, path):
    bpy.ops.object.select_all(action="DESELECT")
    platform.select_set(True)
    bpy.context.view_layer.objects.active = platform
    result = bpy.ops.export_scene.directx_x(
        filepath=str(path),
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"DirectX X export failed: {path}")
    normalize_x_file(path)
    with path.open("rb") as exported_file:
        exported_data = exported_file.read()
    if exported_data.startswith(b"\xef\xbb\xbf"):
        raise RuntimeError(f"DirectX X file contains a BOM: {path}")
    if not exported_data.startswith(b"xof "):
        raise RuntimeError(f"Invalid DirectX X header: {path}")


def save_preview(path):
    scene = bpy.context.scene
    scene.render.filepath = str(path)
    result = bpy.ops.render.render(write_still=True)
    if "FINISHED" not in result:
        raise RuntimeError(f"Preview render failed: {path}")


def main():
    OUTPUT_DIRECTORY.mkdir(parents=True, exist_ok=True)
    metal_texture = OUTPUT_DIRECTORY / SOURCE_METAL_TEXTURE.name
    wood_texture = OUTPUT_DIRECTORY / SOURCE_WOOD_TEXTURE.name
    if not metal_texture.exists():
        metal_texture.write_bytes(SOURCE_METAL_TEXTURE.read_bytes())
    if not wood_texture.exists():
        wood_texture.write_bytes(SOURCE_WOOD_TEXTURE.read_bytes())

    for name, width_multiplier, depth_multiplier in VARIANTS:
        bpy.ops.wm.open_mainfile(filepath=str(SOURCE_BLEND))
        bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
        configure_relative_textures()
        configure_variant(name, width_multiplier, depth_multiplier)

        blend_path = OUTPUT_DIRECTORY / f"{name}.blend"
        x_path = OUTPUT_DIRECTORY / f"{name}.x"
        preview_path = OUTPUT_DIRECTORY / f"{name}_preview.png"
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        configure_relative_textures()
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        export_platform(bpy.data.objects[name], x_path)
        save_preview(preview_path)
        print(f"Created {name}: {BASE_WIDTH * width_multiplier} x "
              f"{BASE_DEPTH * depth_multiplier} x {BASE_HEIGHT} m")


main()
