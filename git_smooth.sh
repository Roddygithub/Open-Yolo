#!/bin/bash
# git_smooth.sh - Script pour workflow smooth Open-Yolo

set -e  # Stoppe en cas d'erreur

PROJECT_DIR=~/Documents/Open-Yolo
MAIN_BRANCH=main
DEV_BRANCH=development

cd "$PROJECT_DIR"

echo "📂 Vérification du dépôt Git..."
if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "❌ Ce dossier n'est pas un dépôt Git."
    exit 1
fi

# Afficher la branche actuelle
CURRENT_BRANCH=$(git branch --show-current)
echo "🌿 Branche actuelle : $CURRENT_BRANCH"

# Fonction pour pousser les commits sur development
push_development() {
    git checkout $DEV_BRANCH
    git pull --rebase origin $DEV_BRANCH

    echo "📝 Ajouter tous les fichiers modifiés..."
    git add .

    echo "💾 Commit des modifications (message obligatoire)"
    read -p "Message de commit: " MSG
    git commit -m "$MSG"

    echo "📤 Pousser vers development..."
    git push origin $DEV_BRANCH
}

# Fonction pour fusionner development vers main
merge_to_main() {
    git checkout $MAIN_BRANCH
    git pull origin $MAIN_BRANCH

    echo "🔀 Fusionner $DEV_BRANCH dans $MAIN_BRANCH..."
    git merge --no-ff $DEV_BRANCH

    echo "📤 Pousser main vers GitHub..."
    git push origin $MAIN_BRANCH
}

echo "Que voulez-vous faire ?"
echo "1) Pousser les changements vers development"
echo "2) Fusionner development dans main"
echo "3) Quitter"
read -p "Choix [1-3]: " CHOICE

case $CHOICE in
    1)
        push_development
        ;;
    2)
        merge_to_main
        ;;
    3)
        echo "Bye!"
        exit 0
        ;;
    *)
        echo "Choix invalide."
        exit 1
        ;;
esac

