#include "DrawInterface.h"
#include <ctime>
#include <limits>
#include <cmath>

// Dessiner la grille
void DrawInterface::drawgrid(Fenetre& F, std::vector<std::vector<Cell>>& grid)
{
    sf::RenderWindow& window = F.getWindow(); // Récupération de la fenêtre de rendu
    for (auto& row : grid) { // Parcourir chaque ligne de la grille
        for (auto& cell : row) { // Parcourir chaque cellule dans la ligne
            window.draw(cell.cel); // Dessiner la forme graphique associée à la cellule
        }
    }
}

// Dessiner la checklist
void DrawInterface::drawChecklist(Fenetre& F, std::vector<ChecklistItem>& checklist)
{
    sf::RenderWindow& window = F.getWindow(); // Récupération de la fenêtre de rendu
    for (auto& item : checklist) { // Parcourir chaque élément de la checklist
        item.draw(window); // Dessiner l'élément graphique dans la fenêtre
    }
}

// Dessiner les boutons
void DrawInterface::drawButtons(Fenetre& F, std::vector<Button*>& Buttons)
{
    sf::RenderWindow& window = F.getWindow(); // Récupération de la fenêtre de rendu
    for (auto& button : Buttons) { // Parcourir chaque bouton
        button->draw(window); // Dessiner le bouton graphique dans la fenêtre
    }
}

// Générer une carte aléatoire
void DrawInterface::generateRandomMap(std::vector<std::vector<Cell>>& grid)
{
    // Initialisation de la graine du générateur aléatoire avec le temps actuel
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (auto& row : grid) { // Parcourir chaque ligne de la grille
        for (auto& cell : row) { // Parcourir chaque cellule dans la ligne
            if (std::rand() % 4 == 0) { // 25% de chance de transformer la cellule en mur
                cell.clean(); // Nettoyer les propriétés de la cellule
                cell.setType(CellType::Wall); // Définir la cellule comme un mur
            }
            else {
                cell.clean(); // Nettoyer les propriétés de la cellule
                cell.setType(CellType::None); // Définir la cellule comme vide
            }
            // Réinitialiser la distance pour chaque cellule à l'infini
            cell.setdistance(std::numeric_limits<float>::infinity());
        }
    }
}

// Nettoyer la carte
void DrawInterface::cleanMap(std::vector<std::vector<Cell>>& grid)
{
    for (auto& row : grid) { // Parcourir chaque ligne de la grille
        for (auto& cell : row) { // Parcourir chaque cellule dans la ligne
            cell.clean(); // Réinitialiser les propriétés de la cellule
        }
    }
}
