"""Read-only geometry checks for the forest stage-select scene."""
import csv
import json
import os

import bpy
from mathutils import Vector

out = os.path.dirname(bpy.data.filepath)
scene = bpy.context.scene
depsgraph = bpy.context.evaluated_depsgraph_get()
camera = scene.camera
results = []
with open(os.path.join(out, "Interactables.csv"), encoding="utf-8-sig", newline="") as stream:
    for i, row in enumerate(csv.DictReader(stream)):
        target = Vector((float(row["PosX"]), float(row["PosZ"]), float(row["PosY"]) + 0.2))
        direction = (target - camera.location).normalized()
        hit, pos, normal, index, obj, matrix = scene.ray_cast(depsgraph, camera.location, direction)
        assert hit, row["InteractionID"]
        assert obj.name.startswith("RF4_Portal"), (row["InteractionID"], obj.name)
        results.append({"portal": row["InteractionID"], "visible": True})
corners = []
for x in (-0.98, 0.98):
    for y in (-0.55, 0.55):
        direction = camera.matrix_world.to_quaternion() @ Vector((x, y, -1)).normalized()
        hit, pos, normal, index, obj, matrix = scene.ray_cast(depsgraph, camera.location, direction)
        assert hit, (x, y)
        corners.append({"corner": [x, y], "object": obj.name})
for obj in scene.objects:
    if obj.type == "MESH":
        assert obj.data.validate(verbose=False) is False, obj.name
for mat in bpy.data.materials:
    if mat.use_nodes:
        for node in mat.node_tree.nodes:
            if node.type == "TEX_IMAGE" and node.image:
                assert os.path.isfile(bpy.path.abspath(node.image.filepath)), node.image.filepath
print("FOREST_VALIDATION", json.dumps({"portals": results, "corners": corners,
    "mesh_count": sum(o.type == "MESH" for o in scene.objects),
    "faces": sum(len(o.data.polygons) for o in scene.objects if o.type == "MESH")}), flush=True)
