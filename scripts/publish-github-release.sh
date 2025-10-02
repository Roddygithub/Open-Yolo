#!/bin/bash
# Script de publication automatique de la release GitHub

set -e

# Configuration
REPO_OWNER="Roddygithub"
REPO_NAME="Open-Yolo"

# Récupérer la version depuis CMakeLists.txt
VERSION=$(grep -A 2 "project(OpenYolo" "${0%/*}/../CMakeLists.txt" | grep "VERSION" | awk '{print $2}')
if [ -z "$VERSION" ]; then
    echo -e "\033[0;31m[ERREUR]\033[0m Impossible de déterminer la version depuis CMakeLists.txt"
    exit 1
fi

TAG="v${VERSION}"
RELEASE_TITLE="Open-Yolo v${VERSION} – Version stable pour Linux"

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${MAGENTA}"
cat << "EOF"
╔═══════════════════════════════════════════════════════╗
║                                                       ║
║     📤 PUBLICATION GITHUB AUTOMATIQUE 📤             ║
║                                                       ║
║         Open-Yolo v${VERSION} Release                      ║
║                                                       ║
╚═══════════════════════════════════════════════════════╝
EOF
echo -e "${NC}"
echo ""

log_step() {
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${CYAN}  $1${NC}"
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo ""
}

log_success() {
    echo -e "${GREEN}✓${NC} $1"
}

log_error() {
    echo -e "${RED}✗${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

log_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

# ============================================
# ÉTAPE 1 : Vérification du Token GitHub
# ============================================
log_step "ÉTAPE 1/8 : Vérification du Token GitHub"

if [ -z "$GITHUB_TOKEN" ]; then
    log_error "Variable GITHUB_TOKEN non définie"
    echo ""
    echo -e "${YELLOW}Pour créer un token GitHub :${NC}"
    echo "1. Aller sur : https://github.com/settings/tokens/new"
    echo "2. Nom : Open-Yolo Release"
    echo "3. Permissions : repo (toutes)"
    echo "4. Générer le token"
    echo "5. Exporter : export GITHUB_TOKEN=votre_token"
    echo ""
    echo -e "${BLUE}Ou utiliser GitHub CLI :${NC}"
    echo "gh auth login"
    echo ""
    exit 1
fi

log_success "Token GitHub trouvé"

# Vérifier l'accès au dépôt
log_info "Vérification de l'accès au dépôt..."
REPO_CHECK=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
    "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}" | jq -r '.name // empty')

if [ "$REPO_CHECK" = "$REPO_NAME" ]; then
    log_success "Accès au dépôt confirmé"
else
    log_error "Impossible d'accéder au dépôt $REPO_OWNER/$REPO_NAME"
    exit 1
fi
echo ""

# ============================================
# ÉTAPE 2 : Vérification du Tag
# ============================================
log_step "ÉTAPE 2/8 : Vérification du Tag v${VERSION}"

# Vérifier si le tag existe localement
if git rev-parse "$TAG" >/dev/null 2>&1; then
    log_success "Tag $TAG existe localement"
else
    log_warning "Tag $TAG n'existe pas localement"
    log_info "Création du tag..."
    
    git tag -a "$TAG" -m "Open-Yolo v${VERSION} – Première version stable Linux

Fonctionnalités principales :
- Curseurs personnalisés et animés (GIF)
- Support multi-écrans et HiDPI
- Effets visuels (ombre, lueur)
- Interface graphique GTK
- Raccourcis clavier personnalisables
- Rendu GPU optimisé (< 1% CPU)
- Packaging DEB/RPM/PKGBUILD

Voir CHANGELOG.md pour les détails complets."
    
    log_success "Tag créé"
fi

# Pousser le tag vers GitHub
log_info "Push du tag vers GitHub..."
if git push origin "$TAG" 2>/dev/null; then
    log_success "Tag poussé vers GitHub"
else
    log_warning "Tag déjà présent sur GitHub ou erreur de push"
fi
echo ""

# ============================================
# ÉTAPE 3 : Préparation de la Description
# ============================================
log_step "ÉTAPE 3/8 : Préparation de la Description"

DESCRIPTION_FILE="GITHUB_RELEASE_v${VERSION}.md"
if [ -f "$DESCRIPTION_FILE" ]; then
    log_success "Description trouvée : $DESCRIPTION_FILE"
    RELEASE_BODY=$(cat "$DESCRIPTION_FILE")
else
    log_warning "Fichier $DESCRIPTION_FILE non trouvé"
    log_info "Utilisation de la description par défaut..."
    RELEASE_BODY="# Open-Yolo v${VERSION}

Première version stable pour Linux.

Voir CHANGELOG.md pour les détails."
fi
echo ""

# ============================================
# ÉTAPE 4 : Vérification des Artefacts
# ============================================
log_step "ÉTAPE 4/8 : Vérification des Artefacts"

PACKAGES_DIR="packages"
SCREENSHOTS_DIR="docs/screenshots"

# Vérifier les paquets
if [ ! -d "$PACKAGES_DIR" ] || [ -z "$(ls -A $PACKAGES_DIR 2>/dev/null)" ]; then
    log_warning "Aucun paquet trouvé dans $PACKAGES_DIR/"
    if [ -f "scripts/build-packages.sh" ]; then
        log_info "Tentative de génération des paquets..."
        ./scripts/build-packages.sh || {
            log_error "Échec de la génération des paquets. Publication annulée."
            exit 1
        }
    else
        log_error "Script 'build-packages.sh' non trouvé. Impossible de générer les paquets."
        exit 1
    fi
fi

log_success "Paquets disponibles :"
ls -lh "$PACKAGES_DIR" | tail -n +2 | awk '{print "  - " $9 " (" $5 ")"}'

# Vérifier les captures d'écran
if [ -d "$SCREENSHOTS_DIR" ] && [ -n "$(ls -A $SCREENSHOTS_DIR 2>/dev/null)" ]; then
    log_success "Captures d'écran disponibles :"
    ls -1 "$SCREENSHOTS_DIR" | sed 's/^/  - /'
else
    log_warning "Aucune capture d'écran trouvée dans $SCREENSHOTS_DIR/"
fi
echo ""

# ============================================
# ÉTAPE 5 : Suppression de l'Ancienne Release
# ============================================
log_step "ÉTAPE 5/8 : Vérification de la Release Existante"

RELEASE_ID=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
    "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/tags/${TAG}" \
    | jq -r '.id // empty')

if [ -n "$RELEASE_ID" ]; then
    log_warning "Release existante trouvée (ID: $RELEASE_ID)"
    log_info "Suppression de l'ancienne release..."
    
    curl -s -X DELETE \
        -H "Authorization: token $GITHUB_TOKEN" \
        "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/${RELEASE_ID}"
    
    log_success "Ancienne release supprimée"
else
    log_success "Aucune release existante"
fi
echo ""

# ============================================
# ÉTAPE 6 : Création de la Release
# ============================================
log_step "ÉTAPE 6/8 : Création de la Release GitHub"

log_info "Création de la release..."

# Échapper le JSON pour la description
RELEASE_BODY_ESCAPED=$(echo "$RELEASE_BODY" | jq -Rs .)

# Créer la release
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
UPLOAD_URL=$(echo "$RELEASE_RESPONSE" | jq -r '.upload_url // empty' | sed 's/{?name,label}//')

if [ -n "$RELEASE_ID" ] && [ -n "$UPLOAD_URL" ]; then
    log_success "Release créée (ID: $RELEASE_ID)"
    log_info "URL de release : https://github.com/${REPO_OWNER}/${REPO_NAME}/releases/tag/${TAG}"
else
    log_error "Échec de création de la release"
    echo "$RELEASE_RESPONSE" | jq .
    exit 1
fi
echo ""

# ============================================
# ÉTAPE 7 : Upload des Artefacts
# ============================================
log_step "ÉTAPE 7/8 : Upload des Artefacts"

upload_asset() {
    local file=$1
    local filename=$(basename "$file")
    
    log_info "Upload de $filename..."
    
    UPLOAD_RESPONSE=$(curl -s -X POST \
        -H "Authorization: token $GITHUB_TOKEN" \
        -H "Content-Type: application/octet-stream" \
        --data-binary @"$file" \
        "${UPLOAD_URL}?name=${filename}")
    
    ASSET_ID=$(echo "$UPLOAD_RESPONSE" | jq -r '.id // empty')
    
    if [ -n "$ASSET_ID" ]; then
        log_success "$filename uploadé (ID: $ASSET_ID)"
        return 0
    else
        log_error "Échec d'upload de $filename"
        echo "$UPLOAD_RESPONSE" | jq .
        return 1
    fi
}

# Upload des paquets
log_info "Upload des paquets..."
for file in "$PACKAGES_DIR"/*; do
    if [ -f "$file" ]; then
        upload_asset "$file"
    fi
done

# Upload des captures d'écran
if [ -d "$SCREENSHOTS_DIR" ] && [ -n "$(ls -A $SCREENSHOTS_DIR 2>/dev/null)" ]; then
    log_info "Upload des captures d'écran..."
    for file in "$SCREENSHOTS_DIR"/*; do
        if [ -f "$file" ]; then
            upload_asset "$file"
        fi
    done
fi

# Upload du CHANGELOG
if [ -f "CHANGELOG.md" ]; then
    log_info "Upload du CHANGELOG..."
    upload_asset "CHANGELOG.md"
fi

# Upload du PKGBUILD (déjà inclus dans le dossier packages)
# if [ -f "PKGBUILD" ]; then
#     log_info "Upload du PKGBUILD..."
#     upload_asset "PKGBUILD"
# fi

echo ""

# ============================================
# ÉTAPE 8 : Vérification Finale
# ============================================
log_step "ÉTAPE 8/8 : Vérification Finale"

log_info "Récupération des informations de la release..."
FINAL_CHECK=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
    "https://api.github.com/repos/${REPO_OWNER}/${REPO_NAME}/releases/tags/${TAG}")

ASSETS_COUNT=$(echo "$FINAL_CHECK" | jq '.assets | length')
RELEASE_URL=$(echo "$FINAL_CHECK" | jq -r '.html_url')

log_success "Release publiée avec succès !"
log_info "Nombre d'artefacts : $ASSETS_COUNT"
echo ""

# ============================================
# RÉSUMÉ FINAL
# ============================================
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✅ Publication Réussie !${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

echo -e "${BLUE}📊 Résumé de la Release :${NC}"
echo ""
echo -e "  ${YELLOW}Tag :${NC} $TAG"
echo -e "  ${YELLOW}Titre :${NC} $RELEASE_TITLE"
echo -e "  ${YELLOW}Artefacts :${NC} $ASSETS_COUNT fichiers"
echo -e "  ${YELLOW}URL :${NC} $RELEASE_URL"
echo ""

echo -e "${BLUE}📦 Artefacts Inclus :${NC}"
echo "$FINAL_CHECK" | jq -r '.assets[] | "  - \(.name) (\(.size / 1024 / 1024 | floor)MB)"'
echo ""

echo -e "${BLUE}🔗 Liens :${NC}"
echo -e "  ${CYAN}Release :${NC} $RELEASE_URL"
echo -e "  ${CYAN}Téléchargements :${NC} https://github.com/${REPO_OWNER}/${REPO_NAME}/releases/download/${TAG}/"
echo ""

echo -e "${MAGENTA}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${MAGENTA}  🎉 Release v${VERSION} Publiée ! 🎉${NC}"
echo -e "${MAGENTA}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
