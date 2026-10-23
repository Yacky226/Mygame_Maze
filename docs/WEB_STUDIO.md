# Maze Studio Web — atelier local

L'atelier web utilise le même `maze_core` C++ que le jeu SFML et la CLI.
React et PixiJS gèrent l'édition et l'affichage ; les recherches sont exécutées
en WebAssembly dans un Worker dédié.

## Lancer

Depuis la racine du dépôt, avec Docker Desktop en mode Linux :

```powershell
docker compose up --build -d --wait web
```

Ouvrir **http://localhost:8080**. Le premier build télécharge le SDK Emscripten
et Node, puis compile le moteur et le client. Ces couches sont ensuite mises
en cache. Le navigateur exécute les recherches localement ; le serveur sert
uniquement les fichiers de l'application.

Changer le port si nécessaire :

```powershell
$env:MAZE_WEB_PORT = "8081"
docker compose up -d --wait web
```

Le jeu historique reste accessible séparément :

```powershell
docker compose up -d --wait maze
```

Adresse historique : http://localhost:6080/vnc.html?autoconnect=true&resize=scale.
`docker compose up --build -d --wait` lance les deux services.

## Utilisation

- **Mur / Gomme** : cliquer et glisser. Les cases entre deux événements de souris
  sont interpolées pour éviter les trous pendant un geste rapide.
- **Départ / Arrivée** : placer une extrémité sur une case ; celle-ci devient libre.
  Les outils mur/terrain/gomme protègent les extrémités.
- **Terrain** : peindre un poids 2, 3, 5 ou 9 ; le poids multiplie le coût d'entrée.
- **Algorithme** : A*, Dijkstra, BFS, DFS ou Greedy Best-First. BFS est refusé
  lorsque les coûts ne sont pas uniformes. DFS et Greedy ne garantissent pas
  le coût minimal.
- **Voisinage** : 4 ou 8 directions. En diagonale, les coins sont protégés par
  défaut ; un interrupteur permet de changer cette règle explicitement.
- **Lecture** : lancer, pause, reprendre, avancer d'une opération et annuler.
  La vitesse ajuste le nombre d'opérations par lot, de 1 à 4096.
- **Carte** : dimensions de 2 à 256 cases par côté, obstacles aléatoires, carte
  vide et cinq scénarios intégrés. Les modifications sont désactivées pendant
  une recherche ; l'annulation rend à nouveau la carte éditable.
- **Historique** : annuler les 32 dernières modifications de carte. Un geste
  continu de pinceau correspond à une seule modification.
- **Zoom** : 75 à 175 % du cadrage automatique ; défilement si nécessaire.
- **Créer un labyrinthe** : backtracker, Prim ou Kruskal, dimensions impaires
  de 3 à 255, graine visible, départ/arrivée automatiques et provenance exportée.
- **Style** : univers Ruines avec textures locales ou vue technique. Le style
  est conservé à l’export et ne change ni la carte ni le résultat des recherches.
- **Comparer les algorithmes** : un échauffement, puis 3 à 15 mesures sur le même
  instantané ; médiane/minimum/maximum, surcoût et rapports JSON/CSV.

Le [guide génération, comparaison et graphismes](GENERATION_COMPARISON_VISUALS.md)
précise les garanties, le protocole de mesure et les ressources livrées.

Raccourcis : M mur, E gomme, D départ, A arrivée, T terrain, Espace lancer/pause,
Ctrl+Z annuler une modification. Le canvas est également accessible au clavier :
flèches pour déplacer le curseur, Espace ou Entrée pour appliquer l'outil.
Les raccourcis globaux n'interceptent pas la saisie dans les champs.

## Résultats et temps

Le coût est affiché en pas : un mouvement orthogonal vaut 1 ; une diagonale
vaut 1,414 avant pondération. Le format interne reste un entier 64 bits en
milli-pas. Le résultat conserve les garanties propres à chaque algorithme.

« Développements » compte les extractions valides du moteur. Le pas à pas borne
les retraits de frontière ; une entrée périmée peut donc consommer une opération
sans augmenter ce compteur. Une recherche limitée par un budget ne prouve pas
l'absence de solution.

« Calcul actif » reprend `computeActiveNs` du cœur : il exclut la copie initiale
de carte, les messages, le rendu et les pauses. « Temps de session » inclut le
temps depuis le lancement, donc la lecture et les pauses. Ces mesures ne sont
pas interchangeables et ne remplacent pas un protocole de benchmark.

## Import/export

Le bouton Exporter télécharge un fichier `.maze-studio.json` contenant :

```json
{
  "schemaVersion": 1,
  "title": "Un exemple",
  "board": {
    "width": 2, "height": 2,
    "blocked": [0, 0, 0, 0],
    "weights": [1, 1, 1, 1],
    "start": 0, "goal": 3
  },
  "settings": {
    "algorithm": "astar",
    "connectivity": 4,
    "allowCornerCutting": false,
    "maxExpansions": 0
  }
}
```

Les cases sont numérotées par ligne : `id = y * width + x`. L'import valide la
version, la taille (2 Mio), les dimensions, les longueurs des tableaux, chaque
valeur, les extrémités et les paramètres. Les poids importés peuvent aller de 1
à 1000, conformément au moteur. Un fichier refusé ne remplace pas la carte.

Ce premier format représente l'atelier courant. Il ne contient pas encore les
agents multiples, bibliothèques de thèmes, scripts, historiques de résultats ou migrations du
futur format complet décrit dans le plan directeur. Le travail reste en mémoire
dans l'onglet : **exporter avant de recharger**. Aucun compte n'est nécessaire.

Depuis la version web 0.3, le champ facultatif `board.generation` conserve
algorithme, graine, version de génération et indicateur de modification.
`appearance.view` conserve `technical` ou `ruins`. Les projets version 1 sans
ces champs restent acceptés. Le rapport du comparateur possède son propre
contenu ; il ne s’importe pas comme un fichier projet.

## Développer sous Windows

Prérequis : Node 22.12 ou plus et Docker. Sous PowerShell, `npm.cmd` permet
d'utiliser npm sans modifier la politique d'exécution des scripts de la machine.

```powershell
cd apps/web
npm.cmd ci
npm.cmd run wasm
npm.cmd run dev
```

La commande `wasm` utilise Docker et exporte les deux artefacts générés dans
`public/engine/`. Les chemins contenant des espaces sont transmis comme arguments
séparés. Après une modification de `core/` ou `bindings/`, relancer cette commande.
Vite recharge le TypeScript et le CSS en développement.

Pour reconstruire automatiquement la version Docker :

```powershell
docker compose watch web
```

Les répertoires générés, `node_modules` et les résultats des tests sont exclus
du contexte Docker et de la surveillance.

## Architecture et limites

```text
React : carte, outils, paramètres, contrôles
    → Worker : runId, démarrage, pause/reprise, lots, annulation
        → Embind : validation de frontière et conversion des données
            → maze_core : carte, session, recherches et vérification du chemin
    ← deltas des développements, statut, métriques, chemin final
PixiJS : rendu à la demande, calques carte / exploration / chemin / repères
```

Chaque recherche reçoit un identifiant croissant. Le contrôleur ignore les
réponses d'une ancienne recherche après un effacement, une annulation ou un
nouveau lancement. Le Worker rend la main entre les lots pour traiter ses
messages. La pause est confirmée par le Worker avant d'être affichée.

Les lots sont bornés à 4096 retraits et espacés de 24 ms. Les développements
ne sont envoyés qu'une fois ; le chemin complet est transmis en fin de succès.
L'intervalle borne la fréquence prévue des messages, pas le délai absolu garanti
par le navigateur. La préparation de la carte et la reconstruction finale ne
sont pas découpées en opérations.

Limites web : 65 536 cases, mémoire WASM plafonnée à 256 Mio, une recherche active
par onglet. Le moteur ne crée pas de threads. Le rendu demande WebGL ; une erreur
explicite s'affiche si le navigateur ne peut pas l'initialiser. Les polices et
ressources sont incluses dans les fichiers servis localement.

La construction est verrouillée par le tag du SDK Emscripten 6.0.10, le tag Node
22.20.0 et `package-lock.json` avec versions npm exactes. Les mises à jour des
outils doivent être accompagnées des tests de parité et du parcours navigateur.
Le cœur WASM n'a aucune dépendance C++ externe ; vcpkg reste une évolution de
la chaîne native, sans ajout nécessaire pour compiler ce lot.

Le service utilise un utilisateur sans privilèges, un port publié sur loopback
et une politique CSP. PixiJS utilise son adaptateur sans génération dynamique de
JavaScript ; le navigateur autorise séparément la compilation WebAssembly.
Les licences des polices sont disponibles sous `/licenses/` dans l'application
construite. Cette configuration est destinée au développement local.

## Vérifier

Construire d'abord la CLI native selon [ENGINE.md](ENGINE.md), puis :

```powershell
cd apps/web
npm.cmd run wasm
npm.cmd test
npm.cmd run build
npx.cmd playwright install chromium
npm.cmd run test:e2e
```

`MAZE_CLI` peut désigner un autre exécutable CLI. Par défaut, les tests attendent
la CLI Windows Debug dans `build/core-debug/apps/cli/Debug/maze_cli.exe`.
Les tests de parité comparent statut, coût, chemin et développements entre le
module WASM et le binaire natif sur 24 cartes pondérées, trois algorithmes et deux
voisinages (144 comparaisons). Neuf configurations de génération vérifient aussi
la carte et les extrémités bit pour bit entre natif et WASM. `MAZE_GENERATE`
peut désigner un autre binaire générateur ; par défaut il se trouve à côté de
`MAZE_CLI`. D’autres tests couvrent édition, import, provenance, statistiques,
annulation, export, payload du benchmark et coins.

Les tests Playwright lancent l'aperçu local après le build. Pour vérifier un
conteneur déjà démarré :

```powershell
$env:MAZE_WEB_URL = "http://127.0.0.1:8080"
npm.cmd run test:e2e
```

`PLAYWRIGHT_CHROMIUM_EXECUTABLE_PATH` permet de réutiliser un Chromium déjà
installé. Les captures et traces de panne sont générées dans `test-results/`.

Les traces conservent les étapes, les sources et les échanges, sans captures
GPU automatiques ni instantanés DOM : les lectures répétées du framebuffer
perturbaient le rendu logiciel pendant les essais. Des captures explicites
contrôlent les dispositions bureau et mobile, les textures et le comparateur. Les assertions d'interaction
restent actives pendant tous les parcours.

## Références techniques

L'initialisation asynchrone et le rendu à la demande suivent
[l'API Application de PixiJS](https://pixijs.com/8.x/guides/components/application).
Le chargement du moteur utilise
[la sortie modulaire Emscripten](https://emscripten.org/docs/compiling/Modularized-Output.html)
et [Embind](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html).
