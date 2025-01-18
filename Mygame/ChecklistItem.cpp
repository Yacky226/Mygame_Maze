#include "ChecklistItem.h"

// Constructeur de la classe ChecklistItem
ChecklistItem::ChecklistItem(sf::Font& font, const std::string& text, const sf::Vector2f& position)
    : checked(false) // Initialise l'état à non coché
{
    // Configuration de la case à cocher
    checkbox.setSize(sf::Vector2f(20, 20)); // Dimensions de la case
    checkbox.setFillColor(sf::Color::White); // Couleur de remplissage (blanc par défaut)
    checkbox.setOutlineThickness(2); // Épaisseur du contour
    checkbox.setOutlineColor(sf::Color::Black); // Couleur du contour
    checkbox.setPosition(position); // Position de la case à cocher

    // Configuration du texte de l'étiquette
    label.setFont(font); // Définit la police pour l'étiquette
    label.setString(text); // Texte associé à la case
    label.setCharacterSize(20); // Taille des caractères
    label.setFillColor(sf::Color::White); // Couleur du texte
    label.setPosition(position.x + 30, position.y - 5); // Position ajustée par rapport à la case
}

// Méthode pour dessiner la case à cocher et son étiquette
void ChecklistItem::draw(sf::RenderWindow& window)
{
    // Dessine la case à cocher
    window.draw(checkbox);

    // Dessine l'étiquette associée
    window.draw(label);

    // Si la case est cochée, dessiner un indicateur (tick)
    if (checked)
    {
        sf::RectangleShape tick(sf::Vector2f(14, 14)); // Indicateur (tick)
        tick.setFillColor(sf::Color::Green); // Couleur de l'indicateur
        tick.setPosition(checkbox.getPosition().x + 3, checkbox.getPosition().y + 3); // Position centrée dans la case
        window.draw(tick);
    }
}

// Méthode pour récupérer le texte de l'étiquette
std::string ChecklistItem::getText() const
{
    return label.getString().toAnsiString(); // Retourne le texte en format ANSI
}

// Méthode pour vérifier si la case est cochée
bool ChecklistItem::isChecked() const
{
    return checked;
}

// Méthode pour gérer les clics sur les cases à cocher
void ChecklistItem::handleClick(const sf::Vector2f& mousePosition, std::vector<ChecklistItem>& checklist)
{
    // Parcourt toutes les cases de la checklist
    for (auto& item : checklist)
    {
        // Vérifie si la position du clic se trouve dans la zone de la case
        if (item.checkbox.getGlobalBounds().contains(mousePosition))
        {
            // Décoche toutes les autres cases
            for (auto& otherItem : checklist)
            {
                otherItem.checked = false;
            }
            // Coche la case cliquée
            item.checked = true;
        }
    }
}
