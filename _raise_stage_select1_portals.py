import bpy
import os
from mathutils import Vector


model_dir = os.path.dirname(bpy.data.filepath)
island_export_path = os.path.join(model_dir, "stageSelectIsland.x")
sea_export_path = os.path.join(model_dir, "stageSelectSea.x")
preview_path = os.path.join(model_dir, "world1_preview.png")
bpy.context.preferences.filepaths.save_version = 0

portal_layout = [
    (-15.0, -11.5, 0.8),
    (-12.8, -9.4, 0.7),
    (-7.0, -6.0, 1.1),
    (0.0, -6.0, 2.9),
    (7.0, -6.0, 1.3),
    (7.0, -4.0, 2.6),
    (1.0, -4.0, 4.6),
    (-4.0, -4.0, 4.1),
    (-3.0, 0.0, 5.7),
    (2.0, 0.0, 6.5),
]
for index, portal_position in enumerate(portal_layout):
    position_x, position_y, ground_height = portal_position
    prefix = f"RF1_Portal_{index:02d}"
    base = bpy.data.objects[f"{prefix}_Base"]
    inset = bpy.data.objects[f"{prefix}_Inset"]
    ring = bpy.data.objects[f"{prefix}_Ring"]

    base.location.x = position_x
    base.location.y = position_y
    base.location.z = ground_height + 0.45
    inset.location.x = position_x
    inset.location.y = position_y
    inset.location.z = ground_height + 0.625
    ring.location.x = position_x
    ring.location.y = position_y
    ring.location.z = ground_height + 0.77

    base.scale.x = 0.5
    base.scale.y = 0.5
    inset.scale.x = 0.5
    inset.scale.y = 0.5
    ring.scale.x = 0.5
    ring.scale.y = 0.5

for material in bpy.data.materials:
    material["_x_power"] = 500.0

preview_camera = bpy.context.scene.camera
if preview_camera is not None:
    preview_camera.location = Vector((0.0, -26.0, 18.0))
    preview_target = Vector((0.0, 0.0, 3.0))
    preview_camera.rotation_euler = (preview_target - preview_camera.location).to_track_quat("-Z", "Y").to_euler()
    preview_camera.data.lens = 28.0

sea_object_names = {
    "StageSelect_DeepSea.001",
    "StageSelect_ShallowWaterRing.001",
    "StageSelect_FoamArc_00.001",
    "StageSelect_FoamArc_01.001",
    "StageSelect_FoamArc_02.001",
    "RF1_Portal_00_Ring",
    "RF1_Portal_09_Ring",
    "RF1_FoamBreak_00",
    "RF1_FoamBreak_01",
    "RF1_FoamBreak_02",
    "RF1_FoamBreak_03",
    "RF1_FoamBreak_04",
    "RF1_FoamBreak_05",
    "RF1_FoamBreak_06",
}


def export_objects(filepath, object_names):
    bpy.ops.object.select_all(action="DESELECT")
    selected_objects = []
    for object_name in object_names:
        export_object = bpy.data.objects.get(object_name)
        if export_object is None or export_object.type != "MESH":
            raise RuntimeError(f"Export mesh was not found: {object_name}")
        export_object.select_set(True)
        selected_objects.append(export_object)

    bpy.context.view_layer.objects.active = selected_objects[0]
    result = bpy.ops.export_scene.directx_x(
        filepath=filepath,
        use_selection=True,
        axis_forward="-Z",
        axis_up="Y",
    )
    if "FINISHED" not in result:
        raise RuntimeError(f"Official DirectX X export failed: {filepath}")


island_object_names = {
    obj.name
    for obj in bpy.data.objects
    if obj.type == "MESH" and obj.name not in sea_object_names
}

bpy.ops.wm.save_as_mainfile(filepath=bpy.data.filepath)
export_objects(island_export_path, island_object_names)
export_objects(sea_export_path, sea_object_names)

bpy.context.scene.render.filepath = preview_path
bpy.ops.render.render(write_still=True)

print("Arranged portals:", len(portal_layout))
print("Preview camera:", tuple(preview_camera.location) if preview_camera is not None else None)
