#!/bin/bash

set -e

# Vérifier que le script est exécuté depuis la racine du projet
if [ ! -f "CMakeLists.txt" ]; then
    echo "Erreur : Ce script doit être exécuté depuis la racine du projet."
    exit 1
fi

# Créer le répertoire de sortie s'il n'existe pas
mkdir -p dist

# Version du projet
VERSION=$(cat VERSION)

# Construction du paquet source
echo "🔨 Construction du paquet source..."
rm -rf build-package
mkdir -p build-package
cd build-package

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INSTALL_LIBDIR=lib \
    -DBUILD_TESTS=OFF \
    -DBUILD_DOCS=OFF \
    -DINSTALL_GTEST=OFF

cpack -G "TGZ;DEB;RPM" -C Release

# Déplacer les paquets dans le répertoire dist
mv *.tar.gz *.deb *.rpm ../dist/

# Afficher un résumé
echo "\n📦 Paquets générés avec succès :"
ls -lh ../dist/

# Vérifier les dépendances pour les paquets DEB
echo -e "\n🔍 Vérification des dépendances pour les paquets DEB :"
for pkg in ../dist/*.deb; do
    echo -e "\nFichier : $(basename $pkg)"
    dpkg-deb -I $pkg | grep -E 'Depends|Recommends|Suggests' || true
done

echo -e "\n✅ Construction des paquets terminée !"
