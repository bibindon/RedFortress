"""Add 0.1 m perimeter guard rails to all static platform assets."""

from pathlib import Path
import os
import sys

import bpy
from mathutils import Vector


ROOT_DIRECTORY = Path(__file__).resolve().parents[1]
OUTPUT_DIRECTORY = (
    ROOT_DIRECTORY
    / "RedFortress2"
    / "MultiPassRendering"
    / "res"
    / "model"
    / "static_platform"
)
SOURCE_DIRECTORY = OUTPUT_DIRECTORY

VARIANTS = (
    ("static_platform_1x2", "static_platform_1x2.blend", 1.0),
    ("static_platform_2x1", "static_platform_2x1.blend", 1.0),
    ("static_platform_2x2", "static_platform_2x2.blend", 1.0),
    ("static_platform_4x4", "static_platform_2x2.blend", 2.0),
    ("static_platform_1x1", "static_platform_1x1.blend", 1.0),
)

GUARD_RAIL_HEIGHT = 0.1
GUARD_RAIL_THICKNESS = 0.3
GUARD_RAIL_BEVEL = 0.015
FLOOR_HEIGHT = 0.8
COLLISION_TOP_HEIGHT = 0.203


def normalize_x_file(path):
    with path.open("rb") as source_file:
        data = source_file.read()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    with path.open("wb") as destination_file:
        destination_file.write(data)


def get_platform_object():
    existing_guard_rails = [
        obj for obj in bpy.context.scene.objects
        if obj.type == "MESH" and "_GuardRail_" in obj.name
    ]
    for guard_rail in existing_guard_rails:
        bpy.data.objects.remove(guard_rail, do_unlink=True)
    mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if len(mesh_objects) != 1:
        raise RuntimeError(
            f"Expected one platform mesh, found {len(mesh_objects)}."
        )
    return mesh_objects[0]


def configure_relative_textures():
    for image in bpy.data.images:
        texture_path = SOURCE_DIRECTORY / image.name
        if texture_path.exists():
            image.filepath = f"//{image.name}"
            image.reload()


def get_world_bounds(obj):
    corners = [obj.matrix_world @ Vector(corner) for corner in obj.bound_box]
    minimum = Vector((
        min(corner.x for corner in corners),
        min(corner.y for corner in corners),
        min(corner.z for corner in corners),
    ))
    maximum = Vector((
        max(corner.x for corner in corners),
        max(corner.y for corner in corners),
        max(corner.z for corner in corners),
    ))
    return minimum, maximum


def extend_platform_downward(obj):
    minimum, maximum = get_world_bounds(obj)
    current_height = maximum.z - minimum.z
    if current_height <= 0.0:
        raise RuntimeError(f"Invalid platform height: {current_height}")
    if abs(current_height - FLOOR_HEIGHT) <= 0.000001:
        return

    top_height = maximum.z
    height_scale = FLOOR_HEIGHT / current_height
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    obj.scale.z *= height_scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    _, scaled_maximum = get_world_bounds(obj)
    obj.location.z += top_height - scaled_maximum.z
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)

    updated_minimum, updated_maximum = get_world_bounds(obj)
    updated_height = updated_maximum.z - updated_minimum.z
    if abs(updated_maximum.z - top_height) > 0.000001:
        raise RuntimeError("Platform top moved while extending the floor downward.")
    if abs(updated_height - FLOOR_HEIGHT) > 0.000001:
        raise RuntimeError(
            f"Unexpected platform height: {updated_height}; expected {FLOOR_HEIGHT}"
        )


def create_box(name, dimensions, location, material, bevel_width=0.0):
    bpy.ops.mesh.primitive_cube_add(location=location)
    box = bpy.context.active_object
    box.name = name
    box.dimensions = dimensions
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
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
    material.diffuse_color = (0.025, 0.035, 0.045, 1.0)
    material.metallic = 0.85
    material.roughness = 0.24
    material.use_nodes = True
    shader = material.node_tree.nodes.get("Principled BSDF")
    if shader is None:
        raise RuntimeError("Principled BSDF was not found for the guard rail.")
    shader.inputs["Base Color"].default_value = (0.025, 0.035, 0.045, 1.0)
    shader.inputs["Metallic"].default_value = 0.85
    shader.inputs["Roughness"].default_value = 0.24
    return material


def create_guard_rails(name, minimum, maximum, material, bevel_width):
    width = maximum.x - minimum.x
    depth = maximum.y - minimum.y
    center_x = (minimum.x + maximum.x) * 0.5
    center_y = (minimum.y + maximum.y) * 0.5
    rail_z = maximum.z + GUARD_RAIL_HEIGHT * 0.5
    half_thickness = GUARD_RAIL_THICKNESS * 0.5
    rails = [
        create_box(
            f"{name}_GuardRail_North",
            (width, GUARD_RAIL_THICKNESS, GUARD_RAIL_HEIGHT),
            (center_x, maximum.y - half_thickness, rail_z),
            material,
            bevel_width,
        ),
        create_box(
            f"{name}_GuardRail_South",
            (width, GUARD_RAIL_THICKNESS, GUARD_RAIL_HEIGHT),
            (center_x, minimum.y + half_thickness, rail_z),
            material,
            bevel_width,
        ),
        create_box(
            f"{name}_GuardRail_East",
            (GUARD_RAIL_THICKNESS,
             depth - GUARD_RAIL_THICKNESS * 2.0,
             GUARD_RAIL_HEIGHT),
            (maximum.x - half_thickness, center_y, rail_z),
            material,
            bevel_width,
        ),
        create_box(
            f"{name}_GuardRail_West",
            (GUARD_RAIL_THICKNESS,
             depth - GUARD_RAIL_THICKNESS * 2.0,
             GUARD_RAIL_HEIGHT),
            (minimum.x + half_thickness, center_y, rail_z),
            material,
            bevel_width,
        ),
    ]
    return rails


def export_objects(objects, path):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    temporary_path = path.with_name(
        f".{path.stem}.{os.getpid()}.official-export.x"
    )
    result = bpy.ops.export_scene.directx_x(
        filepath=str(temporary_path),
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"DirectX X export failed: {path}")
    normalize_x_file(temporary_path)
    with temporary_path.open("rb") as exported_file:
        exported_data = exported_file.read()
    if exported_data.startswith(b"\xef\xbb\xbf"):
        raise RuntimeError(f"DirectX X file contains a BOM: {path}")
    if not exported_data.startswith(b"xof "):
        raise RuntimeError(f"Invalid DirectX X header: {temporary_path}")
    temporary_path.replace(path)


def save_preview(path, scale_multiplier):
    camera = bpy.data.objects.get("Preview camera")
    if camera is not None and scale_multiplier != 1.0:
        camera.location *= scale_multiplier
        direction = Vector((0.0, 0.0, 0.0)) - camera.location
        camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()
    temporary_path = path.with_name(
        f".{path.stem}.{os.getpid()}.render.png"
    )
    bpy.context.scene.render.filepath = str(temporary_path)
    result = bpy.ops.render.render(write_still=True)
    if "FINISHED" not in result:
        raise RuntimeError(f"Preview render failed: {temporary_path}")
    temporary_path.replace(path)


def create_collision_mesh(name, minimum, maximum):
    collision_material = bpy.data.materials.get("CollisionMat")
    if collision_material is None:
        collision_material = bpy.data.materials.new(name="CollisionMat")
    collision_minimum = Vector((
        minimum.x,
        minimum.y,
        COLLISION_TOP_HEIGHT - FLOOR_HEIGHT,
    ))
    collision_maximum = Vector((
        maximum.x,
        maximum.y,
        COLLISION_TOP_HEIGHT,
    ))
    dimensions = collision_maximum - collision_minimum
    center = (collision_minimum + collision_maximum) * 0.5
    floor = create_box(
        f"{name}_Collision",
        tuple(dimensions),
        tuple(center),
        collision_material,
    )
    rails = create_guard_rails(
        f"{name}_Collision",
        collision_minimum,
        collision_maximum,
        collision_material,
        0.0,
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


def build_variant(name, source_blend_name, scale_multiplier):
    source_path = SOURCE_DIRECTORY / source_blend_name
    bpy.ops.wm.open_mainfile(filepath=str(source_path))
    configure_relative_textures()
    platform = get_platform_object()
    platform.name = name
    platform.data.name = f"{name}_Mesh"
    if scale_multiplier != 1.0:
        platform.scale.x *= scale_multiplier
        platform.scale.y *= scale_multiplier
        bpy.context.view_layer.objects.active = platform
        platform.select_set(True)
        bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)

    extend_platform_downward(platform)
    minimum, maximum = get_world_bounds(platform)
    guard_rails = create_guard_rails(
        name,
        minimum,
        maximum,
        get_guard_rail_material(),
        GUARD_RAIL_BEVEL,
    )
    blend_path = OUTPUT_DIRECTORY / f"{name}.blend"
    x_path = OUTPUT_DIRECTORY / f"{name}.x"
    collision_path = OUTPUT_DIRECTORY / f"{name}_collision.x"
    preview_path = OUTPUT_DIRECTORY / f"{name}_preview.png"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    configure_relative_textures()
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    export_objects([platform] + guard_rails, x_path)
    save_preview(preview_path, scale_multiplier)
    collision = create_collision_mesh(name, minimum, maximum)
    export_objects([collision], collision_path)
    print(
        f"Updated {name}: {maximum.x - minimum.x:.3f} x "
        f"{maximum.y - minimum.y:.3f} x {maximum.z - minimum.z:.3f} m, "
        f"guard rail {GUARD_RAIL_HEIGHT:.3f} m"
    )


def main():
    global SOURCE_DIRECTORY
    if "--" in sys.argv:
        script_arguments = sys.argv[sys.argv.index("--") + 1:]
        if len(script_arguments) == 2 and script_arguments[0] == "--source-directory":
            SOURCE_DIRECTORY = Path(script_arguments[1]).resolve()
        elif len(script_arguments) != 0:
            raise RuntimeError(
                "Usage: -- --source-directory <static-platform-directory>"
            )
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    for name, source_blend_name, scale_multiplier in VARIANTS:
        build_variant(name, source_blend_name, scale_multiplier)


main()
