#!/bin/bash
# Script d'automatisation complète de la release v1.0.0

set -e

VERSION="1.0.0"
TAG="v${VERSION}"

# Couleurs
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

ERRORS=0
WARNINGS=0

echo -e "${MAGENTA}"
cat <<"EOF"
╔═══════════════════════════════════════════════════════╗
║                                                       ║
║     🚀 AUTOMATISATION RELEASE v1.0.0 🚀              ║
║                                                       ║
║              Open-Yolo - Linux                        ║
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
  ((ERRORS++))
}

log_warning() {
  echo -e "${YELLOW}⚠${NC} $1"
  ((WARNINGS++))
}

log_info() {
  echo -e "${BLUE}ℹ${NC} $1"
}

# ============================================
# ÉTAPE 1 : Génération des Curseurs
# ============================================
log_step "ÉTAPE 1/10 : Génération des Curseurs par Défaut"

if command -v python3 &>/dev/null; then
  if python3 -c "import PIL" 2>/dev/null; then
    log_info "Génération des curseurs..."
    python3 scripts/generate-default-cursors.py
    log_success "Curseurs générés"
  else
    log_warning "Pillow non installé, curseurs non générés"
    log_info "Installation : pip install Pillow"
  fi
else
  log_warning "Python3 non trouvé, curseurs non générés"
fi
echo ""

# ============================================
# ÉTAPE 2 : Vérification Pré-Release
# ============================================
log_step "ÉTAPE 2/10 : Vérification Pré-Release"

chmod +x scripts/pre-release-check.sh 2>/dev/null || true
if [ -f "scripts/pre-release-check.sh" ]; then
  if ./scripts/pre-release-check.sh; then
    log_success "Vérifications pré-release OK"
  else
    log_error "Échec des vérifications pré-release"
    exit 1
  fi
else
  log_warning "Script pre-release-check.sh non trouvé"
fi
echo ""

# ============================================
# ÉTAPE 3 : Compilation et Tests
# ============================================
log_step "ÉTAPE 3/10 : Compilation et Tests"

log_info "Compilation en mode Release..."
if make clean && make build BUILD_TYPE=Release; then
  log_success "Compilation réussie"
else
  log_error "Échec de compilation"
  exit 1
fi

log_info "Exécution des tests..."
if make test; then
  log_success "Tous les tests passent"
else
  log_warning "Certains tests ont échoué"
fi
echo ""

# ============================================
# ÉTAPE 4 : Benchmark des Performances
# ============================================
log_step "ÉTAPE 4/10 : Benchmark des Performances"

chmod +x scripts/benchmark-performance.sh 2>/dev/null || true
if [ -f "scripts/benchmark-performance.sh" ]; then
  ./scripts/benchmark-performance.sh
  log_success "Benchmark terminé"
else
  log_warning "Script benchmark non trouvé"
fi
echo ""

# ============================================
# ÉTAPE 5 : Génération des Paquets
# ============================================
log_step "ÉTAPE 5/10 : Génération des Paquets"

log_info "Génération des paquets DEB/RPM/TGZ..."
chmod +x scripts/build-packages.sh 2>/dev/null || true
if [ -f "scripts/build-packages.sh" ]; then
  ./scripts/build-packages.sh
  log_success "Paquets générés"
else
  log_warning "Script build-packages.sh non trouvé"
fi
echo ""

# ============================================
# ÉTAPE 6 : Vérification Git
# ============================================
log_step "ÉTAPE 6/10 : Vérification Git"

# Vérifier la branche
CURRENT_BRANCH=$(git branch --show-current 2>/dev/null || echo "unknown")
if [ "$CURRENT_BRANCH" = "main" ]; then
  log_success "Sur la branche main"
else
  log_warning "Pas sur la branche main (actuellement sur $CURRENT_BRANCH)"
  read -p "Continuer quand même ? (y/N) " -n 1 -r
  echo
  if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    exit 1
  fi
fi

# Vérifier les modifications non commitées
if git diff-index --quiet HEAD -- 2>/dev/null; then
  log_success "Aucune modification non commitée"
else
  log_info "Modifications non commitées détectées"
  git status --short
  echo ""
  read -p "Commiter les changements ? (Y/n) " -n 1 -r
  echo
  if [[ ! $REPLY =~ ^[Nn]$ ]]; then
    git add .
    git commit -m "Release v${VERSION} - Ajout curseurs par défaut et scripts d'automatisation"
    log_success "Modifications commitées"
  fi
fi
echo ""

# ============================================
# ÉTAPE 7 : Synchronisation avec GitHub
# ============================================
log_step "ÉTAPE 7/10 : Synchronisation avec GitHub"

log_info "Pull des derniers changements..."
if git pull origin main --rebase 2>/dev/null; then
  log_success "Repository à jour"
else
  log_warning "Impossible de pull (vérifier la connexion)"
fi

log_info "Push vers GitHub..."
if git push origin main 2>/dev/null; then
  log_success "Changements poussés vers GitHub"
else
  log_warning "Impossible de push (vérifier la connexion)"
fi
echo ""

# ============================================
# ÉTAPE 8 : Création du Tag
# ============================================
log_step "ÉTAPE 8/10 : Création du Tag v${VERSION}"

if git rev-parse "$TAG" >/dev/null 2>&1; then
  log_warning "Le tag $TAG existe déjà"
  read -p "Supprimer et recréer ? (y/N) " -n 1 -r
  echo
  if [[ $REPLY =~ ^[Yy]$ ]]; then
    git tag -d "$TAG"
    git push origin ":refs/tags/$TAG" 2>/dev/null || true
    log_info "Tag supprimé"
  else
    log_info "Conservation du tag existant"
  fi
fi

if ! git rev-parse "$TAG" >/dev/null 2>&1; then
  log_info "Création du tag $TAG..."
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

  log_info "Push du tag vers GitHub..."
  if git push origin "$TAG" 2>/dev/null; then
    log_success "Tag poussé vers GitHub"
  else
    log_warning "Impossible de push le tag"
  fi
fi
echo ""

# ============================================
# ÉTAPE 9 : Préparation de la Release
# ============================================
log_step "ÉTAPE 9/10 : Préparation de la Release GitHub"

log_info "Génération du fichier de release..."
if [ -f "RELEASE_NOTES_v${VERSION}.md" ]; then
  log_success "Fichier de release déjà présent"
else
  log_warning "Fichier de release non trouvé"
fi

log_info "Vérification des artefacts..."
if [ -d "packages" ] && [ "$(ls -A packages)" ]; then
  log_success "Artefacts disponibles :"
  ls -lh packages/ | tail -n +2 | awk '{print "  - " $9 " (" $5 ")"}'
else
  log_warning "Aucun artefact trouvé dans packages/"
fi
echo ""

# ============================================
# ÉTAPE 10 : Publication GitHub Automatique
# ============================================
log_step "ÉTAPE 10/11 : Publication GitHub Automatique"

if [ $ERRORS -eq 0 ]; then
  log_info "Préparation de la publication GitHub..."

  # Vérifier si le token GitHub est défini
  if [ -n "$GITHUB_TOKEN" ]; then
    log_success "Token GitHub détecté"

    read -p "Publier automatiquement sur GitHub ? (Y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Nn]$ ]]; then
      log_info "Lancement de la publication..."
      chmod +x scripts/publish-github-release.sh 2>/dev/null || true

      if ./scripts/publish-github-release.sh; then
        log_success "Release publiée sur GitHub !"
      else
        log_warning "Échec de publication automatique"
        log_info "Publication manuelle nécessaire"
      fi
    else
      log_info "Publication manuelle choisie"
    fi
  else
    log_warning "GITHUB_TOKEN non défini"
    log_info "Pour activer la publication automatique :"
    echo "  export GITHUB_TOKEN=votre_token"
    echo "  Ou utiliser : gh auth login"
  fi
else
  log_warning "Publication ignorée à cause des erreurs"
fi
echo ""

# ============================================
# ÉTAPE 11 : Instructions Finales
# ============================================
log_step "ÉTAPE 11/11 : Résumé Final"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✅ Automatisation Terminée !${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

if [ $ERRORS -eq 0 ]; then
  echo -e "${GREEN}🎉 Aucune erreur détectée !${NC}"
else
  echo -e "${RED}❌ $ERRORS erreur(s) détectée(s)${NC}"
fi

if [ $WARNINGS -gt 0 ]; then
  echo -e "${YELLOW}⚠ $WARNINGS avertissement(s)${NC}"
fi

echo ""

# Vérifier si la release a été publiée
if [ -n "$GITHUB_TOKEN" ] && [ $ERRORS -eq 0 ]; then
  RELEASE_CHECK=$(curl -s -H "Authorization: token $GITHUB_TOKEN" \
    "https://api.github.com/repos/Roddygithub/Open-Yolo/releases/tags/${TAG}" 2>/dev/null | jq -r '.html_url // empty')

  if [ -n "$RELEASE_CHECK" ]; then
    echo -e "${GREEN}✅ Release publiée sur GitHub !${NC}"
    echo -e "${CYAN}🔗 $RELEASE_CHECK${NC}"
    echo ""
  else
    echo -e "${YELLOW}📋 Publication manuelle nécessaire :${NC}"
    echo ""
    echo -e "1. ${YELLOW}Créer la release sur GitHub :${NC}"
    echo -e "   ${CYAN}https://github.com/Roddygithub/Open-Yolo/releases/new?tag=${TAG}${NC}"
    echo ""
    echo -e "2. ${YELLOW}Copier la description depuis :${NC}"
    echo -e "   ${CYAN}GITHUB_RELEASE_v${VERSION}.md${NC}"
    echo ""
    echo -e "3. ${YELLOW}Uploader les artefacts depuis :${NC}"
    echo -e "   ${CYAN}packages/${NC}"
    if [ -d "packages" ]; then
      ls -1 packages/ | sed 's/^/   - /'
    fi
    echo ""
  fi
else
  echo -e "${YELLOW}📋 Pour publication manuelle :${NC}"
  echo ""
  echo -e "1. ${YELLOW}Exporter le token GitHub :${NC}"
  echo -e "   ${CYAN}export GITHUB_TOKEN=votre_token${NC}"
  echo ""
  echo -e "2. ${YELLOW}Publier automatiquement :${NC}"
  echo -e "   ${CYAN}./scripts/publish-github-release.sh${NC}"
  echo ""
  echo -e "3. ${YELLOW}Ou manuellement sur :${NC}"
  echo -e "   ${CYAN}https://github.com/Roddygithub/Open-Yolo/releases/new?tag=${TAG}${NC}"
  echo ""
fi

echo -e "${MAGENTA}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${MAGENTA}  🚀 Release v${VERSION} Complète !${NC}"
echo -e "${MAGENTA}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""
