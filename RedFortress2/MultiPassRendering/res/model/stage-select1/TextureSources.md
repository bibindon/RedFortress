# World 1 texture sources

Created with the built-in ImageGen tool for this project. The original generated PNGs are kept in this folder, so rebuilding does not require image generation or an API key.

- `rf1_grass_bright.png`: bright spring-green grass with fine leaf detail and minimal soil. Current terrain source, sampled at approximately 4.5 meters per tile. Edited from `rf1_grass_painted.png` with built-in ImageGen; see `TextureBrightGrassPrompt.json`. The original darker source remains available.
- `rf1_rock_painted.png`: weathered coastal limestone, fracture strata, mineral grains, moss and lichen. Used on rocks with box-projected UVs and on steep terrain slopes.
- `rf1_palm_painted.png`: green palm tissue with a vertical central midrib, diagonal veins and sun-faded areas. Each frond uses one complete texture, with its UV V coordinate following the frond length.
- `rf1_island_detail.png`: the runtime terrain albedo, baked in Blender from the current bright grass and rock textures and the existing sand material. The shader blends materials using surface height and slope; lighting is not baked into this image.

Run `tools/RebuildStageSelect1.py` in Blender with `world1.blend` open to rebuild the shader bake, models and previews. The script exports through the official DirectX X add-on.

The exact generation prompts are recorded in `TexturePrompts.json`.
