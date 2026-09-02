from pathlib import Path

import bmesh
import bpy


MODEL_DIRECTORY = Path(__file__).resolve().parent
MODEL_PATH = MODEL_DIRECTORY / "itemIconMaterial.x"
BLEND_PATH = MODEL_DIRECTORY / "itemIconMaterial.blend"


def load_blend_source():
    current_blend_path = Path(bpy.data.filepath).resolve()
    if current_blend_path != BLEND_PATH.resolve():
        bpy.ops.wm.open_mainfile(filepath=str(BLEND_PATH))

    mesh_objects = [obj for obj in bpy.context.scene.objects if obj.type == "MESH"]
    if not mesh_objects:
        raise RuntimeError("itemIconMaterial.blend にメッシュがありません。")


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


load_blend_source()
recalculate_normals_outside()
bpy.context.preferences.filepaths.save_version = 0
bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
export_model()
