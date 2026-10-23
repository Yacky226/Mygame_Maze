# Moteur Maze Studio — C++20, CLI et génération

Le cœur `maze_core` ne connaît ni SFML, ni les pixels, ni les polices. Il contient
la carte, les règles de déplacement, cinq recherches, trois générateurs et la validation des
chemins. Le jeu existant l'utilise via `Mygame/Pathfinding.cpp`.

## Construire sans interface graphique

Prérequis : CMake 3.21 ou plus et compilateur C++20, par exemple Visual Studio
2022 ou GCC 13. L'ancien MinGW GCC 6.3 n'est pas adapté à ce nouveau cœur.

```powershell
cmake --preset core-debug
cmake --build --preset core-debug
ctest --preset core-debug
```

Avec Visual Studio, les exécutables se trouvent dans
`build/core-debug/apps/cli/Debug/`. Avec un générateur à configuration unique,
ils se trouvent directement dans `build/core-debug/apps/cli/`.

La première configuration des tests télécharge Catch2 si la version verrouillée
n'est pas déjà disponible. L'archive possède une empreinte vérifiée. Le cœur de
production n'a aucune dépendance externe ; `-DBUILD_TESTING=OFF` permet de le
compiler sans téléchargement de Catch2.

## Exemple de CLI

```powershell
.\build\core-debug\apps\cli\Debug\maze_cli.exe scenarios/weighted-detour.maze.txt --algorithm astar
```

Dans le conteneur du jeu, une fois la nouvelle image construite :

```powershell
docker compose exec -T maze ./maze_cli scenarios/weighted-detour.maze.txt --algorithm astar
```

Carte de l'exemple :

```text
.....
S999G
.....
```

Le trajet direct traverse du terrain cher. Dijkstra et A* trouvent un détour de
6 mouvements orthogonaux, coût **6000**. Les coûts sont exprimés en milli-pas :
1000 pour un mouvement orthogonal, 1414 pour une diagonale, multipliés par le
facteur de la case d'arrivée. `1414` est une approximation entière de la diagonale.

Le JSON conserve les coûts 64 bits sous forme de chaînes, l'algorithme, le statut,
les coordonnées du chemin et les statistiques. `computeActiveNs` mesure les
phases de travail de la recherche et de sa trace éventuelle ; il exclut la
préparation/copie de carte, la lecture du fichier et toute animation. Cette durée
murale ne constitue pas automatiquement le temps CPU du processus.

## Format texte transitoire

- `#` : mur ; `.` : sol de poids 1.
- `1` à `9` : terrain de ce poids.
- `S` : départ ; `G` : arrivée ; `B` : départ égal à l'arrivée.
- Les lignes sont non vides et possèdent la même largeur. LF et CRLF sont acceptés.
- Une seule paire d'extrémités, ou un unique `B`, est requise.
- Limites : 1 048 576 cases et fichier de 4 Mio maximum.

Options : `--algorithm bfs|dfs|dijkstra|astar|greedy`, `--diagonal`,
`--allow-corner-cutting`, `--max-expansions N`. La limite zéro signifie absence
de plafond d'expansions supplémentaire. `--help` décrit les codes de sortie.
Les diagonales interdisent de couper les coins par défaut.

Le binaire `maze_generate` produit des scénarios ASCII reproductibles :
`--generator backtracker|prim|kruskal --seed N --width N --height N`.
Le [guide du troisième lot](GENERATION_COMPARISON_VISUALS.md) décrit le contrat
de génération version 1 et les limites. Greedy Best-First utilise seulement la
priorité heuristique ; il peut trouver un chemin plus coûteux que Dijkstra/A*.

Codes de sortie : 0 succès, 2 entrée/configuration invalide, 3 absence de chemin,
4 limite de ressources, 5 erreur interne.

Ce format est limité aux scénarios de départ. Il n'implémente pas encore les
projets, agents, thèmes ou événements du futur format JSON versionné.

## API du cœur

```cpp
maze::GridMap map(10, 8);
maze::SearchRequest request;
request.algorithm = maze::Algorithm::AStar;
request.start = map.node_at(0, 0);
request.goal = map.node_at(9, 7);

maze::SearchSession session(map, request); // copie propriétaire de l'instantané
while (session.step(128) == maze::SearchStatus::Running) {
    // Rendre la main au futur contrôleur entre ces appels.
}
const auto& result = session.result();
```

`step` borne le nombre de retraits de frontière, y compris les entrées périmées.
Il ne promet pas encore une borne temporelle stricte : l'allocation, la
reconstruction finale du chemin et l'ordonnancement de l'OS restent variables.
`cancel()` est idempotente et n'altère pas un résultat déjà finalisé.

Un Worker devra ordonnancer ces étapes et traiter ses messages entre les appels.
Le moteur seul n'ajoute pas de threads. L'interface SFML lance encore le calcul
et la lecture de façon synchrone ; pause/reprise graphique arriveront avec
le contrôleur de lecture.

## Garanties et validation

BFS refuse les coûts non uniformes. DFS peut retourner un chemin sans garantir
son coût minimal. Dijkstra et A* utilisent relaxation, entrées de priorité figées
et extraction valide du but. A* propose heuristique automatique admissible ou
nulle ; aucune heuristique utilisateur arbitraire n'est exposée dans ce lot.

Les tests couvrent les coûts analytiques, un objectif amélioré après découverte,
les diagonales et coins, les recherches indépendantes, l'annulation, les budgets,
les coûts dépassant 32 bits et un corpus déterministe de petites cartes vérifié
avec un oracle distinct.

La configuration GitHub Actions décrit les contrôles natifs Windows et Linux,
avec sanitizers sous Linux. Son ajout local ne signifie pas qu'une exécution
distante de ce workflow a déjà eu lieu.

## Interface web

Le moteur possède désormais une passerelle WebAssembly et un atelier web avec
Worker, éditeur, métriques, pause, pas à pas et annulation. Le
[guide web](WEB_STUDIO.md) décrit sa construction, le protocole et les tests de
parité avec la CLI. Le [suivi de modernisation](MODERNIZATION_STATUS.md) distingue
les fonctions livrées des capacités encore prévues dans le plan directeur.
