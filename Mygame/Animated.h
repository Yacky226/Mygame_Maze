#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Cell.h"
#include "Button.h"
#include "ChecklistItem.h"
#include"DrawInterface.h"

class Animated
{
public:
	static void animatePath(const std::vector<Cell*>& path, std::vector<Button*>& Buttons,std::vector<ChecklistItem>& checklist, Cell* end, Fenetre& F, float durationPerCell, std::vector<std::vector<Cell>>& grid);
	static void animateCels(const std::vector<Cell*>& visitedOrder,std::vector<Button*>& Buttons, std::vector<std::vector<Cell>>& grid, std::vector<ChecklistItem>& checklist, Cell& start, Cell& end, Fenetre& F);
};

