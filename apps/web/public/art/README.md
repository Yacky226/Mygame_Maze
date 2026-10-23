# Ressources graphiques — univers Ruines

Créées le 23 octobre 2026 pour Maze Studio avec le générateur d’images OpenAI.
Ce sont des ressources générées, sans téléchargement d’images de tiers.
Les fichiers WebP sont inclus dans Git et servis par le conteneur web local.
Les PNG originaux restent dans le répertoire de génération de la machine de
création ; ils ne sont pas nécessaires pour construire ou lancer le projet.

| Fichier | Usage | Dimensions | Taille |
| --- | --- | --- | --- |
| `ruins-atlas.webp` | Atlas 2 × 2 : mur, sol, herbe, roche | 1 254 × 1 254 | 647 910 octets |
| `maze-world.webp` | Illustration décorative du bandeau | 1 536 × 1 024 | 266 772 octets |

L’atlas comporte quatre rectangles égaux au chargement ; chacun fournit seize
fragments de texture pour éviter un motif trop fin à l’échelle d’une case. Conserver sa
structure lors du remplacement : haut gauche mur, haut droite sol, bas gauche
terrain végétal, bas droite terrain rocheux. Le rendu n’utilise pas de détourage.
Le nom de fichier est stable : le serveur demande une revalidation des fichiers
`art/` pour qu’un remplacement soit visible après reconstruction et rechargement.

Brief de l’atlas : matières vues de dessus, pierre sombre bleu-vert avec mousse,
grès beige, herbe olive et roche ocre, style jeu de stratégie peint, lumière
uniforme, sans texte ni interface. Brief du bandeau : diorama isométrique d’un
labyrinthe sur une île de pierre, murs bleu-vert, chemin lumineux lime, balise
bleue, fond forêt sombre avec espace calme à gauche pour le titre.

Les PNG ont été réencodés en WebP qualité 0,88 sans changement de dimensions.
L’atlas passe de 3 879 172 à 647 910 octets. Les deux ressources livrées totalisent
914 682 octets. Le script facultatif utilise Chromium de Playwright déjà présent
pour les tests ; il ne participe pas à la construction de production :

```powershell
cd apps/web
node scripts/optimize-art.mjs chemin/source.png public/art/destination.webp
```

Remplacer une ressource demande une reconstruction du client. Aucun secret ni
appel au service de génération n’est intégré au navigateur.
