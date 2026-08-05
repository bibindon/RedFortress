# Debug RPC

The x64 Debug build exposes a local named pipe named `RedFortress.Debug`.
Release builds do not create the pipe.

Supported commands are:

- `PING`
- `GET_FPS`
- `GET_STATE`
- `LOAD_STAGE <stage-id>`
- `SET_RENDER_QUALITY <LOW|MIDDLE|HIGH>`
- `QUIT`
- `KEY_DOWN <key>`
- `KEY_UP <key>`
- `MOUSE_DOWN <button>`
- `MOUSE_UP <button>`
- `CLEAR_KEYS`
- `CLEAR_INPUT`

Supported key names are `RETURN`, `SPACE`, `ESCAPE`, `LEFT`, `RIGHT`, `UP`,
`DOWN`, `W`, `A`, `S`, `D`, `R`, and `LCONTROL`.

From the repository root, send one command with:

```powershell
.\tools\DebugRpcClient.ps1 GET_STATE
```

Run the complete launch and menu smoke test with:

```powershell
.\tools\RunDebugRpcScenario.ps1
```

Launch stage 1-1, approach the nearest enemy, and repeatedly attack with:

```powershell
.\tools\RunStage1AttackScenario.ps1
```

Measure stage 1-1 performance in the x64 Release build with:

```powershell
& "C:\Program Files\Microsoft Visual Studio\18\Community\MSBuild\Current\Bin\MSBuild.exe" RedFortress2\MultiPassRendering.sln /p:Configuration=Release /p:Platform=x64 /p:EnableDebugRpc=true
.\tools\MeasureStage1Performance.ps1
```

The normal Release build does not expose the RPC pipe unless `EnableDebugRpc=true` is specified.

Launch the game directly into a stage without navigating the title screen. The launcher passes `--stage 4-8` to the game and uses RPC only to verify the loaded stage:

```powershell
.\tools\LaunchDebugStage.ps1 4-8
```

Launch stage 4-2 with LOW render quality, wait 10 seconds, then quit the game:

```powershell
.\tools\MeasureStage42LowQuality.ps1
```
