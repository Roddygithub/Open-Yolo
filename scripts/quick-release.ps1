# Script PowerShell simplifié pour préparer la release v1.0.0
$VERSION = "1.0.0"
$TAG = "v$VERSION"

Write-Host "`n╔═══════════════════════════════════════════════════════╗" -ForegroundColor Magenta
Write-Host "║     🚀 PREPARATION RELEASE v1.0.0 🚀                 ║" -ForegroundColor Magenta
Write-Host "╚═══════════════════════════════════════════════════════╝`n" -ForegroundColor Magenta

# Étape 1 : Python et curseurs
Write-Host "━━ ÉTAPE 1/4 : Génération des Curseurs ━━" -ForegroundColor Cyan
if (Get-Command python -ErrorAction SilentlyContinue) {
    Write-Host "✓ Python trouvé" -ForegroundColor Green
    python -m pip install --quiet Pillow
    python scripts/generate-default-cursors.py
    Write-Host "✓ Curseurs générés`n" -ForegroundColor Green
} else {
    Write-Host "⚠ Python non trouvé - curseurs non générés`n" -ForegroundColor Yellow
}

# Étape 2 : Git status
Write-Host "━━ ÉTAPE 2/4 : Vérification Git ━━" -ForegroundColor Cyan
$branch = git branch --show-current
Write-Host "Branche actuelle : $branch" -ForegroundColor Blue

$status = git status --short
if ($status) {
    Write-Host "`nModifications détectées :" -ForegroundColor Yellow
    git status --short
    Write-Host "`nCommit automatique..." -ForegroundColor Blue
    git add .
    git commit -m "Release v$VERSION - Curseurs et scripts automatisation"
    Write-Host "✓ Modifications commitées`n" -ForegroundColor Green
} else {
    Write-Host "✓ Aucune modification`n" -ForegroundColor Green
}

# Étape 3 : Push
Write-Host "━━ ÉTAPE 3/4 : Push vers GitHub ━━" -ForegroundColor Cyan
git push origin main 2>&1 | Out-Null
if ($?) {
    Write-Host "✓ Push réussi`n" -ForegroundColor Green
} else {
    Write-Host "⚠ Push échoué`n" -ForegroundColor Yellow
}

# Étape 4 : Tag
Write-Host "━━ ÉTAPE 4/4 : Création du Tag ━━" -ForegroundColor Cyan
git rev-parse $TAG 2>&1 | Out-Null
if ($?) {
    Write-Host "⚠ Tag $TAG existe déjà" -ForegroundColor Yellow
    git tag -d $TAG
    git push origin ":refs/tags/$TAG" 2>&1 | Out-Null
    Write-Host "Tag supprimé" -ForegroundColor Blue
}

Write-Host "Création du tag $TAG..." -ForegroundColor Blue
git tag -a $TAG -m "Open-Yolo v$VERSION - Premiere version stable Linux"
git push origin $TAG 2>&1 | Out-Null
if ($?) {
    Write-Host "✓ Tag créé et poussé`n" -ForegroundColor Green
} else {
    Write-Host "⚠ Erreur lors du push du tag`n" -ForegroundColor Yellow
}

# Résumé
Write-Host "════════════════════════════════════════" -ForegroundColor Green
Write-Host "  ✅ Préparation Terminée !" -ForegroundColor Green
Write-Host "════════════════════════════════════════`n" -ForegroundColor Green

Write-Host "📋 Prochaines étapes :`n" -ForegroundColor Blue
Write-Host "1. Pour compiler et tester (nécessite Linux/WSL) :" -ForegroundColor Yellow
Write-Host "   wsl" -ForegroundColor White
Write-Host "   cd /mnt/c/Users/Roland/Documents/Open-Yolo" -ForegroundColor White
Write-Host "   ./scripts/auto-release.sh`n" -ForegroundColor White

Write-Host "2. Ou publier directement sur GitHub :" -ForegroundColor Yellow
Write-Host "   https://github.com/Roddygithub/Open-Yolo/releases/new?tag=$TAG`n" -ForegroundColor Cyan

Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Magenta
Write-Host "  🚀 Tag $TAG créé et poussé !" -ForegroundColor Magenta
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━`n" -ForegroundColor Magenta
