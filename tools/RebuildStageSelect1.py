"""Rebuild World 1 from its portal layout; run in Blender with world1.blend open.

All geometry is authored in meters, Blender Z up, X/Y matching game X/Z.
The official DirectX add-on owns serialization and axis conversion.
Existing shared textures and portal navigation are deliberately preserved.
"""

import csv
import math
import os
import random

import bpy
import numpy as np
from mathutils import Vector


OUTPUT = os.path.dirname(bpy.data.filepath)
RNG = random.Random(1701)
SEA_Z = -2.35
SEA_NAMES = {
    "StageSelect_DeepSea.001", "StageSelect_ShallowWaterRing.001",
    "RF1_Portal_00_Ring", "RF1_Portal_09_Ring",
}
PORTALS = []
with open(os.path.join(OUTPUT, "Interactables.csv"), encoding="utf-8-sig", newline="") as stream:
    for row in csv.DictReader(stream):
        if row["Type"] == "StagePortal":
            PORTALS.append((float(row["PosX"]), float(row["PosZ"]), float(row["PosY"])))
if len(PORTALS) != 10:
    raise RuntimeError("Expected the ten World 1 portals.")


def mesh_object(name, vertices, faces, materials, smooth=True, tile=5.0):
    mesh = bpy.data.meshes.new(name + "Mesh")
    mesh.from_pydata(vertices, [], faces)
    mesh.update()
    for material in materials:
        mesh.materials.append(material)
    uv = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        polygon.use_smooth = smooth
        for loop in polygon.loop_indices:
            point = mesh.vertices[mesh.loops[loop].vertex_index].co
            uv.data[loop].uv = (point.x / tile, point.y / tile)
    obj = bpy.data.objects.new(name, mesh)
    bpy.context.scene.collection.objects.link(obj)
    return obj


def texture(name, color, amplitude=0.05, water=False):
    size = 512
    yy, xx = np.mgrid[0:size, 0:size] / float(size)
    noise = np.zeros((size, size))
    for frequency, weight in ((4, 0.45), (12, 0.25), (32, 0.17), (80, 0.08), (160, 0.05)):
        lattice = np.array([RNG.uniform(-1, 1) for unused in range(frequency * frequency)]).reshape(frequency, frequency)
        u = xx * frequency
        v = yy * frequency
        ix = u.astype(int)
        iy = v.astype(int)
        tx = u - ix
        ty = v - iy
        tx = tx * tx * (3 - 2 * tx)
        ty = ty * ty * (3 - 2 * ty)
        noise += weight * ((1 - tx) * (1 - ty) * lattice[iy % frequency, ix % frequency]
            + tx * (1 - ty) * lattice[iy % frequency, (ix + 1) % frequency]
            + (1 - tx) * ty * lattice[(iy + 1) % frequency, ix % frequency]
            + tx * ty * lattice[(iy + 1) % frequency, (ix + 1) % frequency])
    if water:
        noise = 0.55 * np.sin(math.tau * (yy * 12 + 0.16 * np.sin(xx * math.tau * 3)))
        noise += 0.20 * np.sin(math.tau * (xx * 7 + yy * 17))
    rgba = np.ones((size, size, 4), dtype=np.float32)
    for channel in range(3):
        rgba[:, :, channel] = np.clip(color[channel] * (1.0 + amplitude * noise), 0, 1)
    image = bpy.data.images.new(name, width=size, height=size, alpha=False)
    image.pixels.foreach_set(rgba.ravel())
    image.filepath_raw = os.path.join(OUTPUT, name + ".png")
    image.file_format = "PNG"
    image.save()
    image.filepath = "//" + name + ".png"
    return image


def material(name, color, image=None, specular=0.015):
    mat = bpy.data.materials.new(name)
    mat.use_nodes = True
    mat.diffuse_color = (*color, 1)
    mat.roughness = 0.9
    mat.specular_intensity = specular
    mat["_x_face_color"] = (*color, 1)
    mat["_x_power"] = 500.0
    mat["_x_specular"] = (specular, specular, specular)
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*color, 1)
    bsdf.inputs["Roughness"].default_value = 0.9
    bsdf.inputs["Specular IOR Level"].default_value = specular
    if image is not None:
        node = mat.node_tree.nodes.new("ShaderNodeTexImage")
        node.image = image
        mat.node_tree.links.new(node.outputs["Color"], bsdf.inputs["Base Color"])
        mat["_x_texture_filename"] = os.path.basename(image.filepath)
    return mat


def coast(angle):
    irregular = 1 + 0.045 * math.sin(angle * 5 + 0.3) + 0.03 * math.cos(angle * 3 - 0.4)
    entry_angle = math.atan2(math.sin(angle + 2.418), math.cos(angle + 2.418))
    irregular += 0.30 * math.exp(-(entry_angle / 0.32) ** 2)
    return (21.5 * math.cos(angle) * irregular, -1.8 + 15.7 * math.sin(angle) * irregular)


def kernel(distances):
    return np.sqrt(distances + 6.25)


def fit_height():
    points = []
    for index in range(len(PORTALS) - 1):
        start = np.array(PORTALS[index])
        end = np.array(PORTALS[index + 1])
        count = max(2, int(np.linalg.norm(end[:2] - start[:2]) / 1.0))
        for step in range(count):
            points.append(start + (end - start) * (step / count))
    points.append(np.array(PORTALS[-1]))
    # A rounded summit and a low continuous shoreline constrain the interpolant.
    points.extend([np.array((0, 3, 6.6)), np.array((-5, 4, 4.8)), np.array((5, 4, 4.2))])
    for index in range(48):
        x, y = coast(index * math.tau / 48)
        points.append(np.array((x * 0.83, -1.8 + (y + 1.8) * 0.83, -0.25)))
    samples = np.array(points)
    delta = samples[:, None, :2] - samples[None, :, :2]
    matrix = kernel(np.sum(delta * delta, axis=2))
    matrix += np.eye(len(samples)) * 0.000001
    return samples[:, :2], np.linalg.solve(matrix, samples[:, 2])


SAMPLES, WEIGHTS = fit_height()


def height(x, y):
    delta = SAMPLES - (x, y)
    return float(kernel(np.sum(delta * delta, axis=1)) @ WEIGHTS)


def build_land(grass, sand, rock):
    segments = 160
    rings = 90
    vertices = [(0, -1.8, height(0, -1.8))]
    for ring in range(1, rings + 1):
        radius = ring / rings
        for segment in range(segments):
            x, y = coast(segment * math.tau / segments)
            x *= radius
            y = -1.8 + (y + 1.8) * radius
            z = height(x, y)
            if radius > 0.83:
                blend = (radius - 0.83) / 0.17
                z = -0.25 - 2.55 * blend
            vertices.append((x, y, z))
    faces = []
    for segment in range(segments):
        faces.append((0, 1 + segment, 1 + (segment + 1) % segments))
    for ring in range(rings - 1):
        start = 1 + ring * segments
        for segment in range(segments):
            next_segment = (segment + 1) % segments
            faces.append((start + segment, start + segments + segment,
                          start + segments + next_segment, start + next_segment))
    land = mesh_object("RF1_SculptedIsland", vertices, faces, [grass, sand, rock])
    size = 1024
    yy, xx = np.mgrid[0:size, 0:size] / float(size - 1)
    world_x = (xx - 0.5) * 48
    world_y = (yy - 0.5) * 36 - 1.8
    heights = np.zeros((size, size))
    for start in range(0, size, 16):
        locations = np.stack((world_x[start:start + 16], world_y[start:start + 16]), axis=-1)
        delta = locations[:, :, None, :] - SAMPLES[None, None, :, :]
        heights[start:start + 16] = kernel(np.sum(delta * delta, axis=-1)) @ WEIGHTS
    grass_weight = np.clip((heights + 0.20) / 0.9, 0, 1)
    grass_weight *= grass_weight * (3 - 2 * grass_weight)
    dy, dx = np.gradient(heights, 36 / size, 48 / size)
    rock_weight = np.clip((np.sqrt(dx * dx + dy * dy) - 0.88) / 0.9, 0, 0.75)
    grass_color = np.array((0.29, 0.46, 0.23))
    sand_color = np.array((0.86, 0.77, 0.55))
    rock_color = np.array((0.47, 0.51, 0.39))
    rgba = np.ones((size, size, 4), dtype=np.float32)
    rgba[:, :, :3] = sand_color * (1 - grass_weight[:, :, None]) + grass_color * grass_weight[:, :, None]
    rgba[:, :, :3] = rgba[:, :, :3] * (1 - rock_weight[:, :, None]) + rock_color * rock_weight[:, :, None]
    # Broad mottling has no high-contrast directional streaks.
    variation = 1 + 0.018 * np.sin(world_x * 1.7 + np.sin(world_y * 2.3)) * np.cos(world_y * 1.9 + np.sin(world_x))
    rgba[:, :, :3] *= variation[:, :, None]
    image = bpy.data.images.new("rf1_island_albedo", width=size, height=size, alpha=False)
    image.pixels.foreach_set(rgba.ravel())
    image.filepath_raw = os.path.join(OUTPUT, "rf1_island_albedo.png")
    image.file_format = "PNG"
    image.save()
    image.filepath = "//rf1_island_albedo.png"
    land.data.materials.clear()
    land.data.materials.append(material("RF1_IslandAlbedo", (1, 1, 1), image))
    for loop in land.data.uv_layers.active.data:
        u, v = loop.uv
        loop.uv = (u * 5 / 48 + 0.5, (v * 5 + 1.8) / 36 + 0.5)
    return land


def build_path(sand):
    vertices = []
    faces = []
    for index in range(len(PORTALS) - 1):
        start = np.array(PORTALS[index])
        end = np.array(PORTALS[index + 1])
        direction = end[:2] - start[:2]
        length = np.linalg.norm(direction)
        side = np.array((-direction[1], direction[0])) / length
        count = max(6, int(length * 6))
        offset = len(vertices)
        for step in range(count + 1):
            t = step / count
            center = start[:2] + direction * t
            width = 0.58 + 0.07 * math.sin(t * math.pi)
            for sign in (-1, 1):
                x, y = center + side * width * sign
                vertices.append((x, y, height(x, y) + 0.10))
        for step in range(count):
            a = offset + step * 2
            faces.append((a, a + 2, a + 3, a + 1))
    mesh_object("RF1_ContinuousSandTrail", vertices, faces, [sand], tile=3)


def build_water():
    # A single radial UV map provides a continuous color transition without bands.
    size = 1024
    yy, xx = np.mgrid[0:size, 0:size] / (size - 1)
    dx = (xx - 0.5) * 180
    dy = (yy - 0.5) * 180
    radius = np.sqrt((dx / 21.5) ** 2 + (dy / 15.7) ** 2)
    transition = np.clip((radius - 0.91) / 1.8, 0, 1)
    transition = transition * transition * (3 - 2 * transition)
    near = np.array((0.33, 0.79, 0.73))
    far = np.array((0.065, 0.34, 0.53))
    rgba = np.ones((size, size, 4), dtype=np.float32)
    rgba[:, :, :3] = near[None, None, :] * (1 - transition[:, :, None]) + far[None, None, :] * transition[:, :, None]
    ripple = np.sin(dx * 3.4 + 0.7 * np.sin(dy * 0.7)) * np.sin(dy * 4.3 + dx * 1.1)
    rgba[:, :, :3] *= (1 + 0.018 * ripple[:, :, None])
    image = bpy.data.images.new("rf1_lagoon", width=size, height=size, alpha=False)
    image.pixels.foreach_set(rgba.ravel())
    image.filepath_raw = os.path.join(OUTPUT, "rf1_lagoon.png")
    image.file_format = "PNG"
    image.save()
    image.filepath = "//rf1_lagoon.png"
    lagoon = material("RF1_Lagoon", (1, 1, 1), image, 0.035)
    vertices = [(-90, -91.8, SEA_Z), (90, -91.8, SEA_Z), (90, 88.2, SEA_Z), (-90, 88.2, SEA_Z)]
    water = mesh_object("StageSelect_ShallowWaterRing.001", vertices, [(0, 1, 2, 3)], [lagoon])
    for loop in water.data.uv_layers.active.data:
        u, v = loop.uv
        loop.uv = (u * 5 / 180 + 0.5, (v * 5 + 1.8) / 180 + 0.5)
    deep = material("RF1_DeepOcean", tuple(far), specular=0.025)
    mesh_object("StageSelect_DeepSea.001", [(-5000, -5000, SEA_Z - 0.02), (5000, -5000, SEA_Z - 0.02), (5000, 5000, SEA_Z - 0.02), (-5000, 5000, SEA_Z - 0.02)], [(0, 1, 2, 3)], [deep])
    foam = material("RF1_SoftShoreFoam", (0.76, 0.88, 0.78), specular=0)
    vertices = []
    faces = []
    for index in range(240):
        angle = math.tau * index / 240
        for radius in (0.977, 0.982 + 0.002 * math.sin(angle * 29)):
            x, y = coast(angle)
            vertices.append((x * radius, -1.8 + (y + 1.8) * radius, SEA_Z + 0.015))
    for index in range(240):
        if index % 19 < 14:
            a = index * 2
            b = (index + 1) % 240 * 2
            faces.append((a, a + 1, b + 1, b))
    mesh_object("RF1_ShoreFoam", vertices, faces, [foam])


def pebble(name, position, scale, mat):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=1, location=position)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    obj.rotation_euler = (RNG.uniform(-0.2, 0.2), RNG.uniform(-0.2, 0.2), RNG.uniform(0, math.tau))
    for vertex in obj.data.vertices:
        vertex.co *= RNG.uniform(0.91, 1.09)
    obj.data.materials.append(mat)
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    return obj


def palm(index, x, y, size, bark, leaf, tip):
    z = height(x, y)
    vertices = []
    faces = []
    bend = RNG.uniform(-0.65, 0.65)
    for ring in range(13):
        t = ring / 12
        for segment in range(10):
            angle = segment * math.tau / 10
            radius = size * (0.047 - 0.022 * t) * (1 + 0.055 * (ring % 2))
            vertices.append((x + bend * t * t + radius * math.cos(angle), y + 0.3 * t * t + radius * math.sin(angle), z + size * t))
    for ring in range(12):
        for segment in range(10):
            a = ring * 10 + segment
            b = ring * 10 + (segment + 1) % 10
            faces.append((a, b, b + 10, a + 10))
    mesh_object(f"RF1_Palm_{index:02d}_Trunk", vertices, faces, [bark], tile=1)
    crown = Vector((x + bend, y + 0.3, z + size))
    vertices = []
    faces = []
    for frond in range(9):
        angle = frond * math.tau / 9 + index * 0.73
        forward = Vector((math.cos(angle), math.sin(angle), 0))
        side = Vector((-math.sin(angle), math.cos(angle), 0))
        length = size * RNG.uniform(0.57, 0.74)
        offset = len(vertices)
        for step in range(13):
            t = step / 12
            center = crown + forward * length * t
            center.z += size * (0.22 * math.sin(math.pi * t) - 0.26 * t * t)
            width = size * 0.092 * math.sin(math.pi * t) ** 0.65
            for sign in (-1, 0, 1):
                point = center + side * width * sign
                if sign != 0:
                    point.z -= width * 0.28
                vertices.append(tuple(point))
        for step in range(12):
            a = offset + step * 3
            faces.extend([(a, a + 3, a + 4, a + 1), (a + 1, a + 4, a + 5, a + 2)])
    obj = mesh_object(f"RF1_Palm_{index:02d}_Fronds", vertices, faces, [leaf, tip])
    for polygon in obj.data.polygons:
        if (polygon.index // 24) % 3 == 0:
            polygon.material_index = 1
    # Give the leaves a back face for the game's culling renderer.
    modifier = obj.modifiers.new("LeafThickness", "SOLIDIFY")
    modifier.thickness = 0.012


def export_models():
    for filename, sea in (("stageSelectIsland.x", False), ("stageSelectSea.x", True)):
        bpy.ops.object.select_all(action="DESELECT")
        selected = []
        for obj in bpy.context.scene.objects:
            if obj.type == "MESH" and (obj.name in SEA_NAMES) == sea:
                obj.select_set(True)
                selected.append(obj)
        if not selected:
            raise RuntimeError("Empty model export: " + filename)
        bpy.context.view_layer.objects.active = selected[0]
        result = bpy.ops.export_scene.directx_x(filepath=os.path.join(OUTPUT, filename),
            use_selection=True, axis_forward="Z", axis_up="Y", export_animation=False,
            export_armature=False, export_weights=False, use_mesh_modifiers=True,
            use_original_material_data=False, export_format="TEXT_X")
        if "FINISHED" not in result:
            raise RuntimeError("Official DirectX export failed: " + filename)


def preview():
    scene = bpy.context.scene
    bpy.ops.object.camera_add(location=(0, -26, 18))
    camera = bpy.context.object
    camera.name = "RF1_GameCameraPreview"
    camera.rotation_euler = (Vector((0, -2, 2)) - camera.location).to_track_quat("-Z", "Y").to_euler()
    camera.data.type = "PERSP"
    camera.data.lens = 18
    camera.data.clip_end = 20000
    scene.camera = camera
    bpy.ops.object.light_add(type="AREA", location=(-18, -12, 32))
    bpy.context.object.data.energy = 4000
    bpy.context.object.data.shape = "DISK"
    bpy.context.object.data.size = 18
    bpy.ops.object.light_add(type="SUN", location=(-10, -15, 30))
    bpy.context.object.rotation_euler = (0.35, -0.4, -0.4)
    bpy.context.object.data.energy = 2
    bpy.context.object.data.angle = 0.15
    scene.world = bpy.data.worlds.new("RF1_CoastalSky")
    scene.world.use_nodes = True
    scene.world.node_tree.nodes.get("Background").inputs[0].default_value = (0.40, 0.60, 0.72, 1)
    scene.world.node_tree.nodes.get("Background").inputs[1].default_value = 0.5
    scene.render.engine = "CYCLES"
    scene.cycles.samples = 24
    scene.cycles.use_denoising = True
    scene.render.resolution_x = 1600
    scene.render.resolution_y = 900
    scene.render.resolution_percentage = 100
    scene.view_settings.view_transform = "AgX"
    scene.render.filepath = os.path.join(OUTPUT, "world1_preview.png")
    bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUTPUT, "world1.blend"))
    bpy.ops.render.render(write_still=True)


def main():
    if not OUTPUT:
        raise RuntimeError("Open world1.blend before rebuilding.")
    bpy.context.preferences.filepaths.save_version = 0
    for obj in list(bpy.data.objects):
        if not obj.name.startswith("RF1_Portal_"):
            bpy.data.objects.remove(obj, do_unlink=True)
    for mesh in list(bpy.data.meshes):
        if mesh.users == 0:
            bpy.data.meshes.remove(mesh)
    # Repeated runs have the same materials, image names and deterministic geometry.
    for mat in list(bpy.data.materials):
        if mat.users == 0:
            bpy.data.materials.remove(mat)
    for image in list(bpy.data.images):
        if image.name.startswith("rf1_"):
            bpy.data.images.remove(image)
    grass = material("RF1_JadeGrass", (1, 1, 1), texture("rf1_grass", (0.27, 0.46, 0.24), 0.10))
    sand = material("RF1_CoralSand", (1, 1, 1), texture("rf1_sand", (0.86, 0.77, 0.55), 0.045))
    rock = material("RF1_Limestone", (1, 1, 1), texture("rf1_limestone", (0.48, 0.52, 0.43), 0.10))
    bark = material("RF1_PalmBark", (1, 1, 1), texture("rf1_bark", (0.43, 0.31, 0.19), 0.16))
    leaf = material("RF1_PalmEmerald", (0.12, 0.34, 0.19))
    tip = material("RF1_PalmNewGrowth", (0.27, 0.46, 0.20))
    build_land(grass, sand, rock)
    build_path(sand)
    build_water()
    for index, (x, y, size) in enumerate([(-14, -3, 4.2), (-11, 1, 5.6), (-8, 5, 4.1), (8.8, 1, 5.1), (12, -2, 4.2), (13, 4, 3.7), (-16, -7, 3.2), (4, 7, 3.8)]):
        palm(index, x, y, size, bark, leaf, tip)
    for index, (x, y) in enumerate([(-12, -4), (-10, 0), (-6, 2), (-5, -8.2), (2.6, -8.1), (9.4, -5), (6.5, 1.8), (3, 4.5), (-2, 5), (12, 1)]):
        for part in range(3):
            bx = x + RNG.uniform(-0.45, 0.45)
            by = y + RNG.uniform(-0.45, 0.45)
            pebble(f"RF1_Shrub_{index:02d}_{part}", (bx, by, height(bx, by) + 0.14), (0.52, 0.45, 0.36), leaf)
    for index in range(45):
        angle = RNG.uniform(0, math.tau)
        x, y = coast(angle)
        factor = RNG.uniform(0.74, 0.89)
        x *= factor
        y = -1.8 + (y + 1.8) * factor
        if min(math.hypot(x - px, y - py) for px, py, pz in PORTALS) < 2.0:
            continue
        z = height(x, y)
        if factor > 0.83:
            z = -0.25 - 2.55 * (factor - 0.83) / 0.17
        size = RNG.uniform(0.24, 1.05)
        pebble(f"RF1_CoastalBoulder_{index:02d}", (x, y, z + size * 0.18), (size, size * 0.75, size * 0.65), rock)
    for index, (x, y, z) in enumerate(PORTALS):
        # Existing ring heights are the visual anchor used by the selector marker.
        for suffix in ("Base", "Inset", "Ring"):
            obj = bpy.data.objects.get(f"RF1_Portal_{index:02d}_{suffix}")
            if obj is None:
                raise RuntimeError("Missing original portal pedestal.")
            obj.location.x = x
            obj.location.y = y
        pebble(f"RF1_PortalFoundation_{index:02d}", (x, y, z + 0.12), (0.84, 0.84, 0.24), rock)
    export_models()
    print("WORLD1_REBUILD", len(bpy.context.scene.objects), "objects; portal coordinates preserved", flush=True)
    preview()


if __name__ == "__main__":
    main()
