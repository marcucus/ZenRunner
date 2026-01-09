# Interface Unifiée ZenRunner

## Vue d'ensemble

La nouvelle interface de ZenRunner a été restructurée pour offrir une expérience utilisateur plus intuitive et efficace, avec une vue unifiée des projets et des workspaces.

## Structure de l'Interface

### 1. Écran Principal (UnifiedListView)

L'écran principal contient deux sections principales :

#### A. Liste Unifiée
- **Workspaces** (affichés en premier, en haut de la liste)
  - Icône distinctive 🗂️
  - Nom et description
  - Badge indiquant le nombre de projets
  - Couleur de fond violette pour différenciation

- **Projets** (affichés en dessous des workspaces)
  - Icône distinctive 📦
  - Nom et chemin du projet
  - Badge indiquant le nombre de scripts
  - Couleur de fond bleue pour différenciation

#### B. Boutons d'Action (en haut à droite)
- **📁 Scan Folder** : Scanner un dossier pour détecter des projets
- **+ Add Project** : Ajouter manuellement un projet
- **+ Add Workspace** : Créer un nouveau workspace

#### C. Panneau de Statistiques (StatisticsPanel)
Situé en dessous de la liste, ce panneau compact affiche :
- **Total CPU** : Utilisation CPU totale avec barre de progression
- **Total Memory** : Utilisation mémoire totale avec barre de progression
- **Active Processes** : Nombre de processus en cours d'exécution
- **Application Status** : État de l'application (Running/Idle)

### 2. Vue Détaillée (DetailView)

Lorsqu'on clique sur un projet ou un workspace, on accède à une vue détaillée :

#### A. Header
- **Bouton Retour** (← Back) : Retour à la liste principale
- **Icône et Nom** : Affichage de l'élément sélectionné
- **Description/Path** : Informations supplémentaires

#### B. Section Projets et Commandes
- **Pour un Workspace** :
  - Boutons horizontaux pour chaque projet du workspace
  - Cliquer sur un bouton projet affiche ses scripts
  - Les scripts du projet sélectionné s'affichent dans une grille

- **Pour un Projet Simple** :
  - Grille de tous les scripts disponibles
  - Boutons cliquables pour lancer chaque script

#### C. Vue Multi-Terminaux (MultiTerminalView)
- **Affichage des sorties** : Console-style avec fond noir et texte vert
- **Tabs pour Workspaces** : Si c'est un workspace avec plusieurs projets
  - Chaque projet a son propre tab
  - Cliquer sur un tab affiche le terminal correspondant
- **Indicateur de processus actifs** : Badge montrant le nombre de processus en cours
- **Bouton Clear** : Nettoyer la sortie du terminal
- **Barre d'info** : Affiche l'heure et des conseils d'utilisation

## Fichiers QML

### Nouveaux Composants
1. **Main.qml** (modifié)
   - Gestion du StackView pour naviguer entre vues
   - État de navigation (liste principale ↔ vue détaillée)

2. **UnifiedListView.qml**
   - Liste unifiée workspaces + projets
   - Boutons d'action (Scan, Add Project, Add Workspace)
   - Intégration du panneau de statistiques

3. **StatisticsPanel.qml**
   - Affichage compact des statistiques
   - 4 métriques principales en grille
   - Barres de progression pour CPU et Memory

4. **DetailView.qml**
   - Vue détaillée d'un projet ou workspace
   - Gestion des projets multiples dans un workspace
   - Lancement de scripts avec tracking des processus

5. **MultiTerminalView.qml**
   - Affichage des sorties de processus
   - Système de tabs pour workspaces multi-projets
   - Écoute des signaux du ProcessManager

## Flux de Navigation

```
┌─────────────────────────────────────┐
│       Écran Principal               │
│                                     │
│  ┌───────────────────────────────┐ │
│  │   Boutons Actions             │ │
│  │   [Scan] [+Project] [+WS]    │ │
│  └───────────────────────────────┘ │
│                                     │
│  ┌───────────────────────────────┐ │
│  │   Liste Unifiée               │ │
│  │   🗂️ Workspaces (en haut)    │ │
│  │   📦 Projets                  │ │◄──┐
│  └───────────────────────────────┘ │  │
│                                     │  │
│  ┌───────────────────────────────┐ │  │ Click
│  │   Statistiques                │ │  │ "Retour"
│  │   CPU | Memory | Processes    │ │  │
│  └───────────────────────────────┘ │  │
└─────────────────────────────────────┘  │
            │                            │
            │ Click sur                  │
            │ Item                       │
            ▼                            │
┌─────────────────────────────────────┐  │
│       Vue Détaillée                 │  │
│                                     │  │
│  [← Back]  🗂️/📦 Nom               │  │
│                                     │  │
│  ┌───────────────────────────────┐ │  │
│  │   Projets & Scripts           │ │  │
│  │   [Projet1] [Projet2] ...     │ │  │
│  │   [Script1] [Script2] ...     │ │  │
│  └───────────────────────────────┘ │  │
│                                     │  │
│  ┌───────────────────────────────┐ │  │
│  │   Multi-Terminal View         │ │  │
│  │   [Tab1] [Tab2] [Clear]       │ │  │
│  │   ┌───────────────────────┐   │ │  │
│  │   │ Terminal Output       │   │ │  │
│  │   │ $ command...          │   │ │  │
│  │   └───────────────────────┘   │ │  │
│  └───────────────────────────────┘ │  │
└─────────────────────────────────────┘  │
                                         │
                                         └──┘
```

## Modifications du Code C++

### WorkspaceViewModel.h/cpp
- Ajout de la méthode `Q_INVOKABLE QVariantList getWorkspaceProjects(int index)`
- Permet de récupérer les projets d'un workspace depuis QML

### main.cpp
- Ajout de l'include `#include "ui/StatisticsViewModel.h"`
- Création et initialisation du `StatisticsViewModel`
- Exposition du `statisticsViewModel` au contexte QML

## Utilisation

### Créer un Workspace
1. Cliquer sur **+ Add Workspace**
2. Saisir le nom et la description
3. Le workspace apparaît en haut de la liste

### Ajouter des Projets
1. **Scan Folder** : Scanner un dossier automatiquement
2. **Add Project** : Ajouter manuellement un chemin

### Lancer des Scripts
1. Cliquer sur un projet ou workspace dans la liste
2. Sélectionner un projet (si workspace)
3. Cliquer sur un script pour le lancer
4. Observer la sortie dans le terminal en dessous

### Gérer les Terminaux (Workspaces)
1. Ouvrir un workspace avec plusieurs projets
2. Utiliser les tabs en haut du terminal
3. Chaque tab correspond à un projet
4. Le terminal affiche uniquement les sorties du projet sélectionné

## Améliorations Futures

- [ ] Enrichir `getWorkspaceProjects()` pour récupérer les vraies données depuis le repository
- [ ] Ajouter un système de filtrage/recherche dans la liste
- [ ] Permettre le réordonnancement des projets par drag & drop
- [ ] Ajouter des raccourcis clavier pour la navigation
- [ ] Sauvegarder l'état de navigation entre les sessions
- [ ] Permettre d'avoir plusieurs terminaux visibles simultanément (split view)
