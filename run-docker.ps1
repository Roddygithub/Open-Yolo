# Script pour construire et exécuter Open-Yolo dans un conteneur Docker

# Arrêter et supprimer les conteneurs existants
Write-Host "Arrêt des conteneurs existants..." -ForegroundColor Cyan
docker-compose down 2>&1 | Out-Null

# Construire l'image Docker
Write-Host "Construction de l'image Docker..." -ForegroundColor Cyan
docker-compose build

if ($LASTEXITCODE -ne 0) {
    Write-Host "Erreur lors de la construction de l'image Docker" -ForegroundColor Red
    exit 1
}

# Démarrer le conteneur
Write-Host "Démarrage du conteneur..." -ForegroundColor Cyan
Write-Host "L'application devrait démarrer dans quelques secondes..." -ForegroundColor Green

try {
    # Démarrer le conteneur en arrière-plan
    docker-compose up -d
    
    # Attendre que le conteneur soit prêt
    Start-Sleep -Seconds 5
    
    # Afficher les logs du conteneur
    docker logs -f open-yolo
}
catch {
    Write-Host "Une erreur s'est produite : $_" -ForegroundColor Red
    Write-Host "Tentative de nettoyage..." -ForegroundColor Yellow
    docker-compose down
    exit 1
}

# Nettoyage lors de la fermeture du script
Register-EngineEvent -SourceIdentifier PowerShell.Exiting -Action {
    Write-Host "Arrêt du conteneur..." -ForegroundColor Cyan
    docker-compose down
}
