#!/bin/bash
# Script pour générer les packages via Docker

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=========================================="
echo "  Package Generation via Docker"
echo "=========================================="
echo ""

# Créer le répertoire packages
mkdir -p "$PROJECT_ROOT/packages"

# Générer DEB (Ubuntu 22.04)
echo "Generating DEB package..."
docker run --rm -v "$PROJECT_ROOT:/workspace" -w /workspace ubuntu:22.04 bash -c "
    apt-get update && apt-get install -y build-essential cmake pkg-config \
        libgtkmm-3.0-dev libsdl2-dev libsdl2-image-dev libgl1-mesa-dev \
        libglew-dev libgif-dev xorg-dev libcairo2-dev libcairomm-1.0-dev \
        file rpm && \
    cmake -B build-release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF -DENABLE_LTO=ON && \
    cmake --build build-release -j\$(nproc) && \
    cd build-release && \
    cpack -G DEB && \
    cpack -G RPM && \
    cpack -G TGZ && \
    mv *.deb *.rpm *.tar.gz /workspace/packages/ 2>/dev/null || true
"

# Copier les fichiers de packaging
cp "$PROJECT_ROOT/PKGBUILD" "$PROJECT_ROOT/packages/" 2>/dev/null || true
cp "$PROJECT_ROOT/flake.nix" "$PROJECT_ROOT/packages/" 2>/dev/null || true

echo ""
echo "=========================================="
echo "  Packages Generated Successfully!"
echo "=========================================="
echo ""
ls -lh "$PROJECT_ROOT/packages/"
echo ""
