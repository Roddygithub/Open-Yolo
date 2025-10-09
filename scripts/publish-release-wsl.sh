#!/bin/bash
# Script wrapper pour publication GitHub depuis WSL
# Usage: ./publish-release-wsl.sh <GITHUB_TOKEN>

set -e

if [ -z "$1" ]; then
  echo "Usage: $0 <GITHUB_TOKEN>"
  echo "Example: $0 ghp_your_token_here"
  exit 1
fi

# Définir le token de manière sécurisée
export GITHUB_TOKEN="$1"

# Vérifier que jq est installé
if ! command -v jq &>/dev/null; then
  echo "Installation de jq..."
  sudo apt-get update -qq
  sudo apt-get install -y jq
fi

# Vérifier que curl est installé
if ! command -v curl &>/dev/null; then
  echo "Installation de curl..."
  sudo apt-get install -y curl
fi

# Exécuter le script de publication
cd "$(dirname "$0")/.."
chmod +x scripts/publish-github-release.sh
./scripts/publish-github-release.sh

# Nettoyer le token de l'environnement
unset GITHUB_TOKEN

echo ""
echo "Token nettoyé de l'environnement pour sécurité"
