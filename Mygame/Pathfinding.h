#pragma once
#include <vector>
#include <queue>
#include <stack>
#include <SFML/Graphics.hpp>
#include "Cell.h"
#include "Button.h"
#include "ChecklistItem.h"
class Pathfinding
{
public:
  
    //Methode de parcours en largeur
   static std::pair<std::vector<Cell*>, std::vector<Cell*>> bfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);

    //Methode de parcours en prondeur
   static std::pair<std::vector<Cell*>, std::vector<Cell*>> dfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);

    //Methode de parcours des pluscourts chemins avec dijskra
   static  std::pair<std::vector<Cell*>, std::vector<Cell*>> dijkstra(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);

    //Methode qui retourne les voisins d'une case
   static  std::vector<Cell*> getNeighbors(std::vector<std::vector<Cell>>& grid, Cell& cell);

    //Methode de parcours A*
   static std::pair<std::vector<Cell*>, std::vector<Cell*>> astar(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end);
};

