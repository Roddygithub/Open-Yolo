# Script PowerShell pour mettre à jour la release via Docker
param(
    [Parameter(Mandatory=$true)]
    [string]$GithubToken
)

Write-Host "`nUpdating GitHub release v1.0.0 with NixOS support..." -ForegroundColor Cyan

# Utiliser l'image Docker existante
docker run --rm `
    -e GITHUB_TOKEN=$GithubToken `
    -v "${PWD}:/workspace" `
    -w /workspace `
    openyolo-release-simple `
    bash /workspace/scripts/update-release.sh

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nRelease updated successfully!" -ForegroundColor Green
    Write-Host "`nYou can view it at:" -ForegroundColor Cyan
    Write-Host "https://github.com/Roddygithub/Open-Yolo/releases/tag/v1.0.0" -ForegroundColor Blue
}
else {
    Write-Host "`nRelease update failed" -ForegroundColor Red
    exit 1
}
