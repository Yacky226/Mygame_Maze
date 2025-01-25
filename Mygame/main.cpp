#include "Fenetre.h"          // Gestion de la fen�tre personnalis�e
#include "Cell.h"             // Repr�sentation des cellules (par exemple, dans une grille)
#include "MainController.h"   // Contr�leur principal pour g�rer les interactions et le flux

int main() {
    // Cr�ation de la fen�tre avec des dimensions dynamiques
    Fenetre F(1300, 800);  // Fen�tre de 1300x800 pixels
    Cell start;            // Cellule de d�part (start)
    Cell end;              // Cellule de fin (end)
    bool startPointDefined = false;  // Indique si le point de d�part est d�fini
    bool endPointDefined = false;    // Indique si le point de fin est d�fini

    // Initialisation du contr�leur principal 
    MainController game(F, 30, 50);  // Grille de 30 lignes et 50 colonnes

    // Obtention de la fen�tre SFML
    sf::RenderWindow& window = F.getWindow();

    // Boucle principale : le c�ur du programme
    while (window.isOpen()) {  // Tant que la fen�tre est ouverte
        sf::Event event;       // Objet pour g�rer les �v�nements
        while (window.pollEvent(event)) {  // R�cup�ration des �v�nements
            // Si l'utilisateur ferme la fen�tre
            if (event.type == sf::Event::Closed)
                window.close();

            // Si un clic gauche de souris est d�tect�
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                // Gestion du clic de la souris 
                game.handleMouseClick(F, start, end, startPointDefined, endPointDefined);
            }
        }

        // Phase de dessin sur la fen�tre
        window.clear(sf::Color::Black);  // Nettoie l'�cran avec un fond noir
        game.drawGrid(F);                // Dessine la grille 
        game.drawChecklist(F);           // Dessine une checklist interactive 
        game.drawButtons(F);             // Dessine les boutons interactifs 
        window.display();                // Affiche le contenu dessin� sur l'�cran
    }

    return 0;  // Fin du programme
}
