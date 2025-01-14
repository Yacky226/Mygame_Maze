#pragma once
#include <iostream>
#include <list>
#include <queue>
#include <map>
#include <SFML/Graphics.hpp>

template<typename T>
class Graph {
private:
    std::map<T, std::list<std::pair<T, int>>> G;  // Graphe
    T start;  // Sommet de départ
public:
    Graph(T start);  // Constructeur
    std::map<T, std::list<std::pair<T, int>>> getG() { return G; }  // Getter
    void clear() { G.clear(); }  // Vider le graphe
    T getStart() { return start; }  // Getter
    void ajouterNoeud(T u, T v, int w);  // Ajouter un arc
    void afficher();  // Afficher le graphe
};

template<typename T>
// Classe pour l'algorithme de Dijkstra
class Pathfinding {
public:
    static void dijkstra(Graph<T>& G, sf::RenderWindow& Window, const int menuWidth,
        std::vector<sf::RectangleShape>& grille,int rows,int cols);
    /*static void bfs(Graph<T>& G, int startX, int startY,
        std::vector<sf::RectangleShape>& grille, sf::RenderWindow& window);*/
};

template<typename T>
inline Graph<T>::Graph(T start) : start(start) {}

template<typename T>
inline void Graph<T>::ajouterNoeud(T u, T v, int w) {
    G[u].push_back({ v, w });  // Ajouter un arc
}

//template<typename T>
//inline void Pathfinding<T>::bfs(Graph<T>& G, int startX, int startY,
//    std::vector<sf::RectangleShape>& grille, sf::RenderWindow& window) {
//    int rows = grille.size();
//    int cols = grille[0].getSize().x;
//
//    std::vector<std::vector<bool>> visited(rows, std::vector<bool>(cols, false));
//    std::queue<std::pair<int, int>> q;
//    q.push({ startX, startY });
//    visited[startX][startY] = true;
//
//    const int dx[] = { -1, 1, 0, 0 };
//    const int dy[] = { 0, 0, -1, 1 };
//
//    while (!q.empty()) {
//        auto [x, y] = q.front();
//        q.pop();
//
//        grille[x * cols + y].setFillColor(sf::Color::Green);
//        window.clear();
//        for (const auto& cell : grille) {
//            window.draw(cell);
//        }
//        window.display();
//
//        sf::sleep(sf::milliseconds(100));
//
//        for (int i = 0; i < 4; ++i) {
//            int nx = x + dx[i];
//            int ny = y + dy[i];
//
//            if (nx >= 0 && nx < rows && ny >= 0 && ny < cols && !visited[nx][ny]) {
//                visited[nx][ny] = true;
//                q.push({ nx, ny });
//            }
//        }
//    }
//}

template<typename T>
inline void Pathfinding<T>::dijkstra(Graph<T>& G, sf::RenderWindow& Window, const int menuWidth,
    std::vector<sf::RectangleShape>& grille,int rows, int cols) {
    size_t spacePos;
    std::map<T, int> distance;  // Distance
    std::map<T, T> parent;  // Parent
    std::priority_queue<std::pair<int, T>, std::vector<std::pair<int, T>>, std::greater<std::pair<int, T>>> Q;
    auto gr = G.getG();

    // Initialisation des distances
    for (auto i : gr) {
        distance[i.first] = INT_MAX;
    }

    int abs, ord;
    distance[G.getStart()] = 0;  // Distance de la racine à elle-même est 0
    Q.push({ 0, G.getStart() });  // On ajoute la racine à la file de priorité
    // Créer un chrono pour le timer
    sf::Clock clock;
    sf::Time elapsedTime;
    float interval = 0.1f;  // Intervalle de 0.1 seconde
    while (!Q.empty()) {
        T u = Q.top().second;  // On récupère le sommet de la file de priorité
        Q.pop();  // On le supprime de la file de priorité

        spacePos = u.find(' ');
        abs = std::stoi(u.substr(0, spacePos));
        ord = std::stoi(u.substr(spacePos + 1));
       

        // Vérifier si l'intervalle est écoulé
        
        for (auto i : gr[u])
        {
            T v = i.first;  // On récupère le sommet v
            int w = i.second;  // On récupère le poids du sommet u au sommet v

            if (distance[v] > distance[u] + w) {  // Si la distance du sommet u au sommet v est plus petite
                distance[v] = distance[u] + w;  // On met à jour la distance
                parent[v] = u;  // On met à jour le parent
                Q.push({ distance[v], v });  // On ajoute le sommet v à la file de priorité
            }
        }
        // Pause pour ralentir le processus et permettre au timer de fonctionner
      //  sf::sleep(sf::milliseconds(10));  // Optionnel, pour éviter que tout se passe trop vite
        
    }

    G.clear();  // On vide le graphe

    // On reconstruit le graphe
    for (auto i : parent) {
        G.ajouterNoeud(i.second, i.first, distance[i.first]);
    }
}

template<typename T>
inline void Graph<T>::afficher() {
    for (auto i : G) {
        std::cout << "Père: " << i.first << " :---> ";
        for (auto j : i.second) {
            std::cout << "Fils: " << j.first << " Poids: " << j.second << "  ";
        }
        std::cout << std::endl;
    }
}
