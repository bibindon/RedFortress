# -*- coding: utf-8 -*-
"""CGTrader Teddy Bear (OBJ) -> warp_bear.x (mirror chrome look preserved)

Usage:
    1. Download the free model from CGTrader
       https://www.cgtrader.com/free-3d-models/animal/mammal/teddy-bear-toy-plush
       (OBJ format, requires a CGTrader account), unzip it into the
       teddy/ subfolder next to this script (any *.obj in there is used;
       a filename containing "low" is preferred - the high-poly variant
       has a UV mismatch with the texture per buyer reviews, and the
       low-poly one is the game-ready asset).

    2. Run headless:
       "/c/Program Files/Blender Foundation/Blender 5.1/blender" --background --python _build_warp_bear_teddy.py

    Output:
        warp_bear.x        - replaced game model (same path, 4 stages pick it up)
        warp_bear_teddy.blend - Blender source of the new model

    The mirror/chrome look comes from the sidecar warp_bear.csv
    (EnvMap=y + Texture1.dds + CubeMappingRate=1.0), which is UNCHANGED.
    The material below reproduces the old sphere bear's specular settings.
"""

from pathlib import Path

import bpy

BASE_DIR = Path(__file__).resolve().parent
TEDDY_DIR = BASE_DIR / "teddy"
BLEND_PATH = BASE_DIR / "warp_bear_teddy.blend"
X_PATH = BASE_DIR / "warp_bear.x"

# 目標の高さ（メートル）。1 ワールド単位 = 1m。
TEDDY_HEIGHT_M = 1.5
# デシメート目標面数（stage 3-5 は 20 体配置されるため抑える）
DECIMATE_TARGET_FACES = 1500
# ファイル名に "low" を含む OBJ を優先する（レビュー指摘の高ポリ版 UV 不一致対策）
OBJ_PREFER_LOW = True

# 現行ワープベアのマテリアル設定を踏襲（完全鏡面ルック）
FACE_COLOR = (0.18, 0.32, 0.48, 1.0)
SPECULAR_POWER = 160.0
SPECULAR_COLOR = (0.7, 0.85, 1.0)
EMISSIVE_COLOR = (0.02, 0.03, 0.05)


def enable_directx_addon():
    bpy.ops.preferences.addon_enable(module="bl_ext.blender_org.io_directx_x")


def clear_scene():
    bpy.ops.object.select_all(action="SELECT")
    bpy.ops.object.delete(use_global=False)
    for collection in (bpy.data.meshes, bpy.data.materials):
        for data_block in list(collection):
            collection.remove(data_block)


def find_obj_file():
    if not TEDDY_DIR.exists():
        raise RuntimeError(
            "teddy/ フォルダがありません。CGTrader から OBJ をダウンロードして "
            + str(TEDDY_DIR) + " に展開してください。"
        )
    obj_files = sorted(TEDDY_DIR.glob("*.obj"))
    if not obj_files:
        raise RuntimeError("teddy/ 内に .obj ファイルが見つかりません: " + str(TEDDY_DIR))
    if OBJ_PREFER_LOW:
        low = [p for p in obj_files if "low" in p.name.lower()]
        if low:
            return low[0]
    return obj_files[0]


def import_obj(path):
    result = bpy.ops.wm.obj_import(filepath=str(path))
    if "FINISHED" not in result:
        raise RuntimeError("OBJ import failed: " + str(path))
    meshes = [o for o in bpy.data.objects if o.type == "MESH"]
    if not meshes:
        raise RuntimeError("OBJ からメッシュがインポートされませんでした: " + str(path))
    return meshes


def join_meshes(meshes):
    if len(meshes) == 1:
        bear = meshes[0]
        bear.name = "WarpBear"
        return bear
    bpy.ops.object.select_all(action="DESELECT")
    for mesh in meshes:
        mesh.select_set(True)
    bpy.context.view_layer.objects.active = meshes[0]
    bpy.ops.object.join()
    bear = bpy.context.object
    bear.name = "WarpBear"
    return bear


def mesh_extents(bear):
    """頂点座標から (min_x, min_y, min_z, max_x, max_y, max_z) を直接計算。
    Object.dimensions はヘッドレスで depsgraph 未評価のため (0,0,0) を返し得る。"""
    xs, ys, zs = [], [], []
    for v in bear.data.vertices:
        xs.append(v.co.x)
        ys.append(v.co.y)
        zs.append(v.co.z)
    return (min(xs), min(ys), min(zs), max(xs), max(ys), max(zs))


def orient_and_scale(bear, target_height):
    """Z 軸を上にする。高さを target_height メートルに合わせ、足元を原点へ。"""
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    min_x, min_y, min_z, max_x, max_y, max_z = mesh_extents(bear)
    # 横倒し（Z 高さが Y 高さより小さい）なら X 軸周りに -90 度回して立てる
    if (max_z - min_z) < (max_y - min_y):
        bear.rotation_euler[0] = -1.5707963267948966
        bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
        min_x, min_y, min_z, max_x, max_y, max_z = mesh_extents(bear)
    print("TEDDY_DIMS_BEFORE_SCALE", max_x - min_x, max_y - min_y, max_z - min_z)
    height = max_z - min_z
    if height <= 0.0:
        raise RuntimeError("モデルの高さが 0 です")
    scale = target_height / height
    bear.scale = (scale, scale, scale)
    bpy.ops.object.transform_apply(location=False, rotation=False, scale=True)
    # 足元（Z 最小）が Z=0 になるよう移動。X/Y は中心に寄せる。
    min_x, min_y, min_z, max_x, max_y, max_z = mesh_extents(bear)
    bear.location = (-(min_x + max_x) * 0.5, -(min_y + max_y) * 0.5, -min_z)
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)
    min_x, min_y, min_z, max_x, max_y, max_z = mesh_extents(bear)
    final_height = max_z - min_z
    print("TEDDY_FINAL_HEIGHT_M", final_height)
    if abs(final_height - target_height) > 0.02:
        raise RuntimeError("高さ合わせに失敗しました: " + str(final_height))


def decimate(bear, target_faces):
    face_count = len(bear.data.polygons)
    print("TEDDY_FACES_IMPORTED", face_count)
    if face_count <= target_faces:
        return
    decimate = bear.modifiers.new("Decimate", "DECIMATE")
    decimate.decimate_type = "COLLAPSE"
    decimate.ratio = target_faces / float(face_count)
    bpy.context.view_layer.update()
    print("TEDDY_FACES_AFTER_DECIMATE", decimate.face_count)
    bpy.context.view_layer.objects.active = bear
    bear.select_set(True)
    bpy.ops.object.modifier_apply(modifier="Decimate")


def create_mirror_material():
    material = bpy.data.materials.new("WarpBearMirrorFur")
    material.use_nodes = True
    material.diffuse_color = FACE_COLOR
    material.metallic = 0.72
    material.roughness = 0.18
    material["_x_power"] = SPECULAR_POWER
    material["_x_specular"] = SPECULAR_COLOR
    material["_x_emissive"] = EMISSIVE_COLOR
    if material.use_nodes and material.node_tree is not None:
        principled = next(
            node for node in material.node_tree.nodes if node.type == "BSDF_PRINCIPLED"
        )
        principled.inputs["Base Color"].default_value = FACE_COLOR
        principled.inputs["Metallic"].default_value = 0.72
        principled.inputs["Roughness"].default_value = 0.18
    return material


def assign_single_material(bear, material):
    bear.data.materials.clear()
    bear.data.materials.append(material)
    for polygon in bear.data.polygons:
        polygon.use_smooth = True
    for other in list(bpy.data.materials):
        if other != material:
            bpy.data.materials.remove(other)


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
        unweld_on_export=True,
        export_format="TEXT_X",
        triangulate=True,
    )
    if "FINISHED" not in result:
        raise RuntimeError("DirectX X export failed: " + str(X_PATH))


def postprocess_x():
    """RedFortress 契約: .x は BOM なし UTF-8 + CRLF"""
    raw = X_PATH.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        raw = raw[3:]
    text = raw.decode("utf-8", errors="replace")
    text = text.replace("\r\n", "\n").replace("\r", "\n").replace("\n", "\r\n")
    X_PATH.write_bytes(text.encode("utf-8"))


def verify_x():
    raw = X_PATH.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        raise RuntimeError("BOM が残っています")
    without_crlf = raw.replace(b"\r\n", b"")
    if b"\r" in without_crlf or b"\n" in without_crlf:
        raise RuntimeError("CRLF 以外の改行が混在しています")
    if b"\r\n" not in raw:
        raise RuntimeError("CRLF が見つかりません")
    text = raw.decode("utf-8", errors="replace")
    material_count = text.count("Material ")
    print("VERIFY_MATERIAL_BLOCKS", material_count)
    if material_count != 1:
        raise RuntimeError("Material ブロックが 1 つではありません: " + str(material_count))
    import re
    # Material ブロックの 2 行目（face color の次の行）が specular power
    material_block = re.search(r"Material WarpBearMirrorFur \{(.*?)\}", text, re.S)
    if material_block is None:
        raise RuntimeError("Material WarpBearMirrorFur が見つかりません")
    material_lines = [line.strip() for line in material_block.group(1).splitlines() if line.strip()]
    if len(material_lines) < 2:
        raise RuntimeError("Material ブロックの行数が不足しています")
    power = float(material_lines[1].rstrip(";").strip())
    print("VERIFY_SPECULAR_POWER", power)
    if abs(power - SPECULAR_POWER) > 0.001:
        raise RuntimeError("specular power が期待値と一致しません: " + str(power))
    # テクスチャ参照タグ（TextureFileName/TextureFilename）の有無で判定。
    # MeshTextureCoords 等のブロック名は "Texture" を含むため単純一致は不可。
    if "TextureFileName" in text or "TextureFilename" in text:
        raise RuntimeError("鏡面ルックのため Texture 参照は不要のはずです")
    # Mesh ブロック: count の直後から続く count 行の頂点を抽出し、y（= Blender の Z=高さ）を検証
    mesh_match = re.search(r"Mesh WarpBearGeo \{\r?\n\s*(\d+);\r?\n", text)
    if mesh_match is None:
        raise RuntimeError("Mesh WarpBearGeo が見つかりません")
    count = int(mesh_match.group(1))
    rest = text[mesh_match.end():]
    # 頂点は「x; y; z;」+ 末尾に `,`（途中行）または `;`（最終行）。法線も同形式で後に続くため先頭 count 行を採用
    vertex_lines = re.findall(r"^[ \t]*[-\d.]+; [-\d.]+; [-\d.]+;+,?\r?$", rest, re.M)
    if len(vertex_lines) < count:
        raise RuntimeError("頂点数が不足しています: count=" + str(count) + " parsed=" + str(len(vertex_lines)))
    y_values = []
    for line in vertex_lines[:count]:
        parts = [p.strip() for p in line.rstrip(",").rstrip(";").split(";")]
        y_values.append(float(parts[1]))
    height = max(y_values) - min(y_values)
    print("VERIFY_VERTEX_COUNT", count)
    print("VERIFY_X_HEIGHT_M", height)
    if abs(height - TEDDY_HEIGHT_M) > 0.05:
        raise RuntimeError("X 内の高さが 1.5m になりません: " + str(height))


def main():
    enable_directx_addon()
    bpy.context.preferences.filepaths.save_version = 0
    clear_scene()
    obj_path = find_obj_file()
    print("TEDDY_OBJ", obj_path)
    meshes = import_obj(obj_path)
    bear = join_meshes(meshes)
    orient_and_scale(bear, TEDDY_HEIGHT_M)
    decimate(bear, DECIMATE_TARGET_FACES)
    material = create_mirror_material()
    assign_single_material(bear, material)
    bear["_x_frame_name"] = "WarpBear"
    bear["_x_mesh_name"] = "WarpBearGeo"
    bpy.ops.wm.save_as_mainfile(filepath=str(BLEND_PATH))
    export_x(bear)
    postprocess_x()
    verify_x()
    print("WARP_BEAR_TEDDY_BLEND", BLEND_PATH)
    print("WARP_BEAR_TEDDY_X", X_PATH)
    print("WARP_BEAR_TEDDY_OK")


main()
