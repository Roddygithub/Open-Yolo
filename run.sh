#!/bin/bash

# Script pour gérer facilement le conteneur Open-Yolo

# Couleurs pour les messages
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Fonction pour afficher l'aide
show_help() {
    echo "Utilisation: $0 [commande]"
    echo "Commandes disponibles:"
    echo "  build         - Construit l'image Docker"
    echo "  run           - Lance l'application (mode production)"
    echo "  debug         - Lance un shell dans le conteneur pour le débogage"
    echo "  logs          - Affiche les logs de l'application"
    echo "  clean         - Nettoie les conteneurs et images inutilisés"
    echo "  rebuild       - Reconstruit complètement l'application"
    echo "  help          - Affiche cette aide"
}

# Vérifier si Docker est installé
if ! command -v docker &> /dev/null; then
    echo "Erreur: Docker n'est pas installé. Veuillez installer Docker d'abord."
    exit 1
fi

# Vérifier si Docker Compose est disponible
if ! command -v docker-compose &> /dev/null; then
    # Essayer avec docker compose (nouvelle syntaxe)
    if ! docker compose version &> /dev/null; then
        echo "Erreur: Docker Compose n'est pas installé. Veuillez l'installer d'abord."
        exit 1
    else
        # Utiliser la nouvelle syntaxe docker compose
        DOCKER_COMPOSE_CMD="docker compose"
    fi
else
    DOCKER_COMPOSE_CMD="docker-compose"
fi

# Vérifier les arguments
if [ $# -eq 0 ]; then
    show_help
    exit 1
fi

# Traitement des commandes
case "$1" in
    build)
        echo -e "${GREEN}Construction de l'image Docker...${NC}"
        $DOCKER_COMPOSE_CMD build
        ;;
        
    run)
        echo -e "${GREEN}Lancement de l'application...${NC}"
        $DOCKER_COMPOSE_CMD up open-yolo
        ;;
        
    debug)
        echo -e "${YELLOW}Lancement du mode débogage...${NC}"
        echo -e "${YELLOW}Utilisez 'make' pour recompiler et './bin/OpenYolo' pour lancer l'application${NC}"
        $DOCKER_COMPOSE_CMD run --service-ports open-yolo-debug /bin/bash
        ;;
        
    logs)
        $DOCKER_COMPOSE_CMD logs -f open-yolo
        ;;
        
    clean)
        echo -e "${YELLOW}Nettoyage des conteneurs et images inutilisés...${NC}"
        docker system prune -f
        ;;
        
    rebuild)
        echo -e "${GREEN}Reconstruction complète de l'application...${NC}"
        $DOCKER_COMPOSE_CMD build --no-cache
        $DOCKER_COMPOSE_CMD up -d --force-recreate
        ;;
        
    help|--help|-h)
        show_help
        ;;
        
    *)
        echo "Commande inconnue: $1"
        show_help
        exit 1
        ;;
esac

exit 0
