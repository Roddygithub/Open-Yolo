#!/bin/bash
# Script de test automatisé multi-plateformes pour Open-Yolo

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=========================================="
echo "  Open-Yolo - Tests Multi-Plateformes"
echo "=========================================="
echo ""

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Résultats des tests
declare -A TEST_RESULTS

# Fonction pour tester une plateforme
test_platform() {
    local platform=$1
    local dockerfile=$2
    
    echo -e "${BLUE}========================================${NC}"
    echo -e "${BLUE}Testing on: $platform${NC}"
    echo -e "${BLUE}========================================${NC}"
    echo ""
    
    # Build Docker image
    echo -e "${YELLOW}Building Docker image...${NC}"
    if docker build -f "$PROJECT_ROOT/docker/$dockerfile" -t "openyolo-test-$platform" "$PROJECT_ROOT"; then
        echo -e "${GREEN}✓ Build successful${NC}"
    else
        echo -e "${RED}✗ Build failed${NC}"
        TEST_RESULTS[$platform]="BUILD_FAILED"
        return 1
    fi
    
    echo ""
    
    # Run tests
    echo -e "${YELLOW}Running tests...${NC}"
    if docker run --rm "openyolo-test-$platform"; then
        echo -e "${GREEN}✓ Tests passed${NC}"
        TEST_RESULTS[$platform]="PASSED"
    else
        echo -e "${RED}✗ Tests failed${NC}"
        TEST_RESULTS[$platform]="FAILED"
        return 1
    fi
    
    echo ""
}

# Test toutes les plateformes
cd "$PROJECT_ROOT"

echo "Testing platforms:"
echo "  - Ubuntu 22.04 LTS"
echo "  - Ubuntu 24.04 LTS"
echo "  - Fedora Latest"
echo "  - Arch Linux"
echo "  - CachyOS"
echo "  - Windows (WSL Ubuntu)"
echo "  - NixOS"
echo ""

# Ubuntu 22.04
test_platform "ubuntu-22.04" "Dockerfile.ubuntu22" || true

# Ubuntu 24.04
test_platform "ubuntu-24.04" "Dockerfile.ubuntu24" || true

# Fedora
test_platform "fedora-latest" "Dockerfile.fedora" || true

# Arch Linux
test_platform "archlinux" "Dockerfile.archlinux" || true

# CachyOS
test_platform "cachyos" "Dockerfile.cachyos" || true

# Windows (WSL)
test_platform "windows-wsl" "Dockerfile.windows-wsl" || true

# NixOS
test_platform "nixos" "Dockerfile.nixos" || true

# Afficher le résumé
echo ""
echo "=========================================="
echo "  Test Summary"
echo "=========================================="
echo ""

TOTAL=0
PASSED=0
FAILED=0
BUILD_FAILED=0

for platform in "${!TEST_RESULTS[@]}"; do
    TOTAL=$((TOTAL + 1))
    result="${TEST_RESULTS[$platform]}"
    
    if [ "$result" = "PASSED" ]; then
        echo -e "${GREEN}✓ $platform: PASSED${NC}"
        PASSED=$((PASSED + 1))
    elif [ "$result" = "FAILED" ]; then
        echo -e "${RED}✗ $platform: FAILED${NC}"
        FAILED=$((FAILED + 1))
    elif [ "$result" = "BUILD_FAILED" ]; then
        echo -e "${RED}✗ $platform: BUILD FAILED${NC}"
        BUILD_FAILED=$((BUILD_FAILED + 1))
    fi
done

echo ""
echo "Total: $TOTAL"
echo -e "${GREEN}Passed: $PASSED${NC}"
echo -e "${RED}Failed: $FAILED${NC}"
echo -e "${RED}Build Failed: $BUILD_FAILED${NC}"
echo ""

# Exit avec code d'erreur si des tests ont échoué
if [ $FAILED -gt 0 ] || [ $BUILD_FAILED -gt 0 ]; then
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
else
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
fi
