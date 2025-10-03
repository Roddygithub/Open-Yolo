#!/bin/bash
# Script de vérification pré-release pour Open-Yolo

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

ERRORS=0
WARNINGS=0

# Extraire dynamiquement la version depuis le fichier CMakeLists.txt principal
VERSION=$(grep -oP 'project\(OpenYolo\s+VERSION\s+\K[0-9\.]+' CMakeLists.txt)
if [ -z "$VERSION" ]; then
    echo -e "${RED}[ERREUR]${NC} Impossible de déterminer la version depuis CMakeLists.txt"
    exit 1
fi
echo -e "${BLUE}Vérification de la pré-release pour la version : ${YELLOW}$VERSION${NC}"

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Open-Yolo - Vérification Pré-Release${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

check_pass() {
    echo -e "${GREEN}✓${NC} $1"
}

check_fail() {
    echo -e "${RED}✗${NC} $1"
    ((ERRORS++))
}

check_warn() {
    echo -e "${YELLOW}⚠${NC} $1"
    ((WARNINGS++))
}

# 1. Vérifier la structure du projet
echo -e "${YELLOW}[1/10]${NC} Vérification de la structure du projet..."
if [ -f "CMakeLists.txt" ] && [ -d "src" ] && [ -d "include" ]; then
    check_pass "Structure du projet OK"
else
    check_fail "Structure du projet incomplète"
fi

# 2. Vérifier les fichiers de documentation
echo -e "${YELLOW}[2/10]${NC} Vérification de la documentation..."
DOCS=("README.md" "BUILD.md" "CHANGELOG.md" "LICENSE" "QUICK_START.md")
for doc in "${DOCS[@]}"; do
    if [ -f "$doc" ]; then
        check_pass "$doc présent"
    else
        check_fail "$doc manquant"
    fi
done

# 3. Vérifier les scripts
echo -e "${YELLOW}[3/10]${NC} Vérification des scripts..."
SCRIPTS=("scripts/build-packages.sh" "scripts/quick-test.sh" "Makefile" "PKGBUILD")
for script in "${SCRIPTS[@]}"; do
    if [ -f "$script" ]; then
        check_pass "$script présent"
    else
        check_warn "$script manquant"
    fi
done

# 4. Vérifier la compilation
echo -e "${YELLOW}[4/10]${NC} Test de compilation..."
if [ -d "build-test" ]; then
    rm -rf build-test
fi
mkdir build-test
cd build-test
LOG_DIR="../build-test-logs"
mkdir -p "$LOG_DIR"

if cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON > "$LOG_DIR/cmake.log" 2>&1; then
    check_pass "Configuration CMake OK"
else
    check_fail "Échec de la configuration CMake"
    echo -e "   ${BLUE}Logs: $LOG_DIR/cmake.log${NC}"
    cd ..
    # Pas besoin de quitter, la suite des tests peut être pertinente
fi

if make -j$(nproc) > "$LOG_DIR/make.log" 2>&1; then
    check_pass "Compilation OK"
else
    check_fail "Échec de la compilation"
    echo -e "   ${BLUE}Logs: $LOG_DIR/make.log${NC}"
fi

cd ..
rm -rf "$LOG_DIR" # Nettoyer les logs si tout s'est bien passé

# 5. Vérifier le binaire
echo -e "${YELLOW}[5/10]${NC} Vérification du binaire..."
if [ -f "build-test/src/OpenYolo" ]; then
    check_pass "Binaire généré"
    SIZE=$(du -h build-test/src/OpenYolo | cut -f1)
    echo -e "   ${BLUE}Taille : $SIZE${NC}"
else
    check_fail "Binaire non trouvé"
fi

# 6. Vérifier les tests
echo -e "${YELLOW}[6/10]${NC} Vérification des tests..."
if [ -d "build-test" ]; then
    cd build-test
    if ctest --output-on-failure > /dev/null 2>&1; then
        check_pass "Tests unitaires OK"
    else
        check_warn "Certains tests ont échoué"
    fi
    cd ..
else
    check_warn "Répertoire de build non trouvé"
fi

# 7. Vérifier la génération des paquets
echo -e "${YELLOW}[7/10]${NC} Test de génération des paquets..."
cd build-test
if cpack -G TGZ > /dev/null 2>&1; then
    check_pass "Génération de paquet TGZ OK"
else
    check_warn "Échec de génération du paquet TGZ"
fi
cd ..

# 8. Vérifier les dépendances
echo -e "${YELLOW}[8/10]${NC} Vérification des dépendances..."
DEPS=("pkg-config" "cmake" "make")
for dep in "${DEPS[@]}"; do
    if command -v "$dep" &> /dev/null; then
        check_pass "$dep installé"
    else
        check_warn "$dep non trouvé"
    fi
done

# 9. Vérifier Git
echo -e "${YELLOW}[9/10]${NC} Vérification Git..."
if git diff-index --quiet HEAD --; then
    check_pass "Aucune modification non commitée"
else
    check_warn "Il y a des modifications non commitées"
fi

BRANCH=$(git branch --show-current)
if [ "$BRANCH" = "main" ]; then
    check_pass "Sur la branche main"
else
    check_warn "Pas sur la branche main (actuellement sur $BRANCH)"
fi

# 10. Vérifier la version
echo -e "${YELLOW}[10/10]${NC} Vérification de la version..."
if grep -q "VERSION $VERSION" CMakeLists.txt; then
    check_pass "Version $VERSION dans CMakeLists.txt"
else
    check_warn "Version $VERSION non trouvée dans CMakeLists.txt"
fi

if grep -q "pkgver=$VERSION" PKGBUILD 2>/dev/null; then
    check_pass "Version $VERSION dans PKGBUILD"
else
    check_warn "Version $VERSION non trouvée dans PKGBUILD"
fi

# Résumé
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Résumé de la Vérification${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

if [ $ERRORS -eq 0 ] && [ $WARNINGS -eq 0 ]; then
    echo -e "${GREEN}✓ Tout est OK ! Prêt pour la release !${NC}"
    EXIT_CODE=0
elif [ $ERRORS -eq 0 ]; then
    echo -e "${YELLOW}⚠ $WARNINGS avertissement(s)${NC}"
    echo -e "${YELLOW}La release peut être créée, mais vérifiez les avertissements${NC}"
    EXIT_CODE=0
else
    echo -e "${RED}✗ $ERRORS erreur(s), $WARNINGS avertissement(s)${NC}"
    echo -e "${RED}Corrigez les erreurs avant de créer la release${NC}"
    EXIT_CODE=1
fi

echo ""
exit $EXIT_CODE
