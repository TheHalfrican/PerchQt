[CmdletBinding()]
param(
    [string]$Config = 'Release'
)

$ErrorActionPreference = 'Stop'

$candidates = @(
    "$PSScriptRoot\build-mingw\PerchQt.exe",
    "$PSScriptRoot\build\$Config\PerchQt.exe",
    "$PSScriptRoot\build\PerchQt.exe"
)

$exe = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $exe) {
    Write-Host "Executable not found. Running build.ps1..." -ForegroundColor Yellow
    & "$PSScriptRoot\build.ps1" -Config $Config
    $exe = $candidates | Where-Object { Test-Path $_ } | Select-Object -First 1
}

if ($exe) {
    Write-Host "Launching $exe" -ForegroundColor Green
    Start-Process $exe
} else {
    Write-Host "Build did not produce PerchQt.exe" -ForegroundColor Red
    exit 1
}
