# Script PowerShell pour publier la release v1.0.0 sur GitHub
param(
    [Parameter(Mandatory=$true)]
    [string]$GithubToken
)

$ErrorActionPreference = "Stop"

$VERSION = "1.0.0"
$TAG = "v$VERSION"
$REPO_OWNER = "Roddygithub"
$REPO_NAME = "Open-Yolo"
$RELEASE_TITLE = "Open-Yolo v$VERSION – Première version stable pour Linux"

Write-Host "`n╔═══════════════════════════════════════════════════════╗" -ForegroundColor Magenta
Write-Host "║                                                       ║" -ForegroundColor Magenta
Write-Host "║     📤 PUBLICATION GITHUB RELEASE v1.0.0 📤          ║" -ForegroundColor Magenta
Write-Host "║                                                       ║" -ForegroundColor Magenta
Write-Host "╚═══════════════════════════════════════════════════════╝" -ForegroundColor Magenta
Write-Host ""

# Fonction pour logger
function Log-Step {
    param([string]$Message)
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
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

# Étape 1 : Vérifier le token
Log-Step "Vérification du token GitHub"

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

# Étape 2 : Vérifier si la release existe
Log-Step "Vérification de la release existante"

try {
    $existingRelease = Invoke-RestMethod -Uri "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases/tags/$TAG" -Headers $headers
    Log-Info "Release existante trouvée (ID: $($existingRelease.id))"
    
    $confirm = Read-Host "Supprimer et recréer la release ? (y/N)"
    if ($confirm -eq 'y' -or $confirm -eq 'Y') {
        Invoke-RestMethod -Uri "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases/$($existingRelease.id)" -Method Delete -Headers $headers | Out-Null
        Log-Success "Release supprimée"
    }
    else {
        Log-Info "Conservation de la release existante"
        exit 0
    }
}
catch {
    Log-Info "Aucune release existante"
}

# Étape 3 : Charger la description
Log-Step "Chargement de la description"

$descriptionFile = "GITHUB_RELEASE_v$VERSION.md"
if (Test-Path $descriptionFile) {
    $description = Get-Content $descriptionFile -Raw
    Log-Success "Description chargée depuis $descriptionFile"
}
else {
    Log-Error "Fichier $descriptionFile non trouvé"
    exit 1
}

# Étape 4 : Créer la release
Log-Step "Création de la release"

$releaseData = @{
    tag_name = $TAG
    name = $RELEASE_TITLE
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

# Étape 5 : Upload des assets
Log-Step "Upload des assets"

$packagesDir = "packages"
if (Test-Path $packagesDir) {
    $files = Get-ChildItem $packagesDir -File
    
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
        }
        catch {
            Log-Error "Échec d'upload de $($file.Name)"
        }
    }
}
else {
    Log-Info "Aucun package trouvé dans $packagesDir"
}

# Étape 6 : Upload des fichiers de packaging
Log-Step "Upload des fichiers de packaging"

$packagingFiles = @("PKGBUILD", "flake.nix")
foreach ($fileName in $packagingFiles) {
    if (Test-Path $fileName) {
        Log-Info "Upload de $fileName..."
        
        $uploadUri = "$uploadUrl?name=$fileName"
        $fileBytes = [System.IO.File]::ReadAllBytes($fileName)
        
        try {
            $asset = Invoke-RestMethod -Uri $uploadUri `
                -Method Post `
                -Headers $headers `
                -Body $fileBytes `
                -ContentType "application/octet-stream"
            
            Log-Success "$fileName uploadé"
        }
        catch {
            Log-Error "Échec d'upload de $fileName"
        }
    }
}

# Résumé final
Write-Host ""
Write-Host "╔═══════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║                                                       ║" -ForegroundColor Green
Write-Host "║          ✅ RELEASE PUBLIÉE AVEC SUCCÈS ! ✅         ║" -ForegroundColor Green
Write-Host "║                                                       ║" -ForegroundColor Green
Write-Host "╚═══════════════════════════════════════════════════════╝" -ForegroundColor Green
Write-Host ""
Write-Host "Release URL: $($release.html_url)" -ForegroundColor Cyan
Write-Host ""
