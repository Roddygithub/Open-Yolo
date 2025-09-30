# 🤖 Guide d'Automatisation Release v1.0.0

Guide complet pour utiliser les scripts d'automatisation de la release.

---

## 🎯 Vue d'Ensemble

L'automatisation de la release v1.0.0 est divisée en plusieurs scripts spécialisés :

| Script | Fonction | Durée |
|--------|----------|-------|
| `generate-default-cursors.py` | Génère les curseurs par défaut | 1 min |
| `pre-release-check.sh` | Vérifie que tout est prêt | 5 min |
| `test-multi-distros.sh` | Test sur Ubuntu/Fedora/Arch | 30 min |
| `benchmark-performance.sh` | Mesure les performances | 2 min |
| `build-packages.sh` | Génère DEB/RPM/TGZ | 10 min |
| `auto-screenshots.sh` | Capture d'écran automatique | 5 min |
| `auto-release.sh` | **Script master** | 20 min |

---

## 🚀 Utilisation Rapide (Script Master)

### Option 1 : Automatisation Complète

```bash
# Tout en une seule commande
chmod +x scripts/auto-release.sh
./scripts/auto-release.sh
```

Ce script va automatiquement :
1. ✅ Générer les curseurs par défaut
2. ✅ Vérifier que tout est prêt
3. ✅ Compiler le projet
4. ✅ Exécuter les tests
5. ✅ Mesurer les performances
6. ✅ Générer les paquets
7. ✅ Vérifier Git
8. ✅ Commiter les changements
9. ✅ Créer et pousser le tag v1.0.0
10. ✅ Préparer la release GitHub

**Durée totale :** ~20 minutes

---

## 📋 Utilisation Détaillée (Scripts Individuels)

### 1. Génération des Curseurs

```bash
# Installer Pillow si nécessaire
pip install Pillow

# Générer les curseurs
python3 scripts/generate-default-cursors.py
```

**Curseurs générés :**
- `assets/cursors/default.png` (32x32)
- `assets/cursors/pointer.png` (32x32)
- `assets/cursors/animated-cursor.gif` (32x32, 8 frames)
- `assets/cursors/loading.gif` (32x32, 12 frames)
- `assets/cursors/default-hd.png` (64x64)
- `assets/cursors/pointer-hd.png` (64x64)
- `assets/cursors/animated-cursor-hd.gif` (64x64, 8 frames)

### 2. Vérification Pré-Release

```bash
chmod +x scripts/pre-release-check.sh
./scripts/pre-release-check.sh
```

**Vérifie :**
- Structure du projet
- Documentation complète
- Scripts présents
- Compilation réussie
- Tests passants
- Génération de paquets
- État Git
- Versions correctes

### 3. Tests Multi-Distributions

```bash
# Nécessite Docker
chmod +x scripts/test-multi-distros.sh
./scripts/test-multi-distros.sh
```

**Teste sur :**
- Ubuntu 22.04
- Ubuntu 20.04
- Fedora 38
- Arch Linux

**Pour chaque distribution :**
- ✅ Compilation
- ✅ Tests unitaires
- ✅ Génération du binaire

**Durée :** ~30 minutes (selon connexion)

### 4. Benchmark des Performances

```bash
chmod +x scripts/benchmark-performance.sh
./scripts/benchmark-performance.sh
```

**Mesure :**
- Taille du binaire
- Temps de démarrage
- Consommation RAM (RSS/VSZ)
- Consommation CPU (moyenne sur 5s)

**Résultat attendu :**
- CPU < 5% : ✅ Excellent
- CPU < 10% : ✅ Bon
- CPU > 10% : ⚠ À optimiser

### 5. Génération des Paquets

```bash
chmod +x scripts/build-packages.sh
./scripts/build-packages.sh
```

**Génère :**
- `packages/open-yolo_1.0.0_amd64.deb` (~2 MB)
- `packages/open-yolo-1.0.0-1.x86_64.rpm` (~2 MB)
- `packages/open-yolo-1.0.0-Linux.tar.gz` (~3 MB)

### 6. Captures d'Écran

```bash
chmod +x scripts/auto-screenshots.sh
./scripts/auto-screenshots.sh
```

**Capture :**
- Interface principale
- Onglet Effets
- Onglet Raccourcis
- Curseur actif (optionnel)

**Sauvegarde dans :** `docs/screenshots/`

---

## 🔄 Workflow Complet

### Scénario 1 : Release Complète Automatisée

```bash
# 1. Exécuter le script master
./scripts/auto-release.sh

# 2. Créer la release sur GitHub
# https://github.com/Roddygithub/Open-Yolo/releases/new?tag=v1.0.0

# 3. Copier la description
cat GITHUB_RELEASE_v1.0.0.md

# 4. Uploader les artefacts
ls packages/

# 5. Publier !
```

### Scénario 2 : Tests Approfondis

```bash
# 1. Vérifications
./scripts/pre-release-check.sh

# 2. Tests multi-distros (optionnel, long)
./scripts/test-multi-distros.sh

# 3. Benchmark
./scripts/benchmark-performance.sh

# 4. Si tout OK, release
./scripts/auto-release.sh
```

### Scénario 3 : Release Manuelle

```bash
# 1. Curseurs
python3 scripts/generate-default-cursors.py

# 2. Compilation et tests
make build
make test

# 3. Paquets
./scripts/build-packages.sh

# 4. Git
git add .
git commit -m "Release v1.0.0"
git push origin main
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0

# 5. Créer la release sur GitHub
```

---

## 📊 Checklist de Release

### Avant d'Exécuter auto-release.sh

- [ ] Python3 et Pillow installés
- [ ] Docker installé (pour tests multi-distros)
- [ ] Git configuré (user.name, user.email)
- [ ] Accès push vers GitHub
- [ ] Sur la branche `main`
- [ ] Modifications commitées ou prêtes

### Après auto-release.sh

- [ ] Tag v1.0.0 créé et poussé
- [ ] Paquets générés dans `packages/`
- [ ] Curseurs dans `assets/cursors/`
- [ ] Fichier `RELEASE_NOTES_v1.0.0.md` créé
- [ ] Aucune erreur critique

### Publication GitHub

- [ ] Aller sur GitHub Releases
- [ ] Sélectionner le tag v1.0.0
- [ ] Copier la description depuis `GITHUB_RELEASE_v1.0.0.md`
- [ ] Uploader tous les fichiers de `packages/`
- [ ] Uploader les captures (optionnel)
- [ ] Cocher "Set as the latest release"
- [ ] Publier !

---

## 🐛 Dépannage

### Erreur : "Pillow non installé"

```bash
pip install Pillow
# ou
pip3 install Pillow
```

### Erreur : "Docker non trouvé"

```bash
# Ubuntu/Debian
sudo apt install docker.io
sudo usermod -aG docker $USER
# Se déconnecter/reconnecter

# Fedora
sudo dnf install docker
sudo systemctl start docker
```

### Erreur : "Permission denied" sur les scripts

```bash
chmod +x scripts/*.sh
```

### Erreur : "Tag already exists"

Le script propose automatiquement de supprimer et recréer le tag.
Ou manuellement :

```bash
git tag -d v1.0.0
git push origin :refs/tags/v1.0.0
```

### Erreur : "Cannot push to GitHub"

Vérifier :
```bash
git remote -v
git config user.name
git config user.email
ssh -T git@github.com  # ou tester HTTPS
```

### Tests qui échouent

```bash
# Recompiler proprement
make clean
make build

# Exécuter les tests avec détails
cd build
ctest -V
```

---

## 📈 Métriques de Succès

### Performances Attendues

| Métrique | Cible | Excellent |
|----------|-------|-----------|
| CPU (idle) | < 5% | < 1% |
| RAM | < 100 MB | < 70 MB |
| Démarrage | < 2s | < 1s |
| Binaire | < 5 MB | < 3 MB |

### Tests

- ✅ 100% des tests doivent passer
- ✅ Compilation sur 4 distributions
- ✅ Aucune fuite mémoire (Valgrind)
- ✅ Aucune erreur ASAN

### Paquets

- ✅ DEB installable sur Ubuntu
- ✅ RPM installable sur Fedora
- ✅ PKGBUILD compilable sur Arch
- ✅ TGZ extractible partout

---

## 🎯 Résultat Final

Après exécution complète, vous aurez :

### Fichiers Générés

```
Open-Yolo/
├── assets/cursors/          # 7 curseurs par défaut
├── packages/                # 3 paquets (DEB, RPM, TGZ)
├── docs/screenshots/        # Captures d'écran
├── RELEASE_NOTES_v1.0.0.md # Description de release
└── build/                   # Binaire compilé
```

### Git

- ✅ Branche `main` à jour
- ✅ Tag `v1.0.0` créé et poussé
- ✅ Tous les changements commitées

### GitHub

- ⏳ Release à créer manuellement
- ⏳ Artefacts à uploader
- ⏳ Description à copier-coller

---

## 🚀 Commande Ultime

Pour tout faire en une seule commande :

```bash
# Installation des dépendances (une fois)
pip install Pillow

# Automatisation complète
chmod +x scripts/auto-release.sh && ./scripts/auto-release.sh

# Puis créer la release sur GitHub avec les instructions affichées
```

---

## 📞 Support

Si vous rencontrez des problèmes :

1. Vérifier les logs de chaque script
2. Consulter `RELEASE_INSTRUCTIONS.md`
3. Ouvrir une issue sur GitHub

---

**🎊 Bonne release ! 🎊**

*Document créé le 30 septembre 2025*
