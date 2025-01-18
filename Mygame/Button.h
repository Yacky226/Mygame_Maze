#pragma once
#include<iostream>
#include<SFML/Graphics.hpp> 
#include "Cell.h"
#include "ChecklistItem.h"


// class pour le bouton
class Button 
{
private:
    sf::RectangleShape shape;   //Rectangle pour contenir le texte du Bouton
    sf::Text label;             //Texte du Bouton

public:
    //Constructeur
    Button(sf::Font& font, const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size);
   
    //Dessine le Bouton
    void draw(sf::RenderWindow& window);

    //Fontion pour designer le Bouton qui clicqué
    bool isClicked(const sf::Vector2f& mousePos);

    //Gestionnaire de clic d'un bouton
    void onClick(std::vector<std::vector<Cell>>& grid, const std::vector<ChecklistItem>& checklist,Cell &start,Cell &end, sf::RenderWindow& window, bool& startPointDefined, bool& endPointDefined);

    //Fonction pour generer une Map aléatoire
    void generateRandomMap(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);

    //Gestionnaire des choix des checkbox
    void handleChecklistSelection(std::vector<std::vector<Cell>>& grid,const std::vector<ChecklistItem>& checklist,Cell &start, Cell &end, sf::RenderWindow& window);
    
    //Fonction qui Netoie la Map pour une Map sans obstacle
    void cleanMap(std::vector<std::vector<Cell>>& grid,Cell &start, Cell &end);
    
    //Methode de parcours en largeur
    void bfs(std::vector<std::vector<Cell>>& grid, Cell &start, Cell &end, sf::RenderWindow& window);

    //Methode de parcours en prondeur
    void dfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window);

    //Methode de parcours des pluscourts chemins avec dijskra
    void dijkstra(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window);

    //Methode qui retourne les voisins d'une case
    std::vector<Cell*> getNeighbors(std::vector<std::vector<Cell>>& grid, Cell& cell);

    //Methode pour animer le Chemin trouver
    void animatePath(Cell* end, sf::RenderWindow& window, float durationPerCell, std::vector<std::vector<Cell>>& grid);
  
    //Methode de parcours A*
    void astar(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window);
};
