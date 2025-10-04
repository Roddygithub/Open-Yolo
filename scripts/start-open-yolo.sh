#!/bin/bash
# Script de démarrage pour Open-Yolo

# Vérifier si le répertoire de configuration existe
CONFIG_DIR="${XDG_CONFIG_HOME:-$HOME/.config}/open-yolo"
if [ ! -d "$CONFIG_DIR" ]; then
    echo "Création du répertoire de configuration: $CONFIG_DIR"
    mkdir -p "$CONFIG_DIR"
    
    # Copier les fichiers de configuration par défaut s'ils n'existent pas
    if [ -d "/usr/share/open-yolo/config" ]; then
        echo "Copie des fichiers de configuration par défaut..."
        cp -r /usr/share/open-yolo/config/* "$CONFIG_DIR/"
    fi
fi

# Vérifier si le répertoire des données utilisateur existe
DATA_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/open-yolo"
if [ ! -d "$DATA_DIR" ]; then
    echo "Création du répertoire de données utilisateur: $DATA_DIR"
    mkdir -p "$DATA_DIR"
    
    # Créer un lien symbolique vers les curseurs système s'ils existent
    if [ -d "/usr/share/icons" ] && [ ! -e "$DATA_DIR/cursors" ]; then
        echo "Création d'un lien vers les curseurs système..."
        ln -s /usr/share/icons "$DATA_DIR/cursors"
    fi
fi

# Lancer l'application avec les paramètres appropriés
echo "Lancement d'Open-Yolo..."
/usr/bin/open-yolo --config "$CONFIG_DIR/config.yaml" --data-dir "$DATA_DIR" "$@"
