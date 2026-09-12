"""Build a walkable masonry goal dais using Blender's official DirectX exporter.
The game places this asset at clearPosition.y - 2 with uniform scale 2.
The lower one-meter foundation is embedded in the nominal goal ground; four
25-cm steps lead to a one-meter-high platform. Footprint remains four meters.
"""
import argparse
import math
from pathlib import Path
import sys

import bmesh
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'RedFortress2/MultiPassRendering/res/model/portal'
OBJECTS = []


def stone_material(name, image, trim=False):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    mat.diffuse_color = (0.6,0.6,0.6,1)
    mat['_x_power'] = 20.0
    mat['_x_specular'] = (0.12,0.12,0.12)
    mat['_x_texture_filename'] = image.name
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    bsdf.inputs['Roughness'].default_value = 0.82
    bsdf.inputs['Specular IOR Level'].default_value = 0.12
    tex = mat.node_tree.nodes.new('ShaderNodeTexImage')
    tex.image = image
    mat.node_tree.links.new(tex.outputs['Color'],bsdf.inputs['Base Color'])
    return mat


def outline(half, cut):
    return [(half,half-cut),(half-cut,half),(-half+cut,half),(-half,half-cut),
            (-half,-half+cut),(-half+cut,-half),(half-cut,-half),(half,-half+cut)]


def block(name, center, levels, material, cut=0.20, trim=False):
    # Each level is (local height, half width). Ring profiles provide actual bevels.
    vertices=[]
    for z,half in levels:
        for x,y in outline(half,min(cut,half*0.45)):
            vertices.append((x+center[0],y+center[1],z))
    faces=[tuple(reversed(range(8)))]
    for j in range(len(levels)-1):
        for i in range(8):
            k=(i+1)%8
            faces.append((j*8+i,j*8+k,(j+1)*8+k,(j+1)*8+i))
    faces.append(tuple((len(levels)-1)*8+i for i in range(8)))
    mesh=bpy.data.meshes.new(name+'Mesh')
    mesh.from_pydata(vertices,[],faces)
    mesh.update()
    bm=bmesh.new()
    bm.from_mesh(mesh)
    bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces))
    assert all(e.is_manifold for e in bm.edges),name
    assert bm.calc_volume(signed=True)>0,name
    assert all(f.calc_area()>1e-10 for f in bm.faces),name
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()
    uv=mesh.uv_layers.new(name='MasonryUV')
    for polygon in mesh.polygons:
        normal=polygon.normal
        for li in polygon.loop_indices:
            p=mesh.vertices[mesh.loops[li].vertex_index].co
            if abs(normal.z)>0.7:
                u,v=p.x/0.9,p.y/0.9
            else:
                tangent=Vector((-normal.y,normal.x,0)).normalized()
                u=p.dot(tangent)/0.9
                v=p.z/0.9
            uv.data[li].uv=(u,v)
    mesh.materials.append(material)
    obj=bpy.data.objects.new(name,mesh)
    bpy.context.collection.objects.link(obj)
    OBJECTS.append(obj)
    return obj


def build():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.context.preferences.filepaths.save_version=0
    bpy.ops.preferences.addon_enable(module='bl_ext.blender_org.io_directx_x')
    image=bpy.data.images.load(str(OUT/'stone_bricks.png'))
    image.filepath='//stone_bricks.png'
    masonry=stone_material('GoalStoneBrick',image)
    coping=stone_material('GoalCutStone',image,True)
    block('BuriedOctagonalFoundation',(0,0),[(0,1.0),(0.5,1.0)],masonry)
    for i in range(4):
        half=1.0-i*0.15
        bottom=0.5+i*0.125
        top=bottom+0.125
        block('MasonryStep'+str(i+1),(0,0),[(bottom,half-0.014),(top-0.028,half-0.014)],masonry)
        block('BeveledCoping'+str(i+1),(0,0),[(top-0.028,half-0.009),
              (top-0.020,half),(top-0.006,half),(top,half-0.008)],coping,trim=True)
    # Low corner piers stand outside the central approach and leave all four sides open.
    for x in (-0.73,0.73):
        for y in (-0.73,0.73):
            label=str(len(OBJECTS))
            block('CornerPierFoot'+label,(x,y),[(0.625,0.135),(0.655,0.145),
                  (0.68,0.135)],coping,cut=0.05,trim=True)
            block('CornerPierBrick'+label,(x,y),[(0.665,0.106),(0.94,0.096)],masonry,cut=0.04)
            block('CornerPierCap'+label,(x,y),[(0.93,0.122),(0.95,0.135),
                  (0.975,0.135),(0.995,0.113)],coping,cut=0.04,trim=True)
    # A low, broad seal supplies a focal point without blocking the goal column.
    block('CentralOctagonalSeal',(0,0),[(1.0,0.355),(1.008,0.35)],coping,cut=0.205,trim=True)
    scene=bpy.context.scene
    scene.unit_settings.system='METRIC'
    scene.unit_settings.scale_length=1.0
    scene['GameScale']=2.0
    scene['GoalDais']='4m footprint; nominal ground local Z=0.5; four 0.25m risers; clear central platform.'
    assert max(v.co.x for o in OBJECTS for v in o.data.vertices)<=1.0
    assert min(v.co.x for o in OBJECTS for v in o.data.vertices)>=-1.0
    print('GEOMETRY',len(OBJECTS),'closed outward components;',
          sum(sum(len(p.vertices)-2 for p in o.data.polygons) for o in OBJECTS),'triangles',flush=True)


def export(collision_only=False):
    # Three stacked boxes approximate the dais without following each visual step.
    boxes = []
    for index, (half, bottom, top) in enumerate([
            (1.0, 0.0, 0.625),
            (0.85, 0.625, 0.875),
            (0.55, 0.875, 1.008)]):
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=(0, 0, (bottom + top) / 2))
        box = bpy.context.object
        box.name = 'GoalCollisionBox' + str(index + 1)
        box.dimensions = (half * 2, half * 2, top - bottom)
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        bm = bmesh.new()
        bm.from_mesh(box.data)
        assert len(bm.verts) == 8 and len(bm.faces) == 6
        assert all(e.is_manifold for e in bm.edges)
        assert bm.calc_volume(signed=True) > 0
        bm.free()
        boxes.append(box)
    bpy.ops.object.select_all(action='DESELECT')
    for box in boxes:
        box.select_set(True)
    bpy.context.view_layer.objects.active = boxes[0]
    bpy.ops.object.join()
    collision = bpy.context.object
    collision.name = 'GoalStepsCollision'
    triangles = sum(len(p.vertices) - 2 for p in collision.data.polygons)
    assert len(collision.data.vertices) == 24 and triangles == 36
    from mathutils.bvhtree import BVHTree
    tree = BVHTree.FromPolygons(
        [v.co for v in collision.data.vertices],
        [tuple(p.vertices) for p in collision.data.polygons])
    for x, height in [(0.925, 0.625), (0.70, 0.875), (0.0, 1.008)]:
        for dx, dy in [(1, 0), (-1, 0), (0, 1), (0, -1)]:
            hit, normal, _, _ = tree.ray_cast(Vector((dx*x, dy*x, 2)), Vector((0, 0, -1)))
            assert hit is not None and abs(hit.z-height) < 1e-5
            assert normal.z > 0.99
    filenames = ['stone_steps_collision.x']
    if not collision_only:
        filenames.insert(0, 'stone_steps.x')
    for filename in filenames:
        is_collision = filename == 'stone_steps_collision.x'
        bpy.ops.object.select_all(action='DESELECT')
        selected = OBJECTS
        if is_collision:
            selected = [collision]
        for obj in selected:
            obj.select_set(True)
        bpy.context.view_layer.objects.active = selected[0]
        result=bpy.ops.export_scene.directx_x(filepath=str(OUT/filename),check_existing=False,
            use_selection=True,axis_forward='Z',axis_up='Y',global_scale=1.0,
            export_normals=True,export_uvs=not is_collision,
            export_materials=not is_collision,export_textures=not is_collision,
            export_animation=False,export_armature=False,export_weights=False,
            triangulate=True,unweld_on_export=True)
        assert 'FINISHED' in result,filename
        path=OUT/filename
        data=path.read_bytes().replace(b'\r\n',b'\n').replace(b'\n',b'\r\n')
        assert data.startswith(b'xof ')
        path.write_bytes(data)
    bpy.data.objects.remove(collision, do_unlink=True)
    print('EXPORT: collision is three boxes, 36 triangles; top surfaces validated.', flush=True)


def preview(directory):
    directory.mkdir(parents=True,exist_ok=True)
    scene=bpy.context.scene
    scene.render.engine='CYCLES'
    scene.cycles.samples=40
    scene.cycles.use_denoising=True
    scene.render.resolution_x=1200
    scene.render.resolution_y=950
    scene.render.resolution_percentage=100
    scene.world=bpy.data.worlds.new('GoalStudio')
    scene.world.use_nodes=True
    scene.world.node_tree.nodes.get('Background').inputs[0].default_value=(0.19,0.21,0.25,1)
    scene.world.node_tree.nodes.get('Background').inputs[1].default_value=0.5
    target=Vector((0,0,0.74))
    for position,energy,size in [((2,-3,4),330,4),((-3,-1,2),170,3),((1,3,3),280,3)]:
        bpy.ops.object.light_add(type='AREA',location=position)
        lamp=bpy.context.object
        lamp.data.energy=energy
        lamp.data.size=size
        lamp.rotation_euler=(target-lamp.location).to_track_quat('-Z','Y').to_euler()
    bpy.ops.object.camera_add(location=(2.8,-3.7,3.0))
    cam=bpy.context.object
    cam.rotation_euler=(target-cam.location).to_track_quat('-Z','Y').to_euler()
    cam.data.type='ORTHO'
    cam.data.ortho_scale=3.15
    scene.camera=cam
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'stone_steps.blend'))
    # Preview plane represents the nominal stage floor, hiding the buried foundation.
    bpy.ops.mesh.primitive_plane_add(size=200,location=(0,0,0.499))
    floor=bpy.context.object
    floor.name='PreviewOnlyGround'
    mat=bpy.data.materials.new('PreviewFloor')
    mat.diffuse_color=(0.065,0.08,0.10,1)
    mat.use_nodes=True
    mat.node_tree.nodes.get('Principled BSDF').inputs['Base Color'].default_value=(0.065,0.08,0.10,1)
    mat.node_tree.nodes.get('Principled BSDF').inputs['Roughness'].default_value=0.9
    floor.data.materials.append(mat)
    scene.render.filepath=str(directory/'goal-portal-stone-brick.png')
    bpy.ops.render.render(write_still=True)


if __name__=='__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('--preview-dir',type=Path)
    parser.add_argument('--collision-only', action='store_true')
    args=[]
    if '--' in sys.argv:
        args=sys.argv[sys.argv.index('--')+1:]
    options=parser.parse_args(args)
    build()
    export(options.collision_only)
    if options.collision_only:
        sys.exit(0)
    if options.preview_dir:
        preview(options.preview_dir)
    else:
        bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'stone_steps.blend'))