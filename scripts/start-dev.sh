#!/bin/bash

# Fonction pour afficher un message d'erreur et quitter
error_exit() {
    echo "ERREUR: $1" >&2
    exit 1
}

# Vérifier si Docker est installé
if ! command -v docker &> /dev/null; then
    error_exit "Docker n'est pas installé. Veuillez installer Docker avant de continuer."
fi

# Vérifier si docker-compose est disponible
if ! command -v docker-compose &> /dev/null; then
    # Essayer avec docker compose (version plus récente)
    if ! docker compose version &> /dev/null; then
        error_exit "docker-compose n'est pas installé. Veuillez installer docker-compose avant de continuer."
    else
        # Utiliser docker compose au lieu de docker-compose
        DOCKER_COMPOSE_CMD="docker compose"
    fi
else
    DOCKER_COMPOSE_CMD="docker-compose"
fi

echo "========================================"
echo "  Configuration de l'environnement Docker"
echo "========================================"

# Arrêter et supprimer les conteneurs existants
echo "[1/4] Arrêt des conteneurs existants..."
$DOCKER_COMPOSE_CMD down || error_exit "Échec de l'arrêt des conteneurs"

# Construire les images
echo "[2/4] Construction des images Docker (cela peut prendre quelques minutes)..."
$DOCKER_COMPOSE_CMD build --no-cache || error_exit "Échec de la construction des images"

# Démarrer les services
echo "[3/4] Démarrage de l'environnement de développement..."
$DOCKER_COMPOSE_CMD up -d || error_exit "Échec du démarrage des conteneurs"

# Vérifier que les conteneurs sont en cours d'exécution
echo "[4/4] Vérification de l'état des conteneurs..."
if $DOCKER_COMPOSE_CMD ps | grep -q "Up"; then
    echo -e "\n========================================"
    echo -e "  Environnement prêt !"
    echo -e "========================================"
    echo -e "Pour accéder au conteneur, exécutez :"
    echo -e "  $DOCKER_COMPOSE_CMD exec openyolo bash"
    echo -e "\nPour afficher les logs :"
    echo -e "  $DOCKER_COMPOSE_CMD logs -f"
    echo -e "\nPour arrêter l'environnement :"
    echo -e "  $DOCKER_COMPOSE_CMD down"
    echo -e "========================================"
else
    error_exit "Les conteneurs ne sont pas démarrés correctement. Vérifiez les logs avec: $DOCKER_COMPOSE_CMD logs"
fi
