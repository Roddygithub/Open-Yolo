# Script PowerShell simplifié pour publier la release via Docker
param(
    [Parameter(Mandatory=$true)]
    [string]$GithubToken
)

Write-Host "`nBuilding Docker image..." -ForegroundColor Cyan

docker build -f Dockerfile.release-simple -t openyolo-release-simple .

if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to build Docker image" -ForegroundColor Red
    exit 1
}

Write-Host "`nPublishing release to GitHub..." -ForegroundColor Cyan
Write-Host "This will take a few seconds...`n" -ForegroundColor Yellow

docker run --rm -e GITHUB_TOKEN=$GithubToken openyolo-release-simple

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nRelease published successfully!" -ForegroundColor Green
    Write-Host "`nYou can view it at:" -ForegroundColor Cyan
    Write-Host "https://github.com/Roddygithub/Open-Yolo/releases/tag/v1.0.0" -ForegroundColor Blue
}
else {
    Write-Host "`nRelease publication failed" -ForegroundColor Red
    exit 1
}
