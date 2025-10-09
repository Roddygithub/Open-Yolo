#!/bin/bash
# Script pour créer les captures d'écran pour la release

set -e

SCREENSHOTS_DIR="docs/screenshots"
mkdir -p "$SCREENSHOTS_DIR"

echo "========================================="
echo "  Création des Captures d'Écran"
echo "========================================="
echo ""
echo "Ce script va vous guider pour créer les captures d'écran nécessaires."
echo ""

# Vérifier que l'application est compilée
if [ ! -f "build/src/OpenYolo" ]; then
  echo "❌ Erreur : L'application n'est pas compilée"
  echo "Exécutez d'abord : make build"
  exit 1
fi

echo "📸 Captures nécessaires :"
echo ""
echo "1. Interface principale (onglet Curseur)"
echo "2. Curseur personnalisé actif"
echo "3. Curseur animé en action"
echo "4. Configuration multi-écrans (si disponible)"
echo "5. Onglet Effets"
echo "6. Onglet Raccourcis"
echo ""

read -p "Appuyez sur Entrée pour lancer l'application..."

# Lancer l'application
cd build/src
./OpenYolo &
APP_PID=$!

echo ""
echo "✅ Application lancée (PID: $APP_PID)"
echo ""
echo "📸 Instructions pour les captures :"
echo ""
echo "1. Utilisez votre outil de capture préféré :"
echo "   - gnome-screenshot (GNOME)"
echo "   - spectacle (KDE)"
echo "   - flameshot (multi-DE)"
echo "   - scrot (ligne de commande)"
echo ""
echo "2. Sauvegardez les captures dans : $SCREENSHOTS_DIR/"
echo "   avec les noms suivants :"
echo "   - main-window.png"
echo "   - cursor-active.png"
echo "   - cursor-animated.gif (ou .png)"
echo "   - multi-screen.png (optionnel)"
echo "   - effects-tab.png"
echo "   - shortcuts-tab.png"
echo ""
echo "3. Appuyez sur Entrée quand vous avez terminé..."

read -p ""

# Tuer l'application
kill $APP_PID 2>/dev/null || true

echo ""
echo "✅ Captures terminées !"
echo ""
echo "📁 Vérification des fichiers..."

CAPTURES=(
  "main-window.png:Interface principale"
  "cursor-active.png:Curseur actif"
  "effects-tab.png:Onglet Effets"
  "shortcuts-tab.png:Onglet Raccourcis"
)

FOUND=0
MISSING=0

for capture in "${CAPTURES[@]}"; do
  IFS=':' read -r file desc <<<"$capture"
  if [ -f "$SCREENSHOTS_DIR/$file" ]; then
    echo "✅ $desc : $file"
    ((FOUND++))
  else
    echo "⚠️  $desc : $file (manquant)"
    ((MISSING++))
  fi
done

echo ""
if [ $MISSING -eq 0 ]; then
  echo "🎉 Toutes les captures sont prêtes !"
else
  echo "⚠️  $MISSING capture(s) manquante(s)"
  echo "Vous pouvez les ajouter plus tard dans $SCREENSHOTS_DIR/"
fi

echo ""
echo "📝 Prochaine étape :"
echo "   Ajoutez ces images à la release GitHub"
echo ""
