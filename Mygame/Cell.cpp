#include "Cell.h"
#include<iostream>


// Constructeur avec param�tres
Cell::Cell(int abs ,int ord,float x, float y, int w, int h)
    : posX(x), posY(y), width(w), height(h), cellcolor(sf::Color::White), type(None),indexX(abs),indexY(ord)
{
    cel.setSize(sf::Vector2f(w, h));
    cel.setPosition(sf::Vector2f(x, y));
    cel.setFillColor(sf::Color::White);
    cel.setOutlineThickness(2);
    cel.setOutlineColor(sf::Color::Black);
}

// Constructeur par d�faut
Cell::Cell() : posX(0), posY(0), width(10), height(10), cellcolor(sf::Color::White), type(None) {}

Cell::Cell(const Cell& C)
{
    type = C.type;
    cellcolor = C.cellcolor;
    cel = C.cel;
    posX = C.posX;
    posY = C.posY;
    height = C.height;
    width = C.width;
    heuristic = C.heuristic;
    parent = C.parent;
    distance = C.distance;
    visited = C.visited;
    indexX = C.indexX;
    indexY = C.indexY;

}

void Cell::setparent(Cell* c)
{
    this->parent = c;
}

Cell* Cell::getparent()
{
    return parent;
}

void Cell::setVisited()
{
    this->visited = true;
}

bool Cell::getVisited()
{
    return visited;
}

void Cell::setdistance(float dis)
{
    distance = dis;
}

float Cell::getdistance()
{
    return distance;
}

void Cell::setheuristic(float dis)
{
    heuristic = dis;
}

float Cell::getheuristic()
{
    return heuristic;
}

// Mise � jour de la couleur de la cellule en fonction de son type
void Cell::updateColor()
{
    switch (type)
    {
    case StartPoint:
        cellcolor = sf::Color::Green; // Couleur de d�part
        break;
    case EndPoint:
        cellcolor = sf::Color::Red;   // Couleur d'arriv�e
        break;
    case Wall:
        cellcolor = sf::Color::Black; // Couleur de mur
        break;
    case None:
        cellcolor = sf::Color::White; // Cellule vide
        break;
    }
    cel.setFillColor(cellcolor); // Appliquer la couleur
}

Cell& Cell::getCell()
{
    return *this;
   
}


void Cell::updateColor(const sf::Color& couleur)
{
    cellcolor = couleur;
    cel.setFillColor(cellcolor);
}

// G�rer les clics sur une cellule
void Cell::handleClick(bool& startPointDefined, bool& endPointDefined,Cell &start,Cell &end)
{
    // R�initialiser les types si d�j� d�finis
    if (type == StartPoint) 
    {
        type = None;
        startPointDefined = false;
        start = *this;
    }
    else if (type == EndPoint)
    {
        type = None;
        endPointDefined = false;
        end = *this;

    }
    else if (type == Wall) {
        type = None;
    }
    else {
        // Priorit� : D�finir le point de d�part
        if (!startPointDefined)
        {
            type = StartPoint;
            startPointDefined = true;
            start = *this;
        
        }
        // D�finir le point d'arriv�e
        else if (!endPointDefined) 
        {
            type = EndPoint;
            endPointDefined = true;
            end = *this;
        }
        // Sinon, d�finir comme mur
        else {
            type = Wall;
        }
    }
    updateColor(); // Mettre � jour la couleur apr�s changement
}

// D�finir une couleur sp�cifique
void Cell::SetColor(sf::Color& couleur)
{
    cellcolor = couleur;
    cel.setFillColor(cellcolor);
}

void Cell::clean()
{
    this->cellcolor = sf::Color::White;
    this->heuristic = 0.0f;
    this->parent = nullptr;
    this->type = CellType::None;
    this->visited = false;
    this->cel.setFillColor(this->cellcolor);
}

// D�finir la cellule comme mur
void Cell::SetWall()
{
    type = Wall;
    updateColor();
}

bool Cell::operator==(const Cell& C)
{
    if ((this->type == C.type) && (this->indexX == C.indexX) && (this->indexY = C.indexY))
    {
        return true;
    }

    return false;
  
}

// D�finir le type de cellule
void Cell::setType(CellType newType)
{
    type = newType;
    updateColor();
}

// R�cup�rer le type de la cellule
CellType Cell::getType() const
{
    return type;
}

