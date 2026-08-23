"""Create the solid-yellow star pickup with the official DirectX X exporter."""

import math
import os

import bpy


OUTER_RADIUS = 0.25
INNER_RADIUS = 0.1075
DEPTH = 0.075
POINT_COUNT = 5


def normalize_x_file(path):
    with open(path, "rb") as source_file:
        data = source_file.read()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    with open(path, "wb") as destination_file:
        destination_file.write(data)


def create_star_mesh():
    ring_count = POINT_COUNT * 2
    half_depth = DEPTH * 0.5
    vertices = []
    for y in (-half_depth, half_depth):
        for index in range(ring_count):
            radius = OUTER_RADIUS
            if index % 2 != 0:
                radius = INNER_RADIUS
            angle = math.pi * 0.5 - math.pi * float(index) / float(POINT_COUNT)
            vertices.append((math.cos(angle) * radius, y, math.sin(angle) * radius))

    front_center = len(vertices)
    vertices.append((0.0, -half_depth, 0.0))
    back_center = len(vertices)
    vertices.append((0.0, half_depth, 0.0))

    faces = []
    for index in range(ring_count):
        next_index = (index + 1) % ring_count
        faces.append((front_center, next_index, index))
        faces.append((back_center, ring_count + index, ring_count + next_index))
        faces.append((index,
                      next_index,
                      ring_count + next_index,
                      ring_count + index))

    mesh = bpy.data.meshes.new("StarPickupMesh")
    mesh.from_pydata(vertices, (), faces)
    mesh.update()

    uv_layer = mesh.uv_layers.new(name="StarUV")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex = mesh.vertices[mesh.loops[loop_index].vertex_index]
            u = vertex.co.x / (OUTER_RADIUS * 2.0) + 0.5
            v = vertex.co.z / (OUTER_RADIUS * 2.0) + 0.5
            uv_layer.data[loop_index].uv = (u, v)

    star = bpy.data.objects.new("StarPickup", mesh)
    bpy.context.collection.objects.link(star)
    return star


def create_yellow_material(texture_path):
    material = bpy.data.materials.new("StarSolidYellow")
    material.diffuse_color = (1.0, 0.82, 0.0, 1.0)
    material.use_nodes = True
    principled = material.node_tree.nodes.get("Principled BSDF")
    principled.inputs["Base Color"].default_value = (1.0, 0.82, 0.0, 1.0)
    principled.inputs["Roughness"].default_value = 0.4

    image = bpy.data.images.load(texture_path, check_existing=True)
    texture = material.node_tree.nodes.new("ShaderNodeTexImage")
    texture.name = "StarSolidYellowTexture"
    texture.image = image
    material.node_tree.links.new(
        texture.outputs["Color"],
        principled.inputs["Base Color"],
    )
    return material


def main():
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    output_directory = os.path.join(
        repo_root,
        "RedFortress2",
        "MultiPassRendering",
        "res",
        "model",
        "itemIconStar",
    )
    os.makedirs(output_directory, exist_ok=True)
    texture_path = os.path.join(output_directory, "itemIconStar_yellow.png")
    if not os.path.isfile(texture_path):
        raise RuntimeError("The solid-yellow texture is missing: " + texture_path)

    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")

    star = create_star_mesh()
    star.data.materials.append(create_yellow_material(texture_path))
    bpy.ops.object.select_all(action="DESELECT")
    star.select_set(True)
    bpy.context.view_layer.objects.active = star

    blend_path = os.path.join(output_directory, "itemIconStar.blend")
    x_path = os.path.join(output_directory, "itemIconStar.x")
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)
    result = bpy.ops.export_scene.directx_x(
        filepath=x_path,
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + x_path)

    normalize_x_file(x_path)
    with open(x_path, "rb") as exported_file:
        exported_data = exported_file.read()
    if not exported_data.startswith(b"xof "):
        raise RuntimeError("The exported DirectX X header is invalid.")
    if b"itemiconstar_yellow.png" not in exported_data.lower():
        raise RuntimeError("The solid-yellow texture reference is missing.")

    print("Saved: " + blend_path)
    print("Exported: " + x_path)


if __name__ == "__main__":
    main()
