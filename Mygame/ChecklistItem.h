#pragma once
#include <iostream>
#include<SFML/Graphics.hpp>
#include<string>
// Structure pour un élément de checklist
struct ChecklistItem 
{
    sf::RectangleShape checkbox; // Case à cocher
    sf::Text label;              // Texte de l'élément
    bool checked;                // État de la case

   
    ChecklistItem(sf::Font& font, const std::string& text, const sf::Vector2f& position);
    void draw(sf::RenderWindow& window);
    std::string getText()const;
    bool isChecked() const;
  static  void handleClick(const sf::Vector2f& mousePosition, std::vector<ChecklistItem>& checklist); // Gérer les clics
};