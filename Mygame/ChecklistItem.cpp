#include "ChecklistItem.h"

// Constructeur de la classe ChecklistItem
ChecklistItem::ChecklistItem(sf::Font& font, const std::string& text, const sf::Vector2f& position)
    : checked(false) // Initialise l'�tat � non coch�
{
    // Configuration de la case � cocher
    checkbox.setSize(sf::Vector2f(20, 20)); // Dimensions de la case
    checkbox.setFillColor(sf::Color::White); // Couleur de remplissage (blanc par d�faut)
    checkbox.setOutlineThickness(2); // �paisseur du contour
    checkbox.setOutlineColor(sf::Color::Black); // Couleur du contour
    checkbox.setPosition(position); // Position de la case � cocher

    // Configuration du texte de l'�tiquette
    label.setFont(font); // D�finit la police pour l'�tiquette
    label.setString(text); // Texte associ� � la case
    label.setCharacterSize(20); // Taille des caract�res
    label.setFillColor(sf::Color::White); // Couleur du texte
    label.setPosition(position.x + 30, position.y - 5); // Position ajust�e par rapport � la case
}

// M�thode pour dessiner la case � cocher et son �tiquette
void ChecklistItem::draw(sf::RenderWindow& window)
{
    // Dessine la case � cocher
    window.draw(checkbox);

    // Dessine l'�tiquette associ�e
    window.draw(label);

    // Si la case est coch�e, dessiner un indicateur (tick)
    if (checked)
    {
        sf::RectangleShape tick(sf::Vector2f(14, 14)); // Indicateur (tick)
        tick.setFillColor(sf::Color::Green); // Couleur de l'indicateur
        tick.setPosition(checkbox.getPosition().x + 3, checkbox.getPosition().y + 3); // Position centr�e dans la case
        window.draw(tick);
    }
}

// M�thode pour r�cup�rer le texte de l'�tiquette
std::string ChecklistItem::getText() const
{
    return label.getString().toAnsiString(); // Retourne le texte en format ANSI
}

// M�thode pour v�rifier si la case est coch�e
bool ChecklistItem::isChecked() const
{
    return checked;
}

// M�thode pour g�rer les clics sur les cases � cocher
void ChecklistItem::handleClick(const sf::Vector2f& mousePosition, std::vector<ChecklistItem>& checklist)
{
    // Parcourt toutes les cases de la checklist
    for (auto& item : checklist)
    {
        // V�rifie si la position du clic se trouve dans la zone de la case
        if (item.checkbox.getGlobalBounds().contains(mousePosition))
        {
            // D�coche toutes les autres cases
            for (auto& otherItem : checklist)
            {
                otherItem.checked = false;
            }
            // Coche la case cliqu�e
            item.checked = true;
        }
    }
}
