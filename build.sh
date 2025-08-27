#!/bin/bash

# Script de construction et d'installation d'OpenYolo

# Configuration
BUILD_DIR="build"
INSTALL_PREFIX="${HOME}/.local"
NUM_JOBS=$(nproc)

# Création du répertoire de build
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}" || exit 1

# Configuration avec CMake
cmake .. \
    -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTS=ON \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

# Compilation
make -j"${NUM_JOBS}"

# Installation
make install

# Mise à jour du schéma GSettings
if [ -f "${INSTALL_PREFIX}/share/glib-2.0/schemas/org.openyolo.gschema.xml" ]; then
    glib-compile-schemas "${INSTALL_PREFIX}/share/glib-2.0/schemas/"
fi

echo "\nOpenYolo a été installé avec succès dans ${INSTALL_PREFIX}"
echo "Vous pouvez maintenant lancer l'application avec la commande: ${INSTALL_PREFIX}/bin/OpenYolo"
