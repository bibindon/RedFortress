#!/usr/bin/env python3
"""ワールド2～4の安全な拠点モデルとCSVを生成する。"""

import csv
import math
from pathlib import Path

import bpy


BASE_DIR = Path(__file__).resolve().parent
MODEL_DIR = BASE_DIR.parent
GROUND_HALF_WIDTH = 16.0
GROUND_HALF_DEPTH = 32.0
GROUND_THICKNESS = 3.0

HUBS = (
    {"world": 2, "folder": "base2", "sky": "../SkySphere_cave/SkySphere.blend.x"},
    {"world": 3, "folder": "base3", "sky": "../SkySphere_evening/SkySphere.blend.x"},
    {"world": 4, "folder": "base4", "sky": "../SkySphere_night/SkySphere.blend.x"},
)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials, bpy.data.images):
        for data_block in list(collection):
            if data_block.users == 0:
                collection.remove(data_block)


def create_material(name, color, roughness=0.8, metallic=0.0, emissive=None):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = color
    material.roughness = roughness
    material.metallic = metallic
    material["_x_power"] = 0.0
    material["_x_specular"] = (0.0, 0.0, 0.0)
    material["_x_texture_filename"] = ""
    material["_x_emissive"] = (0.0, 0.0, 0.0)
    principled = next(node for node in material.node_tree.nodes if node.type == "BSDF_PRINCIPLED")
    principled.inputs["Base Color"].default_value = color
    principled.inputs["Roughness"].default_value = roughness
    principled.inputs["Metallic"].default_value = metallic
    if color[3] < 1.0:
        principled.inputs["Alpha"].default_value = color[3]
        material.surface_render_method = "DITHERED"
    if emissive is not None:
        material["_x_emissive"] = emissive
        principled.inputs["Emission Color"].default_value = (emissive[0], emissive[1], emissive[2], 1.0)
        principled.inputs["Emission Strength"].default_value = 2.0
    return material


def tag_object(obj):
    obj["_x_frame_name"] = obj.name
    if obj.type == "MESH":
        obj.data.name = obj.name + "Geo"
        obj["_x_mesh_name"] = obj.data.name
    return obj


def add_box(name, x, z, y, width, depth, height, material, rotation=0.0):
    bpy.ops.mesh.primitive_cube_add(location=(x, z, y), rotation=(0.0, 0.0, math.radians(rotation)))
    obj = bpy.context.active_object
    obj.name = name
    obj.dimensions = (width, depth, height)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material)
    return tag_object(obj)


def add_cylinder(name, x, z, y, radius, height, material, vertices=12):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices, radius=radius, depth=height, location=(x, z, y))
    obj = bpy.context.active_object
    obj.name = name
    obj.data.materials.append(material)
    return tag_object(obj)


def add_cone(name, x, z, y, radius, height, material, vertices=8):
    bpy.ops.mesh.primitive_cone_add(vertices=vertices, radius1=radius, radius2=0.0, depth=height, location=(x, z, y))
    obj = bpy.context.active_object
    obj.name = name
    obj.data.materials.append(material)
    return tag_object(obj)


def add_rock(name, x, z, y, scale_x, scale_z, scale_y, material):
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=1, radius=1.0, location=(x, z, y))
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = (scale_x, scale_z, scale_y)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material)
    return tag_object(obj)


def add_portal(materials, collision):
    stone = materials["collision"]
    if not collision:
        stone = materials["portal_stone"]
    add_box("PortalStep", 0.0, 26.0, 0.18, 6.4, 3.8, 0.36, stone)
    add_box("PortalPillarLeft", -2.1, 26.0, 2.0, 0.85, 0.95, 3.8, stone)
    add_box("PortalPillarRight", 2.1, 26.0, 2.0, 0.85, 0.95, 3.8, stone)
    if collision:
        return
    add_box("PortalLintel", 0.0, 26.0, 4.0, 5.0, 1.05, 0.75, stone)
    add_box("PortalGlow", 0.0, 26.05, 2.05, 3.25, 0.08, 3.15, materials["portal_glow"])


def add_workshop(materials, collision, world):
    solid = materials["collision"]
    if not collision:
        solid = materials["wood"]
    add_box("WorkshopDeck", -8.0, -18.0, 0.18, 9.0, 7.0, 0.36, solid)
    add_box("WorkshopBench", -8.0, -18.0, 0.95, 3.8, 1.3, 1.5, solid)
    for index, x in enumerate((-11.0, -5.0)):
        add_box("WorkshopPost" + str(index), x, -20.3, 1.65, 0.22, 0.22, 3.3, solid)
    if collision:
        return
    cloth = materials["cloth"]
    add_box("WorkshopCanopy", -8.0, -18.5, 3.25, 7.0, 5.2, 0.16, cloth, -4.0)
    metal = materials["metal"]
    add_box("WorkshopToolRack", -10.4, -18.0, 1.5, 0.18, 2.2, 2.2, metal)
    for index, z in enumerate((-20.2, -18.5, -16.8)):
        add_cylinder("WorkshopBarrel" + str(index), -4.7, z, 0.7, 0.55, 1.4, materials["dark_wood"], 12)
    accent = materials["accent"]
    add_box("WorkshopWorldMarker", -8.0, -17.25, 1.85, 1.2, 0.12, 0.8, accent)
    for index in range(world):
        add_box("WorkshopMarkerBar" + str(index), -8.45 + index * 0.45, -17.15, 2.35, 0.22, 0.10, 0.62, accent)


def build_base2(materials, collision):
    rock_material = materials["collision"]
    if not collision:
        rock_material = materials["rock"]
    edge_rocks = (
        (-13.8, -25.0, 1.2, 2.4, 2.3, 1.8), (13.7, -24.0, 1.1, 2.5, 2.1, 1.7),
        (-14.2, -10.0, 1.5, 2.2, 3.0, 2.1), (14.0, -7.0, 1.4, 2.3, 2.8, 2.0),
        (-13.8, 8.0, 1.3, 2.5, 2.4, 1.9), (14.0, 11.0, 1.5, 2.2, 3.0, 2.2),
        (-12.8, 24.0, 1.2, 2.5, 2.2, 1.8), (12.8, 25.0, 1.3, 2.4, 2.3, 1.9),
    )
    for index, values in enumerate(edge_rocks):
        add_rock("CaveRock" + str(index), *values, rock_material)
    beam_material = materials["collision"]
    if not collision:
        beam_material = materials["dark_wood"]
    for index, z in enumerate((-10.0, 8.0)):
        add_box("MineSupportLeft" + str(index), -11.0, z, 2.1, 0.45, 0.55, 4.2, beam_material)
        add_box("MineSupportRight" + str(index), 11.0, z, 2.1, 0.45, 0.55, 4.2, beam_material)
        if not collision:
            add_box("MineSupportTop" + str(index), 0.0, z, 4.05, 22.5, 0.55, 0.45, beam_material)
    if collision:
        return
    rail = materials["metal"]
    for x in (-1.1, 1.1):
        add_box("MineRail" + str(x), x, -2.0, 0.08, 0.12, 25.0, 0.12, rail)
    for index, z in enumerate(range(-14, 11, 3)):
        add_box("MineTie" + str(index), 0.0, float(z), 0.06, 3.2, 0.28, 0.12, materials["wood"])
    water = materials["water"]
    add_box("MineralPool", 8.0, 3.0, 0.05, 7.0, 9.0, 0.08, water)
    for index, values in enumerate(((5.0, 1.0, 0.8), (10.5, 0.0, 1.2), (11.0, 6.0, 0.9), (5.5, 7.0, 1.1))):
        x, z, scale = values
        add_cone("Crystal" + str(index), x, z, scale, 0.55 * scale, 2.0 * scale, materials["crystal"], 6)


def build_base3(materials, collision):
    stone = materials["collision"]
    if not collision:
        stone = materials["ruin_stone"]
    add_box("RuinPlaza", 0.0, 2.0, 0.15, 17.0, 15.0, 0.30, stone)
    for index, values in enumerate(((-7.0, -2.0, 2.1), (7.0, -2.0, 1.6), (-7.0, 7.0, 1.4), (7.0, 7.0, 2.4))):
        x, z, height = values
        add_cylinder("RuinColumn" + str(index), x, z, height * 0.5, 0.65, height, stone, 12)
    for index in range(5):
        height = 0.30 + index * 0.36
        add_box("LookoutStep" + str(index), 9.0, 12.0 + index * 1.35, height * 0.5, 7.0, 1.5, height, stone)
    add_box("LookoutDeck", 9.0, 20.0, 1.82, 10.0, 7.0, 0.36, stone)
    add_box("LookoutRailEast", 14.0, 20.0, 2.55, 0.25, 7.0, 1.5, stone)
    add_box("LookoutRailNorth", 9.0, 23.4, 2.55, 10.0, 0.25, 1.5, stone)
    if collision:
        return
    for index, x in enumerate((-5.5, 0.0, 5.5)):
        add_box("RuinBannerPole" + str(index), x, 8.0, 2.5, 0.12, 0.12, 5.0, materials["metal"])
        add_box("RuinBanner" + str(index), x + 0.65, 8.0, 3.2, 1.3, 0.08, 1.8, materials["cloth"])
    add_cylinder("RelicPedestal", 9.0, 20.0, 2.35, 1.1, 0.8, stone, 12)
    add_cone("RelicGlow", 9.0, 20.0, 3.4, 0.55, 1.4, materials["accent"], 8)
    grass = materials["grass"]
    for index, values in enumerate(((-12.0, -24.0), (12.0, -22.0), (-12.5, 14.0), (3.0, 15.0), (-11.0, 25.0))):
        add_cone("MountainShrub" + str(index), values[0], values[1], 0.65, 0.75, 1.3, grass, 7)


def add_battlements(prefix, start, end, fixed, horizontal, material):
    index = 0
    value = start
    while value <= end:
        if horizontal:
            add_box(prefix + str(index), value, fixed, 3.5, 1.1, 0.8, 1.1, material)
        else:
            add_box(prefix + str(index), fixed, value, 3.5, 0.8, 1.1, 1.1, material)
        value += 2.2
        index += 1


def build_base4(materials, collision):
    stone = materials["collision"]
    if not collision:
        stone = materials["fortress_stone"]
    add_box("FortressWallWest", -14.7, 0.0, 1.6, 1.0, 61.0, 3.2, stone)
    add_box("FortressWallEast", 14.7, 0.0, 1.6, 1.0, 61.0, 3.2, stone)
    add_box("FortressWallNorthLeft", -9.0, 30.0, 1.6, 11.0, 1.0, 3.2, stone)
    add_box("FortressWallNorthRight", 9.0, 30.0, 1.6, 11.0, 1.0, 3.2, stone)
    for index, values in enumerate(((-13.5, -26.0), (13.5, -26.0), (-13.5, 26.0), (13.5, 26.0))):
        add_cylinder("FortressTower" + str(index), values[0], values[1], 2.2, 2.2, 4.4, stone, 12)
    table_material = materials["collision"]
    if not collision:
        table_material = materials["dark_wood"]
    add_box("CommandTable", 0.0, 2.0, 0.95, 6.0, 3.8, 1.6, table_material)
    add_box("ArmoryRack", 9.5, -16.0, 1.4, 1.0, 8.0, 2.8, table_material)
    if collision:
        return
    add_battlements("BattlementWest", -27.0, 27.0, -14.7, False, stone)
    add_battlements("BattlementEast", -27.0, 27.0, 14.7, False, stone)
    map_material = materials["map"]
    add_box("CommandMap", 0.0, 2.0, 1.82, 4.8, 2.8, 0.08, map_material)
    for index, x in enumerate((-9.0, 9.0)):
        add_cylinder("BrazierStand" + str(index), x, -17.0, 1.0, 0.32, 2.0, materials["metal"], 10)
        add_cone("BrazierFlame" + str(index), x, -17.0, 2.25, 0.48, 1.1, materials["fire"], 8)
    for index, z in enumerate((-18.5, -16.0, -13.5)):
        add_box("ArmoryWeapon" + str(index), 9.3, z, 2.2, 0.12, 1.4, 0.12, materials["metal"], 25.0)


def create_materials(world, collision=False):
    collision_material = create_material("HubCollision", (0.35, 0.35, 0.35, 1.0), 1.0)
    materials = {"collision": collision_material}
    if collision:
        return materials
    materials.update({
        "wood": create_material("HubWood", (0.32, 0.15, 0.05, 1.0), 0.86),
        "dark_wood": create_material("HubDarkWood", (0.13, 0.055, 0.02, 1.0), 0.9),
        "metal": create_material("HubMetal", (0.13, 0.16, 0.20, 1.0), 0.35, 0.55),
        "cloth": create_material("HubCloth", (0.34, 0.08 + world * 0.03, 0.06, 1.0), 0.78),
        "portal_stone": create_material("PortalStone", (0.22, 0.25, 0.31, 1.0), 0.92),
        "portal_glow": create_material("PortalGlow", (0.04, 0.25, 0.75, 0.76), 0.25, 0.0, (0.08, 0.45, 1.0)),
        "accent": create_material("HubAccent", (0.10, 0.45, 1.0, 1.0), 0.28, 0.1, (0.08, 0.35, 0.9)),
        "rock": create_material("CaveRock", (0.16, 0.18, 0.22, 1.0), 0.96),
        "crystal": create_material("CaveCrystal", (0.08, 0.45, 0.95, 0.88), 0.18, 0.05, (0.08, 0.42, 1.0)),
        "water": create_material("MineralWater", (0.02, 0.12, 0.22, 0.70), 0.22, 0.0, (0.01, 0.08, 0.16)),
        "ruin_stone": create_material("RuinStone", (0.38, 0.34, 0.30, 1.0), 0.94),
        "grass": create_material("MountainGrass", (0.15, 0.28, 0.10, 1.0), 0.92),
        "fortress_stone": create_material("FortressStone", (0.10, 0.12, 0.18, 1.0), 0.90),
        "map": create_material("CommandMap", (0.12, 0.28, 0.58, 1.0), 0.70, 0.0, (0.03, 0.12, 0.38)),
        "fire": create_material("BrazierFire", (1.0, 0.16, 0.02, 0.92), 0.15, 0.0, (1.0, 0.10, 0.01)),
    })
    return materials


def create_ground(world):
    color = (0.18, 0.20, 0.23, 1.0)
    if world == 3:
        color = (0.34, 0.30, 0.23, 1.0)
    if world == 4:
        color = (0.09, 0.10, 0.15, 1.0)
    material = create_material("HubGround", color, 0.96)
    return add_box("Base" + str(world) + "Ground", 0.0, 0.0, -GROUND_THICKNESS * 0.5,
                   GROUND_HALF_WIDTH * 2.0, GROUND_HALF_DEPTH * 2.0, GROUND_THICKNESS, material)


def build_decor(world, collision=False):
    materials = create_materials(world, collision)
    add_workshop(materials, collision, world)
    add_portal(materials, collision)
    if world == 2:
        build_base2(materials, collision)
    elif world == 3:
        build_base3(materials, collision)
    elif world == 4:
        build_base4(materials, collision)
    return list(bpy.context.scene.objects)


def export_x(path, objects):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    if objects:
        bpy.context.view_layer.objects.active = objects[0]
    result = bpy.ops.export_scene.directx_x(
        filepath=str(path), use_selection=True, use_mesh_modifiers=True,
        global_scale=1.0, axis_forward="Z", axis_up="Y",
        export_normals=True, export_uvs=True, export_materials=True,
        export_textures=False, export_armature=False, export_weights=False,
        export_animation=False, unweld_on_export=False,
        export_format="TEXT_X", triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(path))


def write_csv(path, header, rows):
    with path.open("w", encoding="utf-8-sig", newline="") as output:
        writer = csv.writer(output, lineterminator="\r\n")
        writer.writerow(header)
        writer.writerows(rows)


def write_hub_csvs(hub):
    world = hub["world"]
    folder = hub["folder"]
    output_dir = MODEL_DIR / folder
    render_rows = (
        (2, "base_ground.x", 0, 0, 0, 0, 0, 0, 1, "normal"),
        (3, "base_decor.x", 0, 0, 0, 0, 0, 0, 1, "normal"),
        (4, hub["sky"], 0, 0.01, 0, 0, 0, 0, 1, "normal"),
    )
    physics_rows = (
        (1, "res/model/cubeNormalInverse30x60.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""),
        (2, "res/model/" + folder + "/base_ground.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""),
        (3, "res/model/" + folder + "/base_decor_collision.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""),
    )
    write_csv(output_dir / "XFileList_simple.csv",
              ("ID", "FileName", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "loadType"), render_rows)
    write_csv(output_dir / "XFileListPhysics.csv",
              ("ID", "FileName", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "Type", "Move", "Instancing"), physics_rows)
    write_csv(output_dir / "XFileListMove.csv",
              ("ID", "RenderID", "PhysicsID", "PosX", "PosY", "PosZ", "RotX", "RotY", "RotZ", "Scale", "StartX", "StartY", "StartZ", "EndX", "EndY", "EndZ", "Duration"), ())
    write_csv(output_dir / "EnemyPositions.csv", ("Type", "PosX", "PosY", "PosZ", "RotY"), ())
    write_csv(output_dir / "Collectibles.csv", ("CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"), ())
    write_csv(output_dir / "Interactables.csv", ("InteractionID", "Type", "PosX", "PosY", "PosZ", "PromptDistance"), (
        (folder + "-crafting-station-01", "CraftingStation", -8, 1, -18, 3.2),
        (folder + "-return-portal", "ReturnPortal", 0, 1, 26, 1.8),
    ))
    write_csv(output_dir / "Stars.csv", ("PosX", "PosY", "PosZ"), ())
    write_csv(output_dir / "SpeedUps.csv", ("PosX", "PosY", "PosZ"), ())
    write_csv(output_dir / "Destructibles.csv", ("PosX", "PosY", "PosZ", "HP"), ())
    write_csv(output_dir / "DashBoosters.csv", ("DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ", "Speed", "Duration", "Radius", "Scale"), ())
    write_csv(output_dir / "LavaZones.csv", ("ID", "PhysicsID", "Damage"), ())


def main():
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    for hub in HUBS:
        world = hub["world"]
        output_dir = MODEL_DIR / hub["folder"]
        output_dir.mkdir(parents=True, exist_ok=True)

        clear_scene()
        ground = create_ground(world)
        bpy.ops.wm.save_as_mainfile(filepath=str(output_dir / "base_ground.blend"))
        export_x(output_dir / "base_ground.x", (ground,))

        clear_scene()
        decor_objects = build_decor(world, False)
        bpy.ops.wm.save_as_mainfile(filepath=str(output_dir / "base_decor.blend"))
        export_x(output_dir / "base_decor.x", decor_objects)

        clear_scene()
        collision_objects = build_decor(world, True)
        bpy.ops.wm.save_as_mainfile(filepath=str(output_dir / "base_decor_collision.blend"))
        export_x(output_dir / "base_decor_collision.x", collision_objects)

        write_hub_csvs(hub)
        print("EXPORTED HUB", world, output_dir)


main()