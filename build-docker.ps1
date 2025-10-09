# Script pour construire le projet avec Docker sous Windows

# Arrêter et supprimer les conteneurs existants
Write-Host "Nettoyage des conteneurs existants..."
docker-compose down 2>&1 | Out-Null

# Construire l'image Docker
Write-Host "Construction de l'image Docker..."
docker build --platform linux/amd64 -t openyolo-builder .

# Créer le répertoire de sortie
$distDir = "$PSScriptRoot\dist"
if (-not (Test-Path $distDir)) {
    New-Item -ItemType Directory -Path $distDir | Out-Null
}

# Exécuter la construction dans un conteneur
Write-Host "Démarrage de la construction..."
docker run --rm --platform linux/amd64 `
    -v "${PSScriptRoot}:/src" `
    -w /src `
    openyolo-builder `
    /bin/bash -c "`
        echo 'Installation des dépendances...' && \
        apt-get update && \
        apt-get install -y build-essential cmake ninja-build git wget curl pkg-config \
            libgtkmm-3.0-dev libcairomm-1.0-dev libsdl2-dev libcurl4-openssl-dev \
            libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev && \
        echo 'Configuration du projet...' && \
        mkdir -p build && \
        cd build && \
        cmake .. -DCMAKE_BUILD_TYPE=Release && \
        echo 'Compilation...' && \
        make -j$(nproc) && \
        echo 'Génération des paquets...' && \
        cpack -G TGZ -C Release && \
        mkdir -p ../dist && \
        mv *.tar.gz ../dist/ && \
        echo 'Construction terminée avec succès!'
    "

# Afficher les paquets générés
if (Test-Path "$distDir\*") {
    Write-Host "`nPaquets générés dans le dossier dist/ :"
    Get-ChildItem -Path $distDir | Select-Object Name, Length, LastWriteTime | Format-Table -AutoSize
} else {
    Write-Host "Aucun paquet n'a été généré. Vérifiez les messages d'erreur ci-dessus." -ForegroundColor Red
}

Write-Host "`nTerminé !"
