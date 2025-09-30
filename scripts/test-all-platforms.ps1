# Script PowerShell de test automatisé multi-plateformes pour Open-Yolo

Write-Host "`n==========================================" -ForegroundColor Blue
Write-Host "  Open-Yolo - Tests Multi-Plateformes" -ForegroundColor Blue
Write-Host "==========================================" -ForegroundColor Blue
Write-Host ""

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$TestResults = @{}

function Test-Platform {
    param(
        [string]$Platform,
        [string]$Dockerfile
    )
    
    Write-Host "`n==========================================" -ForegroundColor Blue
    Write-Host "Testing on: $Platform" -ForegroundColor Blue
    Write-Host "==========================================" -ForegroundColor Blue
    Write-Host ""
    
    # Build Docker image
    Write-Host "Building Docker image..." -ForegroundColor Yellow
    docker build -f "$ProjectRoot\docker\$Dockerfile" -t "openyolo-test-$Platform" $ProjectRoot 2>&1 | Out-Null
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Build successful" -ForegroundColor Green
    }
    else {
        Write-Host "Build failed" -ForegroundColor Red
        $TestResults[$Platform] = "BUILD_FAILED"
        return $false
    }
    
    Write-Host ""
    
    # Run tests
    Write-Host "Running tests..." -ForegroundColor Yellow
    docker run --rm "openyolo-test-$Platform" 2>&1 | Out-Null
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Tests passed" -ForegroundColor Green
        $TestResults[$Platform] = "PASSED"
        return $true
    }
    else {
        Write-Host "Tests failed" -ForegroundColor Red
        $TestResults[$Platform] = "FAILED"
        return $false
    }
}

# Test toutes les plateformes
Set-Location $ProjectRoot

Write-Host "Testing platforms:"
Write-Host "  - Ubuntu 22.04 LTS"
Write-Host "  - Ubuntu 24.04 LTS"
Write-Host "  - Fedora Latest"
Write-Host "  - Arch Linux"
Write-Host "  - CachyOS"
Write-Host "  - Windows (WSL Ubuntu)"
Write-Host "  - NixOS"
Write-Host ""

# Ubuntu 22.04
Test-Platform -Platform "ubuntu-22.04" -Dockerfile "Dockerfile.ubuntu22"

# Ubuntu 24.04
Test-Platform -Platform "ubuntu-24.04" -Dockerfile "Dockerfile.ubuntu24"

# Fedora
Test-Platform -Platform "fedora-latest" -Dockerfile "Dockerfile.fedora"

# Arch Linux
Test-Platform -Platform "archlinux" -Dockerfile "Dockerfile.archlinux"

# CachyOS
Test-Platform -Platform "cachyos" -Dockerfile "Dockerfile.cachyos"

# Windows (WSL)
Test-Platform -Platform "windows-wsl" -Dockerfile "Dockerfile.windows-wsl"

# NixOS
Test-Platform -Platform "nixos" -Dockerfile "Dockerfile.nixos"

# Afficher le résumé
Write-Host "`n==========================================" -ForegroundColor Blue
Write-Host "  Test Summary" -ForegroundColor Blue
Write-Host "==========================================" -ForegroundColor Blue
Write-Host ""

$Total = 0
$Passed = 0
$Failed = 0
$BuildFailed = 0

foreach ($platform in $TestResults.Keys) {
    $Total++
    $result = $TestResults[$platform]
    
    if ($result -eq "PASSED") {
        Write-Host "✓ $platform : PASSED" -ForegroundColor Green
        $Passed++
    }
    elseif ($result -eq "FAILED") {
        Write-Host "✗ $platform : FAILED" -ForegroundColor Red
        $Failed++
    }
    elseif ($result -eq "BUILD_FAILED") {
        Write-Host "✗ $platform : BUILD FAILED" -ForegroundColor Red
        $BuildFailed++
    }
}

Write-Host ""
Write-Host "Total: $Total"
Write-Host "Passed: $Passed" -ForegroundColor Green
Write-Host "Failed: $Failed" -ForegroundColor Red
Write-Host "Build Failed: $BuildFailed" -ForegroundColor Red
Write-Host ""

# Exit avec code d'erreur si des tests ont échoué
if ($Failed -gt 0 -or $BuildFailed -gt 0) {
    Write-Host "Some tests failed!" -ForegroundColor Red
    exit 1
}
else {
    Write-Host "All tests passed!" -ForegroundColor Green
    exit 0
}
