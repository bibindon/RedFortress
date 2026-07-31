# Stage ground top-surface textures

The top surface of each stage's `stage_ground.x` is textured per world.
The side surface (cliff / pit walls) stays `whiteWall.png` for all stages.

| World | Theme | File | Source | License |
|---|---|---|---|---|
| 1 | Grassland (casual style) | `world1.png` | user-authored | — |
| 2 | Cave / mine | `world2.png` | user-authored | — |
| 3 | Mountain ruins (dusk) | `world3.png` | user-authored | — |
| 4 | Fortress (night) | `world4.png` | user-authored | — |

All four world top textures are user-authored images placed directly in this
folder (`world1.png` .. `world4.png`). They were made seamless (tileable) for
the game's repeating UVs.

Regenerate the `.x` files with `_build_stage_grounds.py` (Blender official
DirectX X exporter). The script maps each stage folder to a world and writes
the matching texture filename into the `StageGroundTop` material.
