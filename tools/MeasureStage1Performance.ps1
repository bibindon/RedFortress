param(
    [string]$ExePath = "",
    [int]$TimeoutSeconds = 60,
    [int]$SampleSeconds = 4,
    [switch]$LeaveRunning
)

$ErrorActionPreference = "Stop"

if ([string]::IsNullOrWhiteSpace($ExePath)) {
    $ExePath = Join-Path $PSScriptRoot "..\RedFortress2\x64\Release\simple-directx9.exe"
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

function Measure-Condition {
    param(
        [string]$Name,
        [string[]]$Commands
    )

    foreach ($command in $Commands) {
        Invoke-Rpc $command | Out-Null
    }
    Start-Sleep -Milliseconds 750
    Invoke-Rpc "PROFILE_RESET" | Out-Null
    Start-Sleep -Seconds $SampleSeconds
    $result = Invoke-Rpc "PROFILE_RESULT"
    $averageFrameMilliseconds = 1000.0 / [float]$result.averageFps
    $otherGameLoopMilliseconds = $averageFrameMilliseconds - [float]$result.renderTotalMs
    [PSCustomObject]@{
        Condition = $Name
        AverageFps = [float]$result.averageFps
        FrameMs = [Math]::Round($averageFrameMilliseconds, 2)
        RenderMs = [float]$result.renderTotalMs
        SceneMs = [float]$result.sceneUpdateMs
        GBufferMs = [float]$result.gBufferMs
        MirrorMs = [float]$result.mirrorMs
        MainPassMs = [float]$result.mainPassMs
        PostMs = [float]$result.postEffectMs
        Draw2DMs = [float]$result.draw2DMs
        PresentMs = [float]$result.presentMs
        WaitMs = [float]$result.frameWaitMs
        OtherMs = [Math]::Round($otherGameLoopMilliseconds, 2)
    }
}

try {
    $pipe.Connect($TimeoutSeconds * 1000)
    $writer = [System.IO.StreamWriter]::new($pipe)
    $writer.AutoFlush = $true
    $reader = [System.IO.StreamReader]::new($pipe)

    Invoke-Rpc "PING" | Out-Null
    Wait-GameState { param($state) $state.gameState -eq "Title" } "title screen" | Out-Null
    Press-Key "RETURN"
    Wait-GameState { param($state) $state.gameState -eq "SlideShow" } "opening slideshow" | Out-Null

    Invoke-Rpc "KEY_DOWN SPACE" | Out-Null
    try {
        Wait-GameState {
            param($state)
            $state.gameState -eq "Playing" -and $state.stageId -eq "select1"
        } "stage select after skipping the opening" | Out-Null
    }
    finally {
        Invoke-Rpc "KEY_UP SPACE" | Out-Null
    }

    Wait-GameState {
        param($state)
        $state.selectedPortalId -eq "portal-to-1-1"
    } "stage 1-1 selection" | Out-Null
    Press-Key "RETURN"
    $stageState = Wait-GameState {
        param($state)
        $state.gameState -eq "Playing" -and $state.stageId -eq "1-1"
    } "stage 1-1 start"

    $initialWidth = [int]$stageState.screenWidth
    $initialHeight = [int]$stageState.screenHeight
    $halfWidth = [int]($initialWidth / 2)
    $halfHeight = [int]($initialHeight / 2)

    Invoke-Rpc "SET_INVINCIBLE true" | Out-Null
    Start-Sleep -Seconds 8

    $results = @()
    $results += Measure-Condition "Baseline" @(
        "SET_PLAYER_RENDER true",
        "SET_SKIN_ANIMATION true",
        "SET_PLAYER_PHYSICS true",
        "SET_ENEMY_UPDATE true")
    $results += Measure-Condition "Player render disabled" @(
        "SET_PLAYER_RENDER false")
    $results += Measure-Condition "Skin and bone updates disabled" @(
        "SET_PLAYER_RENDER true",
        "SET_SKIN_ANIMATION false")
    $results += Measure-Condition "Player physics disabled" @(
        "SET_SKIN_ANIMATION true",
        "SET_PLAYER_PHYSICS false")
    $results += Measure-Condition "Enemy update disabled" @(
        "SET_PLAYER_PHYSICS true",
        "SET_ENEMY_UPDATE false")
    $results += Measure-Condition "Baseline restored" @(
        "SET_ENEMY_UPDATE true")
    $results += Measure-Condition "Half resolution" @(
        "SET_RESOLUTION $halfWidth $halfHeight")
    $results += Measure-Condition "Resolution restored" @(
        "SET_RESOLUTION $initialWidth $initialHeight")

    $results | Select-Object Condition, AverageFps, FrameMs, RenderMs, OtherMs | Format-Table -AutoSize
    $results | Select-Object Condition, SceneMs, GBufferMs, MainPassMs, PostMs, PresentMs, WaitMs | Format-Table -AutoSize
}
finally {
    if ($pipe.IsConnected) {
        try {
            Invoke-Rpc "CLEAR_INPUT" | Out-Null
        }
        finally {
            $pipe.Dispose()
        }
    }
    else {
        $pipe.Dispose()
    }

    if (-not $LeaveRunning -and -not $process.HasExited) {
        Stop-Process -Id $process.Id -Force
        $process.WaitForExit()
    }
}
