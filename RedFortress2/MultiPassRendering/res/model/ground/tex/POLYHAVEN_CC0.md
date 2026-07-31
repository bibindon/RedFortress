# Poly Haven CC0 textures used for stage ground tops

The top surface of each stage's `stage_ground.x` is textured with one of the
following CC0 ground textures, selected per world according to `STAGE_PLAN.md`:

| World | Theme | File | Poly Haven slug | License |
|---|---|---|---|---|
| 1 | Grassland / wetland | `field_world1_grassland.jpg` | `leaves_forest_ground` | CC0 |
| 2 | Cave / mine | `field_world2_cave.jpg` | `mossy_cobblestone` | CC0 |
| 3 | Mountain ruins (dusk) | `field_world3_ruins.jpg` | `castle_brick_01` | CC0 |
| 4 | Fortress (night) | `field_world4_fortress.jpg` | `brick_moss_001` | CC0 |

All textures are the `Diffuse` map at `1k` resolution (1024x1024 JPG),
downloaded from `https://polyhaven.com/` which publishes every asset under
the CC0 license (public domain, no attribution required).

Download URLs (example, 1k diffuse JPG):

- https://dl.polyhaven.org/file/ph-assets/Textures/jpg/1k/leaves_forest_ground/leaves_forest_ground_diff_1k.jpg
- https://dl.polyhaven.org/file/ph-assets/Textures/jpg/1k/mossy_cobblestone/mossy_cobblestone_diff_1k.jpg
- https://dl.polyhaven.org/file/ph-assets/Textures/jpg/1k/castle_brick_01/castle_brick_01_diff_1k.jpg
- https://dl.polyhaven.org/file/ph-assets/Textures/jpg/1k/brick_moss_001/brick_moss_001_diff_1k.jpg

Regenerate the `.x` files with `_build_stage_grounds.py` (Blender official
DirectX X exporter). The script maps each stage folder to a world and writes
the matching texture filename into the `StageGroundTop` material.
