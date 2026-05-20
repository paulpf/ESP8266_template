param(
    [string]$IpAddress
)

$ErrorActionPreference = "Stop"

$repoRoot = Resolve-Path "$PSScriptRoot\.."
$secretsBase = Resolve-Path "$repoRoot\.."
$secretsDir = Join-Path $secretsBase "_secrets"
$lastIpFile = Join-Path $secretsDir "last_ota_ip.txt"

if (-not (Test-Path $secretsDir)) {
    New-Item -ItemType Directory -Path $secretsDir | Out-Null
}

$lastIp = ""
if (Test-Path $lastIpFile) {
    $lastIp = (Get-Content $lastIpFile -ErrorAction SilentlyContinue | Select-Object -First 1).Trim()
}

if ([string]::IsNullOrWhiteSpace($IpAddress)) {
    $defaultIp = $lastIp
    if ([string]::IsNullOrWhiteSpace($defaultIp)) { $defaultIp = "" }

    try {
        Add-Type -AssemblyName Microsoft.VisualBasic | Out-Null
        $msg = "ESP32 IP-Adresse fuer OTA Upload eingeben"
        $title = "ESP32 OTA Upload"
        $inputIp = [Microsoft.VisualBasic.Interaction]::InputBox($msg, $title, $defaultIp)

        if ([string]::IsNullOrWhiteSpace($inputIp)) {
            if (-not [string]::IsNullOrWhiteSpace($lastIp)) {
                $IpAddress = $lastIp
            }
        }
        else {
            $IpAddress = $inputIp
        }
    }
    catch {
        if ([string]::IsNullOrWhiteSpace($lastIp)) {
            $IpAddress = Read-Host "ESP32 IP-Adresse fuer OTA Upload eingeben"
        }
        else {
            $inputIp = Read-Host "ESP32 IP-Adresse fuer OTA Upload eingeben [$lastIp]"
            if ([string]::IsNullOrWhiteSpace($inputIp)) {
                $IpAddress = $lastIp
            }
            else {
                $IpAddress = $inputIp
            }
        }
    }
}

if ([string]::IsNullOrWhiteSpace($IpAddress)) {
    Write-Error "Keine IP-Adresse angegeben. Abbruch."
    exit 1
}

Set-Content -Path $lastIpFile -Value $IpAddress -Encoding UTF8

Write-Host "Starte OTA-Upload zu $IpAddress ..." -ForegroundColor Cyan
Push-Location $repoRoot
try {
    & $env:USERPROFILE\.platformio\penv\Scripts\pio.exe run -e az-delivery-devkit-v4-ota --upload-port "$IpAddress" -t upload
}
finally {
    Pop-Location
}
