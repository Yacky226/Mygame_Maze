#include "Pathfinding.h"

// Algorithme pour le parcours en largeur (BFS)
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::bfs(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    std::vector<Cell*> visitedOrder; // Liste des cellules visitées
    std::vector<Cell*> finalPath;    // Chemin final
    std::queue<Cell*> queue;         // Queue pour BFS

    // Marquer le début comme visité et l'ajouter à la queue
    grid[start.indexX][start.indexY].setVisited();
    queue.push(&grid[start.indexX][start.indexY]);

    // Tant que la queue n'est pas vide, on explore les voisins
    while (!queue.empty())
    {
        Cell* current = queue.front(); // Récupère le premier élément de la queue
        queue.pop(); // Retire cet élément de la queue
        visitedOrder.push_back(current); // Ajouter la cellule courante à la liste des visités

        // Si la cellule actuelle est la destination, reconstruire le chemin
        if (*current == end)
        {
            Cell* pathNode = current;
            while (pathNode) { // Remonter les parents pour reconstruire le chemin
                finalPath.push_back(pathNode);
                pathNode = pathNode->getparent();
            }
            std::reverse(finalPath.begin(), finalPath.end()); // Inverser le chemin pour le bon ordre
            return { visitedOrder, finalPath }; // Retourner les cellules visitées et le chemin final
        }

        // Vérifie les voisins de la cellule courante
        for (auto& neighbor : getNeighbors(grid, *current)) 
        {
            // Si le voisin n'a pas été visité et n'est pas un mur
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {
                neighbor->setVisited(); // Marquer comme visité
                neighbor->setparent(current); // Définir le parent pour pouvoir reconstruire le chemin
                queue.push(neighbor); // Ajouter le voisin à la queue pour exploration
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les listes si la destination n'a pas été atteinte
}

// Algorithme pour le parcours en profondeur (DFS)
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::dfs(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    std::vector<Cell*> visitedOrder; // Liste des cellules visitées
    std::vector<Cell*> finalPath;    // Chemin final
    std::stack<Cell*> Stak;          // Pile pour DFS

    // Marquer le début comme visité et l'ajouter à la pile
    grid[start.indexX][start.indexY].setVisited();
    Stak.push(&grid[start.indexX][start.indexY]);

    // Tant que la pile n'est pas vide, on explore les voisins
    while (!Stak.empty()) 
    {
        Cell* current = Stak.top(); // Récupère le sommet de la pile
        Stak.pop(); // Retirer l'élément de la pile
        visitedOrder.push_back(current); // Ajouter la cellule courante à la liste des visités

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

        // Vérifie les voisins de la cellule courante
        for (auto& neighbor : getNeighbors(grid, *current))
        {
            // Si le voisin n'a pas été visité et n'est pas un mur
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall) 
            {
                neighbor->setVisited(); // Marquer comme visité
                neighbor->setparent(current); // Définir le parent
                Stak.push(neighbor); // Ajouter le voisin à la pile
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les listes si la destination n'a pas été atteinte
}

// Algorithme de Dijkstra
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::dijkstra(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    // Comparateur pour la priorité de la queue
    auto compare = [](Cell* a, Cell* b) {
        return a->getdistance() > b->getdistance();
        };
    std::priority_queue<Cell*, std::vector<Cell*>, decltype(compare)> pq(compare); // Priority queue

    std::vector<Cell*> visitedOrder; // Liste des cellules visitées
    std::vector<Cell*> finalPath;    // Chemin final

    grid[start.indexX][start.indexY].setdistance(0); // Initialiser la distance du point de départ
    pq.push(&grid[start.indexX][start.indexY]); // Ajouter le début à la queue de priorité

    while (!pq.empty()) 
    {
        Cell* current = pq.top(); // Récupère l'élément de priorité 
        pq.pop(); // Retirer cet élément de la queue
        visitedOrder.push_back(current); // Ajouter la cellule courante à la liste des visités
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
            // Si le voisin n'a pas été visité et n'est pas un mur
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall) 
            {
                int newCost = current->getdistance() + 1; // Calculer le nouveau coût
                if (newCost < neighbor->getdistance()) 
                { // Si le nouveau coût est plus faible
                    neighbor->setdistance(newCost); // Mettre à jour la distance
                    neighbor->setparent(current); // Définir le parent
                    neighbor->setVisited(); // Marquer comme visité
                    pq.push(neighbor); // Ajouter le voisin à la queue de priorité
                }
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les listes si la destination n'a pas été atteinte
}

// Algorithme A*
std::pair<std::vector<Cell*>, std::vector<Cell*>> Pathfinding::astar(
    std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    // Heuristique de distance Manhattan
    auto heuristic = [](Cell* a, Cell* b) {
        return abs(a->posX - b->posX) + abs(a->posY - b->posY); // Distance de Manhattan
        };

    // Comparateur pour la queue de priorité A* (f = g + h)
    auto compare = [](Cell* a, Cell* b) {
        return (a->getdistance() + a->getheuristic()) > (b->getdistance() + b->getheuristic());
        };
    std::priority_queue<Cell*, std::vector<Cell*>, decltype(compare)> pq(compare);

    std::vector<Cell*> visitedOrder; // Liste des cellules visitées
    std::vector<Cell*> finalPath;    // Chemin final

    // Initialiser la distance et l'heuristique de départ
    grid[start.indexX][start.indexY].setdistance(0);
    grid[start.indexX][start.indexY].setVisited();
    grid[start.indexX][start.indexY].setheuristic(heuristic(&grid[start.indexX][start.indexY], &grid[end.indexX][end.indexY]));

    pq.push(&grid[start.indexX][start.indexY]); // Ajouter le début à la queue de priorité

    while (!pq.empty()) 
    {
        Cell* current = pq.top(); // Récupère l'élément en top
        pq.pop(); // Retirer cet élément de la queue
        visitedOrder.push_back(current); // Ajouter la cellule courante à la liste des visités

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

        // Vérifie les voisins de la cellule courante
        for (auto& neighbor : getNeighbors(grid, *current)) 
        {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {
                int newCost = current->getdistance() + 1; // Calcul du coût
                if (newCost < neighbor->getdistance()) 
                { // Si le coût est plus faible
                    neighbor->setdistance(newCost); // Mettre à jour la distance
                    neighbor->setheuristic(heuristic(neighbor, &end)); // Mettre à jour l'heuristique
                    neighbor->setparent(current); // Définir le parent
                    neighbor->setVisited(); // Marquer comme visité
                    pq.push(neighbor); // Ajouter à la queue de priorité
                }
            }
        }
    }

    return { visitedOrder, finalPath }; // Retourner les listes si la destination n'a pas été atteinte
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