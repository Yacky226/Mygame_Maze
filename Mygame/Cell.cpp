// Inclusion des en-têtes nécessaires
#include "Cell.h"
#include<iostream>

// Constructeur avec paramètres : initialise les propriétés de la cellule (position, taille, couleurs, type, etc.)
Cell::Cell(int abs, int ord, float x, float y, int w, int h)
    : posX(x), posY(y), width(w), height(h), cellcolor(sf::Color::White), type(None), indexX(abs), indexY(ord)
{
    // Configuration de l'apparence graphique de la cellule
    cel.setSize(sf::Vector2f(w, h));       // Définit la taille du rectangle SFML
    cel.setPosition(sf::Vector2f(x, y));  // Position initiale
    cel.setFillColor(sf::Color::White);   // Couleur de remplissage par défaut
    cel.setOutlineThickness(2);           // Épaisseur des contours
    cel.setOutlineColor(sf::Color::Black); // Couleur des contours
}

// Constructeur par défaut : initialise une cellule avec des valeurs par défaut
Cell::Cell() : posX(0), posY(0), width(10), height(10), cellcolor(sf::Color::White), type(None) {}

// Constructeur par copie : copie les attributs d'une autre cellule
Cell::Cell(const Cell& C)
{
    type = C.type;               // Type de la cellule
    cellcolor = C.cellcolor;     // Couleur
    cel = C.cel;                 // Rectangle graphique
    posX = C.posX;               // Position X
    posY = C.posY;               // Position Y
    height = C.height;           // Hauteur
    width = C.width;             // Largeur
    heuristic = C.heuristic;     // Heuristique (utilisée pour les algorithmes de cheminement)
    parent = C.parent;           // Parent dans un chemin (algorithmes de graphe)
    distance = C.distance;       // Distance calculée
    visited = C.visited;         // État visité ou non
    indexX = C.indexX;           // Index X dans une grille
    indexY = C.indexY;           // Index Y dans une grille
}

// Définit le parent de la cellule (utilisé pour suivre un chemin)
void Cell::setparent(Cell* c)
{
    this->parent = c;
}

// Récupère le parent de la cellule
Cell* Cell::getparent()
{
    return parent;
}

// Définit la cellule comme visitée
void Cell::setVisited()
{
    this->visited = true;
}

// Vérifie si la cellule est visitée
bool Cell::getVisited()
{
    return visited;
}

// Définit la distance associée à la cellule
void Cell::setdistance(float dis)
{
    distance = dis;
}

// Récupère la distance associée à la cellule
float Cell::getdistance()
{
    return distance;
}

// Définit la valeur heuristique de la cellule (pour A* ou autre algorithme)
void Cell::setheuristic(float dis)
{
    heuristic = dis;
}

// Récupère la valeur heuristique
float Cell::getheuristic()
{
    return heuristic;
}

// Met à jour la couleur de la cellule en fonction de son type
void Cell::updateColor()
{
    switch (type)
    {
    case StartPoint:
        cellcolor = sf::Color::Green; // Couleur pour le point de départ
        break;
    case EndPoint:
        cellcolor = sf::Color::Red;   // Couleur pour le point d'arrivée
        break;
    case Wall:
        cellcolor = sf::Color::Black; // Couleur pour les murs
        break;
    case None:
        cellcolor = sf::Color::White; // Cellule vide
        break;
    }
    cel.setFillColor(cellcolor); // Applique la couleur au rectangle SFML
}

// Retourne une référence à la cellule
Cell& Cell::getCell()
{
    return *this;
}

// Met à jour la couleur de la cellule avec une couleur personnalisée
void Cell::updateColor(const sf::Color& couleur)
{
    cellcolor = couleur;
    cel.setFillColor(cellcolor);
}

// Gère les clics sur la cellule pour modifier son type (point de départ, arrivée, mur, ou vide)
void Cell::handleClick(bool& startPointDefined, bool& endPointDefined, Cell& start, Cell& end)
{
    if (type == StartPoint) // Si la cellule est le point de départ, réinitialisez-la
    {
        type = None;
        startPointDefined = false;
        start = *this;
    }
    else if (type == EndPoint) // Si la cellule est le point d'arrivée, réinitialisez-la
    {
        type = None;
        endPointDefined = false;
        end = *this;
    }
    else if (type == Wall) // Si c'est un mur, réinitialisez-le
    {
        type = None;
    }
    else
    {
        if (!startPointDefined) // Définit la cellule comme point de départ si non défini
        {
            type = StartPoint;
            startPointDefined = true;
            start = *this;
        }
        else if (!endPointDefined) // Définit la cellule comme point d'arrivée si non défini
        {
            type = EndPoint;
            endPointDefined = true;
            end = *this;
        }
        else // Sinon, la cellule devient un mur
        {
            type = Wall;
        }
    }
    updateColor(); // Met à jour la couleur après modification
}

// Définit une couleur spécifique pour la cellule
void Cell::SetColor(sf::Color& couleur)
{
    cellcolor = couleur;
    cel.setFillColor(cellcolor);
}

// Nettoie la cellule réinitialise ses paramètres par défaut
void Cell::clean()
{
    this->cellcolor = sf::Color::White;
    this->heuristic = 0.0f;
    this->parent = nullptr;
    this->type = CellType::None;
    this->visited = false;
    this->cel.setFillColor(this->cellcolor);
}

// Définit la cellule comme mur
void Cell::SetWall()
{
    type = Wall;
    updateColor();
}

// Surcharge de l'opérateur == pour comparer deux cellules
bool Cell::operator==(const Cell& C)
{
    return (this->type == C.type) && (this->indexX == C.indexX) && (this->indexY == C.indexY);
}

// Définit le type de la cellule
void Cell::setType(CellType newType)
{
    type = newType;
    updateColor();
}

// Récupère le type de la cellule
CellType Cell::getType() const
{
    return type;
}
