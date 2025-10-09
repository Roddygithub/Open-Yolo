#!/bin/bash

# Script pour exécuter le formatage du code avec Docker
# Utilisation : ./scripts/run-format.sh [options]
# Options :
#   --check : Vérifie uniquement le formatage sans appliquer de modifications

set -euo pipefail

# Couleurs pour la sortie
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Vérifier si Docker est installé
if ! command -v docker-compose &>/dev/null; then
  echo -e "${RED}Erreur: docker-compose n'est pas installé.${NC}"
  echo "Veuillez installer Docker et Docker Compose pour continuer."
  exit 1
fi

# Vérifier si le fichier docker-compose.yml existe
if [ ! -f "docker-compose.yml" ]; then
  echo -e "${RED}Erreur: Le fichier docker-compose.yml est introuvable.${NC}"
  echo "Assurez-vous d'exécuter ce script depuis la racine du projet."
  exit 1
fi

# Vérifier si l'utilisateur a les droits pour exécuter Docker
if ! docker info &>/dev/null; then
  echo -e "${RED}Erreur: Vous n'avez pas les droits pour exécuter Docker.${NC}"
  echo "Assurez-vous que votre utilisateur fait partie du groupe 'docker' ou utilisez 'sudo'."
  exit 1
fi

# Démarrer le service de formatage
echo -e "${YELLOW}Démarrage du formatage du code...${NC}"

# Construire l'image si nécessaire
echo -e "${YELLOW}Construction de l'image Docker...${NC}"
docker-compose build format

# Exécuter le formatage
echo -e "${YELLOW}Exécution du formatage...${NC}"

# Créer le fichier .docker.xauth s'il n'existe pas
if [ ! -f "/tmp/.docker.xauth" ]; then
  touch /tmp/.docker.xauth
  chmod 666 /tmp/.docker.xauth
fi

# Exécuter le formatage
docker-compose run --rm format

# Vérifier le code de sortie
if [ $? -eq 0 ]; then
  echo -e "${GREEN}✓ Formatage terminé avec succès.${NC}"
  exit 0
else
  echo -e "${RED}✗ Une erreur s'est produite lors du formatage.${NC}"
  exit 1
fi
