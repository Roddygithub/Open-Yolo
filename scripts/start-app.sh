#!/bin/bash

# Configuration
DISPLAY_NUM=99
APP_NAME="OpenYolo"
APP_CMD="/usr/local/bin/OpenYolo"

# Vérifier que Xvfb est en cours d'exécution
if ! pgrep -x "Xvfb" > /dev/null; then
    echo "Erreur: Xvfb n'est pas en cours d'exécution"
    exit 1
fi

# Exporter la variable DISPLAY
export DISPLAY=:${DISPLAY_NUM}

# Vérifier que l'application existe
if [ ! -f "${APP_CMD}" ]; then
    echo "Erreur: L'application ${APP_NAME} n'a pas été trouvée à l'emplacement ${APP_CMD}"
    exit 1
fi

# Démarrer l'application
echo "Démarrage de ${APP_NAME}..."
exec ${APP_CMD} "$@"
