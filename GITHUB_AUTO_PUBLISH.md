# 📤 Publication Automatique GitHub - Guide Complet

Guide pour publier automatiquement la release v1.0.0 sur GitHub sans intervention manuelle.

---

## 🎯 Vue d'Ensemble

La publication automatique utilise l'API GitHub pour :
1. ✅ Créer la release
2. ✅ Uploader tous les artefacts (DEB, RPM, TGZ, PKGBUILD)
3. ✅ Ajouter les captures d'écran
4. ✅ Définir comme "latest release"
5. ✅ Publier instantanément

---

## 🔑 Configuration du Token GitHub

### Option 1 : Token Personnel (Recommandé)

1. **Créer un token :**
   - Aller sur : https://github.com/settings/tokens/new
   - **Note :** `Open-Yolo Release Automation`
   - **Expiration :** 90 jours (ou personnalisé)
   - **Permissions :** Cocher `repo` (toutes les sous-permissions)
   - Cliquer sur "Generate token"
   - **Copier le token** (il ne sera affiché qu'une fois !)

2. **Exporter le token :**
   ```bash
   export GITHUB_TOKEN=ghp_votre_token_ici
   ```

3. **Rendre permanent (optionnel) :**
   ```bash
   # Ajouter à ~/.bashrc ou ~/.zshrc
   echo 'export GITHUB_TOKEN=ghp_votre_token_ici' >> ~/.bashrc
   source ~/.bashrc
   ```

### Option 2 : GitHub CLI (Alternative)

```bash
# Installer GitHub CLI
# Ubuntu/Debian
sudo apt install gh

# Fedora
sudo dnf install gh

# Arch
sudo pacman -S github-cli

# Authentification
gh auth login

# Le token sera automatiquement configuré
```

---

## 🚀 Utilisation

### Méthode 1 : Publication Automatique Complète

```bash
# 1. Configurer le token
export GITHUB_TOKEN=ghp_votre_token

# 2. Exécuter l'automatisation complète
chmod +x scripts/auto-release.sh
./scripts/auto-release.sh

# Le script va demander confirmation avant de publier
# Répondre 'Y' pour publier automatiquement
```

**Durée :** ~20-25 minutes (incluant la publication)

### Méthode 2 : Publication Séparée

```bash
# 1. Préparer la release (sans publier)
./scripts/auto-release.sh
# Répondre 'N' quand demandé

# 2. Publier manuellement plus tard
export GITHUB_TOKEN=ghp_votre_token
chmod +x scripts/publish-github-release.sh
./scripts/publish-github-release.sh
```

### Méthode 3 : Publication Uniquement

Si tout est déjà prêt (tag, paquets, etc.) :

```bash
export GITHUB_TOKEN=ghp_votre_token
./scripts/publish-github-release.sh
```

---

## 📋 Ce qui est Publié Automatiquement

### Informations de la Release

- **Tag :** `v1.0.0`
- **Titre :** `Open-Yolo v1.0.0 – Première version stable pour Linux`
- **Description :** Contenu complet de `GITHUB_RELEASE_v1.0.0.md`
- **Statut :** Latest release (non-draft, non-prerelease)

### Artefacts Uploadés

#### Paquets (depuis `packages/`)
- `open-yolo_1.0.0_amd64.deb` (~2 MB)
- `open-yolo-1.0.0-1.x86_64.rpm` (~2 MB)
- `open-yolo-1.0.0-Linux.tar.gz` (~3 MB)

#### Documentation
- `CHANGELOG.md` - Historique complet
- `PKGBUILD` - Pour Arch Linux

#### Captures d'Écran (depuis `docs/screenshots/`)
- `main-window.png`
- `effects-tab.png`
- `shortcuts-tab.png`
- `cursor-active.png`
- Toutes les autres captures disponibles

---

## 🔍 Vérifications Automatiques

Le script effectue les vérifications suivantes :

### Avant Publication

1. ✅ **Token GitHub** valide et avec les bonnes permissions
2. ✅ **Accès au dépôt** confirmé
3. ✅ **Tag v1.0.0** existe (créé si nécessaire)
4. ✅ **Artefacts** présents dans `packages/`
5. ✅ **Description** disponible (`GITHUB_RELEASE_v1.0.0.md`)

### Pendant Publication

1. ✅ **Suppression** de l'ancienne release si existante
2. ✅ **Création** de la nouvelle release
3. ✅ **Upload** de chaque artefact avec vérification
4. ✅ **Confirmation** de chaque upload

### Après Publication

1. ✅ **Vérification** que la release est visible
2. ✅ **Comptage** des artefacts uploadés
3. ✅ **Affichage** de l'URL de la release

---

## 📊 Résultat Attendu

### Console Output

```
╔═══════════════════════════════════════════════════════╗
║                                                       ║
║     📤 PUBLICATION GITHUB AUTOMATIQUE 📤             ║
║                                                       ║
║         Open-Yolo v1.0.0 Release                      ║
║                                                       ║
╚═══════════════════════════════════════════════════════╝

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  ÉTAPE 1/8 : Vérification du Token GitHub
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

✓ Token GitHub trouvé
✓ Accès au dépôt confirmé

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  ÉTAPE 2/8 : Vérification du Tag v1.0.0
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

✓ Tag v1.0.0 existe localement
✓ Tag poussé vers GitHub

[... autres étapes ...]

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  ÉTAPE 7/8 : Upload des Artefacts
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

ℹ Upload de open-yolo_1.0.0_amd64.deb...
✓ open-yolo_1.0.0_amd64.deb uploadé (ID: 123456)
ℹ Upload de open-yolo-1.0.0-1.x86_64.rpm...
✓ open-yolo-1.0.0-1.x86_64.rpm uploadé (ID: 123457)
[...]

========================================
  ✅ Publication Réussie !
========================================

📊 Résumé de la Release :

  Tag : v1.0.0
  Titre : Open-Yolo v1.0.0 – Première version stable pour Linux
  Artefacts : 8 fichiers
  URL : https://github.com/Roddygithub/Open-Yolo/releases/tag/v1.0.0

📦 Artefacts Inclus :
  - open-yolo_1.0.0_amd64.deb (2MB)
  - open-yolo-1.0.0-1.x86_64.rpm (2MB)
  - open-yolo-1.0.0-Linux.tar.gz (3MB)
  - CHANGELOG.md (0MB)
  - PKGBUILD (0MB)
  - main-window.png (0MB)
  - effects-tab.png (0MB)
  - shortcuts-tab.png (0MB)

🔗 Liens :
  Release : https://github.com/Roddygithub/Open-Yolo/releases/tag/v1.0.0
  Téléchargements : https://github.com/Roddygithub/Open-Yolo/releases/download/v1.0.0/

━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
  🎉 Release v1.0.0 Publiée ! 🎉
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
```

### Sur GitHub

La release sera visible sur :
```
https://github.com/Roddygithub/Open-Yolo/releases/tag/v1.0.0
```

Avec :
- ✅ Badge "Latest"
- ✅ Description complète formatée
- ✅ Tous les artefacts téléchargeables
- ✅ Captures d'écran visibles
- ✅ Statistiques de téléchargement

---

## 🐛 Dépannage

### Erreur : "GITHUB_TOKEN non défini"

```bash
# Vérifier si le token est défini
echo $GITHUB_TOKEN

# Si vide, l'exporter
export GITHUB_TOKEN=ghp_votre_token
```

### Erreur : "Impossible d'accéder au dépôt"

**Causes possibles :**
1. Token invalide ou expiré
2. Permissions insuffisantes
3. Nom de dépôt incorrect

**Solution :**
```bash
# Vérifier le token
curl -H "Authorization: token $GITHUB_TOKEN" \
  https://api.github.com/user

# Vérifier les permissions
curl -H "Authorization: token $GITHUB_TOKEN" \
  https://api.github.com/repos/Roddygithub/Open-Yolo
```

### Erreur : "Échec d'upload d'artefact"

**Causes possibles :**
1. Fichier trop volumineux (> 2GB)
2. Connexion interrompue
3. Nom de fichier invalide

**Solution :**
```bash
# Réessayer l'upload
./scripts/publish-github-release.sh

# Ou uploader manuellement un fichier spécifique
UPLOAD_URL="..." # Récupéré du script
curl -X POST \
  -H "Authorization: token $GITHUB_TOKEN" \
  -H "Content-Type: application/octet-stream" \
  --data-binary @packages/fichier.deb \
  "$UPLOAD_URL?name=fichier.deb"
```

### Erreur : "Release déjà existante"

Le script supprime automatiquement l'ancienne release.
Si l'erreur persiste :

```bash
# Supprimer manuellement
gh release delete v1.0.0 --yes

# Ou via l'interface GitHub
# https://github.com/Roddygithub/Open-Yolo/releases
```

### Erreur : "jq: command not found"

```bash
# Ubuntu/Debian
sudo apt install jq

# Fedora
sudo dnf install jq

# Arch
sudo pacman -S jq
```

---

## 🔒 Sécurité

### Bonnes Pratiques

1. **Ne jamais commiter le token** dans Git
2. **Utiliser des tokens avec expiration**
3. **Permissions minimales** (seulement `repo`)
4. **Révoquer** les tokens inutilisés
5. **Variables d'environnement** plutôt que fichiers

### Révocation du Token

Si le token est compromis :

1. Aller sur : https://github.com/settings/tokens
2. Trouver le token "Open-Yolo Release Automation"
3. Cliquer sur "Delete"
4. Créer un nouveau token

---

## 📈 Métriques de Succès

### Indicateurs de Réussite

- ✅ Release visible sur GitHub
- ✅ Badge "Latest" affiché
- ✅ Tous les artefacts téléchargeables
- ✅ Description complète affichée
- ✅ Captures d'écran visibles
- ✅ Statistiques de téléchargement actives

### Vérification Post-Publication

```bash
# Vérifier la release
curl -s https://api.github.com/repos/Roddygithub/Open-Yolo/releases/tags/v1.0.0 | jq .

# Compter les téléchargements
curl -s https://api.github.com/repos/Roddygithub/Open-Yolo/releases/tags/v1.0.0 \
  | jq '.assets[] | {name: .name, downloads: .download_count}'
```

---

## 🎯 Workflow Complet

### Scénario Idéal : Tout Automatique

```bash
# 1. Configuration (une fois)
export GITHUB_TOKEN=ghp_votre_token
echo 'export GITHUB_TOKEN=ghp_votre_token' >> ~/.bashrc

# 2. Installation des dépendances (une fois)
pip install Pillow
sudo apt install jq curl

# 3. Automatisation complète
./scripts/auto-release.sh

# Répondre 'Y' quand demandé pour la publication

# 4. Vérifier la release
# https://github.com/Roddygithub/Open-Yolo/releases/tag/v1.0.0
```

**Durée totale :** ~25 minutes  
**Intervention manuelle :** Aucune (après configuration)

---

## 📞 Support

### En Cas de Problème

1. **Vérifier les logs** du script
2. **Consulter** ce guide
3. **Tester** avec GitHub CLI : `gh release create`
4. **Ouvrir une issue** sur GitHub

### Ressources

- **API GitHub** : https://docs.github.com/en/rest/releases
- **GitHub CLI** : https://cli.github.com/
- **Tokens** : https://github.com/settings/tokens

---

## 🎊 Résumé

La publication automatique permet de :

✅ **Gagner du temps** : ~15 minutes économisées  
✅ **Éviter les erreurs** : Pas d'oubli d'artefact  
✅ **Être reproductible** : Même processus à chaque fois  
✅ **Être traçable** : Logs complets de chaque étape  

**Une seule commande pour tout publier !**

```bash
export GITHUB_TOKEN=ghp_votre_token
./scripts/auto-release.sh
```

---

*Document créé le 30 septembre 2025*
