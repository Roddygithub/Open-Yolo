#!/bin/bash
# Script de désinstallation d'Open-Yolo

# Fonction pour afficher un message d'erreur et quitter
function error_exit {
  echo "[ERREUR] $1" >&2
  exit 1
}

# Vérifier les privilèges root
if [ "$(id -u)" -ne 0 ]; then
  error_exit "Ce script doit être exécuté en tant qu'administrateur (root)."
fi

echo "=== Désinstallation d'Open-Yolo ==="

# Arrêter le processus s'il est en cours d'exécution
if pgrep -x "open-yolo" >/dev/null; then
  echo "Arrêt du processus Open-Yolo en cours d'exécution..."
  pkill -x "open-yolo"
  sleep 1
fi

# Supprimer les fichiers d'installation
echo "Suppression des fichiers système..."

# Fichiers binaires
rm -f /usr/bin/open-yolo
rm -f /usr/local/bin/open-yolo

# Fichiers de l'application
rm -rf /usr/share/open-yolo
rm -f /usr/share/applications/open-yolo.desktop
rm -f /usr/share/icons/hicolor/*/apps/open-yolo.png
rm -f /usr/share/icons/hicolor/*/apps/open-yolo.svg

# Mise à jour des bases de données
echo "Mise à jour des bases de données système..."
update-desktop-database /usr/share/applications
update-mime-database /usr/share/mime
gtk-update-icon-cache /usr/share/icons/hicolor

# Determine the user who invoked sudo
SUDO_USER_HOME=$(getent passwd "${SUDO_USER:-$USER}" | cut -d: -f6)

# Demander la suppression des fichiers utilisateur
read -p "Voulez-vous supprimer les fichiers de configuration utilisateur ? [o/N] " -n 1 -r
echo
if [[ $REPLY =~ ^[Oo]$ ]]; then
  echo "Suppression des fichiers utilisateur..."
  rm -rf "$HOME/.config/open-yolo"
  rm -rf "$HOME/.local/share/open-yolo"
  rm -f "$HOME/.local/share/applications/open-yolo.desktop"
  if [ -n "$SUDO_USER_HOME" ] && [ -d "$SUDO_USER_HOME" ]; then
    echo "Suppression des fichiers utilisateur pour '$SUDO_USER'..."
    rm -rf "$SUDO_USER_HOME/.config/open-yolo"
    rm -rf "$SUDO_USER_HOME/.local/share/open-yolo"
  else
    echo "[AVERTISSEMENT] Impossible de déterminer le répertoire personnel de l'utilisateur. Fichiers de configuration non supprimés."
  fi
fi

echo "=== Désinstallation terminée avec succès ==="
echo "Merci d'avoir utilisé Open-Yolo !"
