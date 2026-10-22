# Maze / Mygame

Atelier de recherche de chemin avec un moteur C++20, une interface web
React/TypeScript/PixiJS et le jeu SFML historique. BFS, DFS, Dijkstra et A*
partagent le même moteur.

## Atelier web

```powershell
docker compose up --build -d --wait web
```

Ouvrir **[Maze Studio](http://localhost:8080)** : dessin des murs et terrains,
départ/arrivée, diagonales, scénarios, pause, pas à pas, vitesse, résultats et
import/export JSON. Les recherches s'exécutent dans un Worker WebAssembly.
Le premier build télécharge le SDK et peut prendre plusieurs minutes.

Voir [le guide de l'atelier web](docs/WEB_STUDIO.md) pour les raccourcis,
le développement, les tests et les limites du format de projet.

Le [plan directeur Maze Studio](PLAN_DIRECTEUR_MAZE_STUDIO.md) décrit en détail
la cible du projet : choix technologiques, architecture, modes, graphismes,
algorithmes, qualité, migration et backlog avec critères d'acceptation.
Il distingue les fonctions actuelles des évolutions proposées.

## Modernisation en cours

Le premier lot introduit un moteur **C++20 indépendant de SFML**, des recherches
isolées et testables, les terrains pondérés dans le moteur, une CLI et des tests
Catch2. Le jeu existant utilise ce moteur ; départ, arrivée et murs sont conservés
entre les traitements. Voir [le guide du moteur](docs/ENGINE.md) et
[la décision d'architecture](docs/adr/0001-extract-core-first.md).
Le [suivi de modernisation](docs/MODERNIZATION_STATUS.md) précise ce qui est livré
et le contenu du prochain lot.

## Lancer avec Docker

Prérequis : Docker Desktop démarré en mode **conteneurs Linux**, avec Docker
Compose v2. Le mode de développement automatique nécessite Compose 2.22 ou plus.
Visual Studio, SFML et un serveur X sur Windows ne sont pas nécessaires pour
ce lancement.

Depuis le dossier contenant `compose.yaml` :

```powershell
docker compose up --build -d --wait
```

Ouvrir [le jeu dans le navigateur](http://localhost:6080/vnc.html?autoconnect=true&resize=scale).
Le premier lancement télécharge les dépendances et compile le projet ; il peut
prendre plusieurs minutes. Si noVNC affiche un bouton « Connect », cliquer dessus.

Le programme reste une application SFML native : Xvfb fournit l'écran virtuel,
x11vnc transmet l'affichage, et noVNC permet de le voir et de cliquer depuis le
navigateur. Le rendu OpenGL logiciel fonctionne sans configuration GPU.
Une même instance est partagée par les onglets connectés.

Le port est publié uniquement sur `127.0.0.1`. L'accès local ne demande pas de
mot de passe ; cette configuration est destinée au développement sur ta machine.

Commandes utiles :

```powershell
# État du service et contrôle de santé
docker compose ps

# Journaux
docker compose logs -f maze

# Arrêter le jeu
docker compose down

# Relancer après des modifications, sans mode automatique
docker compose up --build -d --wait
```

Si le port 6080 est occupé, choisir un autre port dans PowerShell :

```powershell
$env:MAZE_PORT = "6081"
docker compose up --build -d --wait
```

Le jeu sera alors accessible sur
`http://localhost:6081/vnc.html?autoconnect=true&resize=scale`.

## Développer avec Docker

```powershell
docker compose up --build --watch
```

Modifier les fichiers C++ dans `Mygame/` avec ton éditeur habituel. À chaque
enregistrement, Compose reconstruit l'image et redémarre le jeu. La connexion
noVNC peut être interrompue pendant le redémarrage : reconnecter ou actualiser
l'onglet. **La grille en cours est perdue lors d'un redémarrage.**
`Ctrl+C` arrête ce mode.

Les dépendances Linux sont conservées dans le cache Docker entre deux builds.
Les bibliothèques Windows `Mygame/lib`, les en-têtes embarqués, les sorties de
Visual Studio et les vidéos de démonstration sont exclus du contexte Docker.
Les fichiers Windows du projet ne sont pas montés dans le conteneur et aucun
binaire Linux n'est écrit dans le dossier source.

Les sources ont été migrées vers UTF-8. Utiliser cet encodage pour les nouveaux
fichiers et un compilateur C++20 pour le moteur et son adaptateur SFML.

## Utilisation du jeu

1. Cliquer sur une case vide pour placer le départ (vert).
2. Cliquer sur une autre case pour placer l'arrivée (rouge).
3. Cliquer sur d'autres cases pour ajouter des murs ; recliquer pour les retirer.
4. Cocher BFS, DFS, Dijkstra ou A*, puis cliquer sur **Traitement**.
5. Les cases explorées deviennent orange et le chemin trouvé apparaît en vert.

**Map Aleatoire** remplace la grille par environ 25 % de murs. Placer ensuite
le départ et l'arrivée. **Clean Map** efface toute la grille.

Il est possible de relancer un algorithme ou d'en changer sur la même carte :
chaque recherche possède un contexte neuf et efface l'ancienne visualisation.
**Clean Map** reste réservé à l'effacement complet de la carte.
Les animations bloquent les interactions jusqu'à leur fin. Ces comportements
existants restent à améliorer.

### Performances de l'affichage

La grille est dessinée en un seul appel graphique et l'écran n'est rafraîchi
que lorsqu'il change. Les animations avancent selon le temps écoulé, par lots
de cellules, à 30 images/s maximum, sans attente active. Chaque phase vise une
durée maximale de trois secondes, même pour les longs chemins DFS ; la durée
réelle dépend aussi de la charge de la machine. La fenêtre peut être fermée
pendant l'animation ; les autres commandes attendent sa fin.

Le navigateur affiche toujours une fenêtre distante via noVNC. Pour une future
version principalement destinée au Web, une interface Canvas/WebGL permettrait
d'éviter cette transmission d'écran et demanderait une adaptation de l'affichage.

## Compiler sans Docker sous Linux

SFML **2.6.x** est utilisé dans Docker (Ubuntu 24.04). SFML 3 demande une
migration du code et n'est pas compatible avec cette configuration.

```bash
sudo apt-get update
sudo apt-get install build-essential cmake libsfml-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 2
cd build
./maze
```

Le lancement natif nécessite un affichage graphique. CMake copie `Fonts/`
dans le dossier de compilation. La solution Visual Studio `Mygame.sln` reste
disponible pour le développement Windows.

## Évolutions proposées

L'objectif conseillé est un **outil de comparaison d'algorithmes**, avec des
résultats mesurables sur les mêmes cartes.

1. **Fiabiliser les recherches.** Réinitialiser `visited`, `distance` et
   `parent` avant chaque exécution sans effacer les murs ; conserver le départ
   et l'arrivée. Corriger A* : calculer Manhattan avec les indices de grille,
   et autoriser la mise à jour d'un meilleur coût. Ajouter des tests sur des
   cartes connues, y compris sans solution.
2. **Séparer le calcul de l'affichage.** Déplacer la logique de lancement de
   `Button` vers le contrôleur, puis faire progresser l'animation dans la boucle
   principale. Ajouter pause, pas à pas et réglage de vitesse. Cela permettra
   aussi de mesurer le temps de calcul indépendamment des animations.
3. **Comparer les résultats.** Afficher le temps de calcul, le nombre de cases
   explorées, la longueur et le coût du chemin. Utiliser des cartes reproductibles
   avec une graine aléatoire et exporter les résultats en CSV.
4. **Ajouter BFS bidirectionnel et Greedy Best-First.** Le premier permettra
   d'étudier une recherche depuis les deux extrémités ; le second illustrera
   le compromis entre exploration guidée et qualité du chemin, sans garantie
   du plus court chemin.
5. **Introduire des terrains pondérés.** Donner des coûts différents aux cases
   (route, sable, eau) pour rendre la comparaison Dijkstra / A* plus intéressante.
   Adapter alors Dijkstra pour finaliser une case à sa sortie de la file de
   priorité et permettre les améliorations de coût avant cette étape.
   Sur la grille actuelle, tous les déplacements coûtent 1 : BFS suffit pour
   trouver un plus court chemin, tandis que DFS ne le garantit pas.
6. **Enrichir les cartes.** Génération de labyrinthes avec DFS aléatoire ou Prim,
   sauvegarde/chargement JSON, taille de grille configurable. Plus tard, étudier
   Jump Point Search sur des grilles adaptées ou D* Lite pour des obstacles
   qui changent pendant la recherche.

La configuration Docker prépare le lancement et le développement ; les
nouveaux algorithmes et les corrections fonctionnelles ci-dessus constituent
les étapes suivantes.

Documentation : [SFML sous Linux](https://www.sfml-dev.org/tutorials/2.6/getting-started/linux/),
[Compose Watch](https://docs.docker.com/compose/how-tos/file-watch/),
[noVNC](https://github.com/novnc/noVNC).

## Démonstrations et diagrammes

Le diagramme de packages, la vidéo de démonstration et les explications du jeu
se trouvent dans [Demo_et_explication_jeu](Demo_et_explication_jeu/).
