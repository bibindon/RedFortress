# Debug RPC

The x64 Debug build exposes a local named pipe named `RedFortress.Debug`.
Release builds do not create the pipe.

Supported commands are:

- `PING`
- `GET_FPS`
- `GET_STATE`
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
