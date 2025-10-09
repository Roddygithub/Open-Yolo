#!/bin/bash

# Se déplacer dans le répertoire de build
cd build

# Exécuter les tests avec CTest
echo "Exécution des tests..."
ctest --output-on-failure

# Générer un rapport de couverture de code si activé
if [ "$ENABLE_COVERAGE" = "ON" ]; then
  echo "Génération du rapport de couverture..."
  make coverage

  # Afficher le rapport de couverture
  if [ -f "coverage/index.html" ]; then
    echo "Rapport de couverture généré : file://$(pwd)/coverage/index.html"
  fi
fi
