#!/bin/bash
set -e

echo "Nettoyage du répertoire build..."
rm -rf build dist
mkdir -p build dist

cd build

echo "Configuration avec CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "Compilation..."
make -j$(nproc)

echo "Génération du paquet TGZ..."
cpack -G TGZ -C Release

# Déplacer les paquets dans le répertoire dist
mv *.tar.gz ../dist/

echo "Construction terminée avec succès !"
echo "Paquets disponibles dans le dossier dist/ :"
ls -lh ../dist/
