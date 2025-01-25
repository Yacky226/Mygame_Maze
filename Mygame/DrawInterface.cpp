#include "DrawInterface.h"
#include <ctime>
#include <limits>
#include <cmath>

// Dessiner la grille
void DrawInterface::drawgrid(Fenetre& F, std::vector<std::vector<Cell>>& grid)
{
    sf::RenderWindow& window = F.getWindow(); // R�cup�ration de la fen�tre de rendu
    for (auto& row : grid) { // Parcourir chaque ligne de la grille
        for (auto& cell : row) { // Parcourir chaque cellule dans la ligne
            window.draw(cell.cel); // Dessiner la forme graphique associ�e � la cellule
        }
    }
}

// Dessiner la checklist
void DrawInterface::drawChecklist(Fenetre& F, std::vector<ChecklistItem>& checklist)
{
    sf::RenderWindow& window = F.getWindow(); // R�cup�ration de la fen�tre de rendu
    for (auto& item : checklist) { // Parcourir chaque �l�ment de la checklist
        item.draw(window); // Dessiner l'�l�ment graphique dans la fen�tre
    }
}

// Dessiner les boutons
void DrawInterface::drawButtons(Fenetre& F, std::vector<Button*>& Buttons)
{
    sf::RenderWindow& window = F.getWindow(); // R�cup�ration de la fen�tre de rendu
    for (auto& button : Buttons) { // Parcourir chaque bouton
        button->draw(window); // Dessiner le bouton graphique dans la fen�tre
    }
}

// G�n�rer une carte al�atoire
void DrawInterface::generateRandomMap(std::vector<std::vector<Cell>>& grid)
{
    // Initialisation de la graine du g�n�rateur al�atoire avec le temps actuel
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    for (auto& row : grid) { // Parcourir chaque ligne de la grille
        for (auto& cell : row) { // Parcourir chaque cellule dans la ligne
            if (std::rand() % 4 == 0) { // 25% de chance de transformer la cellule en mur
                cell.clean(); // Nettoyer les propri�t�s de la cellule
                cell.setType(CellType::Wall); // D�finir la cellule comme un mur
            }
            else {
                cell.clean(); // Nettoyer les propri�t�s de la cellule
                cell.setType(CellType::None); // D�finir la cellule comme vide
            }
            // R�initialiser la distance pour chaque cellule � l'infini
            cell.setdistance(std::numeric_limits<float>::infinity());
        }
    }
}

// Nettoyer la carte
void DrawInterface::cleanMap(std::vector<std::vector<Cell>>& grid)
{
    for (auto& row : grid) { // Parcourir chaque ligne de la grille
        for (auto& cell : row) { // Parcourir chaque cellule dans la ligne
            cell.clean(); // R�initialiser les propri�t�s de la cellule
        }
    }
}
