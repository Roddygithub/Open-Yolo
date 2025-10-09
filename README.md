# OpenYolo - Gestionnaire de curseurs personnalisés pour Linux

[![CI/CD](https://github.com/Roddygithub/Open-Yolo/actions/workflows/ci.yml/badge.svg)](https://github.com/Roddygithub/Open-Yolo/actions/workflows/ci.yml)
[![Docker Image](https://img.shields.io/docker/pulls/roddygithub/open-yolo)](https://hub.docker.com/r/roddygithub/open-yolo)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

OpenYolo est un gestionnaire de curseurs personnalisables pour Linux, inspiré de Yolo Mouse. Il permet de personnaliser facilement l'apparence du curseur de la souris sur un système Linux.

## 🚀 Fonctionnalités

- Personnalisation avancée des curseurs
- Support de thèmes personnalisés
- Interface utilisateur intuitive
- Léger et rapide
- Intégration native avec les environnements de bureau Linux

## 📦 Prérequis

- Linux (testé sur Ubuntu 22.04+)
- Docker et Docker Compose (pour le développement)
- Git

## 🛠 Installation

### Depuis les sources

1. **Cloner le dépôt** :
   ```bash
   git clone https://github.com/Roddygithub/Open-Yolo.git
   cd Open-Yolo
   ```

2. **Construire avec Docker** :
   ```bash
   docker-compose build
   ```

3. **Lancer l'application** :
   ```bash
   xhost +local:docker
   docker-compose up openyolo
   ```

### Installation directe (pour les développeurs)

1. **Installer les dépendances système** :
   ```bash
   sudo apt-get update
   sudo apt-get install -y build-essential cmake ninja-build pkg-config \
       libgtk-3-dev libgtkmm-3.0-dev libcairomm-1.0-dev \
       libglibmm-2.4-dev libsigc++-2.0-dev libx11-dev \
       libxrandr-dev libxi-dev
   ```

2. **Construire le projet** :
   ```bash
   mkdir -p build && cd build
   cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
   cmake --build . --config Release
   ```

3. **Installer** (optionnel) :
   ```bash
   sudo cmake --install .
   ```

## 🐳 Utilisation avec Docker

### Développement

Pour lancer un shell dans l'environnement de développement :

```bash
docker-compose run --rm openyolo
```

### Exécuter les tests

```bash
docker-compose run --rm test
```

### Formater le code

```bash
docker-compose run --rm format
```

## 🧪 Tests

Pour exécuter les tests localement :

```bash
cd build
ctest --output-on-failure
```

## 🤝 Contribution

Les contributions sont les bienvenues ! Voici comment contribuer :

1. Forkez le projet
2. Créez une branche pour votre fonctionnalité (`git checkout -b feature/AmazingFeature`)
3. Committez vos changements (`git commit -m 'Add some AmazingFeature'`)
4. Poussez vers la branche (`git push origin feature/AmazingFeature`)
5. Ouvrez une Pull Request

## 📝 Licence

Distribué sous la licence MIT. Voir le fichier `LICENSE` pour plus d'informations.

## 📞 Contact

Roddy - [@VotreTwitter](https://twitter.com/VotreTwitter)

Lien du projet : [https://github.com/Roddygithub/Open-Yolo](https://github.com/Roddygithub/Open-Yolo)

## 🙏 Remerciements

- [Yolo Mouse](https://pavelfatin.com/typing-slow-and-typing-fast/) pour l'inspiration
- Tous les contributeurs qui ont participé à ce projet
