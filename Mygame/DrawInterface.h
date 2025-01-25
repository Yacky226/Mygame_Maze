#pragma once
#include <SFML/Graphics.hpp>
#include "Fenetre.h"
#include "ChecklistItem.h"
#include "Button.h"
class DrawInterface
{
public:
	// Dessiner la grille
	static void drawgrid(Fenetre& F, std::vector<std::vector<Cell>>& grid);
    // Dessiner la checklist dans la fen�tre
    static void drawChecklist(Fenetre& F,std::vector<ChecklistItem>& checklist);
    // Dessiner les boutons dans la fen�tre
    static void drawButtons(Fenetre& F, std::vector<Button*>& Buttons);
	// G�n�rer une carte al�atoire
    static void generateRandomMap(std::vector<std::vector<Cell>>& grid);
	// Nettoyer la carte
    static void cleanMap(std::vector<std::vector<Cell>>& grid);
};

