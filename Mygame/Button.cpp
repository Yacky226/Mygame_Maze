#include "Button.h"
#include "ChecklistItem.h"
#include <vector>
#include "Pathfinding.h"
#include "Animated.h"

// Constructeur de la classe Button
Button::Button(sf::Font& font, const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size)
{
    // Initialisation des propriétés du conteneur du bouton
    shape.setSize(size); // Définir la taille du bouton
    shape.setFillColor(sf::Color::Blue); // Définir la couleur de remplissage
    shape.setPosition(position); // Positionner le bouton
    shape.setOutlineThickness(2); // Définir l'épaisseur de la bordure
    shape.setOutlineColor(sf::Color::Black); // Définir la couleur de la bordure

    // Initialisation des propriétés du texte du bouton
    label.setFont(font); // Associer une police au texte
    label.setString(text); // Définir le texte affiché sur le bouton
    label.setCharacterSize(20); // Définir la taille du texte
    label.setFillColor(sf::Color::White); // Définir la couleur du texte

    // Centrer le texte à l'intérieur du bouton
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setPosition(
        position.x + (size.x - textBounds.width) / 2 - textBounds.left,
        position.y + (size.y - textBounds.height) / 2 - textBounds.top
    );
}

// Dessiner le bouton sur la fenêtre
void Button::draw(sf::RenderWindow& window)
{
    window.draw(shape); // Dessiner le conteneur du bouton
    window.draw(label); // Dessiner le texte du bouton
}

// Vérifier si le bouton a été cliqué
bool Button::isClicked(const sf::Vector2f& mousePos)
{
    // Vérifie si la position de la souris se trouve dans les limites du bouton
    return shape.getGlobalBounds().contains(mousePos);
}

// Gérer les clics sur le bouton
void Button::onClick(std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F, bool& startPointDefined, bool& endPointDefined)
{
    // Si le texte du bouton est "Map Aléatoire", générer une carte aléatoire
    if (label.getString() == "Map Aleatoire")
    {
        startPointDefined = false;
        endPointDefined = false;
        DrawInterface::generateRandomMap(grid); // Génère une carte aléatoire
    }
    // Si le texte du bouton est "Clean Map", nettoyer la carte
    else if (label.getString() == "Clean Map")
    {
        startPointDefined = false;
        endPointDefined = false;
        DrawInterface::cleanMap(grid); // Nettoie la carte
    }
    // Si le texte du bouton est "Traitement", traiter les options de la checklist
    else if (label.getString() == "Traitement")
    {
        handleChecklistSelection(Buttons, grid, checklist, start, end, F); // Traite les choix de la checklist
        startPointDefined = false;
        endPointDefined = false;
    }
}

// Gérer la sélection de la checklist et exécuter les algorithmes correspondants
void Button::handleChecklistSelection(std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F)
{
    // Parcourir tous les éléments de la checklist
    for (const auto& item : checklist)
    {
        // Vérifier si l'élément est coché
        if (item.isChecked())
        {
            // Si l'élément coché est "BFS"
            if (item.getText() == "BFS")
            {
                // Vérifier que les points de départ et d'arrivée sont définis
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint)
                {
                    // Exécuter l'algorithme BFS
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::bfs(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first; // Ordre des cellules visitées
                    std::vector<Cell*> finalPath = result.second; // Chemin final trouvé

                    // Animer les cellules visitées et le chemin final
                    Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
                    Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);

                    // Réinitialiser les types des points de départ et d'arrivée
                    start.setType(CellType::None);
                    end.setType(CellType::None);
                }
            }
            // Si l'élément coché est "DFS"
            else if (item.getText() == "DFS")
            {
                // Vérifier que les points de départ et d'arrivée sont définis
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint)
                {
                    // Exécuter l'algorithme DFS
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::dfs(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first;
                    std::vector<Cell*> finalPath = result.second;

                    // Animer les cellules visitées et le chemin final
                    Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
                    Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);

                    // Réinitialiser les types des points de départ et d'arrivée
                    start.setType(CellType::None);
                    end.setType(CellType::None);
                }
            }
            // Si l'élément coché est "Dijkstra"
            else if (item.getText() == "Dijkstra")
            {
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint)
                {
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::dijkstra(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first;
                    std::vector<Cell*> finalPath = result.second;

                    Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
                    Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);

                    start.setType(CellType::None);
                    end.setType(CellType::None);
                }
            }
            // Si l'élément coché est "A_star"
            else if (item.getText() == "A_star")
            {
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint)
                {
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::astar(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first;
                    std::vector<Cell*> finalPath = result.second;

                    Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
                    Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);

                    start.setType(CellType::None);
                    end.setType(CellType::None);
                }
            }
        }
    }
}
