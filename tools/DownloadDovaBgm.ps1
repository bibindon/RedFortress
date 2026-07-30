# Download user-approved free BGM (MP3) from DOVA-SYNDROME.
# Reproduces the download page POST form (csrfmiddlewaretoken + track).
# Prefers the loop track when available.

$ErrorActionPreference = "Stop"
[Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12

$outDir = "c:\Users\bibindon\source\repos\bibindon\RedFortress\tools\_downloads\bgm"
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$ua = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0 Safari/537.36"

# "loop" / "non-loop" written with char codes to keep this file ASCII-only.
$loopWord = -join @([char]0x30EB, [char]0x30FC, [char]0x30D7)      # ru-pu
$nonLoopWord = -join @([char]0x975E, $loopWord)                    # hi + ru-pu

$songs = @(
    @{ id = 13787; name = "story" },
    @{ id = 1021;  name = "select" },
    @{ id = 23141; name = "w1_field" },
    @{ id = 12420; name = "w1_swamp" },
    @{ id = 5103;  name = "w2_cave" },
    @{ id = 12728; name = "w2_mine" },
    @{ id = 7674;  name = "w3_ruins" },
    @{ id = 2450;  name = "w3_trail" },
    @{ id = 280;   name = "w4_fortress" },
    @{ id = 2096;  name = "w4_assault" },
    @{ id = 2406;  name = "boss" },
    @{ id = 2623;  name = "boss2" },
    @{ id = 10232; name = "lastboss" }
)

foreach ($song in $songs)
{
    $id = $song.id
    $name = $song.name
    $outFile = Join-Path $outDir "$name.mp3"
    if (Test-Path $outFile)
    {
        Write-Host "[skip] $name.mp3 already exists"
        continue
    }

    $url = "https://dova-s.jp/bgm/detail/$id/download"
    Write-Host "[get ] $url"
    $page = Invoke-WebRequest -Uri $url -SessionVariable session -UserAgent $ua -UseBasicParsing
    $html = [System.Text.Encoding]::UTF8.GetString($page.RawContentStream.ToArray())

    if ($html -notmatch 'name="csrfmiddlewaretoken" value="([^"]+)"')
    {
        Write-Host "[fail] $id : csrf token not found"
        continue
    }
    $token = $Matches[1]

    # Parse track options; prefer the loop version.
    $track = "1"
    $optionMatches = [regex]::Matches($html, '<option value="(\d+)"[^>]*>([^<]*)</option>')
    foreach ($opt in $optionMatches)
    {
        $label = $opt.Groups[2].Value
        if ($label.Contains($loopWord) -and -not $label.Contains($nonLoopWord))
        {
            $track = $opt.Groups[1].Value
            break
        }
    }

    Write-Host "[post] id=$id track=$track -> $name.mp3"
    $body = @{ csrfmiddlewaretoken = $token; track = $track }
    Invoke-WebRequest -Uri $url -Method Post -WebSession $session -UserAgent $ua `
        -Headers @{ Referer = $url } -Body $body -OutFile $outFile -UseBasicParsing

    # Verify MP3 magic bytes (ID3 tag or MPEG frame sync).
    $bytes = [System.IO.File]::ReadAllBytes($outFile)
    $isMp3 = $false
    if ($bytes.Length -gt 1024)
    {
        if (($bytes[0] -eq 0x49 -and $bytes[1] -eq 0x44 -and $bytes[2] -eq 0x33) -or
            ($bytes[0] -eq 0xFF -and (($bytes[1] -band 0xE0) -eq 0xE0)))
        {
            $isMp3 = $true
        }
    }
    if ($isMp3)
    {
        $mb = [math]::Round($bytes.Length / 1MB, 2)
        Write-Host "[ ok ] $name.mp3 ($mb MB)"
    }
    else
    {
        Write-Host "[fail] $name.mp3 is not MP3 (size=$($bytes.Length))"
        Rename-Item $outFile "$outFile.bad" -Force
    }

    Start-Sleep -Seconds 3
}

Write-Host "---- result ----"
Get-ChildItem $outDir | Select-Object Name, @{n='MB';e={[math]::Round($_.Length/1MB,2)}} | Format-Table -AutoSize
