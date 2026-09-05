"""Rebuild the moonless mountain in world3.blend using the official X exporter.

Blender Z is up, meters; portal coordinates map Blender XY to game XZ.
The river is exported separately and uses MeshType Water, never WaterMirror.
"""
import csv
import math
import os
import random

import bpy
import numpy as np
from mathutils import Vector

OUT = os.path.dirname(bpy.data.filepath)
RNG = random.Random(3309)
PORTALS = []
with open(os.path.join(OUT, "Interactables.csv"), encoding="utf-8-sig", newline="") as stream:
    for row in csv.DictReader(stream):
        PORTALS.append((float(row["PosX"]), float(row["PosZ"]), float(row["PosY"])))
assert len(PORTALS) == 11


def material(name, color, texture=None):
    m = bpy.data.materials.new(name)
    m.diffuse_color = (*color, 1)
    m.use_nodes = True
    m.roughness = 0.92
    m.specular_intensity = 0.0
    bsdf = m.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1)
    bsdf.inputs["Roughness"].default_value = 0.92
    bsdf.inputs["Specular IOR Level"].default_value = 0.0
    if texture:
        node = m.node_tree.nodes.new("ShaderNodeTexImage")
        node.image = bpy.data.images.load(os.path.join(OUT, texture), check_existing=True)
        node.image.filepath = "//" + texture
        m.node_tree.links.new(node.outputs["Color"], bsdf.inputs["Base Color"])
    return m


def mesh(name, vertices, faces, mat, smooth=False):
    data = bpy.data.meshes.new(name)
    data.from_pydata(vertices, [], faces)
    data.update()
    data.materials.append(mat)
    uv = data.uv_layers.new(name="UVMap")
    for p in data.polygons:
        p.use_smooth = smooth
        axis = max(range(3), key=lambda a: abs(p.normal[a]))
        axes = [a for a in range(3) if a != axis]
        for loop in p.loop_indices:
            v = data.vertices[data.loops[loop].vertex_index].co
            uv.data[loop].uv = (v[axes[0]] * 0.25, v[axes[1]] * 0.25)
    obj = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(obj)
    return obj


def boundary(t):
    r = 1 + 0.06 * math.sin(t * 5) + 0.035 * math.cos(t * 9)
    return (29 * r * math.cos(t), 6 + 31 * r * math.sin(t))


def base_height(x, y):
    return 0.2 + (y + 9) * 0.19 - x * x * 0.005


ANCHORS = [(x, y, z - 0.45) for x, y, z in PORTALS]
ANCHORS.extend([(0, 22, 7.0), (-4, 23, 6.95), (4, 23, 6.95), (0, 27, 6.8)])
for i in range(24):
    x, y = boundary(math.tau * i / 24)
    ANCHORS.append((x, y, -3.0))
XY = np.array([(x, y) for x, y, z in ANCHORS])
delta = XY[:, None, :] - XY[None, :, :]
K = np.sqrt(np.sum(delta * delta, axis=2) + 16)
WEIGHTS = np.linalg.solve(K, np.array([z - base_height(x, y) for x, y, z in ANCHORS]))


def river_x(y):
    return 13.5 + 2.1 * math.sin((y + 1) * 0.15) + 0.4 * math.sin(y * 0.5)


def raw_height(x, y):
    d = XY - (x, y)
    return base_height(x, y) + float(np.sqrt(np.sum(d * d, axis=1) + 16) @ WEIGHTS)


def height(x, y):
    t = math.atan2((y - 6) / 31, x / 29)
    bx, by = boundary(t)
    ratio = math.hypot(x, y - 6) / math.hypot(bx, by - 6)
    z = raw_height(x, y)
    if ratio > 1:
        z = raw_height(bx, by) - 0.035 * math.hypot(x - bx, y - by)
    if -70 < y < 23:
        dist = abs(x - river_x(y))
        z -= 0.8 * math.exp(-((dist / 1.3) ** 4))
    return z


def foothills(stone, grass):
    vertices, faces = [], []
    segments, rings = 160, 24
    for r in range(rings + 1):
        factor = 1 + 1.8 * r / rings
        for i in range(segments):
            x, y = boundary(math.tau * i / segments)
            x *= factor
            y = 6 + (y - 6) * factor
            vertices.append((x, y, height(x, y)))
    for r in range(rings):
        for i in range(segments):
            a = r * segments + i
            b = r * segments + (i + 1) % segments
            faces.append((a, a + segments, b + segments, b))
    mesh("RF3R_ExtendedFoothills", vertices, faces, grass, True)


def vegetation():
    rng = random.Random(3317)
    green = material("RF3R_UnderstoryGreen", (0.20, 0.40, 0.24))
    tips = material("RF3R_UnderstoryTips", (0.38, 0.53, 0.28))
    white = material("RF3R_WhiteWildflowers", (0.90, 0.91, 0.76))
    violet = material("RF3R_VioletWildflowers", (0.58, 0.44, 0.79))
    buffers = [([], []) for unused in range(4)]
    routes = [PORTALS[:9], [PORTALS[0], PORTALS[10]],
              [PORTALS[8], (9, 19, 0), (20, 12, 0), (21, 1, 0), PORTALS[9]],
              [PORTALS[8], (0, 20.5, 7.1)]]

    def clear(x, y):
        if min(math.hypot(x - px, y - py) for px, py, pz in PORTALS) < 1.8:
            return False
        if abs(x - river_x(y)) < 1.45 or math.hypot(x, y - 22) < 5.5:
            return False
        p = Vector((x, y))
        for route in routes:
            for a, b in zip(route, route[1:]):
                a, b = Vector(a[:2]), Vector(b[:2])
                d = b - a
                u = max(0, min(1, (p - a).dot(d) / d.length_squared))
                if (p - (a + d * u)).length < 1.05:
                    return False
        return True

    def face(points, index):
        vertices, faces = buffers[index]
        start = len(vertices)
        vertices.extend(points)
        faces.append(tuple(start + i for i in range(len(points))))
        # A tiny offset prevents coincident front/back faces in the game renderer.
        normal = (Vector(points[1]) - Vector(points[0])).cross(Vector(points[2]) - Vector(points[0])).normalized() * 0.003
        start = len(vertices)
        vertices.extend(Vector(p) - normal for p in points)
        faces.append(tuple(start + i for i in reversed(range(len(points)))))

    centers = [(-13,0),(-12,7),(-8,4),(1,4),(8,12),(-11,17),(-21,-9),(-24,-3),
               (-18,9),(-18,14),(-14,22),(21,-10),(-19,-13),(-8,-15),(2,-14),
               (-5,7),(3,13),(4,-6),(-13,-5),(-2,-10),(8,-13),(-25,-18),(26,-18)]
    for y in range(-26, 22, 4):
        centers.extend([(river_x(y) - 2, y), (river_x(y) + 2, y)])
    for cx, cy in centers:
        for unused in range(45):
            angle = rng.uniform(0, math.tau)
            radius = rng.uniform(0.1, 2.1)
            x, y = cx + radius * math.cos(angle), cy + radius * math.sin(angle)
            if not clear(x, y):
                continue
            z = height(x, y) + 0.025
            h = rng.uniform(0.18, 0.48)
            for blade in range(3):
                t = angle + blade * 2.1
                side = Vector((math.cos(t), math.sin(t), 0)) * 0.09
                base = Vector((x, y, z))
                top = base + Vector((0.14 * math.cos(t + 1), 0.14 * math.sin(t + 1), h))
                face((base - side, base + side, top), blade % 2)
            if rng.random() < 0.22:
                top = Vector((x, y, z + h + 0.1))
                index = 2 + rng.randrange(2)
                for petal in range(5):
                    t = petal * math.tau / 5
                    along = Vector((math.cos(t), math.sin(t), -0.12)) * 0.16
                    side = Vector((-math.sin(t), math.cos(t), 0)) * 0.065
                    face((top, top + along * 0.7 - side, top + along, top + along * 0.7 + side), index)
        # Broad fern rosettes near the center of each patch.
        for unused in range(3):
            x, y = cx + rng.uniform(-1, 1), cy + rng.uniform(-1, 1)
            if not clear(x, y):
                continue
            root = Vector((x, y, height(x, y) + 0.05))
            for frond in range(7):
                t = frond * math.tau / 7 + rng.uniform(-0.15, 0.15)
                direction = Vector((math.cos(t), math.sin(t), 0))
                side = Vector((-math.sin(t), math.cos(t), 0))
                for step in range(1, 6):
                    u = step / 6
                    center = root + direction * (u * 0.8) + Vector((0, 0, 0.5 * math.sin(u * math.pi * 0.85)))
                    width = 0.19 * (1 - u) + 0.035
                    for sign in (-1, 1):
                        tip = center + side * width * sign + direction * 0.09
                        face((center - direction * 0.05, tip, center + direction * 0.07), step % 2)
    for i, mat in enumerate((green, tips, white, violet)):
        mesh("RF3R_BotanicalBeds_" + str(i), buffers[i][0], buffers[i][1], mat)


def terrain(stone, grass):
    vertices, faces = [(0, 6, height(0, 6))], []
    rings, segments = 80, 160
    for r in range(1, rings + 1):
        for i in range(segments):
            x, y = boundary(math.tau * i / segments)
            x *= r / rings
            y = 6 + (y - 6) * r / rings
            vertices.append((x, y, height(x, y)))
    for i in range(segments):
        faces.append((0, i + 1, (i + 1) % segments + 1))
    for r in range(rings - 1):
        for i in range(segments):
            a = 1 + r * segments + i
            b = 1 + r * segments + (i + 1) % segments
            faces.append((a, a + segments, b + segments, b))
    land = mesh("RF3R_Mountain", vertices, faces, stone, True)
    for loop in land.data.loops:
        p = land.data.vertices[loop.vertex_index].co
        land.data.uv_layers.active.data[loop.index].uv = ((p.x + 34) / 68, (p.y + 30) / 76)
    shader = bpy.data.materials.new("RF3R_TerrainBake")
    shader.use_nodes = True
    n, links = shader.node_tree.nodes, shader.node_tree.links
    geom = n.new("ShaderNodeNewGeometry")
    scale = n.new("ShaderNodeVectorMath")
    scale.operation = "SCALE"
    scale.inputs[3].default_value = 0.22
    links.new(geom.outputs["Position"], scale.inputs[0])
    images = []
    for mat in (stone, grass):
        tex = n.new("ShaderNodeTexImage")
        tex.image = next(node.image for node in mat.node_tree.nodes if node.type == "TEX_IMAGE")
        tex.projection = "BOX"
        tex.projection_blend = 0.35
        links.new(scale.outputs["Vector"], tex.inputs[0])
        images.append(tex)
    sep = n.new("ShaderNodeSeparateXYZ")
    links.new(geom.outputs["Normal"], sep.inputs[0])
    remap = n.new("ShaderNodeMapRange")
    remap.inputs["From Min"].default_value = 0.68
    remap.inputs["From Max"].default_value = 0.97
    remap.inputs["To Max"].default_value = 0.72
    links.new(sep.outputs["Z"], remap.inputs[0])
    mix = n.new("ShaderNodeMixRGB")
    links.new(remap.outputs[0], mix.inputs[0])
    links.new(images[0].outputs[0], mix.inputs[1])
    links.new(images[1].outputs[0], mix.inputs[2])
    links.new(mix.outputs[0], n.get("Principled BSDF").inputs["Base Color"])
    image = bpy.data.images.new("rf3_mountain_albedo", width=2048, height=2048, alpha=False)
    target = n.new("ShaderNodeTexImage")
    target.image = image
    n.active = target
    land.data.materials.clear()
    land.data.materials.append(shader)
    bpy.ops.object.select_all(action="DESELECT")
    land.select_set(True)
    bpy.context.view_layer.objects.active = land
    bpy.context.scene.render.engine = "CYCLES"
    bpy.context.scene.cycles.samples = 1
    bpy.context.scene.render.bake.margin = 16
    bpy.ops.object.bake(type="DIFFUSE", pass_filter={"COLOR"})
    image.filepath_raw = os.path.join(OUT, "rf3_mountain_albedo.png")
    image.file_format = "PNG"
    image.save()
    land.data.materials.clear()
    land.data.materials.append(material("RF3R_Terrain", (1, 1, 1), "rf3_mountain_albedo.png"))


def block(name, position, size, mat, angle=0):
    bpy.ops.mesh.primitive_cube_add(size=1, location=position)
    o = bpy.context.object
    o.name = "RF3R_" + name
    o.scale = size
    o.rotation_euler.z = angle
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bevel = o.modifiers.new("WornEdges", "BEVEL")
    bevel.width = min(size) * 0.12
    bevel.segments = 1
    o.data.materials.append(mat)
    return o


def rock(name, x, y, z, size, mat):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=1, location=(x, y, z))
    o = bpy.context.object
    o.name = "RF3R_" + name
    for v in o.data.vertices:
        v.co *= RNG.uniform(0.86, 1.14)
    o.scale = size
    o.rotation_euler.z = RNG.uniform(0, math.tau)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    o.data.materials.append(mat)
    uv = o.data.uv_layers.new(name="RockUV")
    o.data.uv_layers.active = uv
    for p in o.data.polygons:
        axis = max(range(3), key=lambda a: abs(p.normal[a]))
        axes = [a for a in range(3) if a != axis]
        for loop in p.loop_indices:
            v = o.data.vertices[o.data.loops[loop].vertex_index].co
            uv.data[loop].uv = (v[axes[0]] * 0.25, v[axes[1]] * 0.25)


def path(points, stone, width=1.6):
    for a, b in zip(points, points[1:]):
        a, b = Vector(a), Vector(b)
        count = max(2, int((b - a).length / 0.7))
        angle = math.atan2(b.y - a.y, b.x - a.x)
        for i in range(count):
            p = a.lerp(b, (i + 0.5) / count)
            z = height(p.x, p.y) + 0.18
            if abs(p.x - river_x(p.y)) < 2.2:
                z = raw_height(p.x, p.y) + 0.25
            block("TrailStone", (p.x, p.y, z), (0.73, width * RNG.uniform(0.92, 1.06), 0.28), stone, angle + RNG.uniform(-0.035, 0.035))


def pine(x, y, h, bark, leaves):
    z = height(x, y)
    block("Trunks", (x, y, z + h * 0.4), (0.24, 0.25, h * 0.8), bark)
    vs, fs = [], []
    for tier in range(7):
        radius = h * 0.26 * (1 - tier / 8)
        bottom = z + h * (0.25 + tier * 0.09)
        offset = len(vs)
        for j in range(16):
            t = math.tau * j / 16 + tier * 0.31
            r = radius * RNG.uniform(0.75, 1.15)
            vs.append((x + r * math.cos(t), y + r * math.sin(t), bottom + RNG.uniform(-0.15, 0.15)))
        vs.append((x + 0.10 * math.sin(tier), y, bottom + h * 0.30))
        for j in range(16):
            fs.append((offset + j, offset + (j + 1) % 16, offset + 16))
        fs.append(tuple(offset + j for j in reversed(range(16))))
    mesh("RF3R_Foliage", vs, fs, leaves)


def river(mat, foam):
    vs, fs = [], []
    for i in range(321):
        y = -65 + 88 * i / 320
        x = river_x(y)
        z = height(x, y) + 0.22
        for j in range(5):
            vs.append((x + (j - 2) * 0.55, y, z))
    for i in range(320):
        for j in range(4):
            a = i * 5 + j
            fs.append((a, a + 1, a + 6, a + 5))
    mesh("RF3R_River", vs, fs, mat, True)
    for i in range(79):
        y = -64 + i * 1.1
        x = river_x(y)
        z = height(x, y) + 0.28
        vertices = [(x - 0.9, y, z), (x - 0.25, y + 0.12, z), (x + 0.65, y, z),
                    (x + 0.65, y + 0.06, z), (x - 0.25, y + 0.18, z), (x - 0.9, y + 0.06, z)]
        mesh("RF3R_Ripples", vertices, [(0, 1, 2, 3, 4, 5)], foam)


def batch(prefix, name):
    selected = [o for o in bpy.context.scene.objects if o.type == "MESH" and o.name.startswith(prefix)]
    if not selected:
        return
    bpy.ops.object.select_all(action="DESELECT")
    for o in selected:
        o.select_set(True)
    bpy.context.view_layer.objects.active = selected[0]
    bpy.ops.object.convert(target="MESH")
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    bpy.ops.object.join()
    bpy.context.object.name = name


def main():
    bpy.context.preferences.filepaths.save_version = 0
    anchors = {o.name: o.matrix_world.copy() for o in bpy.data.objects if o.name.startswith("RF3_Portal")}
    keep = ("RF3_Portal", "RF3_MoonlessSky", "RF3_StarField", "RF3_Seal", "RF3_GoldenCross")
    for o in list(bpy.data.objects):
        if not o.name.startswith(keep):
            bpy.data.objects.remove(o, do_unlink=True)
    stone = material("RF3R_WeatheredStone", (1, 1, 1), "../stage-select1/rf1_rock_painted.png")
    grass = material("RF3R_MountainMoss", (1, 1, 1), "../stage-select1/rf1_grass_painted.png")
    paving = material("RF3R_OldMasonry", (1, 1, 1), "../stage-select1/rf1_ruin_stone.png")
    bark = material("RF3R_CedarBark", (1, 1, 1), "../stage-select1/rf1_bark.png")
    leaves = material("RF3R_CedarGreen", (0.12, 0.28, 0.22))
    water = material("RF3R_StreamBlue", (0.12, 0.33, 0.42))
    foam = material("RF3R_StreamFoam", (0.36, 0.56, 0.62))
    terrain(stone, grass)
    foothills(stone, grass)
    path(PORTALS[:9], paving)
    path([PORTALS[0], PORTALS[10]], paving)
    path([PORTALS[8], (9, 19, 0), (20, 12, 0), (21, 1, 0), PORTALS[9]], paving, 1.3)
    path([PORTALS[8], (0, 20.5, 7.1)], paving, 2.3)
    # A broad circular sanctuary with weathered columns frames the existing seal.
    for tier in range(3):
        bpy.ops.mesh.primitive_cylinder_add(vertices=48, radius=5.4 - tier * 0.42, depth=0.24, location=(0, 22, 6.55 + tier * 0.24))
        o = bpy.context.object
        o.name = "RF3R_AltarSteps"
        o.data.materials.append(paving)
    for i in range(7):
        t = math.pi * i / 6
        x, y = 4.7 * math.cos(t), 22 + 4.7 * math.sin(t)
        h = RNG.uniform(1.8, 3.3)
        for j in range(int(h / 0.55)):
            block("Ruins", (x, y, 7.05 + j * 0.55), (0.75, 0.8, 0.53), paving, RNG.uniform(-0.04, 0.04))
        block("Ruins", (x, y, 6.9), (1.2, 1.2, 0.35), paving)
    for x, y in [(-13, 0), (-12, 7), (-8, 4), (1, 4), (8, 12), (-11, 17)]:
        z = height(x, y)
        for i in range(RNG.randrange(2, 5)):
            block("Ruins", (x, y, z + 0.3 + i * 0.55), (0.8, 0.9, 0.5), paving, RNG.uniform(-0.08, 0.08))
        block("Ruins", (x + 1.0, y + 0.4, height(x + 1, y + 0.4) + 0.25), (1.5, 0.55, 0.45), paving, 0.4)
    for i, (x, y, h) in enumerate([(-22, 7, 7), (-18, 20, 8), (-12, 29, 8), (10, 30, 9), (20, 24, 8), (25, 12, 6)]):
        rock("Crags", x, y, height(x, y) + 0.2, (4.2, 4.5, h), stone)
        rock("Crags", x + 2, y - 2, height(x + 2, y - 2), (3.5, 3, h * 0.65), stone)
    for x, y in [(-23,-8),(-24,-2),(-22,4),(-18,9),(-18,14),(-14,22),(-8,28),(8,29),(14,25),(23,14),(24,7),(24,-2),(21,-10),(-19,-13),(-8,-15),(2,-14),(-17,18),(20,20)]:
        pine(x, y, RNG.uniform(3.2, 5.7), bark, leaves)
    for i in range(100):
        x, y = RNG.uniform(-25, 25), RNG.uniform(-18, 29)
        if min(math.hypot(x - px, y - py) for px, py, pz in PORTALS) < 2.1:
            continue
        if abs(x - river_x(y)) < 1.6:
            continue
        size = RNG.uniform(0.2, 0.75)
        rock("Scatter", x, y, height(x, y), (size * 1.3, size, size * 0.7), stone)
    river(water, foam)
    vegetation()
    for name, matrix in anchors.items():
        assert bpy.data.objects[name].matrix_world == matrix, name
    batch(("RF3R_TrailStone", "RF3R_Ruins", "RF3R_AltarSteps"), "RF3R_Masonry")
    batch(("RF3R_Crags", "RF3R_Scatter"), "RF3R_Rocks")
    batch("RF3R_Foliage", "RF3R_Forest")
    batch("RF3R_Trunks", "RF3R_TreeTrunks")
    batch("RF3R_Ripples", "RF3R_RiverFoam")
    for filename, category in [("stageSelectMoonMountain.x", "land"), ("stageSelectMoonMountainStars.x", "stars"), ("stageSelectMountainRiver.x", "water"), ("stageSelectMountainPortals.x", "portals")]:
        bpy.ops.object.select_all(action="DESELECT")
        chosen = []
        for o in bpy.context.scene.objects:
            if o.type != "MESH":
                continue
            group = "land"
            if o.name == "RF3_StarField":
                group = "stars"
            elif o.name in ("RF3R_River", "RF3R_RiverFoam"):
                group = "water"
            elif o.name.startswith(("RF3_Portal", "RF3_Seal", "RF3_GoldenCross")):
                group = "portals"
            if group == category:
                o.select_set(True)
                chosen.append(o)
        assert chosen
        bpy.context.view_layer.objects.active = chosen[0]
        result = bpy.ops.export_scene.directx_x(filepath=os.path.join(OUT, filename), use_selection=True,
            axis_forward="Z", axis_up="Y", export_animation=False, export_armature=False,
            export_weights=False, use_mesh_modifiers=True, use_original_material_data=False, export_format="TEXT_X")
        assert "FINISHED" in result
    scene = bpy.context.scene
    bpy.ops.object.camera_add(location=(0, -38, 23))
    scene.camera = bpy.context.object
    scene.camera.name = "RF3R_PreviewCamera"
    scene.camera.rotation_euler = (Vector((0, 6, 4.2)) - scene.camera.location).to_track_quat("-Z", "Y").to_euler()
    scene.camera.data.lens = 18
    scene.camera.data.clip_end = 500
    bpy.ops.object.light_add(type="AREA", location=(-12, -12, 30))
    bpy.context.object.data.energy = 4500
    bpy.context.object.data.shape = "DISK"
    bpy.context.object.data.size = 25
    bpy.ops.object.light_add(type="AREA", location=(10, 18, 20))
    bpy.context.object.data.energy = 2500
    bpy.context.object.data.color = (0.6, 0.72, 1)
    bpy.context.object.data.size = 18
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 16
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 1600
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.render.filepath = os.path.join(OUT, "world3_preview.png")
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT, "world3.blend"))
    print("WORLD3_REBUILD: preserved", len(anchors), "portal components; water exported separately", flush=True)
    bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    main()
