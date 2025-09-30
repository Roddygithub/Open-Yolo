# Changelog - Open-Yolo

Tous les changements notables de ce projet seront documentés dans ce fichier.

Le format est basé sur [Keep a Changelog](https://keepachangelog.com/fr/1.0.0/),
et ce projet adhère au [Semantic Versioning](https://semver.org/lang/fr/).

## [1.0.0] - 2025-09-30

### ✨ Ajouté

#### Fonctionnalités principales
- **Curseurs personnalisés animés** : Support complet des GIF et images statiques
- **Rendu GPU optimisé** : Utilisation d'OpenGL pour un rendu performant avec faible consommation CPU
- **Support multi-écrans** : Gestion complète des configurations multi-moniteurs
- **Support HiDPI** : Adaptation automatique aux écrans haute densité
- **Effets visuels** :
  - Effet d'ombre configurable (offset, flou, couleur)
  - Effet de lueur (glow) avec intensité réglable
  - Pipeline d'effets personnalisable
- **Interface graphique GTK4** :
  - Onglet de configuration du curseur
  - Onglet de gestion des raccourcis clavier
  - Onglet de configuration multi-écrans
  - Prévisualisation en temps réel
- **Raccourcis clavier** : Système complet de raccourcis personnalisables
- **Configuration persistante** : Sauvegarde automatique des paramètres

#### Système de build
- **CMake moderne** : Configuration CMake 3.15+ avec options avancées
- **Support des tests** : Intégration de Google Test
- **Options de compilation** :
  - `BUILD_TESTS` : Compilation des tests unitaires
  - `ENABLE_COVERAGE` : Couverture de code
  - `ENABLE_ASAN` : AddressSanitizer pour la détection de bugs
  - `ENABLE_LTO` : Link Time Optimization
- **Packaging** :
  - Support CPack (DEB, RPM, TGZ)
  - PKGBUILD pour Arch Linux
  - Script automatique de génération de paquets

#### CI/CD
- **GitHub Actions** : Pipeline complet de build et test
- **Multi-distributions** : Tests sur Ubuntu 20.04 et 22.04
- **Builds Debug et Release** : Validation des deux modes
- **Couverture de code** : Intégration avec Codecov
- **Releases automatiques** : Upload des artefacts lors des releases

#### Docker
- **Dockerfile optimisé** : Build multi-stage pour images légères
- **Docker Compose** : Configuration pour le développement
- **Support Xvfb** : Affichage virtuel pour environnements headless

#### Documentation
- **README.md** : Documentation complète en français
- **BUILD.md** : Guide détaillé de compilation
- **CHANGELOG.md** : Historique des modifications
- **Commentaires de code** : Documentation inline extensive

### 🔧 Corrigé

#### Bugs critiques
- **CursorManager** :
  - Ajout des méthodes manquantes `setEnabled()` et `setSize()`
  - Correction de la gestion de l'état d'activation
  - Fix des fuites mémoire dans les textures OpenGL
  
- **DisplayManager** :
  - Ajout des méthodes `setWindowSize()`, `setFullscreen()`, `beginFrame()`, `endFrame()`
  - Correction du champ `ptr` manquant dans `DisplayInfo`
  - Fix de la détection des écrans avec XRandR
  
- **InputManager** :
  - Ajout de la méthode `initialize()` manquante
  - Correction de la gestion des périphériques d'entrée
  - Fix des raccourcis clavier avec modificateurs

#### Tests
- **test_cursormanager.cpp** :
  - Correction des tests attendant des exceptions non lancées
  - Amélioration de la robustesse des tests
  - Ajout de tests de performance GPU

#### Compilation
- **CMakeLists.txt** :
  - Correction des dépendances manquantes
  - Fix des warnings de compilation
  - Amélioration de la détection des bibliothèques
  - Support complet de GTKmm 3.0

### 🚀 Optimisé

#### Performance
- **Rendu GPU** :
  - Utilisation de framebuffers pour les effets
  - Pipeline de shaders optimisé
  - Réduction de la consommation CPU (< 1% en idle)
  - Support du VSync pour éviter le tearing
  
- **Gestion mémoire** :
  - Implémentation PIMPL pour réduire les dépendances
  - Smart pointers pour éviter les fuites mémoire
  - Optimisation des allocations dans la boucle de rendu

#### Architecture
- **Séparation des responsabilités** :
  - CursorManager : Gestion des curseurs et rendu
  - DisplayManager : Gestion des écrans
  - InputManager : Gestion des entrées
  - MainWindow : Interface utilisateur
  
- **Thread safety** :
  - Utilisation de mutex pour les accès concurrents
  - Boucle de rendu dans un thread séparé
  - Synchronisation propre entre threads

### 📚 Documentation

#### Guides
- **Installation** : Instructions détaillées pour Ubuntu, Fedora, Arch
- **Compilation** : Guide complet avec toutes les options
- **Utilisation** : Exemples d'utilisation et configuration
- **Développement** : Guide pour les contributeurs

#### API
- **Headers documentés** : Commentaires Doxygen pour toutes les classes publiques
- **Exemples de code** : Snippets d'utilisation dans la documentation

### 🔒 Sécurité

- **Validation des entrées** : Vérification de tous les paramètres utilisateur
- **Gestion des erreurs** : Try-catch pour toutes les opérations critiques
- **Permissions** : Exécution en tant qu'utilisateur non-root
- **Sanitizers** : Support d'AddressSanitizer et UndefinedBehaviorSanitizer

### 🎨 Interface

- **Thème moderne** : Interface GTK4 avec thème Adwaita
- **Responsive** : Adaptation à différentes tailles d'écran
- **Accessibilité** : Support des raccourcis clavier
- **Internationalisation** : Préparation pour le support multi-langues

### 🐳 Docker

- **Image optimisée** : Build multi-stage pour réduire la taille
- **Dépendances minimales** : Seulement les bibliothèques nécessaires au runtime
- **Support X11** : Configuration pour l'affichage graphique
- **Variables d'environnement** : Configuration flexible via ENV

### 📦 Packaging

- **DEB** : Paquet Debian/Ubuntu avec dépendances automatiques
- **RPM** : Paquet Fedora/RHEL avec métadonnées complètes
- **PKGBUILD** : Support natif pour Arch Linux
- **AppImage** : (Prévu pour v1.1.0)

## [Unreleased]

### Prévu pour v1.1.0

- [ ] Support Wayland natif
- [ ] Thèmes de curseurs prédéfinis
- [ ] Import/Export de configurations
- [ ] Profils de curseurs par application
- [ ] Support des curseurs vectoriels (SVG)
- [ ] Interface en ligne de commande (CLI)
- [ ] Plugin system pour effets personnalisés
- [ ] Support de Qt6 en alternative à GTK
- [ ] AppImage packaging
- [ ] Flatpak packaging
- [ ] Snap packaging

### Prévu pour v1.2.0

- [ ] Synchronisation cloud des configurations
- [ ] Marketplace de curseurs
- [ ] Éditeur de curseurs intégré
- [ ] Support des animations complexes (sprite sheets)
- [ ] Mode économie d'énergie
- [ ] Statistiques d'utilisation
- [ ] Thème sombre/clair automatique

## Notes de migration

### De v0.x à v1.0.0

**Changements incompatibles :**
- Les fichiers de configuration ont changé de format
- L'API C++ a été refactorisée (PIMPL pattern)
- Les chemins d'installation ont été normalisés

**Migration :**
1. Sauvegarder vos configurations existantes
2. Désinstaller l'ancienne version
3. Installer la nouvelle version
4. Reconfigurer vos préférences via l'interface graphique

## Contributeurs

Merci à tous les contributeurs qui ont rendu cette version possible !

## Licence

Ce projet est sous licence MIT. Voir le fichier [LICENSE](LICENSE) pour plus de détails.
