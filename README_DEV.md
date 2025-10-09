# Guide de développement OpenYolo

Ce document fournit des instructions pour configurer l'environnement de développement, construire le projet et contribuer au code source d'OpenYolo.

## Prérequis

### Système d'exploitation
- Linux (Ubuntu 22.04 recommandé)
- Windows 10/11 avec WSL2 (pour le développement sous Windows)
- macOS 12+ (avec Xcode 14+)

### Outils de base
- Git 2.30+
- CMake 3.20+
- Compilateur C++20 (GCC 11+/Clang 14+/MSVC 2022)
- Python 3.11+ (pour les scripts de développement)
- Docker 24.0+ et Docker Compose 2.20+

### Dépendances système (pour développement natif)
```bash
# Sur Ubuntu/Debian
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    ninja-build \
    clang \
    lld \
    clang-tidy \
    clang-format \
    gdb \
    valgrind \
    libgtk-3-dev \
    libgtkmm-3.0-dev \
    libsdl2-dev \
    libsdl2-image-dev \
    libsdl2-ttf-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libx11-dev \
    xorg-dev
```

## Configuration de l'environnement de développement

### 1. Clonage du dépôt

```bash
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo
git checkout develop
```

### 2. Configuration de l'environnement Docker

Le projet utilise Docker pour assurer une configuration cohérente entre les environnements de développement. Un fichier `Dockerfile` et un fichier `docker-compose.yml` sont fournis.

#### Construction de l'image Docker

```bash
docker-compose build
```

#### Démarrage du conteneur de développement

```bash
# Pour un shell interactif
docker-compose run --rm openyolo

# Pour exécuter les tests
docker-compose run --rm test

# Pour formater le code
docker-compose run --rm format
```

## Construction du projet

## Gestion des dépendances

### Dépendances principales
- **GTK/GTKmm** : Interface utilisateur graphique
- **SDL2** : Gestion du rendu et des entrées
- **OpenGL** : Rendu accéléré
- **X11/Wayland** : Support des systèmes d'affichage
- **CURL** : Téléchargement de contenu
- **nlohmann-json** : Manipulation de JSON

### Mise à jour des dépendances
Les dépendances système sont gérées via les gestionnaires de paquets du système. Pour les mettre à jour :

```bash
# Sur Ubuntu/Debian
sudo apt update
sudo apt upgrade

# Dans le conteneur Docker
apt-get update
apt-get upgrade -y
```

## Construction du projet

### Construction avec CMake (sans Docker)

```bash
# Création du répertoire de build
mkdir -p build && cd build

# Configuration avec CMake
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..

# Construction
cmake --build .

# Exécution des tests
ctest --output-on-failure
```

### Construction avec CMake (avec Docker)

```bash
# Construction et exécution dans un conteneur
docker-compose run --rm openyolo /bin/bash -c "mkdir -p build && cd build && cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug .. && cmake --build ."
```

## Formatage du code

Le projet utilise `clang-format` pour le formatage du code C++ et `black` pour le code Python. Ces outils sont configurés via des fichiers de configuration dans la racine du projet.

### Formatage automatique

```bash
# Formater tout le code (C++ et Python)
./scripts/format.sh

# Formater uniquement le code C++
find src include tests -name '*.h' -o -name '*.cpp' | xargs clang-format -i

# Formater uniquement le code Python
black .
```

### Vérification du formatage

```bash
# Vérifier le formatage C++
find src include tests -name '*.h' -o -name '*.cpp' | xargs clang-format --dry-run --Werror

# Vérifier le formatage Python
black --check .
```

## Tests

### Exécution des tests unitaires

```bash
# Exécuter tous les tests
cd build && ctest --output-on-failure

# Exécuter un test spécifique
cd build && ctest -R "test_name" --output-on-failure

# Exécuter les tests avec couverture de code (nécessite l'option ENABLE_COVERAGE=ON)
cd build && ctest -T Test
```

### Débogage des tests

```bash
# Construire en mode Debug
mkdir -p build-debug && cd build-debug
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .

# Lancer GDB avec un test
gdb ./tests/test_name
```

## Intégration continue

Le projet utilise GitHub Actions pour l'intégration continue. Le fichier de configuration se trouve dans `.github/workflows/ci.yml`.

### Exécution locale de la CI

```bash
# Installer act (https://github.com/nektos/act)
# Puis exécuter le workflow CI localement
act -P ubuntu-latest=catthehacker/ubuntu:act-latest
```

## Soumission de modifications

1. Créez une branche pour votre fonctionnalité ou correction de bug :
   ```bash
   git checkout -b feature/ma-nouvelle-fonctionnalite
   ```

2. Effectuez vos modifications et validez-les :
   ```bash
   git add .
   git commit -m "Description claire et concise des modifications"
   ```

3. Poussez vos modifications vers le dépôt distant :
   ```bash
   git push -u origin feature/ma-nouvelle-fonctionnalite
   ```

4. Créez une pull request vers la branche `develop`.

## Conventions de code

### Style de code C++

- Suivez le [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Utilisez `clang-format` pour formater automatiquement votre code
- Les noms des classes commencent par une majuscule (ex: `MyClass`)
- Les noms des méthodes et variables sont en camelCase
- Les constantes sont en UPPER_CASE

### Messages de commit

Utilisez le format suivant pour les messages de commit :

```
type(portée): description courte (50 caractères max)

Description plus détaillée si nécessaire. Expliquez le quoi et le pourquoi du changement,
pas le comment. Limitez chaque ligne à 72 caractères.

- Utilisez des puces pour les listes
- Les points sont optionnels pour les listes à puces

Issue: #123  # Numéro de l'issue associée
```

Types de commit :
- `feat`: Nouvelle fonctionnalité
- `fix`: Correction de bug
- `docs`: Modification de la documentation
- `style`: Mise en forme, point-virgule manquant, etc. (pas de changement de code)
- `refactor`: Refactorisation du code de production
- `test`: Ajout ou modification de tests
- `chore`: Mise à jour des tâches de construction, gestionnaire de paquets, etc.

## Débogage

### Outils recommandés
- **GDB/LLDB** : Débogueur
- **Valgrind** : Détection de fuites mémoire
- **Clang-Tidy** : Analyse statique du code
- **gprof** : Profilage des performances

### Configuration du débogage dans VS Code
Ajoutez cette configuration à `.vscode/launch.json` :

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Debug OpenYolo",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/openyolo",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {"name": "DISPLAY", "value": ":0"},
                {"name": "WAYLAND_DISPLAY", "value": "wayland-0"}
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "setupCommands": [
                {
                    "description": "Activer l'impression en mode pretty",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "build-debug"
        }
    ]
}
```

## Dépannage

### Problèmes courants

#### Erreurs de compilation

- **Problème** : Fichiers manquants ou dépendances non résolues
  - **Solution** : Exécutez `git submodule update --init --recursive` pour mettre à jour les sous-modules

- **Problème** : Erreurs de lien
  - **Solution** : Nettoyez le répertoire de build et reconstruisez :
    ```bash
    rm -rf build/*
    cmake -B build -G Ninja .
    cmake --build build
    ```

#### Problèmes avec Docker

- **Problème** : Erreurs de permissions avec Docker
  - **Solution** : Assurez-vous que votre utilisateur est dans le groupe `docker` :
    ```bash
    sudo usermod -aG docker $USER
    newgrp docker
    ```

- **Problème** : Problèmes d'affichage X11
  - **Solution** : Autorisez les connexions X11 depuis localhost :
    ```bash
    xhost +local:root
    ```

## Licence

Ce projet est sous licence [MIT](LICENSE).
