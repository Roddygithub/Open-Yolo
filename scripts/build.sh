#!/bin/bash

# Créer le répertoire de build s'il n'existe pas
mkdir -p build

# Se déplacer dans le répertoire de build
cd build

# Configurer CMake
echo "Configuration de CMake..."
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      -DENABLE_LOGGING=ON \
      -DBUILD_TESTS=ON \
      -DENABLE_COVERAGE=OFF \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      ..

# Compiler le projet
echo "Compilation en cours..."
make -j$(nproc)

# Installer le projet (optionnel)
# echo "Installation..."
# sudo make install

echo "Construction terminée avec succès !"
