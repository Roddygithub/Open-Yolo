#!/bin/bash

# Vérifier si Docker est installé
if ! command -v docker &> /dev/null; then
    echo "Docker n'est pas installé. Installation en cours..."
    sudo pacman -S --noconfirm docker
    sudo systemctl enable --now docker
    sudo usermod -aG docker $USER
    echo "Veuillez vous déconnecter et vous reconnecter pour que les changements prennent effet."
    exit 1
fi

# Nettoyer le cache CMake
echo "Nettoyage du cache CMake..."
rm -rf build/CMakeCache.txt build/CMakeFiles/ 2>/dev/null

# Construire l'image Docker
echo "Construction de l'image Docker..."
docker build -t open-yolo .

# Lancer le conteneur
echo "Lancement de l'application Open-Yolo..."
docker run --rm -it --net=host -e DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix open-yolo
