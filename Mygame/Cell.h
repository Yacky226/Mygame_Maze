#pragma once 
#include<SFML/Graphics.hpp>

// Énumération pour les types de cellules
enum CellType { None, StartPoint, EndPoint, Wall };

class Cell {
private:
    // Propriétés privées
    float cellesize = 20.0f; // Taille par défaut de la cellule
    float posX;             // Position X de la cellule
    float posY;             // Position Y de la cellule
    int height;             // Hauteur de la cellule
    int width;              // Largeur de la cellule
    float heuristic = 0.0f; // Heuristique (utilisé pour A*)
    CellType type;          // Type de la cellule (None, StartPoint, EndPoint, Wall)
    Cell* parent = nullptr; // Pointeur vers la cellule parente pour retracer le chemin
    float distance = std::numeric_limits<float>::infinity(); // Distance pour Dijkstra
    bool visited = false;   // Indique si la cellule a été visitée 

public:
    // Propriétés publiques
    int indexX = -1;        // Indice X dans la grille
    int indexY = -1;        // Indice Y dans la grille
    sf::Color cellcolor = sf::Color::White; // Couleur actuelle de la cellule
    sf::RectangleShape cel;                // Représentation graphique de la cellule

    // Constructeur avec paramètres
    Cell(int x, int y, float posX, float posY, int height, int width);

    // Constructeur par défaut
    Cell();

    // Constructeur de copie
    Cell(const Cell& C);

    // Définit la cellule parente
    void setparent(Cell* c);

    // Récupère la cellule parente
    Cell* getparent();

    // Définit la cellule comme visitée
    void setVisited();

    // Vérifie si la cellule est visitée
    bool getVisited();

    // Définit la distance depuis la source
    void setdistance(float dis);

    // Récupère la distance depuis la source
    float getdistance();

    // Définit l'heuristique
    void setheuristic(float dis);

    // Récupère l'heuristique
    float getheuristic();

    // Met à jour la couleur en fonction du type
    void updateColor();

    // Retourne une référence vers la cellule actuelle
    Cell& getCell();

    // Met à jour la couleur avec une couleur spécifique
    void updateColor(const sf::Color& couleur);

    // Gère le clic sur la cellule
    void handleClick(bool& startPointDefined, bool& endPointDefined, Cell& start, Cell& end);

    // Définit la couleur de la cellule
    void SetColor(sf::Color& couleur);

    // Définit la cellule comme un mur
    void SetWall();

    // Surcharge de l'opérateur == pour comparer deux cellules
    bool operator==(const Cell& C);

    // Récupère le type de la cellule
    CellType getType() const;

    // Définit un nouveau type pour la cellule
    void setType(CellType newType);

    // Définit la couleur de la cellule avec une couleur constante
    void SetColor(const sf::Color& couleur);

    // Nettoie la cellule 
    void clean();
};
