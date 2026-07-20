import os

import bpy


OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
MODEL_DIR = os.path.dirname(OUTPUT_DIR)
BLEND_PATH = os.path.join(OUTPUT_DIR, "ground.blend")
FIELD_TEXTURE_PATH = os.path.join(MODEL_DIR, "field.png")
WHITE_WALL_TEXTURE_PATH = os.path.join(MODEL_DIR, "whiteWall.png")
def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for data_collection in (
        bpy.data.meshes,
        bpy.data.materials,
        bpy.data.images,
    ):
        for data_block in list(data_collection):
            data_collection.remove(data_block)


def create_material(name, texture_path, texture_filename):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = (0.64, 0.64, 0.64, 1.0)
    material.roughness = 1.0
    material.metallic = 0.0
    material["_x_power"] = 0.0
    material["_x_specular"] = (0.0, 0.0, 0.0)
    material["_x_emissive"] = (0.0, 0.0, 0.0)
    material["_x_texture_filename"] = texture_filename

    principled = next(
        node for node in material.node_tree.nodes
        if node.type == "BSDF_PRINCIPLED"
    )
    principled.inputs["Base Color"].default_value = (0.64, 0.64, 0.64, 1.0)
    principled.inputs["Roughness"].default_value = 1.0

    image = bpy.data.images.load(texture_path, check_existing=True)
    texture = material.node_tree.nodes.new("ShaderNodeTexImage")
    texture.name = name + "_Texture"
    texture.image = image
    texture.extension = "REPEAT"
    material.node_tree.links.new(texture.outputs["Color"], principled.inputs["Base Color"])
    return material


def create_mesh_object(name, vertices, faces, uvs, material):
    mesh = bpy.data.meshes.new(name + "Geo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        polygon.use_smooth = False
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]

    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    obj.data.materials.append(material)
    obj["_x_frame_name"] = name
    obj["_x_mesh_name"] = name + "Geo"
    return obj


def create_plane(name, half_width, half_depth, material):
    vertices = (
        (-half_width, -half_depth, 0.0),
        (half_width, -half_depth, 0.0),
        (half_width, half_depth, 0.0),
        (-half_width, half_depth, 0.0),
    )
    faces = ((0, 1, 2, 3),)
    uvs = (
        (0.0, 0.0),
        (1.0, 0.0),
        (1.0, 1.0),
        (0.0, 1.0),
    )
    return create_mesh_object(name, vertices, faces, uvs, material)


def create_fall_hole_plane(material):
    vertices = (
        (-16.0, -32.0, 0.0),
        (-2.0, -32.0, 0.0),
        (-2.0, 32.0, 0.0),
        (-16.0, 32.0, 0.0),
        (2.0, -32.0, 0.0),
        (16.0, -32.0, 0.0),
        (16.0, 32.0, 0.0),
        (2.0, 32.0, 0.0),
        (-2.0, -32.0, 0.0),
        (2.0, -32.0, 0.0),
        (2.0, -6.0, 0.0),
        (-2.0, -6.0, 0.0),
        (-2.0, 6.0, 0.0),
        (2.0, 6.0, 0.0),
        (2.0, 32.0, 0.0),
        (-2.0, 32.0, 0.0),
    )
    faces = (
        (0, 1, 2, 3),
        (4, 5, 6, 7),
        (8, 9, 10, 11),
        (12, 13, 14, 15),
    )
    uvs = (
        (0.0, 0.0),
        (0.4375, 0.0),
        (0.4375, 1.0),
        (0.0, 1.0),
        (0.5625, 0.0),
        (1.0, 0.0),
        (1.0, 1.0),
        (0.5625, 1.0),
        (0.4375, 0.0),
        (0.5625, 0.0),
        (0.5625, 0.40625),
        (0.4375, 0.40625),
        (0.4375, 0.59375),
        (0.5625, 0.59375),
        (0.5625, 1.0),
        (0.4375, 1.0),
    )
    return create_mesh_object(
        "PlateField32x64FallHole",
        vertices,
        faces,
        uvs,
        material,
    )


def export_object(obj, filename):
    bpy.ops.object.select_all(action="DESELECT")
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    result = bpy.ops.export_scene.directx_x(
        filepath=os.path.join(OUTPUT_DIR, filename),
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        export_format="TEXT_X",
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + filename)


def main():
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    clear_scene()
    field_material = create_material(
        "Field",
        FIELD_TEXTURE_PATH,
        "../field.png",
    )
    white_wall_material = create_material(
        "WhiteWall",
        WHITE_WALL_TEXTURE_PATH,
        "../whiteWall.png",
    )

    models = (
        (create_plane("PlateField32x64", 16.0, 32.0, white_wall_material), "plateField32x64.x"),
        (create_fall_hole_plane(field_material), "plateField32x64FallHole.x"),
        (create_plane("PlateField64x128", 64.0, 128.0, field_material), "plateField64x128.x"),
        (create_plane("PlateField256x256", 128.0, 128.0, field_material), "plateField256x256.x"),
        (create_plane("PlateField256x512", 128.0, 256.0, field_material), "plateField256x512.x"),
    )

    bpy.ops.wm.save_as_mainfile(filepath=BLEND_PATH)
    for obj, filename in models:
        export_object(obj, filename)
        print("EXPORTED", filename)
    print("BLEND_PATH", BLEND_PATH)


main()
