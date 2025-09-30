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
    local deps=("gosu" "getent" "id" "chown" "chmod" "mkdir")
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
    
    # Créer le groupe s'il n'existe pas
    if ! getent group "$USER_GID" >/dev/null; then
        log "INFO" "Création du groupe $USERNAME avec GID $USER_GID..."
        groupadd --gid "$USER_GID" "$USERNAME" || {
            log "ERROR" "Échec de la création du groupe $USERNAME"
            return 1
        }
    fi
    
    # Créer l'utilisateur s'il n'existe pas
    if ! getent passwd "$USER_UID" >/dev/null; then
        local home_dir="$USER_HOME"
        
        log "INFO" "Création de l'utilisateur $USERNAME avec UID $USER_UID..."
        useradd --uid "$USER_UID" --gid "$USER_GID" --create-home --home-dir "$home_dir" "$USERNAME" || {
            log "ERROR" "Échec de la création de l'utilisateur $USERNAME"
            return 1
        }
        
        # Ajouter l'utilisateur aux groupes nécessaires
        usermod -aG video,audio,input "$USERNAME" || {
            log "WARNING" "Impossible d'ajouter l'utilisateur aux groupes supplémentaires"
        }
        
        log "INFO" "Utilisateur $USERNAME créé avec succès"
    else
        log "INFO" "Utilisateur $USERNAME (UID: $USER_UID) existe déjà"
    fi
    
    # Définir le répertoire personnel
    export HOME="$USER_HOME"
    
    return 0
}

# Configurer les répertoires de l'application
setup_application_directories() {
    log "INFO" "Configuration des répertoires de l'application..."
    
    # Créer les répertoires système
    local system_dirs=("$LOG_DIR" "$CONFIG_DIR" "$DATA_DIR" "$CACHE_DIR")
    
    for dir in "${system_dirs[@]}"; do
        mkdir -p "$dir"
        chown -R "$USER_UID:$USER_GID" "$dir"
        chmod 755 "$dir"
    done
    
    # Créer les répertoires utilisateur
    local user_dirs=(
        "$USER_HOME/.local/share/${APP_NAME,,}"
        "$USER_HOME/.config/${APP_NAME,,}"
        "$USER_HOME/.cache/${APP_NAME,,}"
        "$USER_HOME/.local/state/${APP_NAME,,}/logs"
    )
    
    for dir in "${user_dirs[@]}"; do
        mkdir -p "$dir"
        chown -R "$USER_UID:$USER_GID" "$(dirname "$dir")"
        chmod -R 755 "$dir"
    done
    
    # Créer des liens symboliques si nécessaire
    if [ ! -L "$USER_HOME/.${APP_NAME,,}" ]; then
        ln -sf "$USER_HOME/.local/share/${APP_NAME,,}" "$USER_HOME/.${APP_NAME,,}"
    fi
    
    return 0
}

# Copier les fichiers de configuration par défaut
copy_default_configs() {
    log "INFO" "Vérification des fichiers de configuration..."
    
    local config_src="$APP_HOME/src/data/config"
    local config_dest="$USER_HOME/.config/${APP_NAME,,}"
    
    # Créer le répertoire de configuration s'il n'existe pas
    mkdir -p "$config_dest"
    
    # Copier les fichiers de configuration s'ils n'existent pas
    if [ -d "$config_src" ]; then
        for config_file in "$config_src"/*; do
            local dest_file="$config_dest/$(basename "$config_file")"
            if [ ! -f "$dest_file" ] && [ -f "$config_file" ]; then
                log "INFO" "Copie du fichier de configuration: $(basename "$config_file")"
                cp "$config_file" "$dest_file"
                chown "$USER_UID:$USER_GID" "$dest_file"
                chmod 644 "$dest_file"
            fi
        done
    else
        log "WARNING" "Répertoire de configuration source introuvable: $config_src"
    fi
    
    return 0
}

# Configurer l'environnement X11
setup_x11_environment() {
    log "INFO" "Configuration de l'environnement X11..."
    
    # Variables d'environnement X11
    export DISPLAY=${DISPLAY:-:99}
    export XAUTHORITY=${XAUTHORITY:-/tmp/.Xauthority}
    
    # Créer le répertoire .X11-unix s'il n'existe pas
    mkdir -p /tmp/.X11-unix
    chmod 1777 /tmp/.X11-unix
    
    # Configurer Xauthority
    if [ ! -f "$XAUTHORITY" ]; then
        touch "$XAUTHORITY"
    fi
    
    chmod 666 "$XAUTHORITY" 2>/dev/null || true
    
    # Configurer le répertoire de runtime XDG
    export XDG_RUNTIME_DIR="/tmp/runtime-$USERNAME"
    mkdir -p "$XDG_RUNTIME_DIR"
    chown -R "$USER_UID:$USER_GID" "$XDG_RUNTIME_DIR"
    chmod 700 "$XDG_RUNTIME_DIR"
    
    return 0
}

# Configurer l'environnement GTK
setup_gtk_environment() {
    log "INFO" "Configuration de l'environnement GTK..."
    
    # Configuration GTK de base
    export GTK_THEME="Adwaita"
    export GDK_BACKEND="x11"
    export NO_AT_BRIDGE=1
    export CLUTTER_BACKEND="x11"
    
    # Optimisations de performance
    export G_SLICE="always-malloc"
    export G_DEBUG="gc-friendly"
    export G_ENABLE_DIAGNOSTIC=0
    
    # Désactiver les décorations côté client et les animations
    export GTK_CSD=0
    export GTK_OVERLAY_SCROLLING=0
    
    # Créer le fichier de configuration GTK
    local gtk_config_dir="$USER_HOME/.config/gtk-3.0"
    mkdir -p "$gtk_config_dir"
    
    cat > "$gtk_config_dir/settings.ini" << EOF
[Settings]
gtk-theme-name=Adwaita
gtk-icon-theme-name=Adwaita
gtk-font-name=Sans 10
gtk-cursor-theme-name=Adwaita
gtk-enable-animations=0
gtk-application-prefer-dark-theme=false
EOF
    
    chown -R "$USER_UID:$USER_GID" "$gtk_config_dir"
    
    return 0
}

# Démarrer l'application
start_application() {
    local app_cmd="/workspace/src/start-app.sh"
    
    # Vérifier si le script existe
    if [ ! -f "$app_cmd" ]; then
        log "ERROR" "Script de démarrage introuvable: $app_cmd"
        return 1
    fi
    
    # Rendre le script exécutable
    chmod +x "$app_cmd"
    
    # Démarrer l'application avec gosu
    log "INFO" "Démarrage de $APP_NAME en tant que $USERNAME (UID: $USER_UID, GID: $USER_GID)..."
    log "INFO" "Commande: $app_cmd $*"
    
    exec gosu "$USERNAME" "$app_cmd" "$@"
}

# Fonction principale
main() {
    log "INFO" "=== Démarrage de $APP_NAME v$ENTRYPOINT_VERSION ==="
    
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
        log "ERROR" "Échec de la configuration de l'environnement X11"
        exit 1
    }
    
    # Configurer l'environnement GTK
    setup_gtk_environment || {
        log "WARNING" "Échec de la configuration de l'environnement GTK"
    }
    
    # Démarrer l'application
    start_application "$@"
}

# Exécuter la fonction principale
main "$@"
