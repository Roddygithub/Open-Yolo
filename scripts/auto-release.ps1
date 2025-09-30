# Script PowerShell d'automatisation de la release v1.0.0 pour Windows
# Ce script prépare tout pour la release mais nécessite WSL pour la compilation

$VERSION = "1.0.0"
$TAG = "v$VERSION"

Write-Host ""
Write-Host "╔═══════════════════════════════════════════════════════╗" -ForegroundColor Magenta
Write-Host "║                                                       ║" -ForegroundColor Magenta
Write-Host "║     🚀 AUTOMATISATION RELEASE v1.0.0 🚀              ║" -ForegroundColor Magenta
Write-Host "║                                                       ║" -ForegroundColor Magenta
Write-Host "║         Open-Yolo - Préparation Windows               ║" -ForegroundColor Magenta
Write-Host "║                                                       ║" -ForegroundColor Magenta
Write-Host "╚═══════════════════════════════════════════════════════╝" -ForegroundColor Magenta
Write-Host ""

# Vérifier Python
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "  ÉTAPE 1/5 : Vérification Python" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

if (Get-Command python -ErrorAction SilentlyContinue) {
    Write-Host "✓ Python trouvé" -ForegroundColor Green
    
    # Installer Pillow si nécessaire
    Write-Host "ℹ Installation de Pillow..." -ForegroundColor Blue
    python -m pip install --quiet Pillow
    
    # Générer les curseurs
    Write-Host "ℹ Génération des curseurs..." -ForegroundColor Blue
    python scripts/generate-default-cursors.py
    Write-Host "✓ Curseurs générés" -ForegroundColor Green
} else {
    Write-Host "⚠ Python non trouvé" -ForegroundColor Yellow
    Write-Host "Installation : https://www.python.org/downloads/" -ForegroundColor Yellow
}
Write-Host ""

# Vérifier Git
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "  ÉTAPE 2/5 : Vérification Git" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

if (Get-Command git -ErrorAction SilentlyContinue) {
    Write-Host "✓ Git trouvé" -ForegroundColor Green
    
    $currentBranch = git branch --show-current
    Write-Host "ℹ Branche actuelle : $currentBranch" -ForegroundColor Blue
    
    # Vérifier les modifications
    $status = git status --short
    if ($status) {
        Write-Host "ℹ Modifications non commitées détectées" -ForegroundColor Blue
        Write-Host $status
        Write-Host ""
        
        $commit = Read-Host "Commiter les changements ? (Y/n)"
        if (($commit -ne "n") -and ($commit -ne "N")) {
            git add .
            git commit -m "Release v$VERSION - Curseurs par défaut et scripts d'automatisation"
            Write-Host "✓ Modifications commitées" -ForegroundColor Green
        }
    }
    else {
        Write-Host "✓ Aucune modification non commitée" -ForegroundColor Green
    }
}
else {
    Write-Host "✗ Git non trouvé" -ForegroundColor Red
    exit 1
}
Write-Host ""

# Synchronisation GitHub
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "  ÉTAPE 3/5 : Synchronisation GitHub" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Write-Host "ℹ Push vers GitHub..." -ForegroundColor Blue
git push origin main 2>&1 | Out-Null
if ($LASTEXITCODE -eq 0) {
    Write-Host "✓ Changements poussés" -ForegroundColor Green
}
else {
    Write-Host "⚠ Impossible de push (vérifier la connexion)" -ForegroundColor Yellow
}
Write-Host ""

# Création du tag
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "  ÉTAPE 4/5 : Création du Tag v$VERSION" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

git rev-parse $TAG 2>&1 | Out-Null
$tagExists = ($LASTEXITCODE -eq 0)

if ($tagExists) {
    Write-Host "⚠ Le tag $TAG existe déjà" -ForegroundColor Yellow
    $recreate = Read-Host "Supprimer et recréer ? (y/N)"
    if (($recreate -eq "y") -or ($recreate -eq "Y")) {
        git tag -d $TAG
        git push origin ":refs/tags/$TAG" 2>&1 | Out-Null
        Write-Host "ℹ Tag supprimé" -ForegroundColor Blue
        $tagExists = $false
    }
}

if (-not $tagExists) {
    Write-Host "ℹ Création du tag $TAG..." -ForegroundColor Blue
    
    $tagMessage = @"
Open-Yolo v$VERSION - Premiere version stable Linux

Fonctionnalites principales :
- Curseurs personnalises et animes (GIF)
- Support multi-ecrans et HiDPI
- Effets visuels (ombre, lueur)
- Interface graphique GTK
- Raccourcis clavier personnalisables
- Rendu GPU optimise (< 1% CPU)
- Packaging DEB/RPM/PKGBUILD

Voir CHANGELOG.md pour les details complets.
"@
    
    git tag -a $TAG -m $tagMessage
    Write-Host "✓ Tag créé" -ForegroundColor Green
    
    Write-Host "ℹ Push du tag..." -ForegroundColor Blue
    git push origin $TAG 2>&1 | Out-Null
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ Tag poussé vers GitHub" -ForegroundColor Green
    }
    else {
        Write-Host "⚠ Impossible de push le tag" -ForegroundColor Yellow
    }
}
Write-Host ""

# Instructions finales
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host "  ÉTAPE 5/5 : Instructions Finales" -ForegroundColor Cyan
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
Write-Host ""

Write-Host "════════════════════════════════════════" -ForegroundColor Green
Write-Host "  ✅ Préparation Terminée !" -ForegroundColor Green
Write-Host "════════════════════════════════════════" -ForegroundColor Green
Write-Host ""

Write-Host "📋 Prochaines étapes :" -ForegroundColor Blue
Write-Host ""
Write-Host "Pour compiler et générer les paquets, vous devez utiliser Linux/WSL :" -ForegroundColor Yellow
Write-Host ""
Write-Host "Option 1 : WSL (Windows Subsystem for Linux)" -ForegroundColor Cyan
Write-Host "  wsl" -ForegroundColor White
Write-Host "  cd /mnt/c/Users/Roland/Documents/Open-Yolo" -ForegroundColor White
Write-Host "  ./scripts/auto-release.sh" -ForegroundColor White
Write-Host ""
Write-Host "Option 2 : Docker" -ForegroundColor Cyan
Write-Host "  docker run -it -v ${PWD}:/workspace ubuntu:22.04" -ForegroundColor White
Write-Host "  cd /workspace" -ForegroundColor White
Write-Host "  ./scripts/auto-release.sh" -ForegroundColor White
Write-Host ""
Write-Host "Option 3 : Publication GitHub directe" -ForegroundColor Cyan
Write-Host "  1. Aller sur : https://github.com/Roddygithub/Open-Yolo/releases/new?tag=$TAG" -ForegroundColor White
Write-Host "  2. Copier la description depuis : GITHUB_RELEASE_v$VERSION.md" -ForegroundColor White
Write-Host "  3. Publier la release" -ForegroundColor White
Write-Host ""

Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Magenta
Write-Host "  🚀 Tag v$VERSION créé et poussé !" -ForegroundColor Magenta
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Magenta
Write-Host ""
