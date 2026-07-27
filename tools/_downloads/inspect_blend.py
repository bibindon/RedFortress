"""Inspect a saved blend to report armature, meshes, materials, images, actions."""
import sys
import bpy

if "--" not in sys.argv:
    raise RuntimeError("Expected arguments after --")
args = sys.argv[sys.argv.index("--") + 1:]
blend_path = args[0]

bpy.ops.wm.open_mainfile(filepath=blend_path)

print("=" * 60)
print(f"INSPECT: {blend_path}")
print("=" * 60)

print("\n## Objects")
for obj in bpy.context.scene.objects:
    parent_name = obj.parent.name if obj.parent else "(none)"
    print(f"  - {obj.name}  type={obj.type}  parent={parent_name}")

print("\n## Armatures & bones")
for obj in bpy.context.scene.objects:
    if obj.type == "ARMATURE":
        bones = [b.name for b in obj.data.bones]
        print(f"  Armature '{obj.name}': {len(bones)} bones")
        for b in bones[:8]:
            print(f"    bone: {b}")

print("\n## Meshes & materials")
for obj in bpy.context.scene.objects:
    if obj.type != "MESH":
        continue
    mods = [(m.type, getattr(m.object, "name", None)) for m in obj.modifiers]
    print(f"  Mesh '{obj.name}': materials={len(obj.material_slots)} modifiers={mods}")
    for i, slot in enumerate(obj.material_slots):
        mat = slot.material
        if mat is None:
            print(f"    slot[{i}]: None")
            continue
        images = []
        if mat.node_tree:
            for n in mat.node_tree.nodes:
                if n.type == "TEX_IMAGE" and n.image:
                    images.append((n.name, n.image.name, n.image.filepath, n.image.size[0], n.image.size[1]))
        print(f"    slot[{i}]: material='{mat.name}' images={images}")

print("\n## All images in data")
for img in bpy.data.images:
    print(f"  image '{img.name}': filepath='{img.filepath}' size={img.size[0]}x{img.size[1]} packed={img.packed_file is not None}")

print("\n## Actions")
for action in bpy.data.actions:
    print(f"  action '{action.name}': frame_range={list(action.frame_range)}")

# armature animation_data
for obj in bpy.context.scene.objects:
    if obj.type == "ARMATURE":
        ad = obj.animation_data
        if ad:
            print(f"\n## Armature '{obj.name}' animation_data")
            print(f"  active action: {ad.action.name if ad.action else None}")
            if ad.action:
                for track in ad.nla_tracks:
                    print(f"  NLA track '{track.name}':")
                    for strip in track.strips:
                        print(f"    strip '{strip.name}' action='{strip.action.name if strip.action else None}'")
