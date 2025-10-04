# Gestion des Raccourcis Clavier

Ce document décrit comment les raccourcis clavier sont gérés dans Open-Yolo, y compris comment les configurer et les personnaliser.

## Configuration des Raccourcis

Les raccourcis sont stockés dans le fichier `~/.config/open-yolo/shortcuts.json`. Ce fichier est créé automatiquement avec des valeurs par défaut lors de la première exécution.

### Format du Fichier de Configuration

Le fichier de configuration utilise le format JSON avec les paires clé-valeur suivantes :

```json
{
    "toggle_visible": ["Control_L", "Alt_L", "S"],
    "increase_size": ["Control_L", "Alt_L", "Up"],
    "decrease_size": ["Control_L", "Alt_L", "Down"],
    "next_theme": ["Control_L", "Alt_L", "Right"],
    "prev_theme": ["Control_L", "Alt_L", "Left"],
    "reset_cursor": ["Control_L", "Alt_L", "R"],
    "toggle_tracking": ["Control_L", "Alt_L", "P"],
    "toggle_lock": ["Control_L", "Alt_L", "L"],
    "toggle_game_mode": ["Control_L", "Alt_L", "G"],
    "increase_opacity": ["Control_L", "Alt_L", "KP_Add"],
    "decrease_opacity": ["Control_L", "Alt_L", "KP_Subtract"],
    "set_theme_1": ["Control_L", "Alt_L", "1"],
    "set_theme_2": ["Control_L", "Alt_L", "2"],
    "set_theme_3": ["Control_L", "Alt_L", "3"],
    "set_theme_4": ["Control_L", "Alt_L", "4"],
    "set_theme_5": ["Control_L", "Alt_L", "5"],
    "set_theme_6": ["Control_L", "Alt_L", "6"],
    "set_theme_7": ["Control_L", "Alt_L", "7"],
    "set_theme_8": ["Control_L", "Alt_L", "8"],
    "set_theme_9": ["Control_L", "Alt_L", "9"]
}
```

### Actions Disponibles

| Action | Description | Raccourci par défaut |
|--------|-------------|----------------------|
| `toggle_visible` | Affiche/Masque le curseur | Ctrl+Alt+S |
| `increase_size` | Augmente la taille du curseur | Ctrl+Alt+Flèche Haut |
| `decrease_size` | Diminue la taille du curseur | Ctrl+Alt+Flèche Bas |
| `next_theme` | Passe au thème suivant | Ctrl+Alt+Flèche Droite |
| `prev_theme` | Passe au thème précédent | Ctrl+Alt+Flèche Gauche |
| `reset_cursor` | Réinitialise le curseur | Ctrl+Alt+R |
| `toggle_tracking` | Active/Désactive le mode suivi | Ctrl+Alt+P |
| `toggle_lock` | Verrouille/Déverrouille le curseur | Ctrl+Alt+L |
| `toggle_game_mode` | Active/Désactive le mode jeu | Ctrl+Alt+G |
| `increase_opacity` | Augmente l'opacité | Ctrl+Alt++ (Pavé Num) |
| `decrease_opacity` | Diminue l'opacité | Ctrl+Alt+- (Pavé Num) |
| `set_theme_N` | Définit le thème N (1-9) | Ctrl+Alt+[1-9] |

### Personnalisation des Raccourcis

1. Fermez l'application Open-Yolo si elle est en cours d'exécution.
2. Éditez le fichier `~/.config/open-yolo/shortcuts.json` avec votre éditeur de texte préféré.
3. Modifiez les touches comme souhaité. Les noms des touches doivent correspondre aux codes de touches X11.
4. Enregistrez le fichier et redémarrez Open-Yolo.

### Réinitialisation des Raccourcis

Pour rétablir les raccourcis par défaut, supprimez simplement le fichier `~/.config/open-yolo/shortcuts.json` et redémarrez l'application. Un nouveau fichier avec les valeurs par défaut sera recréé automatiquement.

## Dépannage

Si un raccourci ne fonctionne pas :

1. Vérifiez que la combinaison de touches n'est pas déjà utilisée par une autre application.
2. Assurez-vous que le fichier de configuration est au format JSON valide.
3. Consultez les journaux de l'application pour des messages d'erreur.
4. Si vous utilisez Wayland, certains raccourcis globaux pourraient ne pas fonctionner en raison des restrictions du protocole.

## Notes Techniques

- Les raccourcis sont gérés par la classe `ShortcutManager` qui utilise X11 pour la détection des touches.
- Les modifications des raccourcis prennent effet immédiatement après le redémarrage de l'application.
- Les raccourcis sont stockés de manière sécurisée avec des permissions restreintes (600).
