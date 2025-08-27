# Prompt pour WindSurf – Open-Yolo

## Titre du projet
Open-Yolo – Curseur personnalisé natif Linux

## Objectif
Recoder YoloMouse pour Linux (X11/Wayland) nativement avec :

- Curseur animé et personnalisé
- Multi-écran et support DPI élevé
- Faible consommation CPU/GPU (GPU rendering avec OpenGL/SDL2)
- Raccourcis clavier pour activer/désactiver le curseur
- Interface simple pour configuration

## Technologies recommandées

- Langage : C++ ou Rust
- GUI : Qt 6 ou GTK4
- Graphisme : SDL2/OpenGL/Vulkan
- Build : CMake + PKGBUILD (Arch Linux), DEB/RPM pour autres distributions

## Fonctionnalités à implémenter

- Curseur personnalisé (image ou GIF)
- Animation du curseur avec FPS configurable
- Activation/désactivation via raccourci clavier
- Multi-écran et DPI scaling
- Transparence et effets simples (ombre, glow)
- Interface GUI pour sélectionner curseur, taille, FPS, raccourcis

## Contraintes de performance

- Rendu GPU pour minimiser l’utilisation CPU
- Optimisation de la boucle principale
- Compatible avec jeux OpenGL/Vulkan plein écran

## Arborescence projet recommandée

YoloMouse-Linux/
- src/
  - main.cpp
  - cursor_manager.cpp / cursor_manager.h
  - cursor.cpp / cursor.h
  - gui.cpp / gui.h
  - input.cpp / input.h
  - display.cpp / display.h
  - utils.cpp / utils.h
- assets/
  - cursors/
  - icons/
- include/
- tests/
  - test_cursor.cpp
  - test_gui.cpp
- CMakeLists.txt
- README.md
- LICENSE
- PKGBUILD


## Instructions pour WindSurf

- Compiler le projet sur Arch Linux avec le CMake minimal fourni
- Implémenter les modules `CursorManager`, `DisplayManager`, `GUI` et `Input` selon le pseudo-code existant
- Utiliser SDL2/OpenGL pour le rendu du curseur
- Assurer compatibilité multi-écran et haute DPI
- Ajouter tests unitaires dans `tests/` pour valider le comportement

