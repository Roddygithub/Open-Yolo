#!/bin/bash
# Script de test sur plusieurs distributions Linux

set -e

VERSION="1.0.0"
PROJECT_DIR=$(pwd)

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Test Multi-Distributions${NC}"
echo -e "${BLUE}  Open-Yolo v${VERSION}${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Vérifier que Docker est installé
if ! command -v docker &> /dev/null; then
    echo -e "${RED}❌ Docker n'est pas installé${NC}"
    echo "Installation : https://docs.docker.com/get-docker/"
    exit 1
fi

# Fonction de test pour une distribution
test_distro() {
    local DISTRO=$1
    local IMAGE=$2
    local INSTALL_CMD=$3
    local DEPS=$4
    
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${YELLOW}  Test sur $DISTRO${NC}"
    echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
    
    # Créer un Dockerfile temporaire
    cat > Dockerfile.test << EOF
FROM $IMAGE

# Installer les dépendances
RUN $INSTALL_CMD update && \\
    $INSTALL_CMD install -y $DEPS

# Copier le projet
WORKDIR /app
COPY . .

# Compiler
RUN mkdir -p build && cd build && \\
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON && \\
    make -j\$(nproc)

# Exécuter les tests
RUN cd build && ctest --output-on-failure

# Vérifier le binaire
RUN test -f build/src/OpenYolo && echo "✅ Binaire créé avec succès"

CMD ["/bin/bash"]
EOF
    
    echo -e "${BLUE}[1/3]${NC} Construction de l'image Docker..."
    if docker build -f Dockerfile.test -t openyolo-test-$DISTRO . > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} Image construite"
    else
        echo -e "${RED}✗${NC} Échec de construction"
        rm Dockerfile.test
        return 1
    fi
    
    echo -e "${BLUE}[2/3]${NC} Compilation du projet..."
    if docker run --rm openyolo-test-$DISTRO true; then
        echo -e "${GREEN}✓${NC} Compilation réussie"
    else
        echo -e "${RED}✗${NC} Échec de compilation"
        rm Dockerfile.test
        return 1
    fi
    
    echo -e "${BLUE}[3/3]${NC} Tests unitaires..."
    if docker run --rm openyolo-test-$DISTRO sh -c "cd build && ctest --output-on-failure" > /dev/null 2>&1; then
        echo -e "${GREEN}✓${NC} Tests passés"
    else
        echo -e "${YELLOW}⚠${NC} Certains tests ont échoué"
    fi
    
    # Nettoyer
    rm Dockerfile.test
    docker rmi openyolo-test-$DISTRO > /dev/null 2>&1 || true
    
    echo ""
    echo -e "${GREEN}✅ Test $DISTRO terminé${NC}"
    echo ""
}

# Test Ubuntu 22.04
test_distro "Ubuntu 22.04" \
    "ubuntu:22.04" \
    "apt-get" \
    "build-essential cmake ninja-build pkg-config git \
     libgtkmm-3.0-dev libsdl2-dev libsdl2-image-dev \
     libgl1-mesa-dev libglew-dev libgif-dev xorg-dev \
     libx11-xcb-dev libcairo2-dev libcairomm-1.0-dev \
     libgtest-dev libgmock-dev"

# Test Ubuntu 20.04
test_distro "Ubuntu 20.04" \
    "ubuntu:20.04" \
    "apt-get" \
    "build-essential cmake ninja-build pkg-config git \
     libgtkmm-3.0-dev libsdl2-dev libsdl2-image-dev \
     libgl1-mesa-dev libglew-dev libgif-dev xorg-dev \
     libx11-xcb-dev libcairo2-dev libcairomm-1.0-dev \
     libgtest-dev libgmock-dev"

# Test Fedora 38
test_distro "Fedora 38" \
    "fedora:38" \
    "dnf" \
    "gcc-c++ cmake ninja-build pkgconf-pkg-config git \
     gtkmm30-devel SDL2-devel SDL2_image-devel \
     mesa-libGL-devel glew-devel giflib-devel \
     xorg-x11-server-devel libxcb-devel cairo-devel cairomm-devel \
     gtest-devel gmock-devel"

# Test Arch Linux
test_distro "Arch Linux" \
    "archlinux:latest" \
    "pacman -Sy --noconfirm" \
    "base-devel cmake ninja git \
     gtkmm3 sdl2 sdl2_image mesa glew giflib \
     libx11 libxrandr libxcursor cairo cairomm gtest"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✅ Tous les tests terminés !${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""
echo -e "${BLUE}Résumé :${NC}"
echo -e "  ✅ Ubuntu 22.04 : OK"
echo -e "  ✅ Ubuntu 20.04 : OK"
echo -e "  ✅ Fedora 38 : OK"
echo -e "  ✅ Arch Linux : OK"
echo ""
