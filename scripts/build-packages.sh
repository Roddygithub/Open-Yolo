#!/bin/bash
# Script pour construire les paquets DEB et RPM pour Open-Yolo

set -e

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Fonction pour afficher les messages
log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Vérifier que nous sommes à la racine du projet
if [ ! -f "CMakeLists.txt" ]; then
    log_error "Ce script doit être exécuté depuis la racine du projet"
    exit 1
fi

# Variables
PROJECT_NAME="open-yolo"
VERSION=$(grep -oP 'project\(OpenYolo\s+VERSION\s+\K[0-9.]+' ../CMakeLists.txt)
if [ -z "$VERSION" ]; then
    log_error "Impossible de déterminer la version depuis CMakeLists.txt"
    exit 1
fi
BUILD_DIR="build-package"
PACKAGE_DIR="packages"

# Nettoyer les anciens builds
log_info "Nettoyage des anciens builds..."
rm -rf "$BUILD_DIR" "$PACKAGE_DIR"
mkdir -p "$BUILD_DIR" "$PACKAGE_DIR"

# Construire le projet
log_info "Construction du projet..."
cd "$BUILD_DIR"

cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DENABLE_LOGGING=ON \
    -DENABLE_LTO=ON \
    -DBUILD_TESTS=OFF

make -j$(nproc)

# Générer les paquets avec CPack
log_info "Génération des paquets..."

# Paquet DEB
if command -v dpkg &> /dev/null; then
    log_info "Génération du paquet DEB..."
    cpack -G DEB
    mv *.deb "../$PACKAGE_DIR/"
else
    log_warn "dpkg n'est pas installé, impossible de générer le paquet DEB"
fi

# Paquet RPM
if command -v rpmbuild &> /dev/null; then
    log_info "Génération du paquet RPM..."
    cpack -G RPM
    mv *.rpm "../$PACKAGE_DIR/"
else
    log_warn "rpmbuild n'est pas installé, impossible de générer le paquet RPM"
fi

# Archive TGZ (générique)
log_info "Génération de l'archive TGZ..."
cpack -G TGZ
mv *.tar.gz "../$PACKAGE_DIR/"

cd ..

# Afficher les paquets générés
log_info "Paquets générés dans $PACKAGE_DIR:"
ls -lh "$PACKAGE_DIR"

log_info "Construction terminée avec succès!"
