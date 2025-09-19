# Script de construction pour Windows

# Vérifier si vcpkg est installé
$vcpkgRoot = "$env:USERPROFILE\vcpkg"
$vcpkgExe = "$vcpkgRoot\vcpkg.exe"

if (-not (Test-Path $vcpkgExe)) {
    Write-Host "Installation de vcpkg..."
    Set-Location $env:USERPROFILE
    git clone https://github.com/Microsoft/vcpkg.git
    Set-Location vcpkg
    .\bootstrap-vcpkg.bat
    .\vcpkg.exe integrate install
}

# Installer les dépendances
Write-Host "Installation des dépendances..."
& $vcpkgExe install @(Get-Content vcpkg.json | ConvertFrom-Json).dependencies --triplet x64-windows

# Créer le répertoire de build
$buildDir = "$PSScriptRoot\build"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
}

# Configurer avec CMake
Write-Host "Configuration du projet avec CMake..."
Set-Location $buildDir
& cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$vcpkgRoot\scripts\buildsystems\vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DVCPKG_TARGET_TRIPLET=x64-windows

# Compiler le projet
Write-Host "Compilation du projet..."
& cmake --build . --config Release

# Copier les DLL requises
Write-Host "Copie des DLL requises..."
$dlls = @(
    "$vcpkgRoot\installed\x64-windows\bin\*.dll"
)

foreach ($dll in $dlls) {
    if (Test-Path $dll) {
        Copy-Item -Path $dll -Destination "$buildDir\Release\" -Force -ErrorAction SilentlyContinue
    }
}

Write-Host "Compilation terminée. Vous pouvez exécuter le programme depuis le dossier 'build\Release'."
