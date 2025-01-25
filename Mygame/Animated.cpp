#include "Animated.h"

// Fonction pour animer le chemin final trouvé
void Animated::animatePath(const std::vector<Cell*>& path, std::vector<Button*>& Buttons, std::vector<ChecklistItem>& checklist, Cell* end, Fenetre& F, float durationPerCell, std::vector<std::vector<Cell>>& grid)
{
    sf::Clock clock; // Chronomètre pour contrôler l'animation
    sf::RenderWindow& window = F.getWindow(); // Récupérer la fenêtre de rendu

    for (size_t i = 0; i < path.size(); ++i)
    {
        clock.restart(); // Redémarrer le chronomètre pour chaque cellule
        float elapsed = 0.f; // Temps écoulé pour cette cellule

        // Animation progressive de chaque cellule du chemin
        while (elapsed < durationPerCell)
        {
            elapsed = clock.getElapsedTime().asSeconds(); // Temps écoulé en secondes
            float progress = elapsed / durationPerCell; // Progression de l'animation entre 0 et 1

            // Définir les couleurs de début et de fin de l'animation
            sf::Color startColor = sf::Color(255, 0, 0); // Rouge
            sf::Color endColor = sf::Color(0, 255, 0);   // Vert

            // Appliquer une transition de couleur si la cellule n'est ni le point de départ ni le point d'arrivée
            if (path[i]->getType() != CellType::StartPoint && path[i]->getType() != CellType::EndPoint) {
                path[i]->cel.setFillColor(sf::Color(
                    static_cast<sf::Uint8>(startColor.r + progress * (endColor.r - startColor.r)), // Interpolation rouge
                    static_cast<sf::Uint8>(startColor.g + progress * (endColor.g - startColor.g)), // Interpolation vert
                    static_cast<sf::Uint8>(startColor.b + progress * (endColor.b - startColor.b)), // Interpolation bleu
                    static_cast<sf::Uint8>(startColor.a + progress * (endColor.a - startColor.a))  // Interpolation alpha
                ));
            }

            // Rafraîchir l'affichage de la fenêtre
            window.clear(sf::Color::Black); // Nettoyer la fenêtre avec un fond noir
            DrawInterface::drawgrid(F, grid); // Dessiner la grille
            DrawInterface::drawChecklist(F, checklist); // Dessiner la checklist
            DrawInterface::drawButtons(F, Buttons); // Dessiner les boutons
            window.display(); // Mettre à jour l'affichage
        }

        // Finalisation de la couleur de la cellule une fois l'animation terminée
        if (path[i]->getType() != CellType::StartPoint && path[i]->getType() != CellType::EndPoint) {
            path[i]->cel.setFillColor(sf::Color(50, 205, 50)); // Couleur vert clair pour le chemin final
        }
    }
}

// Fonction pour animer les cellules visitées lors de l'exécution de l'algorithme
void Animated::animateCels(const std::vector<Cell*>& visitedOrder, std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F)
{
    sf::Clock clock; // Chronomètre pour contrôler l'animation
    sf::RenderWindow& window = F.getWindow(); // Récupérer la fenêtre de rendu

    // Parcourir toutes les cellules dans l'ordre où elles ont été visitées
    for (auto cell : visitedOrder)
    {
        // Pause pour ralentir l'animation (10 millisecondes entre chaque cellule)
        while (clock.getElapsedTime().asMilliseconds() < 10) {
            // Attente active pour contrôler la cadence
        }
        clock.restart(); // Redémarrer le chronomètre

        // Mettre à jour la couleur de la cellule si elle n'est pas un point de départ ou d'arrivée
        if (cell->getType() != CellType::StartPoint && cell->getType() != CellType::EndPoint)
            cell->updateColor(sf::Color(200, 100, 50)); // Couleur pour les cellules visitées orange

        // Rafraîchir l'affichage de la fenêtre
        window.clear(sf::Color::Black); // Nettoyer la fenêtre avec un fond noir
        DrawInterface::drawgrid(F, grid); // Dessiner la grille
        DrawInterface::drawChecklist(F, checklist); // Dessiner la checklist
        DrawInterface::drawButtons(F, Buttons); // Dessiner les boutons
        window.display(); // Mettre à jour l'affichage
    }
}
