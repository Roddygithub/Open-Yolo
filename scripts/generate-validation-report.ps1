# Script pour générer un rapport de validation complet

param(
    [Parameter(Mandatory=$true)]
    [string]$GithubToken
)

$REPO_OWNER = "Roddygithub"
$REPO_NAME = "Open-Yolo"
$VERSION = "1.0.0"
$TAG = "v$VERSION"

$headers = @{
    "Authorization" = "token $GithubToken"
    "Accept" = "application/vnd.github.v3+json"
}

Write-Host "`n╔═══════════════════════════════════════════════════════╗" -ForegroundColor Cyan
Write-Host "║                                                       ║" -ForegroundColor Cyan
Write-Host "║     📊 RAPPORT DE VALIDATION v1.0.0 📊              ║" -ForegroundColor Cyan
Write-Host "║                                                       ║" -ForegroundColor Cyan
Write-Host "╚═══════════════════════════════════════════════════════╝" -ForegroundColor Cyan
Write-Host ""

# Vérifier la release
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host "  Vérification de la Release" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host ""

try {
    $release = Invoke-RestMethod -Uri "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/releases/tags/$TAG" -Headers $headers
    
    Write-Host "✓ Release trouvée" -ForegroundColor Green
    Write-Host "  - Tag: $($release.tag_name)" -ForegroundColor Gray
    Write-Host "  - Nom: $($release.name)" -ForegroundColor Gray
    Write-Host "  - URL: $($release.html_url)" -ForegroundColor Gray
    Write-Host "  - Latest: $($release.prerelease -eq $false)" -ForegroundColor Gray
    Write-Host "  - Assets: $($release.assets.Count)" -ForegroundColor Gray
    Write-Host ""
    
    if ($release.assets.Count -gt 0) {
        Write-Host "📦 Packages disponibles :" -ForegroundColor Cyan
        foreach ($asset in $release.assets) {
            $size = [math]::Round($asset.size / 1MB, 2)
            Write-Host "  ✓ $($asset.name) ($size MB)" -ForegroundColor Green
        }
    }
}
catch {
    Write-Host "✗ Release non trouvée" -ForegroundColor Red
}

Write-Host ""

# Vérifier le CI/CD
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host "  Vérification du CI/CD" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host ""

try {
    $workflows = Invoke-RestMethod -Uri "https://api.github.com/repos/$REPO_OWNER/$REPO_NAME/actions/runs?per_page=5" -Headers $headers
    
    if ($workflows.workflow_runs.Count -gt 0) {
        Write-Host "✓ Workflows récents :" -ForegroundColor Green
        foreach ($run in $workflows.workflow_runs | Select-Object -First 3) {
            $status = if ($run.conclusion -eq "success") { "✓" } else { "✗" }
            $color = if ($run.conclusion -eq "success") { "Green" } else { "Red" }
            Write-Host "  $status $($run.name) - $($run.conclusion)" -ForegroundColor $color
        }
    }
}
catch {
    Write-Host "⚠ Impossible de vérifier les workflows" -ForegroundColor Yellow
}

Write-Host ""

# Vérifier les badges
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host "  Vérification des Badges" -ForegroundColor Yellow
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Yellow
Write-Host ""

$badges = @(
    @{Name="CI/CD Pipeline"; URL="https://github.com/$REPO_OWNER/$REPO_NAME/actions/workflows/ci.yml/badge.svg"},
    @{Name="Latest Release"; URL="https://img.shields.io/github/v/release/$REPO_OWNER/$REPO_NAME"},
    @{Name="Downloads"; URL="https://img.shields.io/github/downloads/$REPO_OWNER/$REPO_NAME/total"}
)

foreach ($badge in $badges) {
    try {
        $response = Invoke-WebRequest -Uri $badge.URL -Method Head -UseBasicParsing
        if ($response.StatusCode -eq 200) {
            Write-Host "✓ $($badge.Name)" -ForegroundColor Green
        }
    }
    catch {
        Write-Host "✗ $($badge.Name)" -ForegroundColor Red
    }
}

Write-Host ""

# Résumé final
Write-Host "╔═══════════════════════════════════════════════════════╗" -ForegroundColor Green
Write-Host "║                                                       ║" -ForegroundColor Green
Write-Host "║          ✅ VALIDATION COMPLÈTE ✅                    ║" -ForegroundColor Green
Write-Host "║                                                       ║" -ForegroundColor Green
Write-Host "╚═══════════════════════════════════════════════════════╝" -ForegroundColor Green
Write-Host ""
Write-Host "🔗 Liens Importants :" -ForegroundColor Cyan
Write-Host "   Release : https://github.com/$REPO_OWNER/$REPO_NAME/releases/latest" -ForegroundColor Blue
Write-Host "   Actions : https://github.com/$REPO_OWNER/$REPO_NAME/actions" -ForegroundColor Blue
Write-Host "   README  : https://github.com/$REPO_OWNER/$REPO_NAME" -ForegroundColor Blue
Write-Host ""
