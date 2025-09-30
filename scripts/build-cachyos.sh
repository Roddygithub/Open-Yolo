#!/bin/bash

# Script de compilation pour CachyOS/Arch Linux
# Ce script installe les dépendances nécessaires et compile Open-Yolo

set -e  # Arrêter le script en cas d'erreur

# Vérification des privilèges root
if [ "$EUID" -ne 0 ]; then 
    echo "Ce script nécessite des privilèges root. Utilisez 'sudo' ou connectez-vous en tant que root."
    exit 1
fi

# Couleurs pour la sortie
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}[*] Mise à jour du système...${NC}"
pacman -Syu --noconfirm

echo -e "\n${GREEN}[*] Installation des dépendances...${NC}"
pacman -S --needed --noconfirm \
    base-devel cmake ninja pkgconf git \
    gtkmm3 sdl2 sdl2_image \
    glew giflib xorg-server-devel \
    libxcb cairo cairomm glm \
    gcc gcc-libs gcc-fortran

# Vérification de la version de GCC
GCC_VERSION=$(gcc --version | head -n1 | awk '{print $4}')
if [[ "$GCC_VERSION" < "13.2.1" ]]; then
    echo -e "\n${YELLOW}[!] Attention: Votre version de GCC ($GCC_VERSION) est plus ancienne que la version recommandée (13.2.1+)"
    echo -e "    La compilation pourrait échouer ou avoir des problèmes de compatibilité.${NC}"
    read -p "Voulez-vous continuer malgré tout ? [o/N] " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Oo]$ ]]; then
        echo "Annulation de l'installation."
        exit 1
    fi
fi

# Création du répertoire de build
echo -e "\n${GREEN}[*] Configuration de la compilation...${NC}"
BUILD_DIR="${HOME}/build-open-yolo"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configuration CMake
cmake "$OLDPWD" \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_CXX_STANDARD=17 \
    -DCMAKE_CXX_STANDARD_REQUIRED=ON \
    -DCMAKE_CXX_EXTENSIONS=OFF \
    -DENABLE_LOGGING=ON \
    -DENABLE_LTO=ON

# Compilation
echo -e "\n${GREEN}[*] Compilation en cours...${NC}"
ninja -j$(nproc)

# Installation
echo -e "\n${GREEN}[*] Installation...${NC}"
ninja install

# Message de fin
echo -e "\n${GREEN}[+] Installation terminée avec succès !${NC}"
echo -e "Pour lancer Open-Yolo, exécutez simplement :"
echo -e "  ${YELLOW}OpenYolo${NC}"

exit 0
