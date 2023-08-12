#include "../headers/Game.h"

void Game::run()
{
	update();
	while (m_window.isOpen())
	{
		display();
		processInput();
	}
}

void Game::processInput()
{
	sf::Event e;

	if (m_window.waitEvent(e))
	{
		if (m_window.checkIfWasClosed(e))
			return;

		m_board.processInput(e, m_window);
	}
}

void Game::update()
{
	m_board.update();
}

void Game::display()
{
	m_window.clear();
	m_board.display(m_window);
	m_window.display();
}
