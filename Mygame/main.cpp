#include "Fenetre.h"          // Gestion de la fenêtre personnalisée
#include "Cell.h"             // Représentation des cellules (par exemple, dans une grille)
#include "MainController.h"   // Contrôleur principal pour gérer les interactions et le flux

int main() {
    // Création de la fenêtre avec des dimensions dynamiques
    Fenetre F(1300, 800);  // Fenêtre de 1300x800 pixels
    Cell start;            // Cellule de départ (start)
    Cell end;              // Cellule de fin (end)
    bool startPointDefined = false;  // Indique si le point de départ est défini
    bool endPointDefined = false;    // Indique si le point de fin est défini

    // Initialisation du contrôleur principal 
    MainController game(F, 30, 50);  // Grille de 30 lignes et 50 colonnes

    // Obtention de la fenêtre SFML
    sf::RenderWindow& window = F.getWindow();

    // Boucle principale : le cœur du programme
    while (window.isOpen()) {  // Tant que la fenêtre est ouverte
        sf::Event event;       // Objet pour gérer les événements
        while (window.pollEvent(event)) {  // Récupération des événements
            // Si l'utilisateur ferme la fenêtre
            if (event.type == sf::Event::Closed)
                window.close();

            // Si un clic gauche de souris est détecté
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                // Gestion du clic de la souris 
                game.handleMouseClick(F, start, end, startPointDefined, endPointDefined);
            }
        }

        // Phase de dessin sur la fenêtre
        window.clear(sf::Color::Black);  // Nettoie l'écran avec un fond noir
        game.drawGrid(F);                // Dessine la grille 
        game.drawChecklist(F);           // Dessine une checklist interactive 
        game.drawButtons(F);             // Dessine les boutons interactifs 
        window.display();                // Affiche le contenu dessiné sur l'écran
    }

    return 0;  // Fin du programme
}
