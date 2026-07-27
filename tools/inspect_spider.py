"""Inspect the Spider Blender model orientation and left/right."""
import bpy
import sys
import os

source_path = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..",
    "RedFortress2",
    "MultiPassRendering",
    "res",
    "model2",
    "Spider",
    "source.blend",
)

bpy.ops.wm.open_mainfile(filepath=source_path)

armature = bpy.data.objects.get("SpiderArmature")
if armature is None:
    print("ERROR: SpiderArmature not found")
    sys.exit(1)

print(f"Armature: {armature.name}")
print(f"  location: {armature.location[:]}")
print(f"  rotation_euler: {armature.rotation_euler[:]}")
print(f"  scale: {armature.scale[:]}")
print(f"  matrix_world determinant: {armature.matrix_world.to_3x3().determinant():.6f}")

# Check bone positions in rest pose
print("\nBones (rest pose):")
for bone in armature.data.bones:
    head = bone.head_local
    tail = bone.tail_local
    if "Leg" in bone.name or "Head" in bone.name or "Abdomen" in bone.name or "Thorax" in bone.name:
        print(f"  {bone.name}: head=({head.x:.3f}, {head.y:.3f}, {head.z:.3f}) "
              f"tail=({tail.x:.3f}, {tail.y:.3f}, {tail.z:.3f})")

# Check mesh vertex extent
mesh_objects = [obj for obj in bpy.data.objects if obj.type == "MESH" and obj.parent == armature]
for mesh_obj in mesh_objects:
    print(f"\nMesh: {mesh_obj.name}")
    print(f"  location: {mesh_obj.location[:]}")
    print(f"  rotation_euler: {mesh_obj.rotation_euler[:]}")
    print(f"  scale: {mesh_obj.scale[:]}")
    print(f"  matrix_world det: {mesh_obj.matrix_world.to_3x3().determinant():.6f}")

    # Check vertex extent in local space
    xs = [v.co.x for v in mesh_obj.data.vertices]
    ys = [v.co.y for v in mesh_obj.data.vertices]
    zs = [v.co.z for v in mesh_obj.data.vertices]
    print(f"  vertices: {len(xs)}")
    print(f"  X: [{min(xs):.3f}, {max(xs):.3f}]")
    print(f"  Y: [{min(ys):.3f}, {max(ys):.3f}]")
    print(f"  Z: [{min(zs):.3f}, {max(zs):.3f}]")

# Check which direction is "forward" (head vs abdomen)
head_bone = armature.data.bones.get("Head")
abdomen_bone = armature.data.bones.get("Abdomen")
body_bone = armature.data.bones.get("Body")
if head_bone and body_bone:
    h = head_bone.head_local
    b = body_bone.head_local
    print(f"\nForward check:")
    print(f"  Head relative to Body: ({h.x-b.x:.3f}, {h.y-b.y:.3f}, {h.z-b.z:.3f})")
if abdomen_bone and body_bone:
    a = abdomen_bone.head_local
    b = body_bone.head_local
    print(f"  Abdomen relative to Body: ({a.x-b.x:.3f}, {a.y-b.y:.3f}, {a.z-b.z:.3f})")

# Check left/right leg positions
fl = armature.data.bones.get("FrontLeg.L")
fr = armature.data.bones.get("FrontLeg.R")
if fl and fr:
    print(f"\nLeft/Right check:")
    print(f"  FrontLeg.L head: ({fl.head_local.x:.3f}, {fl.head_local.y:.3f}, {fl.head_local.z:.3f})")
    print(f"  FrontLeg.R head: ({fr.head_local.x:.3f}, {fr.head_local.y:.3f}, {fr.head_local.z:.3f})")
    print(f"  L is at {'−X' if fl.head_local.x < 0 else '+X'} side, R is at {'−X' if fr.head_local.x < 0 else '+X'} side")
