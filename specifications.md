# **Spécifications Techniques et Cahier des Charges du Projet ZenRunner : Excellence en Gestion de Processus Natifs**

L'industrie du développement logiciel traverse une phase de remise en question profonde concernant l'efficacité de ses outils de travail quotidiens. Alors que la puissance de calcul des machines modernes n'a jamais été aussi élevée, une tendance paradoxale s'est installée : l'utilisation massive de frameworks basés sur des moteurs de navigation web pour concevoir des applications de bureau. Cette approche, bien que facilitant le développement multiplateforme, impose une surcharge systémique souvent disproportionnée par rapport aux tâches accomplies. Le projet ZenRunner s'inscrit en faux contre cette tendance en proposant un gestionnaire de processus pour développeurs conçu selon un paradigme de performance radicale, s'appuyant sur des technologies natives pour garantir une efficacité maximale et une discrétion absolue.1

## **Vision Stratégique et Analyse du Marché**

La genèse de ZenRunner repose sur le constat que les outils actuels, principalement basés sur Electron, consomment des ressources mémoire et processeur excessives pour des fonctionnalités de base telles que l'exécution de scripts shell et la surveillance de logs. En encapsulant un moteur Chromium et une instance Node.js pour chaque application, ces outils créent une fragmentation de la RAM qui nuit à la stabilité globale des environnements de développement. L'objectif de ZenRunner est de restaurer la fluidité du flux de travail en proposant une alternative dont l'empreinte mémoire est inférieure à 30 Mo, tout en offrant une interface visuelle moderne et réactive.3

Cette vision s'aligne sur les standards de spécification logicielle les plus rigoureux, notamment ceux définis par la norme IEEE 830, qui préconise une définition claire des interfaces, des fonctions et des contraintes de performance pour assurer la viabilité d'un produit logiciel complexe.1 Le choix d'une stack technologique basée sur C++ 20 et Qt 6 n'est pas fortuit ; il permet une gestion granulaire de la mémoire et un accès direct aux API du système d'exploitation, conditions sine qua non pour atteindre les objectifs de performance fixés.6

## **Analyse des Objectifs de Performance et Contraintes Système**

La performance dans ZenRunner n'est pas perçue comme un simple avantage compétitif, mais comme une exigence fonctionnelle fondamentale de priorité P0. L'application doit rester virtuellement invisible pour le système, ne sollicitant les ressources que lors des interactions utilisateur ou de la réception de flux de données massifs en provenance des sous-processus.3

### **Optimisation de l'Empreinte Mémoire**

L'objectif de maintenir une consommation de RAM sous la barre des 30 Mo impose une architecture logicielle stricte. Contrairement aux applications web qui délèguent la gestion de la mémoire à un ramasse-miettes (Garbage Collector) imprévisible, ZenRunner utilise des techniques de gestion manuelle et semi-automatique via les pointeurs intelligents de C++ 20 et le système de parenté de Qt.6 La réduction de l'usage des variables globales et statiques, le choix de types de données compacts et l'utilisation de la pile (stack) pour les allocations temporaires sont des stratégies systématiquement appliquées.8

| Paramètre de Performance | Valeur Cible | Justification Technique |
| :---- | :---- | :---- |
| Consommation RAM au repos | \< 15 Mo | Allocation statique des structures de base et chargement paresseux des modules UI.3 |
| Consommation RAM en charge | \< 30 Mo | Utilisation de buffers circulaires pour les logs et limitation de la hiérarchie d'objets QML.4 |
| Taux de rafraîchissement UI | 60 FPS | Utilisation du Scene Graph de Qt Quick accéléré par le GPU (RHI).4 |
| Temps de réponse aux signaux | \< 10 ms | Communication asynchrone non-bloquante via le mécanisme des slots et signaux.7 |

### **Réactivité et Rendu Graphique**

Pour assurer une fluidité de 60 FPS, ZenRunner s'appuie sur le moteur de rendu de Qt 6, qui fait abstraction des API graphiques via le Rendering Hardware Interface (RHI). Cela permet à l'application d'utiliser Vulkan sur Linux, Metal sur macOS ou Direct3D sur Windows, garantissant que les opérations de dessin sont déportées vers le GPU, laissant le CPU libre pour la gestion des processus et le parsing des données.4 Cette approche évite le phénomène de "jank" (saccades) fréquent dans les interfaces web lors de la mise à jour dynamique de listes volumineuses.3

## **Architecture Logicielle et Choix Technologiques**

L'architecture de ZenRunner est conçue pour être modulaire, extensible et hautement performante. Elle se divise en plusieurs couches distinctes qui communiquent via des interfaces strictement définies, minimisant les dépendances croisées et facilitant les tests unitaires.1

### **Le Backend : C++ 20 et le Framework Qt 6**

Le choix du C++ 20 permet d'utiliser des fonctionnalités modernes comme les attributs \[\[likely\]\] et \[\[unlikely\]\] pour optimiser les branchements critiques, ainsi que les concepts pour garantir la robustesse des templates au moment de la compilation.14 Qt 6 apporte la gestion des événements, la couche réseau et surtout la classe QProcess, qui est le moteur central de ZenRunner pour l'exécution des scripts de développement.6

L'utilisation de QProcess permet une exécution asynchrone des commandes shell. ZenRunner ne bloque jamais le thread principal, même lors de l'exécution de scripts longs ou complexes. La capture des flux de sortie se fait par réaction aux signaux readyReadStandardOutput et readyReadStandardError, assurant une réactivité constante de l'interface.13

### **Le Frontend : QML et le Design Glassmorphism**

L'interface utilisateur est développée en QML (Qt Modeling Language), un langage déclaratif optimisé pour les interfaces tactiles et réactives. QML permet de séparer la logique de présentation de la logique métier codée en C++.3 Le design adopté, le Glassmorphism, repose sur des effets de transparence, de flou d'arrière-plan et de bordures subtiles pour créer une impression de profondeur et de hiérarchie visuelle sans alourdir l'interface avec des ombres portées complexes.19

Le Glassmorphism dans ZenRunner n'est pas qu'une question d'esthétique. En utilisant des matériaux natifs comme "Mica" sur Windows 11 et "Vibrancy" sur macOS, l'application réduit sa consommation d'énergie car ces effets sont gérés directement par le gestionnaire de fenêtres du système d'exploitation.21

| Composant Architectural | Technologie | Rôle |
| :---- | :---- | :---- |
| Noyau (Core) | C++ 20 / Qt Core | Gestion des données, parsing JSON, cycle de vie des processus.6 |
| Interface (UI) | QML / Qt Quick | Présentation visuelle, animations, gestion des événements utilisateur.3 |
| Système (Platform) | C++ / Native APIs | Intégration spécifique (Mica, Vibrancy, Notch, System Tray).25 |
| Stockage | QSettings / JSON | Persistance de l'état des projets et des préférences utilisateur.28 |

## **Spécifications Fonctionnelles : Gestion de Projets et Workspaces**

ZenRunner est conçu pour s'intégrer naturellement dans le flux de travail des développeurs. La gestion des projets doit être intuitive, automatisée et capable de gérer des architectures complexes comme les monorepos ou les micro-services.1

### **Importation et Auto-détection Intelligente**

Le processus commence par la sélection d'un dossier racine. Le backend C++ scanne alors récursivement (avec une profondeur limitée pour préserver les performances) la présence de fichiers package.json. Lorsqu'un tel fichier est détecté, QJsonDocument est utilisé pour extraire l'objet scripts.24

Cette détection automatique élimine la configuration manuelle fastidieuse. Chaque script trouvé (par exemple start, dev, test, lint) est transformé en un bouton d'action dynamique sur le tableau de bord. L'utilisateur a la possibilité de marquer certains scripts comme "épinglés", les rendant accessibles en un clic sans avoir à ouvrir le détail du projet.24

### **Organisation par Workspaces (Espaces de Travail)**

Pour les projets d'envergure impliquant plusieurs entités (Front-end, Back-end, Base de données, outils de build), ZenRunner propose la notion de Workspace. Un Workspace est un conteneur logique permettant de regrouper plusieurs projets indépendants.1

L'innovation réside dans les actions groupées. Un bouton unique permet de lancer l'ensemble des scripts de développement d'un Workspace. La logique interne gère le lancement séquentiel ou parallèle des processus via une file d'attente de QProcess, surveillant chaque état pour s'assurer que le démarrage groupé ne sature pas le CPU de la machine hôte.32

## **Console de Logs et Performance de Capture**

L'affichage des logs est l'une des fonctionnalités les plus gourmandes en ressources dans un gestionnaire de processus. Une mauvaise gestion peut entraîner une fuite mémoire continue à mesure que les logs s'accumulent.3

### **Buffer Circulaire et Gestion de la Mémoire**

Pour éviter l'inflation de la mémoire, ZenRunner implémente un buffer circulaire pour chaque projet. La taille est fixée à 5000 lignes par défaut. Lorsqu'un processus émet une nouvelle ligne de log et que le buffer est plein, la ligne la plus ancienne est automatiquement supprimée.11

Cette structure de données est gérée en C++ pour garantir une complexité temporelle de $O(1)$ pour l'insertion. Les données sont stockées de manière contiguë pour optimiser l'utilisation du cache processeur.11 Le modèle de données est ensuite exposé à QML via un QAbstractListModel, ce qui permet au ListView de l'interface de ne rendre que les lignes visibles à l'écran, économisant ainsi des cycles GPU précieux.18

### **Traitement des Couleurs ANSI et Formatage**

La plupart des outils de développement modernes produisent des sorties colorées via des codes d'échappement ANSI. ZenRunner intègre un parser léger capable d'interpréter ces codes pour colorer dynamiquement le texte dans la console de l'application.38 Cela permet aux développeurs de repérer immédiatement les erreurs (généralement en rouge) ou les avertissements (en jaune) au sein d'un flux de texte important.38

| Fonctionnalité des Logs | Implémentation Technique | Bénéfice Utilisateur |
| :---- | :---- | :---- |
| Capture Asynchrone | QProcess \+ Signaux/Slots.6 | Pas de gel de l'interface lors de logs massifs. |
| Rétention Limitée | Buffer Circulaire de 5000 lignes.11 | Stabilité mémoire sur le long terme. |
| Coloration Syntaxique | Parser de codes ANSI/VT100.38 | Lisibilité accrue des erreurs de compilation. |
| Recherche Rapide | Filtrage sur le modèle C++.18 | Identification instantanée des événements clés. |

## **Intégration Native et Expérience Utilisateur (UX)**

L'un des piliers de ZenRunner est son intégration profonde avec les systèmes d'exploitation Windows et macOS, surpassant les capacités des applications multiplateformes génériques.1

### **Support de Mica sur Windows 11**

Sur Windows 11, ZenRunner utilise l'API Desktop Window Manager (DWM) pour appliquer l'effet Mica. Ce matériau échantillonne le fond d'écran de l'utilisateur pour créer un arrière-plan translucide qui s'adapte aux changements de thème système.21 L'implémentation nécessite d'appeler DwmSetWindowAttribute avec l'attribut DWMWA\_SYSTEMBACKDROP\_TYPE sur le handle de fenêtre (HWND) géré par Qt.25

### **Vibrancy et Intégration Notch sur macOS**

Sur macOS, ZenRunner exploite la NSVisualEffectView pour obtenir l'effet de "vibrance" caractéristique du système d'Apple. En plus de cette esthétique, l'application propose une intégration unique avec l'encoche (Notch) des MacBook Pro récents.22

L'application peut détecter la présence de l'encoche via les safeAreaInsets fournies par AppKit. Cela permet de positionner un menu de contrôle rapide de type "Dynamic Island" qui s'aligne parfaitement sur la zone morte de l'écran, offrant un accès immédiat aux contrôles des Workspaces sans encombrer le bureau.27

### **Fonctionnement en Arrière-plan et System Tray**

ZenRunner est conçu pour être persistant mais discret. La fermeture de la fenêtre principale ne doit pas interrompre les serveurs de développement en cours. L'application se réduit dans la barre système (System Tray sur Windows, Menu Bar sur macOS).45

Une icône d'état indique par sa couleur si des processus sont actuellement actifs (Vert), en erreur (Rouge) ou si l'application est en veille (Gris). Un clic droit sur cette icône permet d'accéder aux fonctions rapides de démarrage/arrêt des Workspaces sans avoir à restaurer la fenêtre principale.45

## **Architecture du Code et Cycle de Vie**

La structure du projet suit les meilleures pratiques de développement C++, avec une séparation claire des responsabilités pour favoriser la maintenance et l'évolution future du logiciel.1

### **Arborescence du Projet**

L'organisation des fichiers reflète la modularité de l'architecture. Chaque répertoire contient des composants cohérents et isolés.1

* /src/core/ : Contient la logique de gestion des processus (ProcessManager), les modèles de données pour les projets et les Workspaces, ainsi que le moteur de parsing JSON.6  
* /src/ui/ : Regroupe les fichiers QML pour l'interface utilisateur, les shaders GLSL pour les effets visuels et les fichiers JavaScript pour la logique d'animation légère.3  
* /src/platform/ : Héberge le code spécifique à chaque OS, comme les wrappers Objective-C++ pour macOS et les utilitaires Win32 pour Windows.25  
* /include/ : Contient les headers publics et les définitions de types partagées entre les modules.

### **Persistance et État de l'Application**

La sauvegarde de l'état est cruciale pour que l'utilisateur retrouve son environnement de travail intact après un redémarrage. ZenRunner utilise QSettings pour stocker les préférences globales de l'application et les chemins des projets. Les états spécifiques aux projets (scripts épinglés, appartenance aux Workspaces) sont stockés dans un fichier JSON structuré dans le dossier des données utilisateur (AppData sur Windows, Application Support sur macOS).28

L'application enregistre périodiquement son état de manière atomique pour éviter toute corruption de données en cas d'arrêt brutal du système. Au démarrage, une routine de vérification s'assure de l'existence des dossiers de projets précédemment importés et met à jour l'interface en conséquence.28

## **Matrice de Priorité des Spécifications**

Pour guider le développement, les fonctionnalités sont classées selon leur criticité pour le succès du projet ZenRunner.1

| Fonctionnalité | Priorité | État | Justification |
| :---- | :---- | :---- | :---- |
| Exécution asynchrone via package.json | P0 (Critique) | Défini | Cœur métier de l'application.16 |
| Gestion des Workspaces | P0 (Critique) | Défini | Valeur ajoutée pour les projets complexes.1 |
| Buffer circulaire pour les logs | P1 (Haute) | Défini | Garant de la stabilité mémoire à long terme.11 |
| Intégration Tray et Réduction | P1 (Haute) | Défini | Respect du workflow "discret" de l'utilisateur.45 |
| Effets visuels (Mica/Vibrancy/Notch) | P1 (Haute) | Défini | Identité visuelle et intégration native.21 |
| Notifications de crash de processus | P2 (Moyenne) | Défini | Confort d'utilisation et réactivité.47 |
| Statistiques d'usage CPU/RAM par projet | P2 (Moyenne) | À définir | Optimisation avancée pour l'utilisateur. |

## **Gestion des Erreurs et Fiabilité**

ZenRunner doit être plus stable que les processus qu'il surveille. La fiabilité est assurée par une gestion rigoureuse des exceptions et des signaux système.6

### **Surveillance des Crashs et Redémarrage Automatique**

Chaque QProcess est surveillé par un gestionnaire d'état. Si un processus se termine avec un code de sortie non nul ou suite à un signal de crash (CrashExit), ZenRunner notifie immédiatement l'utilisateur via une notification système discrète.6 L'interface met à jour l'indicateur d'état du projet en rouge, et propose une option de redémarrage rapide.

Une attention particulière est portée à la terminaison des processus. Lors de la fermeture de ZenRunner, l'application tente de terminer proprement tous les processus enfants via SIGTERM. Si certains processus ne répondent pas après un délai de grâce, un signal SIGKILL est envoyé pour s'assurer qu'aucun processus "zombie" ne reste actif en mémoire.53

### **Robustesse face aux I/O et au Parsing**

Le module de parsing JSON utilise des mécanismes de validation pour éviter les crashs lors de la lecture de fichiers malformés. En cas d'erreur de lecture d'un package.json, ZenRunner n'ignore pas le projet mais affiche un message d'erreur explicatif, permettant au développeur de corriger son fichier sans quitter l'outil.24

La capture des logs est également protégée contre les flux excessifs (log flood). Si un processus génère des données plus vite que l'interface ne peut les afficher, un mécanisme de "throttle" (limitation) intervient pour regrouper les mises à jour UI et préserver la fluidité globale.3

## **Conclusion et Perspectives**

ZenRunner se positionne comme un outil de rupture dans l'écosystème des développeurs. En combinant la puissance brute du C++ 20, la flexibilité de Qt 6 et une esthétique native moderne, il répond à une demande croissante pour des outils plus respectueux des ressources matérielles.1

L'architecture retenue garantit non seulement le respect des objectifs de performance initiaux (\< 30 Mo de RAM), mais pose également les bases d'une plateforme évolutive. Les futures versions pourraient intégrer la surveillance de conteneurs Docker via les API natives, ou encore proposer une intégration poussée avec des outils de monitoring distants, tout en conservant cette philosophie de légèreté et d'efficacité native qui définit ZenRunner.1

En conclusion, ZenRunner n'est pas simplement un gestionnaire de processus supplémentaire ; c'est un plaidoyer technique pour le retour aux applications natives de haute qualité, prouvant que l'élégance visuelle et la performance extrême peuvent coexister harmonieusement au service de la productivité.1

#### **Sources des citations**

1. Srs Template | PDF | Use Case | Specification (Technical Standard) \- Scribd, consulté le janvier 5, 2026, [https://www.scribd.com/document/610990330/Srs-Template](https://www.scribd.com/document/610990330/Srs-Template)  
2. IEEE Standard for Software Requirements Specifications (IEEE 830–1998) | by Abdul Rehman | Medium, consulté le janvier 5, 2026, [https://medium.com/@abdul.rehman\_84899/ieee-standard-for-software-requirements-specifications-ieee-830-1998-0395f1da639a](https://medium.com/@abdul.rehman_84899/ieee-standard-for-software-requirements-specifications-ieee-830-1998-0395f1da639a)  
3. Performance considerations and suggestions | Qt Quick | Qt 6.10.1, consulté le janvier 5, 2026, [https://doc.qt.io/qt-6/qtquick-performance.html](https://doc.qt.io/qt-6/qtquick-performance.html)  
4. Qt Quick / QML performance optimisation dos and dont's \- Spyrosoft, consulté le janvier 5, 2026, [https://spyro-soft.com/expert-hub/qt-quick-qml-performance-optimisation](https://spyro-soft.com/expert-hub/qt-quick-qml-performance-optimisation)  
5. 830-1984 \- IEEE Guide for Software Requirements Specifications, consulté le janvier 5, 2026, [https://ieeexplore.ieee.org/document/278253](https://ieeexplore.ieee.org/document/278253)  
6. QProcess Class | Qt Core | Qt 6.10.1, consulté le janvier 5, 2026, [https://doc.qt.io/qt-6/qprocess.html](https://doc.qt.io/qt-6/qprocess.html)  
7. Asynchronous APIs in Qt 6 | Multithreaded Programming, consulté le janvier 5, 2026, [https://www.qt.io/blog/asynchronous-apis-in-qt-6](https://www.qt.io/blog/asynchronous-apis-in-qt-6)  
8. Optimizing RAM Usage in Embedded Systems with C/C++ \- Please Don't Code Blog, consulté le janvier 5, 2026, [https://www.pleasedontcode.com/blog/optimizing-ram-usage-in-embedded-systems-with-cc](https://www.pleasedontcode.com/blog/optimizing-ram-usage-in-embedded-systems-with-cc)  
9. what are the best practices for optimizing c++ applications on ubuntu considering memory management and system architecture, consulté le janvier 5, 2026, [https://askubuntu.com/questions/1539178/what-are-the-best-practices-for-optimizing-c-applications-on-ubuntu-considerin](https://askubuntu.com/questions/1539178/what-are-the-best-practices-for-optimizing-c-applications-on-ubuntu-considerin)  
10. QProcess doesn't kill/terminate the process if be defined on heap \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/32645706/qprocess-doesnt-kill-terminate-the-process-if-be-defined-on-heap](https://stackoverflow.com/questions/32645706/qprocess-doesnt-kill-terminate-the-process-if-be-defined-on-heap)  
11. When to Consider Using a Circular Buffer: A Comprehensive Guide \- AlgoCademy Blog, consulté le janvier 5, 2026, [https://algocademy.com/blog/when-to-consider-using-a-circular-buffer-a-comprehensive-guide/](https://algocademy.com/blog/when-to-consider-using-a-circular-buffer-a-comprehensive-guide/)  
12. Integrating Custom Rendering Engine with QML Quick | \#QtWS22, consulté le janvier 5, 2026, [https://www.qt.io/development/resources/videos/integrating-custom-rendering-engine-with-qml-quick](https://www.qt.io/development/resources/videos/integrating-custom-rendering-engine-with-qml-quick)  
13. Using QProcess to read standard output \- c++ \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/49058051/using-qprocess-to-read-standard-output](https://stackoverflow.com/questions/49058051/using-qprocess-to-read-standard-output)  
14. Advanced C++ Optimization Techniques for High-Performance Applications — Part 1 | by Martin Ayvazyan | Medium, consulté le janvier 5, 2026, [https://medium.com/@martin00001313/advanced-c-optimization-techniques-for-high-performance-applications-part-1-55aba61b4dad](https://medium.com/@martin00001313/advanced-c-optimization-techniques-for-high-performance-applications-part-1-55aba61b4dad)  
15. Qt 4.4: QProcess Class Reference, consulté le janvier 5, 2026, [https://radekp.github.io/qtmoko/api/qprocess.html](https://radekp.github.io/qtmoko/api/qprocess.html)  
16. Using QProcess to Run External Programs in PySide6 \- Python GUIs, consulté le janvier 5, 2026, [https://www.pythonguis.com/tutorials/pyside6-qprocess-external-programs/](https://www.pythonguis.com/tutorials/pyside6-qprocess-external-programs/)  
17. Thread: QProcess and capturing output \- Qt Centre, consulté le janvier 5, 2026, [https://www.qtcentre.org/threads/6722-QProcess-and-capturing-output](https://www.qtcentre.org/threads/6722-QProcess-and-capturing-output)  
18. Using QAbstractListModel With QML ListView in Qt6 \- Akshay Raj Gollahalli, consulté le janvier 5, 2026, [https://www.gollahalli.com/blog/using-qabstractlistmodel-with-qml-listview-in-qt6/](https://www.gollahalli.com/blog/using-qabstractlistmodel-with-qml-listview-in-qt6/)  
19. What is Glassmorphism: Principles, Practices & Examples | Ramotion Agency, consulté le janvier 5, 2026, [https://www.ramotion.com/blog/what-is-glassmorphism/](https://www.ramotion.com/blog/what-is-glassmorphism/)  
20. 12 Glassmorphism UI Features, Best Practices, and Examples \- UX Pilot, consulté le janvier 5, 2026, [https://uxpilot.ai/blogs/glassmorphism-ui](https://uxpilot.ai/blogs/glassmorphism-ui)  
21. Mica material \- Windows apps | Microsoft Learn, consulté le janvier 5, 2026, [https://learn.microsoft.com/en-us/windows/apps/design/style/mica](https://learn.microsoft.com/en-us/windows/apps/design/style/mica)  
22. UIVibrancyEffect | Apple Developer Documentation, consulté le janvier 5, 2026, [https://developer.apple.com/documentation/uikit/uivibrancyeffect](https://developer.apple.com/documentation/uikit/uivibrancyeffect)  
23. QtLiquidGlass – Real macOS glass effects for Qt 6 (using native ..., consulté le janvier 5, 2026, [https://www.reddit.com/r/QtFramework/comments/1p6y3m3/qtliquidglass\_real\_macos\_glass\_effects\_for\_qt\_6/](https://www.reddit.com/r/QtFramework/comments/1p6y3m3/qtliquidglass_real_macos_glass_effects_for_qt_6/)  
24. Parse nested JSON with QJsonDocument in Qt \- c++ \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/39524151/parse-nested-json-with-qjsondocument-in-qt](https://stackoverflow.com/questions/39524151/parse-nested-json-with-qjsondocument-in-qt)  
25. Phantom white box whilst using Application Window and Windows 11 Mica background on QML, qt 6.10.1 \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/79846711/phantom-white-box-whilst-using-application-window-and-windows-11-mica-background](https://stackoverflow.com/questions/79846711/phantom-white-box-whilst-using-application-window-and-windows-11-mica-background)  
26. Use Mica material in Win32 apps \- Windows \- Microsoft Learn, consulté le janvier 5, 2026, [https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/ui/apply-mica-win32](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/ui/apply-mica-win32)  
27. Integrate MacOS notch in window design : IJPL-124561 | \- JetBrains YouTrack, consulté le janvier 5, 2026, [https://youtrack.jetbrains.com/projects/IJPL/issues/IJPL-124561/Integrate-MacOS-notch-in-window-design](https://youtrack.jetbrains.com/projects/IJPL/issues/IJPL-124561/Integrate-MacOS-notch-in-window-design)  
28. MoooDob/QSettingsJson: Json Formatter for QSettings \- GitHub, consulté le janvier 5, 2026, [https://github.com/MoooDob/QSettingsJson](https://github.com/MoooDob/QSettingsJson)  
29. Save UI settings with QSettings or QJson? \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/42060573/save-ui-settings-with-qsettings-or-qjson](https://stackoverflow.com/questions/42060573/save-ui-settings-with-qsettings-or-qjson)  
30. Qt parsing JSON using QJsonDocument, QJsonObject, QJsonArray \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/19822211/qt-parsing-json-using-qjsondocument-qjsonobject-qjsonarray](https://stackoverflow.com/questions/19822211/qt-parsing-json-using-qjsondocument-qjsonobject-qjsonarray)  
31. Thread: How to parse Json array give below \- Qt Centre, consulté le janvier 5, 2026, [https://www.qtcentre.org/threads/64464-How-to-parse-Json-array-give-below](https://www.qtcentre.org/threads/64464-How-to-parse-Json-array-give-below)  
32. QProcess \- how to run multiple processes in a loop \- openSUSE Forums, consulté le janvier 5, 2026, [https://forums.opensuse.org/t/qprocess-how-to-run-multiple-processes-in-a-loop/46352](https://forums.opensuse.org/t/qprocess-how-to-run-multiple-processes-in-a-loop/46352)  
33. Running multiple QProcess | Qt Forum, consulté le janvier 5, 2026, [https://forum.qt.io/topic/53288/running-multiple-qprocess](https://forum.qt.io/topic/53288/running-multiple-qprocess)  
34. \[SOLVED\] Monitoring multiple processes using QProcess \- Qt Forum, consulté le janvier 5, 2026, [https://forum.qt.io/topic/4871/solved-monitoring-multiple-processes-using-qprocess](https://forum.qt.io/topic/4871/solved-monitoring-multiple-processes-using-qprocess)  
35. Circular Buffer Performance Trick \- Cybernetist, consulté le janvier 5, 2026, [https://cybernetist.com/2024/04/11/circular-buffer-performance-trick/](https://cybernetist.com/2024/04/11/circular-buffer-performance-trick/)  
36. Using a QAbstractListModel in QML \- DMC, Inc., consulté le janvier 5, 2026, [https://www.dmcinfo.com/blog/17671/using-a-qabstractlistmodel-in-qml/](https://www.dmcinfo.com/blog/17671/using-a-qabstractlistmodel-in-qml/)  
37. Circular buffer model | Qt Forum, consulté le janvier 5, 2026, [https://forum.qt.io/topic/20660/circular-buffer-model](https://forum.qt.io/topic/20660/circular-buffer-model)  
38. Keep ANSI Escape Sequences in QProcess Output \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/46371370/keep-ansi-escape-sequences-in-qprocess-output](https://stackoverflow.com/questions/46371370/keep-ansi-escape-sequences-in-qprocess-output)  
39. Parsing ANSI escape codes? \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/4097311/parsing-ansi-escape-codes](https://stackoverflow.com/questions/4097311/parsing-ansi-escape-codes)  
40. \[feature request\] ANSI escape sequences in Viewer \- Total Commander \- ghisler.ch, consulté le janvier 5, 2026, [https://www.ghisler.ch/board/viewtopic.php?t=85463](https://www.ghisler.ch/board/viewtopic.php?t=85463)  
41. Get Mica or Mica Alt color in Windows 11 \- AutoIt General Help and Support, consulté le janvier 5, 2026, [https://www.autoitscript.com/forum/topic/212369-get-mica-or-mica-alt-color-in-windows-11/](https://www.autoitscript.com/forum/topic/212369-get-mica-or-mica-alt-color-in-windows-11/)  
42. deprecated functions \- Microsoft Q\&A, consulté le janvier 5, 2026, [https://learn.microsoft.com/en-us/answers/questions/4375268/deprecated-functions](https://learn.microsoft.com/en-us/answers/questions/4375268/deprecated-functions)  
43. safeAreaInsets | Apple Developer Documentation, consulté le janvier 5, 2026, [https://developer.apple.com/documentation/appkit/nsscreen/safeareainsets](https://developer.apple.com/documentation/appkit/nsscreen/safeareainsets)  
44. \[Feature Request\]: Add MacBook notch area detection · Issue \#31478 \- GitHub, consulté le janvier 5, 2026, [https://github.com/electron/electron/issues/31478](https://github.com/electron/electron/issues/31478)  
45. System tray & Mac menu bar applications in PySide6 \- Python GUIs, consulté le janvier 5, 2026, [https://www.pythonguis.com/tutorials/pyside6-system-tray-mac-menu-bar-applications/](https://www.pythonguis.com/tutorials/pyside6-system-tray-mac-menu-bar-applications/)  
46. Run application in background and show icon in menubar and pop up menu in finder(mac os) | Qt Forum, consulté le janvier 5, 2026, [https://forum.qt.io/topic/3343/run-application-in-background-and-show-icon-in-menubar-and-pop-up-menu-in-finder-mac-os](https://forum.qt.io/topic/3343/run-application-in-background-and-show-icon-in-menubar-and-pop-up-menu-in-finder-mac-os)  
47. System Tray Icon Example | Qt Widgets | Qt 6.10.1, consulté le janvier 5, 2026, [https://doc.qt.io/qt-6/qtwidgets-desktop-systray-example.html](https://doc.qt.io/qt-6/qtwidgets-desktop-systray-example.html)  
48. QSystemTrayIcon Class | Qt Widgets | Qt 6.10.1, consulté le janvier 5, 2026, [https://doc.qt.io/qt-6/qsystemtrayicon.html](https://doc.qt.io/qt-6/qsystemtrayicon.html)  
49. Thread: How to display Systemtray message \- Qt Centre, consulté le janvier 5, 2026, [https://www.qtcentre.org/threads/28947-How-to-display-Systemtray-message](https://www.qtcentre.org/threads/28947-How-to-display-Systemtray-message)  
50. What is the best approach to save and restore widgets states? : r/QtFramework \- Reddit, consulté le janvier 5, 2026, [https://www.reddit.com/r/QtFramework/comments/i014yy/what\_is\_the\_best\_approach\_to\_save\_and\_restore/](https://www.reddit.com/r/QtFramework/comments/i014yy/what_is_the_best_approach_to_save_and_restore/)  
51. Save Programm Settings internally \+ how to organize with folders? \- Qt Forum, consulté le janvier 5, 2026, [https://forum.qt.io/topic/155108/save-programm-settings-internally-how-to-organize-with-folders](https://forum.qt.io/topic/155108/save-programm-settings-internally-how-to-organize-with-folders)  
52. What is the best approach to save and restore widgets states? \- Qt Forum, consulté le janvier 5, 2026, [https://forum.qt.io/topic/117487/what-is-the-best-approach-to-save-and-restore-widgets-states](https://forum.qt.io/topic/117487/what-is-the-best-approach-to-save-and-restore-widgets-states)  
53. terminate() or QProcess::close() \- Qt Forum, consulté le janvier 5, 2026, [https://forum.qt.io/topic/162600/qprocess-terminate-or-qprocess-close](https://forum.qt.io/topic/162600/qprocess-terminate-or-qprocess-close)  
54. How can processes detach when I try to kill them? \- Super User, consulté le janvier 5, 2026, [https://superuser.com/questions/990349/how-can-processes-detach-when-i-try-to-kill-them](https://superuser.com/questions/990349/how-can-processes-detach-when-i-try-to-kill-them)  
55. Terminating QProcess doesn't kill child process \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/52931974/terminating-qprocess-doesnt-kill-child-process](https://stackoverflow.com/questions/52931974/terminating-qprocess-doesnt-kill-child-process)  
56. Viewing data in a circular buffer in real-time \- Stack Overflow, consulté le janvier 5, 2026, [https://stackoverflow.com/questions/1606952/viewing-data-in-a-circular-buffer-in-real-time](https://stackoverflow.com/questions/1606952/viewing-data-in-a-circular-buffer-in-real-time)