#include "Button.h"
#include <cstdlib> // Pour std::rand et std::srand
#include <ctime>   // Pour initialiser le générateur de nombres aléatoires
#include "ChecklistItem.h"
#include <queue>
#include <vector>
#include <limits>
#include <cmath>
#include<stack>

Button::Button(sf::Font& font, const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size)
{
    shape.setSize(size);
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition(position);
    shape.setOutlineThickness(2);
    shape.setOutlineColor(sf::Color::Black);

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

// Vérifier si le bouton est cliqué
bool Button::isClicked(const sf::Vector2f& mousePos)
{
    return shape.getGlobalBounds().contains(mousePos);
}

// Gérer les clics sur le bouton
void Button::onClick(std::vector<std::vector<Cell>>& grid, const std::vector<ChecklistItem>& checklist,Cell &start, Cell &end, sf::RenderWindow& window, bool& startPointDefined, bool& endPointDefined)
{
    if (label.getString() == "Map Aleatoire") {
        startPointDefined = false;
        endPointDefined = false;
        generateRandomMap(grid,start,end); // Génère une carte aléatoire
    }
    else if (label.getString() == "Clean Map") {
        startPointDefined = false;
        endPointDefined = false;
        cleanMap(grid,start,end); // Nettoie la carte
    }
    else if (label.getString() == "Traitement") {
     
        handleChecklistSelection(grid,checklist,start,end,window); // Traite les choix de la checklist
        startPointDefined = false;
        endPointDefined = false;
    }
}

// Gérer la sélection de la checklist
void Button::handleChecklistSelection(std::vector<std::vector<Cell>>& grid, const std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, sf::RenderWindow& window)
{
    for (const auto& item : checklist)
    {
        if (item.isChecked()) { // Vérifie si l'élément est coché

            if (item.getText() == "BFS") 
            {
               
                if (start.getType()==CellType::StartPoint && end.getType() ==CellType::EndPoint ) {
                  
                    bfs(grid, start, end,window);
                 
                    start.setType(CellType::None);
                    end.setType(CellType::None);

                }
               
            }
            else if (item.getText() == "DFS")
            {
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint) {
                    dfs(grid, start, end,window);
                }
            }
            else if (item.getText() == "Dijkstra")
            {
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint) {
                    dijkstra(grid, start, end, window);
                }
            }
            else if (item.getText() == "A_star") 
            {
                if (start.getType() == CellType::StartPoint && end.getType() == CellType::EndPoint) {
                    astar(grid, start, end, window);
                }
            }
        }
    }
}

// Générer une carte aléatoire
void Button::generateRandomMap(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr))); // Initialisation

    for (auto& row : grid)
    {
        for (auto& cell : row)
        {
            if (std::rand() % 4 == 0) { // 25% de chances de placer un mur
                cell.clean();
                cell.setType(CellType::Wall);
            }
            else
            {
                cell.clean();
                cell.setType(CellType::None); // Cellule vide
            }
            cell.setdistance(std::numeric_limits<float>::infinity()); // Réinitialisation des distances
        }
    }
  
}


// Nettoyer la carte

void Button::cleanMap(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end)

{
    for (auto& row : grid) {
        for (auto& cell : row) {
            cell.clean();
            cell.setType(CellType::None); // Réinitialise la cellule
        }
    }
}
  



void Button::bfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window)
{
    std::queue<Cell*> queue;
    sf::Clock clock;
    sf::Color amber(255, 191, 0); // RGB pour ambre
  //  start.visited = true;
    grid[start.indexX][start.indexY].setVisited();
    queue.push(&grid[start.indexX][start.indexY]);

    while (!queue.empty())
    {
        // Gérer le délai pour rendre l'animation progressive
        while (clock.getElapsedTime().asMilliseconds() < 20) {
            // Attendre un petit moment (ajuster les millisecondes si nécessaire)
        }
        clock.restart();

        Cell* current = queue.front();
        queue.pop();

       

        // Vérifie les voisins
        for (auto& neighbor : getNeighbors(grid, *current))
        {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {
                
                neighbor->setVisited();
                neighbor->setparent (current); // Définit le parent pour retracer le chemin
                if (*neighbor == end)
                {
                   // tracePath(end); // Trace le chemin
                    animatePath(&end, window, 0.2f,grid);
                    return;
                }
                queue.push(neighbor);

                // Change la couleur des cellules visitées
                neighbor->updateColor(amber);

                // Rafraîchit l'affichage de la fenêtre
                window.clear(sf::Color::Black); // Nettoie la fenêtre

                // Dessine la grille mise à jour
                for (auto& row : grid) {
                    for (auto& cell : row) {
                        window.draw(cell.cel);
                    }
                }

                // Affiche le contenu à l'écran
                window.display();
            }
        }
    }
}


void Button::dfs(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window)
{
  
    std::stack<Cell*>Stak;
    sf::Clock clock;

    sf::Color amber(255, 191, 0); // RGB pour ambre
    //  start.visited = true;
    grid[start.indexX][start.indexY].setVisited();
    Stak.push(&grid[start.indexX][start.indexY]);
    

    while (!Stak.empty())
    {
        // Gérer le délai pour rendre l'animation progressive
        while (clock.getElapsedTime().asMilliseconds() < 20) {
            // Attendre un petit moment (ajuster les millisecondes si nécessaire)
        }
        clock.restart();

        Cell* current = Stak.top();
        Stak.pop();



        // Vérifie les voisins
        for (auto& neighbor : getNeighbors(grid, *current))
        {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {

                neighbor->setVisited();
                neighbor->setparent(current); // Définit le parent pour retracer le chemin
                if (*neighbor == end)
                {
                    // tracePath(end); // Trace le chemin
                    animatePath(&end, window, 0.2f, grid);
                    return;
                }
                Stak.push(neighbor);

                // Change la couleur des cellules visitées
                neighbor->updateColor(amber);

                // Rafraîchit l'affichage de la fenêtre
                window.clear(sf::Color::Black); // Nettoie la fenêtre

                // Dessine la grille mise à jour
                for (auto& row : grid) {
                    for (auto& cell : row) {
                        window.draw(cell.cel);
                    }
                }

                // Affiche le contenu à l'écran
                window.display();
            }
        }
    }
}



void Button::dijkstra(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window) {
   
    auto compare = [](Cell* a, Cell* b) {
        return a->getdistance() > b->getdistance(); // Comparer par coût croissant
        };
    sf::Clock clock;
    std::priority_queue<Cell*, std::vector<Cell*>, decltype(compare)> pq(compare);

    grid[start.indexX][start.indexY].setdistance(0); // Le coût du point de départ est 0
    grid[start.indexX][start.indexY].setVisited();
    pq.push(&grid[start.indexX][start.indexY]);

    while (!pq.empty())
    {
        Cell* current = pq.top();
        pq.pop();

        // Gérer le délai pour rendre l'animation progressive
        while (clock.getElapsedTime().asMilliseconds() < 20) {
            // Attendre un petit moment
        }
        clock.restart();

        // Parcours des voisins
        for (auto &neighbor : getNeighbors(grid, *current))
        {
            if (!neighbor->getVisited() && neighbor->getType() != CellType::Wall)
            {
                int newCost = current->getdistance() + 1; // Supposons un coût uniforme
                if (newCost < neighbor->getdistance()) 
                {
                    neighbor->setdistance(newCost);
                    neighbor->setparent(current);
                    neighbor->setVisited();

                    if (*neighbor==end) {
                        animatePath(&end, window, 0.2f,grid);
                        return;

                    }

                    pq.push(neighbor);
                    neighbor->updateColor(sf::Color(200, 100, 50)); // Couleur intermédiaire

                    // Rafraîchit l'affichage de la fenêtre
                    window.clear(sf::Color::Black); // Nettoie la fenêtre

                    // Dessine la grille mise à jour
                    for (auto& row : grid) 
                    {
                        for (auto& cell : row)
                        {
                            window.draw(cell.cel);
                        }
                    }
                    window.display();
                    
                }
            }
        }

      
       
    
    }
}


std::vector<Cell*> Button::getNeighbors(std::vector<std::vector<Cell>>& grid, Cell& cell) {
    std::vector<Cell*> neighbors;
    int x = cell.indexX;
    int y = cell.indexY;
  
    if (y > 0){
        neighbors.push_back(&grid[x][y - 1]); // Gauche
    }
    if (y < grid[0].size() - 1) neighbors.push_back(&grid[x][y + 1]); // Droite
    if (x > 0) neighbors.push_back(&grid[x - 1][y]); // Haut
    if (x < grid.size() - 1) neighbors.push_back(&grid[x + 1][y]); // Bas

    return neighbors;
}



void Button::animatePath(Cell* end, sf::RenderWindow& window, float durationPerCell, std::vector<std::vector<Cell>>& grid)
{
    sf::Clock clock;
    std::vector<Cell*> path;

    // Remonte le chemin à partir de la cellule de fin
    Cell* current =&grid[end->indexX][end->indexY];
    while (current != nullptr)
    {

        path.push_back(current);
        current = current->getparent();

    }
   
    // Parcours inverse pour tracer le chemin du début à la fin
    std::reverse(path.begin(), path.end());

    for (size_t i = 0; i < path.size(); ++i)
    {
        clock.restart();
        float elapsed = 0.f;

        // Animation progressive de chaque cellule du chemin
        while (elapsed < durationPerCell)
        {
            elapsed = clock.getElapsedTime().asSeconds();
            float progress = elapsed / durationPerCell;

          
            sf::Color startColor = sf::Color(255, 0, 0); 
            sf::Color endColor = sf::Color(0, 255, 0);   
            path[i]->cel.setFillColor(sf::Color(
                static_cast<sf::Uint8>(startColor.r + progress * (endColor.r - startColor.r)),
                static_cast<sf::Uint8>(startColor.g + progress * (endColor.g - startColor.g)),
                static_cast<sf::Uint8>(startColor.b + progress * (endColor.b - startColor.b)),
                static_cast<sf::Uint8>(startColor.a + progress * (endColor.a - startColor.a))
            ));

            // Afficher la cellule mise à jour
            window.clear(sf::Color::Black); // Nettoie la fenêtre
            for (auto& row : grid)
            {
                for (auto& cell : row)
                {
                    window.draw(cell.cel); // Dessine chaque cellule
                }
            }
            window.display(); // Met à jour l'affichage
        }

        // Finalisation de la cellule
        path[i]->cel.setFillColor(sf::Color(50, 205, 50)); // Couleur finale : Vert clair
    }
}


void Button::astar(std::vector<std::vector<Cell>>& grid, Cell& start, Cell& end, sf::RenderWindow& window)
{
    sf::Clock clock;
    auto heuristic = [](Cell* a, Cell* b) {
       return sqrt(pow(a->indexX-b->indexX,2)+pow(a->indexY-b->indexY,2)) ; // Distance Eucludienne
        // return abs(a->indexX - b->indexX) + abs(a->indexY - b->indexY);// Distance de Manathan
        };

    auto compare = [](Cell* a, Cell* b) {
        return (a->getdistance() + a->getheuristic()) > (b->getdistance() + b->getheuristic());
        };

    std::priority_queue<Cell*, std::vector<Cell*>, decltype(compare)> pq(compare);

    grid[start.indexX][start.indexY].setdistance(0);
    grid[start.indexX][start.indexY].setVisited();
    grid[start.indexX][start.indexY].setheuristic( heuristic(&start, &end));
    pq.push(&grid[start.indexX][start.indexY]);

    while (!pq.empty()) 
    {
        Cell* current = pq.top();
        pq.pop();

        // Gérer le délai pour rendre l'animation progressive
        while (clock.getElapsedTime().asMilliseconds() < 20) {
            // Attendre un petit moment (ajuster les millisecondes si nécessaire)
        }
        clock.restart();

       

     

        // Parcours des voisins
        for (auto &neighbor : getNeighbors(grid, *current)) 
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

                    if (*neighbor == end) {
                        animatePath(&end, window, 0.2f, grid);
                        return;

                    }

                    pq.push(neighbor);

                    // Animation de la cellule explorée
                    if (neighbor != &end) {
                        neighbor->updateColor(sf::Color(255, 165, 79)); // Couleur intermédiaire
                    }
                    // Rafraîchit l'affichage de la fenêtre
                    window.clear(sf::Color::Black); // Nettoie la fenêtre

                    // Dessine la grille mise à jour
                    for (auto& row : grid)
                    {
                        for (auto& cell : row) {
                            window.draw(cell.cel);
                        }
                    }
                    window.display();
                }
            }
        }

        
    
      
       
    }
}


