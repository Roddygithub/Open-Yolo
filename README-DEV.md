# OpenYolo - Guide de développement

Ce guide explique comment configurer et utiliser l'environnement de développement pour OpenYolo.

## Prérequis

- Docker (version 20.10+)
- Docker Compose (version 2.0+)
- Un système d'exploitation Linux
- Au moins 4 Go de RAM disponible
- Au moins 5 Go d'espace disque disponible

## Configuration de l'environnement

1. **Cloner le dépôt**
   ```bash
   git clone https://github.com/Roddygithub/Open-Yolo.git
   cd Open-Yolo
   ```

2. **Configurer l'environnement**
   ```bash
   chmod +x scripts/*.sh
   ./scripts/setup-dev.sh
   ```

3. **Accéder au conteneur de développement**
   ```bash
   docker-compose exec app bash
   ```

## Commandes utiles

### Compiler le projet
```bash
./scripts/build.sh
```

### Exécuter les tests
```bash
./scripts/run-tests.sh
```

### Nettoyer le projet
```bash
./scripts/clean.sh
```

### Formater le code
```bash
./scripts/format.sh
```

### Démarrer l'application
```bash
./scripts/start-dev.sh
```

## Structure des répertoires

- `src/` - Code source de l'application
- `include/` - Fichiers d'en-tête
- `resources/` - Ressources de l'application (icônes, images, etc.)
- `scripts/` - Scripts utilitaires
- `tests/` - Tests unitaires et d'intégration
- `build/` - Fichiers de construction (généré)

## Dépannage

### Problèmes de permissions
Si vous rencontrez des problèmes de permissions avec Docker, assurez-vous que votre utilisateur est dans le groupe `docker` :

```bash
sudo usermod -aG docker $USER
```

Puis déconnectez-vous et reconnectez-vous pour que les changements prennent effet.

### Problèmes de construction
Si la construction échoue, essayez de nettoyer le projet et de reconstruire :

```bash
./scripts/clean.sh
docker-compose build --no-cache
```

## Contribution

1. Créez une branche pour votre fonctionnalité (`git checkout -b feature/nouvelle-fonctionnalite`)
2. Committez vos modifications (`git commit -am 'Ajouter une nouvelle fonctionnalité'`)
3. Poussez vers la branche (`git push origin feature/nouvelle-fonctionnalite`)
4. Créez une Pull Request

## Licence

Ce projet est sous licence GPL-3.0. Voir le fichier `LICENSE` pour plus de détails.
