from pathlib import Path

import bpy


BASE_DIR = Path(__file__).resolve().parent
BLEND_PATH = BASE_DIR / "base_water.blend"
X_PATH = BASE_DIR / "base_water.x"

WIDTH = 22.0
DEPTH = 8.0
GRID_SPACING = 0.2


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials):
        for data_block in list(collection):
            collection.remove(data_block)


def create_water_material():
    material = bpy.data.materials.new("BaseWaterDark")
    material.use_nodes = True
    material.diffuse_color = (0.24, 0.26, 0.28, 0.52)
    material.roughness = 0.24
    material.metallic = 0.0
    material["_x_power"] = 96.0
    material["_x_specular"] = (0.25, 0.25, 0.25)
    material["_x_emissive"] = (0.0, 0.0, 0.0)

    principled = next(
        node
        for node in material.node_tree.nodes
        if node.type == "BSDF_PRINCIPLED"
    )
    principled.inputs["Base Color"].default_value = (0.24, 0.26, 0.28, 0.52)
    principled.inputs["Roughness"].default_value = 0.24
    principled.inputs["Alpha"].default_value = 0.52
    return material


def create_water_mesh(material):
    x_segments = round(WIDTH / GRID_SPACING)
    y_segments = round(DEPTH / GRID_SPACING)
    half_width = WIDTH * 0.5
    half_depth = DEPTH * 0.5

    vertices = []
    uvs = []
    for y_index in range(y_segments + 1):
        y_ratio = y_index / y_segments
        y = -half_depth + (DEPTH * y_ratio)
        for x_index in range(x_segments + 1):
            x_ratio = x_index / x_segments
            x = -half_width + (WIDTH * x_ratio)
            vertices.append((x, y, 0.0))
            uvs.append((x_ratio, y_ratio))

    row_size = x_segments + 1
    faces = []
    for y_index in range(y_segments):
        for x_index in range(x_segments):
            lower_left = (y_index * row_size) + x_index
            lower_right = lower_left + 1
            upper_left = lower_left + row_size
            upper_right = upper_left + 1
            faces.append((lower_left, lower_right, upper_right, upper_left))

    mesh = bpy.data.meshes.new("BaseWaterSurfaceGeo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(material)

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        polygon.use_smooth = True
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]

    water = bpy.data.objects.new("BaseWaterSurface", mesh)
    bpy.context.collection.objects.link(water)
    water["_x_frame_name"] = "BaseWaterSurface"
    water["_x_mesh_name"] = "BaseWaterSurfaceGeo"
    return water


def export_x(water):
    bpy.ops.object.select_all(action="DESELECT")
    water.select_set(True)
    bpy.context.view_layer.objects.active = water
    result = bpy.ops.export_scene.directx_x(
        filepath=str(X_PATH),
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=False,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        export_format="TEXT_X",
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(X_PATH))


def main():
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    clear_scene()
    material = create_water_material()
    water = create_water_mesh(material)
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    export_x(water)
    print("BASE_WATER_BLEND", BLEND_PATH)
    print("BASE_WATER_X", X_PATH)


main()