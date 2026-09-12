"""Build a small green octahedron for the goal portal effect.

Output: RedFortress2/MultiPassRendering/res/model/portal/green_octahedron.x
The mesh is a regular octahedron with 0.3m vertex-to-vertex distance.
Game code instantiates it ~20 times via AddMeshMix and animates rise+shrink.
Weak emission is configured via adjacent green_octahedron.csv (not here).
"""
import argparse
from pathlib import Path
import sys

import bmesh
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'RedFortress2/MultiPassRendering/res/model/portal'

HALF = 0.15


def build():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.preferences.addon_enable(module='bl_ext.blender_org.io_directx_x')

    verts = [
        (0.0, 0.0, HALF),
        (0.0, 0.0, -HALF),
        (HALF, 0.0, 0.0),
        (0.0, HALF, 0.0),
        (-HALF, 0.0, 0.0),
        (0.0, -HALF, 0.0),
    ]
    faces = [
        (0, 2, 3),
        (0, 3, 4),
        (0, 4, 5),
        (0, 5, 2),
        (1, 3, 2),
        (1, 4, 3),
        (1, 5, 4),
        (1, 2, 5),
    ]
    mesh = bpy.data.meshes.new('GreenOctahedronMesh')
    mesh.from_pydata(verts, [], faces)
    mesh.update()
    bm = bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()
    uv = mesh.uv_layers.new(name='OctaUV')
    # 各面をテクスチャ 1 枚いっぱいに平面投影する（立方体の各面と同じ見え方にする）。
    margin = 0.06
    for poly in mesh.polygons:
        normal = poly.normal.copy()
        helper = Vector((0.0, 0.0, 1.0))
        if abs(normal.dot(helper)) > 0.9:
            helper = Vector((1.0, 0.0, 0.0))
        tangent = helper.cross(normal).normalized()
        bitangent = normal.cross(tangent).normalized()
        coords = []
        for vi in poly.vertices:
            co = mesh.vertices[vi].co
            coords.append((co.dot(tangent), co.dot(bitangent)))
        min_u = min(c[0] for c in coords)
        max_u = max(c[0] for c in coords)
        min_v = min(c[1] for c in coords)
        max_v = max(c[1] for c in coords)
        span_u = (max_u - min_u) if (max_u - min_u) > 1e-6 else 1.0
        span_v = (max_v - min_v) if (max_v - min_v) > 1e-6 else 1.0
        for li, (u, v) in zip(poly.loop_indices, coords):
            uv.data[li].uv = (
                margin + (u - min_u) / span_u * (1.0 - 2.0 * margin),
                margin + (v - min_v) / span_v * (1.0 - 2.0 * margin),
            )
    mat = bpy.data.materials.new('GoalOctaGreen')
    mat.use_nodes = True
    # 立方体 (cube_green.x) と同じく、色はテクスチャ側で持つためマテリアルは無彩色にする。
    mat.diffuse_color = (0.64, 0.64, 0.64, 1.0)
    mat['_x_power'] = 20.0
    mat['_x_specular'] = (0.1, 0.1, 0.1)
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    bsdf.inputs['Base Color'].default_value = (0.64, 0.64, 0.64, 1.0)
    bsdf.inputs['Roughness'].default_value = 0.45
    if 'Specular IOR Level' in bsdf.inputs:
        bsdf.inputs['Specular IOR Level'].default_value = 0.2
    # ステージセレクトの緑キューブと同じテクスチャを貼る（.x には ../cubeGreen/... の相対パスで書き出される）。
    texture_path = ROOT / 'RedFortress2/MultiPassRendering/res/model/cubeGreen/cube_green.png'
    image = bpy.data.images.load(str(texture_path), check_existing=True)
    tex_node = mat.node_tree.nodes.new('ShaderNodeTexImage')
    tex_node.image = image
    tex_node.label = 'cube_green'
    tex_node.location = (-360.0, 220.0)
    mat.node_tree.links.new(tex_node.outputs['Color'], bsdf.inputs['Base Color'])
    mesh.materials.append(mat)
    obj = bpy.data.objects.new('GreenOctahedron', mesh)
    bpy.context.collection.objects.link(obj)
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    scene = bpy.context.scene
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.scale_length = 1.0
    return obj


def export(obj):
    bpy.ops.object.select_all(action='DESELECT')
    obj.select_set(True)
    bpy.context.view_layer.objects.active = obj
    path = OUT / 'green_octahedron.x'
    result = bpy.ops.export_scene.directx_x(
        filepath=str(path), check_existing=False,
        use_selection=True, axis_forward='Z', axis_up='Y', global_scale=1.0,
        export_normals=True, export_uvs=True, export_materials=True,
        export_textures=True, export_animation=False, export_armature=False,
        export_weights=False, triangulate=True, unweld_on_export=True)
    assert 'FINISHED' in result, 'export failed'
    data = path.read_bytes().replace(b'\r\n', b'\n').replace(b'\n', b'\r\n')
    assert data.startswith(b'xof ')
    path.write_bytes(data)
    print('EXPORT: green_octahedron.x', len(data), 'bytes', flush=True)


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--save-blend', action='store_true')
    args = []
    if '--' in sys.argv:
        args = sys.argv[sys.argv.index('--') + 1:]
    options = parser.parse_args(args)
    obj = build()
    export(obj)
    if options.save_blend:
        bpy.ops.wm.save_as_mainfile(filepath=str(OUT / 'green_octahedron.blend'))
