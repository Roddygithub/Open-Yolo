#!/bin/bash

# Configuration
BUILD_DIR="build"

# Supprimer le répertoire de build
rm -rf "${BUILD_DIR}"

# Supprimer les fichiers générés par CMake
find . -type f \( -name "CMakeCache.txt" -o -name "cmake_install.cmake" \) -delete
find . -type d -name "CMakeFiles" -exec rm -rf {} + 2>/dev/null || true

# Supprimer les fichiers temporaires
find . -type f \( -name "*.o" -o -name "*.a" -o -name "*.so" \) -delete
find . -type f \( -name "*~" -o -name "*.bak" -o -name "*.swp" \) -delete

# Supprimer les répertoires vides
find . -type d -empty -delete 2>/dev/null || true

echo "Nettoyage terminé !"
