#include "Button.h"
#include "ChecklistItem.h"
#include <vector>
#include "Pathfinding.h"
#include "Animated.h"

// Constructeur de la classe Button
Button::Button(sf::Font& font, const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size)
{
    // Initialisation des propri�t�s du conteneur du bouton
    shape.setSize(size); // D�finir la taille du bouton
    shape.setFillColor(sf::Color::Blue); // D�finir la couleur de remplissage
    shape.setPosition(position); // Positionner le bouton
    shape.setOutlineThickness(2); // D�finir l'�paisseur de la bordure
    shape.setOutlineColor(sf::Color::Black); // D�finir la couleur de la bordure

    // Initialisation des propri�t�s du texte du bouton
    label.setFont(font); // Associer une police au texte
    label.setString(text); // D�finir le texte affich� sur le bouton
    label.setCharacterSize(20); // D�finir la taille du texte
    label.setFillColor(sf::Color::White); // D�finir la couleur du texte

    // Centrer le texte � l'int�rieur du bouton
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setPosition(
        position.x + (size.x - textBounds.width) / 2 - textBounds.left,
        position.y + (size.y - textBounds.height) / 2 - textBounds.top
    );
}

// Dessiner le bouton sur la fen�tre
void Button::draw(sf::RenderWindow& window)
{
    window.draw(shape); // Dessiner le conteneur du bouton
    window.draw(label); // Dessiner le texte du bouton
}

// V�rifier si le bouton a �t� cliqu�
bool Button::isClicked(const sf::Vector2f& mousePos)
{
    // V�rifie si la position de la souris se trouve dans les limites du bouton
    return shape.getGlobalBounds().contains(mousePos);
}

// G�rer les clics sur le bouton
void Button::onClick(std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F, bool& startPointDefined, bool& endPointDefined)
{
    // Si le texte du bouton est "Map Al�atoire", g�n�rer une carte al�atoire
    if (label.getString() == "Map Aleatoire")
    {
        startPointDefined = false;
        endPointDefined = false;
        DrawInterface::generateRandomMap(grid); // G�n�re une carte al�atoire
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

// G�rer la s�lection de la checklist et ex�cuter les algorithmes correspondants
void Button::handleChecklistSelection(std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F)
{
    // Parcourir tous les �l�ments de la checklist
    for (const auto& item : checklist)
    {
        // V�rifier si l'�l�ment est coch�
        if (item.isChecked())
        {
            // Si l'�l�ment coch� est "BFS"
            if (item.getText() == "BFS")
            {
                // V�rifier que les points de d�part et d'arriv�e sont d�finis
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint)
                {
                    // Ex�cuter l'algorithme BFS
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::bfs(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first; // Ordre des cellules visit�es
                    std::vector<Cell*> finalPath = result.second; // Chemin final trouv�

                    // Animer les cellules visit�es et le chemin final
                    Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
                    Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);

                    // R�initialiser les types des points de d�part et d'arriv�e
                    start.setType(CellType::None);
                    end.setType(CellType::None);
                }
            }
            // Si l'�l�ment coch� est "DFS"
            else if (item.getText() == "DFS")
            {
                // V�rifier que les points de d�part et d'arriv�e sont d�finis
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint)
                {
                    // Ex�cuter l'algorithme DFS
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::dfs(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first;
                    std::vector<Cell*> finalPath = result.second;

                    // Animer les cellules visit�es et le chemin final
                    Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
                    Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);

                    // R�initialiser les types des points de d�part et d'arriv�e
                    start.setType(CellType::None);
                    end.setType(CellType::None);
                }
            }
            // Si l'�l�ment coch� est "Dijkstra"
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
            // Si l'�l�ment coch� est "A_star"
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
