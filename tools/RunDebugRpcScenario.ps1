param(
    [string]$ExePath = "",
    [int]$TimeoutSeconds = 60
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ExePath)) {
    $ExePath = Join-Path $PSScriptRoot "..\RedFortress2\x64\Debug\simple-directx9.exe"
}
$ExePath = [System.IO.Path]::GetFullPath($ExePath)
if (-not (Test-Path -LiteralPath $ExePath)) {
    throw "Game executable not found: $ExePath"
}

$process = Start-Process -FilePath $ExePath -WorkingDirectory (Split-Path $ExePath) -PassThru
$pipe = [System.IO.Pipes.NamedPipeClientStream]::new(
    ".",
    "RedFortress.Debug",
    [System.IO.Pipes.PipeDirection]::InOut,
    [System.IO.Pipes.PipeOptions]::None)

function Invoke-Rpc {
    param([string]$Command)

    $script:writer.WriteLine($Command)
    $responseLine = $script:reader.ReadLine()
    if ($null -eq $responseLine) {
        throw "The game closed the debug RPC pipe while processing: $Command"
    }
    $response = $responseLine | ConvertFrom-Json
    if (-not $response.ok) {
        throw "RPC command failed: $Command ($($response.error))"
    }
    $response
}

function Press-Key {
    param([string]$Key)

    Invoke-Rpc "KEY_DOWN $Key" | Out-Null
    Invoke-Rpc "KEY_UP $Key" | Out-Null
}

function Wait-GameState {
    param(
        [scriptblock]$Condition,
        [string]$Description
    )

    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    while ([DateTime]::UtcNow -lt $deadline) {
        $state = Invoke-Rpc "GET_STATE"
        if (& $Condition $state) {
            return $state
        }
        Start-Sleep -Milliseconds 100
    }
    throw "Timed out waiting for: $Description"
}

try {
    $pipe.Connect($TimeoutSeconds * 1000)
    $writer = [System.IO.StreamWriter]::new($pipe)
    $writer.AutoFlush = $true
    $reader = [System.IO.StreamReader]::new($pipe)

    Invoke-Rpc "PING" | Out-Null
    $title = Wait-GameState { param($state) $state.gameState -eq "Title" } "title screen"
    $fps = Invoke-Rpc "GET_FPS"
    Write-Output "FPS: $($fps.fps)"

    Press-Key "RETURN"
    Wait-GameState { param($state) $state.gameState -eq "SlideShow" } "opening slideshow" | Out-Null

    Invoke-Rpc "KEY_DOWN SPACE" | Out-Null
    try {
        $stageSelect = Wait-GameState {
            param($state)
            $state.gameState -eq "Playing" -and $state.stageId -eq "select1"
        } "stage select after skipping the opening"
    }
    finally {
        Invoke-Rpc "KEY_UP SPACE" | Out-Null
    }
    Write-Output "Opening skipped: $($stageSelect.stageId)"

    $selectedStage = Wait-GameState {
        param($state)
        $state.selectedPortalId -eq "portal-to-1-1"
    } "stage 1-1 selection"
    Press-Key "RETURN"

    $stage = Wait-GameState {
        param($state)
        $state.gameState -eq "Playing" -and $state.stageId -eq "1-1"
    } "stage 1-1 start"
    Write-Output "Stage started: $($stage.stageId)"

    Press-Key "ESCAPE"
    Wait-GameState { param($state) $state.pauseOpen } "pause menu" | Out-Null

    Press-Key "RIGHT"
    Press-Key "RIGHT"
    Press-Key "RIGHT"
    Press-Key "RETURN"
    Press-Key "RETURN"
    Press-Key "LEFT"

    Invoke-Rpc "KEY_DOWN RETURN" | Out-Null
    if (-not $process.WaitForExit($TimeoutSeconds * 1000)) {
        throw "The game did not exit from the pause menu."
    }
    Write-Output "Game exited from the pause menu."
}
finally {
    if (-not $process.HasExited) {
        Invoke-Rpc "CLEAR_KEYS" | Out-Null
    }
    $pipe.Dispose()
}
