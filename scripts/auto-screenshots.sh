#!/bin/bash
# Script automatique de génération de captures d'écran

set -e

SCREENSHOTS_DIR="docs/screenshots"
mkdir -p "$SCREENSHOTS_DIR"

BLUE='\033[0;34m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Génération Automatique de Captures${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""

# Vérifier les outils de capture disponibles
SCREENSHOT_TOOL=""
if command -v gnome-screenshot &>/dev/null; then
  SCREENSHOT_TOOL="gnome-screenshot"
elif command -v spectacle &>/dev/null; then
  SCREENSHOT_TOOL="spectacle"
elif command -v scrot &>/dev/null; then
  SCREENSHOT_TOOL="scrot"
elif command -v import &>/dev/null; then
  SCREENSHOT_TOOL="import"
else
  echo -e "${YELLOW}⚠ Aucun outil de capture trouvé${NC}"
  echo "Installation recommandée :"
  echo "  - GNOME : sudo apt install gnome-screenshot"
  echo "  - KDE : sudo apt install spectacle"
  echo "  - Générique : sudo apt install scrot"
  exit 1
fi

echo -e "${GREEN}✓${NC} Outil de capture : $SCREENSHOT_TOOL"
echo ""

# Fonction de capture
take_screenshot() {
  local filename=$1
  local description=$2
  local delay=${3:-3}

  echo -e "${YELLOW}📸 Capture : $description${NC}"
  echo "   Délai : ${delay}s"

  case $SCREENSHOT_TOOL in
    gnome-screenshot)
      gnome-screenshot -d $delay -f "$SCREENSHOTS_DIR/$filename"
      ;;
    spectacle)
      spectacle -b -d $((delay * 1000)) -o "$SCREENSHOTS_DIR/$filename"
      ;;
    scrot)
      sleep $delay
      scrot "$SCREENSHOTS_DIR/$filename"
      ;;
    import)
      sleep $delay
      import -window root "$SCREENSHOTS_DIR/$filename"
      ;;
  esac

  if [ -f "$SCREENSHOTS_DIR/$filename" ]; then
    echo -e "${GREEN}✓${NC} Sauvegardé : $filename"
  else
    echo -e "${YELLOW}⚠${NC} Échec de capture"
  fi
  echo ""
}

# Vérifier que l'application est compilée
if [ ! -f "build/src/OpenYolo" ]; then
  echo -e "${YELLOW}⚠ Application non compilée${NC}"
  echo "Compilation..."
  make build
fi

echo -e "${BLUE}Instructions :${NC}"
echo "1. L'application va se lancer"
echo "2. Préparez chaque vue demandée"
echo "3. La capture sera prise automatiquement après le délai"
echo ""
read -p "Appuyez sur Entrée pour commencer..."

# Lancer l'application
./build/src/OpenYolo &
APP_PID=$!
sleep 3

echo ""
echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo -e "${YELLOW}  Captures Automatiques${NC}"
echo -e "${YELLOW}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
echo ""

# Capture 1 : Interface principale
echo "1️⃣  Préparez : Interface principale (onglet Curseur)"
read -p "Appuyez sur Entrée quand prêt..."
take_screenshot "main-window.png" "Interface principale" 3

# Capture 2 : Onglet Effets
echo "2️⃣  Préparez : Onglet Effets"
read -p "Appuyez sur Entrée quand prêt..."
take_screenshot "effects-tab.png" "Onglet Effets" 3

# Capture 3 : Onglet Raccourcis
echo "3️⃣  Préparez : Onglet Raccourcis"
read -p "Appuyez sur Entrée quand prêt..."
take_screenshot "shortcuts-tab.png" "Onglet Raccourcis" 3

# Capture 4 : Curseur actif (optionnel)
read -p "Capturer le curseur actif ? (Y/n) " -n 1 -r
echo
if [[ ! $REPLY =~ ^[Nn]$ ]]; then
  echo "4️⃣  Préparez : Curseur personnalisé actif"
  read -p "Appuyez sur Entrée quand prêt..."
  take_screenshot "cursor-active.png" "Curseur actif" 3
fi

# Tuer l'application
kill $APP_PID 2>/dev/null || true

echo ""
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  ✅ Captures Terminées !${NC}"
echo -e "${GREEN}========================================${NC}"
echo ""

# Résumé
echo -e "${BLUE}📁 Captures sauvegardées dans : $SCREENSHOTS_DIR/${NC}"
if [ -d "$SCREENSHOTS_DIR" ]; then
  ls -lh "$SCREENSHOTS_DIR" | grep -v "^total" | awk '{print "  - " $9 " (" $5 ")"}'
fi

echo ""
echo -e "${YELLOW}📝 Prochaines étapes :${NC}"
echo "1. Vérifier les captures dans $SCREENSHOTS_DIR/"
echo "2. Optimiser les images si nécessaire"
echo "3. Commiter les captures : git add $SCREENSHOTS_DIR/"
echo "4. Les uploader dans la release GitHub"
echo ""
