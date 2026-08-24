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
    ("static_platform_4x4", 4.0, 4.0),
)

GUARD_RAIL_HEIGHT = 0.1
GUARD_RAIL_THICKNESS = 0.1


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


def create_box(name, dimensions, location, material=None, bevel_width=0.0):
    bpy.ops.mesh.primitive_cube_add(location=location)
    box = bpy.context.active_object
    box.name = name
    box.dimensions = dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    if material is not None:
        box.data.materials.append(material)
    if bevel_width > 0.0:
        bevel = box.modifiers.new(name="GuardRailBevel", type="BEVEL")
        bevel.width = bevel_width
        bevel.segments = 2
        bpy.context.view_layer.objects.active = box
        bpy.ops.object.modifier_apply(modifier=bevel.name)
    return box


def get_guard_rail_material():
    material = bpy.data.materials.get("GuardRail_Metal")
    if material is not None:
        return material
    material = bpy.data.materials.new(name="GuardRail_Metal")
    material.diffuse_color = (0.035, 0.045, 0.055, 1.0)
    material.metallic = 0.85
    material.roughness = 0.24
    return material


def create_guard_rails(name, width, depth, top_z, collision=False):
    material = None
    bevel_width = 0.0
    if not collision:
        material = get_guard_rail_material()
        bevel_width = 0.015

    half_height = GUARD_RAIL_HEIGHT * 0.5
    half_thickness = GUARD_RAIL_THICKNESS * 0.5
    rail_z = top_z + half_height
    rails = [
        create_box(
            f"{name}_GuardRail_North",
            (width, GUARD_RAIL_THICKNESS, GUARD_RAIL_HEIGHT),
            (0.0, depth * 0.5 - half_thickness, rail_z),
            material,
            bevel_width,
        ),
        create_box(
            f"{name}_GuardRail_South",
            (width, GUARD_RAIL_THICKNESS, GUARD_RAIL_HEIGHT),
            (0.0, -depth * 0.5 + half_thickness, rail_z),
            material,
            bevel_width,
        ),
        create_box(
            f"{name}_GuardRail_East",
            (GUARD_RAIL_THICKNESS, depth - GUARD_RAIL_THICKNESS * 2.0,
             GUARD_RAIL_HEIGHT),
            (width * 0.5 - half_thickness, 0.0, rail_z),
            material,
            bevel_width,
        ),
        create_box(
            f"{name}_GuardRail_West",
            (GUARD_RAIL_THICKNESS, depth - GUARD_RAIL_THICKNESS * 2.0,
             GUARD_RAIL_HEIGHT),
            (-width * 0.5 + half_thickness, 0.0, rail_z),
            material,
            bevel_width,
        ),
    ]
    return rails


def create_collision_mesh(name, width, depth):
    collision_material = bpy.data.materials.get("CollisionMat")
    if collision_material is None:
        collision_material = bpy.data.materials.new(name="CollisionMat")
    floor = create_box(
        f"{name}_Collision",
        (width, depth, BASE_HEIGHT),
        (0.0, 0.0, 0.0),
        collision_material,
    )
    rails = create_guard_rails(
        f"{name}_Collision",
        width,
        depth,
        BASE_HEIGHT * 0.5,
        collision=True,
    )
    bpy.ops.object.select_all(action="DESELECT")
    floor.select_set(True)
    for rail in rails:
        rail.select_set(True)
    bpy.context.view_layer.objects.active = floor
    bpy.ops.object.join()
    floor.name = f"{name}_Collision"
    floor.data.name = f"{name}_CollisionMesh"
    return floor


def export_objects(objects, path):
    bpy.ops.object.select_all(action="DESELECT")
    for object_to_export in objects:
        object_to_export.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
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
        platform = bpy.data.objects[name]
        width = BASE_WIDTH * width_multiplier
        depth = BASE_DEPTH * depth_multiplier
        guard_rails = create_guard_rails(
            name,
            width,
            depth,
            BASE_HEIGHT * 0.5,
        )

        blend_path = OUTPUT_DIRECTORY / f"{name}.blend"
        x_path = OUTPUT_DIRECTORY / f"{name}.x"
        collision_path = OUTPUT_DIRECTORY / f"{name}_collision.x"
        preview_path = OUTPUT_DIRECTORY / f"{name}_preview.png"
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        configure_relative_textures()
        bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
        export_objects([platform] + guard_rails, x_path)
        save_preview(preview_path)
        collision = create_collision_mesh(name, width, depth)
        export_objects([collision], collision_path)
        print(f"Created {name}: {BASE_WIDTH * width_multiplier} x "
              f"{BASE_DEPTH * depth_multiplier} x {BASE_HEIGHT} m, "
              f"guard rail {GUARD_RAIL_HEIGHT} m")


main()
