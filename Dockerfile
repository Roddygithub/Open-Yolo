FROM ubuntu:22.04

# Installation des dépendances
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    libgtk-3-dev \
    libgtkmm-3.0-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libglew-dev \
    libgif-dev \
    libglibmm-2.4-dev \
    libsigc++-2.0-dev \
    libcairomm-1.0-dev \
    libpangomm-1.4-dev \
    libatkmm-1.6-dev \
    libgdk-pixbuf2.0-dev \
    xvfb \
    xauth \
    libgtest-dev \
    googletest \
    libglm-dev

# Configuration de l'utilisateur non-root
RUN mkdir -p /etc/sudoers.d && \
    useradd -m -s /bin/bash builder && \
    echo 'builder ALL=(ALL) NOPASSWD: ALL' > /etc/sudoers.d/builder && \
    chmod 440 /etc/sudoers.d/builder

# Copie du code source
COPY . /home/builder/Open-Yolo
RUN chown -R builder:builder /home/builder/Open-Yolo

# Configuration de l'environnement
USER builder
WORKDIR /home/builder/Open-Yolo

# Création du répertoire de build et compilation
RUN mkdir -p build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc)

# Script de démarrage
RUN echo '#!/bin/bash\n\
Xvfb :99 -screen 0 1024x768x16 &\
export DISPLAY=:99\
./src/OpenYolo' > /home/builder/start.sh && \
    chmod +x /home/builder/start.sh

CMD ["/home/builder/start.sh"]
