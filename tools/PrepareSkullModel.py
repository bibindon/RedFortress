"""Create the static throwable skull model from the cleaned Skeleton asset."""

import os

import bmesh
import bpy
import numpy as np
from mathutils import Matrix, Vector


SOURCE_MESH_NAME = "Cylinder.001"
HEAD_GROUP_NAME = "Head"
MODEL_SCALE = 0.5
JAW_EPSILON = 0.025
COLLISION_RADIUS = 0.55
COLLISION_HEIGHT = 0.76
SKULL_X_POWER = 500.0
TEXTURE_SIZE = 512
TEXTURE_FILENAME = "skull_diffuse.png"
AO_MARGIN = 16


def normalize_x_file(path):
    with open(path, "rb") as source_file:
        data = source_file.read()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    with open(path, "wb") as destination_file:
        destination_file.write(data)


def export_selected(path):
    result = bpy.ops.export_scene.directx_x(
        filepath=path,
        check_existing=False,
        use_selection=True,
        axis_forward="Z",
        axis_up="Y",
        export_animation=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"DirectX X export failed: {path}")
    normalize_x_file(path)


def create_collision_model(output_directory):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=16,
        radius=COLLISION_RADIUS,
        depth=COLLISION_HEIGHT,
        end_fill_type="NGON",
        location=(0.0, 0.0, COLLISION_HEIGHT * 0.5),
    )
    collision_object = bpy.context.active_object
    collision_object.name = "SkullCollision"
    collision_object.data.name = "SkullCollision"
    bpy.ops.object.select_all(action="DESELECT")
    collision_object.select_set(True)
    bpy.context.view_layer.objects.active = collision_object
    collision_path = os.path.join(output_directory, "skull_collision.x")
    export_selected(collision_path)
    return collision_path


def keep_head_vertices(mesh_object):
    head_group = mesh_object.vertex_groups.get(HEAD_GROUP_NAME)
    if head_group is None:
        raise RuntimeError(f"Vertex group was not found: {HEAD_GROUP_NAME}")

    mesh = mesh_object.data
    bm = bmesh.new()
    bm.from_mesh(mesh)
    deform_layer = bm.verts.layers.deform.active
    if deform_layer is None:
        bm.free()
        raise RuntimeError("The source mesh has no deform weights.")

    remove_vertices = []
    for vertex in bm.verts:
        weights = vertex[deform_layer]
        if weights.get(head_group.index, 0.0) <= 0.001:
            remove_vertices.append(vertex)

    bmesh.ops.delete(bm, geom=remove_vertices, context="VERTS")
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()

    if len(mesh.vertices) == 0 or len(mesh.polygons) == 0:
        raise RuntimeError("The extracted skull mesh is empty.")


def bake_scale_and_jaw_origin(mesh_object):
    mesh = mesh_object.data
    mesh.transform(Matrix.Scale(MODEL_SCALE, 4))

    minimum_z = min(vertex.co.z for vertex in mesh.vertices)
    jaw_vertices = [
        vertex.co.copy()
        for vertex in mesh.vertices
        if vertex.co.z <= minimum_z + JAW_EPSILON
    ]
    if len(jaw_vertices) == 0:
        raise RuntimeError("Could not find the lower jaw vertices.")

    jaw_center = Vector((0.0, 0.0, 0.0))
    for coordinate in jaw_vertices:
        jaw_center += coordinate
    jaw_center /= len(jaw_vertices)
    jaw_center.z = minimum_z

    mesh.transform(Matrix.Translation(-jaw_center))
    mesh.update()
    mesh_object.matrix_world = Matrix.Identity(4)


def unwrap_skull_uv(mesh_object):
    bpy.ops.object.select_all(action="DESELECT")
    mesh_object.select_set(True)
    bpy.context.view_layer.objects.active = mesh_object
    bpy.ops.object.mode_set(mode="EDIT")
    bpy.ops.mesh.select_all(action="SELECT")
    bpy.ops.uv.smart_project(angle_limit=1.15192, island_margin=0.02)
    bpy.ops.object.mode_set(mode="OBJECT")


def read_image_pixels(image):
    width, height = image.size
    buffer = np.empty(width * height * 4, dtype=np.float32)
    image.pixels.foreach_get(buffer)
    return buffer.reshape(height, width, 4)


def low_frequency_noise(width, height, cells, seed):
    rng = np.random.default_rng(seed)
    grid = rng.random((cells + 1, cells + 1))
    xs = (np.arange(width) + 0.5) / width * cells
    ys = (np.arange(height) + 0.5) / height * cells
    x0 = np.floor(xs).astype(int)
    y0 = np.floor(ys).astype(int)
    xf = (xs - x0)[np.newaxis, :]
    yf = (ys - y0)[:, np.newaxis]
    g00 = grid[np.ix_(y0, x0)]
    g01 = grid[np.ix_(y0, x0 + 1)]
    g10 = grid[np.ix_(y0 + 1, x0)]
    g11 = grid[np.ix_(y0 + 1, x0 + 1)]
    top = g00 * (1.0 - xf) + g01 * xf
    bottom = g10 * (1.0 - xf) + g11 * xf
    return top * (1.0 - yf) + bottom * yf


def get_or_create_principled(material):
    if not material.use_nodes:
        material.use_nodes = True
    node_tree = material.node_tree
    for node in node_tree.nodes:
        if node.type == "BSDF_PRINCIPLED":
            return node
    return node_tree.nodes.new("ShaderNodeBsdfPrincipled")


def bake_ao(mesh_object):
    scene = bpy.context.scene
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 24
    scene.render.bake.margin = AO_MARGIN

    material = mesh_object.data.materials[0]
    get_or_create_principled(material)
    node_tree = material.node_tree

    ao_image = bpy.data.images.new("SkullAO", TEXTURE_SIZE, TEXTURE_SIZE, alpha=False)
    bake_node = node_tree.nodes.new("ShaderNodeTexImage")
    bake_node.name = "SkullAOBakeTarget"
    bake_node.image = ao_image
    bake_node.select = True
    node_tree.nodes.active = bake_node

    bpy.ops.object.select_all(action="DESELECT")
    mesh_object.select_set(True)
    bpy.context.view_layer.objects.active = mesh_object
    bpy.ops.object.bake(type="AO")

    pixels = read_image_pixels(ao_image)
    remove_node = bake_node
    bpy.data.images.remove(ao_image)
    node_tree.nodes.remove(remove_node)
    return pixels


def compose_and_apply_diffuse(mesh_object, ao_pixels, output_directory):
    height, width = ao_pixels.shape[0], ao_pixels.shape[1]
    ao = ao_pixels[:, :, 0]
    mottle = (
        0.5 * low_frequency_noise(width, height, 4, 11)
        + 0.3 * low_frequency_noise(width, height, 8, 23)
        + 0.2 * low_frequency_noise(width, height, 16, 37)
    )
    light = np.array([0.70, 0.64, 0.50], dtype=np.float32)
    dark = np.array([0.38, 0.31, 0.21], dtype=np.float32)
    base = dark[np.newaxis, np.newaxis, :] + (
        light - dark
    )[np.newaxis, np.newaxis, :] * mottle[:, :, np.newaxis]
    shade = 0.35 + 0.65 * ao
    rgb = base * shade[:, :, np.newaxis]

    rgba = np.ones((height, width, 4), dtype=np.float32)
    rgba[:, :, :3] = rgb

    diffuse_image = bpy.data.images.new(
        "SkullDiffuse", width, height, alpha=False
    )
    diffuse_image.colorspace_settings.name = "sRGB"
    diffuse_image.pixels.foreach_set(rgba.ravel())
    diffuse_image.update()
    diffuse_image.filepath_raw = os.path.join(output_directory, TEXTURE_FILENAME)
    diffuse_image.file_format = "PNG"
    diffuse_image.save()

    saved_image = bpy.data.images.load(
        os.path.join(output_directory, TEXTURE_FILENAME)
    )
    saved_pixels = read_image_pixels(saved_image)
    rgb_mean = float(saved_pixels[:, :, 0:3].mean())
    bpy.data.images.remove(saved_image)
    if rgb_mean <= 0.01:
        raise RuntimeError(
            f"The saved diffuse texture is empty (mean={rgb_mean})."
        )

    material = mesh_object.data.materials[0]
    bsdf = get_or_create_principled(material)
    node_tree = material.node_tree
    texture_node = node_tree.nodes.new("ShaderNodeTexImage")
    texture_node.name = "SkullDiffuseTexture"
    texture_node.image = diffuse_image
    node_tree.links.new(texture_node.outputs["Color"], bsdf.inputs["Base Color"])
    bsdf.inputs["Base Color"].default_value = (1.0, 1.0, 1.0, 1.0)
    material["_x_texture_filename"] = TEXTURE_FILENAME


def main():
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
    skeleton_path = os.path.join(
        repo_root,
        "RedFortress2",
        "MultiPassRendering",
        "res",
        "model2",
        "Skeleton",
        "Skeleton_clean.blend",
    )
    output_directory = os.path.join(
        repo_root,
        "RedFortress2",
        "MultiPassRendering",
        "res",
        "model",
        "skull",
    )
    blend_path = os.path.join(output_directory, "skull.blend")
    x_path = os.path.join(output_directory, "skull.x")

    os.makedirs(output_directory, exist_ok=True)
    bpy.ops.wm.open_mainfile(filepath=skeleton_path)

    mesh_object = bpy.data.objects.get(SOURCE_MESH_NAME)
    if mesh_object is None or mesh_object.type != "MESH":
        raise RuntimeError(f"Source mesh was not found: {SOURCE_MESH_NAME}")

    keep_head_vertices(mesh_object)
    for modifier in list(mesh_object.modifiers):
        mesh_object.modifiers.remove(modifier)
    for vertex_group in list(mesh_object.vertex_groups):
        mesh_object.vertex_groups.remove(vertex_group)

    for scene_object in list(bpy.data.objects):
        if scene_object != mesh_object:
            bpy.data.objects.remove(scene_object, do_unlink=True)

    mesh_object.name = "Skull"
    mesh_object.data.name = "Skull"
    for material in mesh_object.data.materials:
        if material is not None:
            material["_x_power"] = SKULL_X_POWER
    bake_scale_and_jaw_origin(mesh_object)

    unwrap_skull_uv(mesh_object)
    ao_pixels = bake_ao(mesh_object)
    compose_and_apply_diffuse(mesh_object, ao_pixels, output_directory)

    bpy.ops.object.select_all(action="DESELECT")
    mesh_object.select_set(True)
    bpy.context.view_layer.objects.active = mesh_object
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)

    export_selected(x_path)
    with open(x_path, "rb") as exported_file:
        exported_data = exported_file.read()
    if not exported_data.startswith(b"xof "):
        raise RuntimeError("The exported DirectX X header is invalid.")
    if exported_data.startswith(b"\xef\xbb\xbf"):
        raise RuntimeError("The exported DirectX X file contains a BOM.")

    dimensions = mesh_object.dimensions.copy()
    minimum_export_y = min(vertex.co.z for vertex in mesh_object.data.vertices)
    if abs(minimum_export_y) > 0.0001:
        raise RuntimeError("The skull origin is not on the lower jaw.")

    collision_path = create_collision_model(output_directory)

    print(f"Saved: {blend_path}")
    print(f"Exported: {x_path}")
    print(f"Exported: {collision_path}")
    print(
        "Skull dimensions (Blender XYZ): "
        f"{dimensions.x:.4f}, {dimensions.y:.4f}, {dimensions.z:.4f}"
    )


if __name__ == "__main__":
    main()
