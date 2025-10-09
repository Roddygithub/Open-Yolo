#!/bin/bash

# Script d'installation des icônes pour OpenYolo
# Ce script doit être exécuté avec les privilèges root ou via sudo

set -euo pipefail

# Chemins des répertoires d'icônes
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
ICON_SRC_DIR="${PROJECT_ROOT}/resources/icons/hicolor"
INSTALL_DIR="/usr/share/icons/hicolor"

# Vérifier si le répertoire des ressources existe
if [ ! -d "$ICON_SRC_DIR" ]; then
  echo "Erreur: Le répertoire des ressources d'icônes est introuvable: $ICON_SRC_DIR"
  exit 1
fi

# Vérifier les permissions
if [ "$(id -u)" -ne 0 ]; then
  echo "Erreur: Ce script doit être exécuté en tant que root ou avec sudo"
  exit 1
fi

# Fonction pour copier les icônes
copy_icons() {
  local src_dir="$1"
  local dest_dir="$2"

  if [ -d "$src_dir" ]; then
    mkdir -p "$dest_dir"
    if [ -f "$src_dir/OpenYolo.png" ]; then
      cp -v "$src_dir/OpenYolo.png" "$dest_dir/"
    elif [ -f "$src_dir/OpenYolo.svg" ]; then
      cp -v "$src_dir/OpenYolo.svg" "$dest_dir/"
    fi
  fi
}

# Créer le répertoire de destination s'il n'existe pas
mkdir -p "$INSTALL_DIR"

# Copier les icônes pour chaque taille
for size_dir in "$ICON_SRC_DIR"/*/; do
  if [ -d "$size_dir" ]; then
    size=$(basename "$size_dir")
    echo "Installation des icônes $size..."

    # Vérifier si c'est un dossier scalable
    if [ "$size" = "scalable" ]; then
      copy_icons "$size_dir/apps" "$INSTALL_DIR/$size/apps"
      if [ -d "$size_dir/actions" ]; then
        copy_icons "$size_dir/actions" "$INSTALL_DIR/$size/actions"
      fi
    else
      # Pour les autres tailles, copier uniquement le dossier apps
      copy_icons "$size_dir/apps" "$INSTALL_DIR/$size/apps"
    fi
  fi
done

# Mettre à jour le cache des icônes
echo "Mise à jour du cache des icônes..."
if command -v gtk-update-icon-cache >/dev/null 2>&1; then
  gtk-update-icon-cache -f -t "$INSTALL_DIR" || true
  echo "Cache des icônes mis à jour"
else
  echo "Attention: gtk-update-icon-cache n'est pas installé, le cache des icônes ne sera pas mis à jour"
fi

# Mettre à jour la base de données des applications desktop
echo "Mise à jour de la base de données des applications..."
if command -v update-desktop-database >/dev/null 2>&1; then
  update-desktop-database /usr/share/applications 2>/dev/null || true
  echo "Base de données des applications mise à jour"
else
  echo "Attention: update-desktop-database n'est pas installé, la base de données des applications ne sera pas mise à jour"
fi

echo "Installation des icônes terminée avec succès !"
exit 0
