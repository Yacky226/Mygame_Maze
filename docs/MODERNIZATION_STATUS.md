# État de la modernisation — Maze Studio

Ce fichier décrit les changements effectivement livrés. Le catalogue complet
des modes et des évolutions reste dans le [plan directeur](../PLAN_DIRECTEUR_MAZE_STUDIO.md).

## Deuxième lot : atelier web

L'atelier est accessible sur **http://localhost:8080** avec le service Docker
`web`. Il utilise le même moteur C++ compilé en WebAssembly, dans un Worker.
Le [guide web](WEB_STUDIO.md) décrit son fonctionnement et ses limites.

| Domaine | Livré dans l'atelier web |
| --- | --- |
| Interface | React/TypeScript, rendu PixiJS à la demande, disposition adaptée au mobile, ressources et polices locales |
| Éditeur | Pinceau de murs, gomme, placement départ/arrivée, terrains pondérés, interpolation des gestes rapides |
| Carte | Dimensions de 2 à 256 cases par côté, carte vide, obstacles aléatoires et cinq scénarios |
| Déplacements | 4 ou 8 voisins et choix explicite du franchissement des coins |
| Recherche | A*, Dijkstra, BFS et DFS du moteur commun ; refus de BFS sur coûts non uniformes |
| Lecture | Lancer, pause confirmée par le Worker, reprise, pas à pas, vitesse et annulation |
| Résultats | Chemin, coût, garantie d'optimalité, développements, découvertes, frontière et temps |
| Historique | Annulation de 32 modifications de carte, y compris Ctrl+Z depuis la grille |
| Fichiers | Export/import `.maze-studio.json` version 1, validation et limite de 2 Mio |
| Compilation | SDK Emscripten et Node versionnés, dépendances npm exactes avec lockfile, construction Docker en plusieurs étapes |
| Qualité | Tests métier, comparaison CLI native/WASM, parcours Playwright et workflow web ajouté |

Les limites restantes sont explicites : une recherche par onglet, 65 536 cases,
pas d'enregistrement automatique, pas encore de nouveaux algorithmes avancés,
de comparaison simultanée, de multi-agent ni de bibliothèque de thèmes.
Le format de fichier représente cet atelier ; il ne réalise pas encore tout
le format de projet prévu dans le plan directeur.

### Validation du deuxième lot — 22 octobre 2026

- Compilation WebAssembly Release avec Emscripten 6.0.10 : réussie. Le chargeur
  et le binaire représentent environ 112 Ko non compressés.
- **9 tests Node réussis** : édition, import, gestes, cartes reproductibles,
  coûts et coins, progression, annulation, limites et parité native/WASM.
- Parité vérifiée sur 24 cartes pondérées × 2 algorithmes × 2 voisinages, soit
  **96 comparaisons** de statut, coût, chemin et développements.
- **8 tests Playwright réussis en 17 secondes** sur le conteneur de production :
  chargement réel du moteur, recherches et relances, règles de déplacement,
  BFS/DFS, pause/pas à pas/annulation, dessin/Ctrl+Z/import-export, budgets,
  absence de chemin et disposition mobile.
- Construction Docker avec vérification TypeScript, tests du modèle et build
  client réussis ; service `web` démarré avec contrôle de santé.
- Le navigateur de test est celui fourni par Playwright 1.64.1. Les captures
  GPU automatiques des traces ont été désactivées après des blocages observés
  sur le rendu logiciel ; les captures de contrôle explicites restent actives.

Ces vérifications sont locales. Le workflow web ajouté n'a pas encore été
exécuté à distance sur GitHub.

## Fondations natives : premier lot

Les sections suivantes décrivent le socle déjà livré avant l'atelier web.

### Objectif du lot

Disposer d'un moteur fiable et portable avant la nouvelle interface. Le même
code de recherche sert au jeu SFML et aux exécutions sans fenêtre. Les prochains
modes pourront ainsi réutiliser les règles et être vérifiés automatiquement.

## Livré

| Domaine | Réalisation | Où l'utiliser |
| --- | --- | --- |
| Moteur | Bibliothèque C++20 sans SFML, carte séparée des recherches | `core/` |
| Recherche | BFS, DFS, Dijkstra et A* réécrits ; relaxation et priorité correctes | Jeu et CLI |
| Cartes | Murs, terrains pondérés, déplacements à 4 ou 8 voisins, choix de franchissement des coins | API et CLI ; l'écran existant reste à 4 voisins et poids uniformes |
| Précision | Coûts entiers 64 bits, validation des chemins, heuristique en coordonnées de grille | Moteur et sortie CLI |
| Sessions | Instantané de carte par recherche, progression par nombre d'opérations, annulation, plafond d'expansions | API C++ ; commandes graphiques de pause/reprise à venir |
| Mesures | Cases découvertes et développées, relaxations, taille maximale de frontière, entrées périmées et durée active | Résultats C++ et JSON de la CLI |
| Automatisation | Scénarios texte bornés, résultats JSON, codes de sortie documentés | `apps/cli/`, `scenarios/` |
| Interface actuelle | Même moteur que la CLI, relance en conservant départ/arrivée/murs, clic affectant une seule cellule | Jeu SFML |
| Ressources | Propriété des boutons et de la police explicite ; sources UTF-8 | Interface SFML et projet Visual Studio |
| Construction | CMake, presets sans interface, dépendance de test verrouillée, tests pendant la construction Docker | Racine et `cmake/` |
| Qualité | Tests métier et comparaison avec un oracle indépendant ; workflow Windows/Linux ajouté | `core/tests/`, `apps/cli/tests/`, `.github/workflows/core.yml` |

Le [guide du moteur](ENGINE.md) contient les commandes, le format des scénarios,
les garanties des algorithmes et les limites techniques. La
[décision d'architecture](adr/0001-extract-core-first.md) explique les choix.

## Vérifications effectuées le 22 octobre 2026

- Windows, Visual Studio 2022, construction du moteur et de la CLI sans SFML :
  **21 tests CTest réussis**.
- Linux, Ubuntu 24.04, GCC 13, construction Release dans Docker :
  **21 tests CTest réussis**, puis installation du jeu SFML et de la CLI.
- Le corpus vérifie notamment 80 petites cartes déterministes avec pondération,
  connexité et règles de coins variées contre un oracle Bellman-Ford indépendant.
- Les contrats CLI vérifient 12 combinaisons de résultats, erreurs, budgets et
  diagonales ; le scénario pondéré retourne bien un coût de `6000`.
- Parcours navigateur réel avec Playwright et noVNC : les quatre algorithmes,
  quatre changements d'algorithme sur la même carte puis une relance A*,
  conservation des extrémités et des murs, carte aléatoire et nettoyage.
- Conteneur relancé avec `docker compose up -d --wait`, état `healthy`.

Les sanitizers sont configurés dans le workflow Linux ; cette validation locale
ne constitue pas une exécution des sanitizers ni une exécution distante de la CI.

## Limites de cette livraison

- Les graphismes, les thèmes et les commandes de l'écran SFML restent ceux de
  l'interface actuelle. Les terrains pondérés et diagonales se testent dans la CLI.
- L'animation SFML conserve sa boucle synchrone. Le moteur propose des étapes
  bornées en opérations, mais l'écran n'offre pas encore pause, reprise ou annulation.
- Les quatre algorithmes existants sont fiabilisés. Aucun nouvel algorithme de
  recherche avancée n'est annoncé comme livré dans ce lot.
- Le format texte sert aux premiers scénarios ; il ne représente pas le futur
  format complet de projet JSON avec version, thèmes, agents et historique.
- Le workflow CI est fourni dans le dépôt. Son exécution sur GitHub dépend de la
  publication du code et ne découle pas de sa seule présence locale.

## Prochain lot proposé : génération et comparaison

1. Ajouter des générateurs de labyrinthes avec graine : backtracker, Prim et
   Kruskal, en conservant la reproductibilité des cartes.
2. Constituer un catalogue de scénarios avec difficulté, densité, pondération
   et conditions de déplacement documentées.
3. Comparer plusieurs algorithmes sur le même instantané : coût, développements,
   mémoire estimée et temps actif, avec un protocole de mesure répétable.
4. Introduire de nouveaux algorithmes avec leurs conditions de validité et leurs
   garanties, puis les vérifier contre l'oracle et les baselines existantes.
5. Étendre la persistance et la gestion des projets avant les modes multi-agent,
   les thèmes multiples et les expériences avancées du plan directeur.

Le critère de fin sera une expérience reproductible : générer ou charger une
carte, exécuter plusieurs recherches comparables, comprendre leurs compromis
et exporter les données nécessaires pour retrouver le résultat.
