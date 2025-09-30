#!/bin/bash

# Script pour générer toutes les icônes nécessaires pour OpenYolo
# Ce script doit être exécuté depuis la racine du projet

set -euo pipefail

# Créer le répertoire des icônes s'il n'existe pas
mkdir -p resources/icons/{16x16,24x24,32x32,48x48,64x64,128x128,256x256,512x512,1024x1024,scalable}/apps

# Générer les icônes dans différentes tailles
python3 scripts/generate_icons.py

# Créer un lien symbolique pour la version scalable
ln -sf ../../OpenYolo.svg resources/icons/scalable/apps/OpenYolo.svg

# Afficher un message de confirmation
echo "Toutes les icônes ont été générées avec succès dans le répertoire resources/icons/"
exit 0
