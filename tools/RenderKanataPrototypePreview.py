import os
import sys

import bpy
from mathutils import Vector


def parse_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected arguments after --")

    arguments = sys.argv[sys.argv.index("--") + 1 :]
    if len(arguments) != 2:
        raise RuntimeError(
            "Usage: blender --background --python "
            "RenderKanataPrototypePreview.py -- <prototype.blend> <preview.png>"
        )

    blend_path = os.path.abspath(arguments[0])
    preview_path = os.path.abspath(arguments[1])
    if not os.path.isfile(blend_path):
        raise RuntimeError(f"Prototype Blender file was not found: {blend_path}")

    return blend_path, preview_path


def point_camera(camera, target):
    direction = Vector(target) - camera.location
    camera.rotation_euler = direction.to_track_quat("-Z", "Y").to_euler()


def add_area_light(name, location, energy, color, size):
    light_data = bpy.data.lights.new(name=name, type="AREA")
    light_data.energy = energy
    light_data.color = color
    light_data.shape = "DISK"
    light_data.size = size
    light_object = bpy.data.objects.new(name, light_data)
    bpy.context.scene.collection.objects.link(light_object)
    light_object.location = location
    light_object.rotation_euler = (
        Vector((0.0, 0.0, 0.9)) - light_object.location
    ).to_track_quat("-Z", "Y").to_euler()


def add_floor():
    bpy.ops.mesh.primitive_plane_add(size=8.0, location=(0.0, 0.0, 0.0))
    floor = bpy.context.object
    floor.name = "PreviewFloor"
    material = bpy.data.materials.new("PreviewFloorMaterial")
    material.diffuse_color = (0.025, 0.035, 0.055, 1.0)
    floor.data.materials.append(material)


def main():
    blend_path, preview_path = parse_arguments()
    bpy.ops.wm.open_mainfile(filepath=blend_path)

    scene = bpy.context.scene
    scene.frame_set(30)
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 900
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.film_transparent = False
    scene.render.filepath = preview_path
    scene.render.resolution_percentage = 100

    if scene.world is None:
        scene.world = bpy.data.worlds.new("PreviewWorld")
    scene.world.color = (0.008, 0.012, 0.025)
    add_floor()

    camera_data = bpy.data.cameras.new("PreviewCamera")
    camera = bpy.data.objects.new("PreviewCamera", camera_data)
    scene.collection.objects.link(camera)
    scene.camera = camera
    camera.location = (0.0, -3.1, 1.15)
    camera.data.lens = 58.0
    point_camera(camera, (0.0, 0.0, 0.82))

    add_area_light(
        "PreviewKey",
        (-2.2, -2.4, 3.2),
        950.0,
        (1.0, 0.78, 0.64),
        4.0,
    )
    add_area_light(
        "PreviewFill",
        (2.4, -1.5, 2.0),
        700.0,
        (0.45, 0.65, 1.0),
        3.0,
    )
    add_area_light(
        "PreviewRim",
        (0.0, 2.0, 2.8),
        1100.0,
        (0.55, 0.72, 1.0),
        2.5,
    )

    bpy.context.view_layer.update()
    render_result = bpy.ops.render.render(write_still=True)
    if "FINISHED" not in render_result:
        raise RuntimeError(f"Preview render failed: {render_result}")
    if not os.path.isfile(preview_path):
        raise RuntimeError(f"Preview image was not created: {preview_path}")

    print(f"KANATA_PREVIEW_RENDERED path={preview_path}")


main()
