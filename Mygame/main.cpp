#include <SFML/Graphics.hpp>
#include "Fenetre.h"
#include "Cell.h"
#include "ChecklistItem.h"
#include "Button.h"
#include "MainController.h"
#include <vector>
#include <iostream>

int main() {
    // Dimensions dynamiques de la fenêtre
    Fenetre F(1300, 800, sf::Color::Black);
    Cell start;
    Cell end;
    bool startPointDefined = false;
    bool endPointDefined = false;

    // Initialisation du contrôleur principal
    MainController game(F, 30, 50);

    sf::RenderWindow& window = F.getWindow();

    // Boucle principale
    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // Gestion des clics souris
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                game.handleMouseClick(F, start, end, startPointDefined, endPointDefined);
            }
        }

        // Dessin
        window.clear(sf::Color::Black);  // Nettoyage de l'écran
        game.drawGrid(F);                // Dessine la grille
        game.drawChecklist(F);           // Dessine la checklist
        game.drawButtons(F);             // Dessine les boutons
        window.display();                // Affiche le contenu à l'écran
    }

    return 0;
}
