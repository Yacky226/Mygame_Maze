#pragma once
#include <iostream>
#include<SFML/Graphics.hpp>
#include<string>
// Structure pour un �l�ment de checklist
struct ChecklistItem 
{
    sf::RectangleShape checkbox; // Case � cocher
    sf::Text label;              // Texte de l'�l�ment
    bool checked;                // �tat de la case

   
    ChecklistItem(sf::Font& font, const std::string& text, const sf::Vector2f& position);
    void draw(sf::RenderWindow& window);
    std::string getText()const;
    bool isChecked() const;
  static  void handleClick(const sf::Vector2f& mousePosition, std::vector<ChecklistItem>& checklist); // G�rer les clics
};