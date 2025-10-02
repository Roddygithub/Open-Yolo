#!/bin/bash
# Script simplifié de publication GitHub (sans paquets)

set -e

VERSION="1.0.0"
TAG="v${VERSION}"
REPO_OWNER="Roddygithub"
REPO_NAME="Open-Yolo"
RELEASE_TITLE="Open-Yolo v${VERSION} – Première version stable pour Linux"

echo "Publication GitHub Release v${VERSION}"
echo ""

# Vérifier le token
if [ -z "$GITHUB_TOKEN" ]; then
    echo "ERREUR: GITHUB_TOKEN non defini"
    exit 1
fi

echo "1. Verification du depot..."
REPO_CHECK=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
    "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}" | jq -r '.name // empty')

if [ "$REPO_CHECK" != "$REPO_NAME" ]; then
    echo "ERREUR: Impossible d'acceder au depot"
    exit 1
fi
echo "OK - Acces au depot confirme"

# Préparer la description
echo ""
echo "2. Preparation de la description..."
if [ -f "GITHUB_RELEASE_v${VERSION}.md" ]; then
    RELEASE_BODY=$(cat "GITHUB_RELEASE_v${VERSION}.md")
    echo "OK - Description chargee"
else
    RELEASE_BODY="# Open-Yolo v${VERSION}

Premiere version stable pour Linux.

Voir CHANGELOG.md pour les details."
    echo "ATTENTION - Description par defaut utilisee"
fi

# Supprimer l'ancienne release si elle existe
echo ""
echo "3. Verification de la release existante..."
RELEASE_ID=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
    "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/tags/${TAG}" \
    | jq -r '.id // empty')

if [ -n "$RELEASE_ID" ]; then
    echo "Release existante trouvee (ID: $RELEASE_ID)"
    echo "Suppression..."
    curl -s -X DELETE \
        -H "Authorization: token $GITHUB_TOKEN" \
        "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/${RELEASE_ID}"
    echo "OK - Ancienne release supprimee"
else
    echo "OK - Aucune release existante"
fi

# Créer la release
echo ""
echo "4. Creation de la release..."
RELEASE_BODY_ESCAPED=$(echo "$RELEASE_BODY" | jq -Rs .)

RELEASE_RESPONSE=$(curl -s -X POST \
    -H "Authorization: token $GITHUB_TOKEN" \
    -H "Content-Type: application/json" \
    "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases" \
    -d "{
        \"tag_name\": \"${TAG}\",
        \"name\": \"${RELEASE_TITLE}\",
        \"body\": ${RELEASE_BODY_ESCAPED},
        \"draft\": false,
        \"prerelease\": false,
        \"make_latest\": \"true\"
    }")

RELEASE_ID=$(echo "$RELEASE_RESPONSE" | jq -r '.id // empty')
RELEASE_URL=$(echo "$RELEASE_RESPONSE" | jq -r '.html_url // empty')

if [ -n "$RELEASE_ID" ] && [ -n "$RELEASE_URL" ]; then
    echo "OK - Release creee (ID: $RELEASE_ID)"
    echo ""
    echo "========================================"
    echo "  SUCCES - Release Publiee !"
    echo "========================================"
    echo ""
    echo "Tag: $TAG"
    echo "Titre: $RELEASE_TITLE"
    echo "URL: $RELEASE_URL"
    echo ""
    echo "Note: Les paquets (DEB/RPM/TGZ) peuvent etre ajoutes"
    echo "      manuellement plus tard via l'interface GitHub"
    echo ""
else
    echo "ERREUR - Echec de creation de la release"
    echo "$RELEASE_RESPONSE" | jq .
    exit 1
fi
