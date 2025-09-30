#!/bin/bash

# Vérifier si Docker est en cours d'exécution
if ! docker info > /dev/null 2>&1; then
    echo "Erreur : Le démon Docker ne semble pas en cours d'exécution."
    exit 1
fi

# Vérifier si le conteneur est en cours d'exécution
if ! docker-compose ps -q app > /dev/null 2>&1; then
    echo "Le conteneur n'est pas en cours d'exécution. Démarrage en cours..."
    docker-compose up -d
    
    # Attendre que le conteneur soit prêt
    echo "Attente du démarrage du conteneur..."
    sleep 5
fi

# Exécuter la commande dans le conteneur
docker-compose exec -u devuser app "$@"
