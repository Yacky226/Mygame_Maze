#include "Pathfinding.h"

// Algorithme pour le parcours en largeur (BFS)
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::bfs(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    std::vector<Cell*> visitedOrder; // Liste des cellules visit�es
    std::vector<Cell*> finalPath;    // Chemin final
    std::queue<Cell*> queue;         // Queue pour BFS

    // Marquer le d�but comme visit� et l'ajouter � la queue
    grid[start.indexX][start.indexY].setVisited();
    queue.push(&grid[start.indexX][start.indexY]);

    // Tant que la queue n'est pas vide, on explore les voisins
    while (!queue.empty())
    {
        Cell* current = queue.front(); // R�cup�re le premier �l�ment de la queue
        queue.pop(); // Retire cet �l�ment de la queue
        visitedOrder.push_back(current); // Ajouter la cellule courante � la liste des visit�s

        // Si la cellule actuelle est la destination, reconstruire le chemin
        if (*current == end)
        {
            Cell* pathNode = current;
            while (pathNode) { // Remonter les parents pour reconstruire le chemin
                finalPath.push_back(pathNode);
                pathNode = pathNode->getparent();
            }
            std::reverse(finalPath.begin(), finalPath.end()); // Inverser le chemin pour le bon ordre
            return { visitedOrder, finalPath }; // Retourner les cellules visit�es et le chemin final
        }

        // V�rifie les voisins de la cellule courante
        for (auto& neighbor : getNeighbors(grid, *current)) 
        {
            // Si le voisin n'a pas �t� visit� et n'est pas un mur
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {
                neighbor->setVisited(); // Marquer comme visit�
                neighbor->setparent(current); // D�finir le parent pour pouvoir reconstruire le chemin
                queue.push(neighbor); // Ajouter le voisin � la queue pour exploration
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les listes si la destination n'a pas �t� atteinte
}

// Algorithme pour le parcours en profondeur (DFS)
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::dfs(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    std::vector<Cell*> visitedOrder; // Liste des cellules visit�es
    std::vector<Cell*> finalPath;    // Chemin final
    std::stack<Cell*> Stak;          // Pile pour DFS

    // Marquer le d�but comme visit� et l'ajouter � la pile
    grid[start.indexX][start.indexY].setVisited();
    Stak.push(&grid[start.indexX][start.indexY]);

    // Tant que la pile n'est pas vide, on explore les voisins
    while (!Stak.empty()) 
    {
        Cell* current = Stak.top(); // R�cup�re le sommet de la pile
        Stak.pop(); // Retirer l'�l�ment de la pile
        visitedOrder.push_back(current); // Ajouter la cellule courante � la liste des visit�s

        // Si la cellule courante est la destination, reconstruire le chemin
        if (*current == end) 
        {
            Cell* pathNode = current;
            while (pathNode)
            { // Remonter les parents pour reconstruire le chemin
                finalPath.push_back(pathNode);
                pathNode = pathNode->getparent();
            }
            std::reverse(finalPath.begin(), finalPath.end()); // Inverser pour le bon ordre
            return { visitedOrder, finalPath };
        }

        // V�rifie les voisins de la cellule courante
        for (auto& neighbor : getNeighbors(grid, *current))
        {
            // Si le voisin n'a pas �t� visit� et n'est pas un mur
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall) 
            {
                neighbor->setVisited(); // Marquer comme visit�
                neighbor->setparent(current); // D�finir le parent
                Stak.push(neighbor); // Ajouter le voisin � la pile
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les listes si la destination n'a pas �t� atteinte
}

// Algorithme de Dijkstra
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::dijkstra(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    // Comparateur pour la priorit� de la queue
    auto compare = [](Cell* a, Cell* b) {
        return a->getdistance() > b->getdistance();
        };
    std::priority_queue<Cell*, std::vector<Cell*>, decltype(compare)> pq(compare); // Priority queue

    std::vector<Cell*> visitedOrder; // Liste des cellules visit�es
    std::vector<Cell*> finalPath;    // Chemin final

    grid[start.indexX][start.indexY].setdistance(0); // Initialiser la distance du point de d�part
    pq.push(&grid[start.indexX][start.indexY]); // Ajouter le d�but � la queue de priorit�

    while (!pq.empty()) 
    {
        Cell* current = pq.top(); // R�cup�re l'�l�ment de priorit� 
        pq.pop(); // Retirer cet �l�ment de la queue
        visitedOrder.push_back(current); // Ajouter la cellule courante � la liste des visit�s
        if (*current == end)
        { // Si on atteint la destination
            Cell* pathNode = &grid[end.indexX][end.indexY]; // Remonter les parents pour le chemin
            while (pathNode)
            {
                finalPath.push_back(pathNode);
                pathNode = pathNode->getparent();
            }
            std::reverse(finalPath.begin(), finalPath.end()); // Inverser le chemin
            return { visitedOrder, finalPath };
        }
        // Parcours des voisins de la cellule courante
        for (auto& neighbor : getNeighbors(grid, *current))
        {
            // Si le voisin n'a pas �t� visit� et n'est pas un mur
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall) 
            {
                int newCost = current->getdistance() + 1; // Calculer le nouveau co�t
                if (newCost < neighbor->getdistance()) 
                { // Si le nouveau co�t est plus faible
                    neighbor->setdistance(newCost); // Mettre � jour la distance
                    neighbor->setparent(current); // D�finir le parent
                    neighbor->setVisited(); // Marquer comme visit�
                    pq.push(neighbor); // Ajouter le voisin � la queue de priorit�
                }
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les listes si la destination n'a pas �t� atteinte
}

// Algorithme A*
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::astar(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    // Heuristique de distance Manhattan
    auto heuristic = [](Cell* a, Cell* b) {
        return abs(a->posX - b->posX) + abs(a->posY - b->posY); // Distance de Manhattan
        };

    // Comparateur pour la queue de priorit� A* (f = g + h)
    auto compare = [](Cell* a, Cell* b) {
        return (a->getdistance() + a->getheuristic()) > (b->getdistance() + b->getheuristic());
        };
    std::priority_queue<Cell*, std::vector<Cell*>, decltype(compare)> pq(compare);

    std::vector<Cell*> visitedOrder; // Liste des cellules visit�es
    std::vector<Cell*> finalPath;    // Chemin final

    // Initialiser la distance et l'heuristique de d�part
    grid[start.indexX][start.indexY].setdistance(0);
    grid[start.indexX][start.indexY].setVisited();
    grid[start.indexX][start.indexY].setheuristic(heuristic(&grid[start.indexX][start.indexY], &grid[end.indexX][end.indexY]));

    pq.push(&grid[start.indexX][start.indexY]); // Ajouter le d�but � la queue de priorit�

    while (!pq.empty()) 
    {
        Cell* current = pq.top(); // R�cup�re l'�l�ment en top
        pq.pop(); // Retirer cet �l�ment de la queue
        visitedOrder.push_back(current); // Ajouter la cellule courante � la liste des visit�s

        if (*current == end) 
        { // Si on atteint la destination
            Cell* pathNode = &grid[end.indexX][end.indexY]; // Remonter les parents pour reconstruire le chemin
            while (pathNode)
            {
                finalPath.push_back(pathNode);
                pathNode = pathNode->getparent();
            }
            std::reverse(finalPath.begin(), finalPath.end()); // Inverser pour obtenir le bon ordre
            return { visitedOrder, finalPath };
        }

        // V�rifie les voisins de la cellule courante
        for (auto& neighbor : getNeighbors(grid, *current)) 
        {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {
                int newCost = current->getdistance() + 1; // Calcul du co�t
                if (newCost < neighbor->getdistance()) 
                { // Si le co�t est plus faible
                    neighbor->setdistance(newCost); // Mettre � jour la distance
                    neighbor->setheuristic(heuristic(neighbor, &end)); // Mettre � jour l'heuristique
                    neighbor->setparent(current); // D�finir le parent
                    neighbor->setVisited(); // Marquer comme visit�
                    pq.push(neighbor); // Ajouter � la queue de priorit�
                }
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les listes si la destination n'a pas �t� atteinte
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