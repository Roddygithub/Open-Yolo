#!/bin/bash

# Script de formatage de code pour OpenYolo
# Utilisation : ./scripts/format.sh [--check]
#   --check : Vérifie uniquement le formatage sans appliquer de modifications

set -euo pipefail

# Couleurs pour la sortie
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Vérifier si on est dans la racine du projet
if [ ! -f "CMakeLists.txt" ]; then
  echo -e "${RED}Erreur : Ce script doit être exécuté depuis la racine du projet.${NC}"
  exit 1
fi

# Vérifier les dépendances
check_dependency() {
  if ! command -v "$1" &>/dev/null; then
    echo -e "${RED}Erreur : $1 n'est pas installé.${NC}"
    exit 1
  fi
}

check_dependency find
check_dependency clang-format
check_dependency black
check_dependency shfmt

# Variables
CHECK_ONLY=false
FILES_TO_FORMAT=()
ERRORS=0

# Analyser les arguments
for arg in "$@"; do
  case $arg in
    --check)
      CHECK_ONLY=true
      shift
      ;;
    --help | -h)
      echo "Utilisation : $0 [--check]"
      echo "  --check : Vérifie uniquement le formatage sans appliquer de modifications"
      exit 0
      ;;
    *)
      # Si ce n'est pas un argument connu, c'est probablement un fichier
      if [ -f "$arg" ]; then
        FILES_TO_FORMAT+=("$arg")
      else
        echo -e "${YELLOW}Avertissement : Fichier non trouvé : $arg${NC}"
      fi
      ;;
  esac
done

# Si aucun fichier spécifique n'est fourni, formater tout le code source
if [ ${#FILES_TO_FORMAT[@]} -eq 0 ]; then
  echo -e "${YELLOW}Aucun fichier spécifié, formatage de tout le code source...${NC}"
  FILES_TO_FORMAT=(
    $(find src include tests -type f \( -name '*.h' -o -name '*.hpp' -o -name '*.cpp' \))
    $(find scripts -type f -name '*.sh')
    $(find . -maxdepth 1 -type f -name '*.py')
  )
fi

# Fonction pour formater un fichier
format_file() {
  local file=$1
  local extension="${file##*.}"
  local needs_formatting=false

  case $extension in
    h | hpp | cpp | cc | c)
      if [ "$CHECK_ONLY" = true ]; then
        if ! clang-format --style=file --Werror --dry-run "$file" >/dev/null 2>&1; then
          echo -e "${RED}Formatage incorrect : $file${NC}"
          return 1
        fi
      else
        echo -e "Formatage de $file"
        clang-format --style=file -i "$file"
      fi
      ;;
    py)
      if [ "$CHECK_ONLY" = true ]; then
        if ! black --check -q "$file" >/dev/null 2>&1; then
          echo -e "${RED}Formatage Python incorrect : $file${NC}"
          return 1
        fi
      else
        echo -e "Formatage de $file"
        black -q "$file"
      fi
      ;;
    sh)
      if [ "$CHECK_ONLY" = true ]; then
        if ! shfmt -d -i 4 -ci -ln bash "$file" >/dev/null 2>&1; then
          echo -e "${RED}Formatage shell incorrect : $file${NC}"
          return 1
        fi
      else
        echo -e "Formatage de $file"
        shfmt -w -i 4 -ci -ln bash "$file"
        chmod +x "$file"
      fi
      ;;
    *)
      echo -e "${YELLOW}Format non pris en charge : $file${NC}"
      ;;
  esac

  return 0
}

# Traiter chaque fichier
for file in "${FILES_TO_FORMAT[@]}"; do
  if [ -f "$file" ]; then
    if ! format_file "$file"; then
      ERRORS=$((ERRORS + 1))
    fi
  fi
done

# Résumé
if [ "$CHECK_ONLY" = true ]; then
  if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ Le formatage du code est correct.${NC}"
    exit 0
  else
    echo -e "${RED}✗ $ERRORS fichier(s) avec un formatage incorrect.${NC}"
    echo -e "Exécutez './scripts/format.sh' pour corriger automatiquement le formatage."
    exit 1
  fi
else
  if [ $ERRORS -eq 0 ]; then
    echo -e "${GREEN}✓ Formatage terminé avec succès.${NC}"
    exit 0
  else
    echo -e "${YELLOW}⚠  $ERRORS fichier(s) n'ont pas pu être formatés.${NC}"
    exit 1
  fi
fi
