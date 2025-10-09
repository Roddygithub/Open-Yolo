#!/bin/bash

# Vérifier si clang-format est installé
if ! command -v clang-format-14 &> /dev/null; then
    echo "Erreur : clang-format-14 n'est pas installé."
    echo "Veuillez l'installer avec : sudo apt-get install clang-format-14"
    exit 1
fi

# Formater les fichiers source C++
echo "Formatage des fichiers source C++..."
find src include -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cpp" -o -name "*.c" \) \
    -exec clang-format-14 -i -style=file {} \;

echo "Formatage terminé !"
