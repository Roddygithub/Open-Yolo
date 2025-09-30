# Script PowerShell de génération automatique de tous les packages Open-Yolo

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $ProjectRoot "build-release"
$PackagesDir = Join-Path $ProjectRoot "packages"

Write-Host "`n==========================================" -ForegroundColor Blue
Write-Host "  Open-Yolo Package Generator" -ForegroundColor Blue
Write-Host "==========================================" -ForegroundColor Blue
Write-Host ""

# Nettoyer les anciens builds
if (Test-Path $BuildDir) {
    Write-Host "Cleaning old build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $BuildDir
}

# Créer le répertoire de packages
New-Item -ItemType Directory -Force -Path $PackagesDir | Out-Null

# Configurer CMake
Write-Host "Configuring CMake..." -ForegroundColor Blue
cmake -B $BuildDir `
    -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    -DBUILD_TESTS=OFF `
    -DENABLE_LTO=ON `
    -DENABLE_LOGGING=OFF

if ($LASTEXITCODE -ne 0) {
    Write-Host "CMake configuration failed" -ForegroundColor Red
    exit 1
}

# Compiler
Write-Host "Building project..." -ForegroundColor Blue
cmake --build $BuildDir -j $env:NUMBER_OF_PROCESSORS

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed" -ForegroundColor Red
    exit 1
}

# Générer les packages
Set-Location $BuildDir

Write-Host "`nGenerating packages...`n" -ForegroundColor Blue

# DEB package
Write-Host "Generating DEB package..." -ForegroundColor Yellow
cpack -G DEB 2>&1 | Out-Null
if ($LASTEXITCODE -eq 0) {
    Write-Host "DEB package generated" -ForegroundColor Green
    Move-Item -Path "*.deb" -Destination $PackagesDir -Force -ErrorAction SilentlyContinue
}
else {
    Write-Host "DEB package generation failed" -ForegroundColor Red
}

# RPM package
Write-Host "Generating RPM package..." -ForegroundColor Yellow
cpack -G RPM 2>&1 | Out-Null
if ($LASTEXITCODE -eq 0) {
    Write-Host "RPM package generated" -ForegroundColor Green
    Move-Item -Path "*.rpm" -Destination $PackagesDir -Force -ErrorAction SilentlyContinue
}
else {
    Write-Host "RPM package generation failed" -ForegroundColor Red
}

# TGZ package
Write-Host "Generating TGZ package..." -ForegroundColor Yellow
cpack -G TGZ 2>&1 | Out-Null
if ($LASTEXITCODE -eq 0) {
    Write-Host "TGZ package generated" -ForegroundColor Green
    Move-Item -Path "*.tar.gz" -Destination $PackagesDir -Force -ErrorAction SilentlyContinue
}
else {
    Write-Host "TGZ package generation failed" -ForegroundColor Red
}

# Copier les fichiers de packaging
Write-Host "`nCopying packaging files..." -ForegroundColor Yellow
Copy-Item -Path "$ProjectRoot\PKGBUILD" -Destination $PackagesDir -Force -ErrorAction SilentlyContinue
Copy-Item -Path "$ProjectRoot\flake.nix" -Destination $PackagesDir -Force -ErrorAction SilentlyContinue

# Afficher le résumé
Write-Host "`n==========================================" -ForegroundColor Blue
Write-Host "  Package Generation Complete" -ForegroundColor Blue
Write-Host "==========================================" -ForegroundColor Blue
Write-Host ""
Write-Host "Packages generated in: $PackagesDir"
Write-Host ""
Get-ChildItem $PackagesDir | Format-Table Name, Length, LastWriteTime
Write-Host ""
Write-Host "All packages ready for distribution!" -ForegroundColor Green
