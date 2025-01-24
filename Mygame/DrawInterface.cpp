#include "DrawInterface.h"
void DrawInterface::drawgrid(Fenetre& F, std::vector<std::vector<Cell>>& grid)
{
	sf::RenderWindow& window = F.getWindow();
	for (auto& row : grid) {
		for (auto& cell : row) {
			window.draw(cell.cel);
		}
	}
}
// Dessiner la checklist
void DrawInterface::drawChecklist(Fenetre& F,std::vector<ChecklistItem>& checklist)
{
    sf::RenderWindow& window = F.getWindow();
    for (auto& item : checklist) {
        item.draw(window); // Dessine chaque élément de la checklist
    }
}

// Dessiner les boutons
void DrawInterface::drawButtons(Fenetre& F, std::vector<Button*>& Buttons)
{
    sf::RenderWindow& window = F.getWindow();
    for (auto& button : Buttons) {
        button->draw(window); // Dessine chaque bouton
    }
}
// Générer une carte aléatoire
void DrawInterface::generateRandomMap(std::vector<std::vector<Cell>>& grid)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Initialisation

    for (auto& row : grid)
    {
        for (auto& cell : row)
        {
            if (std::rand() % 4 == 0) { // 25% de chances de placer un mur
                cell.clean();
                cell.setType(CellType::Wall);
            }
            else
            {
                cell.clean();
                cell.setType(CellType::None); // Cellule vide
            }
            cell.setdistance(std::numeric_limits<float>::infinity()); // Réinitialisation des distances
        }
    }

}


// Nettoyer la carte

void DrawInterface::cleanMap(std::vector<std::vector<Cell>>& grid)

{
    for (auto& row : grid) {
        for (auto& cell : row) {
            cell.clean();
        }
    }
}