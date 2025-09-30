# 📋 Instructions de Release - Open-Yolo v1.0.0

Guide complet pour créer et publier la release GitHub v1.0.0.

---

## ✅ Checklist Pré-Release

### 1. Vérifications Préliminaires

```bash
# Exécuter le script de vérification
chmod +x scripts/pre-release-check.sh
./scripts/pre-release-check.sh
```

**Vérifier que :**
- [ ] Tous les tests passent (`make test`)
- [ ] Le projet compile sans erreur (`make build`)
- [ ] La documentation est à jour
- [ ] Aucune modification non commitée
- [ ] Sur la branche `main`
- [ ] Version correcte dans CMakeLists.txt et PKGBUILD

### 2. Tests sur Différentes Distributions

**Ubuntu 22.04 :**
```bash
docker run -it ubuntu:22.04 bash
# Installer dépendances et compiler
```

**Fedora 36 :**
```bash
docker run -it fedora:36 bash
# Installer dépendances et compiler
```

**Arch Linux :**
```bash
docker run -it archlinux bash
# Tester PKGBUILD
```

---

## 🚀 Création de la Release

### Étape 1 : Préparer les Artefacts

```bash
# Exécuter le script de création de release
chmod +x scripts/create-release.sh
./scripts/create-release.sh
```

Ce script va :
1. ✅ Créer le tag `v1.0.0`
2. ✅ Pousser le tag vers GitHub
3. ✅ Générer les paquets (DEB, RPM, TGZ)
4. ✅ Créer le fichier de description

**Artefacts générés dans `packages/` :**
- `open-yolo_1.0.0_amd64.deb` (~2 MB)
- `open-yolo-1.0.0-1.x86_64.rpm` (~2 MB)
- `open-yolo-1.0.0-Linux.tar.gz` (~3 MB)

### Étape 2 : Créer les Captures d'Écran

```bash
# Lancer le script de captures
chmod +x scripts/create-screenshots.sh
./scripts/create-screenshots.sh
```

**Captures nécessaires :**
1. `main-window.png` - Interface principale
2. `cursor-active.png` - Curseur personnalisé actif
3. `cursor-animated.gif` - Curseur animé (optionnel)
4. `effects-tab.png` - Onglet Effets
5. `shortcuts-tab.png` - Onglet Raccourcis
6. `multi-screen.png` - Configuration multi-écrans (optionnel)

**Sauvegarder dans :** `docs/screenshots/`

### Étape 3 : Créer la Release sur GitHub

1. **Aller sur GitHub :**
   ```
   https://github.com/Roddygithub/Open-Yolo/releases/new
   ```

2. **Sélectionner le tag :**
   - Tag : `v1.0.0`
   - Target : `main`

3. **Titre de la release :**
   ```
   Open-Yolo v1.0.0 – Première version stable pour Linux
   ```

4. **Description :**
   - Copier le contenu de `GITHUB_RELEASE_v1.0.0.md`
   - Ou copier le contenu de `RELEASE_NOTES_v1.0.0.md` (version condensée)

5. **Uploader les artefacts :**
   - Glisser-déposer tous les fichiers du dossier `packages/`
   - Ajouter les captures d'écran du dossier `docs/screenshots/`

6. **Options :**
   - ☑️ Set as the latest release
   - ☐ Set as a pre-release (décocher)
   - ☐ Create a discussion for this release (optionnel)

7. **Publier :**
   - Cliquer sur "Publish release"

---

## 📝 Contenu de la Description (Copier-Coller)

### Version Complète

Utiliser le fichier : `GITHUB_RELEASE_v1.0.0.md`

**Sections incluses :**
- ✨ Fonctionnalités principales
- 📦 Contenu de la release
- 🚀 Installation rapide
- 📊 Performances mesurées
- 🔧 Compilation depuis les sources
- 🧪 Tests et qualité
- 📚 Documentation complète
- 🐛 Problèmes connus
- 🗺️ Roadmap
- 🤝 Contribution
- 📄 Licence
- 🙏 Remerciements
- 📞 Support

### Version Condensée

Utiliser le fichier : `RELEASE_NOTES_v1.0.0.md`

**Sections incluses :**
- ✨ Fonctionnalités principales
- 📦 Contenu de la release
- 🚀 Installation
- 📊 Performances
- 🔧 Compilation
- 🧪 Tests
- 📚 Documentation
- 🗺️ Roadmap

---

## 🖼️ Ajout des Captures d'Écran

### Dans la Description Markdown

Ajouter après la section "Fonctionnalités" :

```markdown
## 📸 Captures d'Écran

### Interface Principale
![Interface principale](https://github.com/Roddygithub/Open-Yolo/raw/main/docs/screenshots/main-window.png)

### Curseur Personnalisé Actif
![Curseur actif](https://github.com/Roddygithub/Open-Yolo/raw/main/docs/screenshots/cursor-active.png)

### Onglet Effets
![Onglet Effets](https://github.com/Roddygithub/Open-Yolo/raw/main/docs/screenshots/effects-tab.png)

### Configuration Multi-Écrans
![Multi-écrans](https://github.com/Roddygithub/Open-Yolo/raw/main/docs/screenshots/multi-screen.png)
```

### Uploader les Images

1. Commiter les captures dans le dépôt :
   ```bash
   git add docs/screenshots/
   git commit -m "Add screenshots for v1.0.0 release"
   git push origin main
   ```

2. Ou les uploader directement dans la release GitHub

---

## 📢 Annonce de la Release

### 1. Mettre à Jour le README

Ajouter un badge de release :

```markdown
[![Latest Release](https://img.shields.io/github/v/release/Roddygithub/Open-Yolo)](https://github.com/Roddygithub/Open-Yolo/releases/latest)
[![Downloads](https://img.shields.io/github/downloads/Roddygithub/Open-Yolo/total)](https://github.com/Roddygithub/Open-Yolo/releases)
```

### 2. Créer une Discussion GitHub

```
Titre : 🎉 Open-Yolo v1.0.0 Released!

Contenu :
Nous sommes ravis d'annoncer la première version stable d'Open-Yolo !

🔗 Release : https://github.com/Roddygithub/Open-Yolo/releases/tag/v1.0.0

Principales fonctionnalités :
- Curseurs personnalisés et animés
- Support multi-écrans et HiDPI
- Rendu GPU optimisé (< 1% CPU)
- Interface GTK intuitive

Merci à tous les contributeurs et testeurs ! 🙏

N'hésitez pas à partager vos retours et suggestions.
```

### 3. Réseaux Sociaux (Optionnel)

**Twitter/X :**
```
🎉 Open-Yolo v1.0.0 is out!

Alternative native Linux à YoloMouse
✅ Curseurs animés
✅ Multi-écrans
✅ Rendu GPU
✅ Open Source

Download: https://github.com/Roddygithub/Open-Yolo/releases/tag/v1.0.0

#Linux #OpenSource #Gaming
```

**Reddit (r/linux, r/linuxgaming) :**
```
Titre : [Release] Open-Yolo v1.0.0 - Custom Cursor Manager for Linux

Contenu : Lien vers la release + description courte
```

### 4. Publier sur AUR (Arch User Repository)

```bash
# Cloner le dépôt AUR
git clone ssh://aur@aur.archlinux.org/open-yolo.git
cd open-yolo

# Copier le PKGBUILD
cp ../Open-Yolo/PKGBUILD .

# Générer .SRCINFO
makepkg --printsrcinfo > .SRCINFO

# Commiter et pousser
git add PKGBUILD .SRCINFO
git commit -m "Update to v1.0.0"
git push
```

---

## ✅ Vérifications Post-Release

### 1. Vérifier la Release

- [ ] La release est visible sur GitHub
- [ ] Tous les artefacts sont téléchargeables
- [ ] Les liens dans la description fonctionnent
- [ ] Les badges sont à jour
- [ ] Les captures d'écran s'affichent

### 2. Tester les Téléchargements

```bash
# Télécharger et tester chaque paquet
wget https://github.com/Roddygithub/Open-Yolo/releases/download/v1.0.0/open-yolo_1.0.0_amd64.deb
sudo dpkg -i open-yolo_1.0.0_amd64.deb
OpenYolo --version
```

### 3. Vérifier les Statistiques

- Nombre de téléchargements
- Nombre de stars
- Issues ouvertes

---

## 🔄 Après la Release

### 1. Préparer la Prochaine Version

```bash
# Créer une branche develop
git checkout -b develop

# Mettre à jour la version
# Dans CMakeLists.txt : VERSION 1.1.0
# Dans PKGBUILD : pkgver=1.1.0

git add CMakeLists.txt PKGBUILD
git commit -m "Bump version to 1.1.0-dev"
git push origin develop
```

### 2. Mettre à Jour la Roadmap

Mettre à jour `CHANGELOG.md` avec la section `[Unreleased]` :

```markdown
## [Unreleased]

### Prévu pour v1.1.0
- [ ] Support Wayland natif
- [ ] Thèmes de curseurs prédéfinis
- [ ] Import/Export de configurations
```

### 3. Répondre aux Issues et Discussions

- Surveiller les nouvelles issues
- Répondre aux questions
- Merger les pull requests

---

## 📊 Métriques de Succès

### Objectifs v1.0.0

- [ ] 100+ téléchargements la première semaine
- [ ] 50+ stars GitHub
- [ ] 10+ discussions/issues
- [ ] 0 bug critique

### Suivi

Utiliser GitHub Insights :
```
https://github.com/Roddygithub/Open-Yolo/pulse
```

---

## 🆘 En Cas de Problème

### Problème : Tag Incorrect

```bash
# Supprimer le tag local
git tag -d v1.0.0

# Supprimer le tag distant
git push origin :refs/tags/v1.0.0

# Recréer le tag
git tag -a v1.0.0 -m "Message"
git push origin v1.0.0
```

### Problème : Release à Supprimer

1. Aller sur la page de la release
2. Cliquer sur "Delete"
3. Confirmer
4. Recréer la release

### Problème : Artefact Corrompu

1. Régénérer les paquets localement
2. Éditer la release
3. Supprimer l'ancien artefact
4. Uploader le nouveau

---

## 📝 Checklist Finale

Avant de publier :

- [ ] Script de vérification exécuté sans erreur
- [ ] Tous les tests passent
- [ ] Paquets générés et testés
- [ ] Captures d'écran créées
- [ ] Description de release rédigée
- [ ] Tag créé et poussé
- [ ] Release créée sur GitHub
- [ ] Artefacts uploadés
- [ ] Captures uploadées
- [ ] Release publiée
- [ ] README mis à jour
- [ ] Annonce faite

---

**🎉 Félicitations ! La release v1.0.0 est publiée ! 🎉**

---

*Document créé le 30 septembre 2025*
