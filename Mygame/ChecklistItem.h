#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>

// Classe repr�sentant un �l�ment de checklist
class ChecklistItem
{
private:
    sf::RectangleShape checkbox; // Case � cocher, repr�sent�e par un rectangle
    sf::Text label;              // Texte associ� � la case
    bool checked;                // �tat de la case (true = coch�e, false = non coch�e)

public:
    // Constructeur : initialise un �l�ment de checklist avec une police, un texte et une position
    ChecklistItem(sf::Font& font, const std::string& text, const sf::Vector2f& position);

    // M�thode pour dessiner la case et son texte dans la fen�tre SFML
    void draw(sf::RenderWindow& window);

    // M�thode pour obtenir le texte de l'�tiquette associ�e
    std::string getText() const;

    // M�thode pour v�rifier si la case est coch�e
    bool isChecked() const;

    // M�thode statique pour g�rer les clics sur les cases de la checklist
    static void handleClick(const sf::Vector2f& mousePosition, std::vector<ChecklistItem>& checklist);
};
