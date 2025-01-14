#include "ChecklistItem.h"

ChecklistItem::ChecklistItem(sf::Font& font, const std::string& text, const sf::Vector2f& position)
    : checked(false)
{
    checkbox.setSize(sf::Vector2f(20, 20));
    checkbox.setFillColor(sf::Color::White);
    checkbox.setOutlineThickness(2);
    checkbox.setOutlineColor(sf::Color::Black);
    checkbox.setPosition(position);

    label.setFont(font);
    label.setString(text);
    label.setCharacterSize(20);
    label.setFillColor(sf::Color::White);
    label.setPosition(position.x + 30, position.y - 5);
}

void ChecklistItem::draw(sf::RenderWindow& window)
{
    window.draw(checkbox);
    window.draw(label);

    if (checked)
    {
        sf::RectangleShape tick(sf::Vector2f(14, 14)); // Indicateur de sélection
        tick.setFillColor(sf::Color::Green);
        tick.setPosition(checkbox.getPosition().x + 3, checkbox.getPosition().y + 3);
        window.draw(tick);
    }
}

std::string ChecklistItem::getText()const
{
    return label.getString().toAnsiString();
}

bool ChecklistItem::isChecked()const
{
    return checked;
}

void ChecklistItem::handleClick(const sf::Vector2f& mousePosition, std::vector<ChecklistItem>& checklist)
{
   
    for (auto& item : checklist)
    {
        if (item.checkbox.getGlobalBounds().contains(mousePosition))
        {
            for (auto& otherItem : checklist)
            {
                otherItem.checked = false;
            }
            item.checked = true;
        }
    }
}
