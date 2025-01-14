#include "Cell.h"
#include<iostream>


// Constructeur avec paramètres
Cell::Cell(int abs ,int ord,float x, float y, int w, int h)
    : posX(x), posY(y), width(w), height(h), cellcolor(sf::Color::White), type(None),indexX(abs),indexY(ord)
{
    cel.setSize(sf::Vector2f(w, h));
    cel.setPosition(sf::Vector2f(x, y));
    cel.setFillColor(sf::Color::White);
    cel.setOutlineThickness(2);
    cel.setOutlineColor(sf::Color::Black);
}

// Constructeur par défaut
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
    istarget = C.istarget;
    parent = C.parent;
    distance = C.distance;
    visited = C.visited;
    indexX = C.indexX;
    indexY = C.indexY;

}

// Mise à jour de la couleur de la cellule en fonction de son type
void Cell::updateColor()
{
    switch (type)
    {
    case StartPoint:
        cellcolor = sf::Color::Green; // Couleur de départ
        break;
    case EndPoint:
        cellcolor = sf::Color::Red;   // Couleur d'arrivée
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

void Cell::updateColor(const sf::Color& couleur)
{
    cellcolor = couleur;
    cel.setFillColor(cellcolor);
}

// Gérer les clics sur une cellule
void Cell::handleClick(bool& startPointDefined, bool& endPointDefined,Cell &start,Cell &end)
{
    // Réinitialiser les types si déjà définis
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
        // Priorité : Définir le point de départ
        if (!startPointDefined)
        {
            type = StartPoint;
            startPointDefined = true;
            start = *this;
        
        }
        // Définir le point d'arrivée
        else if (!endPointDefined) 
        {
            type = EndPoint;
            endPointDefined = true;
            end = *this;
        }
        // Sinon, définir comme mur
        else {
            type = Wall;
        }
    }
    updateColor(); // Mettre à jour la couleur après changement
}

// Définir une couleur spécifique
void Cell::SetColor(sf::Color& couleur)
{
    cellcolor = couleur;
    cel.setFillColor(cellcolor);
}

void Cell::clean()
{
    this->cellcolor = sf::Color::White;
    this->istarget = false;
    this->heuristic = 0.0f;
    this->parent = nullptr;
    this->type = CellType::None;
    this->visited = false;
    this->cel.setFillColor(this->cellcolor);
}

// Définir la cellule comme mur
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

// Définir le type de cellule
void Cell::setType(CellType newType)
{
    type = newType;
    updateColor();
}

// Récupérer le type de la cellule
CellType Cell::getType() const
{
    return type;
}

void Cell::animateVisited(sf::RenderWindow& window, float duration)
{
    sf::Clock clock;
    float elapsed = 0.f;

    // Couleurs de transition (correspondant aux étapes CSS)
    sf::Color startColor = sf::Color(255, 165, 0); // Orange initial
    sf::Color midColor = sf::Color(111, 111, 111, 191); // Gris transparent
    sf::Color endColor = sf::Color(255, 200, 120); // Orange clair

    while (elapsed < duration)
    {
        elapsed = clock.getElapsedTime().asSeconds();
        float progress = elapsed / duration;

        // Interpolation des couleurs
        if (progress < 0.5f) {
            float factor = progress * 2;
            cel.setFillColor(sf::Color(
                static_cast<sf::Uint8>(startColor.r + factor * (midColor.r - startColor.r)),
                static_cast<sf::Uint8>(startColor.g + factor * (midColor.g - startColor.g)),
                static_cast<sf::Uint8>(startColor.b + factor * (midColor.b - startColor.b)),
                static_cast<sf::Uint8>(startColor.a + factor * (midColor.a - startColor.a))
            ));
        }
        else {
            float factor = (progress - 0.5f) * 2;
            cel.setFillColor(sf::Color(
                static_cast<sf::Uint8>(midColor.r + factor * (endColor.r - midColor.r)),
                static_cast<sf::Uint8>(midColor.g + factor * (endColor.g - midColor.g)),
                static_cast<sf::Uint8>(midColor.b + factor * (endColor.b - midColor.b)),
                static_cast<sf::Uint8>(midColor.a + factor * (endColor.a - midColor.a))
            ));
        }

        // Interpolation de l'échelle
        float scaleFactor = 0.3f + progress * (progress < 0.75f ? 1.2f - 0.3f : 1.f - 1.2f);
        cel.setScale(scaleFactor, scaleFactor);

        // Afficher la cellule mise à jour
        window.clear(sf::Color::Black); // Nettoie la fenêtre
        window.draw(cel);               // Dessine la cellule
        window.display();               // Met à jour l'affichage
    }

    // Finalisation : applique l'état final
    cel.setFillColor(endColor);
    cel.setScale(1.f, 1.f);
}

