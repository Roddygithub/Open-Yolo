#!/bin/bash
# Script de test rapide pour Open-Yolo

set -e

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Open-Yolo - Test de Compilation${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Vérifier que nous sommes à la racine du projet
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}[ERREUR]${NC} Ce script doit être exécuté depuis la racine du projet"
    exit 1
fi

# Nettoyer les anciens builds
echo -e "${YELLOW}[1/5]${NC} Nettoyage des anciens builds..."
rm -rf build-test
mkdir -p build-test

# Configuration
echo -e "${YELLOW}[2/5]${NC} Configuration CMake..."
cd build-test
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_LOGGING=ON \
    -DENABLE_WARNINGS_AS_ERRORS=OFF \
    > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} Configuration réussie"
else
    echo -e "${RED}✗${NC} Échec de la configuration"
    exit 1
fi

# Compilation
echo -e "${YELLOW}[3/5]${NC} Compilation du projet..."
make -j$(nproc) > /dev/null 2>&1

if [ $? -eq 0 ]; then
    echo -e "${GREEN}✓${NC} Compilation réussie"
else
    echo -e "${RED}✗${NC} Échec de la compilation"
    echo -e "${YELLOW}Tentative de compilation avec sortie détaillée...${NC}"
    make VERBOSE=1
    exit 1
fi

# Vérifier que le binaire existe
echo -e "${YELLOW}[4/5]${NC} Vérification du binaire..."
if [ -f "src/OpenYolo" ]; then
    echo -e "${GREEN}✓${NC} Binaire trouvé : src/OpenYolo"
    ls -lh src/OpenYolo
elif [ -f "OpenYolo" ]; then
    echo -e "${GREEN}✓${NC} Binaire trouvé : OpenYolo"
    ls -lh OpenYolo
else
    echo -e "${RED}✗${NC} Binaire non trouvé"
    echo "Recherche du binaire..."
    find . -name "OpenYolo" -type f
    exit 1
fi

# Tests
echo -e "${YELLOW}[5/5]${NC} Exécution des tests..."
if ctest --output-on-failure > /dev/null 2>&1; then
    echo -e "${GREEN}✓${NC} Tous les tests sont passés"
else
    echo -e "${YELLOW}⚠${NC} Certains tests ont échoué (voir ci-dessus)"
    ctest --output-on-failure
fi

cd ..

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✓ Test de compilation réussi !${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "Le binaire est disponible dans : ${BLUE}build-test/src/OpenYolo${NC}"
echo ""
echo -e "Pour exécuter l'application :"
echo -e "  ${BLUE}cd build-test/src && ./OpenYolo${NC}"
echo ""
echo -e "Pour installer :"
echo -e "  ${BLUE}cd build-test && sudo make install${NC}"
echo ""
