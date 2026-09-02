from pathlib import Path

import bmesh
import bpy


MODEL_DIRECTORY = Path(__file__).resolve().parent
MODEL_PATH = MODEL_DIRECTORY / "itemIconMaterial.x"
BLEND_PATH = MODEL_DIRECTORY / "itemIconMaterial.blend"


def import_model_if_needed():
    mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if mesh_objects:
        return

    result = bpy.ops.import_scene.directx_x(
        filepath=str(MODEL_PATH),
        use_apply_transform=True,
        global_scale=1.0,
        axis_forward="Z",
        axis_up="Y",
        import_normals=False,
        import_uvs=True,
        import_materials=True,
        import_textures=True,
        split_submeshes=True,
        triangulate_quads=False,
        import_armature=False,
        import_weights=False,
        import_animation=False,
        weld_duplicate_verts=False,
        use_import_collection=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("itemIconMaterial.x のインポートに失敗しました。")


def recalculate_normals_outside():
    for obj in bpy.context.scene.objects:
        if obj.type != "MESH":
            continue

        mesh = obj.data
        work_mesh = bmesh.new()
        work_mesh.from_mesh(mesh)
        bmesh.ops.recalc_face_normals(work_mesh, faces=list(work_mesh.faces))
        work_mesh.to_mesh(mesh)
        work_mesh.free()

        for polygon in mesh.polygons:
            polygon.use_smooth = False
        mesh.update()


def export_model():
    result = bpy.ops.export_scene.directx_x(
        filepath=str(MODEL_PATH),
        use_selection=False,
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
        use_original_material_data=True,
        export_format="TEXT_X",
        triangulate=False,
    )
    if "FINISHED" not in result:
        raise RuntimeError("itemIconMaterial.x のエクスポートに失敗しました。")


import_model_if_needed()
recalculate_normals_outside()
bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
export_model()
