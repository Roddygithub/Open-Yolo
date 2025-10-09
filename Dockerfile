# Étape de construction (builder)
FROM ubuntu:22.04 as builder

# Définition des variables d'environnement
ARG DEBIAN_FRONTEND=noninteractive
ARG TZ=Europe/Paris

# Configuration de base du système
ENV LANG=C.UTF-8 \
    LC_ALL=C.UTF-8 \
    CC=/usr/bin/clang \
    CXX=/usr/bin/clang++ \
    CMAKE_GENERATOR=Ninja \
    DEBIAN_FRONTEND=${DEBIAN_FRONTEND} \
    TZ=${TZ}

# Installation des dépendances système (regroupées par type pour une meilleure mise en cache)
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
    --mount=type=cache,target=/var/lib/apt,sharing=locked \
    rm -f /etc/apt/apt.conf.d/docker-clean && \
    echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/keep-cache && \
    # Mise à jour des dépôts
    apt-get update && \
    # Installation des dépendances
    apt-get install -y --no-install-recommends \
    # Outils de développement essentiels
    build-essential \
    cmake \
    ninja-build \
    git \
    wget \
    curl \
    pkg-config \
    software-properties-common \
    ca-certificates \
    # Compilateurs et outils d'analyse
    clang-14 \
    clang-tidy-14 \
    clang-format-14 \
    lldb-14 \
    lld-14 \
    # Dépendances graphiques (GTK, X11, etc.)
    libgtk-3-dev \
    libgtkmm-3.0-dev \
    libcairomm-1.0-dev \
    libglibmm-2.4-dev \
    libsigc++-2.0-dev \
    libatkmm-1.6-dev \
    libglib2.0-dev \
    libcairo2-dev \
    libcairomm-1.0-dev \
    libpangomm-1.4-dev \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libx11-xcb-dev \
    libxcb-randr0-dev \
    libxcb-xfixes0-dev \
    libxcb-cursor-dev \
    # Bibliothèques multimédia
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-ttf-dev \
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    libssl-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    # Outils de développement
    gdb \
    valgrind \
    # Dépendances pour X11
    xvfb \
    xauth \
    x11-apps \
    x11-utils \
    x11-xserver-utils \
    # Outils réseau et système
    net-tools \
    iputils-ping \
    procps \
    htop \
    # Nettoyage
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/* \
    libxcb-xfixes0-dev \
    libxcb-cursor-dev \
    # Dépendances SDL2
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-ttf-dev \
    # Autres dépendances
    libcurl4-openssl-dev \
    nlohmann-json3-dev \
    libssl-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    # Outils de débogage
    gdb \
    valgrind \
    # Utilitaires
    xvfb \
    xauth \
    x11-apps \
    x11-utils \
    x11-xserver-utils \
    # Outils réseau
    net-tools \
    iputils-ping \
    # Outils système
    procps \
    htop \
    # Nettoyage
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Installation des outils de formatage
RUN apt-get update && apt-get install -y --no-install-recommends \
    python3 \
    python3-pip \
    python3-venv \
    shfmt \
    && rm -rf /var/lib/apt/lists/*

# Installation de black via pip
RUN python3 -m pip install --no-cache-dir black==22.3.0

# Création d'un utilisateur non-root
RUN useradd -m -u 1000 -s /bin/bash developer && \
    echo 'developer ALL=(ALL) NOPASSWD: ALL' >> /etc/sudoers && \
    mkdir -p /home/developer/OpenYolo && \
    chown -R developer:developer /home/developer

# Copie des scripts système avant de passer à l'utilisateur non-root
COPY scripts/start-xvfb.sh /usr/local/bin/start-xvfb.sh
COPY scripts/start-app.sh /usr/local/bin/start-app.sh
COPY scripts/entrypoint.sh /usr/local/bin/entrypoint.sh

# Définition des permissions pour les scripts
RUN chmod +x /usr/local/bin/start-xvfb.sh && \
    chmod +x /usr/local/bin/start-app.sh && \
    chmod +x /usr/local/bin/entrypoint.sh

# Configuration de l'environnement utilisateur
USER developer
ENV HOME=/home/developer
WORKDIR /home/developer/OpenYolo

# Copie des sources
COPY --chown=developer:developer . /home/developer/OpenYolo

# Configuration des variables d'environnement pour X11
ENV DISPLAY=:99 \
    XAUTHORITY=/tmp/.docker.xauth \
    XDG_RUNTIME_DIR=/tmp/runtime-developer \
    WAYLAND_DISPLAY=wayland-1 \
    QT_QPA_PLATFORM=xcb \
    QT_X11_NO_MITSHM=1 \
    _JAVA_AWT_WM_NONREPARENTING=1 \
    NO_AT_BRIDGE=1

# Création des répertoires nécessaires
RUN mkdir -p /tmp/runtime-developer && \
    chown -R developer:developer /tmp/runtime-developer && \
    touch /tmp/.docker.xauth && \
    chmod 666 /tmp/.docker.xauth

# Définition du répertoire de travail
WORKDIR /home/developer/OpenYolo

# Définition de la commande par défaut
ENTRYPOINT ["/bin/bash"]
CMD ["-c", "echo 'Utilisez une commande spécifique comme format ou test'"]
