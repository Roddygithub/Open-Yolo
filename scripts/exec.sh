#!/bin/bash

# Configuration
BUILD_DIR="build"
EXECUTABLE="OpenYolo"

# Vérifier si le répertoire de build existe
if [ ! -d "${BUILD_DIR}" ]; then
  echo "Erreur : Le répertoire de build '${BUILD_DIR}' n'existe pas."
  echo "Exécutez d'abord le script build.sh"
  exit 1
fi

# Exécuter l'application
"${BUILD_DIR}/${EXECUTABLE}" "$@"
