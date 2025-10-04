# Gestionnaire de Curseurs Personnalisés

Ce document décrit l'implémentation du gestionnaire de curseurs personnalisés pour Open-Yolo.

## Architecture

### Composants Principaux

1. **CursorManager**
   - Classe de haut niveau pour gérer les curseurs
   - Gère les profils de curseurs
   - Fournit une API simple pour changer de curseur

2. **X11Backend**
   - Implémentation bas niveau pour X11
   - Gère la capture et le remplacement des curseurs système
   - Prend en charge les curseurs personnalisés avec transparence

3. **Interface Utilisateur**
   - Interface GTK pour gérer les curseurs
   - Permet de sélectionner et d'appliquer des profils
   - Aperçu des curseurs personnalisés

## Fonctionnalités Implémentées

- [x] Capture du curseur système
- [x] Remplacement par des curseurs personnalisés
- [x] Gestion des profils de curseurs
- [x] Interface utilisateur graphique
- [x] Support des curseurs avec transparence
- [x] Gestion des différents types de curseurs (pointeur, main, texte, etc.)

## Utilisation

### API de Base

```cpp
// Obtenir l'instance du gestionnaire
auto& cursorManager = input::CursorManager::getInstance();

// Initialiser le gestionnaire
if (!cursorManager.initialize()) {
    // Gestion de l'erreur
}

// Définir un curseur personnalisé
cursorManager.setCursor(
    CursorManager::CursorType::Pointer,
    "chemin/vers/mon_curseur.png",
    32  // Taille du curseur
);

// Utiliser les curseurs système
cursorManager.useSystemCursors(true);
```

### Interface Graphique

L'interface graphique permet de :
- Sélectionner un profil de curseur
- Prévisualiser les curseurs
- Créer et gérer des profils personnalisés
- Basculer entre les curseurs système et personnalisés

## Formats de Fichiers Supportés

- PNG (recommandé pour la transparence)
- XPM
- XBM

## Structure des Répertoires

```
resources/
└── cursors/
    ├── pointer.png    # Curseur de pointeur par défaut
    ├── hand.png       # Curseur de main
    ├── crosshair.png  # Curseur de viseur
    └── text.png       # Curseur de texte
```

## Dépendances

- GTKMM 3.0+ (interface utilisateur)
- X11 (pour le backend X11)
- Xcursor (pour la gestion des curseurs)
- libpng (pour le chargement des images)

## Prochaines Étapes

- [ ] Support de Wayland
- [ ] Effets de curseurs avancés (ombres, animations)
- [ ] Import/export de profils
- [ ] Bibliothèque de curseurs intégrée

## Dépannage

### Le curseur ne change pas
- Vérifiez que l'application a les droits nécessaires
- Vérifiez que le fichier du curseur existe et est accessible
- Vérifiez les logs pour les erreurs potentielles

### Problèmes de transparence
- Assurez-vous d'utiliser le format PNG avec un canal alpha
- Vérifiez que votre gestionnaire de fenêtres prend en charge les curseurs avec transparence

## Licence

Ce projet est sous licence MIT. Voir le fichier `LICENSE` pour plus de détails.
