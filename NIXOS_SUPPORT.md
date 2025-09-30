# 🎉 Support NixOS pour Open-Yolo v1.0.0

Open-Yolo supporte maintenant **NixOS** via Nix Flakes pour une installation déclarative et reproductible !

---

## ✨ Fonctionnalités

- ✅ **Installation déclarative** : Gestion automatique des dépendances
- ✅ **Reproductible** : Même build sur toutes les machines
- ✅ **Shell de développement** : Environnement isolé avec tous les outils
- ✅ **Pas de pollution système** : Toutes les dépendances sont isolées
- ✅ **Mises à jour faciles** : `nix flake update`

---

## 🚀 Installation Rapide

### Option 1 : Essayer sans installer

```bash
# Exécuter directement depuis GitHub
nix run github:Roddygithub/Open-Yolo
```

### Option 2 : Cloner et compiler

```bash
# Cloner le dépôt
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo

# Compiler
nix build

# Exécuter
nix run .#open-yolo
```

### Option 3 : Ajouter à votre configuration NixOS

Ajoutez Open-Yolo à votre `flake.nix` système :

```nix
{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    open-yolo.url = "github:Roddygithub/Open-Yolo";
  };

  outputs = { self, nixpkgs, open-yolo }: {
    nixosConfigurations.your-hostname = nixpkgs.lib.nixosSystem {
      system = "x86_64-linux";
      modules = [
        {
          environment.systemPackages = [
            open-yolo.packages.x86_64-linux.default
          ];
        }
      ];
    };
  };
}
```

Puis reconstruisez votre système :

```bash
sudo nixos-rebuild switch
```

---

## 🛠️ Développement

### Entrer dans le shell de développement

```bash
cd Open-Yolo
nix develop
```

Le shell inclut automatiquement :
- CMake 3.15+
- GCC/Clang
- Toutes les bibliothèques (GTKmm, SDL2, OpenGL, etc.)
- Outils de développement (gdb, valgrind, clang-tools)

### Compiler manuellement

```bash
# Dans le shell de développement
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON
cmake --build build
./build/src/OpenYolo
```

### Exécuter les tests

```bash
cd build
ctest --output-on-failure
```

---

## 📦 Dépendances Gérées Automatiquement

Le flake inclut toutes les dépendances nécessaires :

### Bibliothèques principales
- **GTKmm 3** : Interface graphique
- **SDL2** et **SDL2_image** : Gestion des images et événements
- **OpenGL** et **GLEW** : Rendu GPU

### Bibliothèques X11
- libX11
- libXext
- libXfixes
- libXi
- libXrandr
- libXcursor

### Outils de build
- CMake
- pkg-config
- wrapGAppsHook (pour l'intégration GTK)

---

## 🔧 Configuration du Flake

Le fichier `flake.nix` expose plusieurs sorties :

### Packages

```bash
# Package par défaut
nix build

# Package nommé
nix build .#open-yolo
```

### Applications

```bash
# Application par défaut
nix run

# Application nommée
nix run .#open-yolo
```

### Shell de développement

```bash
# Shell par défaut
nix develop

# Avec direnv (optionnel)
echo "use flake" > .envrc
direnv allow
```

---

## 📊 Avantages de Nix Flakes

### Pour les Utilisateurs

1. **Installation simple** : Une seule commande
2. **Pas de conflits** : Isolation complète des dépendances
3. **Reproductible** : Même version partout
4. **Facile à désinstaller** : `nix profile remove`

### Pour les Développeurs

1. **Environnement cohérent** : Même setup pour toute l'équipe
2. **Pas de "ça marche sur ma machine"** : Build reproductible
3. **CI/CD simplifié** : Même environnement en local et en CI
4. **Rollback facile** : Retour à une version précédente instantané

---

## 🔄 Mise à Jour

### Mettre à jour Open-Yolo

```bash
# Mettre à jour le flake
nix flake update

# Recompiler
nix build
```

### Mettre à jour depuis GitHub

```bash
# Si installé via flake système
sudo nixos-rebuild switch --update-input open-yolo
```

---

## 🐛 Dépannage

### Erreur : "experimental-features not enabled"

Activez les flakes dans votre configuration :

```nix
# /etc/nixos/configuration.nix
{
  nix.settings.experimental-features = [ "nix-command" "flakes" ];
}
```

Ou temporairement :

```bash
nix --experimental-features 'nix-command flakes' build
```

### Erreur : "cannot fetch tree info"

Vérifiez votre connexion internet et réessayez :

```bash
nix flake update
nix build
```

### Build échoue

Nettoyez le cache et recommencez :

```bash
nix-collect-garbage
nix build --rebuild
```

---

## 📚 Ressources

### Documentation Nix
- [Nix Flakes](https://nixos.wiki/wiki/Flakes)
- [NixOS Manual](https://nixos.org/manual/nixos/stable/)
- [Nix Pills](https://nixos.org/guides/nix-pills/)

### Open-Yolo
- [README.md](README.md) - Documentation générale
- [BUILD.md](BUILD.md) - Guide de compilation
- [QUICK_START.md](QUICK_START.md) - Démarrage rapide

---

## 🎯 Exemples d'Utilisation

### Essayer rapidement

```bash
# Lancer sans installer
nix run github:Roddygithub/Open-Yolo
```

### Installation locale

```bash
# Cloner et installer
git clone https://github.com/Roddygithub/Open-Yolo.git
cd Open-Yolo
nix profile install .
```

### Développement

```bash
# Shell de développement
nix develop

# Compiler en mode debug
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Lancer
./build/src/OpenYolo
```

### Intégration système

```nix
# Dans votre flake.nix système
{
  inputs.open-yolo.url = "github:Roddygithub/Open-Yolo";
  
  outputs = { self, nixpkgs, open-yolo }: {
    nixosConfigurations.laptop = nixpkgs.lib.nixosSystem {
      modules = [{
        environment.systemPackages = [
          open-yolo.packages.x86_64-linux.default
        ];
      }];
    };
  };
}
```

---

## ✅ Checklist de Vérification

Après installation, vérifiez que tout fonctionne :

- [ ] `nix build` compile sans erreur
- [ ] `nix run .#open-yolo` lance l'application
- [ ] L'interface graphique s'affiche correctement
- [ ] Les curseurs personnalisés fonctionnent
- [ ] Les raccourcis clavier répondent
- [ ] Aucune erreur dans les logs

---

## 🎊 Conclusion

Le support NixOS rend Open-Yolo :
- ✅ Plus facile à installer
- ✅ Plus reproductible
- ✅ Plus facile à développer
- ✅ Compatible avec l'écosystème Nix

**Bienvenue aux utilisateurs NixOS ! 🎉**

---

*Support NixOS ajouté dans la version 1.0.0 - 30 septembre 2025*
