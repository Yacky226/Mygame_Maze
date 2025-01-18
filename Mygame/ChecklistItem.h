#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include <string>

// Classe représentant un élément de checklist
class ChecklistItem
{
private:
    sf::RectangleShape checkbox; // Case à cocher, représentée par un rectangle
    sf::Text label;              // Texte associé à la case
    bool checked;                // État de la case (true = cochée, false = non cochée)

public:
    // Constructeur : initialise un élément de checklist avec une police, un texte et une position
    ChecklistItem(sf::Font& font, const std::string& text, const sf::Vector2f& position);

    // Méthode pour dessiner la case et son texte dans la fenêtre SFML
    void draw(sf::RenderWindow& window);

    // Méthode pour obtenir le texte de l'étiquette associée
    std::string getText() const;

    // Méthode pour vérifier si la case est cochée
    bool isChecked() const;

    // Méthode statique pour gérer les clics sur les cases de la checklist
    static void handleClick(const sf::Vector2f& mousePosition, std::vector<ChecklistItem>& checklist);
};
