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
    # Sheltered bays alternate with rocky headlands; the entry peninsula stays intact.
    for center, width, depth in ((-1.12, 0.25, 0.19), (-1.77, 0.19, 0.10),
                                 (-0.32, 0.22, 0.12), (0.75, 0.24, 0.10)):
        offset = math.atan2(math.sin(angle - center), math.cos(angle - center))
        irregular -= depth * math.exp(-(offset / width) ** 2)
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


def surface_height(x, y):
    angle = math.atan2((y + 1.8) / 15.7, x / 21.5)
    coast_x, coast_y = coast(angle)
    radius = math.hypot(x / 21.5, (y + 1.8) / 15.7)
    radius /= math.hypot(coast_x / 21.5, (coast_y + 1.8) / 15.7)
    if radius > 0.83:
        return -0.25 - 2.55 * (radius - 0.83) / 0.17
    return height(x, y)


def path_clearance(x, y):
    point = np.array((x, y))
    minimum = 1000.0
    for start, end in zip(PORTALS[:-1], PORTALS[1:]):
        start_xy = np.array(start[:2])
        delta = np.array(end[:2]) - start_xy
        t = float(np.clip(np.dot(point - start_xy, delta) / np.dot(delta, delta), 0, 1))
        minimum = min(minimum, float(np.linalg.norm(point - start_xy - delta * t)))
    return minimum


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
    world_x = (xx - 0.5) * 60
    world_y = (yy - 0.5) * 44 - 1.8
    heights = np.zeros((size, size))
    for start in range(0, size, 16):
        locations = np.stack((world_x[start:start + 16], world_y[start:start + 16]), axis=-1)
        delta = locations[:, :, None, :] - SAMPLES[None, None, :, :]
        heights[start:start + 16] = kernel(np.sum(delta * delta, axis=-1)) @ WEIGHTS
    grass_weight = np.clip((heights + 0.20) / 0.9, 0, 1)
    grass_weight *= grass_weight * (3 - 2 * grass_weight)
    dy, dx = np.gradient(heights, 44 / size, 60 / size)
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
        loop.uv = (u * 5 / 60 + 0.5, (v * 5 + 1.8) / 44 + 0.5)
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
    # The ocean surrounds the lagoon without overlapping its depth surface.
    ocean_vertices = [(-90, -91.8, SEA_Z), (90, -91.8, SEA_Z),
                      (90, 88.2, SEA_Z), (-90, 88.2, SEA_Z),
                      (-5000, -5001.8, SEA_Z), (5000, -5001.8, SEA_Z),
                      (5000, 4998.2, SEA_Z), (-5000, 4998.2, SEA_Z)]
    mesh_object("StageSelect_DeepSea.001", ocean_vertices,
                [(0, 4, 5, 1), (1, 5, 6, 2), (2, 6, 7, 3), (3, 7, 4, 0)], [deep])
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


class DetailMesh:
    """Batch small authored details into a few renderable meshes."""

    def __init__(self, name, materials):
        self.name = name
        self.materials = materials
        self.vertices = []
        self.faces = []
        self.indices = []

    def face(self, points, material_index=0, double_sided=False):
        start = len(self.vertices)
        self.vertices.extend([tuple(point) for point in points])
        face = tuple(range(start, start + len(points)))
        self.faces.append(face)
        self.indices.append(material_index)
        if double_sided:
            normal = (Vector(points[1]) - Vector(points[0])).cross(Vector(points[2]) - Vector(points[0])).normalized()
            back_start = len(self.vertices)
            self.vertices.extend([tuple(Vector(point) - normal * 0.003) for point in points])
            self.faces.append(tuple(reversed(range(back_start, back_start + len(points)))))
            self.indices.append(material_index)

    def leaf(self, start, end, width, material_index):
        start = Vector(start)
        end = Vector(end)
        delta = end - start
        side = Vector((-delta.y, delta.x, 0)).normalized() * width
        center = start.lerp(end, 0.48)
        ridge = center + Vector((0, 0, width * 0.3))
        self.face((start, center - side, ridge), material_index, True)
        self.face((start, ridge, center + side), material_index, True)
        self.face((center - side, end, ridge), material_index, True)
        self.face((ridge, end, center + side), material_index, True)

    def finish(self):
        if not self.faces:
            raise RuntimeError("Empty scenery batch: " + self.name)
        obj = mesh_object(self.name, self.vertices, self.faces, self.materials, smooth=False)
        for polygon, index in zip(obj.data.polygons, self.indices):
            polygon.material_index = index
        return obj


def stone_block(name, position, scale, mat, angle=0, bevel=0.07):
    bpy.ops.mesh.primitive_cube_add(size=1, location=position)
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    obj.rotation_euler.z = angle
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(mat)
    modifier = obj.modifiers.new("WornCorners", "BEVEL")
    modifier.width = bevel
    modifier.segments = 1
    return obj


def timber(name, start, end, radius, mat):
    start = Vector(start)
    end = Vector(end)
    delta = end - start
    bpy.ops.mesh.primitive_cylinder_add(vertices=8, radius=radius,
        depth=delta.length, location=(start + end) * 0.5)
    obj = bpy.context.object
    obj.name = name
    obj.rotation_euler = delta.to_track_quat("Z", "Y").to_euler()
    obj.data.materials.append(mat)
    return obj


def build_botanical_beds(leaf, tip, bark):
    shade = material("RF1_FernShade", (0.08, 0.23, 0.12))
    grass_tip = material("RF1_GrassSunTips", (0.44, 0.55, 0.20))
    flower = material("RF1_HibiscusCoral", (0.91, 0.29, 0.19))
    flower_gold = material("RF1_FlowerGold", (0.99, 0.72, 0.19))
    cream = material("RF1_BeachFlowers", (0.95, 0.91, 0.67))
    beds = DetailMesh("RF1_BotanicalUnderstory", [leaf, tip, shade, grass_tip, flower, flower_gold, cream])
    centers = [(-15, -4, 2.1), (-12, 0, 2.2), (-9, 4, 2), (-6, 6, 1.6),
               (6, 5, 1.7), (10, 1, 2.1), (13, -3, 1.8), (13, 4, 1.8),
               (-8, -9, 1.4), (-2, -9.8, 1.6), (5, -9, 1.4),
               (9, -7.5, 1.0), (-18, -10.5, 1.2), (1, 6, 1.4)]
    for bed_index, (cx, cy, spread) in enumerate(centers):
        for plant_index in range(32):
            angle = RNG.uniform(0, math.tau)
            radius = spread * math.sqrt(RNG.random())
            x = cx + math.cos(angle) * radius
            y = cy + math.sin(angle) * radius
            if path_clearance(x, y) < 1.15 or surface_height(x, y) < 0.0:
                continue
            origin = Vector((x, y, surface_height(x, y) + 0.02))
            length = RNG.uniform(0.30, 0.65)
            for blade in range(6):
                theta = angle + blade * math.tau / 6
                direction = Vector((math.cos(theta), math.sin(theta), 0))
                end = origin + direction * length * 0.60 + Vector((0, 0, length))
                beds.leaf(origin, end, length * 0.085, (plant_index + blade) % 4)
            if plant_index % 8 == 0:
                center = origin + Vector((0, 0, length * 0.85))
                for petal in range(5):
                    theta = petal * math.tau / 5
                    end = center + Vector((math.cos(theta) * 0.19, math.sin(theta) * 0.19, 0.045))
                    color_index = 4
                    if bed_index % 2 == 0:
                        color_index = 6
                    beds.leaf(center, end, 0.065, color_index)
                beds.face((center + Vector((-0.045, -0.035, 0.04)),
                           center + Vector((0.045, -0.035, 0.04)),
                           center + Vector((0, 0.05, 0.04))), 5, True)
        # Several recognizable fern rosettes punctuate each grass cluster.
        for fern_index in range(4):
            x = cx + RNG.uniform(-spread * 0.6, spread * 0.6)
            y = cy + RNG.uniform(-spread * 0.6, spread * 0.6)
            if path_clearance(x, y) < 1.4 or surface_height(x, y) < 0.0:
                continue
            origin = Vector((x, y, surface_height(x, y)))
            for frond in range(7):
                theta = frond * math.tau / 7 + fern_index
                forward = Vector((math.cos(theta), math.sin(theta), 0))
                side = Vector((-math.sin(theta), math.cos(theta), 0))
                length = RNG.uniform(0.62, 1.03)
                for pair in range(1, 8):
                    t = pair / 8
                    spine = origin + forward * length * t + Vector((0, 0, 0.22 + math.sin(t * math.pi) * length * 0.37))
                    for sign in (-1, 1):
                        end = spine + side * sign * length * 0.23 * math.sin(t * math.pi)
                        end += forward * length * 0.12
                        beds.leaf(spine, end, 0.055 * length, (pair + frond) % 3)
    beds.finish()
    # Taller vegetation stays on the flanks or behind the portal route.
    for index, (x, y, size) in enumerate([(-16, -1, 3.5), (-13, 3, 4.2), (-10, 6, 3.4),
                                         (11.5, 3.5, 4.1), (14.2, 0, 3.5), (8, 6, 3.1)]):
        palm(index + 20, x, y, size, bark, leaf, tip)


def build_sanctuary(rock, leaf, tip):
    stone = material("RF1_RuinSandstone", (1, 1, 1), texture("rf1_ruin_stone", (0.66, 0.64, 0.48), 0.15))
    moss = material("RF1_AncientMoss", (0.24, 0.35, 0.12))
    gold = material("RF1_SunRelief", (0.75, 0.50, 0.16))
    cx, cy = -0.4, 3.7
    base = max(surface_height(cx + dx, cy + dy) for dx in (-1.9, 1.9) for dy in (-1.2, 1.2))
    stone_block("RF1_Ruin_Foundation", (cx, cy, base - 0.26), (4.5, 3.3, 0.65), rock, bevel=0.16)
    stone_block("RF1_Ruin_Step", (cx, cy - 0.18, base + 0.06), (4.05, 3.0, 0.24), stone)
    stone_block("RF1_Ruin_Paving", (cx, cy, base + 0.26), (3.65, 2.60, 0.18), stone)
    floor_z = base + 0.35
    for side in (-1, 1):
        x = cx + side * 1.20
        stone_block(f"RF1_Ruin_ColumnFoot_{side}", (x, cy, floor_z + 0.14), (0.90, 0.9, 0.28), stone)
        for tier in range(4):
            stone_block(f"RF1_Ruin_Column_{side}_{tier}", (x, cy, floor_z + 0.51 + tier * 0.46),
                        (0.58, 0.67, 0.43), stone, angle=side * 0.012 * tier)
        stone_block(f"RF1_Ruin_Capital_{side}", (x, cy, floor_z + 2.18), (0.90, 0.87, 0.22), stone)
    arch = DetailMesh("RF1_Ruin_Arch", [stone, moss])
    center_z = floor_z + 2.18
    for segment in range(11):
        a = segment * math.pi / 11 + 0.015
        b = (segment + 1) * math.pi / 11 - 0.015
        quad = [(cx + math.cos(a) * 1.05, center_z + math.sin(a) * 1.05),
                (cx + math.cos(a) * 1.58, center_z + math.sin(a) * 1.58),
                (cx + math.cos(b) * 1.58, center_z + math.sin(b) * 1.58),
                (cx + math.cos(b) * 1.05, center_z + math.sin(b) * 1.05)]
        front = [Vector((x, cy - 0.38, z)) for x, z in quad]
        back = [Vector((x, cy + 0.38, z)) for x, z in quad]
        arch.face(front, 0)
        arch.face(list(reversed(back)), 0)
        for edge in range(4):
            nxt = (edge + 1) % 4
            index = 0
            if edge == 1 and segment % 3 == 0:
                index = 1
            arch.face((front[edge], back[edge], back[nxt], front[nxt]), index)
    arch.finish()
    # A small sun emblem makes the landmark recognizable without resembling a portal.
    emblem = DetailMesh("RF1_Ruin_SunEmblem", [gold])
    center = Vector((cx, cy - 0.40, center_z + 1.34))
    for ray in range(12):
        a = ray * math.tau / 12
        b = (ray + 1) * math.tau / 12
        emblem.face((center, center + Vector((math.cos(a) * 0.25, 0, math.sin(a) * 0.25)),
                     center + Vector((math.cos(b) * 0.25, 0, math.sin(b) * 0.25))), 0, True)
    emblem.finish()
    for index, (x, y, tiers) in enumerate([(-3.5, 4.3, 2), (2.8, 4.8, 3), (-2.8, 6, 1)]):
        z = surface_height(x, y)
        for tier in range(tiers):
            stone_block(f"RF1_Ruin_BrokenPillar_{index}_{tier}", (x, y, z + 0.24 + tier * 0.43),
                        (0.68, 0.66, 0.41), stone, angle=tier * 0.07)
    for index in range(9):
        x = cx + RNG.uniform(-2.8, 2.8)
        y = cy + RNG.uniform(-0.6, 2.5)
        z = surface_height(x, y)
        stone_block(f"RF1_Ruin_FallenBlock_{index}", (x, y, z + 0.14), (0.63, 0.41, 0.31), stone, RNG.uniform(0, math.tau))
    vines = DetailMesh("RF1_Ruin_TrailingVines", [leaf, tip, moss])
    for vine in range(6):
        x = cx + 1.0 + RNG.uniform(-0.20, 0.42)
        for step in range(10):
            z = floor_z + 3.1 - step * 0.23
            start = Vector((x + math.sin(step * 0.8) * 0.11, cy - 0.44, z))
            end = start + Vector((0.20 * math.cos(step * math.pi), -0.04, -0.22))
            vines.leaf(start, end, 0.105, (step + vine) % 3)
    vines.finish()
    for step in range(5):
        y = 1.0 + step * 0.4
        x = cx + 0.08 * math.sin(step)
        stone_block(f"RF1_Ruin_Approach_{step}", (x, y, surface_height(x, y) + 0.07), (0.75, 0.30, 0.13), stone, step * 0.025)


def build_coastal_details(rock, bark, leaf):
    dark_rock = material("RF1_WetBasalt", (0.22, 0.29, 0.26))
    moss = material("RF1_CliffMoss", (0.28, 0.39, 0.18))
    wood = material("RF1_Driftwood", (1, 1, 1), texture("rf1_weathered_wood", (0.52, 0.41, 0.27), 0.17))
    pale = material("RF1_ShellIvory", (0.94, 0.87, 0.68))
    coral = material("RF1_StarfishOchre", (0.78, 0.35, 0.17))
    # Layered rock outcrops frame the island rather than filling the route.
    for group, (cx, cy, scale) in enumerate([(-17.5, -4.5, 1.4), (-12.5, 5, 1.7),
                                            (15.5, 1, 1.5), (12.5, -7.7, 1.25),
                                            (5.5, -13.5, 0.9)]):
        for part in range(6):
            x = cx + RNG.uniform(-1.2, 1.2)
            y = cy + RNG.uniform(-1, 1)
            if path_clearance(x, y) < 1.9:
                continue
            z = max(SEA_Z - 0.3, surface_height(x, y))
            size = scale * RNG.uniform(0.65, 1.25)
            obj = pebble(f"RF1_Headland_{group}_{part}", (x, y, z + size * 0.20),
                         (size * 1.05, size * 0.78, size * 0.73), rock)
            obj.data.materials.append(dark_rock)
            obj.data.materials.append(moss)
            for polygon in obj.data.polygons:
                polygon.use_smooth = False
                if polygon.center.z > 0.35:
                    polygon.material_index = 2
                elif polygon.center.z < -0.25:
                    polygon.material_index = 1
    # A timber landing reaches into the eastern inlet.
    pier_x, pier_y = 12.7, -11.5
    deck_z = -1.55
    for index in range(16):
        y = pier_y - index * 0.29
        stone_block(f"RF1_Jetty_Plank_{index}", (pier_x, y, deck_z + RNG.uniform(-0.025, 0.025)),
                    (1.7, 0.25, 0.12), wood, RNG.uniform(-0.015, 0.015), 0.025)
    for side in (-1, 1):
        x = pier_x + side * 0.69
        timber(f"RF1_Jetty_Beam_{side}", (x, pier_y + 0.3, deck_z - 0.19),
               (x, pier_y - 4.65, deck_z - 0.19), 0.12, bark)
        for index in range(4):
            y = pier_y - index * 1.4
            timber(f"RF1_Jetty_Post_{side}_{index}", (x, y, SEA_Z - 0.8), (x, y, deck_z + 0.52), 0.13, wood)
    # Driftwood has forks and lies on the beach, with small shell groups beside it.
    for index, (x, y, angle) in enumerate([(-9.0, -12.1, -0.4), (4, -10.8, 0.3), (17, -4, 1.4)]):
        z = surface_height(x, y) + 0.17
        direction = Vector((math.cos(angle), math.sin(angle), 0))
        origin = Vector((x, y, z))
        timber(f"RF1_Driftwood_{index}", origin - direction * 1.2, origin + direction * 1.2, 0.16, wood)
        timber(f"RF1_Driftwood_Fork_{index}", origin, origin + Vector((0.2, 0.75, 0.3)), 0.075, wood)
    shells = DetailMesh("RF1_BeachShellsAndStarfish", [pale, coral])
    for index in range(55):
        angle = RNG.uniform(-2.7, -0.05)
        coast_x, coast_y = coast(angle)
        radius = RNG.uniform(0.87, 0.95)
        x = coast_x * radius
        y = -1.8 + (coast_y + 1.8) * radius
        z = surface_height(x, y) + 0.045
        center = Vector((x, y, z))
        size = RNG.uniform(0.09, 0.18)
        if index % 5 == 0:
            for ray in range(5):
                a = ray * math.tau / 5 + index
                shells.face((center + Vector((math.cos(a - 0.5) * size * 0.32, math.sin(a - 0.5) * size * 0.32, 0)),
                             center + Vector((math.cos(a) * size * 1.5, math.sin(a) * size * 1.5, 0)),
                             center + Vector((math.cos(a + 0.5) * size * 0.32, math.sin(a + 0.5) * size * 0.32, 0))), 1, True)
        else:
            for rib in range(7):
                a = rib * math.pi / 7
                b = (rib + 1) * math.pi / 7
                shells.face((center + Vector((0, 0, 0.07)),
                             center + Vector((math.cos(a) * size, math.sin(a) * size, 0)),
                             center + Vector((math.cos(b) * size, math.sin(b) * size, 0))), 0, True)
    shells.finish()


def consolidate_scenery():
    # Keep the sea split and portal objects intact, batch decorative meshes by theme.
    groups = {"RF1_RuinMasonry": ("RF1_Ruin_",),
              "RF1_CoastalTimber": ("RF1_Jetty_", "RF1_Driftwood_"),
              "RF1_RockGardens": ("RF1_CoastalBoulder_", "RF1_Headland_", "RF1_Shrub_")}
    for name, prefixes in groups.items():
        objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH" and obj.name.startswith(prefixes)]
        if not objects:
            raise RuntimeError("Missing scenery group: " + name)
        bpy.ops.object.select_all(action="DESELECT")
        for obj in objects:
            obj.select_set(True)
        bpy.context.view_layer.objects.active = objects[0]
        bpy.ops.object.convert(target="MESH")
        # Bake each static piece before joining so the batch has an identity frame.
        bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
        bpy.ops.object.join()
        bpy.context.object.name = name


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
    bpy.ops.object.camera_add(location=(0, -32, 21))
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
    camera.location = (32, -38, 26)
    camera.rotation_euler = (Vector((0, -2, 2)) - camera.location).to_track_quat("-Z", "Y").to_euler()
    scene.render.filepath = os.path.join(OUTPUT, "world1_side_preview.png")
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
    build_botanical_beds(leaf, tip, bark)
    build_sanctuary(rock, leaf, tip)
    build_coastal_details(rock, bark, leaf)
    consolidate_scenery()
    export_models()
    print("WORLD1_REBUILD", len(bpy.context.scene.objects), "objects; portal coordinates preserved", flush=True)
    preview()


if __name__ == "__main__":
    main()
