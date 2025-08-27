# Open-Yolo

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://github.com/yourusername/Open-Yolo/actions/workflows/build.yml/badge.svg)](https://github.com/yourusername/Open-Yolo/actions)
[![GitHub release](https://img.shields.io/github/release/yourusername/Open-Yolo.svg)](https://github.com/yourusername/Open-Yolo/releases)

Un gestionnaire de curseurs personnalisés natif pour Linux, compatible avec X11 et Wayland, inspiré de YoloMouse.

## 🚀 Fonctionnalités

### Dernières mises à jour (v0.2.0)
- Migration de GTK4 vers GTK3 pour une meilleure compatibilité
- Amélioration de la gestion des erreurs et de la stabilité
- Optimisation des performances de rendu
- Correction des problèmes de gestion des fenêtres
- Mise à jour de la documentation

- 🖱️ Curseurs personnalisés animés (GIF ou images)
- 🖥️ Support multi-écran avancé
- 🎨 Compatible avec les écrans haute densité (HiDPI)
- ⚡ Rendu GPU optimisé avec OpenGL
- ⌨️ Raccourcis clavier personnalisables
- 🎛️ Interface graphique de configuration intuitive
- 🔄 Mise à jour en temps réel des paramètres
- 📦 Installation simple via des paquets natifs

## 📋 Prérequis

- **Système d'exploitation** : Linux (testé sur Ubuntu 20.04+ et Fedora 33+)
- **CMake** : 3.15 ou supérieur
- **Compilateur C++** : g++ 9+ ou clang 10+
- **Bibliothèques système** :
  - GTKmm 3.24+ (gtkmm-3.0)
  - SDL2 2.0.10+ (sdl2)
  - SDL2_image 2.0.5+ (sdl2_image)
  - OpenGL 3.3+
  - GLEW 2.1.0+
  - giflib 5.2.1+
  - xorg-dev (pour le support X11)
  - libx11-xcb-dev (pour la gestion des fenêtres)

## 🛠 Installation

### Depuis les sources

1. **Installer les dépendances** :

   **Sur Ubuntu/Debian** :
   ```bash
   sudo apt update
   sudo apt install -y git cmake build-essential \
       libgtkmm-3.0-dev libsdl2-dev libsdl2-image-dev \
       libgl1-mesa-dev libglew-dev libgif-dev \
       xorg-dev libx11-xcb-dev
   ```

   **Sur Fedora** :
   ```bash
   sudo dnf install -y git cmake gcc-c++ \
       gtkmm30-devel SDL2-devel SDL2_image-devel \
       mesa-libGL-devel glew-devel giflib-devel \
       libX11-devel libXcursor-devel libXrandr-devel
   ```

2. **Cloner le dépôt** :
   ```bash
   git clone https://github.com/yourusername/Open-Yolo.git
   cd Open-Yolo
   ```

3. **Compiler et installer** :
   ```bash
   ./build.sh
   ```

   Ou manuellement :
   ```bash
   mkdir -p build && cd build
   cmake -DCMAKE_INSTALL_PREFIX=~/.local ..
   make -j$(nproc)
   make install
   ```

### Paquets pré-compilés

Des paquets pour différentes distributions sont disponibles dans la section [Releases](https://github.com/yourusername/Open-Yolo/releases).

## 🚀 Utilisation

### Lancement

```bash
OpenYolo
```

### Interface graphique

L'interface graphique permet de configurer :

- **Onglet Curseur** :
  - Sélectionner une image ou un GIF comme curseur
  - Ajuster la vitesse d'animation (FPS)
  - Modifier l'échelle du curseur
  - Prévisualisation en temps réel

- **Onglet Raccourcis** :
  - Personnaliser les raccourcis clavier
  - Activer/désactiver des fonctionnalités

- **Onglet Affichage** :
  - Gérer les écrans multiples
  - Activer le suivi de la souris
  - Ajuster les paramètres de rendu

### Ligne de commande

```bash
# Afficher l'aide
OpenYolo --help

# Spécifier un fichier de configuration
OpenYolo --config ~/.config/open-yolo/config.ini

# Activer le mode verbeux
OpenYolo --verbose
```

## 🛠 Développement

### Structure du projet

```
Open-Yolo/
├── data/           # Fichiers de données (icônes, schémas, etc.)
├── docs/           # Documentation
├── include/        # En-têtes
│   ├── cursormanager/  # Gestion des curseurs
│   ├── displaymanager/ # Gestion de l'affichage
│   ├── input/          # Gestion des entrées
│   └── gui/            # Interface graphique
├── resources/      # Ressources (images, shaders, etc.)
├── src/            # Code source
│   ├── cursormanager/
│   ├── displaymanager/
│   ├── input/
│   └── gui/
└── tests/          # Tests unitaires
```

### Compilation pour le développement

```bash
mkdir -p build/debug
cd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ../..
make -j$(nproc)
```

### Exécution des tests

```bash
cd build/debug
ctest --output-on-failure
```

## 🤝 Contribution

Les contributions sont les bienvenues ! Voici comment contribuer :

1. Forkez le projet
2. Créez une branche pour votre fonctionnalité (`git checkout -b feature/AmazingFeature`)
3. Committez vos changements (`git commit -m 'Add some AmazingFeature'`)
4. Poussez vers la branche (`git push origin feature/AmazingFeature`)
5. Ouvrez une Pull Request

## 📄 Licence

Distribué sous licence MIT. Voir le fichier `LICENSE` pour plus d'informations.

## 🙏 Remerciements

- [YoloMouse](https://pandateemo.github.io/YoloMouse/) pour l'inspiration
- Tous les contributeurs qui ont aidé à améliorer ce projet

---

<p align="center">
  Fait avec ❤️ pour la communauté Linux
</p>
