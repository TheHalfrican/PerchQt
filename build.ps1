[CmdletBinding()]
param(
    [switch]$Clean,
    [ValidateSet('MinGW', 'MSVC', 'Auto')]
    [string]$Toolchain = 'Auto',
    [string]$Config = 'Release'
)

$ErrorActionPreference = 'Stop'

$projectRoot = $PSScriptRoot

function Resolve-Toolchain {
    param([string]$Hint)

    if ($Hint -eq 'MinGW' -or $Hint -eq 'MSVC') { return $Hint }

    # Auto: prefer MSVC if vcvars is found, otherwise fall back to Qt's MinGW.
    $vcvars = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat'
    if (Test-Path $vcvars) { return 'MSVC' }
    if (Test-Path 'C:\Qt\Tools\mingw1310_64\bin\g++.exe') { return 'MinGW' }
    throw 'No usable toolchain found. Install MSVC 2022 Community or Qt''s MinGW kit.'
}

function Initialize-MsvcEnv {
    if (Get-Command cl -ErrorAction SilentlyContinue) { return }
    $vcvars = 'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat'
    if (-not (Test-Path $vcvars)) { throw "vcvars64.bat not found at $vcvars" }
    cmd /c "`"$vcvars`" && set" | ForEach-Object {
        if ($_ -match '^(.*?)=(.*)$') {
            [System.Environment]::SetEnvironmentVariable($matches[1], $matches[2], 'Process')
        }
    }
}

function Initialize-QtMingwEnv {
    $mingw = 'C:\Qt\Tools\mingw1310_64\bin'
    $ninja = 'C:\Qt\Tools\Ninja'
    $cmake = 'C:\Qt\Tools\CMake_64\bin'
    $env:PATH = "$mingw;$ninja;$cmake;$env:PATH"
}

$toolchain = Resolve-Toolchain $Toolchain
Write-Host "Toolchain: $toolchain" -ForegroundColor Cyan

switch ($toolchain) {
    'MSVC' {
        Initialize-MsvcEnv
        $buildDir = Join-Path $projectRoot 'build'
        $vcpkgToolchain = 'C:/Users/NoahM/DevPkgs/vcpkg/scripts/buildsystems/vcpkg.cmake'
        $configureArgs = @(
            '-S', $projectRoot,
            '-B', $buildDir,
            "-DCMAKE_TOOLCHAIN_FILE=$vcpkgToolchain",
            "-DCMAKE_BUILD_TYPE=$Config"
        )
    }
    'MinGW' {
        Initialize-QtMingwEnv
        $buildDir = Join-Path $projectRoot 'build-mingw'
        $qtPrefix = 'C:/Qt/6.9.1/mingw_64'
        $sdlPrefix = 'C:/Users/NoahM/DevPkgs/SDL2-mingw/SDL2-2.32.8/x86_64-w64-mingw32'
        $configureArgs = @(
            '-S', $projectRoot,
            '-B', $buildDir,
            '-G', 'Ninja',
            "-DCMAKE_BUILD_TYPE=$Config",
            "-DCMAKE_PREFIX_PATH=$qtPrefix;$sdlPrefix",
            "-DCMAKE_CXX_COMPILER=C:/Qt/Tools/mingw1310_64/bin/g++.exe"
        )
    }
}

if ($Clean -and (Test-Path $buildDir)) {
    Write-Host "Removing $buildDir" -ForegroundColor Yellow
    Remove-Item $buildDir -Recurse -Force
}

Write-Host "Configuring..." -ForegroundColor Cyan
cmake @configureArgs
if ($LASTEXITCODE -ne 0) { throw "CMake configure failed" }

Write-Host "Building ($Config)..." -ForegroundColor Cyan
$buildArgs = @('--build', $buildDir, '--parallel')
if ($toolchain -eq 'MSVC') { $buildArgs += @('--config', $Config) }
cmake @buildArgs
if ($LASTEXITCODE -ne 0) { throw "Build failed" }

Write-Host "`nBuild complete: $buildDir" -ForegroundColor Green
