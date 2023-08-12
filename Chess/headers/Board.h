#ifndef NGS
#define NGS
#include "Piece.h"

class Board
{
public:
	Board();
	~Board();

	void processInput(sf::Event& event, Window& window);
	void update();
	void display(Window& window);
	
private:
	void initializeTeams();

	void displayTeams(Window& window);
	void displayBackground(Window& window);

	void restart();

	std::vector<Piece> m_teamBlack;
	std::vector<Piece> m_teamWhite;

	std::list<std::string> m_moveList;

	TeamColour m_teamThatMoves;
	bool m_isGameOver;
};

#endif