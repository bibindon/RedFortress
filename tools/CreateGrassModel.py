import bpy
from pathlib import Path


OBJECT_NAME = "grass01"
MATERIAL_NAME = "grass01"


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for mesh in list(bpy.data.meshes):
        bpy.data.meshes.remove(mesh)
    for material in list(bpy.data.materials):
        bpy.data.materials.remove(material)


def create_grass_mesh():
    vertices = (
        (-0.500000, 0.000000, 0.500000),
        (0.500000, 0.000000, 0.000000),
        (0.500000, 0.000000, 0.500000),
        (0.500000, 0.000000, 0.000000),
        (-0.500000, 0.000000, 0.500000),
        (-0.500000, 0.000000, 0.000000),
        (0.355863, -0.352597, 0.000000),
        (-0.351245, 0.354510, 0.500000),
        (-0.351245, 0.354510, 0.000000),
        (-0.351245, 0.354510, 0.500000),
        (0.355863, -0.352597, 0.000000),
        (0.355863, -0.352597, 0.500000),
    )
    faces = (
        (0, 1, 2),
        (3, 4, 5),
        (6, 7, 8),
        (9, 10, 11),
    )
    face_uvs = (
        ((0.0, 0.0), (1.0, 1.0), (1.0, 0.0)),
        ((1.0, 1.0), (0.0, 0.0), (0.0, 1.0)),
        ((1.0, 1.0), (0.0, 0.0), (0.0, 1.0)),
        ((0.0, 0.0), (1.0, 1.0), (1.0, 0.0)),
    )

    mesh = bpy.data.meshes.new(OBJECT_NAME)
    mesh.from_pydata(vertices, [], faces)
    mesh.validate()
    mesh.update()

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon, polygon_uvs in zip(mesh.polygons, face_uvs):
        for loop_index, uv in zip(polygon.loop_indices, polygon_uvs):
            uv_layer.data[loop_index].uv = uv

    grass_object = bpy.data.objects.new(OBJECT_NAME, mesh)
    bpy.context.collection.objects.link(grass_object)
    grass_object.location = (0.0, 0.0, 0.0)
    grass_object.rotation_euler = (0.0, 0.0, 0.0)
    grass_object.scale = (1.0, 1.0, 1.0)
    return grass_object


def create_grass_material(texture_path):
    material = bpy.data.materials.new(MATERIAL_NAME)
    material.use_nodes = True
    material.diffuse_color = (1.0, 1.0, 1.0, 1.0)

    nodes = material.node_tree.nodes
    links = material.node_tree.links
    principled = None
    for node in nodes:
        if node.bl_idname == "ShaderNodeBsdfPrincipled":
            principled = node
            break
    if principled is None:
        raise RuntimeError("Principled BSDF node was not found")
    image_node = nodes.new("ShaderNodeTexImage")
    image_node.name = "grass01"
    image_node.image = bpy.data.images.load(str(texture_path), check_existing=True)
    links.new(image_node.outputs["Color"], principled.inputs["Base Color"])
    links.new(image_node.outputs["Alpha"], principled.inputs["Alpha"])
    principled.inputs["Roughness"].default_value = 1.0

    if hasattr(material, "surface_render_method"):
        material.surface_render_method = "DITHERED"
    return material


def normalize_x_file(path):
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    data = data.replace(b"\n", b"\r\n")
    path.write_bytes(data)


def main():
    repository_root = Path(__file__).resolve().parents[1]
    asset_directory = (
        repository_root
        / "RedFortress2"
        / "MultiPassRendering"
        / "res"
        / "model"
        / "grass"
    )
    texture_path = asset_directory / "grass01.png"
    blend_path = asset_directory / "grass.blend"
    x_path = asset_directory / "grass.x"

    if not texture_path.exists():
        raise RuntimeError(f"Grass texture was not found: {texture_path}")

    clear_scene()
    grass_object = create_grass_mesh()
    material = create_grass_material(texture_path)
    grass_object.data.materials.append(material)

    bpy.context.view_layer.objects.active = grass_object
    grass_object.select_set(True)
    bpy.data.use_autopack = False
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))

    result = bpy.ops.export_scene.directx_x(
        filepath=str(x_path),
        check_existing=False,
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="-Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        triangulate=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"DirectX X export failed: {x_path}")

    normalize_x_file(x_path)
    print(f"GRASS_BLEND {blend_path}")
    print(f"GRASS_X {x_path}")
    print(f"GRASS_TRIANGLES {len(grass_object.data.polygons)}")


main()
