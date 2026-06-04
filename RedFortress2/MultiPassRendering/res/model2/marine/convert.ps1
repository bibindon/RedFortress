param([string]$filePath)

if (-not $filePath) {
    Write-Host "Usage: drag .x file onto convert.bat"
    exit 1
}

if (-not (Test-Path $filePath)) {
    Write-Host "File not found: $filePath"
    exit 1
}

Write-Host "Converting: $filePath"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$tablePath = Join-Path $scriptDir "maptable.tsv"

if (-not (Test-Path $tablePath)) {
    Write-Host "maptable.tsv not found."
    exit 1
}

$replacements = @{}
Get-Content $tablePath -Encoding UTF8 | ForEach-Object {
    $line = $_.Trim()
    if ($line -eq "" -or $line.StartsWith("#")) { return }
    $parts = $line -split "`t", 2
    if ($parts.Length -eq 2) {
        $replacements[$parts[0]] = $parts[1]
    }
}

$bytes = [System.IO.File]::ReadAllBytes($filePath)
$text = [System.Text.Encoding]::GetEncoding(932).GetString($bytes)

$keys = $replacements.Keys | Sort-Object { $_.Length } -Descending
foreach ($key in $keys) {
    $text = $text.Replace($key, $replacements[$key])
}

$remaining = [regex]::Matches($text, '[\u3040-\u9FFF\uFF00-\uFFEF]+').Count
Write-Host "Remaining Japanese: $remaining"

$outBytes = [System.Text.Encoding]::UTF8.GetBytes($text)
[System.IO.File]::WriteAllBytes($filePath, $outBytes)
Write-Host "Done."
