param(
    [string]$ExePath = "",
    [int]$TimeoutSeconds = 60,
    [float]$AttackDistance = 2.4,
    [int]$AttackCount = 20
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

function Click-LeftMouseButton {
    Invoke-Rpc "MOUSE_DOWN LEFT" | Out-Null
    Invoke-Rpc "MOUSE_UP LEFT" | Out-Null
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
    if ($null -eq $stageState.nearestEnemy) {
        throw "Stage 1-1 has no living enemy."
    }
    Write-Output "Stage 1-1 started. Nearest enemy distance: $($stageState.nearestEnemy.distance)m"

    Invoke-Rpc "KEY_DOWN W" | Out-Null
    try {
        $nearEnemyState = Wait-GameState {
            param($state)
            $null -ne $state.nearestEnemy -and
                [float]$state.nearestEnemy.distance -le $AttackDistance
        } "an enemy within $AttackDistance meters"
    }
    finally {
        Invoke-Rpc "KEY_UP W" | Out-Null
    }

    $enemyCountBefore = [int]$nearEnemyState.livingEnemyCount
    $enemyHpBefore = [int]$nearEnemyState.nearestEnemy.hp
    Write-Output "Enemy reached. Distance: $($nearEnemyState.nearestEnemy.distance)m, HP: $enemyHpBefore"

    for ($attackIndex = 0; $attackIndex -lt $AttackCount; ++$attackIndex) {
        Click-LeftMouseButton
        Start-Sleep -Milliseconds 450
    }

    $result = Invoke-Rpc "GET_STATE"
    Write-Output "Attacks sent: $AttackCount"
    Write-Output "Living enemies: $enemyCountBefore -> $($result.livingEnemyCount)"
    if ($null -ne $result.nearestEnemy) {
        Write-Output "Nearest remaining enemy HP: $($result.nearestEnemy.hp)"
    }
    else {
        Write-Output "No living enemies remain."
    }
    Write-Output "The game remains running for inspection."
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
}
