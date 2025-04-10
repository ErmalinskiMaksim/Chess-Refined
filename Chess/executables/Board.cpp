#include "../headers/Board.h"
#include <iostream>

Board::Board() 
	: m_teamBlack(), m_teamWhite(), m_moveList(), m_teamThatMoves(TeamColour::WHITE), m_isGameOver(false)
{
	m_teamBlack.reserve(16);
	m_teamWhite.reserve(16);
	initializeTeams();
}

void Board::processInput(sf::Event& event, Window& window)
{
	if (!m_isGameOver && event.type == sf::Event::MouseButtonPressed)
	{
		const PieceType::Coordinates pos(PieceType::Coordinates::toBoard(sf::Mouse::getPosition(window.getWindow())));

		Piece::s_isTurnOver = false;

		if (m_teamThatMoves == TeamColour::BLACK)
			for (auto&& a : m_teamBlack)
				a.processEvents(pos, window, m_moveList);
		else for (auto&& a : m_teamWhite)
				a.processEvents(pos, window, m_moveList);

		update();
	}
	else if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::R)
		restart();
}

void Board::update()
{
	auto noMoves = [](std::vector<Piece>& team)
	{
		for (auto&& a : team)
			if (!a.getType()->m_possibleMoves.empty() || !PieceType::s_checkedKing.first || PieceType::s_checkedKing.second != a.getType()->getColour())
				return false;

		return true;
	};
	if (m_teamThatMoves == TeamColour::BLACK)
	{
		for (auto&& a : m_teamBlack)
			a.update(m_moveList);

		for (auto&& a : m_teamWhite)
			a.update(m_moveList);

		if (Piece::s_isTurnOver)
			m_teamThatMoves = TeamColour::WHITE;
		else if (noMoves(m_teamBlack))
		{
			m_isGameOver = true;
			m_moveList.push_back("WHITE WINS!\n");
		}
	}
	else
	{
		for (auto&& a : m_teamWhite)
			a.update(m_moveList);

		for (auto&& a : m_teamBlack)
			a.update(m_moveList);

		if (Piece::s_isTurnOver)
			m_teamThatMoves = TeamColour::BLACK;
		else if (noMoves(m_teamWhite))
		{
			m_isGameOver = true;
			m_moveList.push_back("BLACK WINS!\n");
		}
	}

	std::system("cls");
	int sumBlack = 0, sumWhite = 0;
	for (auto&& a : m_teamBlack)
		sumBlack += a.getValue();
	for (auto&& a : m_teamWhite)
		sumWhite += a.getValue();
	if (sumWhite > sumBlack)
		std::cout << "Score: WHITE +" << (sumWhite - sumBlack) << '\n';
	else if (sumBlack > sumWhite)
		std::cout << "Score: BLACK +" << (sumBlack - sumWhite) << '\n';
	else std::cout << "Score: 0-0\n";
	std::cout << "Press 'r' to restart\n";
	std::cout << "=====Moves=====" << std::endl;
	for (auto&& a : m_moveList)
		std::cout << a << '\n';
}

void Board::display(Window& window)
{

	displayBackground(window);
	displayTeams(window);

}

void Board::restart()
{
	Piece::s_isTurnOver = false;
	Piece::s_anyPiecesSelected = false;
	PieceType::s_positionInfo.clear();
	PieceType::s_dangerousPieces.clear();
	PieceType::s_checkedKing = { false, TeamColour::BLACK };
	m_teamBlack.clear();
	m_teamWhite.clear();
	m_moveList.clear();
	initializeTeams();
	m_isGameOver = false;
	m_teamThatMoves = TeamColour::WHITE;
	update();
}

void Board::initializeTeams()
{
	auto initialization = [](std::vector<Piece>& team, TeamColour colour)
	{
		team.clear();
		team.push_back(Piece(std::make_unique<Rook>(0, 7 * (1 -static_cast<int>(colour)), colour)));
		team.push_back(Piece(std::make_unique<Knight>(1, 7 * (1 -static_cast<int>(colour)), colour)));
		team.push_back(Piece(std::make_unique<Bishop>(2, 7 * (1 -static_cast<int>(colour)), colour)));
		team.push_back(Piece(std::make_unique<Queen>(3, 7 * (1 -static_cast<int>(colour)), colour)));
		team.push_back(Piece(std::make_unique<King>(4, 7 * (1 -static_cast<int>(colour)), colour)));
		team.push_back(Piece(std::make_unique<Bishop>(5, 7 * (1 -static_cast<int>(colour)), colour)));
		team.push_back(Piece(std::make_unique<Knight>(6, 7 * (1 -static_cast<int>(colour)), colour)));
		team.push_back(Piece(std::make_unique<Rook>(7, 7 * (1 -static_cast<int>(colour)), colour)));

		for (int i = 8; i < 16; ++i)
			team.push_back(Piece(std::make_unique<Pawn>((i - 8), 6 - static_cast<int>(colour) * 5, colour)));
	};

	initialization(m_teamBlack, TeamColour::BLACK);
	initialization(m_teamWhite, TeamColour::WHITE);
}


void Board::displayTeams(Window& window)
{
	sf::Texture t;
	sf::Sprite s;
	if (m_teamThatMoves == TeamColour::BLACK)
	{
		for (auto&& a : m_teamBlack)
			a.display(window, s, t);

		for (auto&& a : m_teamWhite)
			a.display(window, s, t);
	}
	else
	{
		for (auto&& a : m_teamWhite)
			a.display(window, s, t);

		for (auto&& a : m_teamBlack)
			a.display(window, s, t);
	}
}

void Board::displayBackground(Window& window)
{
	sf::RectangleShape square({ SQUARE_WIDTH, SQUARE_HEIGHT });

	for (int raw = 0; raw < 8; ++raw)
		for (int column = 0; column < 8; ++column)
		{
			square.setPosition(static_cast<float>(column * SQUARE_WIDTH), static_cast<float>(raw * SQUARE_HEIGHT));

			(raw + column) % 2 ? square.setFillColor({ 112, 112, 112, 255 }) : square.setFillColor({ 181, 181, 181, 255 });

			window.draw(square);
		}
}