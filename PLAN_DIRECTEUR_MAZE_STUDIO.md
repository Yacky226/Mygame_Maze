# Maze Studio — Plan directeur produit et technique

**Date de référence : 22 octobre 2026.**  
**Statut : proposition de conception pour les prochaines versions.**  
**Périmètre : évolution de Mygame vers un atelier de recherche de chemin, de simulation et de création de défis.**

Ce document décrit la cible à construire. Les fonctionnalités annoncées comme futures ne sont pas déjà présentes dans le code. Les budgets de performance sont des objectifs de validation, pas des résultats garantis. Les noms de commandes, dossiers et interfaces de la cible sont des propositions ; ils deviennent exécutables seulement après leur implémentation.

Le nom **Maze Studio** est un nom de travail. La décision structurante proposée est de conserver un cœur algorithmique C++ et de développer une interface web native, au lieu de faire de la transmission d'écran noVNC l'interface définitive du produit.

## Sommaire

1. [Vision et principes](#vision)
2. [Diagnostic du projet existant](#diagnostic)
3. [Décisions technologiques](#technologies)
4. [Architecture cible et organisation du dépôt](#architecture)
5. [Modèle métier et règles mathématiques](#modele)
6. [Catalogue des modes](#modes)
7. [Catalogue des algorithmes](#algorithmes)
8. [Éditeur de cartes et génération procédurale](#editeur)
9. [Direction graphique et expérience utilisateur](#graphismes)
10. [Moteur de calcul, Workers et WebAssembly](#execution)
11. [Animation, traces, pause et relecture](#relecture)
12. [Comparaison et expérimentation scientifique](#mesures)
13. [Simulation, agents et règles de jeu](#simulation)
14. [Sauvegardes, formats et échanges](#donnees)
15. [Performances et gestion des ressources](#performances)
16. [Tests et critères de qualité](#qualite)
17. [Outils, développement et intégration continue](#outillage)
18. [Docker, distribution et fonctionnement hors ligne](#distribution)
19. [Services en ligne, partage et collaboration](#services)
20. [Accessibilité et internationalisation](#accessibilite)
21. [Extensibilité et ajout d'un algorithme](#extensions)
22. [Migration de Mygame](#migration)
23. [Versions, lots et dépendances](#versions)
24. [Backlog détaillé et critères d'acceptation](#backlog)
25. [Risques, arbitrages et points à mesurer](#risques)
26. [Définition d'une version livrable](#livraison)
27. [Parcours complets de référence](#parcours)
28. [Décisions et premières actions](#actions)
29. [Glossaire](#glossaire)
30. [Sources techniques et scientifiques](#sources)

<a id="vision"></a>

## 1. Vision et principes

### 1.1 Produit visé

Construire un outil qui permette de **dessiner un environnement, choisir les règles de déplacement, comprendre un algorithme, comparer des solutions, jouer un défi et reproduire une expérience**.

L'intérêt du produit doit venir de l'association entre une expérience visuelle claire et des résultats algorithmiques fiables. Un grand nombre de boutons ne suffit pas : chaque mode doit posséder un objectif identifiable, une configuration cohérente, des résultats interprétables et des limites explicites.

### 1.2 Utilisateurs et usages

| Utilisateur | Besoin principal | Exemple d'utilisation |
|---|---|---|
| Étudiant | Comprendre pourquoi un algorithme explore certaines cases | Exécuter A* pas à pas en observant `g`, `h`, `f` et les parents |
| Enseignant | Préparer des exercices et des démonstrations reproductibles | Distribuer une carte et une consigne, puis comparer les réponses |
| Développeur | Vérifier et mesurer une implémentation | Lancer un corpus de scénarios depuis une CLI sans animation |
| Créateur de niveaux | Construire et équilibrer un labyrinthe | Ajuster les impasses, la distance optimale et les zones difficiles |
| Joueur | Résoudre des défis et observer des comportements intelligents | Trouver une sortie avec des clés et éviter des ennemis |
| Utilisateur avancé | Étudier des environnements dynamiques ou plusieurs agents | Fermer un passage et comparer recalcul complet et replanification |

### 1.3 Principes de conception

1. **Exactitude avant classement.** Une mesure rapide d'un résultat incorrect ne constitue pas une performance.
2. **Séparation du calcul et du dessin.** Le choix d'un thème graphique ne modifie jamais le coût d'un chemin.
3. **Reproductibilité.** Même carte, mêmes règles, même version et même graine doivent permettre de reproduire une recherche déterministe.
4. **Réactivité.** Une recherche coûteuse ne doit pas bloquer les commandes, la sauvegarde ni l'annulation.
5. **Progressivité.** Les fonctions avancées apparaissent dans des espaces dédiés et disposent de préréglages compréhensibles.
6. **Conception modulaire.** Plusieurs modes réutilisent le même éditeur, le même moteur et les mêmes formats.
7. **Gestion des ressources.** Mémoire, traces et concurrence possèdent des limites mesurées ; les grandes cartes ne sont pas acceptées sans vérification.
8. **Portabilité.** Le cœur fonctionne sans SFML, sans navigateur et sans serveur.
9. **Fonctionnement local.** Les usages fondamentaux ne nécessitent ni compte ni connexion permanente.
10. **Évolution maîtrisée.** Les contrats, les versions de formats et les migrations sont documentés.

### 1.4 Limites de la première grande version

La première version complète vise la 2D sur grille rectangulaire, un agent par recherche, un éditeur utilisable, des comparaisons fiables et des défis locaux. Le multi-agent exact, les graphes libres, la collaboration en ligne et les extensions externes arrivent ensuite.

La 3D, l'apprentissage par renforcement, la génération par modèle d'IA et la simulation physique réaliste constituent des pistes de recherche séparées. Leur ajout doit répondre à un usage démontré. Elles ne sont pas nécessaires pour obtenir un outil puissant de recherche de chemin.

<a id="diagnostic"></a>

## 2. Diagnostic du projet existant

### 2.1 Ce qui existe réellement

- Application C++ avec SFML 2.6, solution Visual Studio et construction CMake ajoutée.
- Grille de 30 × 50 cases, fenêtre de 1300 × 800 pixels.
- Départ, arrivée, murs, carte aléatoire et effacement de la carte.
- BFS, DFS, Dijkstra et A*.
- Visualisation des cases explorées et du chemin trouvé.
- Exécution Docker avec Xvfb, x11vnc et noVNC, accessible dans un navigateur.
- Optimisation récente : dessin de la grille regroupé, absence de redessin permanent au repos, progression des animations par lots.

### 2.2 Écarts à résoudre avant une extension importante

| Zone du code | Constat | Conséquence | Correction attendue |
|---|---|---|---|
| `Mygame/Cell.h` | Données métier, état de recherche, position et forme SFML réunis | Tests et réutilisation Web difficiles | Extraire carte, état de recherche et rendu |
| `Mygame/Pathfinding.cpp` | A* utilise les positions en pixels pour l'heuristique et un coût de déplacement égal à 1 | Heuristique mal dimensionnée ; optimalité non assurée | Calculer l'heuristique dans le modèle de coûts du graphe |
| `Mygame/Pathfinding.cpp` | A* marque les voisins visités dès leur découverte et n'autorise plus une amélioration | Des solutions moins coûteuses peuvent être écartées | Distinguer ouvert/fermé, relaxation et réouverture selon les garanties |
| `Mygame/Pathfinding.cpp` | Dijkstra finalise implicitement des voisins lors de leur découverte | Généralisation aux terrains pondérés incorrecte | Finaliser à l'extraction de la meilleure entrée, ignorer les entrées périmées |
| Recherches successives | `visited`, distances et parents restent associés aux cases | Résultats dépendants de la recherche précédente | Un contexte indépendant par exécution |
| `Mygame/Button.cpp` | Le texte d'un bouton décide de la logique et lance calcul + animation | Couplage et duplication entre algorithmes | Commandes identifiées, contrôleur et registre d'algorithmes |
| `MainController` | Boutons et polices alloués avec `new` sans propriétaire clair | Gestion de durée de vie fragile | Valeurs ou RAII avec `unique_ptr` si allocation nécessaire |
| Départ et arrivée | Copies de cellules distinctes de la grille et remise à zéro partielle | État incohérent entre interface, grille et recherche | Identifiants de nœuds stables associés à une révision de carte |
| Animation | Boucles synchrones ; fermeture traitée, autres actions indisponibles pendant l'animation | Pause et édition contrôlée difficiles | Exécution incrémentale et contrôleur de lecture indépendants |
| Carte aléatoire | Graine dépendante de l'heure et absence d'identifiant de génération | Expériences difficiles à reproduire | Générateur versionné avec graine enregistrée |
| Construction Windows | Chemins locaux absolus et bibliothèques embarquées | Installation fragile ailleurs | Dépendances déclarées et préréglages CMake portables |
| Sources | Encodage historique Windows-1252 | Frictions avec outils modernes et commentaires | Migration UTF-8 contrôlée dans un lot distinct |
| Validation | Vérifications de lancement réalisées, absence de suite métier versionnée | Régressions algorithmiques non détectées automatiquement | Corpus de tests de référence et tests natif/WebAssembly |

**Nuance importante :** la grille actuelle utilise des déplacements orthogonaux de coût uniforme. Certaines simplifications actuelles de Dijkstra peuvent fonctionner dans ce cadre. Cela ne valide pas une implémentation générale de Dijkstra pondéré.

### 2.3 Interprétation des mesures déjà observées

La mesure locale avant optimisation indiquait environ 267 % CPU pour le conteneur, puis environ 1 % au repos après modification du rendu. Une recherche BFS traversant la grille avec animation a été observée autour de 4,8 secondes. Ces observations proviennent de contrôles locaux courts, sans protocole matériel complet. Elles servent à identifier un coût de rendu ; elles ne constituent pas un benchmark publié ni une mesure du seul calcul BFS.

### 2.4 Ce qu'il faut conserver

Conserver les comportements utiles, les exemples de cartes, le savoir acquis sur BFS/DFS/Dijkstra/A*, les documents pédagogiques et un exécutable de transition. Les implémentations sont reprises après tests et correction, sans considérer leur résultat actuel comme une vérité de référence.

<a id="technologies"></a>

## 3. Décisions technologiques

### 3.1 Architecture recommandée

**Cœur C++20 indépendant + interface React/TypeScript + rendu 2D PixiJS + WebAssembly dans un Web Worker + CLI native pour les expériences.**

Ce choix répond aux besoins exprimés : interface riche, modes nombreux, graphismes interchangeables, accès navigateur et conservation d'un véritable moteur algorithmique performant. Il implique une adaptation progressive de l'affichage et de l'architecture. L'application SFML ne se transforme pas automatiquement en application web en changeant un fichier de compilation.

Les Workers permettent de déporter le travail hors du fil principal de l'interface ; WebAssembly permet de compiler un cœur C++ portable. Leur association ne garantit pas automatiquement un gain de vitesse : l'algorithme, les structures de données et le coût des échanges restent à mesurer. [Workers](https://developer.mozilla.org/en-US/docs/Web/API/Web_Workers_API/Using_web_workers), [Emscripten](https://emscripten.org/docs/getting_started/downloads.html).

### 3.2 Socle technique retenu

| Besoin | Choix proposé | Pourquoi | Moment d'introduction |
|---|---|---|---|
| Algorithmes et simulation | C++20, bibliothèque `maze_core` | Préserver le C++, maîtriser mémoire et structures, partager un même moteur | Extraction du cœur |
| Construction native | CMake, Ninja, CMake Presets | Construction Windows/Linux et CI avec options explicites | Fondation |
| Dépendances C++ | vcpkg en mode manifeste, baseline verrouillée | Déclarer et reproduire les dépendances natives | Fondation |
| Compilation navigateur | Emscripten avec SDK stable fixé | Produire un module WebAssembly depuis le cœur | Premier parcours web |
| Interface des panneaux | React, TypeScript strict, CSS avec variables de thème | Contrôles, formulaires, tableaux, accessibilité et composition | Premier parcours web |
| Construction web | Vite, pnpm avec lockfile, Node.js LTS supporté | Développement rapide et artefacts statiques | Premier parcours web |
| Affichage de la carte | PixiJS 8, WebGL en cible de référence | Dessin de nombreuses cases, textures, caméra et couches | Premier parcours web |
| Communication de calcul | Web Worker dédié + Comlink pour les commandes | Contrat asynchrone ; transferts binaires séparés pour gros volumes | Premier parcours web |
| État de l'interface | Zustand avec sélecteurs ciblés | Partager configuration et état de session sans mettre toutes les cases dans React | Dès que l'état dépasse un composant local |
| Validation de fichiers | JSON Schema 2020-12 + Ajv configuré pour ce dialecte | Formats versionnés et validation à l'import | Persistance |
| Stockage local | IndexedDB derrière un adaptateur | Cartes, scénarios, préférences et résultats sans serveur | Persistance |
| Tests C++ | Catch2 + CTest | Régressions, cas limites et vérification des propriétés | Fondation |
| Microbenchmarks natifs | Google Benchmark | Mesurer les noyaux sans interface | Comparateur sérieux |
| Tests TypeScript | Vitest + Testing Library pour les composants | Contrats, commandes, contrôles et migrations | Premier parcours web |
| Tests de parcours | Playwright | Dessin, exécution, annulation, sauvegarde et plusieurs navigateurs | Premier parcours web |
| Validation visuelle de composants | Storybook | Cataloguer et revoir les composants et états | Quand le système d'interface se stabilise |
| Qualité C++ | clang-format, clang-tidy, sanitizers | Conventions et détection d'erreurs mémoire | Fondation |
| Qualité web | ESLint, Prettier, vérification TypeScript | Contrats et conventions | Premier parcours web |
| Livraison locale et CI | Docker multi-stage + Compose | Construire, tester et servir l'application | Transition puis cible web |
| Serveur web de livraison | Nginx avec configuration versionnée | Servir JS, textures et `.wasm` | Première livraison web |
| Services en ligne éventuels | Fastify/TypeScript + PostgreSQL supporté | Partage et comptes, seulement quand ces besoins existent | Version communautaire |

Le cœur initial évite les dépendances lourdes. Catch2 et Google Benchmark appartiennent aux cibles de test et de mesure. Une bibliothèque de lecture JSON native, par exemple nlohmann/json, appartient à l'adaptateur CLI/persistance plutôt qu'au modèle de graphe.

vcpkg fournit un mode manifeste et des mécanismes de versionnement ; le projet devra enregistrer une baseline réelle, sans utiliser un hash fictif. [Documentation vcpkg](https://learn.microsoft.com/en-us/vcpkg/concepts/manifest-mode).

### 3.3 Politique de versions

- Choisir une release stable pour chaque outil au début du lot d'implémentation ; vérifier les compatibilités réelles avant de figer le lockfile.
- Fixer les versions exactes dans le lockfile web, la baseline vcpkg, le SDK Emscripten et les images Docker de livraison.
- C++20 est un choix de langage ; valider le même sous-ensemble utilisé avec MSVC, Clang et Emscripten. Ne pas supposer que toute la bibliothèque standard récente est disponible partout.
- React 19.x et PixiJS 8 sont les familles proposées ; aucune dépendance au numéro mineur le plus récent n'est nécessaire pour le modèle métier.
- Prendre une version LTS Node supportée compatible avec Vite retenu ; Node 24 LTS est une base à vérifier lors du verrouillage. [Calendrier Node](https://nodejs.org/en/about/previous-releases), [prérequis Vite](https://vite.dev/guide/).
- Définir une mise à jour régulière et tester les changements majeurs dans un lot dédié.

### 3.4 Alternatives étudiées et décision

| Alternative | Atouts pour ce projet | Coût ou limite | Décision |
|---|---|---|---|
| Continuer uniquement avec SFML | Préserve presque toute l'interface actuelle ; bon support 2D natif | Création manuelle d'une interface riche ; noVNC ajoute une transmission d'écran | Adaptateur de transition, pas interface principale cible |
| Migrer SFML 2 vers SFML 3 | API modernisée et base native renouvelée | Migration réelle ; ne résout pas à elle seule le produit navigateur | À considérer seulement si l'adaptateur natif devient un livrable maintenu |
| Qt/QML | Bon candidat pour un outil de bureau avec panneaux et formulaires | Autre pile d'interface et de distribution ; besoins de licence à examiner selon les modules | Alternative si la priorité devient exclusivement bureau |
| Godot | Éditeur de scènes, outils de jeu, niveaux et animations | Adaptation importante du projet et intégration du cœur ; contraintes d'export à valider | Alternative si la campagne et le jeu deviennent le produit principal |
| Tout réécrire en TypeScript | Une seule langue pour l'application navigateur | Reprise du cœur et perte de partage natif si deux versions sont ensuite maintenues | Option de repli si le coût WebAssembly n'est pas justifié par le prototype |
| React + moteur 2D + cœur C++/WASM | Interface riche et moteur commun navigateur/CLI | Deux langages et une frontière à concevoir et tester | **Option retenue pour la cible** |
| Moteur 3D complet | Autres expériences possibles | Caméra, assets, navigation et tests d'une autre nature | Exploration séparée après les versions 2D |

Les changements de SFML 3 sont documentés dans le [guide de migration officiel](https://www.sfml-dev.org/tutorials/3.0/getting-started/migrate/). Les caractéristiques de Godot et les contraintes de son export Web doivent être vérifiées sur sa [documentation stable](https://docs.godotengine.org/en/stable/tutorials/export/exporting_for_web.html).

### 3.5 Porte de décision avant migration complète

Construire un prototype vertical limité : une carte, BFS corrigé, un Worker WebAssembly, le dessin PixiJS, un bouton Annuler et l'affichage du coût. Le comparer au même moteur compilé en CLI.

Le choix est confirmé si le prototype conserve les résultats, reste réactif, possède une construction reproductible et montre un coût de frontière raisonnable. Sinon, corriger la frontière ou reconsidérer l'option TypeScript avant d'ajouter tous les modes. Aucun gain chiffré WebAssembly ne doit être promis avant cette mesure.

### 3.6 Outils exclus du socle initial

Ne pas ajouter Kubernetes, des microservices, Redis, une authentification, un moteur physique, un ECS généraliste ou un système d'extensions arbitraires pour les besoins actuels. Un dépôt modulaire et une application locale suffisent pour la première cible. Ajouter un outil seulement lorsqu'une fonction livrée en a besoin et qu'un responsable de maintenance est identifié.

<a id="architecture"></a>

## 4. Architecture cible et organisation du dépôt

### 4.1 Frontières et responsabilités

| Module | Responsabilités | Dépendances interdites |
|---|---|---|
| `maze_core` | Graphes, règles, recherches, générateurs, planification discrète | SFML, React, DOM, HTTP, fichiers système dans les algorithmes |
| `maze_cli` | Charger des cas, lancer les campagnes, exporter les résultats | Fenêtre ou besoin de navigateur |
| `maze_wasm` | Convertir les entrées, gérer les sessions et exposer l'API du cœur | Composants React et objets PixiJS |
| `engine-worker` | Ordonnancer les étapes, annuler, transférer les lots, surveiller les limites | Accès direct au DOM |
| `application` | Commandes, sessions, révisions, compatibilités et orchestration des modes | Détails OpenGL et logique d'optimalité |
| `renderer` | Caméra, textures, couches, sélection et rendu des états | Décider d'un chemin ou modifier les coûts du modèle |
| `ui` | Formulaires, barres d'outils, résultats, raccourcis, dialogues | Modifier directement les tableaux internes d'une recherche |
| `persistence` | Validation, migrations et stockage | Lancer une recherche comme effet secondaire d'un chargement |
| `contracts` | Messages, formats, capacités et versions de protocole | Accès aux bibliothèques graphiques |

Le cœur définit les règles. Les adaptateurs transforment les entrées/sorties. L'interface exprime une intention comme `RunSearch`, `PaintTerrain` ou `LoadScenario`, pas une succession de mutations internes.

### 4.2 Flux normal d'une recherche

1. L'utilisateur édite une carte ; chaque transaction valide produit une nouvelle révision.
2. Le contrôleur valide l'algorithme et les paramètres contre les capacités du problème.
3. Il fige un instantané de carte et crée un `runId` associé à `mapRevision`, `mapHash` et `rulesHash`.
4. Le Worker charge ou réutilise cet instantané et crée un contexte de recherche indépendant.
5. Le cœur avance par étapes bornées ; le Worker transmet des lots d'événements et des métriques.
6. Le contrôleur de lecture anime les événements à la vitesse choisie.
7. Le résultat final est validé puis associé à l'instantané qui l'a produit.
8. Une réponse portant un ancien `runId` est ignorée et sa mémoire est libérée.

Une modification de carte ne doit jamais altérer silencieusement une recherche déjà lancée. La politique normale est d'annuler la recherche, conserver éventuellement son résultat comme historique, puis démarrer une nouvelle exécution. Le mode dynamique constitue un cas explicite avec événements de monde et algorithmes compatibles.

### 4.3 Organisation proposée

```text
Mygame/                         # existant, conservé pendant la transition
core/
  include/maze/                 # API publique indépendante du rendu
  src/model/
  src/search/
  src/generation/
  src/simulation/
  tests/
  benchmarks/
apps/
  cli/
  web/
    src/app/
    src/features/editor/
    src/features/laboratory/
    src/features/comparison/
    src/features/challenges/
    src/features/simulation/
    src/renderer/
    src/workers/
    src/storage/
    src/ui/
bindings/wasm/
packages/contracts/
schemas/                       # contrats JSON versionnés
assets/                        # ressources possédées ou redistribuables
scenarios/                     # cas de référence et scénarios pédagogiques
tools/                         # validation, migration, jeux de mesures
docs/adr/                      # décisions d'architecture
docs/algorithms/               # fiches, hypothèses et exemples
docker/                        # profils legacy, build, tests et web
CMakePresets.json
vcpkg.json
pnpm-workspace.yaml
```

Cette arborescence est une cible, pas une demande de déplacer immédiatement tous les fichiers. Les déplacements accompagnent l'extraction d'une responsabilité testée.

### 4.4 Méthodes de développement

- Concevoir par **parcours verticaux** : un usage fonctionne de bout en bout avant d'ouvrir dix chantiers incomplets.
- Utiliser des interfaces à l'endroit des variations réelles : algorithmes, générateurs, persistance, rendu. Éviter une interface pour chaque petite classe.
- Enregistrer les changements de carte sous forme de commandes réversibles. Un trait de pinceau est une transaction, pas mille entrées d'historique.
- Centraliser les capacités : mêmes restrictions dans le formulaire, la validation Worker et le cœur.
- Préférer des identifiants et des tableaux contigus aux pointeurs vers des objets de rendu.
- Garder un moteur déterministe pour les résultats ; mesurer les durées séparément.
- Documenter les décisions avec contexte, choix, alternative écartée, coût, critère de réexamen et statut.
- Exiger un test démontrant le problème corrigé pour une correction algorithmique, sans multiplier les tests qui recopient l'implémentation.

### 4.5 Registre d'algorithmes

Chaque algorithme possède un identifiant stable, un nom traduit, sa version, ses paramètres, ses capacités et sa fiche explicative.

Le registre doit notamment indiquer : topologies acceptées, coûts uniformes ou pondérés, graphe dirigé, diagonales autorisées, besoin de ligne de vue, support incrémental, niveau de trace, support multi-agent et garanties annoncées.

Exemple de comportement : si JPS est sélectionné puis un terrain pondéré est ajouté, le panneau explique l'incompatibilité et propose A* ou Dijkstra. Il ne lance pas JPS avec une option ignorée.

<a id="modele"></a>

## 5. Modèle métier et règles mathématiques

### 5.1 Entités principales

| Entité | Champs structurants | Rôle |
|---|---|---|
| `Project` | identifiant, nom, version de format, cartes, scénarios, métadonnées | Unité de travail sauvegardable |
| `GridMap` | largeur, hauteur, obstacles, identifiants de terrain, révision | Définition durable de l'environnement |
| `TerrainCatalog` | identifiant, libellé, multiplicateur, apparence | Séparer règles physiques et thème graphique |
| `MovementRules` | voisinage, diagonales, coûts de base, politique de coins | Définition exacte des arêtes |
| `Scenario` | carte/révision, agents, objectifs, événements, graine | Problème concret à résoudre |
| `SearchConfig` | algorithme, heuristique, paramètres, limites, niveau de trace | Configuration d'une exécution |
| `SearchContext` | ouvert, fermé, distances, parents, compteurs | État temporaire privé d'une exécution |
| `SearchResult` | statut, chemin, coût, métriques, hypothèses, identité du moteur | Résultat vérifiable |
| `Trace` | événements, checkpoints, niveau de détail, version | Support de l'animation et de la relecture |
| `Experiment` | scénarios, configurations, répétitions, ordre et environnement | Comparaison reproductible |
| `Challenge` | règles, ressources, victoire, score et solution de validation | Défi jouable et vérifiable |

Un terrain décrit une propriété de la carte. La couleur « orange exploré » est un état de visualisation de recherche. Le départ et l'arrivée sont attachés au scénario. Aucun de ces concepts ne doit remplacer les autres dans une unique énumération `CellType`.

### 5.2 Identifiants et stockage mémoire

- Nœud d'une grille : `NodeId` entier 32 bits, avec `id = y * width + x`.
- Convention unique : `x` est la colonne, `y` la ligne. Convertir explicitement l'ancien usage `indexX`/`indexY` lors de la migration.
- Vérifier le produit `width * height` avec un type assez large avant allocation ; ne pas se fier au débordement d'un entier 32 bits.
- Obstacles : octets ou bitset selon mesure. Terrains : tableau compact d'identifiants.
- Distances : entiers 64 bits avec une valeur `INF` réservée et additions contrôlées.
- Parents : `NodeId` et sentinelle `InvalidNode`, sans pointeur vers une `Cell` SFML.
- États ouvert/fermé : tableau compact ou marquage par génération ; gérer explicitement le retour à zéro du compteur de génération.
- Files prioritaires : entrées contenant une clé figée et un identifiant ; éviter une comparaison de pointeurs dont la distance change après insertion.
- Les données de rendu sont des caches dérivés et reconstructibles.

### 5.3 Coûts et heuristiques de référence

Pour les grilles pondérées, retenir des coûts entiers documentés : déplacement orthogonal `D = 1000`, diagonal `D2 = 1414`, multipliés par un facteur de terrain entier positif de 1 à 1000. Le coût d'une arête est celui du déplacement multiplié par le facteur de la case d'arrivée.

`1414` est une approximation discrète de `1000 × sqrt(2)`. L'optimalité annoncée porte sur ce modèle entier, pas sur une longueur euclidienne continue exacte. La longueur géométrique peut être affichée séparément.

| Règles | Heuristique admissible de référence |
|---|---|
| 4 voisins, coût de base `D` | Manhattan : `D × (abs(dx) + abs(dy))` |
| 8 voisins, toutes les transitions de même coût `D` | Chebyshev : `D × max(abs(dx), abs(dy))` |
| 8 voisins, orthogonal `D` et diagonal `D2` | Octile : `D2 × min(abs(dx),abs(dy)) + D × abs(abs(dx)-abs(dy))` |
| Terrains à multiplicateurs positifs | Multiplier la borne géométrique par le plus petit multiplicateur applicable |
| Graphe quelconque sans borne connue | Heuristique nulle : comportement de Dijkstra |
| Téléporteurs ou arêtes spéciales | Recalculer une borne valide ; sinon heuristique nulle |

Ne pas employer Manhattan avec des diagonales moins coûteuses que deux pas orthogonaux. Une valeur de `h` séduisante visuellement mais surestimée doit être présentée comme expérimentation sans garantie d'optimalité.

Le modèle « payer le terrain d'arrivée » produit des coûts dirigés lorsque les terrains diffèrent. Les recherches bidirectionnelles doivent explorer le graphe transposé avec le coût original de l'arête inversée. Elles ne peuvent pas réutiliser naïvement la même formule de coût d'entrée dans les deux sens.

### 5.4 Diagonales, coins et taille d'agent

- La règle par défaut interdit de traverser diagonalement le coin de deux obstacles et exige les deux cases orthogonales adjacentes libres.
- Si une règle plus permissive existe, son nom, sa définition et sa prise en charge par l'algorithme sont enregistrés dans le scénario.
- Un agent de rayon non nul nécessite un calcul de dégagement ou une carte d'obstacles élargis. Dessiner un personnage plus large ne modifie pas implicitement la collision.
- Pour les déplacements à angle libre, une ligne de vue doit vérifier les mêmes obstacles, coins et marges que le mouvement réel.
- Les algorithmes sont comparés avec la même politique de déplacement.

### 5.5 États de résultat

Prévoir des statuts distincts : `Running`, `Success`, `NoPath`, `Cancelled`, `Timeout`, `ResourceLimit`, `InvalidRequest`, `UnsupportedConfiguration`, `InternalError`.

`NoPath` signifie que la recherche complète a démontré l'absence de solution dans le modèle concerné. Un dépassement de temps ou de mémoire ne prouve pas cette absence. Un algorithme incomplet ne doit pas présenter son échec comme une preuve générale.

Un résultat réussi doit préciser au moins : premier et dernier nœuds, nombre d'arêtes, coût total, statut de garantie, compteurs de recherche, version du moteur et configuration complète.

### 5.6 Invariants obligatoires

1. Un mur est infranchissable ; ses coordonnées et son identité ne changent pas pendant une recherche figée.
2. Les extrémités sont dans les bornes et praticables. Une extrémité invalide produit `InvalidRequest`.
3. Départ égal à arrivée produit un chemin d'un nœud, zéro arête et coût nul.
4. Chaque paire successive du chemin correspond à une transition autorisée par les règles.
5. Le coût retourné égale la somme recalculée des arêtes, avec gestion du débordement.
6. Une chaîne de parents ne contient pas de cycle ; la reconstruction est bornée par le nombre d'états.
7. Une exécution n'altère ni la carte ni un autre contexte.
8. Un résultat précise la révision de son instantané ; il ne s'applique pas à une carte modifiée.
9. Les métriques utilisent les mêmes définitions pour tous les algorithmes.
10. Le thème graphique et la vitesse de lecture ne modifient pas le résultat mathématique.

### 5.7 Déterminisme

Fixer l'ordre des voisins et une règle de départage des égalités, par exemple `(f, h, ordreInsertion)`. Enregistrer cette règle ; ne pas laisser l'adresse mémoire ou l'ordre d'une table de hachage décider d'un chemin.

Utiliser un générateur pseudo-aléatoire explicitement versionné, par exemple PCG32, avec état initial et sélection de flux enregistrés. Employer une méthode bornée spécifiée plutôt qu'une distribution dont les détails varient entre bibliothèques standard. La référence PCG fournit un générateur et une fonction de tirage borné. [Référence PCG](https://www.pcg-random.org/using-pcg-c-basic.html).

L'identité d'une expérience contient la version du générateur et ses paramètres en plus de la graine. Les durées CPU, l'ordonnancement du navigateur et le débit réseau restent variables ; ils ne font pas partie de la garantie de déterminisme du chemin.

<a id="modes"></a>

## 6. Catalogue des modes

### 6.1 Organisation commune

Les modes partagent une carte, un scénario, un inspecteur de paramètres et une zone de résultats. Changer de mode ne crée pas une nouvelle copie non synchronisée de tout le projet. Les sessions historiques restent associées à leur instantané.

| Identifiant | Mode | Rôle principal | Livraison proposée |
|---|---|---|---|
| M01 | Bac à sable | Explorer librement cartes et recherches | Première version web |
| M02 | Laboratoire pas à pas | Comprendre le fonctionnement interne | Première version complète |
| M03 | Comparaison visuelle | Observer plusieurs recherches côte à côte | Première version complète |
| M04 | Banc d'essai | Mesurer sur un ensemble de scénarios | Première version complète, puis enrichissement |
| M05 | Studio de cartes | Créer, éditer, valider et sauvegarder | Première version web puis enrichissement |
| M06 | Génération procédurale | Produire des cartes reproductibles et contrôlées | Version création |
| M07 | Campagne de labyrinthes | Jouer des niveaux avec progression | Version défis |
| M08 | Défi contre un agent | Comparer une stratégie humaine et une stratégie calculée | Version défis |
| M09 | Programmation d'agent | Résoudre des niveaux avec un petit langage d'instructions | Version défis avancés |
| M10 | Monde dynamique | Observer modifications et replanification | Version simulation |
| M11 | Planification multi-agent | Résoudre des conflits de trajectoires | Version simulation avancée |
| M12 | Champs de direction | Déplacer plusieurs agents vers une destination commune | Version simulation |
| M13 | Graphes et autres grilles | Étudier des graphes libres et une topologie hexagonale | Extension du modèle |
| M14 | Relecture et carnet d'expériences | Expliquer et conserver un résultat | Dès les premières traces puis enrichissement |
| M15 | Cours et exercices | Construire des parcours pédagogiques | Version pédagogie |
| M16 | Atelier partagé | Partager ou coéditer un projet | Version communautaire facultative |

### 6.2 M01 — Bac à sable

**Objectif :** permettre l'exploration immédiate sans imposer de cours ou de score.

- Choisir dimensions, règle de déplacement, terrain, départ et arrivée.
- Dessiner des obstacles, déplacer les extrémités et lancer une recherche.
- Afficher chemin, coût, distance géométrique, cases découvertes et cases développées.
- Permettre réexécuter, vider les résultats ou vider la carte avec des commandes distinctes.
- Enregistrer un scénario et réutiliser sa configuration dans un autre mode.
- Expliquer les réglages incompatibles avant le lancement.

**Acceptation :** dix recherches successives sur une même carte produisent un résultat indépendant des recherches précédentes ; vider les résultats conserve la carte et les extrémités ; annuler rend immédiatement l'éditeur disponible dans le budget de réactivité défini.

### 6.3 M02 — Laboratoire pas à pas

**Objectif :** rendre les décisions de l'algorithme inspectables.

- Commandes Play, Pause, Étape suivante, retour via relecture et remise au début.
- Affichage du nœud courant, de la frontière, des nœuds fermés, des parents et du chemin provisoire.
- Inspection de `g`, `h`, `f`, coût d'entrée et raison d'une relaxation.
- Vue de la file ou de la pile avec limite de lignes, filtre et recherche ; éviter de monter un tableau de centaines de milliers de lignes DOM.
- Pseudo-code associé aux types d'événements, avec distinction entre instruction pédagogique et opération réellement comptée.
- Points d'arrêt : découverte d'un nœud, fermeture d'une case, taille de frontière ou premier objectif trouvé.
- Comparaison d'heuristiques dans un cadre qui conserve les règles du problème.

**Acceptation :** une étape possède une définition stable ; avancer puis revenir à la même position de trace restitue le même état ; les valeurs affichées correspondent au moteur et non à une approximation recalculée par l'interface.

### 6.4 M03 — Comparaison visuelle

**Objectif :** comprendre les différences de stratégie sur exactement le même problème.

- Deux panneaux par défaut ; quatre possibles dans les limites du profil matériel.
- Partager l'instantané, les règles, les extrémités et la caméra si l'utilisateur le choisit.
- Offrir deux synchronisations explicitement nommées : « même nombre d'opérations » et « lecture indépendante ».
- Superposer éventuellement les chemins sur une carte commune avec une légende lisible.
- Afficher garanties, coût, expansions et mémoire logique, sans déclarer gagnant celui dont l'animation finit le premier.
- Détecter les configurations où l'objectif mathématique diffère : terrain pondéré, angle libre, diagonales ou états supplémentaires.

**Acceptation :** un changement de carte invalide tous les panneaux concernés ; la comparaison de temps est issue d'exécutions mesurées séparément ; la vitesse de lecture ne change aucun classement calculé.

### 6.5 M04 — Banc d'essai

**Objectif :** conduire des expériences exportables et reproductibles.

- Définir corpus de cartes, paires départ/arrivée, paramètres et nombre de répétitions.
- Prévoir un échauffement, un ordre équilibré ou aléatoire enregistré et une exécution séquentielle par défaut.
- Produire CSV et JSON, distributions, médiane, percentiles et taux d'atteinte des limites.
- Associer chaque résultat à la version du moteur, au commit, au compilateur et à l'environnement.
- Fournir une CLI native pour les campagnes lourdes et un mode navigateur pour les petites expériences.
- Séparer recherche sans trace, recherche instrumentée et mesure de lecture.
- Importer ultérieurement des jeux de cas Moving AI en conservant leurs conventions et leurs droits de redistribution.

**Acceptation :** une campagne exportée peut être relancée ; un timeout est compté comme tel ; les résultats incorrects sont identifiés avant toute agrégation de vitesse.

### 6.6 M05 — Studio de cartes

**Objectif :** fournir un véritable éditeur, utilisable pour tous les autres modes.

- Pinceau, gomme, trait, rectangle, remplissage et sélection de région.
- Copier/coller, déplacer, dupliquer et appliquer une symétrie.
- Calques logiques : obstacles, terrains, marqueurs, événements et décor ; le décor seul n'affecte pas le calcul.
- Undo/redo transactionnel et historique borné en mémoire.
- Palette de terrains, métadonnées, auteur et description.
- Vue d'ensemble, zoom, déplacement de caméra et recentrage.
- Validation de connectivité et affichage d'erreurs sans détruire la carte.

**Acceptation :** annuler un trait restitue toute la région ; une carte sauvegardée puis rechargée garde la même empreinte sémantique ; un décor ne transforme pas accidentellement une case praticable en mur.

### 6.7 M06 — Génération procédurale

**Objectif :** créer des familles de problèmes variées et contrôlées.

- Labyrinthes par DFS aléatoire, Prim aléatoire, Kruskal, division récursive puis Wilson pour une variante plus avancée.
- Cartes de salles et couloirs, obstacles répartis, terrains pondérés et cavernes par automate cellulaire.
- Enregistrer graine, version, paramètres et transformations après génération.
- Paramétrer taille, densité, présence de cycles, nombre de salles et sélection des extrémités selon le générateur.
- Distinguer les paramètres désirés des propriétés réellement mesurées.
- Proposer un mode « solution requise » qui vérifie et rejette ou répare explicitement une carte non résoluble.
- Prévisualiser puis appliquer en une transaction annulable.

**Acceptation :** même configuration et même version donnent la même carte ; une réparation est annoncée et enregistrée ; l'outil ne promet pas une densité et une difficulté exactes si le générateur ne les contrôle pas.

### 6.8 M07 — Campagne de labyrinthes

**Objectif :** transformer les cartes en niveaux jouables.

- Déplacements clavier et commandes adaptées au tactile ; collision issue du même modèle que le solveur.
- Objectifs : sortie, points à visiter, ressources limitées puis clés et portes.
- Progression locale, niveaux débloqués, reprises et statistiques par niveau.
- Tutoriel intégré et thèmes graphiques cohérents.
- Difficulté réglée par plusieurs propriétés, pas seulement par le nombre de murs.
- Solution de référence validée pour chaque niveau distribué.

**Acceptation :** victoire et défaite sont déterministes à partir d'une séquence d'actions ; recommencer réinitialise tout l'état du niveau ; les règles des portes sont prises en charge par le solveur réellement utilisé.

### 6.9 M08 — Défi contre un agent

**Objectif :** rendre la recherche de chemin tangible et ludique.

- Comparer coût de trajet, nombre de mouvements, ressources consommées et objectifs atteints.
- Offrir un adversaire visible ou révéler son parcours après l'essai humain.
- Pour une course animée, imposer aux deux agents une vitesse de déplacement définie ; le temps CPU du solveur n'est pas le temps de déplacement du personnage.
- Prévoir handicap, limite de connaissance ou brouillard d'exploration pour des défis plus riches.
- Présenter les conditions exactes du score et les aides utilisées.

**Acceptation :** le résultat ne dépend pas du débit d'images ; le classement distingue parcours assisté et non assisté ; aucune comparaison « humain contre calcul CPU » n'est présentée comme une mesure équitable de vitesse de jeu.

### 6.10 M09 — Programmation d'agent

**Objectif :** résoudre un problème par une stratégie exprimée avec des instructions.

- Petit langage décrit par une grammaire : avancer, tourner, répéter un nombre borné de fois, tester un obstacle ou une case.
- Éditeur de blocs ou de texte simple, erreurs de syntaxe localisées et simulation pas à pas.
- Budget d'instructions et limite d'itérations ; arrêt propre d'un programme sans fin.
- Enregistrement du programme avec le niveau et sa version d'interpréteur.
- Évolution ultérieure vers des stratégies de recherche dans un environnement d'extension séparé.

**Acceptation :** un programme invalide n'est pas lancé ; le même programme sur le même niveau produit les mêmes actions ; le langage n'utilise pas `eval` ni l'accès libre aux API du navigateur.

### 6.11 M10 — Monde dynamique

**Objectif :** comparer une recherche initiale puis les réactions aux changements.

- Programmer fermetures de portes, nouveaux murs et modifications de coût sur des ticks de simulation.
- Comparer recalcul A* complet, LPA* puis D* Lite selon le scénario pris en charge.
- Distinguer temps de calcul initial, temps de réparation et coût du trajet effectivement suivi.
- Visualiser carte connue et carte réelle si l'agent ne connaît pas tout l'environnement.
- Éviter de faire suivre un chemin invalidé sans vérifier le prochain mouvement.

**Acceptation :** chaque modification possède un tick et une révision ; la relecture reproduit les événements ; les coûts de réparation sont comparés pour la même série de changements.

### 6.12 M11 — Planification multi-agent

**Objectif :** résoudre des interactions, pas simplement lancer plusieurs A* indépendants.

- Plusieurs agents, départs et destinations, avec mêmes règles de temps et d'occupation.
- Afficher conflits de sommet et échanges d'arêtes en sens opposés.
- Définir l'action d'attente et l'occupation de la destination après arrivée.
- Commencer par planification priorisée avec réservations spatio-temporelles ; documenter son incomplétude et sa dépendance à l'ordre.
- Ajouter CBS exact pour petits cas, avec limites explicites et objectifs coût total ou makespan.
- Afficher impasse, budget dépassé et conflit résiduel comme des résultats distincts.

**Acceptation :** un validateur indépendant détecte tout conflit ; un cas non résolu dans le budget n'est pas annoncé sans solution ; les limites du nombre d'agents sont mesurées par famille de cartes.

### 6.13 M12 — Champs de direction

**Objectif :** étudier un grand nombre d'agents ayant un but partagé.

- Construire un champ de distances depuis la destination, sur les arêtes inversées quand les coûts sont dirigés.
- Déduire des directions à partir des coûts et distances ; animer des agents avec un pas de simulation fixe.
- Afficher potentiel, directions, zones inaccessibles et points de congestion.
- Mesurer temps de construction du champ, mises à jour et nombre d'agents simulés.
- Séparer ce mode d'un solveur MAPF : un champ ne garantit pas des trajectoires individuelles sans collision.

**Acceptation :** tout mouvement choisi descend le coût potentiel selon les règles, sauf comportement local documenté ; les conflits de mouvement ne sont pas masqués par les effets visuels.

### 6.14 M13 — Graphes et autres grilles

**Objectif :** étendre le laboratoire au-delà du damier rectangulaire.

- Graphe explicite avec nœuds déplaçables visuellement, arêtes dirigées ou non et poids.
- Représentation hexagonale avec coordonnées adaptées et voisins définis par la topologie.
- Algorithmes activés uniquement si leurs capacités couvrent la structure choisie.
- Une position visuelle ne devient une heuristique admissible que si elle fournit réellement une borne sur les coûts.
- Panneaux et résultats communs, éditeur de topologie spécialisé.

**Acceptation :** déplacer un nœud pour la lisibilité ne change pas ses poids ; une heuristique géométrique non justifiée est désactivée ; les imports enregistrent la topologie.

### 6.15 M14 — Relecture et carnet d'expériences

**Objectif :** conserver le raisonnement et les conditions d'un résultat.

- Bibliothèque d'exécutions, annotations, signets et comparaison d'un avant/après.
- Navigation dans la trace et reproduction d'un état à une étape donnée.
- Capture PNG de la carte et rapport HTML autonome avec métriques et configuration ; vidéo facultative ensuite.
- Affichage du niveau de trace et des informations qui n'ont pas été enregistrées.
- Regroupement d'exécutions dans un carnet avec description de l'hypothèse testée.

**Acceptation :** un rapport identifie sa carte, ses règles et son moteur ; une trace tronquée ou agrégée ne prétend pas permettre une relecture exacte de chaque opération.

### 6.16 M15 — Cours et exercices

**Objectif :** construire une progression pédagogique sur les fonctions existantes.

- Leçons courtes, scénarios associés, questions sur la frontière, le coût ou l'heuristique.
- Exercices de prédiction de la prochaine étape et de choix d'algorithme.
- Mode enseignant local : créer une fiche, fixer certains paramètres et exporter un paquet d'exercice.
- Correction fondée sur les règles et critères de l'exercice, sans exiger le même chemin lorsqu'il existe plusieurs solutions optimales.
- Progression sauvegardée localement ; synchronisation de classe seulement avec un service prévu à cet effet.

**Acceptation :** un exercice reste utilisable hors ligne après installation des ressources nécessaires ; les questions annoncent les conventions de départage lorsqu'elles demandent une étape précise.

### 6.17 M16 — Atelier partagé

**Objectif :** partager des projets, puis éventuellement les éditer à plusieurs.

- Première étape : export/import de fichiers et partage d'instantanés via un service facultatif.
- Deuxième étape : projets privés/publics, droits de lecture/édition et historique de versions.
- Coédition seulement après choix explicite du modèle de concurrence et du traitement des conflits.
- Les résultats de recherche restent rattachés à un instantané, même si la carte partagée évolue.
- Prévoir reconnexion, édition hors ligne et conflits visibles sans écrasement silencieux.

**Acceptation :** un utilisateur sans droit ne peut ni lire un projet privé ni le modifier ; une reconnexion conserve les modifications selon une règle documentée. Ce mode dépend des services décrits en section 19 et n'entre pas dans la première version complète.

<a id="algorithmes"></a>

## 7. Catalogue des algorithmes

### 7.1 Ordre d'introduction

| Algorithme | Usage | Garantie à annoncer | Conditions et limites | Lot |
|---|---|---|---|---|
| BFS | Plus court nombre de transitions | Optimal en nombre de pas à coûts uniformes | Ne minimise pas un coût pondéré général | Fondation |
| DFS | Exploration et comparaison pédagogique | Trouve un chemin sur un graphe fini correctement marqué | Aucun plus court chemin garanti | Fondation |
| Dijkstra | Référence sur coûts non négatifs | Coût minimal | Finalisation et relaxation correctes ; référence pondérée | Fondation |
| A* | Recherche guidée | Coût minimal sous les hypothèses de l'heuristique et du traitement des états | Heuristique admissible ; réouverture si nécessaire | Fondation |
| BFS bidirectionnel | Départs/buts uniques sur grands espaces uniformes | Même optimum que BFS avec arrêt correct | Gestion des couches et des deux sens | Laboratoire |
| Greedy Best-First | Étudier une recherche très guidée | Pas de garantie d'optimalité | Le résultat dépend beaucoup de l'heuristique | Laboratoire |
| Weighted A* | Compromis coût/effort | Borne de sous-optimalité seulement si ses hypothèses sont respectées | Poids `epsilon >= 1`, heuristique admissible, gestion des améliorations et arrêt correct | Laboratoire |
| Dijkstra bidirectionnel | Recherches pondérées à deux fronts | Coût minimal | Graphe transposé et critère d'arrêt démontré | Comparateur avancé |
| A* multi-objectifs ordonnés | Visiter des objectifs dans un ordre fixé | Somme des segments optimaux si les règles restent indépendantes des étapes | Ce n'est pas un solveur général d'ordre optimal des visites | Défis simples |
| IDA* | Étudier recherche et mémoire réduite | Optimal sous hypothèses adaptées et seuils corrects | Réexplorations potentiellement très coûteuses | Algorithmes avancés |
| Jump Point Search | Accélérer certaines recherches sur grille uniforme | Optimal dans le domaine de la variante implémentée | Élagage dépendant des mouvements ; pas de terrains pondérés arbitraires | Algorithmes avancés |
| Theta* | Trajets à angle libre | Chemins généralement plus directs, sans optimum global garanti | Ligne de vue et coût géométrique distincts du graphe à 4/8 voisins | Géométrie avancée |
| LPA* | Réparer des recherches proches | Optimalité selon les hypothèses de la recherche incrémentale | État `g/rhs`, priorités et mises à jour corrects | Simulation |
| D* Lite | Navigation avec changements de carte connue | Replanification selon son modèle | Déplacement du départ, changements de coûts, destination gérée explicitement | Simulation |
| Champ de distances inverse | Nombreux agents vers un même but | Potentiel de coût optimal pour le graphe retenu | Les règles locales d'agents ne garantissent pas l'absence de collision | Simulation |
| Space-Time A* | Trajet dans des réservations temporelles | Optimal dans l'espace-temps modélisé et le domaine exploré | Horizon et occupation finale explicitement définis | Multi-agent |
| Planification priorisée | Plusieurs agents avec réservations | Solution valide si elle est trouvée ; incomplète en général | Sensible à l'ordre des agents | Multi-agent |
| Conflict-Based Search | Petits problèmes MAPF exacts | Optimalité pour l'objectif et le modèle pris en charge | Complexité élevée ; budgets et solveur bas niveau corrects | Multi-agent avancé |

Le catalogue ne constitue pas une promesse de livrer toutes les lignes dans la même version. Chaque ajout suit la procédure de la section 21.

JPS doit respecter le domaine de validité de son élagage sur grille uniforme ; la publication d'origine ne justifie pas son emploi direct sur tout terrain pondéré. [Harabor et Grastien, 2011](https://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf).

### 7.2 Contrat commun d'une recherche

L'API expose conceptuellement :

```cpp
// Contrat illustratif, à adapter lors de la mise en œuvre.
SearchHandle begin_search(const MapSnapshot&, const SearchConfig&);
StepResult step(SearchHandle, StepBudget);
SearchResult result(SearchHandle);
void cancel(SearchHandle);
void destroy(SearchHandle);
```

- `begin_search` valide les entrées et crée un contexte ; il ne lance pas une boucle bloquante jusqu'à la solution.
- `step` effectue au plus un nombre d'opérations fixé et, en mode interactif, respecte aussi un budget de temps souple.
- `result` distingue résultat partiel et résultat final ; il ne reconstruit pas un chemin invalide depuis des parents périmés.
- `cancel` invalide l'exécution, arrête les étapes futures et conserve seulement les données promises par le contrat.
- `destroy` libère le contexte même après erreur ou annulation.

La même machine d'état est utilisée dans la CLI et dans WebAssembly. La CLI appelle les étapes successivement sans délais d'animation.

### 7.3 BFS et DFS

- BFS marque une case à son insertion dans la file afin d'éviter les insertions répétées.
- DFS conserve une stratégie de marquage et un ordre de voisins documentés.
- Le parent est fixé selon la règle choisie ; la reconstruction est identique à celle des autres recherches simples.
- Une grille à diagonales de coût `1414` et orthogonales `1000` n'est pas un problème de coût uniforme pour BFS.
- Le résultat de DFS n'est jamais utilisé comme oracle du plus court chemin.

**Tests ciblés :** ligne, embranchement, impasse, composantes disjointes, cycle, départ égal au but, terrain pondéré refusé par un préréglage BFS « optimal en coût ».

### 7.4 Dijkstra

- Initialiser toutes les distances à `INF`, celle du départ à zéro.
- À chaque amélioration, mettre à jour distance/parent et insérer une nouvelle entrée avec sa distance figée, ou employer une structure avec diminution de clé réellement testée.
- À l'extraction, ignorer une entrée devenue périmée.
- Finaliser un nœud lorsque son meilleur coût est extrait, pas à sa première découverte.
- Terminer sur l'objectif finalisé ; ne pas terminer à sa première insertion.
- Gérer les additions sans débordement et refuser les poids négatifs dans le domaine initial.

**Test indispensable :** un objectif découvert tôt par un trajet cher, puis atteint à moindre coût par un détour. Ce cas doit échouer avec une implémentation qui finalise à l'insertion et réussir avec l'implémentation retenue.

### 7.5 A*

- `g` représente le coût réel depuis le départ ; `h` une borne cohérente avec les règles ; `f = g + h`.
- Une amélioration de `g` met à jour le parent, même si le nœud a déjà été découvert.
- Si une heuristique admissible mais incohérente est autorisée, prévoir la réouverture des nœuds fermés.
- L'arrêt se produit sur l'extraction valide de l'objectif dans les conditions documentées.
- L'affichage signale si l'utilisateur choisit une heuristique expérimentale sans garantie.
- Les points de départage sont fixés ; `h = 0` doit retrouver le coût de Dijkstra.

**Tests ciblés :** les mêmes cartes que Dijkstra, plusieurs heuristiques, changement d'échelle d'affichage sans changement de résultat, cases à coûts variés et graphe nécessitant une meilleure route vers un état déjà découvert.

### 7.6 Recherches bidirectionnelles

- Maintenir deux contextes indépendants et reconstruire le raccord sans dupliquer le point de jonction.
- Pour BFS, traiter les couches correctement et sélectionner une rencontre compatible avec une borne sur la longueur totale.
- Pour Dijkstra, conserver le meilleur raccord `mu` et appliquer un critère justifié à partir des minima des deux frontières ; la première rencontre ne suffit pas en général.
- Utiliser les prédécesseurs et coûts du graphe transposé dans le sens inverse.
- Mesurer séparément explorations avant/arrière et total des états uniques.

**Acceptation :** sur tous les cas de référence, le coût égale celui de la recherche unidirectionnelle correspondante ; les cas dirigés et pondérés sont couverts avant d'activer leur compatibilité.

### 7.7 Algorithmes approximatifs et à mémoire réduite

Pour Greedy, afficher « chemin trouvé, optimum non garanti ». Pour Weighted A*, enregistrer `epsilon`, les hypothèses et la politique de réouverture ; une borne théorique n'est affichée que si l'implémentation en respecte les conditions. Comparer le coût retourné à l'optimum de référence quand ce dernier est disponible.

Pour IDA*, afficher itérations et réexpansions, pas seulement le nombre de cases distinctes. Un faible pic mémoire peut s'accompagner d'un temps élevé. Les budgets évitent de laisser le navigateur poursuivre indéfiniment un cas difficile.

### 7.8 JPS et géométrie à angle libre

La variante JPS choisie doit spécifier voisinage, politique de coins et coût relatif des mouvements. Les tests de forced neighbors et de sauts sont indépendants des tests de bout en bout. Les animations peuvent reconstruire les cases traversées par un saut, mais le compteur d'expansions conserve la définition du solveur.

Theta* relève d'un autre espace de solutions. Une courbe plus courte à l'écran ne signifie pas qu'A* sur un graphe à huit voisins est incorrect. Définir coût des segments, précision, ligne de vue et traitement des coins ; commencer avec terrains uniformes. Ne pas activer de pondération continue sans définir le coût de traversée d'un segment.

### 7.9 Recherche incrémentale

LPA* et D* Lite conservent de l'information entre des problèmes proches. Leur contexte possède une identité de monde, des valeurs `g/rhs`, une file de priorités et les informations de mise à jour nécessaires.

Chaque modification de coût est appliquée aux arêtes concernées et à leurs dépendances. Déplacer le départ ou changer la destination ne possède pas le même traitement : le contrat doit annoncer ce qui est supporté et ce qui réinitialise le contexte.

**Validation :** après chaque changement d'un scénario déterministe, comparer le résultat réparé avec une nouvelle recherche Dijkstra sur la nouvelle carte. Mesurer construction initiale et réparation séparément. [Publication D* Lite](https://idm-lab.org/bib/abstracts/papers/aaai02b.pdf).

### 7.10 Multi-agent

Les solveurs multi-agent sont une famille distincte avec leurs métriques et leurs limites. CBS s'appuie sur une recherche de conflits et sur un solveur individuel sous contraintes ; le caractère exact dépend de l'ensemble. [Sharon et al., publication CBS](https://doi.org/10.1016/j.artint.2014.11.006).

Pour le premier CBS, choisir un temps discret, des actions de durée un tick et un objectif clairement fixé. Les terrains peuvent influer sur un score de déplacement sans être immédiatement interprétés comme des durées différentes. Les durées continues ou variables constituent un autre modèle à implémenter ensuite.

### 7.11 Fiche obligatoire par algorithme

Chaque fiche comprend : problème résolu, préconditions, structures utilisées, pseudo-code, terminaison, garantie, limites, complexité dans le modèle annoncé, paramètres, événements de trace, cartes d'exemple, tests et référence scientifique si applicable.

Pour les structures classiques : BFS/DFS en `O(V + E)` ; Dijkstra avec tas binaire typiquement en `O((V + E) log V)` selon la gestion du tas. Les performances d'A* dépendent fortement de l'heuristique et des états développés ; éviter une promesse générale de complexité plus faible que Dijkstra.

<a id="editeur"></a>

## 8. Éditeur de cartes et génération procédurale

### 8.1 Contrat d'édition

- Une action utilisateur produit une commande validée avec région avant/après, nouvelle révision et impact sur les recherches.
- Prévisualisation pendant le geste, validation à la fin ; annulation d'un geste ne laisse pas une moitié de mur.
- Interpolation entre positions du pointeur pour ne pas laisser de trous lors d'un déplacement rapide.
- Coordonnées caméra converties en coordonnées monde, puis en cases ; les bornes sont validées avant accès au tableau.
- Déplacement explicite du départ et du but, avec choix d'outil. Ne plus déduire l'outil à partir de « premier clic, deuxième clic, puis murs ».
- Redimensionnement avec aperçu des éléments perdus, contrôle des extrémités et possibilité d'annuler.
- Raccourcis désactivés lorsque le focus est dans un champ texte.

### 8.2 Outils et contraintes

| Outil | Comportement attendu | Cas à vérifier |
|---|---|---|
| Pinceau/gomme | Taille et forme, trait continu, coût du terrain sélectionné | Franchissement des limites de carte |
| Trait/rectangle | Prévisualisation et application transactionnelle | Sens inversé du geste |
| Remplissage | Région selon critère choisi, coût borné et annulation | Très grande composante |
| Sélection | Copie, collage, déplacement, rotation si compatible | Marqueurs uniques et recouvrement |
| Symétrie | Axe horizontal/vertical et mise à jour cohérente | Départ/but dupliqués |
| Inspecteur | Terrain, coordonnées, coût et état de recherche en lecture seule | Résultat d'une ancienne révision |
| Validation | Bornes, extrémités, connectivité et règles | Carte volontairement sans solution |

### 8.3 Métriques de carte

Calculer dimensions, cases praticables, densité d'obstacles, composantes connexes, proportion de terrains, longueur/cost optimal entre les extrémités et, pour les labyrinthes, impasses, embranchements et cycles.

Ces métriques alimentent la difficulté sans la résumer à un nombre arbitraire. Une carte très dense peut être facile si le couloir est unique ; une carte ouverte peut être difficile avec une connaissance partielle. Un score de difficulté doit préciser ses facteurs et être ajusté à partir de parties observées.

### 8.4 Générateurs et contrôles

- DFS aléatoire : labyrinthes à longues branches, utile pour les premiers niveaux.
- Prim/Kruskal aléatoires : autres structures de couloirs ; ne pas présenter ces variantes sans pondération particulière comme un échantillonnage uniforme de tous les labyrinthes.
- Wilson : variante pour étudier l'échantillonnage d'arbres couvrants uniformes sur un graphe connecté, avec implémentation et générateur contrôlés.
- Division récursive : partitions et passages ; vérifier les dimensions et accès.
- Salles/couloirs : graphe de salles connecté, connexions supplémentaires paramétrables et placements validés.
- Automate cellulaire : cavernes avec analyse des composantes et politique explicite de liaison ou de rejet.
- Terrains : distribution et corrélation spatiale contrôlées, valeurs dans le domaine autorisé.

Pour chaque générateur : paramètres bornés, graine, version, test de déterminisme, validation de sortie et coût maximal de génération. Une sélection automatique « deux points éloignés » précise si elle est exacte ou heuristique ; deux parcours BFS ne donnent pas toujours le diamètre exact d'un graphe général.

### 8.5 Historique et mémoire

L'historique utilise des différences de régions plutôt qu'une copie complète après chaque case peinte. Le budget mémoire de l'historique est configurable ; fusionner les gestes proches selon une règle stable et supprimer les plus anciennes transactions lorsque le budget est atteint.

Sauvegarde automatique après un délai de repos, avec indicateur « modifications non enregistrées », « enregistrement en cours » ou « enregistré ». Une erreur de quota ne doit pas afficher à tort un succès.

<a id="graphismes"></a>

## 9. Direction graphique et expérience utilisateur

### 9.1 Direction retenue

Retenir une **interface d'atelier sobre**, avec panneaux lisibles, thème clair et sombre, typographie cohérente, icônes accompagnées de libellés et carte centrale. Ajouter des habillages de jeu sans rendre les informations scientifiques dépendantes d'un univers graphique.

Trois thèmes de carte suffisent pour la première bibliothèque :

| Thème | Apparence | Usage | Exigence de lisibilité |
|---|---|---|---|
| Analytique | Cases simples, états contrastés, flèches et valeurs optionnelles | Laboratoire et comparaison | Lecture immédiate de la frontière et du chemin |
| Donjon | Murs, sol, portes, personnage et sortie | Campagne et défis | Collisions et coûts toujours consultables |
| Terrain | Herbe, route, eau, sable, ponts et unités | Terrains pondérés et simulation | Légende des coûts disponible à tout moment |

Chaque thème respecte les mêmes données. Une case d'eau coûte ce que définit le scénario, pas ce que suggère seulement sa couleur. Les animations de décor peuvent être désactivées indépendamment des états de recherche.

### 9.2 Organisation de l'écran

- Barre supérieure : projet, mode, ouvrir/enregistrer, annuler/rétablir et état de sauvegarde.
- Barre d'outils à gauche : sélection, pinceaux, terrains et marqueurs.
- Zone centrale : carte, caméra et éventuels panneaux de comparaison.
- Inspecteur à droite : configuration de l'algorithme, règles, sélection et résultats.
- Bandeau inférieur : chronologie, commandes de lecture, vitesse et progression.
- Panneau secondaire repliable : statistiques, frontière, pseudo-code et messages de validation.

Regrouper les seize modes en espaces « Créer », « Explorer », « Comparer », « Jouer », « Simuler » et « Apprendre ». Les modes avancés sont accessibles sans surcharger l'écran de départ.

### 9.3 Composants et états

Définir des tokens : couleurs de surfaces et de texte, contrastes, espacements, tailles de contrôle, rayons, ombres et typographie. Construire des composants réutilisables pour boutons, sélecteurs, curseurs, champs numériques, onglets, panneaux, dialogues et tableaux.

Chaque composant doit couvrir normal, survol, focus clavier, actif, désactivé, chargement, erreur et succès lorsque ces états s'appliquent. Un contrôle désactivé pour incompatibilité explique la raison et propose un réglage compatible.

Les grands tableaux de résultats sont virtualisés. La sélection conserve son identité quand les lignes sont triées ou filtrées. Les erreurs de saisie ne réinitialisent pas toute la configuration.

### 9.4 Affichage de la recherche

- Différencier libre, obstacle, frontière, fermé, courant, départ, but et chemin final.
- Combiner couleur avec symbole, motif ou contour ; ne pas réserver toute l'information à une opposition rouge/vert.
- Afficher `g/h/f`, parent et coordonnées seulement au zoom approprié ou pour la sélection.
- Présenter « aucun chemin », « interrompu », « limite mémoire » et « résultat optimal » comme des états distincts.
- Offrir une vue de chaleur avec échelle et unité ; verrouiller la même échelle quand deux cartes sont comparées.
- Conserver une légende correspondant au mode réellement affiché.

### 9.5 Rendu 2D et ressources

Utiliser PixiJS pour la carte et React pour les commandes. Éviter un composant React ou un élément DOM par case. Une animation du moteur graphique ne doit pas provoquer une reconstruction de tout le panneau de configuration.

Les principes retenus sont : textures partagées, atlas lorsque pertinent, regroupement du dessin, culling des régions invisibles, mise à jour des seuls blocs modifiés et arrêt de la boucle active lorsque rien ne change. PixiJS possède un ticker de rendu ; cette architecture doit le piloter explicitement au lieu de conserver systématiquement une boucle maximale. [Boucle PixiJS](https://pixijs.com/8.x/guides/concepts/render-loop), [conseils de performance](https://pixijs.com/8.x/guides/concepts/performance-tips).

- Commencer par WebGL comme cible de référence ; WebGPU reste une expérimentation mesurée, non un prérequis.
- Découper les grandes cartes en blocs, par exemple 32 × 32 cases, puis mesurer l'effet réel du blocage et du culling.
- Ne pas créer un million d'objets graphiques ou d'écouteurs d'événements pour un million de cases.
- Convertir les clics par la caméra et indexer la case directement.
- Adapter la résolution au profil matériel et plafonner le ratio de pixels sur les grandes scènes.
- À perte du contexte graphique, conserver les données métier et reconstruire les ressources ; afficher un état de récupération.
- Si le contexte requis est indisponible, proposer le résultat tabulaire et l'export, ou un rendu analytique Canvas 2D limité, sans promettre les mêmes capacités.

### 9.6 Animations et effets

Le laboratoire privilégie la précision de lecture : transitions courtes, absence d'effets qui cachent l'ordre des opérations, option sans mouvement. Le jeu peut ajouter marche, ouverture de porte, impacts et particules limitées.

Éclairage, ombres, flou et effets de shader sont des améliorations visuelles facultatives. Ils doivent respecter un budget GPU et posséder un préréglage désactivé sur matériel modeste. Leur introduction ne remplace pas la mesure des coûts de rendu.

### 9.7 Caméra et périphériques

- Zoom centré autour du pointeur avec limites, recentrage et retour à l'échelle de référence.
- Déplacement par outil main et raccourci ; l'édition ne doit pas déclencher un déplacement de caméra involontaire.
- Minicarte pour grands niveaux et caméra synchronisée facultative en comparaison.
- Prise en charge Pointer Events et capture du pointeur pendant un geste.
- Clavier : palette de commandes et navigation de sélection de case ; éviter les conflits avec le navigateur.
- Tactile : gestes distincts pour déplacement et dessin, commandes suffisamment grandes et option de confirmation des outils destructifs.

Le bureau et la tablette sont les premières cibles de l'éditeur complet. Sur téléphone, privilégier exploration, relecture et défis avant de promettre un éditeur de grande carte équivalent.

### 9.8 Gestion des assets

Chaque ressource distribuée possède origine, auteur, licence, modifications et attribution requise. Conserver un manifeste `assets/licenses.json` et générer les crédits à partir de ce manifeste.

Réexaminer les polices Arial actuellement embarquées avant une distribution publique ; préférer une police avec licence de redistribution explicitement adaptée. Les exemples graphiques et les cartes de jeux issus de corpus externes ne deviennent pas automatiquement redistribuables avec le logiciel.

<a id="execution"></a>

## 10. Moteur de calcul, Workers et WebAssembly

### 10.1 Frontière retenue

Exposer une petite API C stable autour des sessions C++, puis une enveloppe TypeScript typée. Les tableaux passent par buffers ; les commandes restent des objets peu volumineux. Les classes internes du cœur ne sont pas exportées intégralement.

Embind constitue une alternative pour une première preuve de concept ou une API de haut niveau ; si utilisé, documenter les destructions d'objets et les durées de vie. Éviter d'exposer simultanément deux API complètes non équivalentes. La documentation Emscripten souligne la gestion de propriété des objets et les précautions avec les vues mémoire. [Embind](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html).

### 10.2 Contrats de messages

Un message contient `protocolVersion`, `requestId`, `runId` lorsque pertinent, `mapRevision`, le type et la charge utile. Les réponses d'une recherche contiennent une séquence croissante et un statut.

| Commande | Effet | Réponse attendue |
|---|---|---|
| `LoadSnapshot` | Valider et charger un instantané | Identité, capacité mémoire et erreurs éventuelles |
| `StartSearch` | Créer un contexte validé | Session créée ou configuration refusée |
| `Advance` | Exécuter un lot borné | Événements, métriques cumulées et statut |
| `Pause` | Suspendre l'ordonnancement | Pause effective après l'étape courante |
| `Resume` | Reprendre sans réinitialiser | État courant et reprise des lots |
| `Cancel` | Invalider et libérer le calcul demandé | Accusé d'annulation avec identité |
| `ApplyWorldChanges` | Mettre à jour un monde dynamique compatible | Révision appliquée ou réinitialisation requise |
| `GetResult` | Lire une sortie stable | Résultat partiel/final clairement identifié |
| `Dispose` | Libérer session, buffers et abonnements | Fin de session |

Les erreurs possèdent un code stable, une explication traduisible, la requête concernée et les détails techniques utiles dans les journaux. Les exceptions C++ ne traversent pas arbitrairement la frontière ; elles sont converties en statuts prévus.

### 10.3 Ordonnancement et annulation

- Une étape interactive vise quelques millisecondes, avec plafond d'expansions pour éviter un appel incontrôlé.
- Le Worker doit rendre la main à sa boucle de messages entre étapes. Une longue boucle synchrone WebAssembly ne peut pas recevoir magiquement une commande d'annulation.
- Utiliser une tâche planifiée entre étapes ; une succession illimitée de microtâches peut retarder les messages.
- Le moteur conserve la position nécessaire pour interrompre aussi un gros traitement de voisins, un saut JPS ou une reconstruction.
- Si un Worker ne répond plus au-delà du budget, le contrôleur peut le terminer, recréer une instance et restaurer un instantané sain.
- L'annulation est idempotente et invalide les réponses déjà en transit.
- L'interface reste utilisable durant le chargement du module et présente progression, erreur et réessai.

**Piège à éviter :** placer `solveEntireMap()` dans un Worker puis espérer une pause immédiate. Le calcul n'occupe plus le DOM, mais la commande Pause ne sera traitée qu'après retour de cet appel.

### 10.4 Transferts et mémoire

- Transférer les `ArrayBuffer` volumineux lorsque le propriétaire doit changer ; documenter que l'émetteur perd l'accès au buffer transféré.
- Copier explicitement un lot avant réutilisation d'une vue sur la mémoire WebAssembly ; une croissance mémoire peut invalider les vues antérieures.
- Employer un pool limité de buffers et un mécanisme d'acquittement. Le moteur ne doit pas produire indéfiniment des événements plus vite que l'interface ne les consomme.
- Sérialiser les coûts 64 bits en chaînes décimales dans JSON lorsque nécessaire ; ne pas les arrondir silencieusement au-delà de `Number.MAX_SAFE_INTEGER`.
- Garder les pointeurs internes privés et fournir des handles validés avec génération pour détecter les anciennes sessions.
- Prévoir une libération dans tous les chemins : succès, erreur de validation tardive, annulation, changement de mode et fermeture du projet.

Comlink simplifie les appels entre Workers et interface, mais ne supprime ni les coûts de copie ni les responsabilités de propriété. [Projet Comlink](https://github.com/GoogleChromeLabs/comlink).

### 10.5 Concurrence

Une recherche active par défaut. La comparaison peut utiliser deux Workers après mesure ; quatre panneaux peuvent réutiliser des résultats calculés séquentiellement. Limiter la concurrence selon le profil choisi plutôt que lancer automatiquement un Worker par cœur annoncé par le navigateur.

La première cible WebAssembly fonctionne sans pthreads et sans mémoire partagée. Introduire SIMD ou threads seulement après profilage, vérification des navigateurs et conception des conditions de déploiement. Les pthreads WebAssembly nécessitent des contraintes de mémoire partagée et d'isolation à traiter explicitement. [Documentation Emscripten pthreads](https://emscripten.org/docs/porting/pthreads).

### 10.6 Cache

Cache possible pour instantanés, composantes connexes, champs de distance et ressources graphiques. Chaque entrée indique son empreinte et les paramètres qui l'invalident.

Une modification d'obstacle invalide la connectivité ; une modification de poids invalide les distances dépendantes sans forcément changer la connectivité. Un changement de caméra n'invalide aucun résultat algorithmique. Le budget de cache est plafonné, avec éviction prévisible.

<a id="relecture"></a>

## 11. Animation, traces, pause et relecture

### 11.1 Trois horloges distinctes

1. **Calcul :** opérations et temps réellement consommés par le moteur.
2. **Lecture :** progression dans une trace, réglable par l'utilisateur.
3. **Simulation :** ticks de monde définissant positions, portes et collisions.

Une pause visuelle peut laisser le calcul préparer des lots dans une limite donnée. Une pause de calcul arrête leur production. Une pause de simulation fige les événements du monde. Les commandes doivent nommer ce qu'elles contrôlent.

### 11.2 Événements

Prévoir au minimum `Discovered`, `Expanded`, `Relaxed`, `Reopened`, `ParentChanged`, `GoalReached`, `SearchFinished`, puis les événements spécifiques documentés par l'algorithme.

Chaque événement contient un ordre logique, un nœud ou état, le type et les informations nécessaires à la lecture retenue. Les explications affichées sont dérivées d'événements explicites, pas inventées à partir de la couleur de la case.

### 11.3 Niveaux de trace

| Niveau | Usage | Données conservées |
|---|---|---|
| `None` | Mesure du calcul seul | Résultat et compteurs minimaux |
| `Summary` | Grand problème ou lecture globale | Lots et métriques agrégées |
| `Detailed` | Laboratoire et relecture exacte | Événements nécessaires à l'état et informations d'inspection |

Le mode mesure doit indiquer le niveau effectivement utilisé. Une trace détaillée peut coûter davantage que la recherche sur une petite carte.

### 11.4 Chronologie

- Vitesse exprimée en opérations par seconde en laboratoire, et temps de simulation dans les modes concernés.
- Avance, pause, reprise, étape unique, début/fin et recherche d'un événement.
- Retour en arrière par reconstruction depuis un checkpoint et rejeu des événements, ou événements réversibles réellement complets.
- Aucun « retour arrière » ne modifie l'état interne du solveur déjà avancé sans contrat spécifique ; il s'agit d'une relecture.
- Un saut vers une position lointaine se prépare sans bloquer le fil principal et peut être annulé.

### 11.5 Checkpoints et limites

Choisir la fréquence des checkpoints à partir du budget mémoire et du coût de reconstruction. Une copie complète d'un million de cases toutes les cent opérations n'est pas une stratégie acceptable.

Prévoir snapshots compacts, différences et compression lorsque mesurées utiles. Si la limite de trace est atteinte, proposer continuer sans détail, exporter puis purger, ou arrêter. La perte d'exactitude de lecture est affichée explicitement ; elle ne modifie pas le résultat final du moteur.

### 11.6 Transition depuis le comportement actuel

L'optimisation actuelle accélère la visualisation en regroupant les cases et en limitant la durée des phases. La cible offre un contrôle de lecture explicite : les trois secondes de la version SFML ne deviennent pas une limite fixe obligatoire pour le laboratoire. Le mode Démonstration peut être lent, le mode Instantané peut afficher immédiatement le résultat et le mode Mesure n'anime pas.

<a id="mesures"></a>

## 12. Comparaison et expérimentation scientifique

### 12.1 Définitions des métriques

| Mesure | Définition à conserver | Précaution |
|---|---|---|
| `path_edges` | Nombre de transitions du chemin | Différent du nombre de nœuds |
| `path_cost` | Somme selon le modèle de coût enregistré | Différent de la longueur à l'écran |
| `geometric_length` | Longueur géométrique avec unité déclarée | À angle libre et sur grille : espaces de solutions différents |
| `discovered_unique` | États découverts au moins une fois | Séparer nœuds physiques et états enrichis |
| `expanded_total` | Nombre d'expansions effectivement réalisées | Inclut les réexpansions |
| `expanded_unique` | Nombre d'états distincts développés | Ne révèle pas tout le travail en présence de réouvertures |
| `relaxations` | Améliorations de coût acceptées | Définition identique dans les recherches compatibles |
| `frontier_peak` | Maximum d'entrées de frontière présentes | Les doublons du tas sont comptés et documentés |
| `stale_pops` | Entrées de tas périmées écartées | Permet d'expliquer le surcoût de la stratégie de tas |
| `compute_active_ms` | Durée des phases de travail du moteur hors attente et animation | Mesure murale active, pas automatiquement temps CPU du processus |
| `end_to_end_ms` | Requête utilisateur jusqu'au résultat reçu | Inclut transferts, ordonnancement et initialisation selon le périmètre |
| `setup_ms` | Préparation du contexte et des structures | Affichée séparément, jamais cachée pour favoriser une méthode |
| `trace_bytes` | Taille de la trace conservée | Peut dominer la mémoire |
| `logical_memory_bytes` | Estimation des capacités des structures suivies | Ce n'est pas le RSS exact du navigateur |
| `repair_ms` | Travail d'une mise à jour incrémentale | À associer à un événement précis |
| `status` | Succès, absence démontrée, interruption ou limite | Ne pas agréger ces cas indistinctement |

### 12.2 Qualité de solution

Pour un optimum connu `C* > 0`, afficher `ratio = C / C*` et surcoût relatif. Pour départ égal au but et coût nul, afficher directement l'égalité attendue ; ne pas diviser par zéro.

Un optimum vient d'une preuve applicable, d'un cas analytique ou d'un solveur de référence validé. Une valeur importée d'un corpus n'est réutilisée que si les règles et la précision de coût sont identiques. Le modèle entier diagonal `1414/1000` ne correspond pas exactement à un corpus évalué avec `sqrt(2)`.

Ne pas comparer un chemin Theta* à un optimum Dijkstra à quatre voisins comme s'il s'agissait du même espace de solutions. Le rapport peut montrer les deux résultats mais doit expliciter cette différence.

### 12.3 Protocole recommandé

1. Valider chaque configuration et chaque chemin retourné.
2. Enregistrer carte, règles, algorithmes, versions, seeds et paramètres.
3. Utiliser une construction optimisée, sans debugger ni trace détaillée pour la campagne de vitesse principale.
4. Distinguer démarrage froid, module déjà chargé et cache de données ; ne pas mélanger ces mesures.
5. Prévoir des passages d'échauffement, par exemple 5 à 10, puis 30 mesures pour un premier protocole, à adapter à la durée des cas.
6. Alterner l'ordre des algorithmes ou utiliser un ordre aléatoire enregistré.
7. Exécuter séquentiellement les mesures de référence pour limiter la contention.
8. Inclure les coûts de réinitialisation dans un périmètre annoncé ; publier aussi le coût du noyau si utile.
9. Conserver les observations brutes, les limites atteintes et les erreurs.
10. Publier taille d'échantillon, médiane et dispersion. Les percentiles élevés exigent suffisamment de répétitions ; ne pas annoncer un p99 fiable avec trente essais.

Une recherche très courte peut être sous la résolution effective du chronomètre. La répéter en lots indépendants avec remise à zéro correcte, puis rapporter le protocole. Le temps de la boucle et l'instrumentation sont contrôlés sans soustractions arbitraires qui produiraient des durées négatives.

### 12.4 Corpus

- Cas analytiques : ligne, carte vide, détour imposé, objectif inaccessible et plateau de coûts.
- Familles générées : labyrinthes, espaces ouverts, obstacles répartis, salles/couloirs et terrains corrélés.
- Cas adverses : objectif découvert par un trajet cher, nombreux coûts égaux, réouvertures et grandes frontières.
- Séries dynamiques : coût qui augmente, coût qui diminue, porte fermée puis rouverte et mouvement du départ.
- Cas multi-agent : couloir étroit, échange de positions, croisement et destination qui bloque un passage.
- Corpus externes : Moving AI, avec provenance, conventions, attribution et permissions vérifiées. [Moving AI Benchmarks](https://www.movingai.com/benchmarks/index.html).

### 12.5 Rapport d'expérience

Le rapport comprend : objectif de l'expérience, liste des cas, hypothèses, environnement, versions, paramètres, résultats bruts, agrégats, erreurs, limites et interprétation.

Comparer les familles de cartes séparément avant un agrégat global. Éviter un « meilleur algorithme » universel. Un résultat peut être plus rapide sur une carte ouverte et moins intéressant sur des terrains fortement pondérés.

Pour une méthode incrémentale, publier la préparation initiale, chaque réparation et le total cumulé. Pour le multi-agent, publier objectif, nombre d'agents, taux de résolution, coût total, makespan et conflits détectés.

<a id="simulation"></a>

## 13. Simulation, agents et règles de jeu

### 13.1 Temps de simulation

La logique de monde avance à pas fixe, par exemple un tick logique de 100 ms pour une première simulation discrète. Le rendu interpole visuellement entre états et peut tourner à une autre fréquence.

Enregistrer la fréquence logique, l'ordre d'application des événements et les actions des agents. Un ralentissement du rendu ne doit pas accélérer un ennemi, sauter un déclenchement de porte ou modifier le score.

### 13.2 Entités de jeu

Prévoir `Agent`, `Goal`, `Door`, `Key`, `Trigger`, `Hazard`, `WorldEvent` et `ChallengeRules`. Les décorations restent dans une couche visuelle distincte.

Une clé ou une ressource modifie le problème de recherche : l'état peut devenir `(case, inventaire, ressources)` plutôt qu'une simple case. Introduire alors un `StateId` distinct du `NodeId` de la carte, un générateur de transitions adapté et une estimation mémoire tenant compte des combinaisons. Un solveur classique sur cases seules ne résout pas automatiquement un niveau avec inventaire.

Commencer par un petit nombre de clés et des budgets d'états explicites ; un million de cases multiplié par toutes les combinaisons d'inventaire peut devenir prohibitif. Les niveaux sont validés avec le même modèle enrichi que celui du jeu.

### 13.3 Comportements d'agents

- Joueur : actions validées par la simulation, pas par la seule animation du sprite.
- Patrouilleur : liste de points et politique de blocage.
- Poursuivant : perception, fréquence de replanification et objectif explicites.
- Explorateur : carte connue séparée de la carte réelle.
- Agent de démonstration : suit un trajet et vérifie chaque transition avant de l'exécuter.

Une machine à états simple suffit pour les premiers comportements. Un arbre de comportement ou un ECS n'est ajouté que si la composition des agents le justifie concrètement.

### 13.4 Multi-agent : conventions à figer

1. Un agent occupe une case à chaque tick.
2. Deux agents ne partagent pas une case au même tick.
3. L'échange simultané de deux cases par une même arête est interdit dans le modèle de référence.
4. Attendre est une action autorisée avec coût et durée définis.
5. Après arrivée, l'agent reste sur sa destination dans le préréglage initial ; un mode « disparaît à l'arrivée » est une autre configuration.
6. Le coût total et le makespan sont deux objectifs différents ; le solveur exact annonce celui qu'il optimise.
7. Un horizon borné produit au besoin « aucune solution trouvée dans cet horizon », pas une preuve sur tous les horizons.
8. Les changements de carte pendant un plan multi-agent sont soit interdits, soit traités par un mode dynamique dédié.

Les variantes MAPF possèdent des conventions différentes ; les rapports doivent les conserver. [Stern et al., définitions et benchmarks MAPF](https://arxiv.org/abs/1906.08291).

### 13.5 Score et équilibre

Séparer les modes « coût minimal », « temps de déplacement », « mouvements limités », « collecte » et « survie ». Chaque score présente ses composantes et les aides utilisées.

Le score ne dépend pas de la fréquence d'affichage, du temps de compilation ou du débit noVNC. Les défis distribués sont testés pour détecter les raccourcis involontaires, les niveaux impossibles et les états dont on ne peut plus sortir.

### 13.6 Interaction avec les changements du monde

Définir une politique lorsqu'un obstacle apparaît sur une case occupée : refuser l'édition, différer l'événement ou appliquer une règle de niveau explicite. Ne pas déplacer arbitrairement l'agent vers une case supposée libre.

Une replanification peut échouer : l'agent attend, explique le blocage et conserve un état cohérent. Les animations d'un trajet périmé sont invalidées dès qu'une transition devient interdite.

<a id="donnees"></a>

## 14. Sauvegardes, formats et échanges

### 14.1 Formats proposés

| Format | Contenu | Première utilisation |
|---|---|---|
| `.maze.json` | Carte et catalogue de terrains | Import/export simple |
| `.scenario.json` | Carte ou référence figée, extrémités, agents et règles | Reproduction d'un problème |
| `.experiment.json` | Configurations, cas, répétitions et résultats référencés | Campagnes de comparaison |
| `.challenge.json` | Niveau, objectifs, contraintes et progression de définition | Campagne et exercices |
| `.replay.json` + données de trace | Métadonnées et événements versionnés | Relecture |
| `.mazeproject` | Archive de projet avec manifeste | Plusieurs ressources et cartes, seulement après le format JSON simple |

Tous les formats possèdent un `schemaVersion`. La version du moteur, la version du protocole Worker et la version de format sont des concepts distincts.

### 14.2 Exemple minimal de carte

```json
{
  "schemaVersion": 1,
  "kind": "maze-map",
  "id": "map-demo",
  "name": "Détour pondéré",
  "topology": "rect-grid",
  "width": 4,
  "height": 3,
  "terrainCatalog": [
    { "id": 0, "name": "Sol", "multiplier": 1 },
    { "id": 1, "name": "Sable", "multiplier": 3 }
  ],
  "blocked": [0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0],
  "terrainIds": [0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0],
  "metadata": { "author": "local", "description": "Carte de démonstration" }
}
```

L'exemple décrit la carte, sans placer départ et arrivée dans les cellules. Le scénario ajoute ces identifiants ainsi que `movementRules` et une référence immuable à la carte.

### 14.3 Validation

Employer JSON Schema 2020-12 avec un validateur configuré pour ce dialecte ; Ajv dispose de différences de support entre les drafts qu'il faut respecter. [JSON Schema](https://json-schema.org/draft/2020-12), [support Ajv](https://ajv.js.org/json-schema.html).

La validation s'effectue avant mutation du projet et vérifie :

- Version et type de document connus.
- Bornes des dimensions et taille de fichier autorisée.
- Longueur des tableaux égale au nombre attendu de cases.
- Valeurs de terrain valides, multiplicateurs autorisés, identifiants uniques.
- Références d'agents, extrémités et événements dans les bornes.
- Nombres finis et conversions sans dépassement.
- Capacités requises réellement supportées par le moteur.
- Pour une archive future : chemins relatifs sûrs et tailles après décompression plafonnées.

Une erreur fournit emplacement, règle violée et solution possible. Une version plus récente non reconnue est refusée proprement ; elle n'est pas convertie en effaçant des champs inconnus.

### 14.4 Migrations

Écrire des fonctions pures `v1 -> v2 -> v3`, chacune couverte par des fichiers de référence. Conserver une copie de l'original avant réécriture persistante. Une migration ne modifie pas silencieusement les coûts ou la politique de diagonales.

Les anciennes couleurs de recherche ne sont pas interprétées comme des terrains durables. Pour le projet actuel, définir un export explicite depuis la grille ; ne pas déduire un mur uniquement d'un pixel noir d'une capture d'écran.

### 14.5 Stockage local

IndexedDB héberge projets, cartes, scénarios, préférences et résultats sélectionnés, avec index par date, nom et type. L'adaptateur masque les détails de transaction au reste de l'application. [IndexedDB](https://developer.mozilla.org/en-US/docs/Web/API/IndexedDB_API).

Le stockage navigateur possède des quotas et peut être supprimé selon le navigateur et les conditions d'utilisation. Fournir export, import, usage de stockage visible et gestion des échecs ; ne pas présenter une sauvegarde locale comme une sauvegarde distante durable. [Quotas et éviction](https://developer.mozilla.org/en-US/docs/Web/API/Storage_API/Storage_quotas_and_eviction_criteria).

Réserver `localStorage` à de petites préférences si nécessaire, pas aux traces lourdes. Les ressources de l'application mises en cache par un service worker ne remplacent pas la base de projets utilisateur.

### 14.6 Empreintes et identité

Définir une représentation canonique des propriétés sémantiques de la carte et des règles, puis calculer une empreinte. Ne pas utiliser le JSON brut dont l'ordre des champs peut varier.

Nom, thème, position de caméra ou commentaire peuvent être exclus de l'empreinte de recherche, mais les obstacles, poids, topologie, coins et objectifs doivent y participer selon leur responsabilité. Les formats conservent aussi une identité éditoriale du projet, distincte de cette empreinte.

### 14.7 Exports

- CSV : métriques avec noms de colonnes stables, unités et valeurs manquantes explicites ; protéger les cellules texte interprétables comme formules par un tableur.
- JSON : configuration complète et données brutes, coûts 64 bits préservés.
- PNG : carte avec légende facultative, attribution des ressources et taille contrôlée.
- Rapport HTML : résumé, paramètres, résultats et avertissements méthodologiques pertinents ; contenu utilisateur traité comme texte.
- Import Moving AI : conversion documentée du voisinage, des obstacles et des coûts, sans supposer l'équivalence des valeurs de référence.

<a id="performances"></a>

## 15. Performances et gestion des ressources

### 15.1 Objectifs à calibrer sur un matériel de référence

Les valeurs suivantes sont des **cibles initiales**, à vérifier et ajuster après prototype. Enregistrer modèle CPU, mémoire, GPU, OS, navigateur, alimentation et versions. Utiliser au moins une machine de bureau et une machine portable comparable à l'environnement actuel ; ne pas publier de budget universel sans ces informations.

| Indicateur | Cible initiale | Méthode |
|---|---|---|
| Dessin/édition courante | Réponse visible en moins de 100 ms au p95 sur carte 256 × 256 | Parcours instrumenté en navigateur réel |
| Annulation | Accusé et interface disponible en moins de 100 ms au p95 dans le profil standard | Recherche volontairement longue, annulation répétée |
| Animation analytique | Viser 60 images/s sur scène standard ; préréglage économique à 30 | Temps des frames et proportion de frames lentes |
| Temps par frame | Budget indicatif 16,7 ms à 60 Hz ou 33,3 ms à 30 Hz | Distribution et p95, sans masquer les pauses |
| Repos | Boucle de rendu arrêtée et charge proche de l'inactivité du navigateur | Observation de 30 s sans interaction |
| Étape de calcul interactive | Viser 2 à 5 ms par lot, avec plafond d'opérations | Mesure des lots et traitement des cas extrêmes |
| Sauvegarde courante | Confirmation en moins d'une seconde pour carte standard | Stockage local normal, quotas non saturés |
| Fuite mémoire | Pas de croissance monotone après 100 cycles lancer/annuler/charger | Mémoire logique, instances et profilage du navigateur |

Ne pas promettre qu'un A*, un CBS ou un générateur terminera toujours en un temps fixe : la structure du problème et les budgets déterminent les limites. Le système doit surtout rester contrôlable et rendre son statut exact.

### 15.2 Profils de ressources

| Profil | Taille de grille maximale initiale | Budget moteur + traces par session | Trace détaillée maximale | Usage |
|---|---|---|---|---|
| Léger | 256 × 256 | 128 Mio | 16 Mio | Portable, pédagogie et usage courant |
| Standard | 512 × 512 | 256 Mio | 64 Mio | Comparaison et édition avancée |
| Étendu | 1024 × 1024 | 512 Mio | 128 Mio | Expérimentation explicite, trace souvent réduite |

Les budgets comprennent contexte, frontière, événements et checkpoints suivis ; ils n'incluent pas toute la mémoire du navigateur, de React ou des textures. Le produit doit afficher cette distinction. Le total autorisé tient compte du nombre de sessions, des copies de cartes et des caches de rendu.

Les limites d'états enrichis et de multi-agent se calculent séparément. Une carte autorisée dans BFS simple peut être refusée pour un état combinant case, inventaire et temps.

### 15.3 Plan de profilage

1. Établir un cas représentatif et un cas qui reproduit le ralentissement.
2. Séparer coût du moteur, sérialisation, transfert, React et rendu.
3. Identifier un poste dominant avec traces du navigateur et profilage natif adapté.
4. Faire une modification ciblée.
5. Reprendre le même protocole, vérifier exactitude et comparer mémoire/durée.
6. Conserver les résultats de l'expérience avec la version du code.

Dans le cœur : examiner allocations, voisins temporaires, doublons de tas, localité des tableaux et reconstruction. Dans le web : nombre d'objets graphiques, mises à jour de panneaux, taille des messages, copies mémoire et rafraîchissements au repos.

### 15.4 Structures et optimisations à étudier

- Voisins produits dans un petit buffer fixe pour grille 4/8, sans `vector` alloué à chaque expansion.
- Réutilisation contrôlée des capacités entre contextes d'un même profil.
- Tables contiguës et index stables.
- Comparaison entre tas binaire avec entrées périmées et diminution de clé, selon les coûts réels.
- Réduction de trace ou compteur seul en mode benchmark.
- Mise à jour graphique par blocs modifiés.
- Prétraitement de composantes connexes pour rejeter certains problèmes inaccessibles, avec coût de préparation publié et cache invalidé.

SIMD, threads, GPU de calcul et structures exotiques viennent après ces mesures. Leur complexité n'est justifiée que par un gain reproduit sur les usages visés.

### 15.5 Dégradation contrôlée

Si une limite est proche : diminuer la résolution graphique, réduire les détails de trace avec accord explicite, limiter les panneaux actifs ou refuser un scénario avant allocation. Un dépassement produit `ResourceLimit` et propose une action utile : réduire la carte, changer le niveau de trace, fermer une comparaison ou utiliser la CLI.

Une mémoire déjà réservée par WebAssembly peut ne pas être rendue immédiatement au système. Pour récupérer un gros contexte, pouvoir détruire le Worker complet, puis recharger un module propre lorsque nécessaire.

<a id="qualite"></a>

## 16. Tests et critères de qualité

### 16.1 Stratégie

Le plus grand nombre de vérifications porte sur le cœur, les contrats et les règles. Les tests de navigateur couvrent des parcours significatifs. Les captures visuelles complètent les assertions métier sans remplacer la vérification des coûts ou des collisions.

Catch2/CTest, Vitest et Playwright sont les outils proposés ; leurs documentations officielles décrivent leurs usages respectifs. [Catch2](https://github.com/catchorg/Catch2), [Vitest](https://vitest.dev/guide/), [Playwright](https://playwright.dev/docs/intro).

### 16.2 Tests du cœur

| Famille | Exemples requis | Résultat attendu |
|---|---|---|
| Entrées | Dimensions nulles, produit trop grand, extrémités hors bornes | Refus contrôlé sans accès invalide |
| Cas simples | Ligne, grille vide, départ égal au but | Chemin et coût analytiques |
| Obstacles | Mur complet, impasse, boucle, passage unique | Statut et chemin corrects |
| Pondération | Détour bon marché, forte disparité, nombreux coûts égaux | Coût identique à l'oracle |
| Heuristiques | Nulle, Manhattan, octile, cas incohérent admis | Garanties et réouvertures respectées |
| Diagonales | Deux murs d'angle, une seule case adjacente libre | Respect de la politique de coins |
| Réinitialisation | Rejouer, changer de but, changer d'algorithme | Aucun état hérité par erreur |
| Annulation | Avant première étape, au milieu, après succès, deux annulations | État valide et libération |
| Reconstruction | Cycle injecté dans un test interne, parent invalide | Échec borné, pas de boucle infinie |
| Débordement | Très grands coûts et sentinelle `INF` | Aucun wrap-around |
| Déterminisme | Seeds, ordre de voisins et égalités | Sorties de référence stables |
| Dynamique | Hausse/baisse de coût, porte, départ mobile | Équivalence avec recalcul de référence |
| Multi-agent | Croisement, attente, échange, occupation du but | Aucun conflit accepté |

### 16.3 Oracles indépendants et propriétés

- Sur petits graphes, utiliser un oracle indépendant simple, par exemple Bellman-Ford de test avec poids non négatifs, ou une énumération exhaustive réellement bornée.
- Comparer Dijkstra et A* avec les coûts analytiques et cet oracle ; ne pas faire reposer toute la validation sur deux implémentations partageant le même bug.
- Vérifier que BFS et Dijkstra donnent le même coût uniquement dans les configurations uniformes correspondantes.
- Augmenter un coût d'arête sans autre changement ne peut pas diminuer l'optimum.
- Ajouter des obstacles sans autre changement ne peut pas améliorer l'optimum, en tenant compte du cas inaccessible.
- Modifier thème, zoom et durée d'animation conserve coût et chemin déterministe.
- Retourner un problème n'impose le même coût que si le graphe et les coûts sont symétriques ; les terrains payés à l'arrivée peuvent produire un graphe dirigé.
- Pour les générateurs : valider domaine, connectivité demandée et reproductibilité ; ne pas tester uniquement une capture d'image.

Générer de petites cartes aléatoires dans les tests avec seeds enregistrées en cas d'échec, afin de pouvoir reproduire précisément le cas.

### 16.4 Natif et WebAssembly

Exécuter le même corpus de cartes et configurations dans les deux constructions. Comparer statuts, coûts, chemins lorsque déterministes et compteurs dont la définition est indépendante du budget temporel.

Les frontières d'étapes peuvent varier selon la vitesse d'exécution ; ne pas exiger une identité des timestamps ou du découpage de lots. Comparer l'ordre logique de la trace si ce contrat est promis.

### 16.5 Tests des contrats et du stockage

- Messages mal formés, mauvais protocole, ancienne session et lots arrivant après annulation.
- Transfert d'un buffer, réutilisation interdite après transfert et libération après échec.
- Fichiers incomplets, version future, champs inconnus selon politique et tableaux incohérents.
- Migrations de formats avec fichiers avant/après enregistrés.
- Quota saturé, transaction IndexedDB interrompue et export de secours.
- Sauvegarde puis chargement conservant l'empreinte sémantique.

### 16.6 Tests d'interface et de parcours

Créer une carte, dessiner un trait, annuler, placer deux extrémités, lancer A*, mettre en pause, inspecter, reprendre, sauvegarder, recharger et exporter le résultat.

Tester séparément : clavier, tactile simulé, zoom, changement de thème, perte de contexte graphique, absence de support graphique et changement de mode pendant un calcul.

Pour les premières versions : Chromium, Firefox et moteur WebKit via Playwright. Compléter les validations de plateforme sur les navigateurs réels ciblés ; un test WebKit automatisé n'est pas à lui seul une certification sur tous les appareils Safari.

### 16.7 Mémoire, analyse et tests agressifs

- AddressSanitizer et UndefinedBehaviorSanitizer sur les constructions natives compatibles.
- clang-tidy sur les zones de logique, avec règles adaptées et faux positifs traités explicitement.
- Tests d'import avec tailles limites et fuzzing des parseurs lorsque le format public se stabilise.
- Vérification de destruction des contextes et abonnements dans les tests répétés.
- Mode Release avec les mêmes jeux de cas pour détecter les différences masquées par Debug.

[AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) aide à détecter les erreurs mémoire ; un passage sans erreur n'est pas une preuve générale d'absence de bug.

### 16.8 Politique de validation

Une correction de coût, de collision ou d'optimalité exige un cas de régression. Un changement de documentation exige des liens et exemples cohérents. Un changement de thème exige une revue visuelle et d'accessibilité, pas une nouvelle suite algorithmique artificielle.

Les seuils de performance stricts sont exécutés sur un matériel contrôlé. En CI partagée, publier les mesures et détecter les fortes régressions sans faire échouer aléatoirement une PR pour quelques millisecondes de bruit.

<a id="outillage"></a>

## 17. Outils, développement et intégration continue

### 17.1 Environnement développeur

- VS Code ou Visual Studio pour C++ ; clangd ou l'outillage C++ de l'IDE selon la configuration choisie.
- CMake Presets pour Debug, Release, tests avec sanitizers et WebAssembly.
- Ninja pour éviter des divergences de commandes entre machines lorsque disponible.
- pnpm et Node LTS verrouillés pour l'interface.
- Docker pour construire et tester sans installation complète locale.
- Profilage du navigateur pour UI, transferts et GPU ; outil natif adapté à l'OS pour le cœur.
- Scripts PowerShell et shell lorsque nécessaires, sans chemins absolus personnels.

Un développeur doit pouvoir identifier en quelques commandes comment compiler le cœur, lancer les tests et démarrer l'interface. Les scripts doivent afficher une erreur utile lorsqu'un outil requis manque.

### 17.2 Gestion des dépendances

- Un seul système principal de dépendances par écosystème : vcpkg pour les dépendances C++ déclarées, pnpm pour le web.
- Baselines et lockfiles versionnés ; installation reproductible en CI.
- Mise en cache des téléchargements et constructions, avec clés tenant compte des versions.
- Séparer dépendances de production, de test et de développement.
- Contrôler les licences et les mises à jour sans importer un framework entier pour une fonction mineure.
- Le cœur portable reste compilable avec le SDK Emscripten sans dépendance à une bibliothèque système Windows.

### 17.3 Intégration continue proposée

| Job | Contenu | Fréquence |
|---|---|---|
| `format-and-types` | Formats, lint, TypeScript, validation de schémas | Chaque modification concernée |
| `core-native-linux` | Construction, tests métier et sanitizers compatibles | Chaque PR |
| `core-native-windows` | MSVC, tests et chemins portables | Chaque PR affectant le cœur |
| `wasm-contract` | Compilation SDK fixé, corpus de parité et API | Chaque PR affectant cœur/contrats |
| `web-tests` | Vitest, composants et stockage | Chaque PR web |
| `web-e2e` | Parcours critiques et export/import | Chaque PR intégrée ou lot concerné |
| `docker-smoke` | Construction image et vérification du service | Modifications de livraison |
| `benchmark-report` | Campagne et comparaison avec référence | Planifiée ou avant release |
| `release-artifacts` | Paquets, empreintes, notes et inventaire des dépendances | Version publiée |

GitHub Actions est un choix proposé si le dépôt est hébergé sur GitHub ; les commandes doivent rester exécutables localement et portables vers un autre service CI. L'écriture de ce plan ne crée ni pipeline distant ni publication.

### 17.4 Conventions

- Noms et unités explicites ; éviter `distance` sans préciser coût, pixels ou longueur.
- Objets propriétaires visibles et RAII ; pas de `new` brut pour posséder durablement les boutons ou polices.
- Pas de logique métier fondée sur le libellé traduit d'un bouton.
- Encodage UTF-8, fins de ligne gérées par Git et scripts shell en LF.
- Fonctions avec responsabilité claire ; erreurs modélisées à la frontière.
- Identifiants stables indépendants de la traduction.
- Documentation de l'API, des garanties et des limitations d'un algorithme au même niveau de version que son code.

### 17.5 Gestion du travail

Chaque ticket contient : problème, scénario utilisateur, périmètre, dépendances, critères d'acceptation, validation et risques de migration. Chaque lot laisse une application lançable.

Une PR explique le comportement obtenu et les tests pertinents. Les grosses réorganisations sont séparées des changements algorithmiques pour faciliter la revue. Les ADR importants précèdent les fonctionnalités qui en dépendent.

<a id="distribution"></a>

## 18. Docker, distribution et fonctionnement hors ligne

### 18.1 Transition

Conserver le lancement SFML/noVNC comme profil de compatibilité pendant l'extraction. Il sert à vérifier les comportements et à garder le projet utilisable. Ne pas présenter ses performances comme celles de la future interface web native.

### 18.2 Image cible web

La construction cible comporte des étapes distinctes : tests natifs du cœur, compilation WebAssembly avec SDK fixé, construction web, puis image Nginx ne contenant que les artefacts nécessaires.

Le conteneur de livraison sert l'application. Le calcul interactif s'exécute dans le navigateur de chaque utilisateur ; il ne faut pas annoncer que déplacer le serveur Docker vers une machine plus puissante accélérera automatiquement un calcul WebAssembly côté client.

### 18.3 Profils proposés

| Profil | Fonction | Contenu |
|---|---|---|
| `legacy` | Exécuter l'application SFML existante | Xvfb, VNC, noVNC, SFML |
| `web-dev` | Développer les panneaux et le rendu | Vite avec rechargement, module WASM et reconstruction surveillée du cœur |
| `web-release` | Vérifier une livraison locale | Artefacts statiques servis avec la configuration cible |
| `test` | Reproduire les validations | Outils et scripts, aucun besoin de serveur d'affichage pour le cœur |
| `services` | Fonctions communautaires futures | API et base, seulement si implémentées |

Ces profils sont une cible future ; le `compose.yaml` actuel ne les implémente pas encore.

### 18.4 Distribution et cache

- Servir `.wasm` avec le type MIME correct, des chemins cohérents et la compression adaptée.
- Fichiers avec empreinte dans leur nom et cache long ; document d'entrée révalidé pour pointer sur une version cohérente.
- Worker et module WASM construits et distribués ensemble, avec contrôle de version du protocole.
- Si le multithreading partagé est ajouté, tester les en-têtes d'isolation requis et leurs interactions avec les ressources externes.
- Une mise à jour ne doit pas mélanger interface récente et ancien module en cache.

### 18.5 Hors ligne

Introduire une PWA après stabilisation des formats et des mises à jour. Mettre en cache l'application, le moteur et les ressources nécessaires aux modes sélectionnés ; les projets restent dans IndexedDB.

Afficher une mise à jour disponible et préserver une session en cours avant activation. Tester installation, première ouverture hors ligne après téléchargement complet, récupération après mise à jour et échec de quota. Le téléchargement initial reste nécessaire.

### 18.6 Application de bureau éventuelle

Ne pas choisir simultanément plusieurs enveloppes de bureau. Si l'installation native devient demandée, évaluer un emballage de l'interface web ou un adaptateur de bureau dédié en mesurant accès fichiers, taille du paquet, mémoire et maintenance.

Le cœur C++ reste réutilisable, mais cela ne rend pas gratuite l'intégration avec chaque système. Garder la CLI native comme premier livrable de bureau utile aux expérimentations.

<a id="services"></a>

## 19. Services en ligne, partage et collaboration

### 19.1 Conditions d'introduction

Un service est justifié par partage persistant, travail de classe, synchronisation multi-appareil ou collaboration. Les premières versions restent utilisables sans lui.

Choix proposé à ce stade : **API Fastify/TypeScript et PostgreSQL**, dans un service modulaire unique. Fastify fournit une base pour API et validation ; PostgreSQL conserve projets, versions et droits. Choisir une version supportée et des migrations enregistrées. [Fastify](https://fastify.dev/docs/latest/), [politique de versions PostgreSQL](https://www.postgresql.org/support/versioning/).

### 19.2 Modèle de données serveur

- Utilisateurs : identité et préférences synchronisées minimales.
- Projets : propriétaire, visibilité, version courante et métadonnées.
- Versions de projet : contenu immuable ou référence à un document validé.
- Droits : lecteur, éditeur et propriétaire, contrôlés côté serveur.
- Partages : portée, expiration éventuelle et possibilité de révocation.
- Exercices/classes : seulement lorsque le mode enseignant distant est réellement livré.
- Résultats publiés : origine, version du moteur et statut de vérification.

Les gros assets et traces peuvent être stockés hors de la base dans un stockage objet lorsque leur volume le justifie. Cette complexité n'est pas nécessaire pour les premières cartes partagées.

### 19.3 API indicative

| Route | Fonction | Règle de cohérence |
|---|---|---|
| `GET /api/v1/projects` | Lister les projets accessibles | Pagination et filtrage, droits appliqués |
| `POST /api/v1/projects` | Créer un projet validé | Taille et quota contrôlés |
| `GET /api/v1/projects/{id}` | Lire un projet | Autorisation et version renvoyées |
| `PUT /api/v1/projects/{id}` | Enregistrer une nouvelle version | Condition de révision/ETag, pas d'écrasement silencieux |
| `POST /api/v1/projects/{id}/shares` | Créer un partage | Droit propriétaire et portée explicite |
| `DELETE /api/v1/projects/{id}/shares/{shareId}` | Révoquer un partage | Révocation vérifiée à la lecture suivante |

Cette API est une proposition de contrat, pas une API existante. Documenter OpenAPI et tests d'autorisation à partir des routes réellement implémentées. Le CLI Postman déjà disponible peut servir aux contrôles d'API lorsque ces routes existeront ; il n'est pas nécessaire pour le moteur local actuel.

### 19.4 Synchronisation

Commencer par sauvegarde versionnée et verrouillage optimiste : la requête annonce la version de départ ; un conflit renvoie un statut et conserve les deux versions. L'interface propose consulter, dupliquer ou fusionner explicitement.

La coédition temps réel nécessite un modèle séparé : unité d'opération, ordre, conflits sur une même case, redimensionnement concurrent, connexion interrompue et historique. Choisir alors un protocole serveur ou un CRDT après prototype de ces cas. Ajouter une bibliothèque de collaboration ne résout pas la sémantique des portes, agents et recherches en cours.

### 19.5 Exigences de service

- Authentification éprouvée si comptes requis ; gestion de session et autorisation séparées.
- Données validées côté serveur même si elles ont été validées dans l'interface.
- Quotas de fichiers, requêtes et stockage par utilisateur ; pagination et limites.
- Requêtes SQL paramétrées et droits d'accès testés pour chaque ressource.
- Chiffrement de transport pour une publication, secrets hors du dépôt et journaux sans jetons.
- Sauvegardes avec restauration testée, migrations de base réversibles ou procédure de récupération.
- Résultats de défi soumis par le client considérés comme non vérifiés tant qu'un contrôle indépendant n'existe pas.

L'exécution de code C++ arbitraire envoyé par un utilisateur ne fait pas partie de l'API. Un éventuel service de calcul futur lance uniquement des algorithmes connus et des données bornées, dans un environnement conçu pour ces charges.

<a id="accessibilite"></a>

## 20. Accessibilité et internationalisation

### 20.1 Objectif

Viser WCAG 2.2 niveau AA pour les parties applicables, puis documenter les capacités et limitations réelles de la carte interactive. Une palette contrastée seule n'établit pas la conformité. [Recommandation WCAG 2.2](https://www.w3.org/TR/WCAG22/).

### 20.2 Exigences d'interface

- Tous les contrôles principaux accessibles au clavier, ordre de focus cohérent et focus visible.
- Libellés, descriptions d'erreur et états des commandes accessibles aux technologies d'assistance.
- Pas d'information portée uniquement par la couleur ; motifs et symboles pour les états essentiels.
- Respect de la préférence de réduction des animations et option pour arrêter les effets.
- Taille de texte adaptable sans perte d'accès aux commandes.
- Aucun piège de focus dans les dialogues ; retour au contrôle d'origine à la fermeture.
- Annonces de statut regroupées et modérées ; ne pas lire vocalement chaque expansion de mille cases.
- Tableau ou inspecteur sémantique décrivant la case sélectionnée, son terrain, ses voisins et ses valeurs de recherche.
- Aide clavier et alternatives aux gestes de glisser-déposer importants.

La carte Canvas reste un composant visuel spécialisé. Fournir une représentation textuelle de sélection et des résultats, et tester les parcours essentiels avec un lecteur d'écran avant toute revendication de conformité complète.

### 20.3 Traductions

Français et anglais comme premières langues. Identifiants métier et fichiers indépendants des libellés. Textes externalisés, pluriels et formats de nombres centralisés, unités visibles.

Préparer l'interface à des libellés plus longs. Si l'arabe est ajouté, traiter explicitement disposition de droite à gauche, saisie, direction des contrôles et sens des raccourcis ; ne pas inverser arbitrairement les coordonnées mathématiques de la carte.

### 20.4 Terminologie

Conserver un glossaire cohérent : découvert, développé, fermé, coût, distance géométrique, frontière, optimum, timeout et annulation. L'interface doit expliquer ces termes plutôt que changer leur sens d'un mode à l'autre.

<a id="extensions"></a>

## 21. Extensibilité et ajout d'un algorithme

### 21.1 Extensions internes en premier

Les premières extensions sont des modules du dépôt compilés et testés avec le moteur. Un développeur ajoute une implémentation au registre ; il ne modifie pas tous les boutons, formats et panneaux.

Un thème, un générateur et un algorithme constituent trois types d'extension différents avec leurs propres contrats. Les variantes ne doivent pas multiplier les conditions dans le contrôleur principal.

### 21.2 Procédure d'ajout

1. Décrire le besoin et le domaine exact de l'algorithme.
2. Fournir ses hypothèses et sa référence, ou expliciter le caractère expérimental.
3. Déclarer identifiant, version, paramètres et capacités.
4. Implémenter l'API incrémentale, l'annulation et la gestion des limites.
5. Définir ses compteurs et événements spécifiques.
6. Ajouter cas analytiques, cas adverses, tests de compatibilité et validateur de chemin.
7. Vérifier parité native/WebAssembly dans le domaine partagé.
8. Ajouter fiche pédagogique et scénario démontrant son intérêt.
9. Mesurer temps, mémoire et coût de trace sur les cas appropriés.
10. Vérifier qu'un projet sauvegardé annonce sa dépendance à cette version.

L'algorithme n'est proposé comme « stable » qu'après cette validation. Une entrée « expérimental » reste identifiable dans les rapports.

### 21.3 Contrat de configuration

Les paramètres possèdent type, valeur par défaut, bornes, unité, effet sur les garanties et règles de compatibilité. Exemple : `epsilon` de Weighted A* n'est pas seulement un curseur de vitesse ; il modifie la politique de recherche et potentiellement la garantie annoncée.

Un projet qui utilise une extension absente reste ouvrable pour consulter la carte et les résultats enregistrés ; relancer cette configuration nécessite l'extension correspondante.

### 21.4 Extensions externes ultérieures

Les thèmes de données validés peuvent précéder les extensions exécutables. Un format déclaratif de thème n'accède pas aux fichiers ni au réseau arbitrairement.

L'ajout de solveurs externes WebAssembly doit être un chantier dédié : imports autorisés, mémoire bornée, quotas de messages, temps maximal et arrêt du Worker. Le fait d'utiliser WebAssembly ne rend pas un calcul infini automatiquement annulable ni toute interface de plugin sûre.

Ne pas charger de bibliothèque native utilisateur dans le même processus que le cœur pour la première cible. Si ce besoin apparaît, définir isolation, version d'ABI, distribution et responsabilité des extensions avant de l'ouvrir.

<a id="migration"></a>

## 22. Migration de Mygame

### 22.1 Règle générale

Migrer progressivement, garder une version lançable et éviter une réécriture de tout le dépôt sans étapes comparables. Une branche ou un lot d'extraction ne mélange pas déplacement de fichiers, changement d'algorithme, nouvelle UI et modification de formats dans une seule opération.

### 22.2 Étape A — Baseline et formats de référence

- Capturer les usages actuels et conserver de petites cartes de référence.
- Décrire les règles actuelles : quatre voisins et coût uniforme.
- Écrire les premiers tests à partir de coûts connus, sans utiliser l'ancien A* comme oracle.
- Formaliser les limites et l'état actuel dans la documentation.
- Choisir encodage UTF-8 et effectuer la conversion dans un changement isolé, sans remplacer les caractères illisibles à l'aveugle.

### 22.3 Étape B — Cœur indépendant

- Extraire `GridMap`, `MovementRules`, `SearchContext` et `SearchResult`.
- Remplacer parents pointeurs par identifiants et tableaux.
- Corriger A* et Dijkstra, puis réinitialisation et validation d'entrée.
- Compiler le cœur sans inclure SFML.
- Adapter provisoirement la grille SFML pour lire ce modèle.
- Corriger les propriétaires des polices et boutons, ou supprimer ces responsabilités lors du remplacement de l'interface.

### 22.4 Étape C — CLI et mesures

- Charger un scénario validé, exécuter le moteur sans fenêtre et exporter un résultat.
- Ajouter le premier banc d'essai et les métadonnées d'environnement.
- Conserver le même modèle de coûts que les tests.
- Vérifier les résultats avant de mesurer l'amélioration de vitesse.

### 22.5 Étape D — Prototype web vertical

- Compiler `maze_core` vers WebAssembly.
- Créer une UI limitée, un Worker et un rendu analytique PixiJS.
- Ouvrir une carte, placer les extrémités, lancer et annuler.
- Vérifier la parité avec la CLI.
- Mesurer les échanges et la réactivité ; confirmer ou revoir l'ADR de pile technique.

### 22.6 Étape E — Interface de produit

- Ajouter éditeur, stockage local, laboratoire et comparaison.
- Remplacer progressivement les fonctionnalités SFML par des parcours web validés.
- Conserver l'ancien lancement sous profil legacy tant qu'il est utile pour la transition.
- Retirer les dépendances embarquées de l'application principale seulement lorsque les nouvelles constructions fonctionnent et que la procédure de restauration est connue.

### 22.7 Étape F — Modes riches

Introduire génération, défis et graphismes de jeu, puis simulation dynamique et multi-agent. Chaque ajout s'appuie sur le cœur validé, les formats et le contrôleur de lecture. Ne pas faire dépendre la sortie de la première version utile du développement complet de CBS ou de la coédition.

<a id="versions"></a>

## 23. Versions, lots et dépendances

Les versions ci-dessous définissent des ensembles cohérents. Aucun calendrier ferme n'est déduit sans connaître disponibilité, équipe, niveau de finition et volume d'assets. Le plan s'exécute par critères de sortie, puis s'estime à partir du premier prototype.

| Version cible | Contenu principal | Critère de sortie | Dépend de |
|---|---|---|---|
| R0 — Fondation fiable | Cœur extrait, BFS/DFS/Dijkstra/A* corrigés, tests, UTF-8, CLI minimale | Corpus analytique et premières propriétés validés, aucune dépendance SFML dans le cœur | Baseline |
| R1 — Parcours web | Worker/WASM, rendu analytique, édition de base, exécution et annulation | Même résultat CLI/web, interface réactive, construction reproductible | R0 |
| R2 — Outil complet | Laboratoire, traces, comparaison, cartes sauvegardées, benchmark, premières extensions algorithmiques | Parcours créer/comprendre/comparer/exporter complet | R1 |
| R3 — Création et défis | Générateurs, thèmes, campagne, scores, éditeur de niveaux | Niveaux validés, progression persistante et règles cohérentes | R2 |
| R4 — Simulation avancée | Événements dynamiques, LPA*/D* Lite, champs, premiers agents multiples | Réparations comparées au recalcul, conflits vérifiés et budgets respectés | R2, scénarios R3 utiles |
| R5 — Recherche et extensions | CBS limité, graphes/hexagones, programmation d'agent, carnets approfondis | Garanties et limites propres à chaque famille testées | R2/R4 selon module |
| R6 — Communauté facultative | Partage serveur, comptes, exercices distants, puis coédition si justifiée | Droits, versions, restauration et conflits testés | Formats stabilisés et usage validé |

### 23.1 Première version ambitieuse mais livrable

La première version de produit recommandée correspond à R2 : éditeur sérieux, quatre algorithmes fiables plus BFS bidirectionnel et Greedy, terrains pondérés, lecture contrôlée, comparaison et exports. Elle constitue déjà un outil substantiel même sans campagne ni service distant.

### 23.2 Dépendances fortes

- Corriger le modèle de coûts avant d'ajouter des terrains et de classer les algorithmes.
- Disposer d'un contexte indépendant avant de lancer des comparaisons concurrentes.
- Stabiliser traces et révisions avant relecture, carnets et collaboration.
- Définir le temps de simulation avant obstacles mobiles et multi-agent.
- Ajouter un validateur de conflits avant annoncer un solveur multi-agent utilisable.
- Avoir un format de projet stable avant synchronisation serveur.
- Vérifier un modèle d'états enrichis avant des portes/clés complexes ou des instructions avec ressources.

### 23.3 Travail parallélisable dans une équipe future

Une fois les contrats stabilisés, cœur, rendu, composants UI et scénarios peuvent évoluer dans des lots séparés. Les interfaces de messages et formats ont un propriétaire de revue. Les changements de règles et de format sont intégrés avant les fonctionnalités qui les consomment.

Cette description concerne l'organisation future du développement du projet ; elle ne suppose pas qu'une équipe est déjà disponible ni qu'une durée donnée est garantie.

<a id="backlog"></a>

## 24. Backlog détaillé et critères d'acceptation

### 24.1 Lecture du backlog

Priorités : **P0** correction/fondation nécessaire ; **P1** première version complète ; **P2** enrichissement du produit ; **P3** extension avancée. Tous les tickets ci-dessous sont proposés, pas annoncés terminés.

Les dépendances désignent des capacités et les tickets qui les établissent. Un ticket peut être divisé si sa revue devient trop grande, mais doit conserver un résultat vérifiable.

### 24.2 Fondation

#### F01 — Baseline et cas de référence — P0

- **Livrable :** règles actuelles, cartes analytiques, comportements à conserver et procédure de lancement enregistrés.
- **Acceptation :** chaque cas décrit départ, but, obstacles, coût attendu ou absence de solution ; aucun coût attendu ne dépend d'une capture de l'ancien A*.
- **Validation :** revue des cas et première exécution automatique. **Dépendance :** aucune.

#### F02 — Extraction du modèle — P0

- **Livrable :** `GridMap`, identifiants, règles et résultats indépendants de SFML.
- **Acceptation :** compiler et tester le modèle sans fenêtre ni en-tête SFML ; distinguer scénario et couleurs de recherche.
- **Validation :** construction headless, tests de bornes et invariants. **Dépendance :** F01.

#### F03 — Contextes et gestion mémoire — P0

- **Livrable :** contexte par exécution, tableaux de distances/parents, propriétaires RAII et nettoyage.
- **Acceptation :** rejouer et annuler sans données périmées ; destruction correcte après erreur ; aucun parent vers un objet graphique.
- **Validation :** cycles répétés et sanitizers. **Dépendance :** F02.

#### F04 — Dijkstra pondéré fiable — P0

- **Livrable :** relaxation, extraction, entrées périmées et coûts contrôlés.
- **Acceptation :** détour moins cher choisi même lorsqu'un chemin cher découvre d'abord le but ; coût confirmé par oracle.
- **Validation :** cas analytiques et graphes aléatoires bornés. **Dépendance :** F03.

#### F05 — A* et heuristiques fiables — P0

- **Livrable :** heuristiques dans les bonnes unités, amélioration des coûts et réouverture selon capacités.
- **Acceptation :** A* admissible retrouve le coût de référence ; `h=0` donne l'équivalent de Dijkstra ; zoom sans effet métier.
- **Validation :** corpus pondéré, diagonales, réouvertures et invariants. **Dépendance :** F04.

#### F06 — API incrémentale — P0

- **Livrable :** début, étapes bornées, résultat, annulation et destruction.
- **Acceptation :** même résultat pour plusieurs tailles de lots ; annuler ne produit pas de succès tardif dans la session active.
- **Validation :** séquences de machine d'état, budget et annulation à plusieurs étapes. **Dépendance :** F03/F05.

#### F07 — Construction et encodage — P0

- **Livrable :** UTF-8, CMake Presets, dépendances déclarées et tests Windows/Linux.
- **Acceptation :** aucune dépendance à `C:\Users\...` pour le cœur ; installation depuis un clone sur un autre chemin.
- **Validation :** CI ou reproduction locale sur deux environnements. **Dépendance :** F02.

#### F08 — CLI native — P0

- **Livrable :** charger un scénario, lancer, valider, exporter et retourner des codes de sortie documentés.
- **Acceptation :** pas de serveur graphique nécessaire ; résultat structuré utilisable dans une campagne.
- **Validation :** tests d'intégration sur fichiers valides et invalides. **Dépendance :** F04/F05/F07 et format minimal.

### 24.3 Première version web et laboratoire

#### W01 — Prototype C++/WebAssembly — P1

- **Livrable :** module compilé avec SDK fixé et enveloppe TypeScript.
- **Acceptation :** corpus commun natif/web, gestion des handles et des erreurs vérifiée.
- **Validation :** tests de parité et construction reproductible. **Dépendance :** F06/F07.

#### W02 — Worker et protocole — P1

- **Livrable :** commandes versionnées, lots, backpressure et annulation.
- **Acceptation :** recherche longue annulable, ancien lot ignoré, contexte récupéré après Worker interrompu.
- **Validation :** tests de messages et parcours réel d'annulation. **Dépendance :** W01.

#### W03 — Rendu analytique et caméra — P1

- **Livrable :** carte PixiJS, zoom, sélection et couches.
- **Acceptation :** thèmes et caméra ne changent pas le modèle ; aucune boucle de dessin active inutile au repos.
- **Validation :** tests visuels, coordonnées et profilage. **Dépendance :** F02 et UI minimale.

#### W04 — Édition transactionnelle — P1

- **Livrable :** outils de base, traits continus, undo/redo et extrémités explicites.
- **Acceptation :** un geste est annulé en une transaction ; une édition invalide ou annule la recherche correspondante.
- **Validation :** parcours pinceau rapide, limites et annulation. **Dépendance :** W03.

#### W05 — Formats et persistance locale — P1

- **Livrable :** schémas, import/export, migrations initiales et IndexedDB.
- **Acceptation :** rechargement sémantiquement identique, erreur de quota visible, fichier invalide sans mutation partielle.
- **Validation :** fichiers de référence et tests de stockage. **Dépendance :** F02/W04.

#### W06 — Système d'interface et accessibilité — P1

- **Livrable :** panneaux et composants avec états, focus, tokens et traductions de base.
- **Acceptation :** parcours créer/lancer/enregistrer utilisable au clavier ; erreurs liées aux bons champs.
- **Validation :** tests de composants, revue clavier/lecteur d'écran et contrastes. **Dépendance :** UI minimale.

#### W07 — Laboratoire et inspection — P1

- **Livrable :** trace, chronologie, valeurs `g/h/f`, frontière et pseudo-code.
- **Acceptation :** inspection fidèle, étape logique stable, retour par relecture cohérent.
- **Validation :** traces courtes connues et reconstruction depuis checkpoint. **Dépendance :** W02/W03/W06.

#### W08 — Comparaison visuelle — P1

- **Livrable :** deux recherches sur instantané identique et panneaux synchronisables.
- **Acceptation :** unités et garanties visibles ; classement indépendant des animations ; limite de concurrence respectée.
- **Validation :** modifications de carte, résultats tardifs et configurations incompatibles. **Dépendance :** W07 et registre de capacités.

#### W09 — BFS bidirectionnel et Greedy — P1

- **Livrable :** deux extensions avec fiches et tests distincts.
- **Acceptation :** coût BFS bidirectionnel égale la référence uniforme ; Greedy n'annonce pas l'optimalité.
- **Validation :** couches, rencontres, impasses et cas adverses. **Dépendance :** F06 et registre.

#### W10 — Terrains et règles avancées — P1

- **Livrable :** palette pondérée, diagonales, coins et calcul de coût explicite.
- **Acceptation :** refus des configurations incompatibles, données cohérentes dans UI/CLI/WASM.
- **Validation :** corpus de modèles de coût et exports. **Dépendance :** F04/F05/W05.

#### W11 — Banc d'essai et rapports — P1

- **Livrable :** campagnes, observations brutes, agrégats, CSV/JSON et environnement.
- **Acceptation :** recherche seule mesurée séparément ; limites/erreurs conservées ; campagne relançable.
- **Validation :** campagne courte déterministe et contrôle des définitions. **Dépendance :** F08/W05.

#### W12 — Livraison web native — P1

- **Livrable :** image statique, serveur configuré, versions cohérentes Worker/WASM et documentation.
- **Acceptation :** lancement sans Xvfb ni VNC pour la cible web ; module chargé avec bon MIME ; ancien cache sans mélange de protocole.
- **Validation :** image propre et parcours d'ouverture après mise à jour. **Dépendance :** W01/W02/W05.

### 24.4 Création et jeu

#### G01 — Générateurs reproductibles — P2

- **Livrable :** DFS/Prim initiaux puis variantes, seeds et paramètres versionnés.
- **Acceptation :** carte identique pour la même version/configuration ; contraintes de solution vérifiées et réparations enregistrées.
- **Validation :** invariants, graines de référence et cas de tailles limites. **Dépendance :** W05/W10.

#### G02 — Thèmes et assets — P2

- **Livrable :** analytique, donjon et terrain avec manifeste de licences.
- **Acceptation :** changement de thème conserve toutes les propriétés métier ; états accessibles et atlas sans chargements répétés inutiles.
- **Validation :** captures, revue des crédits et profilage. **Dépendance :** W03/W06.

#### G03 — Règles et progression de défis — P2

- **Livrable :** définition de défi, victoire, score et progression locale.
- **Acceptation :** séquence d'actions reproductible ; remise à zéro complète ; score indépendant des FPS.
- **Validation :** scénarios gagnants, perdants et abandon/reprise. **Dépendance :** W05 et simulation de base.

#### G04 — Joueur et adversaire — P2

- **Livrable :** déplacement validé, agent de démonstration et défi contre un agent.
- **Acceptation :** vitesse logique définie, collisions respectées, aides enregistrées.
- **Validation :** clavier, trajectoire interrompue et comparaison de scores. **Dépendance :** G03/G02.

#### G05 — États avec inventaire — P2

- **Livrable :** clés/portes et recherche sur état enrichi avec budget.
- **Acceptation :** solveur valide la même règle que le jeu ; dépassement de combinaison signalé sans allocation excessive.
- **Validation :** clés nécessaires, porte inaccessible et ressources épuisées. **Dépendance :** F06/G03.

#### G06 — Programmation d'agent — P2

- **Livrable :** grammaire, interpréteur borné, éditeur et relecture.
- **Acceptation :** pas d'exécution libre JavaScript ; erreur localisée et arrêt d'un programme infini.
- **Validation :** programmes de référence et budgets d'instructions. **Dépendance :** G03/G04.

#### G07 — Carnets, leçons et exercices — P2

- **Livrable :** annotations, fiches pédagogiques, exercices exportables et correction.
- **Acceptation :** plusieurs chemins optimaux acceptés si le critère porte sur l'optimalité ; provenance des mesures conservée.
- **Validation :** paquet d'exercice hors ligne et rapports. **Dépendance :** W05/W07/W11.

### 24.5 Extensions avancées

#### A01 — Weighted A*, IDA* et Dijkstra bidirectionnel — P2/P3

- **Livrable :** modules séparés avec paramètres et garanties.
- **Acceptation :** vérification des bornes annoncées dans les cas applicables ; mêmes coûts que Dijkstra pour sa variante bidirectionnelle.
- **Validation :** cas de réouverture, graphes dirigés et budget mémoire. **Dépendance :** F06/W11.

#### A02 — JPS et Theta* — P3

- **Livrable :** variantes aux domaines précisément déclarés et ligne de vue testée.
- **Acceptation :** terrain non supporté refusé ; pas de comparaison trompeuse entre espaces de solution différents.
- **Validation :** forced neighbors, politiques de coins, visibilité et corpus adapté. **Dépendance :** W10/W11.

#### A03 — Monde dynamique et LPA*/D* Lite — P3

- **Livrable :** ticks, événements de monde, réparation et résultats par changement.
- **Acceptation :** chaque réparation comparée à un recalcul frais ; déplacement du départ et changement de but gérés selon capacités.
- **Validation :** séries de hausses/baisses et relecture déterministe. **Dépendance :** F06/W07/W11 et simulation fixe.

#### A04 — Champs de direction — P3

- **Livrable :** potentiel inverse, directions et agents partageant un objectif.
- **Acceptation :** calcul inverse correct sur coûts dirigés ; limites de collision annoncées.
- **Validation :** potentiels de référence, zones inaccessibles et congestion. **Dépendance :** F04 et simulation fixe.

#### A05 — Réservations et CBS — P3

- **Livrable :** validateur MAPF, Space-Time A*, planification priorisée puis CBS limité.
- **Acceptation :** aucun conflit sur solution acceptée ; objectif et horizon enregistrés ; pas de fausse preuve d'impossibilité après timeout.
- **Validation :** petits optimums connus et cas de destination bloquante. **Dépendance :** simulation fixe/F06/W11.

#### A06 — Graphes et hexagones — P3

- **Livrable :** topologies et éditeurs spécialisés réutilisant les recherches génériques.
- **Acceptation :** position visuelle distincte des poids ; heuristique admissible ou nulle.
- **Validation :** graphes dirigés, conversion de coordonnées et filtrage des capacités. **Dépendance :** F02/W03/W05.

#### A07 — Hors ligne et mises à jour — P2

- **Livrable :** cache applicatif, installation PWA et migration contrôlée.
- **Acceptation :** ouverture hors ligne après téléchargement, session préservée lors d'une mise à jour et projet exportable.
- **Validation :** réseau coupé, ancien cache et quota faible. **Dépendance :** W05/W12.

#### A08 — Partage et service facultatif — P3

- **Livrable :** API, droits, versions et sauvegarde/restauration.
- **Acceptation :** projets privés protégés, conflits d'écriture visibles et aucune dépendance du bac à sable au service.
- **Validation :** tests d'API, droits croisés, quotas et restauration. **Dépendance :** W05/W12 et besoin de partage confirmé.

#### A09 — Coédition ou solveurs externes — P3, lots distincts

- **Livrable :** prototype du modèle de concurrence ou d'isolation, puis contrat spécialisé.
- **Acceptation :** invariants propres à l'extension démontrés avant ouverture publique ; désactivation sans casser le produit local.
- **Validation :** reconnexion/conflits pour coédition, arrêt/limites/imports pour solveurs. **Dépendance :** A08 pour coédition ; section 21 pour solveurs.

<a id="risques"></a>

## 25. Risques, arbitrages et points à mesurer

| Risque | Symptôme ou effet | Mesure de maîtrise | Moment de décision |
|---|---|---|---|
| Trop de fonctionnalités simultanées | Nombreux menus, peu de parcours terminés | Sortie R2 cohérente avant les extensions lourdes | Revue de chaque lot |
| Algorithme rapide mais faux | Coût inférieur/égal attendu non retrouvé, collision | Oracle indépendant, validateurs et corpus adverses | Avant affichage comme stable |
| Migration web trop coûteuse | Prototype sans parité ou construction fragile | Prototype vertical et ADR réexaminable | Fin de W01–W03 |
| Coût JS/WASM dominant | Beaucoup de copies ou un appel par nœud | Transferts par lots, API étroite et profilage | Prototype puis grandes cartes |
| Annulation non réactive | Worker occupé dans un appel unique | Étapes bornées, points de rendement et interruption de secours | F06/W02 |
| Trace plus grosse que la recherche | Mémoire et chargement excessifs | Niveaux de trace, backpressure et budget global | W07 |
| Concurrence excessive | Portable saturé et mesures trompeuses | Une tâche par défaut, limites de sessions et benchmark séquentiel | W08/W11 |
| Rendu dépendant du nombre de cases | Zoom ou peinture lents | Blocs, culling, buffers partagés et absence de DOM par case | W03 |
| Divergence moteur/graphismes | Agent traverse un mur dessiné ou terrain mal chiffré | Règles métier uniques, caches dérivés et test de transition | G02/G04 |
| Complexité des clés et du temps | Explosion du nombre d'états | `StateId` adapté, limites avant allocation, modèles livrés progressivement | G05/A05 |
| Présentation trompeuse des mesures | Animation assimilée au calcul, temps incomparables | Métriques définies, environnement et résultats bruts | W11 |
| Perte de sauvegarde locale | Quota, éviction ou changement de navigateur | Export, indicateurs d'état et tests d'échec | W05/A07 |
| Compatibilité de formats | Projet ancien illisible ou silencieusement modifié | Versions, migrations et copies de récupération | Chaque évolution de schéma |
| Ressources graphiques non redistribuables | Impossible de publier légalement le paquet prévu | Manifeste de provenance et choix d'assets adaptés | G02 et avant release |
| Entretien de trop de technologies | Builds difficiles et outils non utilisés | Socle mesuré, services et extensions différés | Chaque ADR |
| Coédition sans modèle de conflit | Écrasement de modifications ou carte incohérente | Versions d'abord, prototype de concurrence ensuite | A08/A09 |

### 25.1 Arbitrages permanents

- **Fidélité pédagogique / vitesse :** une trace détaillée explique mieux mais augmente le coût. Les deux modes de mesure doivent être distincts.
- **Nombre de modes / cohérence :** réutiliser les moteurs et composants, mais conserver les règles propres à chaque espace.
- **Aspect visuel / lisibilité :** le thème de donjon peut enrichir le jeu ; la vue analytique reste disponible.
- **Coût optimal / réactivité :** une méthode approximative peut être utile si la qualité et les limites sont visibles.
- **Multi-agent exact / nombre d'agents :** les petits problèmes exacts et les grandes simulations heuristiques sont deux produits expérimentaux différents.
- **Serveur / autonomie :** le partage apporte un service ; le laboratoire local ne doit pas tomber en panne parce que ce service est indisponible.

### 25.2 Questions à résoudre par un prototype

1. Coût réel de la frontière C++/WASM sur traces détaillées et gros tableaux.
2. Taille de bloc de rendu et mémoire de textures sur le portable cible.
3. Fréquence de checkpoints permettant un retour rapide sans surcoût disproportionné.
4. Nombre de sessions simultanées utile et soutenable pour la comparaison.
5. Budgets maximums d'agents et d'horizon pour les cas MAPF ciblés.
6. Apport utilisateur du mode défi et du langage d'agent par rapport au laboratoire.

Ces questions n'empêchent pas de choisir l'architecture ; elles identifient les hypothèses à vérifier avant d'étendre les limites du produit.

<a id="livraison"></a>

## 26. Définition d'une version livrable

Une fonctionnalité n'est pas terminée uniquement parce qu'elle apparaît dans un menu. Pour être livrée, elle remplit les conditions applicables suivantes :

1. Le parcours utilisateur fonctionne depuis une ouverture propre du projet.
2. Entrées, limites et configurations incompatibles sont traitées explicitement.
3. Résultat correct et garantie annoncée vérifiés.
4. Erreurs, annulation et ressources libérées sans état incohérent.
5. Tests de régression significatifs présents et exécutés.
6. Contrats et formats versionnés lorsque modifiés.
7. Import/export ou persistance compatibles avec la fonction.
8. Navigation clavier, focus et messages vérifiés pour les commandes concernées.
9. Mesure de performance pertinente, avec les unités et le périmètre adéquats.
10. Documentation utilisateur et technique mise à jour.
11. Assets et dépendances identifiés avec leur provenance.
12. Construction reproductible depuis un environnement propre.
13. Artefact de livraison vérifié et possibilité de revenir à la version précédente.

### 26.1 Revue avant publication

- Rejouer les parcours de référence de la section 27 sur une version Release.
- Vérifier migrations depuis la version publique précédente et récupération des projets.
- Conserver notes de version, changements de garantie, limites connues et empreintes d'artefacts.
- Vérifier installation/hors ligne si cette capacité est annoncée.
- Publier une matrice de compatibilité testée, sans déduire que tout appareil disposant d'un navigateur est supporté.
- Séparer fonctionnalité stable, expérimentale et planifiée dans l'interface et la documentation.

### 26.2 Indicateurs de maturité

Suivre : nombre de parcours complets, taux de réussite des scénarios, erreurs de coût détectées, régressions mémoire, temps d'annulation, intégrité des sauvegardes et compréhension des résultats par des utilisateurs d'essai.

Le nombre d'algorithmes et le nombre de lignes de code sont des informations de catalogue, pas des indicateurs suffisants de qualité.

<a id="parcours"></a>

## 27. Parcours complets de référence

### 27.1 Créer une carte et comparer deux stratégies

1. Créer une carte rectangulaire et choisir un préréglage quatre voisins.
2. Dessiner une zone de sable et un détour de sol moins coûteux.
3. Placer départ et but avec les outils dédiés.
4. Enregistrer, puis ouvrir Comparaison.
5. Sélectionner Dijkstra et A* avec Manhattan admissible.
6. Lancer ; observer que les expansions peuvent différer alors que le coût optimal correspond.
7. Déplacer la chronologie et inspecter une relaxation.
8. Exporter configuration et résultats ; recharger et obtenir le même coût.

**Vérification :** même carte/règles, chemins valides, optimalité de coût et données de rapport suffisantes pour relancer.

### 27.2 Expliquer l'échec d'une heuristique

1. Ouvrir un scénario pédagogique où un réglage inadéquat favorise un trajet cher.
2. Exécuter le préréglage optimal, puis une variante expérimentale explicitement étiquetée.
3. Observer `g/h/f`, les cases développées et le coût final.
4. Lire l'explication des hypothèses violées et revenir au réglage compatible.

**Vérification :** l'application ne qualifie jamais la variante expérimentale de « solution optimale » sans validation.

### 27.3 Générer et jouer un défi

1. Choisir un générateur et une graine.
2. Prévisualiser puis appliquer la carte.
3. Définir extrémités, objectif et contraintes de score.
4. Valider le niveau avec le solveur adapté.
5. Passer au mode Jeu et terminer ou abandonner.
6. Comparer le coût de son parcours à la référence.
7. Exporter le défi et vérifier sa réouverture.

**Vérification :** même niveau logique dans l'éditeur, le solveur et le jeu ; score indépendant de la vitesse d'affichage.

### 27.4 Replanifier dans un environnement modifié

1. Charger une carte avec une porte qui se ferme au tick défini.
2. Construire un chemin initial.
3. Déplacer l'agent jusqu'au changement de monde.
4. Comparer réparation et nouveau calcul complet.
5. Observer le trajet réellement suivi et les coûts initiaux/de réparation.
6. Rejouer les mêmes événements.

**Vérification :** aucun mouvement interdit ; mêmes changements ; coûts de réparation mesurés séparément ; statut exact si aucune route ne reste disponible.

### 27.5 Vérifier deux agents en conflit

1. Ouvrir un couloir avec deux agents dont les destinations se croisent.
2. Choisir les conventions de temps et d'occupation finale.
3. Lancer les solveurs supportés et afficher les réservations.
4. Exécuter le validateur indépendant de conflits.
5. Examiner coût total, makespan et limites atteintes.

**Vérification :** ni collision de sommet ni échange interdit ; résultat honnête si horizon ou budget insuffisant.

### 27.6 Préparer un exercice hors ligne

1. Créer une leçon et son scénario, fixer les paramètres nécessaires à la question.
2. Définir un critère portant sur coût, étape ou propriété annoncée.
3. Exporter le paquet et l'ouvrir sur une autre installation disposant de l'application.
4. Résoudre sans réseau et consulter la correction.

**Vérification :** les ressources nécessaires sont présentes ; les conventions utiles à une réponse précise sont incluses ; les chemins optimaux équivalents ne sont pas injustement refusés.

<a id="actions"></a>

## 28. Décisions et premières actions

### 28.1 Décisions proposées comme base du développement

| Décision | Choix | Critère de réexamen |
|---|---|---|
| Produit | Atelier de pathfinding avec création, pédagogie, mesure et jeu | Retours d'usage sur R2/R3 |
| Cœur | C++20 indépendant et commun à la CLI et au navigateur | Coût de portabilité démontré dans le prototype |
| Interface principale | React/TypeScript avec rendu PixiJS | Échec documenté du prototype de performance/maintenabilité |
| Calcul navigateur | WebAssembly dans un Worker, exécution incrémentale | Coût de frontière disproportionné malgré transferts adaptés |
| Rendu initial | Vue analytique WebGL, thèmes ensuite | Profil matériel et besoin réel d'autres moteurs |
| Données | Formats versionnés et stockage local avec export | Besoin de partage et de synchronisation vérifié |
| Réseau | Service facultatif ultérieur | Mise en œuvre de M16 ou classe distante |
| Garantie scientifique | Coûts et règles exacts, références et oracles | Toute extension de domaine doit redéfinir ses garanties |

### 28.2 Premier lot concret

Commencer par F01 à F05 avec un périmètre maîtrisé : quatre voisins, terrains uniformes puis pondérés, une seule recherche par contexte. Le résultat attendu est une bibliothèque testée, sans fenêtre, dont Dijkstra et A* trouvent les coûts de référence.

Poursuivre avec F06 à F08 et W01 à W03 pour démontrer le premier parcours complet dans le navigateur. Ce prototype doit être jugé avant la création de la campagne, de CBS ou de l'API communautaire.

### 28.3 Ce qui est à préparer avant le design final

- Identité visuelle et nom définitif.
- Trois ou quatre parcours prioritaires à montrer dans la première démonstration.
- Jeux de scénarios de référence avec hypothèses et coûts attendus.
- Machine et navigateurs de référence pour les budgets.
- Manifeste des ressources redistribuables et choix de police.
- ADR du cœur, du protocole et des coûts, puis schémas des premiers formats.

Ces éléments sont des tâches de réalisation, pas des demandes d'autorisation implicites pour installer, publier ou migrer le projet dans cette intervention documentaire.

<a id="glossaire"></a>

## 29. Glossaire

| Terme | Sens dans Maze Studio |
|---|---|
| Nœud | Position ou sommet du graphe |
| État | Position et toutes les variables nécessaires pour décider des transitions |
| Arête | Transition autorisée avec coût et éventuellement durée |
| Coût | Quantité optimisée selon le modèle du scénario |
| Heuristique admissible | Borne qui ne surestime pas le coût restant optimal |
| Heuristique cohérente | Heuristique respectant l'inégalité de transition sur chaque arête |
| Découvert | État rencontré et pris en compte par la recherche |
| Développé | État dont les successeurs sont traités selon la définition du solveur |
| Frontière | États ou entrées en attente d'exploration |
| Relaxation | Amélioration d'un coût connu et, normalement, du parent associé |
| Snapshot / instantané | Carte et règles figées pour une exécution |
| Révision | Identité d'un état éditorial de carte ou de projet |
| Trace | Historique logique enregistré des opérations |
| Checkpoint | État compact de relecture permettant une reconstruction plus rapide |
| Backpressure | Limitation de la production pour ne pas dépasser la capacité du consommateur |
| Worker | Contexte de calcul séparé du fil principal du navigateur |
| WebAssembly | Format d'exécution ciblé par le compilateur pour le cœur navigateur |
| MAPF | Recherche de trajectoires de plusieurs agents sans conflits selon un modèle défini |
| Makespan | Date d'arrivée du dernier agent dans un plan |
| Oracle | Référence indépendante permettant de vérifier un résultat |
| ADR | Document court expliquant une décision d'architecture |
| CI | Exécution automatisée des constructions et validations |

<a id="sources"></a>

## 30. Sources techniques et scientifiques

Les choix de produit, budgets et priorités sont des recommandations propres à ce projet. Les sources ci-dessous étayent les capacités techniques et les domaines des algorithmes ; elles ne garantissent pas les performances de l'application à construire. Documentation consultée dans le cadre de ce plan le 22 octobre 2026 ; les pages peuvent évoluer. Les versions stables seront verrouillées lors de chaque lot.

### Interface, calcul et formats

- [React — Synchroniser avec un système externe et éviter les effets inutiles](https://react.dev/learn/you-might-not-need-an-effect).
- [Vite — Démarrage, environnements et prérequis](https://vite.dev/guide/).
- [Node.js — Versions et support](https://nodejs.org/en/about/previous-releases).
- [PixiJS — Boucle de rendu](https://pixijs.com/8.x/guides/concepts/render-loop).
- [PixiJS — Conseils de performance](https://pixijs.com/8.x/guides/concepts/performance-tips).
- [Emscripten — Installation du SDK](https://emscripten.org/docs/getting_started/downloads.html).
- [Emscripten — Embind et gestion des objets](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html).
- [Emscripten — Pthreads](https://emscripten.org/docs/porting/pthreads).
- [MDN — Web Workers](https://developer.mozilla.org/en-US/docs/Web/API/Web_Workers_API/Using_web_workers).
- [Comlink — Communication avec les Workers](https://github.com/GoogleChromeLabs/comlink).
- [JSON Schema — Draft 2020-12](https://json-schema.org/draft/2020-12).
- [Ajv — Support des dialectes JSON Schema](https://ajv.js.org/json-schema.html).
- [MDN — IndexedDB](https://developer.mozilla.org/en-US/docs/Web/API/IndexedDB_API).
- [MDN — Quotas et éviction du stockage](https://developer.mozilla.org/en-US/docs/Web/API/Storage_API/Storage_quotas_and_eviction_criteria).
- [W3C — WCAG 2.2](https://www.w3.org/TR/WCAG22/).

### Construction, tests et services

- [vcpkg — Mode manifeste et versionnement](https://learn.microsoft.com/en-us/vcpkg/concepts/manifest-mode).
- [Catch2 — Tests C++](https://github.com/catchorg/Catch2).
- [Google Benchmark — Mesures natives](https://github.com/google/benchmark).
- [Vitest — Guide de démarrage](https://vitest.dev/guide/).
- [Playwright — Tests de navigateur](https://playwright.dev/docs/intro).
- [Clang — AddressSanitizer](https://clang.llvm.org/docs/AddressSanitizer.html).
- [Fastify — Documentation officielle](https://fastify.dev/docs/latest/).
- [PostgreSQL — Politique de support](https://www.postgresql.org/support/versioning/).

### Algorithmes, reproductibilité et alternatives

- [PCG — Générateur et tirage borné de référence](https://www.pcg-random.org/using-pcg-c-basic.html).
- [Moving AI — Corpus de benchmarks](https://www.movingai.com/benchmarks/index.html).
- [Harabor et Grastien — Online Graph Pruning for Pathfinding on Grid Maps, AAAI 2011](https://users.cecs.anu.edu.au/~dharabor/data/papers/harabor-grastien-aaai11.pdf).
- [Koenig et Likhachev — D* Lite, AAAI 2002](https://idm-lab.org/bib/abstracts/papers/aaai02b.pdf).
- [Sharon et al. — Conflict-Based Search, Artificial Intelligence 2015](https://doi.org/10.1016/j.artint.2014.11.006).
- [Stern et al. — Multi-Agent Pathfinding: Definitions, Variants, and Benchmarks](https://arxiv.org/abs/1906.08291).
- [SFML — Migration vers SFML 3](https://www.sfml-dev.org/tutorials/3.0/getting-started/migrate/).
- [Godot — Export Web, documentation stable](https://docs.godotengine.org/en/stable/tutorials/export/exporting_for_web.html).

Les fiches d'algorithmes créées pendant l'implémentation devront compléter cette bibliographie avec la référence de chaque variante effectivement retenue, ses hypothèses et les écarts éventuels à la publication.
