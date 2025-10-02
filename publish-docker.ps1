# Script PowerShell pour publier la release via Docker
param(
    [Parameter(Mandatory=$true)]
    [string]$GithubToken
)

Write-Host "Building Docker image for release publication..." -ForegroundColor Cyan

# Build l'image Docker
docker build -f Dockerfile.release -t openyolo-release .

if ($LASTEXITCODE -ne 0) {
    Write-Host "Failed to build Docker image" -ForegroundColor Red
    exit 1
}

Write-Host "Running release publication in Docker..." -ForegroundColor Cyan

# Exécuter le conteneur avec le token
docker run --rm `
    -e GITHUB_TOKEN=$GithubToken `
    -v "${PWD}:/workspace" `
    openyolo-release

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nRelease published successfully!" -ForegroundColor Green
} else {
    Write-Host "`nRelease publication failed" -ForegroundColor Red
    exit 1
}
