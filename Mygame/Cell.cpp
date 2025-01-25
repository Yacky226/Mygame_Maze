#include "Cell.h"
#include <limits> 

// Constructeur avec param�tres : initialise les propri�t�s de la cellule 
Cell::Cell(int abs, int ord, float x, float y, int w, int h)
    : posX(x), posY(y), width(w), height(h), cellcolor(sf::Color::White), type(None), indexX(abs), indexY(ord)
{
    // Configuration de l'apparence graphique de la cellule
    cel.setSize(sf::Vector2f(w, h));       // D�finit la taille du rectangle SFML
    cel.setPosition(sf::Vector2f(x, y));  // Position initiale de la cellule sur la fen�tre
    cel.setFillColor(sf::Color::White);   // Couleur de remplissage initiale
    cel.setOutlineThickness(2);           // �paisseur des contours
    cel.setOutlineColor(sf::Color::Black); // Couleur des contours
}

// Constructeur par d�faut : initialise une cellule avec des valeurs minimales par d�faut
Cell::Cell() : posX(0), posY(0), width(10), height(10), cellcolor(sf::Color::White), type(None) {}

// Constructeur par copie : copie les attributs d'une autre cellule
Cell::Cell(const Cell& C)
{
    type = C.type;               // Type de la cellule (None, StartPoint, EndPoint, Wall)
    cellcolor = C.cellcolor;     // Couleur actuelle de la cellule
    cel = C.cel;                 // Rectangle graphique SFML
    posX = C.posX;               // Position X
    posY = C.posY;               // Position Y
    height = C.height;           // Hauteur de la cellule
    width = C.width;             // Largeur de la cellule
    heuristic = C.heuristic;     // Heuristique (utilis�e pour l'algorithme A*)
    parent = C.parent;           // Parent dans le chemin 
    distance = C.distance;       // Distance calcul�e depuis le d�part
    visited = C.visited;         // �tat visit� ou non
    indexX = C.indexX;           // indice X dans une grille
    indexY = C.indexY;           // indice Y dans une grille
}

// D�finit le parent de la cellule utilis� pour reconstruire un chemin
void Cell::setparent(Cell* c)
{
    this->parent = c;
}

//R�cup�re le parent de la cellule
Cell* Cell::getparent()
{
    return parent;
}

//D�finit la cellule comme visit�e
void Cell::setVisited()
{
    this->visited = true;
}

// V�rifie si la cellule est visit�e
bool Cell::getVisited()
{
    return visited;
}

// D�finit la distance associ�e � la cellule pour Dijkstra
void Cell::setdistance(float dis)
{
    distance = dis;
}

// R�cup�re la distance associ�e � la cellule
float Cell::getdistance()
{
    return distance;
}

// D�finit la valeur heuristique de la cellule utilis�e pour A*
void Cell::setheuristic(float dis)
{
    heuristic = dis;
}

// R�cup�re la valeur heuristique
float Cell::getheuristic()
{
    return heuristic;
}

// Met � jour la couleur de la cellule en fonction de son type
void Cell::updateColor()
{
    switch (type)
    {
    case StartPoint:
        cellcolor = sf::Color::Green; // Point de d�part
        break;
    case EndPoint:
        cellcolor = sf::Color::Red;   // Point d'arriv�e
        break;
    case Wall:
        cellcolor = sf::Color::Black; // Mur
        break;
    case None:
        cellcolor = sf::Color::White; // Cellule vide
        break;
    }
    cel.setFillColor(cellcolor); // Applique la couleur au rectangle SFML
}

// Retourne une r�f�rence � l'objet cellule
Cell& Cell::getCell()
{
    return *this;
}

// Met � jour la couleur de la cellule avec une couleur personnalis�e
void Cell::updateColor(const sf::Color& couleur)
{
    cellcolor = couleur;
    cel.setFillColor(cellcolor);
}

// G�re les clics sur la cellule pour changer son type (point de d�part, arriv�e, mur ou vide)
void Cell::handleClick(bool& startPointDefined, bool& endPointDefined, Cell& start, Cell& end)
{
    if (type == StartPoint) // Si la cellule est le point de d�part, on la r�initialise
    {
        type = None;
        startPointDefined = false;
        start = *this;
    }
    else if (type == EndPoint) // Si la cellule est le point d'arriv�e, on la r�initialise
    {
        type = None;
        endPointDefined = false;
        end = *this;
    }
    else if (type == Wall) // Si c'est un mur, on le r�initialise
    {
        type = None;
    }
    else
    {
        if (!startPointDefined) // D�finit comme point de d�part si non d�fini
        {
            type = StartPoint;
            startPointDefined = true;
            start = *this;
        }
        else if (!endPointDefined) // D�finit comme point d'arriv�e si non d�fini
        {
            type = EndPoint;
            endPointDefined = true;
            end = *this;
        }
        else // Sinon, transforme la cellule en mur
        {
            type = Wall;
        }
    }
    updateColor(); // Met � jour la couleur apr�s modification
}

// D�finit une couleur sp�cifique pour la cellule
void Cell::SetColor(sf::Color& couleur)
{
    cellcolor = couleur;
    cel.setFillColor(cellcolor);
}

// R�initialise la cellule � son �tat par d�faut
void Cell::clean()
{
    this->cellcolor = sf::Color::White; // Couleur par d�faut
    this->heuristic = 0.0f;            // Heuristique � z�ro
    this->parent = nullptr;            // Aucun parent
    this->type = CellType::None;       // Type vide
    this->visited = false;             // Non visit�
    this->cel.setFillColor(this->cellcolor); // Applique la couleur par d�faut
    distance = std::numeric_limits<float>::infinity(); // Distance infinie par d�faut
}

// D�finit la cellule comme mur
void Cell::SetWall()
{
    type = Wall;
    updateColor();
}

// Surcharge de l'op�rateur == pour comparer deux cellules
bool Cell::operator==(const Cell& C)
{
    return (this->type == C.type) && (this->indexX == C.indexX) && (this->indexY == C.indexY);
}

// D�finit le type de la cellule
void Cell::setType(CellType newType)
{
    type = newType;
    updateColor();
}

//R�cup�re le type de la cellule
CellType Cell::getType() const
{
    return type;
}
