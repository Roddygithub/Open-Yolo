# Compilation d'Open-Yolo

Ce guide explique comment compiler Open-Yolo sur différentes plateformes.

## Prérequis communs

- CMake 3.15 ou supérieur
- Un compilateur C++20 compatible (GCC 10+, Clang 10+, MSVC 2019+)
- Git

## Sous Windows

### Méthode 1 : Utilisation du script batch (recommandé)

1. Ouvrez une invite de commandes en tant qu'administrateur
2. Naviguez vers le répertoire du projet
3. Exécutez la commande suivante :
   ```
   build.bat
   ```
4. Une fois la compilation terminée, l'exécutable se trouvera dans le dossier `build\Release\`

### Méthode 2 : Compilation manuelle

1. Installez les dépendances avec vcpkg :
   ```
   vcpkg install gtkmm sdl2 sdl2-image opengl glew giflib --triplet x64-windows
   ```

2. Créez un dossier de build et configurez CMake :
   ```
   mkdir build
   cd build
   cmake .. -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE=Release -DVCPKG_TARGET_TRIPLET=x64-windows
   ```

3. Compilez le projet :
   ```
   cmake --build . --config Release
   ```

4. Copiez les DLL requises :
   ```
   copy "%VCPKG_ROOT%\installed\x64-windows\bin\*.dll" "Release\"
   ```

## Sous Linux

### Installation des dépendances

#### Ubuntu/Debian :
```bash
sudo apt update
sudo apt install -y git cmake build-essential \
    libgtkmm-3.0-dev libsdl2-dev libsdl2-image-dev \
    libgl1-mesa-dev libglew-dev libgif-dev \
    xorg-dev libx11-xcb-dev libcairo2-dev \
    libcairomm-1.0-dev
```

#### Fedora :
```bash
sudo dnf install -y git cmake gcc-c++ \
    gtkmm30-devel SDL2-devel SDL2_image-devel \
    mesa-libGL-devel glew-devel giflib-devel \
    libX11-devel libXcursor-devel libXrandr-devel \
    cairo-devel cairomm-devel
```

### Compilation

1. Clonez le dépôt :
   ```bash
   git clone https://github.com/yourusername/Open-Yolo.git
   cd Open-Yolo
   ```

2. Créez un dossier de build et configurez CMake :
   ```bash
   mkdir -p build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=~/.local
   ```

3. Compilez le projet :
   ```bash
   make -j$(nproc)
   ```

4. (Optionnel) Installez le programme :
   ```bash
   make install
   ```

## Exécution

### Windows
```
cd build\Release
OpenYolo.exe
```

### Linux
```
./build/OpenYolo
```

Ou si vous avez installé le programme :
```
OpenYolo
```

## Dépannage

### Erreurs de compilation

- **GTKmm non trouvé** : Assurez-vous d'avoir installé toutes les dépendances nécessaires.
- **Erreurs OpenGL** : Vérifiez que vos pilotes graphiques sont à jour.
- **Erreurs de liaison** : Assurez-vous que toutes les bibliothèques sont correctement installées et accessibles.

### Problèmes d'exécution

- **Bibliothèques manquantes** : Assurez-vous que toutes les DLL ou bibliothèques partagées sont dans le PATH ou à côté de l'exécutable.
- **Erreurs de configuration** : Vérifiez le fichier de configuration dans `~/.config/open-yolo/config.ini`.

## Développement

### Configuration de l'environnement de développement

1. Installez Visual Studio Code avec les extensions suivantes :
   - C/C++
   - CMake Tools
   - CMake

2. Ouvrez le dossier du projet dans VS Code

3. Sélectionnez le kit de compilation approprié (Ctrl+Shift+P > "CMake: Select a Kit")

4. Configurez le projet (Ctrl+Shift+P > "CMake: Configure")

5. Compilez le projet (Ctrl+Shift+P > "CMake: Build")
