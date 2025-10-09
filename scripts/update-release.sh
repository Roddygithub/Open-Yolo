#!/bin/bash
# Script pour mettre à jour la release v1.0.0 avec le support NixOS

set -e

VERSION="1.0.0"
TAG="v${VERSION}"
REPO_OWNER="Roddygithub"
REPO_NAME="Open-Yolo"

echo "Mise a jour de la release v${VERSION} avec support NixOS"
echo ""

# Vérifier le token
if [ -z "$GITHUB_TOKEN" ]; then
  echo "ERREUR: GITHUB_TOKEN non defini"
  exit 1
fi

echo "1. Recuperation de la release existante..."
RELEASE_INFO=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
  "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/tags/${TAG}")

RELEASE_ID=$(echo "$RELEASE_INFO" | jq -r '.id // empty')
UPLOAD_URL=$(echo "$RELEASE_INFO" | jq -r '.upload_url // empty' | sed 's/{?name,label}//')

if [ -z "$RELEASE_ID" ]; then
  echo "ERREUR: Release v${VERSION} non trouvee"
  exit 1
fi

echo "OK - Release trouvee (ID: $RELEASE_ID)"

# Mettre à jour la description
echo ""
echo "2. Mise a jour de la description..."

if [ -f "GITHUB_RELEASE_v${VERSION}.md" ]; then
  RELEASE_BODY=$(cat "GITHUB_RELEASE_v${VERSION}.md")
  RELEASE_BODY_ESCAPED=$(echo "$RELEASE_BODY" | jq -Rs .)

  curl -s -X PATCH \
    -H "Authorization: token $GITHUB_TOKEN" \
    -H "Content-Type: application/json" \
    "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/${RELEASE_ID}" \
    -d "{\"body\": ${RELEASE_BODY_ESCAPED}}" >/dev/null

  echo "OK - Description mise a jour avec support NixOS"
else
  echo "ATTENTION - Fichier de description non trouve"
fi

# Uploader flake.nix
echo ""
echo "3. Upload de flake.nix..."

if [ -f "flake.nix" ]; then
  # Vérifier si flake.nix existe déjà dans les assets
  EXISTING_ASSET=$(echo "$RELEASE_INFO" | jq -r '.assets[] | select(.name == "flake.nix") | .id')

  if [ -n "$EXISTING_ASSET" ]; then
    echo "Suppression de l'ancien flake.nix..."
    curl -s -X DELETE \
      -H "Authorization: token $GITHUB_TOKEN" \
      "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/assets/${EXISTING_ASSET}"
  fi

  echo "Upload du nouveau flake.nix..."
  UPLOAD_RESPONSE=$(curl -s -X POST \
    -H "Authorization: token $GITHUB_TOKEN" \
    -H "Content-Type: text/plain" \
    --data-binary @flake.nix \
    "${UPLOAD_URL}?name=flake.nix")

  ASSET_ID=$(echo "$UPLOAD_RESPONSE" | jq -r '.id // empty')

  if [ -n "$ASSET_ID" ]; then
    echo "OK - flake.nix uploade (ID: $ASSET_ID)"
  else
    echo "ERREUR - Echec d'upload de flake.nix"
  fi
else
  echo "ERREUR - flake.nix non trouve"
fi

echo ""
echo "========================================"
echo "  SUCCES - Release Mise a Jour !"
echo "========================================"
echo ""
echo "Tag: $TAG"
echo "URL: https://github.com/${REPO_OWNER}/${REPO_NAME}/releases/tag/${TAG}"
echo ""
echo "Nouveautes:"
echo "  - Support NixOS via Nix Flake"
echo "  - Documentation mise a jour"
echo "  - flake.nix disponible en telechargement"
echo ""
