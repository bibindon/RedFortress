from pathlib import Path

import bpy


BASE_DIR = Path(__file__).resolve().parent
BLEND_PATH = BASE_DIR / "warp_bear.blend"
X_PATH = BASE_DIR / "warp_bear.x"


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials):
        for data_block in list(collection):
            collection.remove(data_block)


def create_material():
    material = bpy.data.materials.new("WarpBearMirrorFur")
    material.use_nodes = True
    material.diffuse_color = (0.18, 0.32, 0.48, 1.0)
    material.metallic = 0.72
    material.roughness = 0.18
    material["_x_power"] = 160.0
    material["_x_specular"] = (0.7, 0.85, 1.0)
    material["_x_emissive"] = (0.02, 0.03, 0.05)
    principled = next(
        node for node in material.node_tree.nodes if node.type == "BSDF_PRINCIPLED"
    )
    principled.inputs["Base Color"].default_value = (0.18, 0.32, 0.48, 1.0)
    principled.inputs["Metallic"].default_value = 0.72
    principled.inputs["Roughness"].default_value = 0.18
    return material


def add_uv_sphere(name, location, scale, material):
    bpy.ops.mesh.primitive_uv_sphere_add(
        segments=32,
        ring_count=20,
        location=location,
    )
    obj = bpy.context.object
    obj.name = name
    obj.scale = scale
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    obj.data.materials.append(material)
    for polygon in obj.data.polygons:
        polygon.use_smooth = True
    return obj


def create_bear(material):
    parts = []
    parts.append(add_uv_sphere("WarpBearBody", (0.0, 0.0, 0.68), (0.48, 0.38, 0.62), material))
    parts.append(add_uv_sphere("WarpBearHead", (0.0, -0.02, 1.42), (0.44, 0.40, 0.40), material))
    parts.append(add_uv_sphere("WarpBearEarL", (-0.32, -0.01, 1.70), (0.19, 0.16, 0.19), material))
    parts.append(add_uv_sphere("WarpBearEarR", (0.32, -0.01, 1.70), (0.19, 0.16, 0.19), material))
    parts.append(add_uv_sphere("WarpBearMuzzle", (0.0, -0.36, 1.30), (0.22, 0.16, 0.16), material))
    parts.append(add_uv_sphere("WarpBearArmL", (-0.49, -0.01, 0.78), (0.17, 0.19, 0.42), material))
    parts.append(add_uv_sphere("WarpBearArmR", (0.49, -0.01, 0.78), (0.17, 0.19, 0.42), material))
    parts.append(add_uv_sphere("WarpBearFootL", (-0.23, -0.13, 0.16), (0.25, 0.31, 0.16), material))
    parts.append(add_uv_sphere("WarpBearFootR", (0.23, -0.13, 0.16), (0.25, 0.31, 0.16), material))

    bpy.ops.object.select_all(action="DESELECT")
    for part in parts:
        part.select_set(True)
    bpy.context.view_layer.objects.active = parts[0]
    bpy.ops.object.join()
    bear = bpy.context.object
    bear.name = "WarpBear"
    bear["_x_frame_name"] = "WarpBear"
    bear["_x_mesh_name"] = "WarpBearGeo"
    return bear


def export_x(bear):
    bpy.ops.object.select_all(action="DESELECT")
    bear.select_set(True)
    bpy.context.view_layer.objects.active = bear
    result = bpy.ops.export_scene.directx_x(
        filepath=str(X_PATH),
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
        raise RuntimeError("DirectX X export failed: " + str(X_PATH))


def main():
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")
    bpy.context.preferences.filepaths.save_version = 0
    clear_scene()
    material = create_material()
    bear = create_bear(material)
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    export_x(bear)
    print("WARP_BEAR_BLEND", BLEND_PATH)
    print("WARP_BEAR_X", X_PATH)


main()