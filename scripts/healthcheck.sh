#!/bin/bash

# Vérifier si le serveur Xvfb est en cours d'exécution
if ! pgrep -x "Xvfb" > /dev/null; then
    echo "Erreur: Xvfb n'est pas en cours d'exécution"
    exit 1
fi

# Vérifier si l'application est installée
if ! command -v OpenYolo &> /dev/null; then
    echo "Erreur: L'application OpenYolo n'est pas installée"
    exit 1
fi

# Vérifier si l'application peut s'exécuter (sans lancer d'interface graphique)
if ! OpenYolo --version &> /dev/null; then
    echo "Erreur: Impossible d'exécuter l'application OpenYolo"
    exit 1
fi

# Vérifier l'accès aux dossiers nécessaires
REQUIRED_DIRS=(
    "$XDG_CONFIG_HOME/OpenYolo"
    "$XDG_DATA_HOME/OpenYolo"
    "$XDG_CACHE_HOME"
)

for dir in "${REQUIRED_DIRS[@]}"; do
    if [ ! -d "$dir" ] || [ ! -w "$dir" ]; then
        echo "Erreur: Le dossier $dir n'existe pas ou n'est pas accessible en écriture"
        exit 1
    fi
done

# Vérifier les variables d'environnement requises
REQUIRED_ENV_VARS=(
    "DISPLAY"
    "XDG_RUNTIME_DIR"
    "GTK_THEME"
    "GDK_BACKEND"
)

for var in "${REQUIRED_ENV_VARS[@]}"; do
    if [ -z "${!var}" ]; then
        echo "Erreur: La variable d'environnement $var n'est pas définie"
        exit 1
    fi
done

# Si tout est OK
echo "Vérification de santé réussie"
exit 0
