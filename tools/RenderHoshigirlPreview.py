"""Renders a quick front preview of Hoshigirl.blend for visual verification.

Usage:
    blender --background <blend> --python RenderHoshigirlPreview.py -- <output-png>
"""

import math
import os
import sys

import bpy


def main():
    arguments = sys.argv[sys.argv.index("--") + 1:]
    output_path = os.path.abspath(arguments[0])

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 768
    scene.render.resolution_y = 1024
    scene.render.film_transparent = False

    world = bpy.data.worlds.new("PreviewWorld")
    world.use_nodes = True
    background = world.node_tree.nodes["Background"]
    background.inputs[0].default_value = (0.85, 0.85, 0.85, 1.0)
    background.inputs[1].default_value = 1.0
    scene.world = world

    camera_data = bpy.data.cameras.new("PreviewCamera")
    camera_obj = bpy.data.objects.new("PreviewCamera", camera_data)
    scene.collection.objects.link(camera_obj)
    # The character faces -Y, so place the camera on -Y looking at +Y.
    camera_obj.location = (0.6, -3.2, 1.0)
    camera_obj.rotation_euler = (math.radians(88.0), 0.0, math.radians(10.5))
    scene.camera = camera_obj

    key_light_data = bpy.data.lights.new("KeyLight", type="SUN")
    key_light_data.energy = 4.0
    key_light = bpy.data.objects.new("KeyLight", key_light_data)
    scene.collection.objects.link(key_light)
    key_light.rotation_euler = (math.radians(55.0), 0.0, math.radians(-30.0))

    fill_light_data = bpy.data.lights.new("FillLight", type="SUN")
    fill_light_data.energy = 1.5
    fill_light = bpy.data.objects.new("FillLight", fill_light_data)
    scene.collection.objects.link(fill_light)
    fill_light.rotation_euler = (math.radians(70.0), 0.0, math.radians(150.0))

    scene.render.filepath = output_path
    bpy.ops.render.render(write_still=True)
    print("HOSHIGIRL_PREVIEW_RENDERED " + output_path)


main()
