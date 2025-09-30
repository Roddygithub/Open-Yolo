#!/bin/bash
# Script pour incrémenter automatiquement la version

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
VERSION_FILE="$PROJECT_ROOT/VERSION"

# Couleurs
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m'

# Lire la version actuelle
if [ ! -f "$VERSION_FILE" ]; then
    echo -e "${RED}ERROR: VERSION file not found${NC}"
    exit 1
fi

CURRENT_VERSION=$(cat "$VERSION_FILE")
echo -e "${BLUE}Current version: $CURRENT_VERSION${NC}"

# Parser la version
IFS='.' read -r -a VERSION_PARTS <<< "$CURRENT_VERSION"
MAJOR="${VERSION_PARTS[0]}"
MINOR="${VERSION_PARTS[1]}"
PATCH="${VERSION_PARTS[2]}"

# Demander le type de bump
echo ""
echo "Select version bump type:"
echo "  1) Patch (bug fixes)       : $MAJOR.$MINOR.$((PATCH + 1))"
echo "  2) Minor (new features)    : $MAJOR.$((MINOR + 1)).0"
echo "  3) Major (breaking changes): $((MAJOR + 1)).0.0"
echo ""
read -p "Enter choice [1-3]: " CHOICE

case $CHOICE in
    1)
        NEW_VERSION="$MAJOR.$MINOR.$((PATCH + 1))"
        BUMP_TYPE="patch"
        ;;
    2)
        NEW_VERSION="$MAJOR.$((MINOR + 1)).0"
        BUMP_TYPE="minor"
        ;;
    3)
        NEW_VERSION="$((MAJOR + 1)).0.0"
        BUMP_TYPE="major"
        ;;
    *)
        echo -e "${RED}Invalid choice${NC}"
        exit 1
        ;;
esac

echo ""
echo -e "${YELLOW}Bumping version from $CURRENT_VERSION to $NEW_VERSION ($BUMP_TYPE)${NC}"
echo ""

# Mettre à jour VERSION
echo "$NEW_VERSION" > "$VERSION_FILE"
echo -e "${GREEN}✓ Updated VERSION file${NC}"

# Mettre à jour CMakeLists.txt
if [ -f "$PROJECT_ROOT/CMakeLists.txt" ]; then
    sed -i "s/VERSION [0-9]\+\.[0-9]\+\.[0-9]\+/VERSION $NEW_VERSION/" "$PROJECT_ROOT/CMakeLists.txt"
    echo -e "${GREEN}✓ Updated CMakeLists.txt${NC}"
fi

# Créer une nouvelle entrée dans CHANGELOG.md
CHANGELOG="$PROJECT_ROOT/CHANGELOG.md"
if [ -f "$CHANGELOG" ]; then
    DATE=$(date +%Y-%m-%d)
    
    # Créer un fichier temporaire avec la nouvelle entrée
    TEMP_FILE=$(mktemp)
    
    # Écrire l'en-tête du changelog
    head -n 7 "$CHANGELOG" > "$TEMP_FILE"
    
    # Ajouter la nouvelle version
    echo "" >> "$TEMP_FILE"
    echo "## [$NEW_VERSION] - $DATE" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
    echo "### ✨ Ajouté" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
    echo "- TODO: Add new features here" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
    echo "### 🔧 Modifié" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
    echo "- TODO: Add modifications here" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
    echo "### 🐛 Corrigé" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
    echo "- TODO: Add bug fixes here" >> "$TEMP_FILE"
    echo "" >> "$TEMP_FILE"
    
    # Ajouter le reste du changelog
    tail -n +8 "$CHANGELOG" >> "$TEMP_FILE"
    
    # Remplacer le fichier original
    mv "$TEMP_FILE" "$CHANGELOG"
    
    echo -e "${GREEN}✓ Updated CHANGELOG.md${NC}"
fi

# Créer le fichier de release GitHub
RELEASE_FILE="$PROJECT_ROOT/GITHUB_RELEASE_v${NEW_VERSION}.md"
cat > "$RELEASE_FILE" << EOF
# 🎉 Open-Yolo v${NEW_VERSION}

**Date de release :** $(date +"%d %B %Y")

---

## ✨ Nouveautés

- TODO: Add new features

## 🔧 Améliorations

- TODO: Add improvements

## 🐛 Corrections

- TODO: Add bug fixes

---

## 📦 Installation

### Ubuntu/Debian

\`\`\`bash
wget https://github.com/Roddygithub/Open-Yolo/releases/download/v${NEW_VERSION}/open-yolo_${NEW_VERSION}_amd64.deb
sudo dpkg -i open-yolo_${NEW_VERSION}_amd64.deb
\`\`\`

### Fedora/RHEL

\`\`\`bash
wget https://github.com/Roddygithub/Open-Yolo/releases/download/v${NEW_VERSION}/open-yolo-${NEW_VERSION}-1.x86_64.rpm
sudo dnf install open-yolo-${NEW_VERSION}-1.x86_64.rpm
\`\`\`

### Arch Linux

\`\`\`bash
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo
makepkg -si
\`\`\`

### NixOS

\`\`\`bash
nix run github:Roddygithub/Open-Yolo
\`\`\`

---

## 📝 Changelog Complet

Voir [CHANGELOG.md](CHANGELOG.md) pour tous les détails.
EOF

echo -e "${GREEN}✓ Created $RELEASE_FILE${NC}"

# Résumé
echo ""
echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  Version Bump Complete${NC}"
echo -e "${BLUE}========================================${NC}"
echo ""
echo -e "Old version: ${YELLOW}$CURRENT_VERSION${NC}"
echo -e "New version: ${GREEN}$NEW_VERSION${NC}"
echo ""
echo "Files updated:"
echo "  - VERSION"
echo "  - CMakeLists.txt"
echo "  - CHANGELOG.md"
echo "  - GITHUB_RELEASE_v${NEW_VERSION}.md"
echo ""
echo -e "${YELLOW}Next steps:${NC}"
echo "  1. Edit CHANGELOG.md and GITHUB_RELEASE_v${NEW_VERSION}.md"
echo "  2. Commit changes: git add -A && git commit -m 'Bump version to $NEW_VERSION'"
echo "  3. Create tag: git tag -a v$NEW_VERSION -m 'Release v$NEW_VERSION'"
echo "  4. Push: git push origin main --tags"
echo "  5. Run: ./scripts/auto-release.sh"
echo ""
