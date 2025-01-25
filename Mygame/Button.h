#pragma once
#include<iostream>
#include<SFML/Graphics.hpp> 
#include "Cell.h"
#include "ChecklistItem.h"
#include "Fenetre.h"

// class pour le bouton
class Button 
{
private:
    sf::RectangleShape shape;   //Rectangle pour contenir le texte du Bouton
    sf::Text label;             //Texte du Bouton

public:
    //Constructeur
    Button(sf::Font& font, const std::string& text, const sf::Vector2f& position, const sf::Vector2f& size);
   
    //Dessine le Bouton
    void draw(sf::RenderWindow& window);

    //Fontion pour designer le Bouton qui clicqué
    bool isClicked(const sf::Vector2f& mousePos);

    //Gestionnaire de clic d'un bouton
    void onClick(std::vector<Button*> &Buttons,std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist,Cell &start,Cell &end,Fenetre &F, bool& startPointDefined, bool& endPointDefined);

    //Gestionnaire des choix des checkbox
    void handleChecklistSelection(std::vector<Button*>& Buttons,std::vector<std::vector<Cell>>& grid,std::vector<ChecklistItem>& checklist,Cell &start, Cell &end,Fenetre &F);
    
};
