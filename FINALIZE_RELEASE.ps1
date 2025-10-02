# Script PowerShell de finalisation complète de la release v1.0.0
# Ce script fait TOUT automatiquement

param(
    [Parameter(Mandatory=$true)]
    [string]$GithubToken
)

$ErrorActionPreference = "Stop"

$VERSION = "1.0.0"
$TAG = "v$VERSION"
$REPO_OWNER = "Roddygithub"
$REPO_NAME = "Open-Yolo"

Write-Host "`n╔═══════════════════════════════════════════════════════╗" -ForegroundColor Magenta
Write-Host "║                                                       ║" -ForegroundColor Magenta
Write-Host "║     🚀 FINALISATION RELEASE v1.0.0 🚀               ║" -ForegroundColor Magenta
Write-Host "║                                                       ║" -ForegroundColor Magenta
Write-Host "║              Open-Yolo - Automatique                  ║" -ForegroundColor Magenta
Write-Host "║                                                       ║" -ForegroundColor Magenta
Write-Host "╚═══════════════════════════════════════════════════════╝" -ForegroundColor Magenta
Write-Host ""

# Fonction de log
function Log-Step {
    param([string]$Message)
    Write-Host "`n━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
    Write-Host "  $Message" -ForegroundColor Cyan
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
    Write-Host ""
}

function Log-Success {
    param([string]$Message)
    Write-Host "✓ $Message" -ForegroundColor Green
}

function Log-Error {
    param([string]$Message)
    Write-Host "✗ $Message" -ForegroundColor Red
}

function Log-Info {
    param([string]$Message)
    Write-Host "ℹ $Message" -ForegroundColor Blue
}

# Étape 1 : Vérifier Docker
Log-Step "Étape 1/6 : Vérification de Docker"

try {
    docker ps | Out-Null
    Log-Success "Docker est opérationnel"
}
catch {
    Log-Error "Docker n'est pas démarré"
    Write-Host "Veuillez démarrer Docker Desktop et réessayer" -ForegroundColor Yellow
    exit 1
}

# Étape 2 : Générer les packages
Log-Step "Étape 2/6 : Génération des packages"

Log-Info "Génération via Docker (cela peut prendre 5-10 minutes)..."
& ".\scripts\generate-packages-docker.ps1"

if ($LASTEXITCODE -ne 0) {
    Log-Error "Échec de génération des packages"
    exit 1
}

Log-Success "Packages générés avec succès"

# Vérifier les packages
$packagesDir = "packages"
if (Test-Path $packagesDir) {
    $packages = Get-ChildItem $packagesDir -File
    Log-Info "Packages disponibles :"
    foreach ($pkg in $packages) {
        Write-Host "  - $($pkg.Name)" -ForegroundColor Gray
    }
}

# Étape 3 : Vérifier le token GitHub
Log-Step "Étape 3/6 : Vérification du token GitHub"

$headers = @{
    "Authorization" = "token $GithubToken"
    "Accept" = "application/vnd.github.v3+json"
}

try {
    $user = Invoke-RestMethod -Uri "https://api.github.com/user" -Headers $headers
    Log-Success "Token valide (utilisateur: $($user.login))"
}
catch {
    Log-Error "Token GitHub invalide"
    exit 1
}

# Étape 4 : Supprimer l'ancienne release si elle existe
Log-Step "Étape 4/6 : Vérification de la release existante"

try {
    $existingRelease = Invoke-RestMethod -Uri "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases/tags/$TAG" -Headers $headers
    Log-Info "Release existante trouvée (ID: $($existingRelease.id))"
    
    Log-Info "Suppression de l'ancienne release..."
    Invoke-RestMethod -Uri "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases/$($existingRelease.id)" -Method Delete -Headers $headers | Out-Null
    Log-Success "Ancienne release supprimée"
}
catch {
    Log-Info "Aucune release existante"
}

# Étape 5 : Créer la nouvelle release
Log-Step "Étape 5/6 : Création de la release GitHub"

# Charger la description
$descriptionFile = "GITHUB_RELEASE_v$VERSION.md"
if (Test-Path $descriptionFile) {
    $description = Get-Content $descriptionFile -Raw
    Log-Success "Description chargée"
}
else {
    Log-Error "Fichier $descriptionFile non trouvé"
    exit 1
}

# Créer la release
$releaseData = @{
    tag_name = $TAG
    name = "Open-Yolo v$VERSION – Première version stable pour Linux"
    body = $description
    draft = $false
    prerelease = $false
} | ConvertTo-Json

try {
    $release = Invoke-RestMethod -Uri "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases" `
        -Method Post `
        -Headers $headers `
        -Body $releaseData `
        -ContentType "application/json"
    
    Log-Success "Release créée (ID: $($release.id))"
    $uploadUrl = $release.upload_url -replace '\{\?name,label\}', ''
}
catch {
    Log-Error "Échec de création de la release"
    Write-Host $_.Exception.Message -ForegroundColor Red
    exit 1
}

# Étape 6 : Upload des packages
Log-Step "Étape 6/6 : Upload des packages"

if (Test-Path $packagesDir) {
    $files = Get-ChildItem $packagesDir -File
    
    $uploaded = 0
    $failed = 0
    
    foreach ($file in $files) {
        Log-Info "Upload de $($file.Name)..."
        
        $uploadUri = "$uploadUrl?name=$($file.Name)"
        $fileBytes = [System.IO.File]::ReadAllBytes($file.FullName)
        
        try {
            $asset = Invoke-RestMethod -Uri $uploadUri `
                -Method Post `
                -Headers $headers `
                -Body $fileBytes `
                -ContentType "application/octet-stream"
            
            Log-Success "$($file.Name) uploadé"
            $uploaded++
        }
        catch {
            Log-Error "Échec d'upload de $($file.Name)"
            $failed++
        }
    }
    
    Write-Host ""
    Log-Info "Résumé : $uploaded uploadés, $failed échecs"
}

# Résumé final
Write-Host ""
Write-Host "╔═══════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║                                                       ║" -ForegroundColor Green
Write-Host "║          ✅ RELEASE PUBLIÉE AVEC SUCCÈS ! ✅         ║" -ForegroundColor Green
Write-Host "║                                                       ║" -ForegroundColor Green
Write-Host "╚═══════════════════════════════════════════════════════╝" -ForegroundColor Green
Write-Host ""
Write-Host "🔗 Release URL:" -ForegroundColor Cyan
Write-Host "   $($release.html_url)" -ForegroundColor Blue
Write-Host ""
Write-Host "📦 Packages uploadés : $uploaded" -ForegroundColor Cyan
Write-Host ""
Write-Host "✅ CI/CD sera déclenché automatiquement" -ForegroundColor Green
Write-Host "   Vérifier : https://github.com/$REPO_OWNER/$REPO_NAME/actions" -ForegroundColor Blue
Write-Host ""
