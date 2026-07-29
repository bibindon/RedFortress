param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$StageId,
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
if ($StageId -notmatch "^[A-Za-z0-9_-]+$") {
    throw "StageId must contain only ASCII letters, digits, underscores, or hyphens."
}

$existingProcess = Get-Process -Name "simple-directx9" -ErrorAction SilentlyContinue
if ($null -ne $existingProcess) {
    throw "The game is already running. Close it before launching a specific debug stage."
}

$process = Start-Process -FilePath $ExePath -ArgumentList @("--stage", $StageId) -WorkingDirectory (Split-Path $ExePath) -PassThru
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

try {
    $pipe.Connect($TimeoutSeconds * 1000)
    $writer = [System.IO.StreamWriter]::new($pipe)
    $writer.AutoFlush = $true
    $reader = [System.IO.StreamReader]::new($pipe)

    Invoke-Rpc "PING" | Out-Null

    $deadline = [DateTime]::UtcNow.AddSeconds($TimeoutSeconds)
    $state = $null
    while ([DateTime]::UtcNow -lt $deadline) {
        $state = Invoke-Rpc "GET_STATE"
        if ($state.stageId -eq $StageId -and
            ($state.gameState -eq "StageIntro" -or $state.gameState -eq "Playing")) {
            break
        }
        Start-Sleep -Milliseconds 100
    }

    if ($null -eq $state -or $state.stageId -ne $StageId) {
        throw "Timed out while loading stage: $StageId"
    }

    [PSCustomObject]@{
        ProcessId = $process.Id
        StageId = $state.stageId
        GameState = $state.gameState
        LivingEnemyCount = $state.livingEnemyCount
    }
}
catch {
    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id
    }
    throw
}
finally {
    $pipe.Dispose()
}