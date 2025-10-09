#!/bin/bash
set -euo pipefail

# =============================================================================
# Configuration
# =============================================================================

# Version de l'entrypoint
ENTRYPOINT_VERSION="1.1.0"

# Configuration de l'utilisateur non-root
USERNAME="dev"
USER_UID=${UID:-1000}
USER_GID=${GID:-1000}

# Chemins importants
APP_NAME="OpenYolo"
APP_HOME="/workspace"
LOG_DIR="/var/log/${APP_NAME,,}"
CONFIG_DIR="/etc/${APP_NAME,,}"
DATA_DIR="/var/lib/${APP_NAME,,}"
CACHE_DIR="/var/cache/${APP_NAME,,}"
USER_HOME="/home/$USERNAME"

# =============================================================================
# Fonctions utilitaires
# =============================================================================

# Fonction pour afficher les messages de log
log() {
  local level="${1:-INFO}"
  local message="${2:-}"
  local timestamp=$(date +'%Y-%m-%d %H:%M:%S')
  echo "[$timestamp] [$level] $message"
}

# Vérifier si une commande existe
command_exists() {
  command -v "$1" >/dev/null 2>&1
}

# Vérifier et installer les dépendances système
check_system_dependencies() {
  local deps=("gosu" "getent" "id" "chown" "chmod" "mkdir" "dbus-launch" "dbus-run-session")
  local missing=()

  for dep in "${deps[@]}"; do
    if ! command_exists "$dep"; then
      missing+=("$dep")
    fi
  done

  if [ ${#missing[@]} -gt 0 ]; then
    log "ERROR" "Dépendances système manquantes: ${missing[*]}"
    return 1
  fi

  return 0
}

# Créer un utilisateur non-root
setup_nonroot_user() {
  log "INFO" "Configuration de l'utilisateur $USERNAME (UID: $USER_UID, GID: $USER_GID)..."

  if ! getent group "$USER_GID" >/dev/null; then
    groupadd -g "$USER_GID" "$USERNAME"
  fi

  if ! id -u "$USERNAME" >/dev/null 2>&1; then
    useradd -u "$USER_UID" -g "$USER_GID" -m -d "$USER_HOME" -s /bin/bash "$USERNAME"
  fi

  # S'assurer que les permissions sont correctes
  chown -R "$USER_UID:$USER_GID" "$USER_HOME"

  return 0
}

# Configurer les répertoires de l'application
setup_application_directories() {
  log "INFO" "Configuration des répertoires de l'application..."

  local dirs=("$LOG_DIR" "$CONFIG_DIR" "$DATA_DIR" "$CACHE_DIR")

  for dir in "${dirs[@]}"; do
    mkdir -p "$dir"
    chown -R "$USER_UID:$USER_GID" "$dir"
    chmod 755 "$dir"
  done

  return 0
}

# Copier les fichiers de configuration par défaut
copy_default_configs() {
  log "INFO" "Copie des fichiers de configuration par défaut..."

  if [ -d "/app/config" ] && [ "$(ls -A /app/config)" ]; then
    cp -r /app/config/* "$CONFIG_DIR"/
    chown -R "$USER_UID:$USER_GID" "$CONFIG_DIR"
  fi

  return 0
}

# Configurer l'environnement X11
setup_x11_environment() {
  log "INFO" "Configuration de l'environnement X11..."

  export DISPLAY=":0"
  export XAUTHORITY="/tmp/.Xauthority"

  if [ ! -f "$XAUTHORITY" ]; then
    touch "$XAUTHORITY"
  fi

  chmod 666 "$XAUTHORITY" 2>/dev/null || true

  # Configurer le répertoire de runtime XDG
  export XDG_RUNTIME_DIR="/tmp/runtime-$USERNAME"
  mkdir -p "$XDG_RUNTIME_DIR"
  chown -R "$USER_UID:$USER_GID" "$XDG_RUNTIME_DIR"

  return 0
}

# Configurer l'environnement GTK
setup_gtk_environment() {
  log "INFO" "Configuration de l'environnement GTK..."

  # Variables d'environnement GTK
  export GTK_THEME="Adwaita"
  export GDK_BACKEND="x11"
  export NO_AT_BRIDGE=1
  export CLUTTER_BACKEND="x11"

  # Optimisations de performance
  export LIBGL_DRI3_DISABLE=1
  export GDK_RENDERING="image"

  return 0
}

# Démarrer l'application
start_application() {
  log "INFO" "Démarrage de l'application $APP_NAME..."

  # Configurer l'environnement DBUS
  if command_exists "dbus-launch"; then
    eval "$(dbus-launch --exit-with-session)"
  fi

  # Exécuter l'application en tant qu'utilisateur non-root
  exec gosu "$USERNAME" "$@"
}

# Fonction principale
main() {
  log "INFO" "Démarrage de l'entrypoint v$ENTRYPOINT_VERSION"

  # Vérifier les dépendances système
  check_system_dependencies || {
    log "ERROR" "Échec de la vérification des dépendances système"
    exit 1
  }

  # Configurer l'utilisateur non-root
  setup_nonroot_user || {
    log "ERROR" "Échec de la configuration de l'utilisateur non-root"
    exit 1
  }

  # Configurer les répertoires de l'application
  setup_application_directories || {
    log "ERROR" "Échec de la configuration des répertoires de l'application"
    exit 1
  }

  # Copier les fichiers de configuration par défaut
  copy_default_configs || {
    log "WARNING" "Échec de la copie des fichiers de configuration par défaut"
  }

  # Configurer l'environnement X11
  setup_x11_environment || {
    log "WARNING" "Échec de la configuration de l'environnement X11"
  }

  # Configurer l'environnement GTK
  setup_gtk_environment || {
    log "WARNING" "Échec de la configuration de l'environnement GTK"
  }

  # Démarrer l'application avec les arguments passés
  start_application "$@"
}

# Exécuter la fonction principale
main "$@"
