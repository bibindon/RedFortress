"""Check all enemy Blender models' orientation and L/R."""
import bpy
import os
import sys

MODEL_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..",
    "RedFortress2",
    "MultiPassRendering",
    "res",
    "model2",
)

ARMATURE_NAMES = {
    "Bird": "bird-skeleton",
    "Crab": "rig crab",
    "Frog": "frog_armature",
    "Ghost": "CharacterArmature",
    "Golem": "CharacterArmature",
    "Mushroom": "CharacterArmature",
    "Skeleton": "SkeletonArmature",
    "Spider": "SpiderArmature",
}


def inspect_model(name, source_path):
    bpy.ops.wm.open_mainfile(filepath=source_path)

    arm_name = ARMATURE_NAMES.get(name)
    armature = bpy.data.objects.get(arm_name) if arm_name else None
    if armature is None:
        # Try finding any armature
        for obj in bpy.data.objects:
            if obj.type == "ARMATURE":
                armature = obj
                break

    if armature is None:
        print(f"\n=== {name} === NO ARMATURE FOUND")
        return

    print(f"\n=== {name} (armature: {armature.name}) ===")

    # Check for head-like and body-like bones
    head_names = ["Head", "head", "HEAD", "Nose", "Snout", "Eye"]
    body_names = ["Body", "body", "Hip", "hip", "Pelvis", "pelvis",
                  "Spine", "spine", "Torso", "torso", "Root", "root"]
    tail_names = ["Tail", "tail", "Abdomen", "abdomen"]

    head_bone = None
    for hn in head_names:
        head_bone = armature.data.bones.get(hn)
        if head_bone:
            break

    body_bone = None
    for bn in body_names:
        body_bone = armature.data.bones.get(bn)
        if body_bone:
            break

    if head_bone and body_bone:
        h = head_bone.head_local
        b = body_bone.head_local
        dx, dy, dz = h.x - b.x, h.y - b.y, h.z - b.z
        print(f"  Head-Body: ({dx:.3f}, {dy:.3f}, {dz:.3f}) "
              f"→ forward={'-Y' if dy < -0.01 else '+Y' if dy > 0.01 else '~Y'}")
    else:
        print(f"  Head/Body bones not found")

    # Check L/R
    fl = armature.data.bones.get("FrontLeg.L") or armature.data.bones.get("frontLeg.L")
    fr = armature.data.bones.get("FrontLeg.R") or armature.data.bones.get("frontLeg.R")
    if not fl:
        # Try other naming conventions
        for bone in armature.data.bones:
            if ".L" in bone.name and ("leg" in bone.name.lower() or "arm" in bone.name.lower()):
                fl = bone
                fr = armature.data.bones.get(bone.name.replace(".L", ".R"))
                break

    if fl and fr:
        print(f"  L bone X: {fl.head_local.x:.3f}, R bone X: {fr.head_local.x:.3f} "
              f"→ L at {'-X' if fl.head_local.x < 0 else '+X'}")
    else:
        print(f"  L/R leg bones not found")

    # Mesh extent
    mesh_objs = [o for o in bpy.data.objects if o.type == "MESH"]
    if mesh_objs:
        all_x = []
        all_y = []
        all_z = []
        for mo in mesh_objs:
            for v in mo.data.vertices:
                wco = mo.matrix_world @ v.co
                all_x.append(wco.x)
                all_y.append(wco.y)
                all_z.append(wco.z)
        if all_x:
            print(f"  Mesh world X: [{min(all_x):.3f}, {max(all_x):.3f}]")
            print(f"  Mesh world Y: [{min(all_y):.3f}, {max(all_y):.3f}]")
            print(f"  Mesh world Z: [{min(all_z):.3f}, {max(all_z):.3f}]")


def main():
    for name in sorted(ARMATURE_NAMES.keys()):
        source = os.path.join(MODEL_DIR, name, "source.blend")
        if os.path.isfile(source):
            inspect_model(name, source)
        else:
            print(f"\n=== {name} === source.blend not found")


if __name__ == "__main__":
    main()
