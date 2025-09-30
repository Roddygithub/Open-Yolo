# 🚀 Quick Start - Open-Yolo

Guide de démarrage rapide pour compiler et utiliser Open-Yolo.

---

## ⚡ Installation Ultra-Rapide

### Ubuntu/Debian

```bash
# 1. Installer les dépendances
sudo apt-get update && sudo apt-get install -y \
    build-essential cmake ninja-build pkg-config git \
    libgtkmm-3.0-dev libsdl2-dev libsdl2-image-dev \
    libgl1-mesa-dev libglew-dev libgif-dev xorg-dev

# 2. Cloner et compiler
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja

# 3. Lancer
./src/OpenYolo
```

### Fedora

```bash
# 1. Installer les dépendances
sudo dnf install -y \
    gcc-c++ cmake ninja-build pkgconf-pkg-config git \
    gtkmm30-devel SDL2-devel SDL2_image-devel \
    mesa-libGL-devel glew-devel giflib-devel xorg-x11-server-devel

# 2. Cloner et compiler
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja

# 3. Lancer
./src/OpenYolo
```

### Arch Linux

```bash
# 1. Installer les dépendances
sudo pacman -S --needed \
    base-devel cmake ninja git \
    gtkmm3 sdl2 sdl2_image mesa glew giflib

# 2. Option A : Via PKGBUILD (recommandé)
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo
makepkg -si

# 2. Option B : Compilation manuelle
mkdir build && cd build
cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
./src/OpenYolo
```

### NixOS

```bash
# Option A : Installation directe avec Nix Flakes
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo
nix build
nix run .#open-yolo

# Option B : Essayer sans installer
nix run github:Roddygithub/Open-Yolo

# Option C : Shell de développement
nix develop
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/src/OpenYolo
```

---

## 🐳 Avec Docker (toutes distributions)

```bash
# 1. Cloner le projet
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo

# 2. Construire l'image
docker build -t open-yolo:latest .

# 3. Lancer (nécessite X11)
xhost +local:docker
docker run -it --rm \
    -e DISPLAY=$DISPLAY \
    -v /tmp/.X11-unix:/tmp/.X11-unix \
    open-yolo:latest
```

---

## 📦 Installation depuis les Paquets

### DEB (Ubuntu/Debian)

```bash
# Télécharger depuis les releases GitHub
wget https://github.com/Roddygithub/Open-Yolo/releases/download/v1.0.0/open-yolo_1.0.0_amd64.deb

# Installer
sudo dpkg -i open-yolo_1.0.0_amd64.deb
sudo apt-get install -f  # Installer les dépendances manquantes

# Lancer
OpenYolo
```

### RPM (Fedora/RHEL)

```bash
# Télécharger depuis les releases GitHub
wget https://github.com/Roddygithub/Open-Yolo/releases/download/v1.0.0/open-yolo-1.0.0-1.x86_64.rpm

# Installer
sudo dnf install ./open-yolo-1.0.0-1.x86_64.rpm

# Lancer
OpenYolo
```

---

## 🎮 Utilisation Basique

### Première Utilisation

1. **Lancer l'application**
   ```bash
   OpenYolo
   ```

2. **Choisir un curseur**
   - Onglet "Curseur"
   - Cliquer sur "Choisir un fichier"
   - Sélectionner une image PNG ou un GIF

3. **Configurer les effets**
   - Onglet "Effets"
   - Activer/désactiver ombre et lueur
   - Ajuster les paramètres

4. **Définir les raccourcis**
   - Onglet "Raccourcis"
   - Double-cliquer sur un raccourci
   - Appuyer sur la nouvelle combinaison

### Raccourcis Clavier par Défaut

| Raccourci | Action |
|-----------|--------|
| `Ctrl+Alt+C` | Activer/désactiver le curseur personnalisé |
| `Ctrl+Alt+E` | Activer/désactiver les effets |
| `Ctrl+Alt+Plus` | Augmenter la taille du curseur |
| `Ctrl+Alt+Moins` | Diminuer la taille du curseur |

### Configuration Avancée

Le fichier de configuration se trouve dans :
```
~/.config/open-yolo/config.ini
```

Exemple de configuration :
```ini
[Application]
name=Open-Yolo
version=1.0.0
debug=false
log_level=1

[Window]
fullscreen=false
width=1024
height=768

[Cursor]
enabled=true
size=1.5
path=/home/user/cursors/my-cursor.gif
fps=30

[Effects]
shadow_enabled=true
shadow_offset_x=2
shadow_offset_y=2
glow_enabled=false
```

---

## 🧪 Tests et Développement

### Compiler avec les Tests

```bash
mkdir build && cd build
cmake .. -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
ctest --output-on-failure
```

### Script de Test Rapide

```bash
chmod +x scripts/quick-test.sh
./scripts/quick-test.sh
```

### Générer les Paquets

```bash
chmod +x scripts/build-packages.sh
./scripts/build-packages.sh
```

Les paquets seront dans le dossier `packages/`.

---

## 🔧 Dépannage Rapide

### Problème : "Could not find GTKmm"

```bash
# Ubuntu/Debian
sudo apt-get install libgtkmm-3.0-dev

# Fedora
sudo dnf install gtkmm30-devel

# Arch
sudo pacman -S gtkmm3
```

### Problème : "OpenGL not found"

```bash
# Ubuntu/Debian
sudo apt-get install libgl1-mesa-dev libglew-dev

# Fedora
sudo dnf install mesa-libGL-devel glew-devel

# Arch
sudo pacman -S mesa glew
```

### Problème : Le curseur ne s'affiche pas

1. Vérifier que les pilotes graphiques sont installés :
   ```bash
   glxinfo | grep "OpenGL"
   ```

2. Vérifier les logs :
   ```bash
   tail -f ~/.local/share/open-yolo/logs/open-yolo.log
   ```

3. Lancer en mode debug :
   ```bash
   OpenYolo --verbose
   ```

### Problème : Performances faibles

1. Vérifier l'accélération matérielle :
   ```bash
   glxinfo | grep "direct rendering"
   ```
   Devrait afficher : `direct rendering: Yes`

2. Désactiver les effets temporairement :
   - Onglet "Effets" → Tout désactiver

3. Réduire le FPS d'animation :
   - Onglet "Curseur" → FPS : 30 au lieu de 60

---

## 📚 Documentation Complète

Pour plus de détails, consultez :

- **[README.md](README.md)** : Documentation générale
- **[BUILD.md](BUILD.md)** : Guide de compilation détaillé
- **[CHANGELOG.md](CHANGELOG.md)** : Historique des versions
- **[AUDIT_REPORT.md](AUDIT_REPORT.md)** : Rapport technique complet

---

## 🤝 Contribution

Contributions bienvenues ! Voir [CONTRIBUTING.md](CONTRIBUTING.md) pour les guidelines.

---

## 📞 Support

- **Issues GitHub** : https://github.com/Roddygithub/Open-Yolo/issues
- **Discussions** : https://github.com/Roddygithub/Open-Yolo/discussions
- **Wiki** : https://github.com/Roddygithub/Open-Yolo/wiki

---

## 📄 Licence

MIT License - Voir [LICENSE](LICENSE) pour les détails.

---

**Fait avec ❤️ pour la communauté Linux**
