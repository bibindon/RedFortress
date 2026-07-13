param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Command,

    [string]$PipeName = "RedFortress.Debug",

    [int]$TimeoutMilliseconds = 5000
)

$pipe = [System.IO.Pipes.NamedPipeClientStream]::new(
    ".",
    $PipeName,
    [System.IO.Pipes.PipeDirection]::InOut,
    [System.IO.Pipes.PipeOptions]::None)

try {
    $pipe.Connect($TimeoutMilliseconds)
    $writer = [System.IO.StreamWriter]::new($pipe)
    $writer.AutoFlush = $true
    $reader = [System.IO.StreamReader]::new($pipe)

    $writer.WriteLine($Command)
    $response = $reader.ReadLine()
    if ($null -eq $response) {
        throw "The game closed the debug RPC pipe without a response."
    }
    $response
}
finally {
    $pipe.Dispose()
}
