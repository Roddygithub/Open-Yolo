#!/bin/sh

echo "Formatage du code C++..."
find src include tests -name '*.h' -o -name '*.hpp' -o -name '*.cpp' | xargs clang-format -style=file -i

echo "Formatage des scripts shell..."
find scripts -name '*.sh' -exec shfmt -w -i 2 -ci -bn -s {} \;

echo "Formatage des fichiers Python..."
find . -name '*.py' -exec black --line-length=88 --target-version=py39 {} \;

echo "Formatage terminé."
