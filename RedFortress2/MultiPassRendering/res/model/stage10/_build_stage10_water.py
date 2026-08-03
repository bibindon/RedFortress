from pathlib import Path

import bpy


BASE_DIR = Path(__file__).resolve().parent
BLEND_PATH = BASE_DIR / "stage10_water.blend"
X_PATH = BASE_DIR / "stage10_water.x"

WIDTH = 120.0
DEPTH = 240.0
GRID_X = 48
GRID_Z = 96


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials):
        for data_block in list(collection):
            collection.remove(data_block)


def create_water_material():
    material = bpy.data.materials.new("Stage10WaterDark")
    material.use_nodes = True
    material.diffuse_color = (0.01, 0.01, 0.01, 0.68)
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
    principled.inputs["Base Color"].default_value = (0.01, 0.01, 0.01, 0.68)
    principled.inputs["Roughness"].default_value = 0.24
    principled.inputs["Alpha"].default_value = 0.52
    return material


def create_water_mesh(material):
    vertices = []
    uvs = []
    for z_index in range(GRID_Z + 1):
        z_ratio = z_index / GRID_Z
        depth = (z_ratio - 0.5) * DEPTH
        for x_index in range(GRID_X + 1):
            x_ratio = x_index / GRID_X
            x = (x_ratio - 0.5) * WIDTH
            vertices.append((x, depth, 0.0))
            uvs.append((x_ratio, z_ratio))

    faces = []
    row_width = GRID_X + 1
    for z_index in range(GRID_Z):
        for x_index in range(GRID_X):
            current = z_index * row_width + x_index
            next_row = current + row_width
            faces.append((current, current + 1, next_row + 1, next_row))

    mesh = bpy.data.meshes.new("Stage10WaterSurfaceGeo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(material)

    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        polygon.use_smooth = True
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]

    water = bpy.data.objects.new("Stage10WaterSurface", mesh)
    bpy.context.collection.objects.link(water)
    water["_x_frame_name"] = "Stage10WaterSurface"
    water["_x_mesh_name"] = "Stage10WaterSurfaceGeo"
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
    bpy.context.preferences.filepaths.save_version = 0
    clear_scene()
    material = create_water_material()
    water = create_water_mesh(material)
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    export_x(water)
    print("STAGE10_WATER_BLEND", BLEND_PATH)
    print("STAGE10_WATER_X", X_PATH)


main()
