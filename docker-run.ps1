# Vérifier si Docker est en cours d'exécution
$dockerRunning = (Get-Service "Docker" -ErrorAction SilentlyContinue) -and ((Get-Service "Docker").Status -eq 'Running')

if (-not $dockerRunning) {
    Write-Host "Docker n'est pas en cours d'exécution. Démarrage de Docker..."
    Start-Service -Name "Docker" -ErrorAction SilentlyContinue
    
    # Attendre que Docker démarre
    $timeout = 60 # 60 secondes de timeout
    $elapsed = 0
    $dockerRunning = $false
    
    while ($elapsed -lt $timeout) {
        try {
            $dockerInfo = docker info 2>&1
            if ($LASTEXITCODE -eq 0) {
                $dockerRunning = $true
                break
            }
        } catch {
            # Ignorer les erreurs
        }
        
        Start-Sleep -Seconds 2
        $elapsed += 2
        Write-Host "Attente du démarrage de Docker... ($elapsed/$timeout secondes)"
    }
    
    if (-not $dockerRunning) {
        Write-Error "Impossible de démarrer Docker. Veuillez le démarrer manuellement et réessayer."
        exit 1
    }
}

# Construire l'image Docker
Write-Host "Construction de l'image Docker..."
docker build -t open-yolo .

if ($LASTEXITCODE -ne 0) {
    Write-Error "Échec de la construction de l'image Docker."
    exit $LASTEXITCODE
}

# Créer un volume pour les données persistantes si nécessaire
$volumeExists = docker volume ls -q | Select-String "open-yolo-data"
if (-not $volumeExists) {
    Write-Host "Création du volume pour les données persistantes..."
    docker volume create open-yolo-data
}

# Lancer le conteneur
Write-Host "Lancement du conteneur..."
$currentDir = Get-Location

docker run -it --rm `
    -v "${currentDir}:/home/builder/Open-Yolo" `
    -v "open-yolo-data:/home/builder/.local/share/open-yolo" `
    -e "DISPLAY=host.docker.internal:0.0" `
    --name open-yolo-container `
    open-yolo
