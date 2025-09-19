# Étape de construction
FROM ubuntu:22.04 as builder

# Définition des arguments de construction
ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Définition des variables d'environnement
ENV DEBIAN_FRONTEND=noninteractive \
    CC=/usr/bin/clang \
    CXX=/usr/bin/clang++ \
    # Optimisations de compilation
    CFLAGS="-march=native -O2 -pipe" \
    CXXFLAGS="-march=native -O2 -pipe" \
    LDFLAGS="-Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now"

# Installation des dépendances système
RUN echo 'APT::Get::Assume-Yes "true";' > /etc/apt/apt.conf.d/90assumeyes && \
    echo 'APT::Get::force-yes "true";' >> /etc/apt/apt.conf.d/90assumeyes && \
    echo 'APT::Get::Show-Upgraded "true";' >> /etc/apt/apt.conf.d/90assumeyes && \
    # Mise à jour des dépôts
    apt-get update -qq && \
    # Installation des dépendances système essentielles
    apt-get install -y --no-install-recommends \
        # Outils système de base
        ca-certificates \
        curl \
        wget \
        gnupg \
        lsb-release \
        software-properties-common \
        # Outils de développement
        build-essential \
        clang-14 \
        clang-tools-14 \
        git \
        pkg-config \
        ninja-build \
        ccache \
        cmake \
        # Outils de débogage
        gdb \
        valgrind \
        # Dépendances pour X11 et OpenGL
        libx11-dev \
        libxext-dev \
        libxrandr-dev \
        libxinerama-dev \
        libxcursor-dev \
        libxi-dev \
        libxcb1-dev \
        libxcb-randr0-dev \
        libxcb-xfixes0-dev \
        libxcb-xinerama0-dev \
        libxcb-shape0-dev \
        libxcb-keysyms1-dev \
        libxcb-icccm4-dev \
        libxcb-image0-dev \
        libxcb-util0-dev \
        libxcb-xkb-dev \
        x11-xkb-utils \
        xvfb \
        xauth \
        x11-apps \
        mesa-utils \
        libgl1-mesa-dri \
        libgl1-mesa-glx \
        libglu1-mesa-dev \
        # Dépendances pour le développement
        libglew-dev \
        libgif-dev \
        libsdl2-image-dev \
        # Dépendances GTK et GTKmm
        libgtk-3-dev \
        libgtkmm-3.0-dev \
        libglibmm-2.4-dev \
        libcairomm-1.0-dev \
        libpangomm-1.4-dev \
        libatkmm-1.6-dev \
        libgdk-pixbuf2.0-dev \
        libgtkmm-3.0-doc \
        # Dépendances graphiques
        libsdl2-dev \
        # Autres dépendances
        libsigc++-2.0-dev \
        libglm-dev \
        libxfixes-dev \
        xorg-dev \
        # GSettings
        libglib2.0-bin \
        # Nettoyage
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    # Créer des liens symboliques pour les bibliothèques
    && ln -sf /usr/lib/x86_64-linux-gnu/libGLEW.so /usr/lib/libGLEW.so \
    # Créer l'utilisateur non-root
    && groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    # Vérifier les versions des outils
    && echo "=== Versions des outils ===" \
    && clang++-14 --version \
    && cmake --version \
    && pkg-config --modversion gtkmm-3.0 \
    && pkg-config --modversion sdl2 \
    && pkg-config --modversion SDL2_image \
    && pkg-config --modversion glew \
    && echo "========================="

# Configuration de l'utilisateur non-root
ARG USERNAME=devuser
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Création de l'utilisateur et configuration de l'environnement
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && mkdir -p /home/$USERNAME/.ccache /home/$USERNAME/.local/share \
    && chown -R $USER_UID:$USER_GID /home/$USERNAME

# Répertoire de travail
WORKDIR /workspace

# Copie des fichiers du projet
COPY --chown=$USER_UID:$USER_GID . .

# Vérification des fichiers copiés
RUN echo '=== Vérification des fichiers copiés ===' \
    && echo 'Contenu du répertoire /workspace:' \
    && ls -la /workspace/ \
    && echo '\nContenu du répertoire /workspace/src:' \
    && ls -la /workspace/src/ \
    && echo '\n=== Vérification des fichiers CMakeLists.txt ===' \
    && find /workspace -name CMakeLists.txt -exec echo "\n=== Fichier: {} ===" \; -exec head -n 5 {} \; \
    && echo '\n=== Contenu du fichier CMakeLists.txt racine ===' \
    && cat /workspace/CMakeLists.txt \
    && echo '\n========================================='

# Construction de l'application
# Créer le répertoire de travail
WORKDIR /workspace
COPY . .

# Configurer les permissions
RUN chown -R $USER_UID:$USER_GID /workspace \
    && su $USERNAME -c " \
        # Afficher les versions des outils \
        echo '=== Versions des outils ===' \
        && cmake --version \
        && clang++-14 --version \
        && echo '======================' \
        # Nettoyer les anciens fichiers de build \
        && rm -rf /workspace/build \
        # Créer le répertoire de build \
        && mkdir -p /workspace/build \
        && cd /workspace/build \
        # Afficher la structure du projet \
        && echo '=== Structure du projet ===' \
        && find /workspace/src -type f -name '*.cpp' -o -name '*.hpp' -o -name '*.h' | sort \
        && echo '=========================' \
        # Configurer avec CMake \
        && echo '=== Configuration CMake ===' \
        && export CC=/usr/bin/clang-14 \
        && export CXX=/usr/bin/clang++-14 \
        && cmake -G 'Unix Makefiles' \
            -DCMAKE_BUILD_TYPE=RelWithDebInfo \
            -DCMAKE_C_COMPILER=/usr/bin/clang-14 \
            -DCMAKE_CXX_COMPILER=/usr/bin/clang++-14 \
            -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_INSTALL_PREFIX=/usr/local \
            /workspace/src \
        # Construire l'application \
        && echo '=== Construction du projet ===' \
        && make -j$(nproc) VERBOSE=1 \
        # Créer le répertoire bin s'il n'existe pas \
        && mkdir -p /workspace/build/bin \
        # Copier l'exécutable dans le répertoire bin \
        && if [ -f "/workspace/build/OpenYolo" ]; then \
               cp /workspace/build/OpenYolo /workspace/build/bin/; \
           elif [ -f "/workspace/build/src/OpenYolo" ]; then \
               cp /workspace/build/src/OpenYolo /workspace/build/bin/; \
           fi \
        # Vérifier que le binaire existe \
        && echo '=== Contenu de /workspace/build ===' \
        && ls -la /workspace/build/ \
        && echo '=== Contenu de /workspace/build/bin ===' \
        && ls -la /workspace/build/bin/ \
        && echo '=== Recherche du binaire OpenYolo ===' \
        && find /workspace -name OpenYolo -type f -ls \
        && echo '=== Build terminé ==='"

# Étape d'exécution
FROM ubuntu:22.04

# Configuration de l'environnement
ENV DEBIAN_FRONTEND=noninteractive

# Installation des dépendances de base d'abord
RUN echo 'APT::Get::Assume-Yes "true";' > /etc/apt/apt.conf.d/90assumeyes && \
    echo 'APT::Get::force-yes "true";' >> /etc/apt/apt.conf.d/90assumeyes && \
    echo 'APT::Get::Show-Upgraded "true";' >> /etc/apt/apt.conf.d/90assumeyes && \
    # Mise à jour des dépôts
    apt-get update -qq && \
    # Installation des dépendances système essentielles
    apt-get install -y --no-install-recommends \
        # Outils système de base
        ca-certificates \
        curl \
        wget \
        gnupg \
        lsb-release \
        software-properties-common \
        # Outils de développement
        build-essential \
        cmake \
        ninja-build \
        pkg-config \
        clang \
        gdb \
        # Dépendances X11 et graphiques
        xvfb \
        x11-utils \
        x11-xserver-utils \
        xauth \
        dbus-x11 \
        x11vnc \
        xterm \
        x11-apps \
        # Bibliothèques GTK
        libgtk-3-0 \
        libgtkmm-3.0-1v5 \
        libgtk-3-dev \
        libgtkmm-3.0-dev \
        libglibmm-2.4-1v5 \
        libglibmm-2.4-dev \
        libcairomm-1.0-1v5 \
        libcairomm-1.0-dev \
        libpangomm-1.4-1v5 \
        libpangomm-1.4-dev \
        libatkmm-1.6-1v5 \
        libatkmm-1.6-dev \
        libgdk-pixbuf2.0-0 \
        libgdk-pixbuf2.0-dev \
        # Autres dépendances graphiques
        libsdl2-2.0-0 \
        libsdl2-dev \
        libsdl2-image-2.0-0 \
        libsdl2-image-dev \
        libopengl0 \
        libgl1-mesa-glx \
        libgl1-mesa-dev \
        libglx0 \
        libgl1 \
        libglew2.2 \
        libglew-dev \
        # Autres dépendances
        libgif7 \
        libgif-dev \
        libsigc++-2.0-0v5 \
        libsigc++-2.0-dev \
        libglm-dev \
        libxfixes3 \
        libxfixes-dev \
        libx11-6 \
        libx11-dev \
        libglib2.0-bin \
        # Outils utiles
        strace \
        sudo \
        procps \
        lsof \
        less \
        vim-tiny \
        # Thèmes et icônes
        adwaita-icon-theme-full \
        gnome-themes-extra \
        hicolor-icon-theme \
        librsvg2-common && \
    # Nettoyage
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* && \
    # Configuration des liens symboliques
    mkdir -p /usr/lib/gtk-3.0/ && \
    ln -sf /usr/lib/x86_64-linux-gnu/gtk-3.0/modules/ /usr/lib/gtk-3.0/ && \
    ln -sf /usr/lib/x86_64-linux-gnu/gtk-3.0/3.0.0/ /usr/lib/gtk-3.0/ && \
    # Configuration de Xvfb
    mkdir -p /var/run/xvfb && \
    chmod 777 /var/run/xvfb && \
    # Configuration des permissions X11
    mkdir -p /tmp/.X11-unix && \
    chmod 1777 /tmp/.X11-unix && \
    # Message de fin
    echo "Dépendances système installées avec succès"

# Configuration de l'utilisateur non-root
ARG USERNAME=devuser
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Création de l'utilisateur et configuration des permissions
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME \
    && mkdir -p /home/$USERNAME/.local/share \
    && chown -R $USER_UID:$USER_GID /home/$USERNAME

# Configuration des permissions et des dossiers
RUN mkdir -p /workspace/build/bin /workspace/data /tmp/.X11-unix /usr/share/glib-2.0/schemas \
    && chmod 1777 /tmp/.X11-unix \
    && chown -R $USER_UID:$USER_GID /workspace /tmp/.X11-unix /usr/share/glib-2.0/schemas \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Configuration de l'environnement utilisateur
USER $USERNAME
WORKDIR /home/$USERNAME/app

# Copie des fichiers de l'étape de construction
COPY --from=builder /workspace/build/bin/OpenYolo /usr/local/bin/
COPY --from=builder /workspace/build/OpenYolo /workspace/build/bin/OpenYolo
COPY --from=builder /workspace/build/lib/ /usr/local/lib/

# Copie des scripts et configuration
COPY --chown=$USER_UID:$USER_GID start-xvfb.sh /usr/local/bin/start-xvfb.sh
COPY --chown=$USER_UID:$USER_GID start-app.sh /usr/local/bin/start-app
# Copie du schéma GSettings
COPY --chown=$USER_UID:$USER_GID data/org.openyolo.gschema.xml /usr/share/glib-2.0/schemas/
RUN chmod +x /usr/local/bin/start-xvfb.sh /usr/local/bin/start-app \
    && glib-compile-schemas /usr/share/glib-2.0/schemas/

# Configuration des dossiers et permissions
RUN mkdir -p /workspace/data /tmp/.X11-unix /tmp/runtime-$USERNAME \
    && chmod 1777 /tmp/.X11-unix \
    && chown -R $USER_UID:$USER_GID /workspace /tmp/.X11-unix /tmp/runtime-$USERNAME

# Variables d'environnement
# Configuration de l'environnement d'exécution
ENV DISPLAY=:99 \
    XAUTHORITY=/tmp/.Xauthority \
    XDG_RUNTIME_DIR=/tmp/runtime-$USERNAME \
    GTK_THEME=Adwaita \
    GDK_BACKEND=x11 \
    # Configuration du gestionnaire de fenêtres
    OPEN_YOLO_WM_CONFIG=/etc/xdg/open-yolo/wm.ini \
    # Configuration du thème
    GTK_THEME=Adwaita-dark \
    # Configuration du cache
    XDG_CACHE_HOME=/tmp/.cache \
    # Configuration des données utilisateur
    XDG_DATA_HOME=/home/$USERNAME/.local/share \
    XDG_CONFIG_HOME=/home/$USERNAME/.config \
    # Configuration des logs
    XDG_STATE_HOME=/home/$USERNAME/.local/state \
    # Configuration de l'application
    OPEN_YOLO_DEBUG=1 \
    # Configuration du gestionnaire de fenêtres
    CLUTTER_BACKEND=x11 \
    # Configuration du rendu
    LIBGL_ALWAYS_INDIRECT=1 \
    NO_AT_BRIDGE=1 \
    SCREEN_WIDTH=1280 \
    SCREEN_HEIGHT=720 \
    SCREEN_DEPTH=24 \
    CLUTTER_VBLANK=none \
    CLUTTER_PAINT=disable-clipped-redraws,disable-culling \
    GTK_OVERLAY_SCROLLING=0 \
    GTK_CSD=0 \
    G_DEBUG=gc-friendly \
    G_SLICE=always-malloc

# Créer le répertoire de runtime XDG
RUN mkdir -p $XDG_RUNTIME_DIR \
    && chown $USER_UID:$USER_GID $XDG_RUNTIME_DIR \
    && chmod 700 $XDG_RUNTIME_DIR

# Utilisateur non-root
USER $USER_UID:$USER_GID

# Répertoire de travail
WORKDIR /workspace

# Commande par défaut
CMD ["/bin/bash", "-c", "/usr/local/bin/start-xvfb.sh && /usr/local/bin/start-app"]
