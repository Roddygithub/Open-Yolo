#!/bin/bash

# Configuration
SCREEN_WIDTH=${SCREEN_WIDTH:-1280}
SCREEN_HEIGHT=${SCREEN_HEIGHT:-720}
SCREEN_DEPTH=${SCREEN_DEPTH:-24}
DISPLAY_NUM=99

# Arrêter Xvfb s'il est déjà en cours d'exécution
if pgrep -x "Xvfb" >/dev/null; then
  echo "Arrêt de Xvfb en cours d'exécution..."
  pkill -x Xvfb
  sleep 1
fi

# Nettoyer les fichiers X11 temporaires
rm -f /tmp/.X${DISPLAY_NUM}-lock
rm -f /tmp/.X11-unix/X${DISPLAY_NUM}

# Démarrer Xvfb en arrière-plan
echo "Démarrage de Xvfb sur l'écran :${DISPLAY_NUM} avec une résolution de ${SCREEN_WIDTH}x${SCREEN_HEIGHT}x${SCREEN_DEPTH}..."
Xvfb :${DISPLAY_NUM} \
  -screen 0 ${SCREEN_WIDTH}x${SCREEN_HEIGHT}x${SCREEN_DEPTH} \
  -ac \
  -noreset \
  -listen tcp \
  -dpi 96 \
  +extension RANDR \
  +extension GLX \
  +extension RENDER \
  +extension XFIXES &

# Attendre que Xvfb démarre
sleep 1

# Vérifier que Xvfb est en cours d'exécution
if ! pgrep -x "Xvfb" >/dev/null; then
  echo "Erreur: Impossible de démarrer Xvfb"
  exit 1
fi

echo "Xvfb est en cours d'exécution sur :${DISPLAY_NUM}"

# Garder le script en cours d'exécution
wait
