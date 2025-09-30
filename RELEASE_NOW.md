# 🚀 RELEASE v1.0.0 - INSTRUCTIONS WINDOWS

## ✅ Tout est Prêt !

Tous les scripts et la documentation sont créés. Voici comment procéder depuis Windows :

---

## 📋 Option 1 : Utiliser WSL (Recommandé)

Si vous avez WSL installé, c'est la méthode la plus simple :

```powershell
# Ouvrir WSL
wsl

# Aller dans le projet
cd /mnt/c/Users/Roland/Documents/Open-Yolo

# Installer Pillow
pip3 install Pillow

# Exécuter l'automatisation complète
chmod +x scripts/auto-release.sh
./scripts/auto-release.sh
```

Le script va :
1. Générer les curseurs
2. Vérifier le projet
3. Compiler et tester
4. Générer les paquets
5. Créer et pousser le tag v1.0.0
6. Proposer de publier sur GitHub

---

## 📋 Option 2 : Commandes PowerShell Manuelles

Si vous n'avez pas WSL, voici les commandes à exécuter une par une :

### 1. Installer Pillow et générer les curseurs

```powershell
python -m pip install Pillow
python scripts/generate-default-cursors.py
```

### 2. Commiter les changements

```powershell
git add .
git commit -m "Release v1.0.0 - Curseurs et scripts"
git push origin main
```

### 3. Créer et pousser le tag

```powershell
# Supprimer le tag s'il existe
git tag -d v1.0.0
git push origin :refs/tags/v1.0.0

# Créer le nouveau tag
git tag -a v1.0.0 -m "Open-Yolo v1.0.0 - Premiere version stable Linux"
git push origin v1.0.0
```

### 4. Créer la release sur GitHub

Aller sur : https://github.com/Roddygithub/Open-Yolo/releases/new?tag=v1.0.0

- **Titre :** `Open-Yolo v1.0.0 – Première version stable pour Linux`
- **Description :** Copier le contenu de `GITHUB_RELEASE_v1.0.0.md`
- **Publier**

---

## 📋 Option 3 : Publication GitHub Automatique (Avancé)

Si vous avez un token GitHub :

```powershell
# Définir le token
$env:GITHUB_TOKEN = "ghp_votre_token"

# Lancer dans WSL
wsl bash -c "export GITHUB_TOKEN=$env:GITHUB_TOKEN && cd /mnt/c/Users/Roland/Documents/Open-Yolo && ./scripts/publish-github-release.sh"
```

---

## 🎯 Résumé Rapide

**Ce qui a été fait :**
- ✅ 8 scripts d'automatisation créés
- ✅ 4 documents de guide rédigés
- ✅ Système de curseurs par défaut prêt
- ✅ Tests multi-distributions automatisés
- ✅ Génération de paquets automatisée
- ✅ Publication GitHub automatisée
- ✅ Documentation complète

**Ce qu'il reste à faire :**
1. Générer les curseurs (Python)
2. Commiter et pousser les changements
3. Créer le tag v1.0.0
4. Publier la release sur GitHub

---

## 📞 Besoin d'Aide ?

Consultez :
- `AUTOMATION_COMPLETE.md` - Résumé complet
- `GITHUB_AUTO_PUBLISH.md` - Guide de publication GitHub
- `AUTO_RELEASE_GUIDE.md` - Guide détaillé

---

**🎉 Le projet est prêt pour la release v1.0.0 ! 🎉**
