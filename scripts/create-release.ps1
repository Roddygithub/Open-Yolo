# Script de création de release pour Open-Yolo
# Ce script va :
# 1. Vérifier les prérequis (Docker)
# 2. Construire l'image de compilation
# 3. Compiler le projet
# 4. Préparer les fichiers pour la release

# Fonction pour afficher des messages avec couleur
function Write-Info {
    param([string]$message)
    Write-Host "[INFO] $message" -ForegroundColor Cyan
}

function Write-Success {
    param([string]$message)
    Write-Host "[SUCCÈS] $message" -ForegroundColor Green
}

function Write-Error {
    param([string]$message)
    Write-Host "[ERREUR] $message" -ForegroundColor Red
    exit 1
}

# Vérifier si Docker est installé
Write-Info "Vérification de l'installation de Docker..."
$dockerInstalled = $false
try {
    $dockerVersion = docker --version
    if ($dockerVersion) {
        Write-Success "Docker est installé : $dockerVersion"
        $dockerInstalled = $true
    }
} catch {
    Write-Error "Docker n'est pas installé ou n'est pas dans le PATH. Veuillez installer Docker Desktop pour Windows."
}

# Vérifier que le démon Docker est en cours d'exécution
try {
    $dockerRunning = docker info *>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Le démon Docker ne semble pas en cours d'exécution. Veuillez démarrer Docker Desktop."
    } else {
        Write-Success "Le démon Docker est en cours d'exécution."
    }
} catch {
    Write-Error "Erreur lors de la vérification du démon Docker."
}

# Vérifier si le fichier Dockerfile.build existe
$dockerfilePath = Join-Path -Path $PSScriptRoot -ChildPath "..\Dockerfile.build"
if (-not (Test-Path $dockerfilePath)) {
    Write-Error "Le fichier Dockerfile.build est introuvable dans le répertoire racine du projet."
}

# Construire l'image Docker
$imageName = "open-yolo-build"
Write-Info "Construction de l'image Docker $imageName..."
Set-Location (Split-Path -Parent $dockerfilePath)

try {
    docker build -t $imageName -f Dockerfile.build .
    if ($LASTEXITCODE -ne 0) {
        Write-Error "Échec de la construction de l'image Docker."
    }
    Write-Success "Image Docker construite avec succès."
} catch {
    Write-Error "Erreur lors de la construction de l'image Docker : $_"
}

# Créer un répertoire pour les artefacts de build
$artifactsDir = Join-Path -Path $PSScriptRoot -ChildPath "..\artifacts"
if (-not (Test-Path $artifactsDir)) {
    New-Item -ItemType Directory -Path $artifactsDir | Out-Null
}

# Lancer un conteneur pour compiler le projet
$containerName = "open-yolo-build-container"
$workspacePath = (Get-Item -Path ".").FullName

Write-Info "Lancement du conteneur de compilation..."
try {
    # Lancer le conteneur en mode détaché
    $containerId = docker run -d `
        --name $containerName `
        -v "${workspacePath}:/workspace" `
        -w /workspace `
        $imageName `
        tail -f /dev/null
    
    Write-Success "Conteneur démarré avec l'ID : $containerId"
    
    # Exécuter les commandes de compilation
    Write-Info "Démarrage de la compilation..."
    
    # Créer le répertoire de build
    docker exec $containerName bash -c "mkdir -p /workspace/build"
    
    # Configurer CMake
    docker exec $containerName bash -c "cd /workspace/build && cmake .. \
        -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_CXX_STANDARD=17 \
        -DCMAKE_CXX_STANDARD_REQUIRED=ON \
        -DCMAKE_CXX_EXTENSIONS=OFF"
    
    # Compiler
    docker exec $containerName bash -c "cd /workspace/build && ninja package"
    
    # Vérifier que les fichiers ont été générés
    $buildDir = Join-Path -Path $PSScriptRoot -ChildPath "..\build"
    $packageFiles = Get-ChildItem -Path $buildDir -Filter "*.tar.*" -File
    
    if ($packageFiles.Count -eq 0) {
        Write-Error "Aucun fichier de package n'a été généré."
    } else {
        Write-Success "Compilation terminée avec succès. Fichiers générés :"
        $packageFiles | ForEach-Object { Write-Host "  - $($_.Name)" }
        
        # Copier les fichiers dans le dossier artifacts
        Copy-Item -Path "$buildDir\*" -Destination $artifactsDir -Recurse -Force
        Write-Success "Les fichiers de build ont été copiés dans : $artifactsDir"
    }
    
} catch {
    Write-Error "Erreur lors de la compilation : $_"
} finally {
    # Nettoyer les conteneurs même en cas d'erreur
    Write-Info "Nettoyage des conteneurs..."
    docker rm -f $containerName *>$null
}

# Afficher les instructions pour la release GitHub
$releaseNotes = @"
# Open-Yolo 1.0.1 - C++17 & CachyOS Support

## Nouveautés
- Support complet de C++17
- Compatibilité avec CachyOS/Arch Linux
- Script de compilation dédié
- Correction des problèmes de compilation avec GCC 15

## Installation sur CachyOS/Arch Linux

### Prérequis
```bash
sudo pacman -S --needed base-devel gtkmm3 sdl2 sdl2_image glew giflib xorg-server-devel libxcb cairo cairomm glm
```

### Installation
1. Téléchargez le package pour Arch Linux ci-dessous
2. Installez-le avec :
   ```bash
   sudo pacman -U open-yolo-1.0.1-1-x86_64.pkg.tar.zst
   ```

## Changelog complet
Voir le fichier [CHANGELOG.md](CHANGELOG.md) pour la liste complète des modifications.
"@

# Enregistrer les notes de version dans un fichier
$releaseNotesPath = Join-Path -Path $artifactsDir -ChildPath "RELEASE_NOTES.md"
$releaseNotes | Out-File -FilePath $releaseNotesPath -Encoding utf8

Write-Success "Préparation de la release terminée !"
Write-Host "`nProchaines étapes :" -ForegroundColor Yellow
Write-Host "1. Allez sur : https://github.com/Roddygithub/Open-Yolo/releases/new"
Write-Host "2. Sélectionnez le tag 'v1.0.1'"
Write-Host "3. Copiez le contenu du fichier : $releaseNotesPath"
Write-Host "4. Téléversez les fichiers depuis : $artifactsDir"
Write-Host "5. Publiez la release !" -ForegroundColor Green
