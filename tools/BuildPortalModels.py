import bpy
from pathlib import Path

BLENDER_EXECUTABLE_NOTE = "Run with: blender.exe --background --python BuildPortalModels.py"

REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
PORTAL_DIRECTORY = REPOSITORY_ROOT / "RedFortress2" / "MultiPassRendering" / "res" / "model" / "portal"

EXPORT_KWARGS = dict(
    check_existing=False,
    use_selection=True,
    use_mesh_modifiers=True,
    global_scale=1.0,
    axis_forward="-Z",
    axis_up="Y",
    export_normals=True,
    export_uvs=True,
    export_materials=True,
    export_textures=True,
    triangulate=True,
    unweld_on_export=False,
)


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for mesh in list(bpy.data.meshes):
        bpy.data.meshes.remove(mesh)
    for material in list(bpy.data.materials):
        bpy.data.materials.remove(material)
    for image in list(bpy.data.images):
        if image.users == 0:
            bpy.data.images.remove(image)
    for armature in list(bpy.data.armatures):
        bpy.data.armatures.remove(armature)
    for action in list(bpy.data.actions):
        bpy.data.actions.remove(action)


def normalize_x_file(path):
    data = path.read_bytes()
    if data.startswith(b"\xef\xbb\xbf"):
        data = data[3:]
    data = data.replace(b"\r\n", b"\n").replace(b"\r", b"\n")
    data = data.replace(b"\n", b"\r\n")
    path.write_bytes(data)


def export_selected_x(x_path, export_armature, export_weights, export_animation):
    result = bpy.ops.export_scene.directx_x(
        filepath=str(x_path),
        export_armature=export_armature,
        export_weights=export_weights,
        export_animation=export_animation,
        **EXPORT_KWARGS,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(x_path))
    normalize_x_file(x_path)


def save_blend(blend_path):
    bpy.data.use_autopack = False
    bpy.context.preferences.filepaths.save_version = 0
    bpy.ops.wm.save_as_mainfile(filepath=str(blend_path))


def add_box(name, size_x, size_y, size_z, center_x=0.0, center_y=0.0, center_z=0.0):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(center_x, center_y, center_z))
    obj = bpy.context.active_object
    obj.name = name
    obj.scale = (size_x, size_y, size_z)
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=True)
    return obj


def build_stone_steps_mesh():
    tiers = [
        (2.0, 2.0, 0.5, 0.00),
        (1.6, 1.6, 0.5, 0.50),
        (1.2, 1.2, 0.5, 1.00),
    ]
    boxes = []
    for index, (size_x, size_y, height, base_z) in enumerate(tiers):
        center_z = base_z + height * 0.5
        obj = add_box("stone_tier_{}".format(index), size_x, size_y, height, center_z=center_z)
        boxes.append(obj)
    bpy.ops.object.select_all(action="DESELECT")
    for obj in boxes:
        obj.select_set(True)
    bpy.context.view_layer.objects.active = boxes[0]
    bpy.ops.object.join()
    joined = bpy.context.active_object
    joined.name = "stone_steps"
    return joined


def build_stone_texture(path):
    width = 256
    height = 256
    image = bpy.data.images.new("stone_steps", width=width, height=height, alpha=False)
    pixels = []
    import random
    random.seed(1234)
    for _y in range(height):
        for _x in range(width):
            noise = random.random() * 0.18
            base = 0.42 + noise
            pixels.extend([base, base * 0.97, base * 0.90, 1.0])
    image.pixels = pixels
    image.filepath_raw = str(path)
    image.file_format = "PNG"
    image.save()
    return image


def create_textured_material(material_name, texture_filename, image):
    material = bpy.data.materials.new(material_name)
    material.use_nodes = True
    material.diffuse_color = (1.0, 1.0, 1.0, 1.0)
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    principled = None
    for node in nodes:
        if node.bl_idname == "ShaderNodeBsdfPrincipled":
            principled = node
            break
    if principled is None:
        raise RuntimeError("Principled BSDF node was not found")
    image_node = nodes.new("ShaderNodeTexImage")
    image_node.name = material_name
    image_node.image = image
    links.new(image_node.outputs["Color"], principled.inputs["Base Color"])
    principled.inputs["Roughness"].default_value = 0.95
    return material


def build_stone_steps():
    PORTAL_DIRECTORY.mkdir(parents=True, exist_ok=True)
    texture_path = PORTAL_DIRECTORY / "stone_steps.png"
    x_path = PORTAL_DIRECTORY / "stone_steps.x"
    collision_x_path = PORTAL_DIRECTORY / "stone_steps_collision.x"
    blend_path = PORTAL_DIRECTORY / "stone_steps.blend"

    clear_scene()
    image = build_stone_texture(texture_path)
    material = create_textured_material("stone_steps", "stone_steps.png", image)
    steps_obj = build_stone_steps_mesh()
    steps_obj.data.materials.append(material)

    bpy.context.view_layer.objects.active = steps_obj
    steps_obj.select_set(True)
    save_blend(blend_path)
    export_selected_x(x_path, export_armature=False, export_weights=False, export_animation=False)
    export_selected_x(collision_x_path, export_armature=False, export_weights=False, export_animation=False)

    print("STONE_STEPS_BLEND {}".format(blend_path))
    print("STONE_STEPS_X {}".format(x_path))
    print("STONE_STEPS_COLLISION_X {}".format(collision_x_path))
    print("STONE_STEPS_TEXTURE {}".format(texture_path))


def build_light_pillar_mesh(radius=0.4, height=20.0, segments=24):
    # Cylinder with base on Z=0 (Blender up axis). Origin stays at world origin.
    bpy.ops.mesh.primitive_cylinder_add(
        radius=radius,
        depth=height,
        vertices=segments,
        location=(0.0, 0.0, height * 0.5),
    )
    obj = bpy.context.active_object
    obj.name = "light_pillar"
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=True)
    return obj


def build_light_pillar_texture(path):
    width = 64
    height = 256
    image = bpy.data.images.new("light_pillar", width=width, height=height, alpha=True)
    pixels = []
    for y in range(height):
        # Vertical gradient: bright white at the bottom fading to soft blue at the top.
        t = y / (height - 1)
        r = 0.85 - 0.25 * t
        g = 0.92 - 0.20 * t
        b = 1.0
        a = 0.75 - 0.45 * t
        for _x in range(width):
            pixels.extend([r, g, b, a])
    image.pixels = pixels
    image.filepath_raw = str(path)
    image.file_format = "PNG"
    image.save()
    return image


def create_emissive_textured_material(material_name, image, emission_strength=2.0):
    material = bpy.data.materials.new(material_name)
    material.use_nodes = True
    material.blend_method = "BLEND"
    material.diffuse_color = (1.0, 1.0, 1.0, 1.0)
    nodes = material.node_tree.nodes
    links = material.node_tree.links
    principled = None
    for node in nodes:
        if node.bl_idname == "ShaderNodeBsdfPrincipled":
            principled = node
            break
    if principled is None:
        raise RuntimeError("Principled BSDF node was not found")
    image_node = nodes.new("ShaderNodeTexImage")
    image_node.name = material_name
    image_node.image = image
    links.new(image_node.outputs["Color"], principled.inputs["Base Color"])
    links.new(image_node.outputs["Alpha"], principled.inputs["Alpha"])
    # Emission so the pillar looks self-lit even without strong lighting.
    if "Emission Color" in principled.inputs:
        links.new(image_node.outputs["Color"], principled.inputs["Emission Color"])
        principled.inputs["Emission Color"].default_value = (0.8, 0.9, 1.0, 1.0)
    if "Emission Strength" in principled.inputs:
        principled.inputs["Emission Strength"].default_value = emission_strength
    principled.inputs["Roughness"].default_value = 0.3
    return material


def write_light_pillar_csv(csv_path):
    # Same Emit format as res/model/Buster/buster.csv
    lines = [
        "meshtype,Emit",
        "EmitIntensity,0.5",
        "EmitColor,200,230,255",
    ]
    csv_path.write_text("\r\n".join(lines) + "\r\n", encoding="ascii")


def build_light_pillar():
    PORTAL_DIRECTORY.mkdir(parents=True, exist_ok=True)
    texture_path = PORTAL_DIRECTORY / "light_pillar.png"
    x_path = PORTAL_DIRECTORY / "light_pillar.x"
    csv_path = PORTAL_DIRECTORY / "light_pillar.csv"
    blend_path = PORTAL_DIRECTORY / "light_pillar.blend"

    clear_scene()
    image = build_light_pillar_texture(texture_path)
    material = create_emissive_textured_material("light_pillar", image)
    pillar_obj = build_light_pillar_mesh()
    pillar_obj.data.materials.append(material)

    bpy.context.view_layer.objects.active = pillar_obj
    pillar_obj.select_set(True)
    save_blend(blend_path)
    export_selected_x(x_path, export_armature=False, export_weights=False, export_animation=False)
    write_light_pillar_csv(csv_path)

    print("LIGHT_PILLAR_BLEND {}".format(blend_path))
    print("LIGHT_PILLAR_X {}".format(x_path))
    print("LIGHT_PILLAR_TEXTURE {}".format(texture_path))
    print("LIGHT_PILLAR_CSV {}".format(csv_path))


if __name__ == "__main__":
    build_stone_steps()
    build_light_pillar()
