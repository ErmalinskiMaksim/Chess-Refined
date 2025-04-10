#include "../headers/Piece.h"

bool Piece::s_anyPiecesSelected = false;
bool Piece::s_isTurnOver = false;

Piece::Piece(std::unique_ptr<PieceType> pt)
	:  m_selected(false), m_hiden(false) , m_type(std::move(pt)) {}

void Piece::processEvents(const PieceType::Coordinates& boardPos, Window& window, std::vector<std::string>& ml)
{
	if (!m_hiden)
	{
		if (m_type->m_coordinates == boardPos && !s_anyPiecesSelected) // selects a piece
		{
			if (!PieceType::s_checkedKing.first && m_type->m_possibleMoves.empty())
				m_type->calculatePossibleMoves(ml);

			m_type->checkIfIsPinned(ml);

			if (!(m_type->m_possibleMoves.empty())) // if there are moves left it gets selected
			{
				m_selected = true;
				s_anyPiecesSelected = true;
			}
		}
		else if (m_selected && s_anyPiecesSelected) // moves
		{
			// * toPosition has only 2 refs
			// * toBoard has only 1 ref
			if (std::find(m_type->m_possibleMoves.begin(), m_type->m_possibleMoves.end(), boardPos) != m_type->m_possibleMoves.end())
			{
				PieceType::s_dangerousPieces.clear();
				PieceType::s_checkedKing.first = false;

				m_type->setPosition(PieceType::Coordinates::toPosition(boardPos)); // sets new coordinates
				checkForEnPassant(m_type->m_coordinates, boardPos); // erase an enpassant-ed pawn
				PieceType::s_positionInfo.erase(m_type->m_coordinates); // erases its old position in the common map
				m_type->m_coordinates.updateBoard(m_type->getPosition()); // updates the board coordinates

				checkForPromotion(window);

				PieceType::s_positionInfo.erase(m_type->m_coordinates); // if the position it's trying to move to is already occupied, it frees it
				PieceType::s_positionInfo.emplace(m_type->m_coordinates, m_type.get());

				checkForCastling();

				m_selected = false;
				s_anyPiecesSelected = false;
				s_isTurnOver = true; // finish the turn (happens here or at the end of promotion)

				if (m_type->isFirstTime())
					m_type->setFirstTime(false);

				m_type->updateMoveList(ml);
			}
			else if (m_type->m_coordinates == boardPos)
			{
				m_selected = false;
				s_anyPiecesSelected = false;
			}
		}
	}
}

void Piece::update(const std::vector<std::string>& ml)
{
	if (!m_hiden)
	{
		m_type->m_coordinates.updateBoard(m_type->getPosition()); // update coordinates

		if (!PieceType::s_positionInfo.contains(m_type->m_coordinates)) // there's no one at its coordinates in the map, it inserts itself
		{
			PieceType::s_positionInfo.emplace(m_type->m_coordinates, m_type.get());
		}
		else if (m_type->getColour() != PieceType::s_positionInfo.at(m_type->m_coordinates)->getColour())
		{
			// if it's missing from the map at its coordinates and its place is taken, it hides itself in order not to be processed further in the game
			m_hiden = true;
			m_type->m_possibleMoves.clear(); // importatnt for determining when it's a mate
			return;
		}

		if (s_isTurnOver) // recaclculate its moves according to the new board setup and check if it can reach the opponent's king
		{
			m_type->calculatePossibleMoves(ml);
			m_type->checkTheKing();
			m_type->tryToPreventCheck();
		}
	}
}

void Piece::display(Window& window, sf::Sprite s, sf::Texture t)
{
	if (!m_hiden)
	{
		t.loadFromFile(m_type->getDirectory());
		s.setTexture(t);
		s.setPosition(sf::Vector2<float>(m_type->getPosition()));
		window.draw(s);
	
		if (m_selected)
		{
			sf::RectangleShape rect({ SQUARE_WIDTH, SQUARE_HEIGHT });
			rect.setFillColor({ 200, 0, 128, 100 });
			for (auto&& a : m_type->m_possibleMoves)
			{
				rect.setPosition(sf::Vector2<float>(PieceType::Coordinates::toPosition(a)));
				window.draw(rect);
			}
		}
	}
}


const PieceType* const Piece::getType() const
{
	return m_type.get();
}

void Piece::setPosition(const sf::Vector2i& pos)
{
	m_type->setPosition(pos);
}

void Piece::checkForEnPassant(const PieceType::Coordinates& current, const PieceType::Coordinates& next)
{
	if (typeid(*m_type) == typeid(Pawn) && !PieceType::s_positionInfo.contains(next) && next.x != current.x)
	{
		PieceType::s_positionInfo.at({ next.x, current.y })->setPosition(m_type->getPosition());
		PieceType::s_positionInfo.erase({ next.x, current.y });
	}
}

void Piece::checkForPromotion(Window& window)
{
	// if it's a pawn and its coordinates are on an according end of the board
	// code smells here but I'll improve it (probably)
	if (typeid(*m_type) == typeid(Pawn) && m_type->m_coordinates.y == 7 * static_cast<int>(m_type->getColour()) && m_selected)
	{
		const float rectangleHeight = SQUARE_HEIGHT + 20;
		const float rectangleWidth = rectangleHeight * 4;
		const sf::Vector2f rectanglePosition = { (BOARD_WIDTH - rectangleWidth) / 2, (BOARD_HEIGHT - rectangleHeight) / 2 };

		sf::RectangleShape rectangle({ rectangleWidth, rectangleHeight });
		rectangle.setPosition(rectanglePosition);
		rectangle.setFillColor({ 0, 127, 0, 127 });
		window.draw(rectangle);

		std::string dirPrefix;
		static_cast<bool>(m_type->getColour()) ? dirPrefix = "w" : dirPrefix = "b";
		std::vector<sf::Texture> promotionPieces(4);
		promotionPieces[0].loadFromFile("Images/Pieces/" + dirPrefix + "R.png");
		promotionPieces[1].loadFromFile("Images/Pieces/" + dirPrefix + "N.png");
		promotionPieces[2].loadFromFile("Images/Pieces/" + dirPrefix + "B.png");
		promotionPieces[3].loadFromFile("Images/Pieces/" + dirPrefix + "Q.png");

		sf::Sprite s;
		for (int i = 0; i < 4; ++i)
		{
			s.setTexture(promotionPieces[i]);
			s.setPosition((BOARD_WIDTH - rectangleWidth) / 2 + rectangleWidth * i / 4, (BOARD_HEIGHT - rectangleHeight) / 2);
			window.draw(s);
		}

		window.display();

		sf::Event e;
		while (true)
			if (window.waitEvent(e) && e.type == sf::Event::MouseButtonPressed)
			{
				const sf::Vector2<float> click(sf::Mouse::getPosition(window.getWindow()));
				if (rectangle.getGlobalBounds().contains(click))
				{
					m_type->m_possibleMoves.clear();
					PieceType::Coordinates c = m_type->m_coordinates;
					const auto& pos = m_type->getPosition();
					TeamColour colour = m_type->getColour();

					if (click.x - rectanglePosition.x <= rectangleWidth / 4) m_type = std::make_unique<Rook>(c.x, c.y, colour);
					else if (click.x - rectanglePosition.x <= rectangleWidth / 2) m_type = std::make_unique<Knight>(c.x, c.y, colour);
					else if (click.x - rectanglePosition.x <= 3 * rectangleWidth / 4) m_type = std::make_unique<Bishop>(c.x, c.y, colour);
					else m_type = std::make_unique<Queen>(c.x, c.y, colour);

					m_type->setPosition(pos);

					promotionPieces.clear();

					break;
				}
			}
	}
}

void Piece::checkForCastling()
{
	int y = m_type->m_coordinates.y;
	if (typeid(*m_type) == typeid(King) && m_type->isFirstTime())
	{
		if (m_type->m_coordinates == PieceType::Coordinates(2, y)) // left rook
		{
			PieceType::s_positionInfo.emplace(PieceType::Coordinates(3, y), PieceType::s_positionInfo.at({ 0, y }));
			PieceType::s_positionInfo.erase({ 0, y });
			PieceType::s_positionInfo.at({ 3, y })->setPosition({ SQUARE_WIDTH * 3, SQUARE_HEIGHT * (7 - y) });
			// update the coordinates of both rooks like in update()
		}
		else if (m_type->m_coordinates == PieceType::Coordinates(6, y)) // right rook
		{
			PieceType::s_positionInfo.emplace(PieceType::Coordinates(5, y), PieceType::s_positionInfo.at({ 7, y }));
			PieceType::s_positionInfo.erase({ 7, y });
			PieceType::s_positionInfo.at({ 5, y })->setPosition({ SQUARE_WIDTH * 5, SQUARE_HEIGHT * (7 - y) });
		}
	}
}

int Piece::getValue() const
{
	if (!m_hiden) return m_type->getValue();
	else return 0;
}

const sf::Vector2i Piece::getPosition() const
{
	return m_type->getPosition();
}