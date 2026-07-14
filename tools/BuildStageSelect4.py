import math
import os
import sys

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import EnhanceStageSelect2 as common
import BuildStageSelect3 as world3


COLLECTION_NAME = "RF4_DawnFinalIsland"

PORTALS = [
    ("select3", -18.0, -12.0, 0.65),
    ("4-1", -12.0, -10.0, 0.70),
    ("4-2", -5.0, -7.0, 0.78),
    ("4-3", 3.0, -9.0, 0.74),
    ("4-4", 10.0, -5.0, 1.00),
    ("4-5", 7.0, 2.0, 1.55),
    ("4-6", 0.0, 5.0, 2.10),
    ("4-7", -7.0, 10.0, 2.80),
    ("4-8", 0.0, 15.5, 3.55),
    ("base", 18.0, -10.0, 0.72),
]


def get_arguments():
    if "--" not in sys.argv:
        raise RuntimeError("Expected output .x and preview paths.")
    arguments = sys.argv[sys.argv.index("--") + 1:]
    if len(arguments) != 2:
        raise RuntimeError("Expected: <output-x-path> <preview-png-path>")
    return os.path.abspath(arguments[0]), os.path.abspath(arguments[1])


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in list(bpy.data.collections):
        if collection.name != "Collection":
            bpy.data.collections.remove(collection)
    collection = bpy.data.collections.get("Collection")
    if collection is None:
        collection = bpy.data.collections.new(COLLECTION_NAME)
        bpy.context.scene.collection.children.link(collection)
    collection.name = COLLECTION_NAME
    return collection


def create_material(name, texture_path, base_color, emission_strength=0.0):
    return world3.create_material(name, texture_path, base_color, emission_strength)


def create_tree(collection, prefix, location, scale, trunk_material, leaf_material, seed):
    x, y, z = location
    common.create_cylinder(collection, prefix + "Trunk", (x, y, z + 1.35 * scale), 0.25 * scale, 2.7 * scale, 9, trunk_material)
    crowns = [
        (x, y, z + 3.1 * scale, 1.25, 1.05, 1.30),
        (x - 0.65 * scale, y + 0.15 * scale, z + 2.85 * scale, 0.85, 0.75, 0.90),
        (x + 0.60 * scale, y - 0.10 * scale, z + 2.95 * scale, 0.90, 0.80, 0.95),
    ]
    for index, (cx, cy, cz, sx, sy, sz) in enumerate(crowns):
        common.create_rock(
            collection,
            prefix + "Crown%02d" % index,
            (cx, cy, cz),
            (sx * scale, sy * scale, sz * scale),
            seed * 0.27 + index,
            leaf_material,
            seed + index,
        )


def create_campfire(collection, center, materials):
    x, y, z = center
    for index in range(9):
        angle = math.tau * index / 9.0
        common.create_rock(
            collection,
            "RF4_FireRingStone_%02d" % index,
            (x + math.cos(angle) * 1.05, y + math.sin(angle) * 1.05, z + 0.18),
            (0.34, 0.28, 0.22),
            angle,
            materials["rock"],
            410 + index,
        )
    common.create_box(collection, "RF4_FireLogA", (x, y, z + 0.25), (1.0, 0.16, 0.16), math.radians(28.0), materials["wood"])
    common.create_box(collection, "RF4_FireLogB", (x, y, z + 0.28), (1.0, 0.16, 0.16), math.radians(-32.0), materials["wood"])
    common.create_crystal(collection, "RF4_CampfireFlameA", (x, y, z + 0.75), 0.50, 1.45, (0.0, 0.0, 0.15), materials["fire"])
    common.create_crystal(collection, "RF4_CampfireFlameB", (x + 0.28, y, z + 0.60), 0.30, 1.05, (0.0, 0.12, -0.3), materials["gold"])


def create_motorboat(collection, materials):
    world3.create_polygon_prism(
        collection,
        "RF4_MotorboatHull",
        [(19.0, -13.0), (23.5, -12.2), (25.5, -10.5), (23.0, -9.8), (18.5, -10.6)],
        0.05,
        0.75,
        materials["boat"],
    )
    common.create_box(collection, "RF4_MotorboatDeck", (21.5, -11.2, 0.92), (2.15, 0.78, 0.13), math.radians(8.0), materials["wood"])
    common.create_box(collection, "RF4_MotorboatCabin", (22.0, -11.0, 1.42), (0.78, 0.58, 0.42), math.radians(8.0), materials["white"])
    common.create_box(collection, "RF4_MotorboatMotor", (19.0, -11.6, 0.92), (0.38, 0.45, 0.52), math.radians(8.0), materials["dark"])


def build_scene(collection, materials):
    world3.create_night_sky(collection, materials["sky"])
    sky = bpy.data.objects.get("RF3_MoonlessSky")
    if sky is not None:
        sky.name = "RF4_DawnSky"

    world3.create_polygon_prism(
        collection,
        "RF4_Ocean",
        [(-42.0, -23.0), (42.0, -23.0), (42.0, 34.0), (-42.0, 34.0)],
        -1.2,
        -0.15,
        materials["sea"],
    )
    world3.create_polygon_prism(
        collection,
        "RF4_SandIsland",
        [(-27.0, -16.0), (-19.0, -21.0), (11.0, -21.0), (27.0, -13.0), (25.0, 4.0), (17.0, 15.0), (8.0, 23.0), (-9.0, 24.0), (-23.0, 14.0), (-29.0, 0.0)],
        -0.9,
        0.35,
        materials["sand"],
    )
    forest_boundary = [
        (-18.0, -0.4), (-14.0, -1.5), (-9.0, -0.7), (-3.0, -1.8),
        (3.0, -0.8), (9.0, -1.6), (17.0, -0.3),
        (19.0, 12.0), (9.0, 22.0), (-10.0, 22.0), (-20.0, 12.0),
    ]
    world3.create_polygon_prism(collection, "RF4_ForestRockBase", forest_boundary, 0.2, 1.92, materials["rock"])
    world3.create_polygon_prism(collection, "RF4_ForestGrassCap", forest_boundary, 1.92, 2.0, materials["grass"])
    world3.create_polygon_prism(
        collection,
        "RF4_FinalClearing",
        [(-9.0, 9.0), (8.0, 9.0), (11.0, 18.0), (6.0, 23.0), (-7.0, 23.0), (-11.0, 17.0)],
        1.8,
        3.1,
        materials["clearing"],
    )

    main_route = [(x, y, z + 0.06) for _, x, y, z in PORTALS[0:9]]
    world3.create_path_ribbon(collection, "RF4_FinalTrail", main_route, 1.25, materials["path"])
    for index in range(len(main_route) - 1):
        world3.create_stair_run(collection, "RF4_TrailStep_%02d_" % index, main_route[index], main_route[index + 1], materials["wood"])

    for index, (destination, x, y, z) in enumerate(PORTALS):
        ring_material = materials["portal"]
        if destination in ("select3", "base"):
            ring_material = materials["blue"]
        common.create_cylinder(collection, "RF4_PortalBase_%02d" % index, (x, y, z - 0.18), 1.40, 0.38, 14, materials["rock"])
        common.create_cylinder(collection, "RF4_PortalInset_%02d" % index, (x, y, z + 0.01), 1.02, 0.10, 14, materials["dark"])
        common.create_torus(collection, "RF4_PortalRing_%02d" % index, (x, y, z + 0.13), 1.12, 0.09, ring_material)

    foam_lines = [
        [(-27.0, -15.7, 0.39), (-18.0, -20.4, 0.39), (-3.0, -21.2, 0.39), (11.0, -20.4, 0.39), (25.5, -12.8, 0.39)],
        [(-29.5, -17.4, -0.02), (-16.0, -22.2, -0.02), (4.0, -22.8, -0.02), (26.5, -15.0, -0.02)],
    ]
    for index, points in enumerate(foam_lines):
        world3.create_path_ribbon(collection, "RF4_WaveFoam_%02d" % index, points, 0.22, materials["foam"])

    create_campfire(collection, (-1.5, -3.2, 0.72), materials)
    create_motorboat(collection, materials)
    common.create_box(collection, "RF4_PierMain", (18.0, -8.0, 0.30), (1.45, 6.0, 0.18), 0.0, materials["wood"])
    for index in range(5):
        common.create_box(collection, "RF4_PierPostL_%02d" % index, (16.7, -12.0 + index * 2.0, 0.15), (0.17, 0.17, 0.85), 0.0, materials["wood"])
        common.create_box(collection, "RF4_PierPostR_%02d" % index, (19.3, -12.0 + index * 2.0, 0.15), (0.17, 0.17, 0.85), 0.0, materials["wood"])

    trees = [
        (-21.0, 1.0, 0.4, 1.3), (-18.0, 8.0, 1.9, 1.5), (-15.0, 16.0, 2.1, 1.4),
        (-9.0, 4.0, 2.0, 1.1), (12.0, 3.0, 2.0, 1.2), (17.0, 9.0, 1.8, 1.5),
        (13.0, 17.0, 2.1, 1.35), (-5.0, 21.0, 3.1, 1.0), (6.0, 21.0, 3.1, 1.0),
    ]
    for index, (x, y, z, scale) in enumerate(trees):
        create_tree(collection, "RF4_Tree_%02d_" % index, (x, y, z), scale, materials["wood"], materials["leaves"], 500 + index * 4)

    rocks = [
        (-24.0, -9.0, 0.5, 2.8), (-21.0, 18.0, 2.2, 3.2),
        (20.0, 18.0, 2.2, 3.0), (25.0, -3.0, 0.6, 2.5),
        (-12.0, 24.0, 2.5, 3.3), (12.0, 25.0, 2.5, 3.2),
        (-15.0, -1.3, 1.25, 2.2), (-9.0, -1.5, 1.10, 1.8),
        (4.0, -1.4, 1.15, 2.0), (14.0, -1.2, 1.25, 2.2),
    ]
    for index, (x, y, z, scale) in enumerate(rocks):
        common.create_rock(collection, "RF4_CoastRock_%02d" % index, (x, y, z), (scale, scale * 0.8, scale * 1.3), index * 0.48, materials["rock"], 560 + index)

    common.create_rock(collection, "RF4_DawnSun", (18.0, 32.0, 11.5), (2.7, 1.2, 2.7), 0.0, materials["sun"], 600)
    common.create_box(collection, "RF4_FallenCrossVertical", (-2.0, 16.5, 3.55), (0.20, 1.10, 0.18), math.radians(22.0), materials["gold"])
    common.create_box(collection, "RF4_FallenCrossHorizontal", (-1.7, 16.9, 3.72), (0.65, 0.18, 0.16), math.radians(22.0), materials["gold"])

    spirit_locations = [(-8.0, -4.0, 2.3), (11.0, 7.0, 4.4), (-6.0, 15.0, 5.2), (5.0, 18.0, 5.4)]
    for index, location in enumerate(spirit_locations):
        common.create_rock(collection, "RF4_WhiteSpirit_%02d" % index, location, (0.34, 0.34, 0.58), index * 0.8, materials["spirit"], 620 + index)
    mist_locations = [(-1.0, 8.0, 3.1), (-5.0, 13.0, 4.1), (3.0, 16.0, 4.3)]
    for index, location in enumerate(mist_locations):
        common.create_rock(collection, "RF4_RemainingMist_%02d" % index, location, (1.7, 1.1, 0.55), index * 0.7, materials["mist"], 640 + index)


def render_preview(preview_path):
    camera_data = bpy.data.cameras.new("RF4_PreviewCameraData")
    camera = bpy.data.objects.new("RF4_PreviewCamera", camera_data)
    bpy.context.scene.collection.objects.link(camera)
    camera.location = (0.0, -38.0, 20.0)
    camera.data.lens = 47.0
    common.aim_object(camera, (0.0, 5.0, 2.3))
    bpy.context.scene.camera = camera

    dawn = common.add_preview_light("RF4_DawnLight", "AREA", (18.0, 6.0, 25.0), 3200.0, (1.0, 0.42, 0.22), 18.0)
    common.aim_object(dawn, (0.0, 5.0, 1.5))
    fill = common.add_preview_light("RF4_SeaFill", "AREA", (-18.0, -14.0, 15.0), 1900.0, (0.20, 0.40, 0.85), 16.0)
    common.aim_object(fill, (0.0, 0.0, 1.0))
    fire = common.add_preview_light("RF4_FireLight", "POINT", (-1.5, -3.2, 2.5), 850.0, (1.0, 0.18, 0.03), 3.0)
    fire.data.shadow_soft_size = 2.5

    scene = bpy.context.scene
    scene.render.engine = "BLENDER_EEVEE"
    scene.render.resolution_x = 1200
    scene.render.resolution_y = 675
    scene.render.resolution_percentage = 100
    scene.render.image_settings.file_format = "PNG"
    scene.render.filepath = preview_path
    scene.world.color = (0.025, 0.018, 0.045)
    os.makedirs(os.path.dirname(preview_path), exist_ok=True)
    bpy.ops.render.render(write_still=True)


def main():
    output_x_path, preview_path = get_arguments()
    collection = clear_scene()
    materials = {
        "sky": create_material("RF2_CaveSky_World4Dawn", "../SkySphere_evening/Skydome.png", (0.22, 0.08, 0.14, 1.0), 0.70),
        "sea": create_material("RF2_CaveSky_World4Sea", "../stage-select1/stageSelectSea.png", (0.02, 0.12, 0.30, 1.0), 0.20),
        "sand": create_material("RF4_DawnSand", "../stage-select1/stageSelectSand.png", (0.48, 0.30, 0.18, 1.0)),
        "grass": create_material("RF4_DarkGrass", "../stage-select1/stageSelectGrass.png", (0.05, 0.18, 0.10, 1.0)),
        "clearing": create_material("RF4_FinalClearingMat", "../stage-select2/stageSelectCavePath.png", (0.16, 0.15, 0.16, 1.0)),
        "path": create_material("RF4_ForestPath", "../stage-select2/stageSelectCavePath.png", (0.25, 0.16, 0.10, 1.0)),
        "rock": create_material("RF4_CoastRockMat", "../stage-select1/stageSelectRock.png", (0.18, 0.20, 0.25, 1.0)),
        "wood": create_material("RF4_DarkWood", "../cubeWood/wood.png", (0.19, 0.10, 0.055, 1.0)),
        "leaves": create_material("RF4_ForestLeaves", "../stage-select1/stageSelectPalmLeaf.png", (0.025, 0.13, 0.07, 1.0)),
        "boat": create_material("RF4_BoatHull", "../cube_red.png", (0.35, 0.025, 0.02, 1.0)),
        "dark": create_material("RF4_DeepShadow", "../stage-select2/stageSelectCaveDeepDark.png", (0.025, 0.018, 0.035, 1.0)),
        "white": create_material("RF4_BoatWhite", "../cube_white.png", (0.75, 0.78, 0.82, 1.0)),
        "foam": create_material("RF2_CaveSky_World4Foam", "../stage-select1/stageSelectWaveFoam.png", (0.65, 0.82, 1.0, 1.0), 0.40),
        "fire": create_material("RF2_CaveSky_World4Fire", "../sphereOrange/sphere_orange.png", (1.0, 0.10, 0.01, 1.0), 2.0),
        "gold": create_material("RF2_CaveSky_World4Gold", "../sphereOrange/sphere_orange.png", (1.0, 0.40, 0.03, 1.0), 1.4),
        "sun": create_material("RF2_CaveSky_World4Sun", "../sphereOrange/sphere_orange.png", (1.0, 0.22, 0.04, 1.0), 2.2),
        "spirit": create_material("RF2_CaveSky_World4Spirit", "../cube_white.png", (0.62, 0.86, 1.0, 1.0), 1.3),
        "mist": create_material("RF4_BlackMist", "../stage-select2/stageSelectCaveDeepDark.png", (0.025, 0.01, 0.04, 1.0)),
        "portal": create_material("RF4_PortalStone", "../stage-select2/stageSelectCaveShrine.png", (0.38, 0.31, 0.28, 1.0)),
        "blue": create_material("RF2_CaveSky_World4Travel", "../cubeBlue/cube_blue.png", (0.03, 0.25, 1.0, 1.0), 0.8),
    }
    build_scene(collection, materials)
    common.export_directx_meshes(bpy.context.scene.collection, output_x_path)
    render_preview(preview_path)
    bpy.ops.wm.save_as_mainfile(filepath=bpy.data.filepath)
    print("SAVED_BLEND", bpy.data.filepath)
    print("PREVIEW", preview_path)


if __name__ == "__main__":
    main()
