# AGENTS.md - RedFortress

## Project overview

Multi-project C++ DirectX 9 game engine. Most directories are git submodules with their own repos. The main game entrypoint is `RedFortress2/MultiPassRendering/simple-directx9.vcxproj`.

### Key directories

| Directory | Role |
|---|---|
| `RedFortress2/MultiPassRendering/` | Main game application |
| `RedFortressRender/Render/` | Rendering engine library (submodule) |
| `PhysicsLib/PhysicsLib/` | Collision / physics library (submodule) |
| `InputDevice/InputDevice/` | Keyboard/mouse/gamepad input (submodule) |
| `SoundLib/SoundLib/` | Audio library (submodule) |
| `RedFortressCommand/Command/` | In-game command menu UI (submodule) |
| `RedFortress/RedFortress/` | Production game (not the active dev target) |

## Build

- **Toolset**: v145 (Visual Studio 2022)
- **NuGet**: `packages.config` references `Microsoft.DXSDK.D3DX 9.29.952.8`
- **Post-build**: `simple-directx9.vcxproj` copies `res/` to `$(OutDir)res/` via xcopy after every build. Resources are NOT embedded; they must be at `<exe-dir>\res\`.

## File loading: working directory matters

`Render::LoadSettingsCsv()` at `Render.cpp:551` opens `RenderSettings.csv` with a bare relative `std::wifstream`. It does NOT resolve relative to the exe directory. If the process working directory differs from the exe location, the file silently fails to load and all settings fall back to hardcoded defaults with no error.

- When launching from Visual Studio, check **Project Properties → Debugging → Working Directory**.
- The `res/` xcopy ensures files exist near the exe, but only the working directory determines if relative paths resolve correctly.

## Configuration system

### RenderSettings.csv

`Render::ApplySettings()` reads key=value pairs and calls the appropriate setter methods. New settings need both:
1. A setter method in the `Render` class
2. A `m_settings.find(L"...")` call in `ApplySettings()`

### Moving platforms (3-CSV system)

Adding a moving platform requires entries in **3 CSV files** with the **same CSV ID**:

| File | Purpose |
|---|---|
| `XFileList_simple.csv` | Model mesh + placement (uses `loadType=normal`) |
| `XFileListPhysics.csv` | Collision body (uses `Move=y`) |
| `XFileListMove.csv` | Animation: Start/End positions + Duration. `RenderID` and `PhysicsID` must match the CSV ID. |

The game loop at `main.cpp:328-350` automatically syncs all platforms from `g_Render.GetMovingPlatforms()` to the physics engine. No code changes needed.

### Base resolution

All 2D coordinates are in a **1600×900** base resolution (`Common::BASE_W=1600, Common::BASE_H=900`). The renderer scales to the actual screen via `Common::ScaledPoint()` and `Common::ScaledSize()`.

## Collision character settings

The player uses a **Cylinder** collision shape. Two settings must stay consistent:
- `settings.height` = total cylinder height
- `settings.collisionCenterY` = height / 2 (center from feet)

`SettingsState::SetCylinderHeight(h)` should match `settings.height` in the mover.

## Command library (UI system)

`RedFortressCommand/Command/` defines the UI command menu. The game provides implementations of three interfaces:
- `NSCommand::ISprite` — cursor rendering (game uses `g_Render.DrawImage()`)
- `NSCommand::IFont` — text rendering (game uses `g_Render.DrawTextExCenter()`)
- `NSCommand::ISoundEffect` — audio feedback

All mouse input to the command system goes through `ConvertMouseToBaseResolution()` before hitting `g_command.MouseMove()` / `g_command.Click()`.

## CharacterMover jump detection

`CharacterMover::JustJumped()` returns `true` only on the frame a jump actually succeeded (physics confirmed). Use this instead of keyboard input to detect jump events, as the input may not result in a valid jump (e.g., air jumps exhausted).
