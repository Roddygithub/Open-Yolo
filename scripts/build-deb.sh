#!/bin/bash

# Vérifier si le script est exécuté en tant que root
if [ "$(id -u)" -ne 0 ]; then
    echo "Ce script doit être exécuté en tant qu'administrateur (root)."
    exit 1
fi

# Variables
VERSION=$(git describe --tags 2>/dev/null || echo "1.0.0")
PACKAGE_NAME="open-yolo"
PACKAGE_VERSION="${VERSION#v}"
ARCHITECTURE="amd64"
MAINTAINER="Votre Nom <votre@email.com>"
DESCRIPTION="Gestionnaire de curseurs personnalisés pour Linux"

# Créer la structure du paquet
BUILD_DIR="/tmp/${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}"
DEBIAN_DIR="${BUILD_DIR}/DEBIAN"
BIN_DIR="${BUILD_DIR}/usr/bin"
SHARE_DIR="${BUILD_DIR}/usr/share/${PACKAGE_NAME}"
APPS_DIR="${BUILD_DIR}/usr/share/applications"
ICONS_DIR="${BUILD_DIR}/usr/share/icons/hicolor/256x256/apps"

# Nettoyer les anciennes constructions
rm -rf "${BUILD_DIR}"
mkdir -p "${DEBIAN_DIR}" "${BIN_DIR}" "${SHARE_DIR}" "${APPS_DIR}" "${ICONS_DIR}"

# Compiler le projet
mkdir -p build
cd build || exit 1
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j"$(nproc)"

# Installer les fichiers dans le répertoire de construction
make DESTDIR="${BUILD_DIR}" install

# Créer le fichier de contrôle
cat > "${DEBIAN_DIR}/control" << 'EOF'
Package: ${PACKAGE_NAME}
Version: ${PACKAGE_VERSION}
Section: utils
Priority: optional
Architecture: ${ARCHITECTURE}
Maintainer: ${MAINTAINER}
Description: ${DESCRIPTION}
 Gestionnaire de curseurs personnalisés pour Linux avec support des animations,
 multi-écrans et rendu GPU optimisé.
Depends: libgtkmm-3.0-dev, libx11-dev, libxcursor-dev, libxrandr-dev, libxinerama-dev, libxi-dev
EOF

# Créer les scripts de pré/post installation
cat > "${DEBIAN_DIR}/postinst" << 'EOF'
#!/bin/bash
# Mise à jour des bases de données
echo "Mise à jour des bases de données système..."
update-desktop-database /usr/share/applications
update-mime-database /usr/share/mime
gtk-update-icon-cache /usr/share/icons/hicolor

# Créer le répertoire de configuration utilisateur
USER_CONFIG_DIR="${HOME}/.config/open-yolo"
if [ ! -d "${USER_CONFIG_DIR}" ]; then
    mkdir -p "${USER_CONFIG_DIR}"
    # Copier les fichiers de configuration par défaut
    if [ -d "/usr/share/open-yolo/config" ]; then
        cp -r /usr/share/open-yolo/config/* "${USER_CONFIG_DIR}/"
    fi
    chown -R ${SUDO_USER}:${SUDO_USER} "${USER_CONFIG_DIR}"
fi
echo "Installation terminée avec succès !"
exit 0
EOF

# Rendre les scripts exécutables
chmod 755 "${DEBIAN_DIR}/postinst"

# Construire le paquet
cd "$(dirname "${BUILD_DIR}")" || exit 1
dpkg-deb --build "$(basename "${BUILD_DIR}")"

# Vérifier que le paquet a été créé
DEB_PACKAGE="${PACKAGE_NAME}_${PACKAGE_VERSION}_${ARCHITECTURE}.deb"
if [ -f "${DEB_PACKAGE}" ]; then
    echo "Paquet créé avec succès : ${DEB_PACKAGE}"
    
    # Vérifier les dépendances
    echo "Vérification des dépendances..."
    dpkg -I "${DEB_PACKAGE}"
    
    # Vérifier le contenu du paquet
    echo "Contenu du paquet :"
    dpkg -c "${DEB_PACKAGE}"
    
    # Déplacer le paquet dans le répertoire du projet
    mv "${DEB_PACKAGE}" "${OLDPWD}/"
    echo "Paquet déplacé vers : ${OLDPWD}/${DEB_PACKAGE}"
else
    echo "Erreur lors de la création du paquet."
    exit 1
fi

# Nettoyer
rm -rf "${BUILD_DIR}"
echo "Construction terminée !"
