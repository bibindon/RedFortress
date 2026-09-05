"""Sculpt the cave enclosure in world2.blend; Blender Z up, meters.

Preserves the stage route, portal objects, materials and runtime lighting.
Exports with the official Blender DirectX X add-on.
"""
import math
import os
import random

import bpy
from mathutils import Vector

OUTPUT = os.path.dirname(bpy.data.filepath)
RNG = random.Random(2207)
GATE = 2.22
WIDTH = 0.155


def wall_point(t, z):
    radius = 1.0 + 0.035 * math.sin(7 * t) + 0.022 * math.cos(13 * t)
    relief = (0.65 * math.sin(t * 24 + z * 0.28)
              + 0.32 * math.sin(t * 49 - z * 0.62)
              + 0.23 * math.cos(z * 1.9 + t * 8))
    # Broad folds and eroded shelves, rather than independent random spikes.
    x = (25 * radius + relief) * math.cos(t)
    y = -12 + (36 * radius + relief) * math.sin(t)
    return Vector((x, y, z))


def lip_height(t):
    u = (t - GATE) / WIDTH
    if abs(u) >= 1:
        return -1.2
    return -1.2 + 7.7 * math.sqrt(max(0, 1 - u * u)) * (1 + 0.04 * math.sin(t * 55))


def mesh(name, vertices, faces, material, smooth=True):
    data = bpy.data.meshes.new(name)
    data.from_pydata(vertices, [], faces)
    data.update()
    obj = bpy.data.objects.new(name, data)
    bpy.context.collection.objects.link(obj)
    data.materials.append(material)
    uv = data.uv_layers.new(name="UVMap")
    for poly in data.polygons:
        poly.use_smooth = smooth
        axis = max(range(3), key=lambda a: abs(poly.normal[a]))
        components = [a for a in range(3) if a != axis]
        for loop_index in poly.loop_indices:
            p = data.vertices[data.loops[loop_index].vertex_index].co
            uv.data[loop_index].uv = (p[components[0]] * 0.18, p[components[1]] * 0.18)
    return obj


def enclosure(material, floor_material):
    # Include the exact opening edges in the wall grid, leaving a real archway.
    angles = sorted(set([math.pi * i / 160 for i in range(161)]
                        + [GATE - WIDTH + 2 * WIDTH * i / 24 for i in range(25)]))
    verts, faces = [], []
    rows = 32
    for t in angles:
        bottom = lip_height(t)
        top = 46 + 2.2 * math.sin(t * 6)
        for j in range(rows + 1):
            z = bottom + (top - bottom) * j / rows
            verts.append(wall_point(t, z))
    for i in range(len(angles) - 1):
        for j in range(rows):
            a = i * (rows + 1) + j
            b = a + rows + 1
            # Winding points into the chamber.
            faces.append((a, a + 1, b + 1, b))
    obj = mesh("RF2_Sculpt_Enclosure", verts, faces, material)
    solid = obj.modifiers.new("RockMassThickness", "SOLIDIFY")
    solid.thickness = 3.5
    solid.offset = -1
    # Low talus skirt connects the enlarged chamber to the existing floor.
    verts, faces = [], []
    for t in angles:
        p = wall_point(t, -1.1)
        n = Vector((math.cos(t), math.sin(t), 0))
        for j in range(4):
            q = p - n * (j * 2.2)
            q.z = -1.1 - 0.12 * j + 0.10 * math.sin(t * 15 + j)
            verts.append(q)
    for i in range(len(angles) - 1):
        for j in range(3):
            a = i * 4 + j
            faces.append((a, a + 4, a + 5, a + 1))
    mesh("RF2_Sculpt_TalusFloor", verts, faces, floor_material)


def entrance(material, floor_material):
    # The mouth follows the actual wall opening; nine meters of curved passage
    # lead out into the rock, preventing the thin punctured-sheet silhouette.
    outline = []
    for i in range(25):
        t = GATE - WIDTH + 2 * WIDTH * i / 24
        outline.append(wall_point(t, lip_height(t)))
    outward = Vector((math.cos(GATE), math.sin(GATE), 0))
    side = Vector((-outward.y, outward.x, 0))
    verts, faces = [], []
    for row in range(13):
        distance = row * 0.8
        for i, point in enumerate(outline):
            p = point + outward * distance + side * (0.045 * distance ** 2)
            p.z += 0.16 * math.sin(row * 0.7 + i * 0.8) * min(1, row)
            verts.append(p)
    for row in range(12):
        for i in range(24):
            a = row * 25 + i
            faces.append((a, a + 1, a + 26, a + 25))
    obj = mesh("RF2_Sculpt_EntranceTunnel", verts, faces, material)
    mod = obj.modifiers.new("TunnelRockThickness", "SOLIDIFY")
    mod.thickness = 1.5
    mod.offset = -1
    floor, faces = [], []
    for row in range(13):
        for i in (0, 24):
            p = Vector(verts[row * 25 + i])
            p.z = -0.65 + row * 0.025
            floor.append(p)
    for row in range(12):
        a = row * 2
        faces.append((a, a + 2, a + 3, a + 1))
    mesh("RF2_Sculpt_EntranceFloor", floor, faces, floor_material)
    # Large embedded stones give the mouth a broken, substantial rock rim.
    for index in range(13):
        t = GATE - WIDTH + 2 * WIDTH * index / 12
        p = wall_point(t, lip_height(t))
        p.z += 0.65
        rock("EntranceRim", index, p, (1.2, 1.25, 1.0), material)


def rock(group, index, position, scale, material):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=1, location=position)
    obj = bpy.context.object
    obj.name = "RF2_Sculpt_" + group + "_" + str(index)
    for v in obj.data.vertices:
        v.co *= RNG.uniform(0.89, 1.11)
    obj.scale = scale
    obj.rotation_euler = (RNG.uniform(-0.2, 0.2), RNG.uniform(-0.2, 0.2), RNG.uniform(0, math.tau))
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    obj.data.materials.append(material)
    uv = obj.data.uv_layers.new(name="RockUV")
    for polygon in obj.data.polygons:
        axis = max(range(3), key=lambda a: abs(polygon.normal[a]))
        axes = [a for a in range(3) if a != axis]
        for loop in polygon.loop_indices:
            p = obj.data.vertices[obj.data.loops[loop].vertex_index].co
            uv.data[loop].uv = (p[axes[0]] * 0.18, p[axes[1]] * 0.18)


def main():
    bpy.context.preferences.filepaths.save_version = 0
    anchors = {o.name: o.matrix_world.copy() for o in bpy.data.objects if o.name.startswith("RF2_Portal")}
    for obj in list(bpy.data.objects):
        if obj.name.startswith(("RF2_NaturalCave_", "RF2_CaveEntrance_", "RF2_Stalactite_", "RF2_WallButtress_", "RF2_Sculpt_")):
            bpy.data.objects.remove(obj, do_unlink=True)
    wall = bpy.data.materials["stageSelectCaveWall"]
    enclosure(wall, wall)
    entrance(wall, wall)
    for i in range(48):
        t = 0.12 + (math.pi - 0.24) * i / 47
        if abs(t - GATE) < WIDTH + 0.11:
            continue
        p = wall_point(t, -0.3)
        p -= Vector((math.cos(t), math.sin(t), 0)) * RNG.uniform(0.8, 2.1)
        size = RNG.uniform(0.65, 1.65)
        rock("FootRocks", i, p, (size * 1.3, size, size * 0.85), wall)
    # Batch small rock details to keep the number of draw submissions modest.
    bpy.ops.object.select_all(action="DESELECT")
    rocks = [o for o in bpy.data.objects if o.name.startswith(("RF2_Sculpt_FootRocks_", "RF2_Sculpt_EntranceRim_"))]
    for obj in rocks:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = rocks[0]
    bpy.ops.object.join()
    bpy.context.object.name = "RF2_Sculpt_RockDetails"
    for name, matrix in anchors.items():
        assert bpy.data.objects[name].matrix_world == matrix, name
    bpy.ops.object.select_all(action="DESELECT")
    selected = [o for o in bpy.context.scene.objects if o.type == "MESH"]
    for obj in selected:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = selected[0]
    result = bpy.ops.export_scene.directx_x(filepath=os.path.join(OUTPUT, "stageSelectCave.x"),
        use_selection=True, axis_forward="Z", axis_up="Y", export_animation=False,
        export_armature=False, export_weights=False, use_mesh_modifiers=True,
        use_original_material_data=False, export_format="TEXT_X")
    assert "FINISHED" in result
    print("CAVE_REBUILD: preserved", len(anchors), "portal components; exported", len(selected), "meshes", flush=True)
    scene = bpy.context.scene
    scene.camera = bpy.data.objects["RF2_PREVIEW_Camera"]
    scene.camera.location = (0, -32, 15)
    scene.camera.rotation_euler = (Vector((0, 5.5, 0.8)) - scene.camera.location).to_track_quat("-Z", "Y").to_euler()
    scene.camera.data.lens = 18
    scene.camera.data.sensor_width = 36
    scene.camera.data.sensor_fit = "HORIZONTAL"
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 16
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 1600
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.render.filepath = os.path.join(OUTPUT, "stageSelectCave_preview.png")
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUTPUT, "world2.blend"))
    bpy.ops.render.render(write_still=True)


if __name__ == "__main__":
    main()
