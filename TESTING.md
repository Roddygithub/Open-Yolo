# 🧪 Testing Guide - Open-Yolo

Guide complet pour tester Open-Yolo sur toutes les plateformes supportées.

---

## 📋 Plateformes Testées

Open-Yolo est automatiquement testé sur les plateformes suivantes via GitHub Actions :

| Plateforme | Version | Statut | Docker Image |
|------------|---------|--------|--------------|
| **Ubuntu** | 22.04 LTS | ✅ Testé | `ubuntu:22.04` |
| **Ubuntu** | 24.04 LTS | ✅ Testé | `ubuntu:24.04` |
| **Fedora** | Latest | ✅ Testé | `fedora:latest` |
| **Arch Linux** | Rolling | ✅ Testé | `archlinux:latest` |
| **CachyOS** | Latest | ✅ Testé | `cachyos/cachyos:latest` |
| **Windows** | WSL Ubuntu 22.04 | ✅ Testé | Via WSL |
| **NixOS** | Unstable | ✅ Testé | `nixos/nix:latest` |

---

## 🚀 Tests Automatisés (CI/CD)

### GitHub Actions

Le workflow CI/CD se déclenche automatiquement sur :
- Push vers `main` ou `develop`
- Pull requests vers `main`
- Création de releases

**Voir les résultats :** https://github.com/Roddygithub/Open-Yolo/actions

### Jobs Exécutés

1. **test-linux** : Tests sur toutes les distributions Linux
2. **test-windows** : Tests sur Windows via WSL
3. **test-nixos** : Tests avec Nix Flakes
4. **code-quality** : Couverture de code et analyse
5. **package** : Génération des packages (sur release uniquement)

---

## 🧪 Tests Locaux

### Option 1 : Tests Multi-Plateformes avec Docker

#### Linux/macOS

```bash
# Donner les permissions d'exécution
chmod +x scripts/test-all-platforms.sh

# Lancer tous les tests
./scripts/test-all-platforms.sh
```

#### Windows (PowerShell)

```powershell
# Lancer tous les tests
.\scripts\test-all-platforms.ps1
```

### Option 2 : Tests sur une Plateforme Spécifique

#### Ubuntu 22.04

```bash
docker build -f docker/Dockerfile.ubuntu22 -t openyolo-test-ubuntu22 .
docker run --rm openyolo-test-ubuntu22
```

#### Ubuntu 24.04

```bash
docker build -f docker/Dockerfile.ubuntu24 -t openyolo-test-ubuntu24 .
docker run --rm openyolo-test-ubuntu24
```

#### Fedora

```bash
docker build -f docker/Dockerfile.fedora -t openyolo-test-fedora .
docker run --rm openyolo-test-fedora
```

#### Arch Linux

```bash
docker build -f docker/Dockerfile.archlinux -t openyolo-test-arch .
docker run --rm openyolo-test-arch
```

#### CachyOS

```bash
docker build -f docker/Dockerfile.cachyos -t openyolo-test-cachyos .
docker run --rm openyolo-test-cachyos
```

#### Windows (WSL)

```bash
docker build -f docker/Dockerfile.windows-wsl -t openyolo-test-wsl .
docker run --rm openyolo-test-wsl
```

#### NixOS

```bash
docker build -f docker/Dockerfile.nixos -t openyolo-test-nixos .
docker run --rm openyolo-test-nixos
```

---

## 🔧 Tests Unitaires Manuels

### Compilation avec Tests

```bash
# Configurer avec tests activés
cmake -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_LOGGING=ON

# Compiler
cmake --build build -j$(nproc)

# Lancer les tests avec Xvfb (nécessaire pour les tests GUI)
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99
cd build && ctest --output-on-failure --verbose
```

### Tests avec Couverture de Code

```bash
# Configurer avec couverture
cmake -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_COVERAGE=ON

# Compiler et tester
cmake --build build -j$(nproc)
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99
cd build && ctest --output-on-failure

# Générer le rapport de couverture
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.info
lcov --list coverage.info
```

### Tests avec AddressSanitizer

```bash
# Configurer avec ASAN
cmake -B build \
    -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON \
    -DENABLE_ASAN=ON

# Compiler et tester
cmake --build build -j$(nproc)
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99
cd build && ctest --output-on-failure
```

---

## 📊 Résultats des Tests

### Tests Unitaires

Les tests couvrent :
- ✅ **CursorManager** : Gestion des curseurs personnalisés
- ✅ **DisplayManager** : Gestion multi-écrans et HiDPI
- ✅ **InputManager** : Gestion des entrées clavier/souris
- ✅ **GUI** : Interface graphique GTK

### Couverture de Code

- **Objectif** : > 70%
- **Actuel** : Voir badge dans README.md
- **Rapport** : Disponible sur Codecov

### Performance

| Scénario | CPU | RAM | GPU |
|----------|-----|-----|-----|
| Idle (curseur statique) | < 1% | ~50 MB | Minimal |
| Animation 30 FPS | 2-3% | ~70 MB | Faible |
| Animation 60 FPS | 4-5% | ~80 MB | Modéré |

---

## 🐛 Dépannage

### Erreur : "DISPLAY not set"

```bash
# Démarrer Xvfb
Xvfb :99 -screen 0 1024x768x24 &
export DISPLAY=:99
```

### Erreur : "Cannot open display"

```bash
# Vérifier que Xvfb est en cours d'exécution
ps aux | grep Xvfb

# Redémarrer si nécessaire
killall Xvfb
Xvfb :99 -screen 0 1024x768x24 &
```

### Erreur : "Docker build failed"

```bash
# Nettoyer les images Docker
docker system prune -a

# Reconstruire
docker build -f docker/Dockerfile.ubuntu22 -t openyolo-test-ubuntu22 .
```

### Tests échouent sur une plateforme spécifique

1. Vérifier les logs détaillés :
   ```bash
   docker run --rm openyolo-test-<platform> 2>&1 | tee test-output.log
   ```

2. Entrer dans le conteneur pour déboguer :
   ```bash
   docker run -it --rm openyolo-test-<platform> bash
   ```

3. Vérifier les dépendances :
   ```bash
   # Dans le conteneur
   ldd build/src/OpenYolo
   ```

---

## 📈 Métriques de Qualité

### Build Status

[![Build Status](https://github.com/Roddygithub/Open-Yolo/actions/workflows/ci.yml/badge.svg)](https://github.com/Roddygithub/Open-Yolo/actions)

### Code Coverage

[![codecov](https://codecov.io/gh/Roddygithub/Open-Yolo/branch/main/graph/badge.svg)](https://codecov.io/gh/Roddygithub/Open-Yolo)

---

## 🔄 Workflow de Contribution

Pour contribuer avec des tests :

1. **Ajouter des tests** dans `tests/`
2. **Vérifier localement** :
   ```bash
   cmake -B build -DBUILD_TESTS=ON
   cmake --build build
   cd build && ctest
   ```
3. **Créer une PR** : Les tests CI/CD se lanceront automatiquement
4. **Vérifier les résultats** sur GitHub Actions

---

## 📚 Ressources

- **GitHub Actions** : `.github/workflows/ci.yml`
- **Dockerfiles** : `docker/`
- **Scripts de test** : `scripts/test-all-platforms.*`
- **Tests unitaires** : `tests/`
- **Documentation** : `BUILD.md`, `README.md`

---

**Dernière mise à jour :** 30 septembre 2025
