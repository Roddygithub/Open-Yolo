#!/bin/bash

# Fonction pour afficher un message de statut
status() {
    echo -e "\033[1;34m==>\033[0m $1"
}

# Supprimer les répertoires de build
status "Nettoyage des répertoires de construction..."
rm -rf build/ build-*/ Debug/ Release/

# Nettoyer les fichiers temporaires
status "Nettoyage des fichiers temporaires..."

# Fichiers objets et bibliothèques
find . -type f \( -name "*.o" -o -name "*.a" -o -name "*.so" -o -name "*.dylib" \
    -o -name "*.dll" -o -name "*.exe" -o -name "*.dll.a" -o -name "*.pdb" \
    -o -name "*.ilk" -o -name "*.exp" -o -name "*.lib" \
    -o -name "*.sln" -o -name "*.sln.*" -o -name "*.vcxproj*" \) -delete

# Fichiers de configuration et de construction
find . -type d -name "CMakeFiles" -exec rm -rf {} + 2>/dev/null || true
find . -type f \( -name "CMakeCache.txt" -o -name "cmake_install.cmake" \
    -o -name "Makefile" -o -name "*.vcxproj*" -o -name "*.sln*" \
    -o -name "*.xcodeproj" -o -name "*.xcworkspace" -o -name "CMakeScripts" \) -delete

# Fichiers de configuration utilisateur et IDE
find . -type f \( -name "*.user" -o -name "*.suo" -o -name "*.userprefs" \
    -o -name "*.ncb" -o -name "*.sdf" -o -name "*.opensdf" \) -delete

# Fichiers de débogage et de couverture de code
find . -type f \( -name "*.gcno" -o -name "*.gcda" -o -name "*.gcov" \) -delete

# Fichiers de sauvegarde et temporaires
find . -type f \( -name "*~" -o -name "*.bak" -o -name "*.swp" \
    -o -name "*.swo" -o -name ".DS_Store" \) -delete

# Nettoyer les répertoires vides
find . -type d -empty -delete 2>/dev/null || true

# Nettoyer les fichiers de configuration générés
rm -f src/config/ConfigManager.cpp
rm -f src/config/ConfigManager.hpp.gch

# Nettoyer les fichiers de logs
echo "" > logs/open-yolo.log 2>/dev/null || true

status "Nettoyage terminé !"
