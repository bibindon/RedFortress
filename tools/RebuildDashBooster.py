"""Rebuild DashBooster as a carriage-free iron cannon with wooden wheels.
Run with Blender 5.1: blender --background --python tools/RebuildDashBooster.py
Geometry is authored in Blender (+Z up, +Y muzzle) to preserve the existing
DashBooster local +Z firing axis after MeshMix2's official-axis correction.
"""
import argparse
import math
from pathlib import Path
import sys

import bmesh
import bpy
from mathutils import Vector

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / 'RedFortress2/MultiPassRendering/res/model/dashBooster'
OBJECTS = []


def texture(name):
    # Authored albedo assets are retained on rebuild, never replaced with noise.
    path = OUT / name
    assert path.is_file(), 'Missing authored texture: ' + str(path)
    image = bpy.data.images.load(str(path))
    image.filepath = '//' + name
    return image


def material(name, color, image=None, roughness=0.3, metallic=0.0):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1)
    mat.specular_intensity = 1.0
    mat.use_nodes = True
    mat['_x_power'] = 500.0
    mat['_x_specular'] = (1.0, 1.0, 1.0)
    mat['_x_face_color'] = (*color, 1)
    bsdf = mat.node_tree.nodes.get('Principled BSDF')
    bsdf.inputs['Base Color'].default_value = (*color, 1)
    bsdf.inputs['Roughness'].default_value = roughness
    bsdf.inputs['Metallic'].default_value = metallic
    bsdf.inputs['Specular IOR Level'].default_value = 1.0
    if image is not None:
        tex = mat.node_tree.nodes.new('ShaderNodeTexImage')
        tex.image = image
        mat.node_tree.links.new(tex.outputs['Color'], bsdf.inputs['Base Color'])
        mat['_x_texture_filename'] = image.name
    return mat


def lathe(name, profile, center, axis, mat, segments=16, swap_uv=False):
    """Closed surface of revolution; poles are single vertices, never zero-area quads."""
    rotation = Vector((0, 0, 1)).rotation_difference(Vector(axis))
    origin = Vector(center)
    vertices, rings, faces, face_uvs = [], [], [], []
    distance = [0.0]
    for a, b in zip(profile, profile[1:]):
        distance.append(distance[-1] + math.hypot(b[0]-a[0], b[1]-a[1]))
    for axial, radius in profile:
        ring = []
        count = segments
        if radius == 0:
            count = 1
        for i in range(count):
            angle = i * math.tau / segments
            ring.append(len(vertices))
            vertices.append(origin + rotation @ Vector((radius*math.cos(angle), radius*math.sin(angle), axial)))
        rings.append(ring)
    for j in range(len(rings)-1):
        a, b = rings[j:j+2]
        for i in range(segments):
            k = (i+1) % segments
            u, u1 = i/segments, (i+1)/segments
            v, v1 = distance[j]/distance[-1], distance[j+1]/distance[-1]
            if len(a) == 1:
                faces.append((a[0], b[k], b[i]))
                face_uvs.append(((u,v), (u1,v1), (u,v1)))
            elif len(b) == 1:
                faces.append((a[i], a[k], b[0]))
                face_uvs.append(((u,v), (u1,v), (u,v1)))
            else:
                faces.append((a[i], a[k], b[k], b[i]))
                face_uvs.append(((u,v), (u1,v), (u1,v1), (u,v1)))
    mesh = bpy.data.meshes.new(name + 'Mesh')
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    uv = mesh.uv_layers.new(name='SurfaceUV')
    for poly, coords in zip(mesh.polygons, face_uvs):
        poly.use_smooth = True
        for li, coord in zip(poly.loop_indices, coords):
            if swap_uv:
                coord = (coord[1], coord[0])
            if '_Spoke' in name:
                coord = (coord[0]*0.20, coord[1])
            uv.data[li].uv = coord
    bm = bmesh.new()
    bm.from_mesh(mesh)
    # Closed profile loops deliberately repeat their first ring; weld only geometry.
    bmesh.ops.remove_doubles(bm, verts=list(bm.verts), dist=0.0000001)
    bmesh.ops.recalc_face_normals(bm, faces=list(bm.faces))
    assert all(e.is_manifold for e in bm.edges), name + ': open/nonmanifold edge'
    assert bm.calc_volume(signed=True) > 0, name + ': inverted surface'
    assert all(f.calc_area() > 1e-10 for f in bm.faces), name + ': degenerate face'
    bm.to_mesh(mesh)
    bm.free()
    mesh.update()
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.collection.objects.link(obj)
    mesh.materials.append(mat)
    OBJECTS.append(obj)
    return obj


def ring(name, center, axis, radius, inner, width, mat, swap_uv=False):
    h = width/2
    if swap_uv:
        # Preserve the wheel rim bevels while reducing the circumferential rings.
        bevel = width * 0.16
        profile = [(-h,inner),(-h,radius-bevel),(-h+bevel,radius),
                   (h-bevel,radius),(h,radius-bevel),(h,inner),(-h,inner)]
        segments = 20
    else:
        # A triangular raised cross section reads as a band at game scale.
        profile = [(-h,inner),(0,radius),(h,inner),(-h,inner)]
        segments = 12
    return lathe(name, profile, center, axis, mat, segments, swap_uv)


def build():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.preferences.addon_enable(module='bl_ext.blender_org.io_directx_x')
    iron_image = texture('dashBooster_iron.png')
    wood_image = texture('dashBooster_wood.png')
    iron = material('CannonGrayIron', (0.30,0.315,0.33), iron_image, 0.27, 0.62)
    wood = material('WheelBrownWood', (0.37,0.175,0.065), wood_image, 0.4)
    edge = material('IronMachinedEdges', (0.24,0.26,0.28), roughness=0.24, metallic=0.65)
    # Barrel is a single closed shell, including lip, bore and the recessed breech.
    profile = [(-0.53,0),(-0.51,0.17),(-0.40,0.222),(-0.20,0.23),
               (0.03,0.212),(0.36,0.186),(0.48,0.22),(0.57,0.229),
               (0.602,0.214),(0.602,0.16),(0.55,0.142),(-0.28,0.132),
               (-0.36,0.105),(-0.36,0)]
    barrel = lathe('Barrel_HollowGrayIron', profile, (0,0,0.35), (0,1,0), iron, 16)
    # Independent bore normal check: normals must face INTO the empty bore.
    count = 0
    for p in barrel.data.polygons:
        c = p.center
        radial = Vector((c.x,0,c.z-0.35))
        if -0.27 < c.y < 0.30 and radial.length < 0.15:
            assert p.normal.dot(radial) < 0, 'Bore normal points into the iron'
            count += 1
    assert count >= 16, 'Bore normal check did not sample the wall'
    for name,y,r in [('BreechBand',-0.34,0.24),('ChaseBand',0.045,0.219)]:
        ring(name,(0,y,0.35),(0,1,0),r,r-0.025,0.036,edge)
    lathe('RearCascabel', [(-0.64,0),(-0.62,0.052),(-0.595,0.06),
          (-0.56,0.031),(-0.515,0.035),(-0.515,0)],
          (0,0,0.35),(0,1,0),iron,8)
    # Two wheels only: no carriage, ground stand, chain or orientation-dependent tail.
    for side,label in [(-1,'Left'),(1,'Right')]:
        x = side*0.345
        center = Vector((x,-0.12,0.325))
        ring('Wheel'+label+'_WoodRim',center,(1,0,0),0.42,0.343,0.092,wood,True)
        lathe('Wheel'+label+'_WoodHub', [(-0.069,0),(-0.056,0.078),
              (0.056,0.078),(0.069,0)],center,(1,0,0),wood,8,True)
        for i in range(8):
            a = math.tau*i/8 + math.pi/8
            radial = Vector((0,math.cos(a),math.sin(a)))
            lathe('Wheel'+label+'_Spoke'+str(i),[(0.045,0),(0.045,0.032),
                  (0.36,0.022),(0.36,0)],
                  center,radial,wood,4)
        # Short side trunnions cannot obstruct the bore.
        lathe('Trunnion'+label,[(0.195,0),(0.195,0.046),(0.325,0.059),(0.325,0)],
              (0,-0.12,0.325),(side,0,0),iron,8)
        lathe('AxleCap'+label,[(0,0),(0,0.033),(0.014,0.033),(0.024,0)],
              (side*0.414,-0.12,0.325),(side,0,0),edge,8)
    return barrel


def export():
    triangle_count = sum(sum(len(p.vertices)-2 for p in o.data.polygons) for o in OBJECTS)
    assert 1450 <= triangle_count <= 1550, triangle_count
    print('TRIANGLE_COUNT', triangle_count, flush=True)
    bpy.ops.object.select_all(action='DESELECT')
    for obj in OBJECTS:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = OBJECTS[0]
    bpy.context.scene.unit_settings.system = 'METRIC'
    bpy.context.scene.unit_settings.scale_length = 1.0
    bpy.context.scene['DashBoosterDesign'] = 'Gray hollow iron barrel; brown wheels; no carriage. Muzzle Blender +Y => game +Z.'
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'dashBooster_alt.blend'))
    result = bpy.ops.export_scene.directx_x(filepath=str(OUT/'dashBooster_alt.x'),
        check_existing=False, use_selection=True, axis_forward='Z', axis_up='Y',
        global_scale=1.0, export_animation=False, export_armature=False,
        export_weights=False, export_normals=True, export_uvs=True,
        export_materials=True, export_textures=True, triangulate=True, unweld_on_export=True)
    assert 'FINISHED' in result
    path = OUT/'dashBooster_alt.x'
    # Encoding/newline normalization only; never rewrite exported geometry/materials.
    data = path.read_bytes().replace(b'\r\n',b'\n').replace(b'\n',b'\r\n')
    path.write_bytes(data)
    assert data.startswith(b'xof ')
    assert b'dashBooster.png' not in data
    assert b'dashBooster_iron.png' in data and b'dashBooster_wood.png' in data
    assert data.count(b'500.000000;') == 3
    print('VALIDATED',len(OBJECTS),'closed outward meshes; bore inward; separate wood/iron UV; power 500',flush=True)


def preview(directory):
    directory.mkdir(parents=True,exist_ok=True)
    scene = bpy.context.scene
    scene.render.engine = 'CYCLES'
    scene.cycles.samples = 40
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 1100
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.world = bpy.data.worlds.new('Studio')
    scene.world.use_nodes = True
    scene.world.node_tree.nodes.get('Background').inputs[0].default_value = (0.16,0.18,0.21,1)
    scene.world.node_tree.nodes.get('Background').inputs[1].default_value = 0.6
    target = Vector((0,0,0.31))
    for loc,energy,size in [((1.5,1.7,3),190,3),((-2,0.5,1.5),130,2),((0,-2,2),220,2)]:
        bpy.ops.object.light_add(type='AREA',location=loc)
        lamp=bpy.context.object
        lamp.data.energy=energy
        lamp.data.shape='DISK'
        lamp.data.size=size
        lamp.rotation_euler=(target-lamp.location).to_track_quat('-Z','Y').to_euler()
    bpy.ops.object.camera_add(location=(1.8,2.6,1.25))
    camera=bpy.context.object
    scene.camera=camera
    camera.data.type='ORTHO'
    camera.data.ortho_scale=1.8
    views=[('front',(1.8,2.6,1.25)),('rear',(-1.8,-2.6,1.2)),('upward',(1.8,2.6,1.25))]
    for name,position in views:
        camera.location=position
        camera.rotation_euler=(target-camera.location).to_track_quat('-Z','Y').to_euler()
        if name=='upward':
            from mathutils import Matrix
            transform=Matrix.Rotation(math.pi/2,4,'X')
            for obj in OBJECTS:
                obj.matrix_world=transform
            rotated_target=transform @ target
            camera.location=Vector(position) + rotated_target - target
            camera.rotation_euler=(rotated_target-camera.location).to_track_quat('-Z','Y').to_euler()
        scene.render.filepath=str(directory/('dashbooster-lowpoly-'+name+'.png'))
        bpy.ops.render.render(write_still=True)
    print('PREVIEWS',str(directory),flush=True)


if __name__=='__main__':
    parser=argparse.ArgumentParser()
    parser.add_argument('--preview-dir',type=Path)
    args=[]
    if '--' in sys.argv:
        args=sys.argv[sys.argv.index('--')+1:]
    options=parser.parse_args(args)
    build()
    export()
    if options.preview_dir:
        preview(options.preview_dir)