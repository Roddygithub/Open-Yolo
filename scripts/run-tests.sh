#!/bin/bash

# Vérifier que nous sommes dans le conteneur Docker
if [ ! -f /.dockerenv ]; then
  echo "Ce script doit être exécuté à l'intérieur du conteneur Docker."
  echo "Utilisez 'docker-compose exec app /workspace/scripts/run-tests.sh'"
  exit 1
fi

# Se déplacer dans le répertoire du projet
cd /workspace

# Créer le répertoire de build s'il n'existe pas
mkdir -p build
cd build

# Configurer CMake si ce n'est pas déjà fait
if [ ! -f "CMakeCache.txt" ]; then
  cmake -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=ON -DENABLE_COVERAGE=ON ..
fi

# Compiler les tests
cmake --build . --target all -- -j$(nproc)

# Exécuter les tests
ctest --output-on-failure -V

# Générer le rapport de couverture
if [ "$?" -eq 0 ] && [ "$ENABLE_COVERAGE" = "ON" ]; then
  make coverage

  # Afficher le rapport de couverture
  if [ -f "coverage/index.html" ]; then
    echo "Rapport de couverture généré : file:///workspace/build/coverage/index.html"
  fi
fi
