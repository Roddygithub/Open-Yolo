#!/bin/bash
# Script de génération automatique de tous les packages Open-Yolo

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build-release"
PACKAGES_DIR="$PROJECT_ROOT/packages"

# Couleurs
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Open-Yolo Package Generator${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Nettoyer les anciens builds
if [ -d "$BUILD_DIR" ]; then
  echo -e "${YELLOW}Cleaning old build directory...${NC}"
  rm -rf "$BUILD_DIR"
fi

# Créer le répertoire de packages
mkdir -p "$PACKAGES_DIR"

# Configurer CMake
echo -e "${BLUE}Configuring CMake...${NC}"
cmake -B "$BUILD_DIR" \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_TESTS=OFF \
  -DENABLE_LTO=ON \
  -DENABLE_LOGGING=OFF

# Compiler
echo -e "${BLUE}Building project...${NC}"
cmake --build "$BUILD_DIR" -j$(nproc)

# Générer les packages
cd "$BUILD_DIR"

echo ""
echo -e "${BLUE}Generating packages...${NC}"
echo ""

# DEB package
echo -e "${YELLOW}Generating DEB package...${NC}"
if cpack -G DEB; then
  echo -e "${GREEN}✓ DEB package generated${NC}"
  mv *.deb "$PACKAGES_DIR/" 2>/dev/null || true
else
  echo -e "${RED}✗ DEB package generation failed${NC}"
fi

# RPM package
echo -e "${YELLOW}Generating RPM package...${NC}"
if cpack -G RPM; then
  echo -e "${GREEN}✓ RPM package generated${NC}"
  mv *.rpm "$PACKAGES_DIR/" 2>/dev/null || true
else
  echo -e "${RED}✗ RPM package generation failed${NC}"
fi

# TGZ package
echo -e "${YELLOW}Generating TGZ package...${NC}"
if cpack -G TGZ; then
  echo -e "${GREEN}✓ TGZ package generated${NC}"
  mv *.tar.gz "$PACKAGES_DIR/" 2>/dev/null || true
else
  echo -e "${RED}✗ TGZ package generation failed${NC}"
fi

# Copier les fichiers de packaging
echo ""
echo -e "${YELLOW}Copying packaging files...${NC}"
cp "$PROJECT_ROOT/PKGBUILD" "$PACKAGES_DIR/" 2>/dev/null || true
cp "$PROJECT_ROOT/flake.nix" "$PACKAGES_DIR/" 2>/dev/null || true

# Afficher le résumé
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Package Generation Complete${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo "Packages generated in: $PACKAGES_DIR"
echo ""
ls -lh "$PACKAGES_DIR"
echo ""
echo -e "${GREEN}All packages ready for distribution!${NC}"
