# Créer un Fichier DMG pour ZenRunner (macOS)

Ce guide explique comment créer un fichier `.dmg` pour installer ZenRunner sur macOS.

## 🚀 Méthode Rapide (Recommandée)

### Prérequis

- **macOS** (10.15 ou supérieur)
- **Xcode Command Line Tools** : `xcode-select --install`
- **Qt 6.2+** installé (via Homebrew ou site officiel)
- **CMake 3.21+**

### Installation de Qt (si nécessaire)

```bash
# Via Homebrew (recommandé)
brew install qt@6

# Ajouter Qt au PATH
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"
export CMAKE_PREFIX_PATH="/opt/homebrew/opt/qt@6"
```

## 📦 Création du DMG

### Étape 1 : Compiler l'application

```bash
# Naviguer vers le dossier du projet
cd /chemin/vers/ZenRunner

# Compiler l'application (sans installer)
./install.sh --no-install
```

**Note**: Si vous n'avez pas le flag `--no-install`, vous pouvez simplement compiler avec:
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
cd ..
```

### Étape 2 : Créer le fichier DMG

```bash
# Naviguer vers le dossier de packaging
cd packaging/macos

# Exécuter le script de création DMG
./build-dmg.sh
```

### Étape 3 : Récupérer le DMG

Le fichier DMG sera créé dans : **`build/ZenRunner-1.0.0.dmg`**

```bash
# Vérifier que le DMG a été créé
ls -lh ../../build/ZenRunner-1.0.0.dmg
```

## ✅ Test du DMG

### Tester l'installation

1. **Ouvrir le DMG** :
   ```bash
   open ../../build/ZenRunner-1.0.0.dmg
   ```

2. **Installer** :
   - Une fenêtre s'ouvre avec l'icône ZenRunner et un raccourci vers Applications
   - Glisser-déposer ZenRunner vers Applications

3. **Lancer** :
   - Ouvrir le Finder → Applications → ZenRunner
   - Au premier lancement : Clic droit → Ouvrir (pour contourner Gatekeeper)

4. **Vérifier** :
   - L'application démarre en < 2 secondes
   - Interface fluide à 60 FPS
   - Utilisation mémoire < 30 MB

## 🎨 Personnalisation (Optionnel)

### Ajouter une Icône Personnalisée

```bash
# Créer une icône .icns (512x512@2x recommandé)
# Placer dans : packaging/macos/ZenRunner.icns

# Le script build-dmg.sh l'utilisera automatiquement
```

### Ajouter une Image de Fond

```bash
# Créer une image de fond (600x400 recommandé)
# Placer dans : packaging/macos/background.png

# Modifier build-dmg.sh pour l'inclure
```

## 🔒 Signature de Code (Distribution)

Pour distribuer le DMG en dehors de l'App Store, la signature est recommandée.

### Prérequis
- Compte Apple Developer
- Certificat "Developer ID Application"

### Signature

Le script `build-dmg.sh` propose automatiquement de signer si un certificat est détecté.

```bash
# Le script demandera :
# "Do you want to sign the application? (y/N):"
# Répondre 'y' pour signer
```

### Notarisation (macOS 10.15+)

Pour que le DMG s'ouvre sans avertissement de sécurité :

```bash
# Soumettre pour notarisation
xcrun notarytool submit build/ZenRunner-1.0.0.dmg \
  --apple-id votre@email.com \
  --team-id VOTRE_TEAM_ID \
  --password mot-de-passe-specifique-app \
  --wait

# Agrafer le ticket de notarisation
xcrun stapler staple build/ZenRunner-1.0.0.dmg

# Vérifier
xcrun stapler validate build/ZenRunner-1.0.0.dmg
```

**Note**: La notarisation nécessite :
- Compte Apple Developer payant
- Mot de passe spécifique à l'application (généré dans votre compte Apple)

## 📋 Structure du DMG

Le DMG créé contient :

```
ZenRunner-1.0.0.dmg (monté)
├── ZenRunner.app           # Bundle de l'application
│   ├── Contents/
│   │   ├── Info.plist      # Métadonnées
│   │   ├── MacOS/
│   │   │   └── ZenRunner   # Exécutable
│   │   ├── Resources/
│   │   │   └── ZenRunner.icns  # Icône
│   │   └── Frameworks/     # Bibliothèques Qt
│   │       ├── QtCore.framework
│   │       ├── QtGui.framework
│   │       ├── QtQuick.framework
│   │       └── QtWidgets.framework
└── Applications (symlink)  # Raccourci vers /Applications
```

## 🛠️ Résolution de Problèmes

### Erreur : "macdeployqt not found"

```bash
# Installer Qt via Homebrew
brew install qt@6

# Ajouter au PATH
export PATH="/opt/homebrew/opt/qt@6/bin:$PATH"

# Ou spécifier le chemin complet dans build-dmg.sh
```

### Erreur : "Executable not found in build directory"

```bash
# Vérifier que la compilation a réussi
ls -la build/bin/ZenRunner

# Recompiler si nécessaire
./install.sh --no-install
```

### Erreur : "hdiutil: Resource busy"

```bash
# Démonter les volumes existants
hdiutil detach "/Volumes/ZenRunner 1.0.0" -force

# Réessayer
cd packaging/macos && ./build-dmg.sh
```

### Le DMG ne s'ouvre pas sur d'autres Macs

- **Cause** : Binaire compilé avec `-march=native`
- **Solution** : Compiler sans `ENABLE_NATIVE_OPTIMIZATIONS` (désactivé par défaut)
  ```bash
  cd build
  cmake .. -DCMAKE_BUILD_TYPE=Release -DENABLE_NATIVE_OPTIMIZATIONS=OFF
  cmake --build . --config Release
  ```

### L'application crash au lancement après installation

- **Cause** : Problème de rpaths ou de signature de code
- **Solution** : Le script build-dmg.sh a été mis à jour pour:
  - Supprimer les rpaths Homebrew absolus
  - Ajouter les rpaths relatifs corrects (`@executable_path/../Frameworks`)
  - Vérifier la présence de tous les frameworks Qt requis
  - Signer correctement le bundle avec les bonnes options
  
  ```bash
  # Vérifier les rpaths après création du bundle
  otool -l build/ZenRunner.app/Contents/MacOS/ZenRunner | grep -A2 LC_RPATH
  
  # Vérifier les frameworks Qt présents
  ls -la build/ZenRunner.app/Contents/Frameworks/
  
  # Vérifier la signature
  codesign --verify --verbose build/ZenRunner.app
  ```

## 📊 Taille Attendue

- **Application non compressée** : ~20-30 MB
- **DMG compressé** : ~10-15 MB
- **Après installation** : ~25-35 MB dans /Applications

## 🎯 Checklist de Distribution

Avant de distribuer le DMG :

- [ ] Compilation en mode Release
- [ ] Désactivation de `-march=native` (portabilité)
- [ ] Test sur un Mac différent
- [ ] Test sur macOS 10.15+ (si possible plusieurs versions)
- [ ] Vérification de l'utilisation mémoire (< 30 MB)
- [ ] Test de tous les scripts npm (si applicables)
- [ ] Signature de code (recommandée)
- [ ] Notarisation (requise pour macOS 10.15+)
- [ ] Documentation mise à jour

## 📚 Documentation Complète

Pour plus de détails :

- **Guide Complet** : [docs/PACKAGING.md](../../docs/PACKAGING.md)
- **Sécurité** : [docs/SECURITY.md](../../docs/SECURITY.md)
- **Installation** : [INSTALLATION_RECOMMANDATIONS.md](../../INSTALLATION_RECOMMANDATIONS.md)
- **Analyse du Code** : [RAPPORT_ANALYSE_FR.md](../../RAPPORT_ANALYSE_FR.md)

## 🆘 Support

En cas de problème :

1. **Consulter le guide de diagnostic**: [DIAGNOSTIC_DMG.md](packaging/macos/DIAGNOSTIC_DMG.md) - Guide complet pour diagnostiquer et résoudre les problèmes de crash
2. **Consulter la documentation**: [docs/PACKAGING.md](../../docs/PACKAGING.md)
3. **Vérifier les logs** pendant la création du DMG
4. **Tester sur une machine différente** pour isoler les problèmes d'environnement
5. **Ouvrir une issue**: [GitHub Issues](https://github.com/marcucus/ZenRunner/issues) avec les logs et informations de diagnostic

---

## 🚀 Commande Complète (Une Ligne)

```bash
# Compiler et créer le DMG en une seule commande
./install.sh --no-install && cd packaging/macos && ./build-dmg.sh && cd ../.. && echo "✅ DMG créé: build/ZenRunner-1.0.0.dmg"
```

---

**Bon packaging ! 🎉**

*Pour toute question, consultez la [documentation complète](../../docs/PACKAGING.md) ou ouvrez une issue sur GitHub.*
