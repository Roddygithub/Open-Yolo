#!/bin/bash

# Script de configuration de l'environnement de développement pour OpenYolo
# Ce script doit être exécuté sur un système Linux avec Docker et Docker Compose installés

# Vérifier que Docker est installé
if ! command -v docker &> /dev/null; then
    echo "Erreur : Docker n'est pas installé. Veuillez installer Docker avant de continuer."
    echo "Voir https://docs.docker.com/engine/install/ pour les instructions d'installation."
    exit 1
fi

# Vérifier que Docker Compose est installé
if ! command -v docker-compose &> /dev/null; then
    echo "Erreur : Docker Compose n'est pas installé. Veuillez installer Docker Compose avant de continuer."
    echo "Voir https://docs.docker.com/compose/install/ pour les instructions d'installation."
    exit 1
fi

# Vérifier que l'utilisateur est dans le groupe docker
if ! groups | grep -q "\bdocker\b"; then
    echo "Attention : Votre utilisateur n'est pas dans le groupe 'docker'."
    echo "Pour ajouter votre utilisateur au groupe docker, exécutez :"
    echo "  sudo usermod -aG docker $USER"
    echo "Puis déconnectez-vous et reconnectez-vous pour que les changements prennent effet."
    exit 1
fi

# Vérifier que le répertoire du projet est correct
if [ ! -f "docker-compose.yml" ]; then
    echo "Erreur : Ce script doit être exécuté depuis le répertoire racine du projet OpenYolo."
    exit 1
fi

# Créer les répertoires nécessaires
echo "Création des répertoires nécessaires..."
mkdir -p build

# Donner les bonnes permissions aux scripts
echo "Configuration des permissions des scripts..."
chmod +x scripts/*.sh

# Construire les images Docker
echo "Construction des images Docker (cela peut prendre plusieurs minutes)..."
docker-compose build

# Démarrer les services
echo "Démarrage des services..."
docker-compose up -d

# Afficher les informations de connexion
echo ""
echo "=================================================="
echo "Environnement de développement OpenYolo configuré !"
echo "=================================================="
echo ""
echo "Pour accéder au conteneur de développement :"
echo "  docker-compose exec app bash"
echo ""
echo "Pour compiler le projet :"
echo "  ./scripts/build.sh"
echo ""
echo "Pour exécuter les tests :"
echo "  ./scripts/run-tests.sh"
echo ""
echo "Pour arrêter l'environnement :"
echo "  docker-compose down"
echo ""
echo "Pour consulter les logs :"
echo "  docker-compose logs -f"
echo ""
echo "Pour plus d'informations, consultez le fichier README.md"
echo ""
