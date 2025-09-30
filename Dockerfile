# syntax=docker/dockerfile:1.5

# =============================================================================
# Étape de construction (builder)
# =============================================================================
FROM --platform=linux/amd64 ubuntu:22.04 as builder

# Arguments de construction
ARG USERNAME=dev
ARG USER_UID=1000
ARG USER_GID=1000
ARG DEBIAN_FRONTEND=noninteractive
ARG TZ=Europe/Paris
ARG BUILD_DATE

# Variables d'environnement pour la construction
ENV \
    # Configuration du compilateur
    CC=/usr/bin/clang-14 \
    CXX=/usr/bin/clang++-14 \
    # Optimisations de compilation
    CFLAGS="-march=native -O2 -pipe -fstack-protector-strong -fno-plt" \
    CXXFLAGS="-march=native -O2 -pipe -fstack-protector-strong -fno-plt" \
    LDFLAGS="-Wl,-O1,--sort-common,--as-needed,-z,relro,-z,now" \
    # Configuration de la locale
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    # Configuration du fuseau horaire
    TZ=${TZ} \
    # Configuration de Python
    PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1 \
    # Configuration de pip
    PIP_NO_CACHE_DIR=off \
    PIP_DISABLE_PIP_VERSION_CHECK=on \
    PIP_DEFAULT_TIMEOUT=100 \
    # Configuration de npm
    NPM_CONFIG_LOGLEVEL=warn \
    NODE_OPTIONS=--max_old_space_size=4096

# Installation des dépendances système
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    set -eux; \
    # Configurer les dépôts
    echo 'APT::Install-Recommends "false";' > /etc/apt/apt.conf.d/00recommends; \
    echo 'APT::Install-Suggests "false";' >> /etc/apt/apt.conf.d/00recommends; \
    echo 'APT::Get::Assume-Yes "true";' > /etc/apt/apt.conf.d/90assumeyes; \
    echo 'APT::Get::force-yes "true";' >> /etc/apt/apt.conf.d/90assumeyes; \
    # Mettre à jour les paquets
    apt-get update -qq; \
    # Installer les dépendances système
    apt-get install -y --no-install-recommends \
        # Outils système de base
        ca-certificates \
        curl \
        wget \
        gnupg \
        lsb-release \
        software-properties-common \
        tzdata \
        locales \
        sudo \
        python3 \
        python3-pip \
        python3-setuptools \
        python3-wheel \
        python3-dev \
        python3-venv \
        # Outils de développement essentiels
        build-essential \
        clang-14 \
        clang-tools-14 \
        clang-format-14 \
        clang-tidy-14 \
        lld-14 \
        lldb-14 \
        llvm-14 \
        gdb \
        git \
        git-lfs \
        pkg-config \
        ninja-build \
        ccache \
        cmake \
        cmake-extras \
        # Outils de débogage et d'analyse
        valgrind \
        strace \
        ltrace \
        bear \
        cppcheck \
        # Dépendances X11 et graphiques
        libx11-dev \
        libxext-dev \
        libxrandr-dev \
        libxinerama-dev \
        libxcursor-dev \
        libxi-dev \
        libgl1-mesa-dev \
        libglu1-mesa-dev \
        mesa-common-dev \
        libegl1-mesa-dev \
        libgles2-mesa-dev \
        libgbm-dev \
        libdrm-dev \
        libwayland-dev \
        wayland-protocols \
        # Dépendances GTK/GTKmm
        libgtk-3-dev \
        libgtkmm-3.0-dev \
        libglib2.0-dev \
        libglibmm-2.4-dev \
        libcairo2-dev \
        libcairomm-1.0-dev \
        libpango1.0-dev \
        libpangomm-1.4-dev \
        libatk1.0-dev \
        libatkmm-1.6-dev \
        libgdk-pixbuf2.0-dev \
        libgdk-pixbuf2.0-bin \
        # Dépendances SDL2
        libsdl2-dev \
        libsdl2-image-dev \
        libsdl2-mixer-dev \
        libsdl2-ttf-dev \
        libsdl2-gfx-dev \
        # Dépendances OpenGL
        libglew-dev \
        libglfw3-dev \
        # Dépendances XCB
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
        # Outils X11
        x11-apps \
        x11-utils \
        x11-xserver-utils \
        x11-xkb-utils \
        xauth \
        xvfb \
        # Dépendances multimédia
        libasound2-dev \
        libpulse-dev \
        # Dépendances réseau
        libssl-dev \
        libcurl4-openssl-dev \
        # Dépendances pour le traitement d'images
        libgif-dev \
        libjpeg-dev \
        libpng-dev \
        libtiff-dev \
        libwebp-dev \
        libopenexr-dev \
        libopenjp2-7-dev \
        # Bibliothèques C++ supplémentaires
        libboost-all-dev \
        libeigen3-dev \
        libgtest-dev \
        libgmock-dev \
        libbenchmark-dev \
        nlohmann-json3-dev \
        rapidjson-dev \
        libyaml-cpp-dev \
        libspdlog-dev \
        libfmt-dev \
        libsigc++-2.0-dev \
        libglm-dev \
        libxfixes-dev \
        # Outils système
        htop \
        iotop \
        iftop \
        net-tools \
        iputils-ping \
        dnsutils \
        procps \
        lsof \
        file \
        # Outils de développement
        doxygen \
        graphviz \
        python3-dev \
        python3-pip \
        python3-venv \
        # Outils de documentation
        plantuml \
        texlive-latex-base \
        texlive-latex-extra \
        texlive-fonts-recommended \
        texlive-fonts-extra \
        lmodern \
        # Outils de productivité
        zsh \
        zsh-syntax-highlighting \
        zsh-autosuggestions \
        bat \
        ripgrep \
        fd-find \
        fzf \
        tmux \
        # Nettoyage final
        && apt-get clean \
        && rm -rf /var/lib/apt/lists/* \
        && rm -rf /tmp/* /var/tmp/* \
        && find /usr/local -depth \
            \( \
                -type d -a \
                \( -name test -o -name tests -o -name '*.pyc' \) \
            \) -exec rm -rf '{}' + \
        && find /usr/lib -name "__pycache__" -exec rm -rf {} + \
        && find /usr/lib -name "*.pyc" -delete \
        && rm -rf /var/cache/apt/archives/*.deb \
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
ARG USER_GID=1000

# Création de l'utilisateur et configuration de l'environnement
RUN groupadd --gid $USER_GID $USERNAME || true \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME || true \
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
    && echo '=== Versions des outils ===' \
    && cmake --version \
    && clang++-14 --version \
    && echo '======================' \
    && rm -rf /workspace/build \
    && mkdir -p /workspace/build \
    && cd /workspace/build \
    && echo '=== Structure du projet ===' \
    && find /workspace/src -type f -name '*.cpp' -o -name '*.hpp' -o -name '*.h' | sort \
    && echo '=========================' \
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
        /workspace \
    && echo '=== Construction du projet ===' \
    && make -j$(nproc) VERBOSE=1 \
    && mkdir -p /workspace/build/bin \
    && if [ -f "/workspace/build/OpenYolo" ]; then \
           cp /workspace/build/OpenYolo /workspace/build/bin/; \
       elif [ -f "/workspace/build/src/OpenYolo" ]; then \
           cp /workspace/build/src/OpenYolo /workspace/build/bin/; \
       fi \
    && echo '=== Contenu de /workspace/build ===' \
    && ls -la /workspace/build/ \
    && echo '=== Contenu de /workspace/build/bin ===' \
    && ls -la /workspace/build/bin/ \
    && echo '=== Recherche du binaire OpenYolo ===' \
    && find /workspace -name OpenYolo -type f -ls \
    && echo '=== Build terminé ==='

# Étape d'exécution
FROM ubuntu:22.04

# Configuration de l'utilisateur
ARG USERNAME=devuser
ARG USER_UID=1000
ARG USER_GID=1000

# Configuration de l'environnement
ENV DEBIAN_FRONTEND=noninteractive

# Création de l'utilisateur non-root
RUN groupadd --gid $USER_GID $USERNAME || true \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME || true \
    && mkdir -p /home/$USERNAME/.ccache /home/$USERNAME/.local/share \
    && chown -R $USER_UID:$USER_GID /home/$USERNAME

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
RUN groupadd --gid $USER_GID $USERNAME || true \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME || true \
    && mkdir -p /home/$USERNAME/.local/share \
    && chown -R $USER_UID:$USER_GID /home/$USERNAME \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Configuration des permissions et des dossiers
RUN mkdir -p /workspace/build/bin /workspace/data /tmp/.X11-unix /usr/share/glib-2.0/schemas \
    && chmod 1777 /tmp/.X11-unix \
    && chown -R $USER_UID:$USER_GID /workspace /tmp/.X11-unix /usr/share/glib-2.0/schemas \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Configuration de l'environnement utilisateur
USER $USERNAME
WORKDIR /home/$USERNAME/app

# Création des répertoires nécessaires
RUN mkdir -p /home/$USERNAME/.local/share/applications \
    && mkdir -p /home/$USERNAME/.config/autostart \
    && mkdir -p /home/$USERNAME/.local/share/icons/hicolor/256x256/apps

# Copie des fichiers de l'étape de construction
COPY --from=builder --chown=$USER_UID:$USER_GID /workspace/build/bin/OpenYolo /usr/local/bin/
COPY --from=builder --chown=$USER_UID:$USER_GID /workspace/build/OpenYolo /workspace/build/bin/OpenYolo
COPY --from=builder --chown=$USER_UID:$USER_GID /workspace/build/lib/ /usr/local/lib/

# Copie des scripts et configuration
COPY --chown=$USER_UID:$USER_GID scripts/start-xvfb.sh /usr/local/bin/start-xvfb.sh
COPY --chown=$USER_UID:$USER_GID scripts/start-app.sh /usr/local/bin/start-app
COPY --chown=$USER_UID:$USER_GID scripts/healthcheck.sh /usr/local/bin/healthcheck

# Création des répertoires nécessaires
RUN mkdir -p /home/$USERNAME/.local/share/applications/ \
    && mkdir -p /home/$USERNAME/.config/autostart/ \
    && mkdir -p /usr/share/glib-2.0/schemas/

# Copie des fichiers de configuration
COPY --chown=$USER_UID:$USER_GID data/org.openyolo.gschema.xml /usr/share/glib-2.0/schemas/
COPY --chown=$USER_UID:$USER_GID resources/OpenYolo.desktop /home/$USERNAME/.local/share/applications/
COPY --chown=$USER_UID:$USER_GID resources/OpenYolo.desktop /home/$USERNAME/.config/autostart/

# Installation des dépendances pour les icônes et le bureau
RUN --mount=type=cache,target=/root/.cache/pip \
    apt-get update -qq && apt-get install -y --no-install-recommends \
    python3-pip \
    python3-setuptools \
    python3-wheel \
    gtk-update-icon-cache \
    hicolor-icon-theme \
    && rm -rf /var/lib/apt/lists/*

# Copie des ressources avant l'installation des icônes
COPY --chown=$USER_UID:$USER_GID resources/ /tmp/resources/
COPY --chown=$USER_UID:$USER_GID scripts/ /tmp/scripts/

# Installation de Pillow et génération des icônes
RUN python3 -m pip install --no-cache-dir --upgrade pip \
    && python3 -m pip install --no-cache-dir Pillow \
    && mkdir -p /tmp/resources/icons/hicolor \
    && cd /tmp \
    && python3 scripts/generate_icons.py \
    && chmod +x scripts/setup_icons.sh \
    && ./scripts/setup_icons.sh \
    && cp -r /tmp/resources/icons/hicolor/* /usr/share/icons/hicolor/ \
    && gtk-update-icon-cache -f -t /usr/share/icons/hicolor/ \
    && rm -rf /tmp/resources /tmp/scripts

# Configuration des permissions et compilation des schémas
RUN chmod +x /usr/local/bin/start-xvfb.sh /usr/local/bin/start-app /usr/local/bin/healthcheck \
    && glib-compile-schemas /usr/share/glib-2.0/schemas/

# Configuration des dossiers et permissions
RUN mkdir -p /workspace/data /tmp/.X11-unix /tmp/runtime-$USERNAME \
    && chmod 1777 /tmp/.X11-unix \
    && chown -R $USER_UID:$USER_GID /workspace /tmp/.X11-unix /tmp/runtime-$USERNAME \
    && chmod +x /workspace/scripts/entrypoint.sh /workspace/start-app.sh

# Variables d'environnement
ENV \
    # Configuration X11 et affichage
    DISPLAY=:99 \
    XAUTHORITY=/tmp/.Xauthority \
    XDG_RUNTIME_DIR=/tmp/runtime-$USERNAME \
    # Configuration GTK et thème
    GTK_THEME=Adwaita-dark \
    GTK_DEBUG=interactive \
    GTK_OVERLAY_SCROLLING=0 \
    GTK_CSD=0 \
    # Configuration du backend graphique
    GDK_BACKEND=x11 \
    CLUTTER_BACKEND=x11 \
    # Configuration du rendu OpenGL
    LIBGL_ALWAYS_INDIRECT=1 \
    NO_AT_BRIDGE=1 \
    # Configuration de l'écran virtuel
    SCREEN_WIDTH=1280 \
    SCREEN_HEIGHT=720 \
    SCREEN_DEPTH=24 \
    # Configuration des performances
    CLUTTER_VBLANK=none \
    CLUTTER_PAINT=disable-clipped-redraws,disable-culling \
    # Configuration de GLib
    G_DEBUG=gc-friendly \
    G_SLICE=always-malloc \
    # Configuration des dossiers utilisateur
    XDG_CACHE_HOME=/tmp/.cache \
    XDG_DATA_HOME=/home/$USERNAME/.local/share \
    XDG_CONFIG_HOME=/home/$USERNAME/.config \
    XDG_STATE_HOME=/home/$USERNAME/.local/state \
    # Configuration de l'application
    OPEN_YOLO_DEBUG=1 \
    OPEN_YOLO_WM_CONFIG=/etc/xdg/open-yolo/wm.ini \
    # Configuration des chemins
    PATH="/usr/local/bin:${PATH}" \
    # Configuration du terminal
    TERM=xterm-256color \
    # Configuration de la locale
    LANG=C.UTF-8 \
    LC_ALL=C.UTF-8

# Définition du point de montage pour les données persistantes
VOLUME ["/home/$USERNAME/.config/OpenYolo", "/home/$USERNAME/.local/share/OpenYolo"]

# Exposition des ports (si nécessaire)
EXPOSE 5900 6080

# Vérification de santé
HEALTHCHECK --interval=30s --timeout=10s --start-period=5s --retries=3 \
    CMD ["/usr/local/bin/healthcheck"]

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
