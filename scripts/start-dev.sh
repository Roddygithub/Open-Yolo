#!/bin/bash

# Arrêter et supprimer les conteneurs existants
echo "Arrêt des conteneurs existants..."
docker-compose down

# Construire les images
echo "Construction des images Docker..."
docker-compose build

# Démarrer les services
echo "Démarrage de l'environnement de développement..."
docker-compose up -d

# Afficher les logs du conteneur
echo "Affichage des logs (CTRL+C pour quitter)..."
docker-compose logs -f
