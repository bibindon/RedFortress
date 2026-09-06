"""Rebuild world4 as an enclosed night forest; Blender Z up, meters.

Run Blender on world4.blend with --background --python this-file.
Only the official DirectX exporter writes the game models.
"""
import csv
import math
import os
import random

import bpy
import bmesh
import numpy as np
from mathutils import Vector

OUT = os.path.dirname(bpy.data.filepath)
RNG = random.Random(4406)
PORTALS = []
with open(os.path.join(OUT, "Interactables.csv"), encoding="utf-8-sig", newline="") as stream:
    for row in csv.DictReader(stream):
        PORTALS.append((float(row["PosX"]), float(row["PosZ"]), float(row["PosY"])))
assert len(PORTALS) == 10
ROUTES = [PORTALS[:9], [PORTALS[4], PORTALS[9]]]
BUFFERS = {}


def material(name, color, texture=None):
    mat = bpy.data.materials.new(name)
    mat.diffuse_color = (*color, 1)
    mat.use_nodes = True
    mat.specular_intensity = 0
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1)
    bsdf.inputs["Roughness"].default_value = 0.95
    bsdf.inputs["Specular IOR Level"].default_value = 0
    if texture:
        tex = mat.node_tree.nodes.new("ShaderNodeTexImage")
        tex.image = bpy.data.images.load(os.path.join(OUT, texture), check_existing=True)
        tex.image.filepath = "//" + texture
        mat.node_tree.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
    return mat


def mesh(name, vertices, faces, mat, smooth=False):
    data = bpy.data.meshes.new(name)
    data.from_pydata(vertices, [], faces)
    data.update()
    bm = bmesh.new()
    bm.from_mesh(data)
    bmesh.ops.remove_doubles(bm, verts=list(bm.verts), dist=0.0001)
    bm.to_mesh(data)
    bm.free()
    data.update()
    data.materials.append(mat)
    uv = data.uv_layers.new(name="UVMap")
    for poly in data.polygons:
        poly.use_smooth = smooth
        axis = max(range(3), key=lambda a: abs(poly.normal[a]))
        axes = [a for a in range(3) if a != axis]
        for loop in poly.loop_indices:
            v = data.vertices[data.loops[loop].vertex_index].co
            uv.data[loop].uv = (v[axes[0]] * 0.28, v[axes[1]] * 0.28)
    obj = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(obj)
    return obj


def face(points, mat, double=False):
    vertices, faces = BUFFERS.setdefault(mat.name, ([], []))
    offset = len(vertices)
    vertices.extend(points)
    faces.append(tuple(range(offset, offset + len(points))))
    if double:
        offset = len(vertices)
        vertices.extend([Vector(p) - Vector((0, 0, 0.002)) for p in points])
        faces.append(tuple(reversed(range(offset, offset + len(points)))))


XY = np.array([(x, y) for x, y, z in PORTALS])
K = np.exp(-np.sum((XY[:, None, :] - XY[None, :, :]) ** 2, axis=2) / 100)


def base(x, y):
    return 0.35 + 2.5 / (1 + math.exp(-max(-100, min(100, (y - 7) * 0.18)))) + 0.18 * math.sin(x * 0.24) * math.cos(y * 0.19)


WEIGHTS = np.linalg.solve(K, np.array([z - 0.37 - base(x, y) for x, y, z in PORTALS]))


def height(x, y):
    d = XY - (x, y)
    rise = max(0, y - 20)
    hill = 0.014 * rise * rise / (1 + rise / 90)
    hill *= 1 + 0.16 * math.sin(x * 0.07) + 0.08 * math.cos(y * 0.13)
    return base(x, y) + float(np.exp(-np.sum(d * d, axis=1) / 100) @ WEIGHTS) + hill


def grass_instances():
    rng = random.Random(4420)
    rows = []
    # One quarter of the previous 31,469 instances, including outer-screen slopes.
    while len(rows) < 7867:
        x, y = rng.uniform(-55, 55), rng.uniform(-34, 68)
        if len(rows) >= 6300:
            x, y = rng.uniform(-170,170), rng.uniform(-40,170)
            if abs(x) < 46 and -30 < y < 65:
                continue
        z = height(x,y)
        depth = (y+38)*0.92472 + (20-z)*0.38064
        screen_y = (y+38)*0.38064 + (z-20)*0.92472
        if depth < 1 or abs(x/depth) > 1.12 or abs(screen_y/depth) > 0.66:
            continue
        if route_distance(x, y) < 2.1 or math.hypot(x, y - 15.5) < 4.0:
            continue
        density = 0.74 + 0.20 * math.sin(x * 0.35) * math.cos(y * 0.27)
        if rng.random() > density:
            continue
        # Match the triangulated two-meter terrain grid, avoiding buried blades.
        x0, y0 = math.floor(x / 2) * 2, math.floor(y / 2) * 2
        u, v = (x - x0) / 2, (y - y0) / 2
        a, b, c, d = height(x0,y0), height(x0+2,y0), height(x0+2,y0+2), height(x0,y0+2)
        z = a + u * (b-a) + v * (c-b)
        if v > u:
            z = a + u * (c-d) + v * (d-a)
        rows.append((round(x,3),round(z+0.012,3),round(y,3),round(rng.uniform(0,360),2),round(rng.uniform(1.70,3.10),3)))
    with open(os.path.join(OUT,"forest_grass.csv"),"w",encoding="utf-8",newline="") as stream:
        writer = csv.writer(stream,lineterminator="\r\n")
        writer.writerow(("sway","wave"))
        writer.writerow(("AutoHide","n"))
        writer.writerows(rows)
    print("FOREST_GRASS_INSTANCES",len(rows),flush=True)


def route_distance(x, y):
    dist = min(math.hypot(x - px, y - py) - 0.5 for px, py, pz in PORTALS)
    for route in ROUTES:
        for a, b in zip(route, route[1:]):
            dx, dy = b[0] - a[0], b[1] - a[1]
            t = max(0, min(1, ((x - a[0]) * dx + (y - a[1]) * dy) / (dx * dx + dy * dy)))
            dist = min(dist, math.hypot(x - a[0] - t * dx, y - a[1] - t * dy))
    return dist


def tube(a, b, r1, r2, mat, sides=9):
    a, b = Vector(a), Vector(b)
    axis = (b - a).normalized()
    side = axis.cross(Vector((0, 1, 0)))
    if side.length < 0.01:
        side = axis.cross(Vector((1, 0, 0)))
    side.normalize()
    other = axis.cross(side).normalized()
    rings = []
    for center, radius in ((a, r1), (b, r2)):
        rings.append([center + radius * (side * math.cos(i * math.tau / sides) + other * math.sin(i * math.tau / sides)) for i in range(sides)])
    for i in range(sides):
        j = (i + 1) % sides
        face((rings[0][i], rings[0][j], rings[1][j], rings[1][i]), mat)
    face(tuple(reversed(rings[0])), mat)
    face(rings[1], mat)


def mound(center, scale, mat):
    # Rounded irregular crowns and mossy boulders, with shared material batches.
    cx, cy, cz = center
    rings = []
    for j in range(9):
        phi = math.pi * j / 8
        row = []
        for i in range(14):
            t = math.tau * i / 14
            noise = RNG.uniform(0.90, 1.10)
            row.append((cx + scale[0] * math.sin(phi) * math.cos(t) * noise,
                        cy + scale[1] * math.sin(phi) * math.sin(t) * noise,
                        cz + scale[2] * math.cos(phi) * noise))
        rings.append(row)
    for j in range(8):
        for i in range(14):
            k = (i + 1) % 14
            face((rings[j][i], rings[j+1][i], rings[j+1][k], rings[j][k]), mat)


def tree(x, y, h, bark, leaves, ground_z=None):
    if ground_z is None:
        z = height(x, y)
    else:
        z = ground_z
    lean = Vector((RNG.uniform(-1, 1), RNG.uniform(-0.5, 0.5), 0))
    root = Vector((x, y, z))
    mid = root + lean + Vector((0, 0, h * 0.58))
    top = root + lean * 1.5 + Vector((0, 0, h * 0.87))
    tube(root, mid, h * 0.065, h * 0.035, bark)
    tube(mid, top, h * 0.035, 0.12, bark)
    for i in range(6):
        angle = i * math.tau / 6 + x
        direction = Vector((math.cos(angle), math.sin(angle), 0))
        end = root + direction * h * 0.19
        if ground_z is None:
            end.z = height(end.x, end.y) + 0.06
        else:
            end.z = ground_z + 0.06
        tube(root + Vector((0, 0, 0.6)), end, h * 0.038, 0.05, bark)
        branch = root + direction * h * RNG.uniform(0.18, 0.26) + Vector((0, 0, h * RNG.uniform(0.69, 0.83)))
        tube(mid, branch, h * 0.023, 0.08, bark)
        crown = branch + Vector((0, 0, h * 0.10))
        mound(crown, (h * 0.22, h * 0.20, h * 0.15), leaves[i % len(leaves)])
    mound(top, (h * 0.26, h * 0.23, h * 0.17), leaves[1])


def write_instance_csv(filename, rows, sway="off"):
    with open(os.path.join(OUT, filename), "w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream, lineterminator="\r\n")
        writer.writerow(("sway", sway))
        writer.writerow(("AutoHide", "n"))
        writer.writerows(rows)


def tree_instances():
    forest_rng = random.Random(4431)
    positions = []
    rows = [[], [], []]
    count = 0
    for unused in range(1600):
        x, y = forest_rng.uniform(-76, 76), forest_rng.uniform(-20, 78)
        if y < 27 and abs(x) < 29:
            continue
        if any(math.hypot(x - px, y - py) < 6.3 for px, py in positions):
            continue
        positions.append((x, y))
        h = forest_rng.uniform(13, 23)
        if y < 27:
            h = forest_rng.uniform(10, 16)
        variant = count % 3
        rows[variant].append((round(x, 3), round(height(x, y), 3), round(y, 3),
                              round(forest_rng.uniform(0, 360), 2), round(h / 10, 3)))
        count += 1
        if count >= 125:
            break
    foreground = [(-17, 7, 9), (-15, 19, 12), (14, 18, 12), (21, 4, 9), (-24, -5, 8)]
    for x, y, h in foreground:
        variant = count % 3
        rows[variant].append((x, round(height(x, y), 3), y,
                              round(forest_rng.uniform(0, 360), 2), round(h / 10, 3)))
        count += 1
    for variant in range(3):
        write_instance_csv("forest_tree%d.csv" % variant, rows[variant])
    print("FOREST_TREE_INSTANCES", count, flush=True)
    return rows


def fern_instances():
    rng = random.Random(4462)
    rows = []
    while len(rows) < 1500:
        x, y = rng.uniform(-55, 55), rng.uniform(-32, 70)
        if route_distance(x, y) < 1.7 or math.hypot(x, y - 15.5) < 3.5:
            continue
        rows.append((round(x, 3), round(height(x, y) + 0.025, 3), round(y, 3),
                     round(rng.uniform(0, 360), 2), round(rng.uniform(0.72, 1.38), 3)))
    write_instance_csv("forest_fern.csv", rows, "wave")
    print("FOREST_FERN_INSTANCES", len(rows), flush=True)
    return rows


def fern_template(mat):
    root = Vector((0, 0, 0))
    for frond in range(7):
        angle = frond * math.tau / 7
        direction = Vector((math.cos(angle), math.sin(angle), 0))
        side = Vector((-direction.y, direction.x, 0))
        for step in range(1, 7):
            u = step / 7
            center = root + direction * u * 0.95 + Vector((0, 0, 0.6 * math.sin(u * 2.6)))
            for sign in (-1, 1):
                width = 0.20 * (1 - u) + 0.02
                face((center - direction * 0.06,
                      center + side * sign * width + direction * 0.10,
                      center + direction * 0.10), mat, True)


def export_x(filename, chosen):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in chosen:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = chosen[0]
    result = bpy.ops.export_scene.directx_x(filepath=os.path.join(OUT, filename), use_selection=True,
        axis_forward="Z", axis_up="Y", export_animation=False, export_armature=False,
        export_weights=False, use_mesh_modifiers=True, use_original_material_data=False, export_format="TEXT_X")
    assert "FINISHED" in result


def join_meshes(name, objects):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    bpy.ops.object.join()
    joined = bpy.context.object
    joined.name = name
    joined.data.name = name
    return joined


def export_instancing_models(bark, leaves, fern_mat):
    templates = []
    for variant in range(3):
        BUFFERS.clear()
        RNG.seed(4500 + variant)
        tree(0, 0, 10, bark, leaves, 0)
        objects = []
        for name, (vertices, faces) in BUFFERS.items():
            objects.append(mesh("RF4I_Tree%d_%s" % (variant, name), vertices, faces,
                                bpy.data.materials[name], name.startswith("RF4R_Canopy")))
        joined = join_meshes("RF4I_Tree%d" % variant, objects)
        export_x("stageSelectForestTree%d.x" % variant, [joined])
        templates.append(joined)
    BUFFERS.clear()
    fern_template(fern_mat)
    fern_objects = []
    for name, (vertices, faces) in BUFFERS.items():
        fern_objects.append(mesh("RF4I_Fern", vertices, faces, bpy.data.materials[name], True))
    fern = join_meshes("RF4I_Fern", fern_objects)
    export_x("stageSelectForestFern.x", [fern])
    templates.append(fern)
    BUFFERS.clear()
    return templates


def add_preview_instances(templates, tree_rows, fern_rows):
    instance_rows = list(tree_rows)
    instance_rows.append(fern_rows)
    for template, rows in zip(templates, instance_rows):
        for row in rows:
            preview = template.copy()
            preview.data = template.data
            bpy.context.collection.objects.link(preview)
            preview.location = (row[0], row[2], row[1])
            preview.rotation_euler[2] = math.radians(row[3])
            preview.scale = (row[4], row[4], row[4])
        template.hide_render = True
        template.hide_set(True)


def main():
    bpy.context.preferences.filepaths.save_version = 0
    anchors = {o.name: o.matrix_world.copy() for o in bpy.data.objects if o.name.startswith("RF4_Portal")}
    assert len(anchors) == 30, len(anchors)
    for obj in list(bpy.data.objects):
        if obj.name not in anchors:
            bpy.data.objects.remove(obj, do_unlink=True)
    ground = material("RF4R_MossFloor", (0.35, 0.50, 0.38), "../stage-select1/rf1_grass_bright.png")
    trail = material("RF4R_EarthTrail", (0.36, 0.30, 0.23), "../stage-select2/stageSelectCavePath.png")
    bark = material("RF4R_RidgedBark", (0.32, 0.27, 0.23), "../cubeWood/wood.png")
    stone = material("RF4R_MossRock", (0.32, 0.40, 0.35), "../stage-select1/rf1_rock_painted.png")
    leaves = [material("RF4R_Canopy%d" % i, color) for i, color in enumerate(((0.09,0.23,0.17),(0.16,0.32,0.22),(0.23,0.37,0.27)))]
    green = material("RF4R_Grasses", (0.25, 0.42, 0.29))
    tips = material("RF4R_Ferns", (0.36, 0.53, 0.34))
    white = material("RF4R_Flowers", (0.78, 0.86, 0.84))
    litter = material("RF4R_LeafLitter", (0.38, 0.32, 0.20))
    vertices, faces = [], []
    for j in range(121):
        y = -60 + j * 2
        for i in range(181):
            x = -180 + i * 2
            vertices.append((x, y, height(x, y)))
    for j in range(120):
        for i in range(180):
            a = j * 181 + i
            faces.append((a, a+1, a+182, a+181))
    mesh("RF4R_ContinuousForestFloor", vertices, faces, ground, True)
    for route in ROUTES:
        for a, b in zip(route, route[1:]):
            a, b = Vector(a), Vector(b)
            delta = b-a
            side = Vector((-delta.y, delta.x, 0)).normalized()
            rows = []
            count = int(delta.length * 3)
            for i in range(count+1):
                t = i/count
                p = a.lerp(b, t) + side * math.sin(math.pi * t) * 0.22
                width = 0.95 + 0.09 * math.sin(t * 12)
                row = []
                for sign in (-1, 1):
                    q = p + side * width * sign
                    q.z = height(q.x, q.y) + 0.045
                    row.append(q)
                rows.append(row)
            for i in range(count):
                face((rows[i][0], rows[i+1][0], rows[i+1][1], rows[i][1]), trail)
    # Concentric clearing around the final portal.
    center = Vector((0, 15.5, height(0, 15.5)+0.05))
    for i in range(64):
        points = [center]
        for k in (i, i+1):
            t = k * math.tau/64
            x, y = 3.4*math.cos(t), 15.5+3.4*math.sin(t)
            points.append((x,y,height(x,y)+0.05))
        face(points, trail)
    # Tree and fern geometry is exported once and placed with MeshInstancing2.
    tree_rows = tree_instances()
    fern_rows = fern_instances()
    for i in range(115):
        x,y = RNG.uniform(-30,30), RNG.uniform(-24,29)
        if route_distance(x,y) < 2.0:
            continue
        s = RNG.uniform(0.3,1.0)
        mound((x,y,height(x,y)+s*0.25),(s*1.5,s,s*0.65),stone)
        mound((x,y,height(x,y)+s*0.65),(s*1.25,s*0.8,s*0.22),leaves[1])
    for x,y,s in [(-18,-18,1.8),(15,-18,2.1),(-12,0,1.7),(18,9,2.0),(-11,19,1.9),(9,23,2.3)]:
        z = height(x,y)
        mound((x,y,z+s*0.6),(s*1.3,s,s*1.1),stone)
        mound((x-0.25,y,z+s*1.45),(s*0.9,s*0.72,s*0.3),leaves[1])
        mound((x+s,y+0.5,height(x+s,y+0.5)+s*0.25),(s*0.6,s*0.5,s*0.45),stone)
    for x,y,angle in [(-10,-17,0.4),(8,-16,-0.2),(-16,3,1),(17,8,-0.4),(-7,20,0.3)]:
        start = Vector((x,y,height(x,y)+0.35))
        end = start + Vector((math.cos(angle)*3.7,math.sin(angle)*3.7,0.1))
        tube(start,end,0.42,0.33,bark,12)
        tube(start,start+Vector((0.1,0.4,1.1)),0.16,0.06,bark)
    grass_instances()
    for name,(vs,fs) in BUFFERS.items():
        mesh(name,vs,fs,bpy.data.materials[name], name.startswith("RF4R_Canopy"))
    land_objects = []
    for obj in bpy.context.scene.objects:
        if obj.type == "MESH" and not obj.name.startswith("RF4_Portal"):
            land_objects.append(obj)
    for name,matrix in anchors.items():
        assert bpy.data.objects[name].matrix_world == matrix, name
    export_x("stageSelectDawnIsland.x", land_objects)
    portal_objects = []
    for obj in bpy.context.scene.objects:
        if obj.type == "MESH" and obj.name.startswith("RF4_Portal"):
            portal_objects.append(obj)
    export_x("stageSelectForestPortals.x", portal_objects)
    BUFFERS.clear()
    templates = export_instancing_models(bark, leaves, tips)
    add_preview_instances(templates, tree_rows, fern_rows)
    scene = bpy.context.scene
    bpy.ops.object.camera_add(location=(0,-38,20))
    scene.camera = bpy.context.object
    scene.camera.rotation_euler = (Vector((0,5,2.3))-scene.camera.location).to_track_quat("-Z","Y").to_euler()
    scene.camera.data.lens = 18
    scene.camera.data.clip_end = 500
    for pos,energy,color,size in [((-18,-15,32),6500,(0.68,0.80,1),28),((15,22,36),8500,(0.62,0.80,1),24)]:
        bpy.ops.object.light_add(type="AREA",location=pos)
        light = bpy.context.object
        light.data.energy = energy
        light.data.color = color
        light.data.shape = "DISK"
        light.data.size = size
    scene.world.color = (0.06,0.08,0.10)
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 16
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 1600
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.render.filepath = os.path.join(OUT,"world4_forest_preview.png")
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT,"world4.blend"))
    tree_count = sum(len(rows) for rows in tree_rows)
    print("WORLD4_FOREST: preserved", len(anchors), "portal parts; trees", tree_count,
          "ferns", len(fern_rows), flush=True)
    bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    main()
