# Guide de Diagnostic DMG pour ZenRunner

Ce guide vous aide à diagnostiquer et résoudre les problèmes de crash de l'application ZenRunner après installation depuis un DMG.

## 🔍 Vérifications Rapides

### 1. Vérifier que le bundle est complet

```bash
# Vérifier la structure du bundle
ls -la build/ZenRunner.app/Contents/

# Doit contenir:
# - MacOS/         (exécutable)
# - Frameworks/    (bibliothèques Qt)
# - PlugIns/       (plugins Qt)
# - Resources/     (ressources)
# - Info.plist     (métadonnées)
```

### 2. Vérifier les frameworks Qt

```bash
# Lister les frameworks Qt bundlés
ls -la build/ZenRunner.app/Contents/Frameworks/

# Frameworks requis:
# - QtCore.framework
# - QtGui.framework
# - QtQuick.framework
# - QtWidgets.framework
# - QtQml.framework
```

### 3. Vérifier les rpaths

```bash
# Vérifier les rpaths de l'exécutable
otool -l build/ZenRunner.app/Contents/MacOS/ZenRunner | grep -A2 LC_RPATH

# Doit contenir:
# path @executable_path/../Frameworks (offset XX)
```

### 4. Vérifier les dépendances

```bash
# Lister toutes les dépendances de l'exécutable
otool -L build/ZenRunner.app/Contents/MacOS/ZenRunner

# Toutes les bibliothèques Qt doivent utiliser @rpath:
# @rpath/QtCore.framework/Versions/A/QtCore
# @rpath/QtGui.framework/Versions/A/QtGui
# etc.

# PAS de chemins absolus comme:
# /opt/homebrew/... ❌
# /usr/local/... ❌
```

### 5. Vérifier la signature

```bash
# Vérifier que le bundle est signé
codesign --verify --verbose build/ZenRunner.app

# Doit afficher:
# build/ZenRunner.app: valid on disk
# build/ZenRunner.app: satisfies its Designated Requirement

# Afficher les détails de la signature
codesign -dv --verbose=4 build/ZenRunner.app
```

## 🔧 Diagnostics Avancés

### Lancer l'application en mode debug

```bash
# Méthode 1: Depuis le terminal pour voir les erreurs
/path/to/ZenRunner.app/Contents/MacOS/ZenRunner

# Méthode 2: Avec les logs système
log stream --predicate 'processImagePath contains "ZenRunner"' --level debug
# Dans un autre terminal:
open /Applications/ZenRunner.app
```

### Vérifier les logs Console.app

1. Ouvrir **Console.app** (Applications > Utilitaires > Console)
2. Lancer ZenRunner
3. Filtrer par "ZenRunner" dans la barre de recherche
4. Chercher des erreurs comme:
   - `dyld: Library not loaded`
   - `Reason: image not found`
   - `Code signature invalid`

### Utiliser dyld_info pour analyser les dépendances

```bash
# Installer dyld_info si pas déjà installé (via Xcode Command Line Tools)
dyld_info -dependents build/ZenRunner.app/Contents/MacOS/ZenRunner

# Vérifier que toutes les dépendances sont trouvables
```

## 🐛 Problèmes Courants et Solutions

### Erreur: "dyld: Library not loaded: @rpath/QtCore.framework"

**Cause**: Les rpaths ne sont pas correctement configurés

**Solution**:
```bash
# Vérifier les rpaths
otool -l build/ZenRunner.app/Contents/MacOS/ZenRunner | grep -A2 LC_RPATH

# Si @executable_path/../Frameworks n'est pas présent, l'ajouter:
install_name_tool -add_rpath "@executable_path/../Frameworks" \
  build/ZenRunner.app/Contents/MacOS/ZenRunner

# Re-signer le bundle
codesign --force --deep --sign - build/ZenRunner.app
```

### Erreur: "Library not loaded: /opt/homebrew/opt/qt@6/..."

**Cause**: Les frameworks Qt ont des chemins absolus Homebrew

**Solution**:
```bash
# Relancer macdeployqt pour fixer les chemins
/opt/homebrew/opt/qt@6/bin/macdeployqt build/ZenRunner.app \
  -always-overwrite

# Ou utiliser le script build-dmg.sh mis à jour qui fait cela automatiquement
cd packaging/macos && ./build-dmg.sh
```

### Erreur: "Code signature invalid"

**Cause**: Signature de code incorrecte ou expirée

**Solution**:
```bash
# Re-signer le bundle (ad-hoc pour usage local)
codesign --force --deep --sign - build/ZenRunner.app

# Vérifier la signature
codesign --verify --verbose build/ZenRunner.app
```

### Erreur: "damaged and can't be opened" (Gatekeeper)

**Cause**: macOS Gatekeeper bloque l'application non notarisée

**Solution 1** (Usage local):
```bash
# Supprimer la quarantine
xattr -cr /Applications/ZenRunner.app

# Ou lors de l'ouverture, clic droit > Ouvrir
```

**Solution 2** (Distribution):
```bash
# Signer avec Developer ID et notariser
# Voir la section "Notarisation" dans CREER_DMG.md
```

### L'application se lance mais crash immédiatement

**Diagnostics**:
```bash
# 1. Vérifier les plugins Qt
ls -la build/ZenRunner.app/Contents/PlugIns/

# 2. Vérifier que resources.qrc est bien intégré
strings build/ZenRunner.app/Contents/MacOS/ZenRunner | grep ":/ui/"

# 3. Vérifier les permissions
chmod -R u+w build/ZenRunner.app
chmod +x build/ZenRunner.app/Contents/MacOS/ZenRunner

# 4. Lancer en mode verbose
QT_LOGGING_RULES="*.debug=true" \
  /Applications/ZenRunner.app/Contents/MacOS/ZenRunner
```

## 📋 Checklist de Validation

Avant de créer le DMG final, vérifier:

- [ ] Tous les frameworks Qt sont présents dans `Contents/Frameworks/`
- [ ] Les plugins Qt sont présents dans `Contents/PlugIns/`
- [ ] `Info.plist` est présent et valide
- [ ] L'exécutable a les bonnes permissions (`chmod +x`)
- [ ] Les rpaths utilisent `@executable_path/../Frameworks`
- [ ] Aucune dépendance n'utilise de chemin absolu Homebrew
- [ ] Le bundle est signé (même ad-hoc)
- [ ] La signature est valide (`codesign --verify`)
- [ ] L'application se lance depuis le terminal sans erreur
- [ ] L'application se lance depuis Finder

## 🧪 Test d'Installation Complet

```bash
# 1. Créer un DMG de test
cd packaging/macos && ./build-dmg.sh

# 2. Monter le DMG
open ../../build/ZenRunner-1.0.0.dmg

# 3. Copier vers Applications (simuler installation utilisateur)
cp -R "/Volumes/ZenRunner 1.0.0/ZenRunner.app" /Applications/

# 4. Lancer depuis Finder
open /Applications/ZenRunner.app

# 5. Vérifier qu'il n'y a pas d'erreur dans Console.app

# 6. Nettoyage
rm -rf /Applications/ZenRunner.app
hdiutil detach "/Volumes/ZenRunner 1.0.0"
```

## 🔗 Ressources

- [Apple Documentation - Code Signing](https://developer.apple.com/documentation/xcode/code-signing-your-app)
- [Apple Documentation - Notarizing macOS Software](https://developer.apple.com/documentation/security/notarizing_macos_software_before_distribution)
- [Qt Documentation - macdeployqt](https://doc.qt.io/qt-6/macos-deployment.html)
- [Debugging dyld Issues](https://developer.apple.com/library/archive/technotes/tn2206/_index.html)

## 💡 Conseils

1. **Toujours tester sur une machine différente** de celle utilisée pour compiler
2. **Tester sur plusieurs versions de macOS** si possible (10.15, 11.0, 12.0+)
3. **Vérifier la taille du DMG** (~10-15 MB est normal, >50 MB peut indiquer un problème)
4. **Garder des logs** de chaque étape de compilation et packaging
5. **Utiliser un certificat Developer ID** pour la distribution publique

## 📞 Support

Si le problème persiste:

1. Exécuter tous les diagnostics ci-dessus
2. Sauvegarder les logs de Console.app
3. Ouvrir une issue sur GitHub avec:
   - Sortie de `codesign -dv --verbose=4 ZenRunner.app`
   - Sortie de `otool -L ZenRunner.app/Contents/MacOS/ZenRunner`
   - Sortie de `otool -l ZenRunner.app/Contents/MacOS/ZenRunner | grep -A2 LC_RPATH`
   - Logs pertinents de Console.app

---

**Dernière mise à jour**: Janvier 2026
