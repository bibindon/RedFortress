from pathlib import Path
import math

import bpy


BASE_DIR = Path(__file__).resolve().parent
SOURCE_DIR = BASE_DIR / "source_quaternius"
GROUND_BLEND_PATH = BASE_DIR / "base_ground.blend"
GROUND_X_PATH = BASE_DIR / "base_ground.x"
DECOR_BLEND_PATH = BASE_DIR / "base_decor.blend"
DECOR_X_PATH = BASE_DIR / "base_decor.x"
COLLISION_BLEND_PATH = BASE_DIR / "base_decor_collision.blend"
COLLISION_X_PATH = BASE_DIR / "base_decor_collision.x"

GROUND_HALF_WIDTH = 16.0
GROUND_HALF_DEPTH = 32.0
GROUND_STEP = 1.0
GROUND_BOTTOM = -3.0


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (
        bpy.data.meshes,
        bpy.data.curves,
        bpy.data.materials,
        bpy.data.cameras,
        bpy.data.lights,
    ):
        for data_block in list(collection):
            collection.remove(data_block)


def create_material(name, color, roughness=0.7, metallic=0.0, emissive=None):
    material = bpy.data.materials.new(name)
    material.use_nodes = True
    material.diffuse_color = color
    material.roughness = roughness
    material.metallic = metallic
    material["_x_power"] = 24.0
    material["_x_specular"] = (0.08, 0.08, 0.08)
    material["_x_emissive"] = (0.0, 0.0, 0.0)

    principled = next(
        node
        for node in material.node_tree.nodes
        if node.type == "BSDF_PRINCIPLED"
    )
    principled.inputs["Base Color"].default_value = color
    principled.inputs["Roughness"].default_value = roughness
    principled.inputs["Metallic"].default_value = metallic
    principled.inputs["Alpha"].default_value = color[3]
    if emissive is not None:
        material["_x_emissive"] = emissive
        principled.inputs["Emission Color"].default_value = (
            emissive[0],
            emissive[1],
            emissive[2],
            1.0,
        )
        principled.inputs["Emission Strength"].default_value = 1.4
    return material


def blend_value(current, target, weight):
    weight = max(0.0, min(1.0, weight))
    smooth = weight * weight * (3.0 - (2.0 * weight))
    return (current * (1.0 - smooth)) + (target * smooth)


def radial_weight(x, y, center_x, center_y, inner_radius, outer_radius):
    distance = math.hypot(x - center_x, y - center_y)
    if distance <= inner_radius:
        return 1.0
    if distance >= outer_radius:
        return 0.0
    return 1.0 - ((distance - inner_radius) / (outer_radius - inner_radius))


def terrain_height(x, y):
    height = (
        0.08 * math.sin((x + 3.0) * 0.35)
        + 0.06 * math.sin((y - 4.0) * 0.22)
    )
    height += 0.85 * math.exp(
        -(((x + 12.5) ** 2) / 30.0) - (((y - 13.0) ** 2) / 80.0)
    )
    height += 0.65 * math.exp(
        -(((x - 12.0) ** 2) / 28.0) - (((y - 15.0) ** 2) / 70.0)
    )
    height += 0.38 * math.exp(
        -(((x + 13.0) ** 2) / 24.0) - (((y + 10.0) ** 2) / 90.0)
    )

    pond_distance = math.sqrt(((x / 9.2) ** 2) + (((y - 2.0) / 4.2) ** 2))
    if pond_distance < 0.82:
        height = -0.42
    elif pond_distance < 1.12:
        pond_weight = 1.0 - ((pond_distance - 0.82) / 0.30)
        height = blend_value(height, -0.42, pond_weight)

    height = blend_value(
        height,
        0.0,
        radial_weight(x, y, 0.0, -28.0, 4.0, 7.0),
    )
    height = blend_value(
        height,
        0.22,
        radial_weight(x, y, -7.0, -20.0, 4.2, 6.5),
    )
    height = blend_value(
        height,
        0.05,
        radial_weight(x, y, 0.0, 26.0, 4.0, 7.0),
    )

    east_path_weight = radial_weight(x, 0.0, 10.5, 0.0, 2.0, 4.0)
    if -5.0 <= y <= 9.0:
        height = blend_value(height, 0.12, east_path_weight * 0.75)
    return height


def create_ground():
    grass = create_material("BaseGroundGrass", (0.19, 0.36, 0.12, 1.0), 0.9)
    dirt = create_material("BaseGroundDirt", (0.22, 0.12, 0.055, 1.0), 1.0)

    x_count = round((GROUND_HALF_WIDTH * 2.0) / GROUND_STEP) + 1
    y_count = round((GROUND_HALF_DEPTH * 2.0) / GROUND_STEP) + 1
    vertices = []
    faces = []
    material_indices = []

    for y_index in range(y_count):
        y = -GROUND_HALF_DEPTH + (y_index * GROUND_STEP)
        for x_index in range(x_count):
            x = -GROUND_HALF_WIDTH + (x_index * GROUND_STEP)
            vertices.append((x, y, terrain_height(x, y)))

    for y_index in range(y_count - 1):
        for x_index in range(x_count - 1):
            lower_left = (y_index * x_count) + x_index
            lower_right = lower_left + 1
            upper_left = lower_left + x_count
            upper_right = upper_left + 1
            faces.append((lower_left, lower_right, upper_right, upper_left))
            material_indices.append(0)

    top_vertex_count = len(vertices)
    perimeter = []
    for x_index in range(x_count):
        perimeter.append(x_index)
    for y_index in range(1, y_count):
        perimeter.append((y_index * x_count) + (x_count - 1))
    for x_index in range(x_count - 2, -1, -1):
        perimeter.append(((y_count - 1) * x_count) + x_index)
    for y_index in range(y_count - 2, 0, -1):
        perimeter.append(y_index * x_count)

    for top_index in perimeter:
        x, y, unused_height = vertices[top_index]
        vertices.append((x, y, GROUND_BOTTOM))

    perimeter_count = len(perimeter)
    for index in range(perimeter_count):
        next_index = (index + 1) % perimeter_count
        top_a = perimeter[index]
        top_b = perimeter[next_index]
        bottom_a = top_vertex_count + index
        bottom_b = top_vertex_count + next_index
        faces.append((top_b, top_a, bottom_a, bottom_b))
        material_indices.append(1)

    bottom_corners = (
        top_vertex_count,
        top_vertex_count + x_count - 1,
        top_vertex_count + x_count + y_count - 2,
        top_vertex_count + (x_count * 2) + y_count - 3,
    )
    faces.append(
        (
            bottom_corners[3],
            bottom_corners[2],
            bottom_corners[1],
            bottom_corners[0],
        )
    )
    material_indices.append(1)

    mesh = bpy.data.meshes.new("BaseGroundGeo")
    mesh.from_pydata(vertices, (), faces)
    mesh.update(calc_edges=True)
    mesh.materials.append(grass)
    mesh.materials.append(dirt)
    for polygon, material_index in zip(mesh.polygons, material_indices):
        polygon.material_index = material_index

    ground = bpy.data.objects.new("BaseGround", mesh)
    bpy.context.collection.objects.link(ground)
    return ground


def get_principled_base_color(material):
    if not material.use_nodes or material.node_tree is None:
        return None
    for node in material.node_tree.nodes:
        if node.type == "BSDF_PRINCIPLED":
            return tuple(node.inputs["Base Color"].default_value)
    return None


def prepare_imported_materials(objects, file_name):
    materials = set()
    for source_object in objects:
        if source_object.data is None:
            continue
        if not hasattr(source_object.data, "materials"):
            continue
        for material in source_object.data.materials:
            if material is not None:
                materials.add(material)

    rpg_colors = {
        "Barrel.blend": (
            (0.20, 0.07, 0.02, 1.0),
            (0.12, 0.035, 0.01, 1.0),
            (0.12, 0.15, 0.17, 1.0),
            (0.38, 0.14, 0.035, 1.0),
        ),
        "Book.blend": ((0.42, 0.045, 0.025, 1.0),),
        "Gems.blend": (
            (0.70, 0.04, 0.05, 1.0),
            (0.05, 0.30, 0.80, 1.0),
            (0.04, 0.62, 0.24, 1.0),
            (0.88, 0.52, 0.04, 1.0),
            (0.52, 0.10, 0.72, 1.0),
            (0.05, 0.68, 0.72, 1.0),
            (0.72, 0.78, 0.86, 1.0),
        ),
        "Shield.blend": (
            (0.48, 0.23, 0.06, 1.0),
            (0.18, 0.21, 0.23, 1.0),
            (0.23, 0.075, 0.018, 1.0),
        ),
        "Sword.blend": ((0.34, 0.39, 0.43, 1.0),),
        "WoodenStaff.blend": ((0.29, 0.095, 0.025, 1.0),),
    }
    ordered_materials = sorted(materials, key=lambda item: item.name)
    color_overrides = rpg_colors.get(file_name)
    for material_index, material in enumerate(ordered_materials):
        base_color = get_principled_base_color(material)
        if color_overrides is not None:
            base_color = color_overrides[material_index % len(color_overrides)]
        if base_color is not None:
            material.diffuse_color = base_color
            if material.use_nodes and material.node_tree is not None:
                for node in material.node_tree.nodes:
                    if node.type == "BSDF_PRINCIPLED":
                        node.inputs["Base Color"].default_value = base_color
        material["_x_power"] = 20.0
        material["_x_specular"] = (0.06, 0.06, 0.06)
        material["_x_emissive"] = (0.0, 0.0, 0.0)


def load_prototype(file_name):
    path = SOURCE_DIR / file_name
    if not path.exists():
        raise FileNotFoundError(path)
    with bpy.data.libraries.load(str(path), link=False) as (data_from, data_to):
        data_to.objects = [
            name
            for name in data_from.objects
            if name is not None
        ]
    objects = [
        source_object
        for source_object in data_to.objects
        if source_object is not None and source_object.type == "MESH"
    ]
    if not objects:
        raise RuntimeError("No mesh found in " + str(path))
    prepare_imported_materials(objects, file_name)
    return objects


def add_asset(prototypes, file_name, name, location, scale, rotation_degrees=0.0):
    source_objects = prototypes[file_name]
    instances = []
    for source_index, source_object in enumerate(source_objects):
        instance = source_object.copy()
        instance.data = source_object.data.copy()
        instance.name = name + "_" + str(source_index)
        instance.location = location
        instance.rotation_euler = (0.0, 0.0, math.radians(rotation_degrees))
        instance.scale = (scale, scale, scale)
        bpy.context.collection.objects.link(instance)
        instances.append(instance)
    return instances


def add_cube(name, location, dimensions, material):
    bpy.ops.mesh.primitive_cube_add(location=location)
    obj = bpy.context.object
    obj.name = name
    obj.dimensions = dimensions
    obj.data.materials.append(material)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    return obj


def add_cylinder(name, location, radius, depth, material, vertices=12):
    bpy.ops.mesh.primitive_cylinder_add(
        vertices=vertices,
        radius=radius,
        depth=depth,
        location=location,
    )
    obj = bpy.context.object
    obj.name = name
    obj.data.materials.append(material)
    return obj


def add_workshop(prototypes, wood, dark_wood, cloth, metal):
    objects = []
    objects.append(add_cube("WorkshopDeck", (-7.0, -20.0, 0.28), (8.0, 6.0, 0.18), wood))
    objects.append(add_cube("WorkshopBenchTop", (-7.0, -19.5, 1.12), (3.4, 1.1, 0.18), dark_wood))
    for x in (-8.35, -5.65):
        for y in (-19.85, -19.15):
            objects.append(add_cube("WorkshopBenchLeg", (x, y, 0.70), (0.16, 0.16, 0.85), dark_wood))
    for x in (-9.25, -4.75):
        for y in (-21.3, -18.7):
            objects.append(add_cube("WorkshopCanopyPost", (x, y, 1.48), (0.16, 0.16, 2.4), dark_wood))
    roof = add_cube("WorkshopCanopy", (-7.0, -20.0, 2.72), (5.1, 3.4, 0.12), cloth)
    roof.rotation_euler[1] = math.radians(-6.0)
    objects.append(roof)
    objects.extend(add_asset(prototypes, "Barrel.blend", "WorkshopBarrelA", (-9.0, -18.9, 0.78), 0.34, 10.0))
    objects.extend(add_asset(prototypes, "Barrel.blend", "WorkshopBarrelB", (-8.5, -18.7, 1.42), 0.26, -15.0))
    objects.extend(add_asset(prototypes, "Book.blend", "WorkshopBook", (-7.4, -19.45, 1.27), 0.20, 18.0))
    objects.extend(add_asset(prototypes, "Gems.blend", "WorkshopGems", (-6.65, -19.42, 1.25), 0.16, -12.0))
    objects.extend(add_asset(prototypes, "Shield.blend", "WorkshopShield", (-5.0, -20.6, 1.05), 0.32, 82.0))
    objects.extend(add_asset(prototypes, "Sword.blend", "WorkshopSword", (-6.0, -19.45, 1.36), 0.25, -20.0))
    objects.extend(add_asset(prototypes, "WoodenStaff.blend", "WorkshopStaff", (-9.15, -20.6, 1.35), 0.34, 8.0))
    objects.append(add_cube("WorkshopToolRack", (-8.95, -20.65, 1.35), (0.14, 0.8, 1.8), metal))
    return objects


def add_portal(stone, glow):
    objects = []
    objects.append(add_cube("PortalStep", (0.0, 26.0, 0.22), (6.0, 3.6, 0.32), stone))
    objects.append(add_cube("PortalPillarLeft", (-2.0, 26.0, 1.85), (0.75, 0.85, 3.2), stone))
    objects.append(add_cube("PortalPillarRight", (2.0, 26.0, 1.85), (0.75, 0.85, 3.2), stone))
    objects.append(add_cube("PortalLintel", (0.0, 26.0, 3.55), (4.75, 0.9, 0.75), stone))
    objects.append(add_cube("PortalGlow", (0.0, 26.05, 1.92), (3.15, 0.08, 2.85), glow))
    return objects


def add_nature(prototypes):
    objects = []
    tree_layout = (
        ("Tree1.blend", "TreeSouthWest", (-13.2, -27.0, 0.15), 1.15, 15.0),
        ("Tree2.blend", "TreeSouthEast", (13.0, -25.0, 0.10), 1.10, -28.0),
        ("Tree4.blend", "TreeWorkshop", (-13.2, -17.0, 0.52), 1.20, 42.0),
        ("Tree1.blend", "TreeMidEast", (13.2, -7.5, 0.18), 1.25, -12.0),
        ("Tree2.blend", "TreeHillWestA", (-12.2, 11.5, 0.82), 1.35, 20.0),
        ("Tree4.blend", "TreeHillWestB", (-14.1, 18.0, 0.74), 1.15, -30.0),
        ("Tree1.blend", "TreeHillEastA", (12.4, 13.0, 0.66), 1.30, 35.0),
        ("Tree2.blend", "TreeHillEastB", (14.0, 20.0, 0.48), 1.10, 0.0),
        ("Tree4.blend", "TreePortalWest", (-10.5, 27.0, 0.20), 1.20, 18.0),
        ("Tree1.blend", "TreePortalEast", (10.8, 28.0, 0.16), 1.15, -18.0),
    )
    for file_name, name, location, scale, rotation in tree_layout:
        objects.extend(add_asset(prototypes, file_name, name, location, scale, rotation))

    rock_layout = (
        ("Rock1.blend", "PondRockW1", (-9.6, -0.5, 0.0), 0.72, 10.0),
        ("Rock2.blend", "PondRockW2", (-9.2, 3.5, 0.0), 0.58, 52.0),
        ("Rock1.blend", "PondRockE1", (9.5, 0.0, 0.0), 0.62, -20.0),
        ("Rock2.blend", "PondRockE2", (9.0, 4.7, 0.0), 0.48, 30.0),
        ("Rock1.blend", "PondRockNorth", (3.6, 6.2, 0.0), 0.55, 82.0),
        ("Rock2.blend", "PondRockSouth", (-4.0, -2.5, 0.0), 0.48, 12.0),
        ("Rock1.blend", "HillRockWest", (-13.3, 8.4, 0.65), 0.82, -15.0),
        ("Rock2.blend", "HillRockEast", (13.0, 9.0, 0.55), 0.74, 45.0),
    )
    for file_name, name, location, scale, rotation in rock_layout:
        objects.extend(add_asset(prototypes, file_name, name, location, scale, rotation))

    climb_rock_layout = (
        ("Rock1.blend", "ClimbRockWestSouth", (-13.1, -2.0, 0.25), 1.45, 12.0),
        ("Rock2.blend", "ClimbRockWestMid", (-12.0, 1.0, 0.35), 2.00, 42.0),
        ("Rock1.blend", "ClimbRockWestNorth", (-13.0, 4.2, 0.42), 2.15, -18.0),
        ("Rock2.blend", "ClimbRockHillWestA", (-11.2, 8.2, 0.58), 1.90, 25.0),
        ("Rock1.blend", "ClimbRockHillWestB", (-8.7, 11.2, 0.72), 1.65, -35.0),
        ("Rock2.blend", "ClimbRockHillWestC", (-6.5, 14.0, 0.62), 2.20, 8.0),
        ("Rock1.blend", "ClimbRockHillEastA", (11.8, 10.0, 0.55), 1.55, 32.0),
        ("Rock2.blend", "ClimbRockHillEastB", (9.6, 13.0, 0.68), 2.15, -12.0),
        ("Rock1.blend", "ClimbRockHillEastC", (7.2, 16.2, 0.58), 2.10, 48.0),
    )
    for file_name, name, location, scale, rotation in climb_rock_layout:
        objects.extend(add_asset(prototypes, file_name, name, location, scale, rotation))

    bush_layout = (
        ("Bush1.blend", "BushSouthWest", (-11.5, -28.0, 0.1), 0.85, 0.0),
        ("Bush2.blend", "BushWorkshop", (-12.0, -19.5, 0.45), 0.75, 25.0),
        ("Bush1.blend", "BushPondWest", (-10.5, 5.5, 0.2), 0.72, 70.0),
        ("Bush2.blend", "BushPondEast", (10.7, 6.2, 0.18), 0.78, -20.0),
        ("Bush1.blend", "BushPortalWest", (-7.0, 27.5, 0.1), 0.75, 15.0),
        ("Bush2.blend", "BushPortalEast", (7.2, 27.0, 0.1), 0.72, -15.0),
    )
    for file_name, name, location, scale, rotation in bush_layout:
        objects.extend(add_asset(prototypes, file_name, name, location, scale, rotation))

    grass_positions = (
        (-11.0, -23.0, 0.3, 18.0),
        (10.5, -19.0, 0.1, -30.0),
        (-12.0, -8.0, 0.25, 42.0),
        (12.0, -1.5, 0.12, 5.0),
        (-11.0, 7.0, 0.42, -18.0),
        (11.5, 8.0, 0.48, 24.0),
        (-8.0, 18.0, 0.3, 12.0),
        (8.5, 20.0, 0.3, -12.0),
    )
    for index, (x, y, z, rotation) in enumerate(grass_positions):
        file_name = "Grass1.blend"
        if index % 2 == 1:
            file_name = "Grass2.blend"
        objects.extend(
            add_asset(
                prototypes,
                file_name,
                "GrassPatch" + str(index),
                (x, y, z),
                0.75,
                rotation,
            )
        )
    return objects


def create_decor():
    wood = create_material("HubWood", (0.31, 0.13, 0.045, 1.0), 0.82)
    dark_wood = create_material("HubDarkWood", (0.13, 0.055, 0.02, 1.0), 0.88)
    cloth = create_material("HubCanopyCloth", (0.55, 0.18, 0.055, 1.0), 0.76)
    metal = create_material("HubMetal", (0.16, 0.18, 0.19, 1.0), 0.38, 0.45)
    stone = create_material("HubPortalStone", (0.25, 0.28, 0.28, 1.0), 0.92)
    glow = create_material(
        "HubPortalGlow",
        (0.025, 0.22, 0.34, 0.78),
        0.28,
        0.0,
        (0.04, 0.58, 0.82),
    )

    source_files = (
        "Tree1.blend",
        "Tree2.blend",
        "Tree4.blend",
        "Rock1.blend",
        "Rock2.blend",
        "Bush1.blend",
        "Bush2.blend",
        "Grass1.blend",
        "Grass2.blend",
        "Barrel.blend",
        "Book.blend",
        "Gems.blend",
        "Shield.blend",
        "Sword.blend",
        "WoodenStaff.blend",
    )
    prototypes = {}
    for file_name in source_files:
        prototypes[file_name] = load_prototype(file_name)

    add_nature(prototypes)
    add_workshop(prototypes, wood, dark_wood, cloth, metal)
    add_portal(stone, glow)
    return list(bpy.context.scene.objects)


def create_collision():
    collision_material = create_material(
        "BaseHubCollision",
        (0.35, 0.35, 0.35, 1.0),
        1.0,
    )
    objects = []
    tree_positions = (
        (-13.2, -27.0, 0.15, 1.15),
        (13.0, -25.0, 0.10, 1.10),
        (-13.2, -17.0, 0.52, 1.20),
        (13.2, -7.5, 0.18, 1.25),
        (-12.2, 11.5, 0.82, 1.35),
        (-14.1, 18.0, 0.74, 1.15),
        (12.4, 13.0, 0.66, 1.30),
        (14.0, 20.0, 0.48, 1.10),
        (-10.5, 27.0, 0.20, 1.20),
        (10.8, 28.0, 0.16, 1.15),
    )
    for index, (x, y, z, scale) in enumerate(tree_positions):
        objects.append(
            add_cylinder(
                "TreeCollision" + str(index),
                (x, y, z + (1.1 * scale)),
                0.30 * scale,
                2.2 * scale,
                collision_material,
                10,
            )
        )
    climb_rock_collisions = (
        ("ClimbRockWestSouthCollision", -13.1, -2.0, 0.25, 1.45),
        ("ClimbRockWestMidCollision", -12.0, 1.0, 0.35, 2.00),
        ("ClimbRockWestNorthCollision", -13.0, 4.2, 0.42, 2.15),
        ("ClimbRockHillWestACollision", -11.2, 8.2, 0.58, 1.90),
        ("ClimbRockHillWestBCollision", -8.7, 11.2, 0.72, 1.65),
        ("ClimbRockHillWestCCollision", -6.5, 14.0, 0.62, 2.20),
        ("ClimbRockHillEastACollision", 11.8, 10.0, 0.55, 1.55),
        ("ClimbRockHillEastBCollision", 9.6, 13.0, 0.68, 2.15),
        ("ClimbRockHillEastCCollision", 7.2, 16.2, 0.58, 2.10),
    )
    for name, x, y, z, scale in climb_rock_collisions:
        objects.append(
            add_cube(
                name,
                (x, y, z + (0.18 * scale)),
                (1.75 * scale, 2.10 * scale, 0.92 * scale),
                collision_material,
            )
        )
    objects.append(add_cube("WorkshopDeckCollision", (-7.0, -20.0, 0.28), (8.0, 6.0, 0.18), collision_material))
    objects.append(add_cube("WorkshopBenchCollision", (-7.0, -19.5, 0.82), (3.4, 1.1, 1.35), collision_material))
    objects.append(add_cube("PortalPillarLeftCollision", (-2.0, 26.0, 1.85), (0.75, 0.85, 3.2), collision_material))
    objects.append(add_cube("PortalPillarRightCollision", (2.0, 26.0, 1.85), (0.75, 0.85, 3.2), collision_material))
    return objects


def export_x(path, objects):
    bpy.ops.object.select_all(action="DESELECT")
    for obj in objects:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = objects[0]
    result = bpy.ops.export_scene.directx_x(
        filepath=str(path),
        use_selection=True,
        use_mesh_modifiers=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        export_normals=True,
        export_uvs=True,
        export_materials=True,
        export_textures=False,
        export_armature=False,
        export_weights=False,
        export_animation=False,
        unweld_on_export=False,
        export_format="TEXT_X",
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(path))


def main():
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    bpy.context.preferences.filepaths.save_version = 0

    clear_scene()
    ground = create_ground()
    bpy.ops.wm.save_as_mainfile(filepath=str(GROUND_BLEND_PATH))
    export_x(GROUND_X_PATH, [ground])

    clear_scene()
    decor_objects = create_decor()
    bpy.ops.wm.save_as_mainfile(filepath=str(DECOR_BLEND_PATH))
    export_x(DECOR_X_PATH, decor_objects)

    clear_scene()
    collision_objects = create_collision()
    bpy.ops.wm.save_as_mainfile(filepath=str(COLLISION_BLEND_PATH))
    export_x(COLLISION_X_PATH, collision_objects)

    print("BASE_GROUND_X", GROUND_X_PATH)
    print("BASE_DECOR_X", DECOR_X_PATH)
    print("BASE_DECOR_COLLISION_X", COLLISION_X_PATH)


main()
