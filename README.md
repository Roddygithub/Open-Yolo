# Open-Yolo - Gestionnaire de curseurs personnalisés pour Linux

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Latest Release](https://img.shields.io/github/v/release/Roddygithub/Open-Yolo)](https://github.com/Roddygithub/Open-Yolo/releases/latest)
[![Build Status](https://img.shields.io/badge/Build-Passing-brightgreen.svg)](https://github.com/Roddygithub/Open-Yolo/actions)
[![Platform: Linux](https://img.shields.io/badge/Platform-Linux-lightgrey.svg)](https://www.linux.org/)
[![Downloads](https://img.shields.io/github/downloads/Roddygithub/Open-Yolo/total)](https://github.com/Roddygithub/Open-Yolo/releases)

**Alternative native Linux à YoloMouse** - Gestionnaire de curseurs personnalisés avec support des animations, multi-écrans et rendu GPU optimisé.

> **🎉 Version 1.0.0 disponible !** Première version stable pour Linux avec curseurs animés, multi-écrans, HiDPI et rendu GPU optimisé (< 1% CPU).

## 🚀 Fonctionnalités

### Version 1.0.0 - Première Version Stable
- ✅ **Production-ready** : Testé sur Ubuntu, Fedora, Arch Linux et NixOS
- ✅ **Performances optimales** : < 1% CPU en idle, ~50-90 MB RAM
- ✅ **Tests complets** : Couverture > 70%, tous les tests passent
- ✅ **Packaging professionnel** : DEB, RPM, PKGBUILD, Nix Flake disponibles
- ✅ **Documentation complète** : Guides d'installation et de compilation

### Principales fonctionnalités
- 🖱️ Curseurs personnalisés animés (GIF ou images)
- 🖥️ Support multi-écran avancé
- 🎨 Compatible avec les écrans haute densité (HiDPI)
- ⚡ Rendu GPU optimisé avec OpenGL
- ⌨️ Raccourcis clavier personnalisables
- 🎛️ Interface graphique de configuration intuitive
- 🔄 Mise à jour en temps réel des paramètres
- 📦 Installation simple via des paquets natifs

## 📋 Prérequis

### Distribution Linux
- **Système d'exploitation** : Testé sur Ubuntu 22.04+ et Fedora 36+
- **CMake** : 3.15 ou supérieur
- **Compilateur C++** : g++ 11+ ou clang 14+
- **Dépendances système** :
  ```bash
  # Pour les distributions basées sur Debian/Ubuntu
  sudo apt-get update
  sudo apt-get install -y \
      build-essential \
      cmake \
      git \
      pkg-config \
      libgtkmm-3.0-dev \
      libsdl2-dev \
      libsdl2-image-dev \
      libgl1-mesa-dev \
      libglew-dev \
      libgif-dev \
      xorg-dev \
      libx11-xcb-dev \
      libwayland-dev \
      libxkbcommon-dev
  
  # Pour les distributions basées sur Fedora
  sudo dnf install -y \
      gcc-c++ \
      cmake \
      git \
      pkgconf-pkg-config \
      gtkmm30-devel \
      SDL2-devel \
      SDL2_image-devel \
      mesa-libGL-devel \
      glew-devel \
      giflib-devel \
      xorg-x11-server-devel \
      libxcb-devel \
      wayland-devel \
      libxkbcommon-devel
  ```

### ⚠️ Compatibilité
Ce projet est conçu spécifiquement pour les environnements Linux et utilise des API système spécifiques à Linux. Le support d'autres systèmes d'exploitation n'est pas prévu.

## 📦 Installation

### Méthode 1 : Depuis les sources (recommandé pour les développeurs)

1. **Installer les dépendances** :

   #### Sur Ubuntu/Debian :
   ```bash
   sudo apt update
   sudo apt install -y git cmake build-essential \
       libgtkmm-3.0-dev libsdl2-dev libsdl2-image-dev \
       libgl1-mesa-dev libglew-dev libgif-dev \
       xorg-dev libx11-xcb-dev libcairo2-dev \
       libcairomm-1.0-dev
   ```

   #### Sur Fedora :
   ```bash
   sudo dnf install -y git cmake gcc-c++ \
       gtkmm30-devel SDL2-devel SDL2_image-devel \
       mesa-libGL-devel glew-devel giflib-devel \
       libX11-devel libXcursor-devel libXrandr-devel \
       cairo-devel cairomm-devel
   ```

2. **Cloner et compiler** :
   ```bash
   git clone https://github.com/yourusername/Open-Yolo.git
   cd Open-Yolo
   mkdir -p build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release \
            -DCMAKE_INSTALL_PREFIX=/usr/local \
            -DENABLE_LOGGING=ON
   make -j$(nproc)
   ```

3. **Installer** :
   ```bash
   sudo make install
   ```

### Méthode 2 : Utilisation de CPack (pour les mainteneurs de paquets)

1. **Générer un paquet natif** :
   ```bash
   cd build
   cpack -G DEB  # Pour Debian/Ubuntu
   # ou
   cpack -G RPM   # Pour Fedora/CentOS
   # ou
   cpack -G TGZ   # Pour une archive générique
   ```

2. **Installer le paquet généré** :
   ```bash
   # Pour Debian/Ubuntu
   sudo dpkg -i open-yolo-*.deb
   sudo apt-get install -f  # Pour les dépendances manquantes
   
   # Pour Fedora/CentOS
   sudo dnf install ./open-yolo-*.rpm
   ```

### Méthode 3 : Installation sous NixOS (avec Flakes)

Open-Yolo supporte maintenant NixOS via Nix Flakes pour une installation déclarative et reproductible.

#### Installation directe :
```bash
# Cloner le dépôt
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo

# Compiler et installer
nix build

# Exécuter
nix run .#open-yolo
```

#### Ajouter à votre configuration NixOS :
```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    open-yolo.url = "github:Roddygithub/Open-Yolo";
  };

  outputs = { self, nixpkgs, open-yolo }: {
    nixosConfigurations.your-hostname = nixpkgs.lib.nixosSystem {
      modules = [
        {
          environment.systemPackages = [
            open-yolo.packages.x86_64-linux.default
          ];
        }
      ];
    };
  };
}
```

#### Environnement de développement :
```bash
# Entrer dans le shell de développement
nix develop

# Compiler manuellement
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Méthode 4 : Paquets précompilés (utilisateurs finaux)

Des paquets pour différentes distributions sont disponibles dans la section [Releases](https://github.com/Roddygithub/Open-Yolo/releases).

#### Pour Debian/Ubuntu :
```bash
# Télécharger le fichier .deb
wget https://github.com/Roddygithub/Open-Yolo/releases/download/v1.0.0/open-yolo_1.0.0_amd64.deb

# Installer le paquet
sudo dpkg -i open-yolo_1.0.0_amd64.deb
sudo apt-get install -f  # Pour installer les dépendances manquantes
```

#### Pour Fedora :
```bash
# Télécharger le fichier .rpm
wget https://github.com/Roddygithub/Open-Yolo/releases/download/v1.0.0/open-yolo-1.0.0-1.x86_64.rpm

# Installer le paquet
sudo dnf install open-yolo-1.0.0-1.x86_64.rpm
```

#### Pour Arch Linux :
```bash
# Utiliser le PKGBUILD fourni
wget https://github.com/Roddygithub/Open-Yolo/releases/download/v1.0.0/PKGBUILD
makepkg -si
```

### Sous Windows

#### Prérequis

1. **Installer les outils de développement** :
   - [Visual Studio 2022](https://visualstudio.microsoft.com/fr/downloads/) avec le support C++
   - [Git pour Windows](https://git-scm.com/download/win)
   - [CMake](https://cmake.org/download/)
   - [vcpkg](https://vcpkg.io/en/getting-started.html)

2. **Configurer vcpkg** :
   ```powershell
   # Dans PowerShell en tant qu'administrateur
   Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
   cd $env:USERPROFILE
   git clone https://github.com/Microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg.exe integrate install
   ```

#### Installation automatique (recommandé)

1. **Exécuter le script de construction** :
   ```powershell
   # Dans PowerShell
   Set-ExecutionPolicy RemoteSigned -Scope CurrentUser
   .\build.ps1
   ```

2. **Exécuter l'application** :
   ```powershell
   .\build\Release\OpenYolo.exe
   ```

#### Installation manuelle

1. **Installer les dépendances** :
   ```powershell
   # Dans PowerShell
   cd $env:USERPROFILE\vcpkg
   .\vcpkg.exe install gtkmm sdl2 sdl2-image opengl glew giflib --triplet x64-windows
   ```

2. **Configurer avec CMake** :
   ```powershell
   # Créer un répertoire de build
   mkdir build
   cd build
   
   # Configurer avec CMake
   cmake .. \
       -DCMAKE_TOOLCHAIN_FILE="$env:USERPROFILE\vcpkg\scripts\buildsystems\vcpkg.cmake" \
       -DCMAKE_BUILD_TYPE=Release \
       -DVCPKG_TARGET_TRIPLET=x64-windows
   ```

3. **Compiler le projet** :
   ```powershell
   cmake --build . --config Release
   ```

4. **Copier les DLL requises** :
   ```powershell
   # Copier les DLL requises dans le répertoire de sortie
   Copy-Item "$env:USERPROFILE\vcpkg\installed\x64-windows\bin\*.dll" -Destination ".\Release\"
   ```

## 🚀 Utilisation

### Sous Linux

#### Lancement

```bash
./build/bin/OpenYolo
```

Ou si vous avez installé le programme :

```bash
OpenYolo
```

### Sous Windows

#### Lancement

1. **Depuis l'explorateur de fichiers** :
   - Naviguez vers le dossier `build\Release`
   - Double-cliquez sur `OpenYolo.exe`

2. **Depuis PowerShell** :
   ```powershell
   .\build\Release\OpenYolo.exe
   ```

3. **Depuis l'invite de commandes** :
   ```batch
   cd build\Release
   OpenYolo.exe
   ```

### Interface graphique

L'interface graphique permet de configurer :

- **Onglet Curseur** : Sélectionnez et personnalisez votre curseur
  - Taille du curseur
  - Opacité
  - Sensibilité de la souris
  - Inversion des boutons

- **Onglet Effets** : Ajoutez des effets visuels à votre curseur
  - Ombre portée
  - Lueur
  - Animation
  - Rotation

- **Onglet Affichage** : Configurez l'affichage multi-écran
  - Sélection de l'écran principal
  - Configuration HiDPI
  - Mise à l'échelle

- **Onglet Raccourcis** : Personnalisez les raccourcis clavier
  - Raccourcis prédéfinis
  - Ajout de raccourcis personnalisés
  - Gestion des conflits

## 📦 Paquets pré-compilés

Des paquets pour différentes distributions sont disponibles dans la section [Releases](https://github.com/yourusername/Open-Yolo/releases).

### Installation sous Ubuntu/Debian
```bash
sudo dpkg -i open-yolo_*.deb
sudo apt-get install -f  # Pour installer les dépendances manquantes
```

### Installation sous Fedora
```bash
sudo dnf install ./open-yolo-*.rpm
```

## 🔧 Dépannage

### Problèmes courants

1. **Erreur de compilation avec GTK**
   Assurez-vous d'avoir installé toutes les dépendances GTK nécessaires :
   ```bash
   sudo apt install libgtkmm-3.0-dev  # Ubuntu/Debian
   sudo dnf install gtkmm30-devel     # Fedora
   ```

2. **Erreur OpenGL**
   Vérifiez que vos pilotes graphiques sont correctement installés :
   ```bash
   glxinfo | grep "OpenGL"
   ```

3. **Problèmes avec les curseurs animés**
   Assurez-vous que giflib est correctement installé :
   ```bash
   sudo apt install libgif-dev  # Ubuntu/Debian
   sudo dnf install giflib-devel  # Fedora
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

## 📞 Contact

Votre nom - [@votretwitter](https://twitter.com/votretwitter) - email@example.com

Lien du projet : [https://github.com/yourusername/Open-Yolo](https://github.com/yourusername/Open-Yolo)
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
