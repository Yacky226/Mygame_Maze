#pragma once
#include<SFML/Graphics.hpp>

enum CellType { None, StartPoint, EndPoint, Wall }; // Types de cellules
struct Cell {

	sf::Color cellcolor = sf::Color::White;
	sf::RectangleShape cel;
	float posX;
	float posY;
	int height;
	int width;
	int indexX=-1;
	int indexY=-1;
	float heuristic = 0.0f; // Estimation du coût jusqu'à la fin
	CellType type;     // Type de la cellule
	bool istarget=false;
	Cell* parent = nullptr; // Pour retracer le chemin
	float distance = std::numeric_limits<float>::infinity(); // Pour Dijkstra
	bool visited = false; // Indique si la cellule a été visitée

public:
	Cell(int x,int y,float posX, float posY, int height, int width);
	Cell();
	Cell(const Cell& C);
	void animateVisited(sf::RenderWindow& window, float duration);
	void updateColor();
    void updateColor(const sf::Color& couleur);
	void handleClick(bool& startPointDefined, bool& endPointDefined,Cell &start,Cell &end);
	void SetColor(sf::Color &couleur);
	void SetWall();
	bool operator==(const Cell& C);
	CellType getType() const;
	void setType(CellType newType);
    void SetColor(const sf::Color &couleur);
	void clean();

};

