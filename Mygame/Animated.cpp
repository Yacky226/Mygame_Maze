#include "Animated.h"

// Fonction pour animer le chemin final trouv�
void Animated::animatePath(const std::vector<Cell*>& path, std::vector<Button*>& Buttons, std::vector<ChecklistItem>& checklist, Cell* end, Fenetre& F, float durationPerCell, std::vector<std::vector<Cell>>& grid)
{
    sf::Clock clock; // Chronom�tre pour contr�ler l'animation
    sf::RenderWindow& window = F.getWindow(); // R�cup�rer la fen�tre de rendu

    for (size_t i = 0; i < path.size(); ++i)
    {
        clock.restart(); // Red�marrer le chronom�tre pour chaque cellule
        float elapsed = 0.f; // Temps �coul� pour cette cellule

        // Animation progressive de chaque cellule du chemin
        while (elapsed < durationPerCell)
        {
            elapsed = clock.getElapsedTime().asSeconds(); // Temps �coul� en secondes
            float progress = elapsed / durationPerCell; // Progression de l'animation entre 0 et 1

            // D�finir les couleurs de d�but et de fin de l'animation
            sf::Color startColor = sf::Color(255, 0, 0); // Rouge
            sf::Color endColor = sf::Color(0, 255, 0);   // Vert

            // Appliquer une transition de couleur si la cellule n'est ni le point de d�part ni le point d'arriv�e
            if (path[i]->getType() != CellType::StartPoint && path[i]->getType() != CellType::EndPoint) {
                path[i]->cel.setFillColor(sf::Color(
                    static_cast<sf::Uint8>(startColor.r + progress * (endColor.r - startColor.r)), // Interpolation rouge
                    static_cast<sf::Uint8>(startColor.g + progress * (endColor.g - startColor.g)), // Interpolation vert
                    static_cast<sf::Uint8>(startColor.b + progress * (endColor.b - startColor.b)), // Interpolation bleu
                    static_cast<sf::Uint8>(startColor.a + progress * (endColor.a - startColor.a))  // Interpolation alpha
                ));
            }

            // Rafra�chir l'affichage de la fen�tre
            window.clear(sf::Color::Black); // Nettoyer la fen�tre avec un fond noir
            DrawInterface::drawgrid(F, grid); // Dessiner la grille
            DrawInterface::drawChecklist(F, checklist); // Dessiner la checklist
            DrawInterface::drawButtons(F, Buttons); // Dessiner les boutons
            window.display(); // Mettre � jour l'affichage
        }

        // Finalisation de la couleur de la cellule une fois l'animation termin�e
        if (path[i]->getType() != CellType::StartPoint && path[i]->getType() != CellType::EndPoint) {
            path[i]->cel.setFillColor(sf::Color(50, 205, 50)); // Couleur vert clair pour le chemin final
        }
    }
}

// Fonction pour animer les cellules visit�es lors de l'ex�cution de l'algorithme
void Animated::animateCels(const std::vector<Cell*>& visitedOrder, std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F)
{
    sf::Clock clock; // Chronom�tre pour contr�ler l'animation
    sf::RenderWindow& window = F.getWindow(); // R�cup�rer la fen�tre de rendu

    // Parcourir toutes les cellules dans l'ordre o� elles ont �t� visit�es
    for (auto cell : visitedOrder)
    {
        // Pause pour ralentir l'animation (10 millisecondes entre chaque cellule)
        while (clock.getElapsedTime().asMilliseconds() < 10) {
            // Attente active pour contr�ler la cadence
        }
        clock.restart(); // Red�marrer le chronom�tre

        // Mettre � jour la couleur de la cellule si elle n'est pas un point de d�part ou d'arriv�e
        if (cell->getType() != CellType::StartPoint && cell->getType() != CellType::EndPoint)
            cell->updateColor(sf::Color(200, 100, 50)); // Couleur pour les cellules visit�es orange

        // Rafra�chir l'affichage de la fen�tre
        window.clear(sf::Color::Black); // Nettoyer la fen�tre avec un fond noir
        DrawInterface::drawgrid(F, grid); // Dessiner la grille
        DrawInterface::drawChecklist(F, checklist); // Dessiner la checklist
        DrawInterface::drawButtons(F, Buttons); // Dessiner les boutons
        window.display(); // Mettre � jour l'affichage
    }
}
