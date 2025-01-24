#include "Pathfinding.h"


//Algorithm pour le parcour en largeur
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::bfs(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    std::vector<Cell*> visitedOrder; // Liste des cellules visitées
    std::vector<Cell*> finalPath;    // Chemin final
    std::queue<Cell*> queue;
  

    grid[start.indexX][start.indexY].setVisited();//Mettre le debut comme dejà visité
    queue.push(&grid[start.indexX][start.indexY]); //Mettre l'adresse de debut dans la queue

    // Tant que la queue n'est pas vide on parcourt tous les elements 
    while (!queue.empty())
    {


        Cell* current = queue.front();
        queue.pop();
        visitedOrder.push_back(current); // Ajouter la cellule à la liste des visités

        // Vérifie les voisins
        for (auto& neighbor : getNeighbors(grid, *current))
        {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {

                neighbor->setVisited();
                neighbor->setparent(current); // Définit le parent pour retracer le chemin


                //Si la fin est trouvée on reconstruit le chemin final
                if (*current == end)
                {
                    // Construire le chemin final en remontant les parents
                    Cell* pathNode = &grid[end.indexX][end.indexY];
                    while (pathNode) {
                        finalPath.push_back(pathNode);
                        pathNode = pathNode->getparent();
                    }
                    std::reverse(finalPath.begin(), finalPath.end());

                    return { visitedOrder, finalPath };
                }

                //si la fin n'est pas trouvée on continue le parcour
                queue.push(neighbor);

            }
        }
    }
    return { visitedOrder, finalPath };
}

//Algorithm pour le parcour en profondeur

std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::dfs(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end) 
{

    std::vector<Cell*> visitedOrder; // Liste des cellules visitées
    std::vector<Cell*> finalPath;    // Chemin final
    std::stack<Cell*>Stak;
 

    grid[start.indexX][start.indexY].setVisited();//Mettre le debut comme dejà visité
    Stak.push(&grid[start.indexX][start.indexY]);//Mettre le debut dans la pile


    while (!Stak.empty())
    {
      

        Cell* current = Stak.top();
        Stak.pop();
		visitedOrder.push_back(current); // Ajouter la cellule à la liste des visités
        //Si la fin est trouvée on reconstruit le chemin final
        if (*current == end)
        {
            // Construire le chemin final en remontant les parents
            Cell* pathNode = &grid[end.indexX][end.indexY];
            while (pathNode) {
                finalPath.push_back(pathNode);
                pathNode = pathNode->getparent();
            }
            std::reverse(finalPath.begin(), finalPath.end());

            return { visitedOrder, finalPath };
        }

        // Vérifie les voisins
        for (auto& neighbor : getNeighbors(grid, *current))
        {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {

                neighbor->setVisited();
                neighbor->setparent(current); // Définit le parent pour retracer le chemin
                
               

				//si la fin n'est pas trouvée on continue le parcour
                Stak.push(neighbor);

                

            }
        }
    }
	return { visitedOrder, finalPath };
}

//Methode pour le parcour des pluscourts chemin avec dijkstra


std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::dijkstra(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{

    auto compare = [](Cell* a, Cell* b) {
        return a->getdistance() > b->getdistance();
        };
    std::priority_queue<Cell*, std::vector<Cell*>, decltype(compare)> pq(compare);

    std::vector<Cell*> visitedOrder; // Liste des cellules visitées
    std::vector<Cell*> finalPath;    // Chemin final

    grid[start.indexX][start.indexY].setdistance(0);
    pq.push(&grid[start.indexX][start.indexY]);

    while (!pq.empty()) {
        Cell* current = pq.top();
        pq.pop();
        visitedOrder.push_back(current); // Ajouter la cellule à la liste des visités



        // Parcours des voisins
        for (auto& neighbor : getNeighbors(grid, *current)) {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall) {
                int newCost = current->getdistance() + 1;
                if (newCost < neighbor->getdistance()) {
                    neighbor->setdistance(newCost);
                    neighbor->setparent(current);
                    neighbor->setVisited();

                    if (*current == end) {
                        // Construire le chemin final en remontant les parents
                        Cell* pathNode = &grid[end.indexX][end.indexY];
                        while (pathNode) {
                            finalPath.push_back(pathNode);
                            pathNode = pathNode->getparent();
                        }
                        std::reverse(finalPath.begin(), finalPath.end());
                        return { visitedOrder, finalPath };
                    }
                    pq.push(neighbor);
                }
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les deux listes
}
//Methode pour le parcourt avec A*
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::astar(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
   

    std::vector<Cell*> visitedOrder; // Liste des cellules visitées
    std::vector<Cell*> finalPath;    // Chemin final

    auto heuristic = [](Cell* a, Cell* b) {
        return abs(a->posX - b->posX) + abs(a->posY - b->posY); // Distance Manhattan
        //  return sqrt(pow(a->posX - b->posX, 2) + pow(a->posY - b->posY, 2));
        };

    auto compare = [](Cell* a, Cell* b) {
        return (a->getdistance() + a->getheuristic()) > (b->getdistance() + b->getheuristic());
        };

    std::priority_queue<Cell*, std::vector<Cell*>, decltype(compare)> pq(compare);

    grid[start.indexX][start.indexY].setdistance(0);
    grid[start.indexX][start.indexY].setVisited();
    grid[start.indexX][start.indexY].setheuristic(heuristic(&grid[start.indexX][start.indexY], &grid[end.indexX][end.indexY]));

    pq.push(&grid[start.indexX][start.indexY]);

    while (!pq.empty())
    {
        Cell* current = pq.top();
        pq.pop();
		visitedOrder.push_back(current); // Ajouter la cellule à la liste des visités

        if (*current == end)
        {
            // Construire le chemin final en remontant les parents
            Cell* pathNode = &grid[end.indexX][end.indexY];
            while (pathNode) {
                finalPath.push_back(pathNode);
                pathNode = pathNode->getparent();
            }
            std::reverse(finalPath.begin(), finalPath.end());
            return { visitedOrder, finalPath };
        }
        // Parcours des voisins
        for (auto& neighbor : getNeighbors(grid, *current))
        {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {
                int newCost = current->getdistance() + 1;
                if (newCost < neighbor->getdistance())
                {
                    neighbor->setdistance(newCost);
                    neighbor->setheuristic(heuristic(neighbor, &end));
                    neighbor->setparent(current);
                    neighbor->setVisited();
               
                    pq.push(neighbor);
                }
            }
        }

    }
	return { visitedOrder, finalPath };

}
//Methode qui retourne les voisins d'une case
std::vector<Cell*> Pathfinding::getNeighbors(std::vector<std::vector<Cell>>& grid, Cell& cell)
{
    std::vector<Cell*> neighbors;
    int x = cell.indexX;
    int y = cell.indexY;

    if (y > 0) {
        neighbors.push_back(&grid[x][y - 1]); // Gauche
    }
    if (y < grid[0].size() - 1) neighbors.push_back(&grid[x][y + 1]); // Droite
    if (x > 0) neighbors.push_back(&grid[x - 1][y]); // Haut
    if (x < grid.size() - 1) neighbors.push_back(&grid[x + 1][y]); // Bas

    return neighbors;
}


