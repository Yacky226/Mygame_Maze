# ADR 0001 — Extraire un moteur portable avant de remplacer l'interface

Date : 22 octobre 2026. Statut : mis en œuvre pour le premier lot.

## Contexte

Les recherches étaient écrites dans des cellules SFML avec un état persistant,
une heuristique exprimée en pixels et un lancement directement depuis les
boutons. Une refonte graphique seule aurait conservé ces erreurs et ce couplage.

## Décision

- Créer `maze_core` en C++20 sans dépendance graphique ni dépendance de production.
- Employer des identifiants, des coûts entiers 64 bits et un contexte propriétaire
  d'un instantané de carte par recherche.
- Partager les mêmes quatre recherches entre l'adaptateur SFML et une CLI.
- Exposer des étapes bornées et l'annulation, préparant une future frontière WASM.
- Conserver l'affichage existant pendant ce premier lot ; les étapes du cœur
  ne rendent pas encore les animations SFML asynchrones.
- Tester les coûts avec des exemples analytiques et un oracle Bellman-Ford
  indépendant de la production.

## Dépendances de test

Catch2 3.7.1 est verrouillé à une référence Git immuable et à l'empreinte SHA-256 de son archive.
La configuration accepte un package Catch2 exactement compatible déjà installé.
À défaut, CMake FetchContent fournit un bootstrap exclusivement destiné aux tests.
`BUILD_TESTING=OFF` ne télécharge aucune dépendance de test.

Dans le bootstrap natif et l'étape de dépendances Docker, la distribution
officielle amalgamée de Catch2 est compilée en une unité plutôt qu'en plus de
cent petits fichiers.
Les cibles et en-têtes standards restent utilisés par les tests ; le `main`
provient de `Catch2WithMain`. Cette optimisation est spécifique à la version
verrouillée et ne modifie pas le moteur de production.
`MAZE_AMALGAMATED_TEST_DEPS=OFF` permet d'utiliser la construction classique.

Ce bootstrap est une étape intermédiaire par rapport au plan directeur qui prévoit
vcpkg en mode manifeste. Le lot web ajoute un SDK Emscripten versionné ; comme
le cœur WASM n'a aucune dépendance C++ externe, la baseline vcpkg reste une
évolution de la chaîne native. Le lot web n'a pas besoin d'un second bootstrap.
Il n'existe pas deux gestionnaires de dépendances de production concurrents.

## Conséquences et limites

Le cœur est testable sans affichage et sans SFML. L'adaptateur copie la petite
grille actuelle avant chaque recherche ; cette copie rend les exécutions
indépendantes et pourra être optimisée si les mesures le justifient.

La CLI reçoit provisoirement une carte texte et exporte du JSON. Ce format
d'entrée n'est pas le futur format de projet `.maze.json` et ne remplace pas
les schémas et migrations prévus.

Les anciennes propriétés de recherche de `Cell` restent pour compatibilité mais
ne pilotent plus les algorithmes. Leur suppression accompagnera le remplacement
de l'adaptateur et du contrôleur de l'interface.

## Validation attendue

Construction sans SFML sous Windows et Linux, corpus métier, diagonales et
pondération vérifiés, recherches successives dans l'interface, exécution de la CLI
sans serveur graphique et absence de nouvelle allocation propriétaire brute
pour les boutons et polices.
