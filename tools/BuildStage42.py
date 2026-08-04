"""Build stage 4-2 (stage_4_2) with Blender's official DirectX X exporter."""

import csv
import math
import random
from pathlib import Path

import bpy


REPO_ROOT = Path(__file__).resolve().parents[1]
MODEL_DIR = REPO_ROOT / "RedFortress2" / "MultiPassRendering" / "res" / "model"
STAGE_DIR = MODEL_DIR / "stage_4_2"
GROUND_TEXTURE = MODEL_DIR / "ground" / "tex" / "world4.png"
SIDE_TEXTURE = MODEL_DIR / "whiteWall.png"

START_POSITION = (0.0, 0.2, -108.0)
GOAL_POSITION = (0.0, 1.0, 108.0)

RENDER_HEADER = (
    "ID", "FileName", "PosX", "PosY", "PosZ",
    "RotX", "RotY", "RotZ", "Scale", "loadType", "PlacementCsv",
)
PHYSICS_HEADER = (
    "ID", "FileName", "PosX", "PosY", "PosZ",
    "RotX", "RotY", "RotZ", "Scale", "Type", "Move", "Instancing",
)
MOVE_HEADER = (
    "ID", "RenderID", "PhysicsID", "PosX", "PosY", "PosZ",
    "RotX", "RotY", "RotZ", "Scale",
    "StartX", "StartY", "StartZ", "EndX", "EndY", "EndZ", "Duration",
)

GROUND_RECTANGLES = (
    (0.0, -108.0, 36.0, 24.0),
    (0.0, 0.0, 30.0, 24.0),
    (0.0, 108.0, 36.0, 24.0),
)

SHARED_DECKS = (
    (0.0, -72.0),
    (18.0, -40.0),
    (0.0, 36.0),
    (-18.0, 72.0),
)

FIXED_ROUTE_SEGMENTS = (
    ((-10.0, -91.0), (-16.0, -84.0), (-10.0, -77.0)),
    ((8.0, -64.0), (14.0, -56.0), (20.0, -49.0)),
    ((27.0, -31.0), (24.0, -22.0), (17.0, -15.0)),
    ((-10.0, 15.0), (-16.0, 22.0), (-10.0, 29.0)),
    ((-8.0, 46.0), (-15.0, 55.0), (-22.0, 62.0)),
    ((-27.0, 81.0), (-22.0, 88.0), (-13.0, 94.0)),
)

MOVING_PLATFORMS = (
    ((0.0, 0.4, -91.0), (0.0, 0.4, -80.0), 7.0),
    ((5.0, 0.4, -64.0), (14.0, 1.2, -48.0), 8.0),
    ((12.0, 0.4, -32.0), (3.0, 0.4, -14.0), 9.0),
    ((0.0, 0.4, 14.0), (0.0, 2.0, 28.0), 7.0),
    ((-5.0, 0.4, 44.0), (-14.0, 1.2, 64.0), 8.0),
    ((-12.0, 0.4, 80.0), (-3.0, 1.0, 94.0), 9.0),
)

ENEMIES = (
    ("mushroom", -3.0, 0.2, -72.0, 0.0),
    ("mushroom", 3.0, 0.2, -72.0, 180.0),
    ("golem", 0.0, 0.2, -68.0, 180.0),
    ("mushroom", 15.0, 0.2, -43.0, 0.0),
    ("mushroom", 21.0, 0.2, -43.0, 180.0),
    ("golem", 15.0, 0.2, -37.0, 0.0),
    ("golem", 21.0, 0.2, -37.0, 180.0),
    ("mushroom", -6.0, 0.2, -4.0, 0.0),
    ("mushroom", 6.0, 0.2, -4.0, 180.0),
    ("golem", -6.0, 0.2, 4.0, 0.0),
    ("golem", 6.0, 0.2, 4.0, 180.0),
    ("enemy2", 0.0, 0.2, 7.0, 180.0),
    ("mushroom", -3.0, 0.2, 33.0, 0.0),
    ("mushroom", 3.0, 0.2, 33.0, 180.0),
    ("golem", -3.0, 0.2, 39.0, 0.0),
    ("enemy2", 3.0, 0.2, 39.0, 180.0),
    ("mushroom", -21.0, 0.2, 69.0, 0.0),
    ("mushroom", -15.0, 0.2, 69.0, 180.0),
    ("golem", -21.0, 0.2, 75.0, 0.0),
    ("enemy2", -15.0, 0.2, 75.0, 180.0),
)

COLLECTIBLES = (
    ("stage14-I01", "Item", "001", -16.0, 0.7, -84.0, 1.0),
    ("stage14-I02", "Item", "010", 27.0, 0.7, -31.0, 1.0),
    ("stage14-I03", "Item", "014", -16.0, 0.7, 22.0, 1.0),
    ("stage14-I04", "Item", "005", -27.0, 0.7, 81.0, 1.0),
)

ANCHOR_RECTANGLES = (
    (-18.0, 18.0, -120.0, -96.0),
    (-6.0, 6.0, -78.0, -66.0),
    (12.0, 24.0, -46.0, -34.0),
    (-15.0, 15.0, -12.0, 12.0),
    (-6.0, 6.0, 30.0, 42.0),
    (-24.0, -12.0, 66.0, 78.0),
    (-18.0, 18.0, 96.0, 120.0),
)


def write_csv(path, header, rows):
    with path.open("w", encoding="utf-8-sig", newline="") as output:
        writer = csv.writer(output, lineterminator="\r\n")
        writer.writerow(header)
        writer.writerows(rows)


def write_placement_csv(path, settings, rows, include_header):
    with path.open("w", encoding="utf-8-sig", newline="") as output:
        writer = csv.writer(output, lineterminator="\r\n")
        for setting in settings:
            writer.writerow(setting)
        if include_header:
            writer.writerow(("#x", "y", "z", "RotY", "Scale"))
        writer.writerows(rows)


def normalize_x_file(path):
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n")
    data = data.replace(b"\n", b"\r\n")
    path.write_bytes(data)
    if not data.startswith(b"xof "):
        raise RuntimeError("Invalid DirectX X header: " + str(path))


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials, bpy.data.images):
        for data_block in list(collection):
            collection.remove(data_block)


def create_material(name, texture_path, texture_filename, color):
    if not texture_path.exists():
        raise RuntimeError("Missing texture: " + str(texture_path))
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = color
    material.roughness = 1.0
    material.metallic = 0.0
    material["_x_power"] = 0.0
    material["_x_specular"] = (0.0, 0.0, 0.0)
    material["_x_emissive"] = (0.0, 0.0, 0.0)
    material["_x_texture_filename"] = texture_filename
    principled = next(
        node for node in material.node_tree.nodes
        if node.type == "BSDF_PRINCIPLED"
    )
    principled.inputs["Base Color"].default_value = color
    principled.inputs["Roughness"].default_value = 1.0
    image = bpy.data.images.load(str(texture_path), check_existing=True)
    texture = material.node_tree.nodes.new("ShaderNodeTexImage")
    texture.image = image
    texture.extension = "REPEAT"
    material.node_tree.links.new(texture.outputs["Color"], principled.inputs["Base Color"])
    return material


def add_quad(vertices, faces, uvs, material_indices, coordinates, quad_uvs, material_index):
    start = len(vertices)
    vertices.extend(coordinates)
    uvs.extend(quad_uvs)
    faces.append((start, start + 1, start + 2, start + 3))
    material_indices.append(material_index)


def add_box(vertices, faces, uvs, material_indices, center_x, center_z, width, depth):
    x_min = center_x - width * 0.5
    x_max = center_x + width * 0.5
    z_min = center_z - depth * 0.5
    z_max = center_z + depth * 0.5
    top_y = 0.0
    bottom_y = -2.5
    add_quad(
        vertices, faces, uvs, material_indices,
        ((x_min, z_min, top_y), (x_max, z_min, top_y),
         (x_max, z_max, top_y), (x_min, z_max, top_y)),
        ((x_min / 8.0, z_min / 8.0), (x_max / 8.0, z_min / 8.0),
         (x_max / 8.0, z_max / 8.0), (x_min / 8.0, z_max / 8.0)),
        0,
    )
    side_uv = ((0.0, 0.0), (width / 4.0, 0.0),
               (width / 4.0, 0.625), (0.0, 0.625))
    add_quad(
        vertices, faces, uvs, material_indices,
        ((x_min, z_min, bottom_y), (x_max, z_min, bottom_y),
         (x_max, z_min, top_y), (x_min, z_min, top_y)),
        side_uv, 1,
    )
    add_quad(
        vertices, faces, uvs, material_indices,
        ((x_max, z_max, bottom_y), (x_min, z_max, bottom_y),
         (x_min, z_max, top_y), (x_max, z_max, top_y)),
        side_uv, 1,
    )
    depth_uv = ((0.0, 0.0), (depth / 4.0, 0.0),
                (depth / 4.0, 0.625), (0.0, 0.625))
    add_quad(
        vertices, faces, uvs, material_indices,
        ((x_min, z_max, bottom_y), (x_min, z_min, bottom_y),
         (x_min, z_min, top_y), (x_min, z_max, top_y)),
        depth_uv, 1,
    )
    add_quad(
        vertices, faces, uvs, material_indices,
        ((x_max, z_min, bottom_y), (x_max, z_max, bottom_y),
         (x_max, z_max, top_y), (x_max, z_min, top_y)),
        depth_uv, 1,
    )


def build_ground_model():
    clear_scene()
    top_material = create_material(
        "Stage42GroundTop", GROUND_TEXTURE, "../ground/tex/world4.png",
        (0.48, 0.40, 0.38, 1.0),
    )
    side_material = create_material(
        "Stage42GroundSide", SIDE_TEXTURE, "../whiteWall.png",
        (0.25, 0.20, 0.20, 1.0),
    )
    vertices = []
    faces = []
    uvs = []
    material_indices = []
    for center_x, center_z, width, depth in GROUND_RECTANGLES:
        add_box(
            vertices, faces, uvs, material_indices,
            center_x, center_z, width, depth,
        )
    mesh = bpy.data.meshes.new("Stage42GroundGeo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(top_material)
    mesh.materials.append(side_material)
    for polygon_index, polygon in enumerate(mesh.polygons):
        polygon.use_smooth = False
        polygon.material_index = material_indices[polygon_index]
    uv_layer = mesh.uv_layers.new(name="UVMap")
    for polygon in mesh.polygons:
        for loop_index in polygon.loop_indices:
            vertex_index = mesh.loops[loop_index].vertex_index
            uv_layer.data[loop_index].uv = uvs[vertex_index]
    ground = bpy.data.objects.new("Stage42Ground", mesh)
    bpy.context.collection.objects.link(ground)
    ground["_x_frame_name"] = "Stage42Ground"
    ground["_x_mesh_name"] = "Stage42GroundGeo"
    blend_path = STAGE_DIR / "stage_ground.blend"
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))
    bpy.ops.object.select_all(action="DESELECT")
    ground.select_set(True)
    bpy.context.view_layer.objects.active = ground
    output_path = STAGE_DIR / "stage_ground.x"
    result = bpy.ops.export_scene.directx_x(
        filepath=str(output_path),
        check_existing=False,
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=True,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        export_format="TEXT_X",
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(output_path))
    normalize_x_file(output_path)


def append_static_platform(render_rows, physics_rows, csv_id, x, z):
    render_rows.append((
        csv_id, "../static_platform/static_platform_2x2.x",
        x, 0.0, z, 0, 0, 0, 1, "meshmix2", "",
    ))
    physics_rows.append((
        csv_id, "res/model/static_platform/static_platform_2x2.x",
        x, 0.0, z, 0, 0, 0, 1, "Collision", "n", "",
    ))


def append_fences(render_rows):
    csv_id = 8000
    x = -56.0
    while x <= 56.0:
        render_rows.append((csv_id, "../fence.x", x, 0.5, -120.0, 0, 0, 0, 1, "normal", ""))
        csv_id += 1
        render_rows.append((csv_id, "../fence.x", x, 0.5, 120.0, 0, 0, 0, 1, "normal", ""))
        csv_id += 1
        x += 8.0
    z = -116.0
    while z <= 116.0:
        render_rows.append((csv_id, "../fence.x", -60.0, 0.5, z, 0, 90, 0, 1, "normal", ""))
        csv_id += 1
        render_rows.append((csv_id, "../fence.x", 60.0, 0.5, z, 0, 90, 0, 1, "normal", ""))
        csv_id += 1
        z += 8.0


def make_exterior_positions(count, seed, scale_min, scale_max, minimum_distance):
    generator = random.Random(seed)
    positions = []
    attempts = 0
    maximum_attempts = count * 200
    minimum_distance_squared = minimum_distance * minimum_distance
    while len(positions) < count and attempts < maximum_attempts:
        attempts += 1
        side = generator.randrange(4)
        if side == 0:
            x = generator.uniform(-96.0, -64.0)
            z = generator.uniform(-154.0, 154.0)
        elif side == 1:
            x = generator.uniform(64.0, 96.0)
            z = generator.uniform(-154.0, 154.0)
        elif side == 2:
            x = generator.uniform(-63.0, 63.0)
            z = generator.uniform(-154.0, -124.0)
        else:
            x = generator.uniform(-63.0, 63.0)
            z = generator.uniform(124.0, 154.0)
        valid = True
        for existing in positions:
            delta_x = x - existing[0]
            delta_z = z - existing[2]
            if delta_x * delta_x + delta_z * delta_z < minimum_distance_squared:
                valid = False
                break
        if not valid:
            continue
        rotation = generator.uniform(0.0, 360.0)
        scale = generator.uniform(scale_min, scale_max)
        positions.append((round(x, 2), 0.0, round(z, 2), round(rotation, 2), round(scale, 2)))
    if len(positions) != count:
        raise RuntimeError("Could not create exterior placements")
    return positions


def rectangle_gap(first, second):
    delta_x = 0.0
    if first[1] < second[0]:
        delta_x = second[0] - first[1]
    elif second[1] < first[0]:
        delta_x = first[0] - second[1]
    delta_z = 0.0
    if first[3] < second[2]:
        delta_z = second[2] - first[3]
    elif second[3] < first[2]:
        delta_z = first[2] - second[3]
    return math.sqrt(delta_x * delta_x + delta_z * delta_z)


def platform_rectangle(center):
    return (center[0] - 3.0, center[0] + 3.0, center[1] - 3.0, center[1] + 3.0)


def moving_rectangle(position):
    return (position[0] - 1.5, position[0] + 1.5, position[2] - 1.5, position[2] + 1.5)


def validate_layout():
    if len(ENEMIES) != 20:
        raise RuntimeError("Stage 4-2 must contain 20 enemies")
    enemy_types = {}
    for enemy in ENEMIES:
        enemy_types[enemy[0]] = enemy_types.get(enemy[0], 0) + 1
        for protected in (START_POSITION, GOAL_POSITION):
            delta_x = enemy[1] - protected[0]
            delta_z = enemy[3] - protected[2]
            if delta_x * delta_x + delta_z * delta_z < 49.0:
                raise RuntimeError("Enemy is too close to start or goal")
    expected_types = {"mushroom": 10, "golem": 7, "enemy2": 3}
    if enemy_types != expected_types:
        raise RuntimeError("Unexpected enemy distribution")
    for segment_index, segment in enumerate(FIXED_ROUTE_SEGMENTS):
        rectangles = [ANCHOR_RECTANGLES[segment_index]]
        rectangles.extend(platform_rectangle(center) for center in segment)
        rectangles.append(ANCHOR_RECTANGLES[segment_index + 1])
        for index in range(len(rectangles) - 1):
            if rectangle_gap(rectangles[index], rectangles[index + 1]) > 4.1:
                raise RuntimeError("Fixed route has an oversized jump")
    for index, moving in enumerate(MOVING_PLATFORMS):
        start_rectangle = moving_rectangle(moving[0])
        end_rectangle = moving_rectangle(moving[1])
        if rectangle_gap(ANCHOR_RECTANGLES[index], start_rectangle) > 4.1:
            raise RuntimeError("Moving route start is unreachable")
        if rectangle_gap(end_rectangle, ANCHOR_RECTANGLES[index + 1]) > 4.1:
            raise RuntimeError("Moving route end is unreachable")


def build_csv_files():
    render_rows = [
        (1, "../ground/stage_visual_ground_world4.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2", ""),
        (2, "stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "meshmix2", ""),
    ]
    physics_rows = [
        (1, "res/model/cubeNormalInverse120x240.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""),
        (2, "res/model/stage_4_2/stage_ground.x", 0, 0, 0, 0, 0, 0, 1, "Collision", "n", ""),
    ]
    static_id = 3000
    for deck_x, deck_z in SHARED_DECKS:
        for offset_x in (-3.0, 3.0):
            for offset_z in (-3.0, 3.0):
                static_id += 1
                append_static_platform(
                    render_rows, physics_rows, static_id,
                    deck_x + offset_x, deck_z + offset_z,
                )
    for segment in FIXED_ROUTE_SEGMENTS:
        for center_x, center_z in segment:
            static_id += 1
            append_static_platform(render_rows, physics_rows, static_id, center_x, center_z)
    move_rows = []
    for index, moving in enumerate(MOVING_PLATFORMS, start=1):
        start, end, duration = moving
        csv_id = 4000 + index
        render_rows.append((
            csv_id, "../collision_moving_platform/collision_moving_platform.x",
            start[0], start[1], start[2], 0, 0, 0, 1, "meshmix2", "",
        ))
        physics_rows.append((
            csv_id, "res/model/collision_moving_platform.x",
            start[0], start[1], start[2], 0, 0, 0, 1, "Collision", "y", "",
        ))
        move_rows.append((
            index, csv_id, csv_id,
            start[0], start[1], start[2], 0, 0, 0, 1,
            start[0], start[1], start[2], end[0], end[1], end[2], duration,
        ))
    lava_rows = []
    lava_id = 5000
    for z in (-105.0, -75.0, -45.0, -15.0, 15.0, 45.0, 75.0, 105.0):
        for x in (-45.0, -15.0, 15.0, 45.0):
            lava_id += 1
            render_rows.append((
                lava_id, "../plateLava.x", x, -1.0, z,
                0, 0, 0, 3.75, "meshmix2", "",
            ))
            physics_rows.append((
                lava_id, "res/model/plateLava.x", x, -1.0, z,
                0, 0, 0, 3.75, "NonCollision", "n", "",
            ))
            lava_rows.append(("stage14-lava-" + str(lava_id - 5000).zfill(2), lava_id, 25))
    render_rows.append((7000, "../SkySphere_night/SkySphere.blend.x", 0, 0.01, 0, 0, 0, 0, 1, "normal", ""))
    append_fences(render_rows)
    grass_positions = make_exterior_positions(1400, 4201, 0.88, 1.16, 0.75)
    tree_positions = make_exterior_positions(140, 4202, 0.90, 1.35, 3.8)
    rock1_positions = make_exterior_positions(90, 4203, 0.65, 2.10, 3.0)
    rock2_positions = make_exterior_positions(70, 4204, 0.65, 2.10, 3.0)
    render_rows.extend((
        (9001, "../grass/grass.x", 0, 0, 0, 0, 0, 0, 1, "instancing", "stage14_grass.csv"),
        (9002, "../tree2/lemonTree.Instancing.x", 0, 0, 0, 0, 0, 0, 1, "instancing", "stage14_trees.csv"),
        (9003, "../base/base_rock1.x", 0, 0, 0, 0, 0, 0, 1, "instancing", "stage14_rocks1.csv"),
        (9004, "../base/base_rock2.x", 0, 0, 0, 0, 0, 0, 1, "instancing", "stage14_rocks2.csv"),
    ))
    write_csv(STAGE_DIR / "XFileList_simple.csv", RENDER_HEADER, render_rows)
    write_csv(STAGE_DIR / "XFileListPhysics.csv", PHYSICS_HEADER, physics_rows)
    write_csv(STAGE_DIR / "XFileListMove.csv", MOVE_HEADER, move_rows)
    write_csv(STAGE_DIR / "EnemyPositions.csv", ("Type", "PosX", "PosY", "PosZ", "RotY"), ENEMIES)
    write_csv(
        STAGE_DIR / "Collectibles.csv",
        ("CollectibleID", "Type", "DataID", "PosX", "PosY", "PosZ", "Scale"),
        COLLECTIBLES,
    )
    write_csv(STAGE_DIR / "LavaZones.csv", ("ID", "PhysicsID", "Damage"), lava_rows)
    write_csv(STAGE_DIR / "Destructibles.csv", ("PosX", "PosY", "PosZ", "HP"), ())
    write_csv(
        STAGE_DIR / "DashBoosters.csv",
        ("DashBoosterID", "PosX", "PosY", "PosZ", "DirX", "DirY", "DirZ",
         "Speed", "Duration", "Radius", "Scale"),
        (),
    )
    write_csv(STAGE_DIR / "SpeedUps.csv", ("PosX", "PosY", "PosZ"), ())
    write_placement_csv(STAGE_DIR / "stage14_grass.csv", (("sway", "wave"), ("AutoHide", "n")), grass_positions, False)
    write_placement_csv(STAGE_DIR / "stage14_trees.csv", (("AutoHide", "n"),), tree_positions, True)
    write_placement_csv(STAGE_DIR / "stage14_rocks1.csv", (("AutoHide", "n"),), rock1_positions, True)
    write_placement_csv(STAGE_DIR / "stage14_rocks2.csv", (("AutoHide", "n"),), rock2_positions, True)


def main():
    STAGE_DIR.mkdir(parents=True, exist_ok=True)
    validate_layout()
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    build_ground_model()
    build_csv_files()
    print("Stage 4-2 generated: stage_4_2")
    print("Enemies:", len(ENEMIES))
    print("Moving platforms:", len(MOVING_PLATFORMS))
    print("Start:", START_POSITION)
    print("Goal:", GOAL_POSITION)


main()
