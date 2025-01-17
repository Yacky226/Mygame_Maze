#pragma once
#include<SFML/Graphics.hpp>

enum CellType { None, StartPoint, EndPoint, Wall }; // Types de cellules

class Cell {

private:

	float cellesize=20.0f;
	float posX;
	float posY;
	int height;
	int width;
	float heuristic = 0.0f; // Estimation du coût jusqu'à la fin
	CellType type;     // Type de la cellule
	Cell* parent = nullptr; // Pour retracer le chemin
	float distance = std::numeric_limits<float>::infinity(); // Pour Dijkstra
	bool visited = false; // Indique si la cellule a été visitée

public:
	int indexX = -1;
	int indexY = -1;
	sf::Color cellcolor = sf::Color::White;
	sf::RectangleShape cel;

	Cell(int x,int y,float posX, float posY, int height, int width);
	Cell();
	Cell(const Cell& C);
	void setparent(Cell* c);
	Cell* getparent();
	void setVisited();
	bool getVisited();
	void setdistance(float dis);
	float getdistance();
	void setheuristic(float dis);
	float getheuristic();
	void updateColor();
	Cell& getCell();
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

