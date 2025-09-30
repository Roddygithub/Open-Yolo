# Guide de Compilation - Open-Yolo

Ce guide détaille les étapes pour compiler Open-Yolo depuis les sources.

## Table des matières

- [Prérequis](#prérequis)
- [Compilation rapide](#compilation-rapide)
- [Compilation détaillée](#compilation-détaillée)
- [Options de compilation](#options-de-compilation)
- [Tests](#tests)
- [Packaging](#packaging)
- [Dépannage](#dépannage)

## Prérequis

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
    libgtkmm-3.0-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libgl1-mesa-dev \
    libglew-dev \
    libgif-dev \
    xorg-dev \
    libx11-xcb-dev \
    libcairo2-dev \
    libcairomm-1.0-dev
```

### Fedora/RHEL

```bash
sudo dnf install -y \
    gcc-c++ \
    cmake \
    ninja-build \
    pkgconf-pkg-config \
    git \
    gtkmm30-devel \
    SDL2-devel \
    SDL2_image-devel \
    mesa-libGL-devel \
    glew-devel \
    giflib-devel \
    xorg-x11-server-devel \
    libxcb-devel \
    cairo-devel \
    cairomm-devel
```

### Arch Linux

```bash
sudo pacman -S --needed \
    base-devel \
    cmake \
    ninja \
    git \
    gtkmm3 \
    sdl2 \
    sdl2_image \
    mesa \
    glew \
    giflib \
    libx11 \
    libxrandr \
    libxcursor \
    cairo \
    cairomm
```

## Compilation rapide

```bash
# Cloner le dépôt
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo

# Compiler
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja

# Installer (optionnel)
sudo ninja install
```

## Compilation détaillée

### 1. Configuration

```bash
mkdir build && cd build

cmake .. \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DENABLE_LOGGING=ON \
    -DENABLE_LTO=ON \
    -DBUILD_TESTS=OFF
```

### 2. Compilation

```bash
# Compilation parallèle (utilise tous les cœurs)
ninja

# Ou avec make si vous n'utilisez pas Ninja
make -j$(nproc)
```

### 3. Installation

```bash
# Installation système (nécessite les droits root)
sudo ninja install

# Ou installation dans un préfixe personnalisé
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
ninja install
```

## Options de compilation

| Option | Valeur par défaut | Description |
|--------|-------------------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | Type de build (Debug, Release, RelWithDebInfo, MinSizeRel) |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Préfixe d'installation |
| `ENABLE_LOGGING` | `ON` | Active les logs de débogage |
| `ENABLE_WARNINGS_AS_ERRORS` | `ON` | Traite les avertissements comme des erreurs |
| `ENABLE_LTO` | `ON` | Active l'optimisation de l'éditeur de liens (LTO) |
| `BUILD_TESTS` | `OFF` | Compile les tests unitaires |
| `ENABLE_COVERAGE` | `OFF` | Active la couverture de code (nécessite BUILD_TESTS=ON) |
| `ENABLE_ASAN` | `OFF` | Active AddressSanitizer pour la détection de bugs mémoire |

### Exemples

**Build de développement avec tests :**
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_ASAN=ON \
    -DENABLE_LOGGING=ON
```

**Build optimisé pour la production :**
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_LTO=ON \
    -DENABLE_LOGGING=OFF
```

**Build avec couverture de code :**
```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_COVERAGE=ON
```

## Tests

### Compilation des tests

```bash
cmake .. -DBUILD_TESTS=ON
ninja
```

### Exécution des tests

```bash
# Tous les tests
ctest --output-on-failure

# Tests en parallèle
ctest -j$(nproc)

# Tests spécifiques
ctest -R CursorManager

# Tests avec sortie détaillée
ctest -V
```

### Couverture de code

```bash
# Configuration avec couverture
cmake .. -DBUILD_TESTS=ON -DENABLE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug
ninja

# Exécution des tests
ctest

# Génération du rapport de couverture
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_html

# Ouvrir le rapport
xdg-open coverage_html/index.html
```

## Packaging

### Génération de paquets avec CPack

```bash
cd build

# Paquet DEB (Debian/Ubuntu)
cpack -G DEB

# Paquet RPM (Fedora/RHEL)
cpack -G RPM

# Archive TGZ (générique)
cpack -G TGZ

# Tous les formats
cpack
```

### Script de packaging automatique

```bash
# Depuis la racine du projet
chmod +x scripts/build-packages.sh
./scripts/build-packages.sh
```

Les paquets générés seront dans le dossier `packages/`.

### PKGBUILD pour Arch Linux

```bash
# Depuis la racine du projet
makepkg -si
```

## Compilation avec Docker

### Build de l'image

```bash
docker build -t open-yolo:latest .
```

### Exécution

```bash
docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    open-yolo:latest
```

### Docker Compose

```bash
docker-compose up --build
```

## Dépannage

### Erreur : "Could not find GTKmm"

**Solution :**
```bash
# Ubuntu/Debian
sudo apt-get install libgtkmm-3.0-dev

# Fedora
sudo dnf install gtkmm30-devel

# Arch
sudo pacman -S gtkmm3
```

### Erreur : "OpenGL not found"

**Solution :**
```bash
# Ubuntu/Debian
sudo apt-get install libgl1-mesa-dev libglew-dev

# Fedora
sudo dnf install mesa-libGL-devel glew-devel

# Arch
sudo pacman -S mesa glew
```

### Erreur de compilation avec C++17

**Solution :**
Assurez-vous d'utiliser un compilateur récent :
- GCC 7+ ou Clang 5+

```bash
# Vérifier la version
g++ --version
clang++ --version

# Spécifier un compilateur spécifique
cmake .. -DCMAKE_CXX_COMPILER=g++-11
```

### Problèmes de liens avec X11

**Solution :**
```bash
# Ubuntu/Debian
sudo apt-get install xorg-dev libx11-dev libxrandr-dev libxcursor-dev

# Fedora
sudo dnf install libX11-devel libXrandr-devel libXcursor-devel

# Arch
sudo pacman -S libx11 libxrandr libxcursor
```

### Tests qui échouent

**Solution :**
1. Vérifiez que toutes les dépendances de test sont installées :
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libgtest-dev libgmock-dev
   
   # Fedora
   sudo dnf install gtest-devel gmock-devel
   
   # Arch
   sudo pacman -S gtest
   ```

2. Exécutez les tests avec plus de détails :
   ```bash
   ctest -V
   ```

### Problèmes de performance

**Solution :**
1. Compilez en mode Release avec LTO :
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_LTO=ON
   ```

2. Vérifiez les pilotes graphiques :
   ```bash
   glxinfo | grep "OpenGL"
   ```

## Support

Pour plus d'aide :
- **Issues GitHub** : https://github.com/Roddygithub/Open-Yolo/issues
- **Documentation** : Consultez le README.md
- **Wiki** : https://github.com/Roddygithub/Open-Yolo/wiki
