#pragma once
#ifndef GAME_H
#define GAME_H

#include "Board.h"

class Game
{
public:
	Game() {}

	void run();
private:
	void processInput();
	void update();
	void display();

	Board m_board;
	Window m_window;
};

#endif