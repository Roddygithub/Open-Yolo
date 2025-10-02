# Script PowerShell pour générer les packages via Docker

$ErrorActionPreference = "Stop"

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$PackagesDir = Join-Path $ProjectRoot "packages"

Write-Host "`n==========================================" -ForegroundColor Blue
Write-Host "  Package Generation via Docker" -ForegroundColor Blue
Write-Host "==========================================" -ForegroundColor Blue
Write-Host ""

# Créer le répertoire packages
New-Item -ItemType Directory -Force -Path $PackagesDir | Out-Null

# Générer les packages via Docker
Write-Host "Generating packages in Docker container..." -ForegroundColor Yellow
Write-Host "This may take several minutes..." -ForegroundColor Yellow
Write-Host ""

$dockerCommand = @"
apt-get update && apt-get install -y build-essential cmake pkg-config \
    libgtkmm-3.0-dev libsdl2-dev libsdl2-image-dev libgl1-mesa-dev \
    libglew-dev libgif-dev xorg-dev libcairo2-dev libcairomm-1.0-dev \
    file rpm && \
cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DENABLE_LTO=ON && \
cmake --build build-release -j`$(nproc) && \
cd build-release && \
cpack -G DEB && \
cpack -G RPM && \
cpack -G TGZ && \
mv *.deb *.rpm *.tar.gz /workspace/packages/ 2>/dev/null || true
"@

docker run --rm -v "${ProjectRoot}:/workspace" -w /workspace ubuntu:22.04 bash -c $dockerCommand

if ($LASTEXITCODE -ne 0) {
    Write-Host "Package generation failed" -ForegroundColor Red
    exit 1
}

# Copier les fichiers de packaging
Copy-Item -Path "$ProjectRoot\PKGBUILD" -Destination $PackagesDir -Force -ErrorAction SilentlyContinue
Copy-Item -Path "$ProjectRoot\flake.nix" -Destination $PackagesDir -Force -ErrorAction SilentlyContinue

Write-Host ""
Write-Host "==========================================" -ForegroundColor Green
Write-Host "  Packages Generated Successfully!" -ForegroundColor Green
Write-Host "==========================================" -ForegroundColor Green
Write-Host ""
Get-ChildItem $PackagesDir | Format-Table Name, Length, LastWriteTime
Write-Host ""
