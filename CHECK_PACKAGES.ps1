# Script rapide pour vérifier l'état des packages

Write-Host "`n📦 Vérification des packages..." -ForegroundColor Cyan
Write-Host ""

$packagesDir = "packages"

if (Test-Path $packagesDir) {
    $files = Get-ChildItem $packagesDir -File
    
    Write-Host "Packages trouvés : $($files.Count)" -ForegroundColor Yellow
    Write-Host ""
    
    $hasDeb = $false
    $hasRpm = $false
    $hasTgz = $false
    $hasPkgbuild = $false
    $hasFlake = $false
    
    foreach ($file in $files) {
        $size = [math]::Round($file.Length / 1MB, 2)
        Write-Host "  ✓ $($file.Name) ($size MB)" -ForegroundColor Green
        
        if ($file.Name -like "*.deb") { $hasDeb = $true }
        if ($file.Name -like "*.rpm") { $hasRpm = $true }
        if ($file.Name -like "*.tar.gz") { $hasTgz = $true }
        if ($file.Name -eq "PKGBUILD") { $hasPkgbuild = $true }
        if ($file.Name -eq "flake.nix") { $hasFlake = $true }
    }
    
    Write-Host ""
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
    Write-Host "  Résumé" -ForegroundColor Cyan
    Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" -ForegroundColor Cyan
    Write-Host ""
    
    $status = if ($hasDeb) { "✓" } else { "✗" }
    $color = if ($hasDeb) { "Green" } else { "Red" }
    Write-Host "  $status DEB (Ubuntu/Debian)" -ForegroundColor $color
    
    $status = if ($hasRpm) { "✓" } else { "✗" }
    $color = if ($hasRpm) { "Green" } else { "Red" }
    Write-Host "  $status RPM (Fedora/RHEL)" -ForegroundColor $color
    
    $status = if ($hasTgz) { "✓" } else { "✗" }
    $color = if ($hasTgz) { "Green" } else { "Red" }
    Write-Host "  $status TGZ (Portable)" -ForegroundColor $color
    
    $status = if ($hasPkgbuild) { "✓" } else { "✗" }
    $color = if ($hasPkgbuild) { "Green" } else { "Red" }
    Write-Host "  $status PKGBUILD (Arch)" -ForegroundColor $color
    
    $status = if ($hasFlake) { "✓" } else { "✗" }
    $color = if ($hasFlake) { "Green" } else { "Red" }
    Write-Host "  $status flake.nix (NixOS)" -ForegroundColor $color
    
    Write-Host ""
    
    if ($hasDeb -and $hasRpm -and $hasTgz -and $hasPkgbuild -and $hasFlake) {
        Write-Host "✅ Tous les packages sont prêts !" -ForegroundColor Green
        Write-Host ""
        Write-Host "Vous pouvez maintenant publier la release avec :" -ForegroundColor Cyan
        Write-Host "  .\FINALIZE_RELEASE.ps1 -GithubToken `"votre_token`"" -ForegroundColor Yellow
    }
    else {
        Write-Host "⏳ Packages en cours de génération..." -ForegroundColor Yellow
        Write-Host ""
        Write-Host "Attendez que Docker termine la compilation." -ForegroundColor Gray
    }
}
else {
    Write-Host "✗ Dossier packages non trouvé" -ForegroundColor Red
}

Write-Host ""
