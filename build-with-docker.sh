#!/bin/bash

echo "Démarrage de la construction avec Docker..."

# Construire l'image Docker
echo "Construction de l'image Docker..."
docker build --platform linux/amd64 -t openyolo-builder -f Dockerfile .

# Créer le répertoire pour les paquets
mkdir -p dist

# Exécuter la construction dans un conteneur
echo "Démarrage du conteneur de construction..."
docker run --rm --platform linux/amd64 \
  -v "$(pwd)":/home/developer/OpenYolo \
  -w /home/developer/OpenYolo \
  openyolo-builder \
  /bin/sh -c "
    echo 'Installation des dépendances...' &&
    sudo apt-get update &&
    sudo apt-get install -y libcurl4-openssl-dev &&
    echo 'Construction des paquets...' &&
    cd /home/developer/OpenYolo &&
    mkdir -p build &&
    cd build &&
    cmake .. -DCMAKE_BUILD_TYPE=Release &&
    make -j$(nproc) &&
    cpack -G "TGZ;DEB;RPM" -C Release &&
    mv *.tar.gz *.deb *.rpm ../dist/ &&
    echo 'Construction terminée avec succès!'
  "
