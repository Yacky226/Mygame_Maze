# Maze Studio 0.3 — génération, comparaison et univers graphique

Livraison du 23 octobre 2026. Client web 0.3.0 ; moteur C++/WASM 0.2.0.
Cette livraison prolonge l’atelier existant sans changer de moteur de recherche.
Le [plan directeur](../PLAN_DIRECTEUR_MAZE_STUDIO.md) conserve la vision complète
de la plateforme ; ce document décrit les fonctions réellement implémentées.

## Parcours de découverte

1. Lancer `docker compose up --build -d --wait web` à la racine du dépôt.
2. Ouvrir http://localhost:8080 puis **Créer un labyrinthe**.
3. Choisir backtracker, Prim ou Kruskal ; laisser 41 × 25 et la graine 42.
4. Observer la carte dans **Univers Ruines**, puis passer en **Vue technique**.
5. Lancer A* avec la vitesse maximale pour voir le chemin.
6. Dans **Une carte. Cinq stratégies.**, choisir cinq répétitions et comparer.
7. Exporter le rapport JSON pour conserver la carte, les règles et les mesures.
8. Ouvrir le scénario **Le détour pondéré** et comparer à nouveau : A* et
   Dijkstra obtiennent 6 pas ; Greedy choisit ici une route directe de 28 pas.
   BFS indique explicitement son incompatibilité avec ces coûts non uniformes.

## Génération dans le cœur C++

Les trois générateurs sont dans `core/src/generator.cpp`. Ils sont appelés par
la CLI et par WebAssembly : aucun second générateur JavaScript ne risque de
produire des cartes différentes. Leur famille d’algorithmes est présentée dans
le [catalogue de Jamis Buck](https://jamisbuck.org/mazes/).

| Choix | Construction | Structures utilisées |
| --- | --- | --- |
| Backtracker | Ouvrir un passage vers une salle voisine non visitée, puis revenir en arrière | Pile explicite, tableau des salles visitées |
| Prim aléatoire | Tirer une arête de la frontière entre région visitée et salles voisines | Tableau d’arêtes, retrait par échange avec la dernière |
| Kruskal aléatoire | Mélanger les arêtes et joindre les composantes encore distinctes | Fisher–Yates, ensembles disjoints avec compression et union par taille |

Tous sont itératifs : la taille de la carte ne fait pas croître la pile d’appels.
Les salles sont aux coordonnées impaires ; les passages relient leurs centres.
Les bordures restent fermées. La carte libre est connexe et sans cycle avec
quatre directions, donc chaque paire de cases libres possède un seul chemin
simple. Cette propriété ne promet pas un chemin unique lorsque les règles de
déplacement changent, notamment si les diagonales traversant les coins sont
autorisées. Les poids initiaux valent 1.

Le départ est `(1, 1)`. Une traversée BFS place l’arrivée sur une case de distance
maximale depuis le départ. En cas d’égalité, le premier maximum découvert selon
l’ordre de voisinage fixe est conservé. Sur une carte 3 × 3, départ et arrivée
coïncident : une seule case est libre et le coût vaut zéro.

### Dimensions et contrat de graine

- Interface web : côtés impairs de 3 à 255 ; graine entière de 0 à 4 294 967 295.
- API C++/CLI : côtés impairs d’au moins 3, dans la limite de 1 048 576 cases.
- La génération fixe les règles à quatre directions et retire le plafond
  d’expansions précédent ; l’algorithme de recherche sélectionné est conservé.
- Le bouton de nouvelle graine utilise `crypto.getRandomValues` ; la graine est
  ensuite visible et exportée. Le générateur n’utilise aucune horloge cachée.

La version de génération **1** fixe SplitMix64 tronqué aux 32 bits supérieurs,
le tirage borné avec rejet, l’ordre des voisins et les règles de départ/arrivée.
Les distributions de la bibliothèque standard ne sont pas utilisées, afin de
conserver le même résultat entre MSVC, GCC et Emscripten. Des cartes de référence
7 × 7 et des tests de parité native/WASM protègent ce contrat.

La provenance facultative est exportée dans le projet version 1 :

```json
"generation": {
  "algorithm": "prim",
  "seed": 42,
  "version": 1,
  "modified": false
}
```

`modified` passe à `true` après un changement effectif de mur, poids ou extrémité.
L’annulation restitue la provenance de l’instantané précédent. Une carte modifiée
se restaure depuis ses tableaux exportés ; sa graine seule ne suffit plus.
La provenance importée décrit une origine déclarée, sans certification : l’import
valide ses champs, mais ne régénère pas la carte pour en vérifier l’authenticité.

### CLI de génération

```powershell
.\build\core-debug\apps\cli\Debug\maze_generate.exe --generator prim --seed 42 --width 41 --height 25
```

La sortie ASCII emploie `#`, `.`, `S`, `G` et `B` et peut être fournie à
`maze_cli`. Pour la conserver sous Windows PowerShell 5 sans l’encodage UTF-16
par défaut de la redirection :

```powershell
.\build\core-debug\apps\cli\Debug\maze_generate.exe --generator kruskal --seed 42 --width 41 --height 25 | Set-Content -Encoding ascii carte.maze.txt
.\build\core-debug\apps\cli\Debug\maze_cli.exe carte.maze.txt --algorithm greedy
```

Codes de sortie du générateur : 0 succès, 2 arguments invalides, 4 allocation
impossible, 5 erreur interne. `--help` documente les options.

## Greedy Best-First

Greedy choisit la plus faible distance heuristique estimée jusqu’au but ; A*
prend aussi en compte le coût déjà parcouru. Cette distinction est expliquée
dans [l’introduction de Red Blob Games](https://www.redblobgames.com/pathfinding/a-star/introduction.html).

L’implémentation utilise la même heuristique géométrique que A*, une file de
priorité déterministe et un marquage à la première découverte. Elle accepte
terrains pondérés, quatre/huit directions et les règles de coins existantes.
Elle ne promet pas le coût minimal. Son résultat porte toujours une garantie
d’optimalité fausse, même s’il coïncide avec le meilleur coût sur une carte donnée.
Elle est disponible dans le sélecteur web, les comparaisons, la CLI et l’API C++.
Les boutons de l’ancienne interface SFML conservent leurs quatre choix.

## Protocole du comparateur

Le comparateur exécute les cinq algorithmes séquentiellement dans le même Worker.
Il verrouille les modifications de carte et de règles pendant son travail.
Changer uniquement le style graphique n’affecte pas les recherches.

Pour chaque algorithme compatible :

1. Une exécution d’échauffement complète, sans conserver sa durée.
2. Trois, cinq, sept, onze ou quinze exécutions mesurées selon le choix affiché.
3. Une nouvelle session et le même instantané à chaque exécution.
4. Des lots de 4 096 retraits de frontière, avec retour à la boucle du Worker
   entre lots pour permettre l’annulation.
5. Trace des développements désactivée et chemin non transmis au client.
   Le moteur calcule et valide quand même le chemin final et son coût.
6. Vérification de stabilité du statut, du coût, du nombre de développements
   et du nombre de mouvements avant de résumer la série.

Les temps bruts restent dans leur ordre d’exécution. La médiane, le minimum et
le maximum sont calculés à partir d’une copie triée avec des entiers `BigInt`.
Les nanosecondes sont sérialisées en chaînes décimales, sans perte de précision.
Le temps représenté est `computeActiveNs` : hors initialisation et copie de carte,
échanges Embind, messages, rendu, attentes et pauses. C’est une durée murale
mesurée dans le moteur ; elle ne représente pas le temps CPU total ni la latence
perçue de l’opération. Les lots de lecture animée utilisent un autre ordonnancement.

Le tableau présente coût en pas, surcoût relatif à Dijkstra, développements,
frontière maximale et durées. La frontière compte des entrées en attente, pas
des octets de mémoire. Un algorithme peut développer plusieurs fois un même
nœud lorsque sa politique de recherche le prévoit.

Une configuration BFS non uniforme est signalée sans répétitions mesurées.
Un budget atteint garde le statut `resource_limit`, distinct de `no_path`.
Sans coût de référence valide, le surcoût reste absent. Un départ égal à
l’arrivée ne produit ni division par zéro ni pourcentage non numérique.

### Annulation et export

L’annulation conserve les lignes dont toutes les répétitions sont terminées.
La ligne en cours n’est pas transformée en moyenne partielle ; le rapport porte
`complete: false`. Les réponses d’une ancienne opération sont écartées par `runId`.
Une modification de carte ou de paramètres efface le rapport affiché.

- **JSON** : instantané complet, règles, date UTC, version moteur, répétitions,
  échauffement, lignes, métriques et durées brutes. Ce rapport n’est pas le format
  d’import d’un projet : utiliser **Exporter le projet** pour reprendre l’édition.
- **CSV** : tableau de synthèse avec unités explicites (`cost_milli_steps`,
  `median_compute_active_ns`), provenance, paramètres et nombre de mesures.
  Encodage UTF-8 avec BOM, virgule, champs cités, fins de ligne CRLF.
- **Empreinte** : FNV-1a 32 bits de la carte et des règles. Elle facilite le
  rapprochement des rapports, mais n’est ni une signature ni une garantie
  d’unicité. Le JSON contient l’instantané nécessaire à la reproduction.

Les durées dépendent du navigateur, de la charge de la machine, de l’optimisation
du moteur WASM et de la carte. Le comparateur n’annonce pas de gagnant universel.
Un échauffement réduit certains effets du premier passage sans les éliminer.

## Direction graphique et ressources locales

L’interface conserve une palette vert profond, pierre et lime, avec les polices
Manrope et DM Sans locales. Une illustration de labyrinthe en diorama accompagne
le bandeau. Elle est décorative : l’éditeur interactif reste une vue 2D de dessus,
dont les murs et les chemins correspondent exactement à la carte du moteur.

**Univers Ruines** utilise un atlas de quatre matières : pierre moussue pour les
murs, dalles de grès pour le sol, végétation pour les poids 2–4, sol rocheux pour
les poids à partir de 5. Les poids chiffrés restent visibles dès que la taille
des cases le permet. Départ, arrivée et chemin restent au-dessus des textures.
Les textures représentent le terrain, sans modifier ses règles ni ses coûts.

**Vue technique** retrouve les couleurs unies pour analyser les expansions et
les poids. Le choix de vue se conserve dans `appearance.view` du projet exporté.
Les anciens projets sans ce champ restent acceptés et s’ouvrent en Ruines.

Les deux images ont été générées pour ce projet, puis encodées en WebP. Leurs
origines, dimensions, tailles et instructions de réencodage figurent dans
[`public/art/README.md`](../apps/web/public/art/README.md). Aucune requête vers
un fournisseur d’images ou une API de génération n’est faite à l’exécution.

### Coût du rendu et petits écrans

- PixiJS rend à la demande ; aucune boucle d’animation permanente n’est ajoutée.
- L’atlas est chargé une seule fois ; chaque matière possède seize fragments
  partagés pour que son motif s’étende sur quatre cases, sans multiplier les
  décodages d’image. Le chargement désactive le Worker d’images temporaire de
  PixiJS pour rester compatible avec la CSP du serveur ; le calcul de recherche
  conserve son Worker dédié servi depuis le projet.
- Une bascule de vue reconstruit les calques, sans réinitialiser la recherche.
- Au-delà de 16 384 cases ou sous 6 pixels par case, le rendu technique prend
  automatiquement le relais avec une indication visible. Ce seuil protège le
  rendu dense ; il ne change pas le choix de vue enregistré.
- Si le chargement des matières échoue, la carte reste disponible en couleurs
  unies avec un message explicite.
- Le comparateur défile horizontalement dans son propre conteneur sur mobile ;
  le reste de la page conserve sa largeur. Les boutons de création et de style
  restent accessibles sans la navigation latérale du bureau.
- Les contrôles sont nommés, les dialogues reçoivent le focus et le réglage
  de réduction des animations continue de s’appliquer.

## Suites cohérentes

Le prochain travail peut ajouter des projets enregistrés dans IndexedDB,
plusieurs univers graphiques utilisant le même contrat d’atlas, des scénarios
de référence avec plusieurs graines et une difficulté mesurée, puis des modes
de jeu guidés. Les expériences multi-agent et la vraie 3D demandent des règles
et budgets de rendu supplémentaires : elles restent dans le plan directeur.
