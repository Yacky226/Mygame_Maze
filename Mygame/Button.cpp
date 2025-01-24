#include "Button.h"
#include <cstdlib> 
#include <ctime>   
#include "ChecklistItem.h"
#include <vector>
#include <limits>
#include <cmath>
#include<stack>
#include "Pathfinding.h"
#include"Animated.h"

//Constructeur
Button::Button(sf::Font& font, const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size)
{
    //Initialisation des valeur du conteneur du Bouton
    shape.setSize(size);
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition(position);
    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color::Black);

    //Initialisation du Texte du Bouton
    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(20);
    label.setFillColor(sf::Color::White);

    // Centrer le texte dans le bouton
    sf::FloatRect textBounds = label.getLocalBounds();
    label.setPosition(
        position.x + (size.x - textBounds.width) / 2 - textBounds.left,
        position.y + (size.y - textBounds.height) / 2 - textBounds.top
    );
}

// Dessiner le bouton
void Button::draw(sf::RenderWindow& window)
{
    window.draw(shape);
    window.draw(label);
}

// V�rifier si le bouton est cliqu�
bool Button::isClicked(const sf::Vector2f& mousePos)
{
    return shape.getGlobalBounds().contains(mousePos);
}

// G�rer les clics sur le bouton
void Button::onClick(std::vector<Button*> &Buttons,std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist,Cell &start, Cell &end,Fenetre &F, bool& startPointDefined, bool& endPointDefined)
{
    //Si le ckic est sur Map Al�atoire
    if (label.getString() == "Map Aleatoire") 
    {
        startPointDefined = false;
        endPointDefined = false;
		DrawInterface::generateRandomMap(grid); // G�n�re une carte al�atoire
    }
    else
        //Si le ckic est sur Clean Map
    if (label.getString() == "Clean Map") 
    {
        startPointDefined = false;
        endPointDefined = false;
		DrawInterface::cleanMap(grid); // Nettoie la carte
    }
    else 
        //Si le ckic est sur Traitement
        if (label.getString() == "Traitement")
        {
     
            handleChecklistSelection(Buttons,grid,checklist,start,end,F); // Traite les choix de la checklist
            startPointDefined = false;
            endPointDefined = false;
        }
}

// G�rer la s�lection de la checklist
void Button::handleChecklistSelection(std::vector<Button*>& Buttons,std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist, Cell& start, Cell& end,Fenetre &F)
{
    for (const auto& item : checklist)
    {
        if (item.isChecked()) { // V�rifie si l'�l�ment est coch�
            //On verifie le choix coch� est BFS
            if (item.getText() == "BFS") 
            {
               //On verifie si bouton de debut et de fin sont choisi
                if (start.getType()==CellType::StartPoint && end.getType() ==CellType::EndPoint ) 
                {
					std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::bfs(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first;
                    std::vector<Cell*> finalPath = result.second;
					Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
					Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);
					
					start.setType(CellType::None);//A la fin on renitailise le debut
					end.setType(CellType::None);//A la fin on renitailise la fin
                 
                 
                    start.setType(CellType::None);//A la fin on renitailise le debut
                    end.setType(CellType::None);//A la fin on renitailise la fin

                }
               
            }
            //On verifie le choix coch� est DFS
            else if (item.getText() == "DFS")
            {
                //On verifie si bouton de debut et de fin sont choisi
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint)
                {
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::dfs(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first;
                    std::vector<Cell*> finalPath = result.second;

					Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
					Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);
                    start.setType(CellType::None);//A la fin on renitailise le debut
                    end.setType(CellType::None);//A la fin on renitailise la fin

                }
            }
            else 
                //On verifie le choix coch� est Dijkstra
            if (item.getText() == "Dijkstra")
            {
                //On verifie si bouton de debut et de fin sont choisi
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint) 
                {
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::dijkstra(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first;
                    std::vector<Cell*> finalPath = result.second;

					Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
					Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);
                    start.setType(CellType::None);//A la fin on renitailise le debut
                    end.setType(CellType::None);//A la fin on renitailise la fin
                }
            }
            else 
                //On verifie le choix coch� est A_star

            if (item.getText() == "A_star") 
            {
                //On verifie si bouton de debut et de fin sont choisi
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint)
                {
                    std::pair<std::vector<Cell*>, std::vector<Cell*>> result = Pathfinding::astar(grid, start, end);
                    std::vector<Cell*> visitedOrder = result.first;
                    std::vector<Cell*> finalPath = result.second;
					Animated::animateCels(visitedOrder, Buttons, grid, checklist, start, end, F);
					Animated::animatePath(finalPath, Buttons, checklist, &end, F, 0.1f, grid);
                    start.setType(CellType::None);//A la fin on renitailise le debut
                    end.setType(CellType::None);//A la fin on renitailise la fin
                }
            }
        }
    }
}

