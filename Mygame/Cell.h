#pragma once 
#include<SFML/Graphics.hpp>

// �num�ration pour les types de cellules
enum CellType { None, StartPoint, EndPoint, Wall };

class Cell {
private:
    // Propri�t�s priv�es
    float cellesize = 20.0f; // Taille par d�faut de la cellule
    float posX;             // Position X de la cellule
    float posY;             // Position Y de la cellule
    int height;             // Hauteur de la cellule
    int width;              // Largeur de la cellule
    float heuristic = 0.0f; // Heuristique (utilis� pour A*)
    CellType type;          // Type de la cellule (None, StartPoint, EndPoint, Wall)
    Cell* parent = nullptr; // Pointeur vers la cellule parente pour retracer le chemin
    float distance = std::numeric_limits<float>::infinity(); // Distance pour Dijkstra
    bool visited = false;   // Indique si la cellule a �t� visit�e 

public:
    // Propri�t�s publiques
    int indexX = -1;        // Indice X dans la grille
    int indexY = -1;        // Indice Y dans la grille
    sf::Color cellcolor = sf::Color::White; // Couleur actuelle de la cellule
    sf::RectangleShape cel;                // Repr�sentation graphique de la cellule

    // Constructeur avec param�tres
    Cell(int x, int y, float posX, float posY, int height, int width);

    // Constructeur par d�faut
    Cell();

    // Constructeur de copie
    Cell(const Cell& C);

    // D�finit la cellule parente
    void setparent(Cell* c);

    // R�cup�re la cellule parente
    Cell* getparent();

    // D�finit la cellule comme visit�e
    void setVisited();

    // V�rifie si la cellule est visit�e
    bool getVisited();

    // D�finit la distance depuis la source
    void setdistance(float dis);

    // R�cup�re la distance depuis la source
    float getdistance();

    // D�finit l'heuristique
    void setheuristic(float dis);

    // R�cup�re l'heuristique
    float getheuristic();

    // Met � jour la couleur en fonction du type
    void updateColor();

    // Retourne une r�f�rence vers la cellule actuelle
    Cell& getCell();

    // Met � jour la couleur avec une couleur sp�cifique
    void updateColor(const sf::Color& couleur);

    // G�re le clic sur la cellule
    void handleClick(bool& startPointDefined, bool& endPointDefined, Cell& start, Cell& end);

    // D�finit la couleur de la cellule
    void SetColor(sf::Color& couleur);

    // D�finit la cellule comme un mur
    void SetWall();

    // Surcharge de l'op�rateur == pour comparer deux cellules
    bool operator==(const Cell& C);

    // R�cup�re le type de la cellule
    CellType getType() const;

    // D�finit un nouveau type pour la cellule
    void setType(CellType newType);

    // D�finit la couleur de la cellule avec une couleur constante
    void SetColor(const sf::Color& couleur);

    // Nettoie la cellule 
    void clean();
};
