from pathlib import Path
import math

import bpy


BASE_DIR = Path(__file__).resolve().parent
BLEND_PATH = BASE_DIR / "base_water.blend"
X_PATH = BASE_DIR / "base_water.x"

WIDTH = 18.0
DEPTH = 7.8
RING_COUNT = 20
RING_SEGMENTS = 96


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials):
        for data_block in list(collection):
            collection.remove(data_block)


def create_water_material():
    material = bpy.data.materials.new("BaseWaterDark")
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
    half_width = WIDTH * 0.5
    half_depth = DEPTH * 0.5

    vertices = [(0.0, 0.0, 0.0)]
    uvs = [(0.5, 0.5)]
    for ring_index in range(1, RING_COUNT + 1):
        ring_ratio = ring_index / RING_COUNT
        for segment_index in range(RING_SEGMENTS):
            angle = (math.tau * segment_index) / RING_SEGMENTS
            shoreline_variation = (
                1.0
                + (0.045 * math.sin((angle * 3.0) + 0.4))
                + (0.025 * math.sin((angle * 7.0) - 0.7))
            )
            radius = ring_ratio * shoreline_variation
            x = math.cos(angle) * half_width * radius
            y = math.sin(angle) * half_depth * radius
            vertices.append((x, y, 0.0))
            uvs.append(
                (
                    0.5 + (0.5 * math.cos(angle) * ring_ratio),
                    0.5 + (0.5 * math.sin(angle) * ring_ratio),
                )
            )

    faces = []
    for segment_index in range(RING_SEGMENTS):
        next_segment = (segment_index + 1) % RING_SEGMENTS
        faces.append((0, 1 + segment_index, 1 + next_segment))

    for ring_index in range(1, RING_COUNT):
        inner_start = 1 + ((ring_index - 1) * RING_SEGMENTS)
        outer_start = 1 + (ring_index * RING_SEGMENTS)
        for segment_index in range(RING_SEGMENTS):
            next_segment = (segment_index + 1) % RING_SEGMENTS
            faces.append(
                (
                    inner_start + segment_index,
                    outer_start + segment_index,
                    outer_start + next_segment,
                    inner_start + next_segment,
                )
            )

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
    bpy.context.preferences.filepaths.save_version = 0
    clear_scene()
    material = create_water_material()
    water = create_water_mesh(material)
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    export_x(water)
    print("BASE_WATER_BLEND", BLEND_PATH)
    print("BASE_WATER_X", X_PATH)


main()