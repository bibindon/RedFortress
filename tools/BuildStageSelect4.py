import math
import os
import sys

import bpy

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import EnhanceStageSelect2 as common
import BuildStageSelect3 as world3


COLLECTION_NAME = "RF4_DawnFinalIsland"
OCEAN_RADIUS_METERS = 500.0
DAWN_SKY_SCALE = 9.0

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
    for root_index in range(5):
        root_angle = math.tau * root_index / 5.0 + seed * 0.13
        common.create_box(
            collection,
            prefix + "Root%02d" % root_index,
            (
                x + math.cos(root_angle) * 0.48 * scale,
                y + math.sin(root_angle) * 0.48 * scale,
                z + 0.12 * scale,
            ),
            (0.78 * scale, 0.13 * scale, 0.13 * scale),
            root_angle,
            trunk_material,
        )
    for band_index in range(3):
        common.create_cylinder(
            collection,
            prefix + "TrunkBand%02d" % band_index,
            (x, y, z + (0.55 + band_index * 0.72) * scale),
            0.29 * scale,
            0.12 * scale,
            9,
            trunk_material,
        )
    for branch_index in range(3):
        branch_angle = seed * 0.19 + math.tau * branch_index / 3.0
        common.create_box(
            collection,
            prefix + "Branch%02d" % branch_index,
            (
                x + math.cos(branch_angle) * 0.38 * scale,
                y + math.sin(branch_angle) * 0.38 * scale,
                z + (2.15 + branch_index * 0.18) * scale,
            ),
            (0.75 * scale, 0.12 * scale, 0.12 * scale),
            branch_angle,
            trunk_material,
        )
    crowns = [
        (x, y, z + 3.1 * scale, 1.25, 1.05, 1.30),
        (x - 0.65 * scale, y + 0.15 * scale, z + 2.85 * scale, 0.85, 0.75, 0.90),
        (x + 0.60 * scale, y - 0.10 * scale, z + 2.95 * scale, 0.90, 0.80, 0.95),
        (x - 0.22 * scale, y - 0.62 * scale, z + 3.18 * scale, 0.72, 0.68, 0.82),
        (x + 0.18 * scale, y + 0.62 * scale, z + 3.28 * scale, 0.78, 0.70, 0.88),
        (x, y, z + 3.78 * scale, 0.70, 0.62, 0.82),
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


def create_bush(collection, prefix, location, scale, leaf_material, seed):
    x, y, z = location
    specifications = [
        (0.0, 0.0, 0.45, 1.00, 0.85, 0.72),
        (-0.55, 0.08, 0.32, 0.72, 0.62, 0.54),
        (0.52, -0.12, 0.34, 0.76, 0.65, 0.58),
        (0.05, 0.50, 0.30, 0.66, 0.58, 0.52),
    ]
    for index, (offset_x, offset_y, offset_z, scale_x, scale_y, scale_z) in enumerate(specifications):
        common.create_rock(
            collection,
            prefix + "Leaf%02d" % index,
            (x + offset_x * scale, y + offset_y * scale, z + offset_z * scale),
            (scale_x * scale, scale_y * scale, scale_z * scale),
            seed * 0.31 + index * 0.63,
            leaf_material,
            seed + index,
        )


def create_ruin(collection, prefix, location, scale, materials, seed):
    x, y, z = location
    common.create_rock(collection, prefix + "Foundation", (x, y, z + 0.18), (1.65 * scale, 1.25 * scale, 0.34 * scale), seed * 0.17, materials["rock"], seed)
    pillar_offsets = [(-0.72, 0.20, 1.85), (0.62, -0.12, 1.35), (0.08, 0.68, 0.92)]
    for index, (offset_x, offset_y, height) in enumerate(pillar_offsets):
        common.create_cylinder(
            collection,
            prefix + "Pillar%02d" % index,
            (x + offset_x * scale, y + offset_y * scale, z + 0.28 + height * scale * 0.5),
            0.28 * scale,
            height * scale,
            7,
            materials["portal"],
        )
        common.create_box(
            collection,
            prefix + "PillarCap%02d" % index,
            (x + offset_x * scale, y + offset_y * scale, z + 0.31 + height * scale),
            (0.42 * scale, 0.42 * scale, 0.14 * scale),
            seed * 0.11 + index * 0.37,
            materials["portal"],
        )
    for index in range(6):
        angle = math.tau * index / 6.0 + seed * 0.09
        common.create_rock(
            collection,
            prefix + "Rubble%02d" % index,
            (x + math.cos(angle) * 1.35 * scale, y + math.sin(angle) * 1.05 * scale, z + 0.20),
            (0.34 * scale, 0.28 * scale, 0.24 * scale),
            angle,
            materials["rock"],
            seed + 20 + index,
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
        sky.scale = (DAWN_SKY_SCALE, DAWN_SKY_SCALE, DAWN_SKY_SCALE)

    common.create_cylinder(
        collection,
        "RF4_Ocean",
        (0.0, 5.0, -0.23),
        OCEAN_RADIUS_METERS,
        0.16,
        64,
        materials["sea"],
    )
    world3.create_polygon_prism(
        collection,
        "RF4_SandIsland",
        [
            (-29.0, -15.0), (-24.0, -19.5), (-17.0, -22.0), (-7.0, -23.0),
            (4.0, -22.5), (14.0, -20.0), (23.0, -15.5), (28.0, -10.0),
            (27.0, -2.0), (25.0, 5.0), (20.0, 12.0), (14.0, 18.0),
            (7.0, 24.0), (-2.0, 25.0), (-11.0, 23.0), (-19.0, 18.0),
            (-25.0, 11.0), (-29.0, 2.0),
        ],
        -0.9,
        0.35,
        materials["sand"],
    )
    world3.create_polygon_prism(
        collection,
        "RF4_ForestRise",
        [
            (-19.0, -1.5), (-8.0, -0.7), (3.0, -1.3), (17.5, -0.8),
            (19.5, 5.0), (18.5, 12.5), (13.0, 18.0), (8.0, 22.5),
            (-1.0, 23.0), (-10.5, 22.0), (-17.0, 17.0), (-20.5, 11.0),
            (-21.0, 4.0),
        ],
        0.2,
        2.0,
        materials["grass"],
    )
    world3.create_polygon_prism(
        collection,
        "RF4_FinalClearing",
        [
            (-9.5, 9.0), (-2.0, 8.6), (4.5, 9.2), (8.5, 9.0),
            (10.5, 13.0), (11.2, 18.0), (7.0, 23.0), (1.0, 24.0),
            (-7.5, 23.0), (-10.5, 19.0), (-11.5, 14.0),
        ],
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
        (-22.0, 13.0, 1.2, 1.15), (-12.0, 21.0, 2.4, 1.10), (20.0, 4.0, 0.8, 1.05),
    ]
    for index, (x, y, z, scale) in enumerate(trees):
        create_tree(collection, "RF4_Tree_%02d_" % index, (x, y, z), scale, materials["wood"], materials["leaves"], 500 + index * 4)

    rocks = [
        (-24.0, -9.0, 0.5, 2.8), (-21.0, 18.0, 2.2, 3.2), (20.0, 18.0, 2.2, 3.0),
        (25.0, -3.0, 0.6, 2.5), (-12.0, 24.0, 2.5, 3.3), (12.0, 25.0, 2.5, 3.2),
        (-27.0, 6.0, 0.3, 1.8), (-20.0, -16.5, 0.0, 1.5), (5.0, -19.5, 0.0, 1.7),
        (25.0, -11.0, 0.2, 1.6), (25.0, 10.0, 0.8, 1.9), (-25.0, 14.0, 0.9, 1.7),
    ]
    for index, (x, y, z, scale) in enumerate(rocks):
        common.create_rock(collection, "RF4_CoastRock_%02d" % index, (x, y, z), (scale, scale * 0.8, scale * 1.3), index * 0.48, materials["rock"], 560 + index)

    bush_locations = [
        (-19.0, 4.0, 1.7, 0.85), (-16.0, 12.0, 2.0, 0.78), (-11.5, 7.5, 2.0, 0.72),
        (-11.0, 18.0, 2.8, 0.68), (-4.0, 12.0, 2.9, 0.62), (4.0, 12.5, 2.9, 0.65),
        (10.0, 8.0, 2.2, 0.76), (15.0, 13.0, 2.0, 0.82), (18.0, 3.5, 1.8, 0.75),
        (-23.0, -4.0, 0.3, 0.70), (22.0, -2.0, 0.4, 0.68), (-14.0, 23.0, 2.4, 0.74),
    ]
    for index, (x, y, z, scale) in enumerate(bush_locations):
        create_bush(collection, "RF4_Bush_%02d_" % index, (x, y, z), scale, materials["leaves"], 700 + index * 5)

    ruin_locations = [
        (-9.5, 14.5, 3.0, 0.90), (9.5, 15.0, 3.0, 0.88),
        (-17.5, 20.0, 2.2, 0.82), (17.5, 20.0, 2.2, 0.84),
    ]
    for index, (x, y, z, scale) in enumerate(ruin_locations):
        create_ruin(collection, "RF4_Ruin_%02d_" % index, (x, y, z), scale, materials, 800 + index * 11)

    for index in range(24):
        angle = math.tau * index / 24.0 + 0.08
        radius_x = 27.0 + 1.4 * math.sin(index * 1.73)
        radius_y = 19.0 + 1.2 * math.cos(index * 1.31)
        x = math.cos(angle) * radius_x
        y = 1.0 + math.sin(angle) * radius_y
        scale = 0.38 + 0.18 * (1.0 + math.sin(index * 2.17))
        common.create_rock(
            collection,
            "RF4_ShoreStone_%02d" % index,
            (x, y, -0.05 + scale * 0.35),
            (scale, scale * 0.72, scale * 0.58),
            angle,
            materials["rock"],
            900 + index,
        )

    fallen_logs = [
        (-22.0, -13.0, 0.15, 0.25), (-13.0, -18.0, 0.10, -0.45),
        (13.0, -16.5, 0.10, 0.55), (23.0, 2.0, 0.45, -0.18),
        (-20.0, 17.0, 2.10, 0.72), (19.0, 16.0, 2.05, -0.62),
    ]
    for index, (x, y, z, rotation) in enumerate(fallen_logs):
        common.create_box(
            collection,
            "RF4_FallenLog_%02d" % index,
            (x, y, z),
            (1.55, 0.18, 0.16),
            rotation,
            materials["wood"],
        )
        common.create_cylinder(
            collection,
            "RF4_FallenLogCut_%02d" % index,
            (x + math.cos(rotation) * 0.78, y + math.sin(rotation) * 0.78, z),
            0.24,
            0.12,
            9,
            materials["wood"],
        )

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
        "fire": create_material("RF2_CaveSky_World4Fire", "../sphereOrange/sphere_orange.png", (1.0, 0.10, 0.01, 1.0), 2.0),
        "gold": create_material("RF2_CaveSky_World4Gold", "../sphereOrange/sphere_orange.png", (1.0, 0.40, 0.03, 1.0), 1.4),
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
