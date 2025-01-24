#include "Animated.h"

void Animated::animatePath(const std::vector<Cell*>& path, std::vector<Button*>& Buttons,std::vector<ChecklistItem>& checklist, Cell* end,Fenetre &F, float durationPerCell, std::vector<std::vector<Cell>>& grid)
{
    sf::Clock clock;
	sf::RenderWindow& window = F.getWindow();

    for (size_t i = 0; i < path.size(); ++i)
    {
        clock.restart();
        float elapsed = 0.f;

        // Animation progressive de chaque cellule du chemin
        while (elapsed < durationPerCell)
        {
            elapsed = clock.getElapsedTime().asSeconds();
            float progress = elapsed / durationPerCell;


            sf::Color startColor = sf::Color(255, 0, 0);
            sf::Color endColor = sf::Color(0, 255, 0);
            if (path[i]->getType() != CellType::StartPoint && path[i]->getType() != CellType::EndPoint) {
                path[i]->cel.setFillColor(sf::Color(
                    static_cast<sf::Uint8>(startColor.r + progress * (endColor.r - startColor.r)),
                    static_cast<sf::Uint8>(startColor.g + progress * (endColor.g - startColor.g)),
                    static_cast<sf::Uint8>(startColor.b + progress * (endColor.b - startColor.b)),
                    static_cast<sf::Uint8>(startColor.a + progress * (endColor.a - startColor.a))
                ));
            }
            // Afficher la cellule mise à jour
            window.clear(sf::Color::Black); // Nettoie la fenêtre
			DrawInterface::drawgrid(F, grid); // Dessine la grille
			DrawInterface::drawChecklist(F, checklist); // Dessine la checklist
			DrawInterface::drawButtons(F, Buttons); // Dessine les boutons
			window.display(); // Met à jour l'affichage

          
        }

        // Finalisation de la cellule
        if (path[i]->getType() != CellType::StartPoint && path[i]->getType() != CellType::EndPoint) {
            path[i]->cel.setFillColor(sf::Color(50, 205, 50)); // Vert clair
        }

    }

}

void Animated::animateCels(const std::vector<Cell*>& visitedOrder,std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid,std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F)
{
    sf::Clock clock;
	sf::RenderWindow& window = F.getWindow();
    // Animation des cellules visitées
    for (auto cell : visitedOrder)
    {
        while (clock.getElapsedTime().asMilliseconds() < 50) {
            // Attente pour l'animation
        }
        clock.restart();
        if (cell->getType() != CellType::StartPoint && cell->getType() != CellType::EndPoint)
            cell->updateColor(sf::Color(200, 100, 50)); // Couleur pour les cellules visitées

        // Rafraîchir l'affichage
        window.clear(sf::Color::Black);
		DrawInterface::drawgrid(F, grid); // Dessine la grille
		DrawInterface::drawChecklist(F, checklist); // Dessine la checklist
		DrawInterface::drawButtons(F, Buttons); // Dessine les boutons
        window.display();
    }

    

}

