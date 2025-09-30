#!/bin/bash

# Supprimer le répertoire de build
echo "Nettoyage des fichiers de construction..."
rm -rf build/

# Nettoyer les fichiers temporaires
echo "Nettoyage des fichiers temporaires..."
find . -type f -name "*.o" -delete
find . -type f -name "*.a" -delete
find . -type f -name "*.so" -delete
find . -type f -name "*.dylib" -delete
find . -type f -name "*.dll" -delete
find . -type f -name "*.exe" -delete
find . -type f -name "*.user" -delete
find . -type d -name "CMakeFiles" -exec rm -rf {} +
find . -type f -name "CMakeCache.txt" -delete
find . -type f -name "cmake_install.cmake" -delete
find . -type f -name "Makefile" -delete
find . -type f -name "*.vcxproj*" -delete
find . -type f -name "*.sln" -delete
find . -type f -name "*.sln.*" -delete
find . -type f -name "*.vcxproj.*" -delete
find . -type f -name "*.user" -delete
find . -type f -name ".DS_Store" -delete

echo "Nettoyage terminé !"
