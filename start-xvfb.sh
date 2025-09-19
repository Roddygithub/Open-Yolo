#!/bin/bash
set -e

# Configuration de l'affichage
export DISPLAY=:99

# Création du répertoire X11 s'il n'existe pas
if [ ! -d "/tmp/.X11-unix" ]; then
    mkdir -p /tmp/.X11-unix
    chmod 1777 /tmp/.X11-unix
fi

# Arrêt de tout processus Xvfb existant
if command -v killall >/dev/null 2>&1; then
    killall Xvfb 2>/dev/null || true
fi

# Nettoyage des anciens fichiers X11
rm -f /tmp/.X99-lock /tmp/.X99-lock/* 2>/dev/null || true

# Démarrage de Xvfb
echo "Démarrage de Xvfb sur $DISPLAY avec une résolution de ${SCREEN_WIDTH}x${SCREEN_HEIGHT}x${SCREEN_DEPTH}..."
Xvfb $DISPLAY \
    -screen 0 ${SCREEN_WIDTH}x${SCREEN_HEIGHT}x${SCREEN_DEPTH} \
    -ac \
    -listen tcp \
    -nolisten inet6 \
    -noreset \
    -auth /tmp/xvfb.auth &

# Attente du démarrage
sleep 2

# Vérification du serveur X
echo "Vérification du serveur X..."
if ! xdpyinfo -display $DISPLAY >/dev/null 2>&1; then
    echo "ERREUR: Impossible de vérifier le serveur X" >&2
    echo "Sortie de xdpyinfo:" >&2
    xdpyinfo -display $DISPLAY || true
    echo "Processus Xvfb en cours d'exécution:" >&2
    ps aux | grep [X]vfb || true
    echo "Fichiers dans /tmp:" >&2
    ls -la /tmp/ || true
    exit 1
fi

echo "Xvfb est en cours d'exécution sur $DISPLAY"

# Configuration des permissions
xhost +local: || true

# Boucle de maintien en vie
echo "Démarrage de la boucle de surveillance..."
while true; do
    if ! xdpyinfo -display $DISPLAY >/dev/null 2>&1; then
        echo "ERREUR: Xvfb a cessé de fonctionner" >&2
        exit 1
    fi
    sleep 5
done
