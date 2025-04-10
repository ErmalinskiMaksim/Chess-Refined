#include "../headers/PieceType.h"
#include <algorithm>

std::map<PieceType::Coordinates, PieceType*> PieceType::s_positionInfo{};
std::pair<bool, TeamColour> PieceType::s_checkedKing(false, TeamColour::BLACK);
std::vector<PieceType::Coordinates> PieceType::s_dangerousPieces{};

PieceType::PieceType(int x, int y, TeamColour tc, int val)
	: m_movesFirstTime(true), m_possibleMoves(), m_coordinates(x, y), m_teamColour(tc), m_value(val) 
{
	m_position = Coordinates::toPosition(m_coordinates);
}
PieceType::~PieceType(){}

TeamColour PieceType::getColour() const
{
	return m_teamColour;
}

const char* PieceType::getDirectory()const
{
	return m_textureDirectory;
}

bool PieceType::isFirstTime() const
{
	return m_movesFirstTime;
}

sf::Vector2i PieceType::getPosition() const
{
	return m_position;
}

int PieceType::getValue() const
{
	return m_value;
}

void PieceType::setFirstTime(bool b)
{
	m_movesFirstTime = b;
}

void PieceType::setPosition(const sf::Vector2i& pos)
{
	m_position = pos;
}

void PieceType::updateMoveList(std::vector<std::string>& ml)
{
	std::string pieceName = typeid(*this).name();
	pieceName = pieceName.substr(pieceName.find_last_of(" \t\n") + 1);
	pieceName.erase(2, pieceName.length() - 2);
	pieceName.push_back('A' + m_coordinates.x);
	pieceName.push_back('1' + m_coordinates.y);
	if (m_teamColour == TeamColour::WHITE)
	{
		pieceName.push_back(':');
		ml.push_back(pieceName);
	}
	else
	{
		std::string& last = ml.back();
		last += pieceName;
	}
}

void PieceType::checkTheKing()
{
	PieceType::Coordinates kingPos = [this]() // finds the opponents's king position on the board
	{
		for (auto&& a : s_positionInfo)
			if (typeid(*(a.second)) == typeid(King) && a.second->getColour() != m_teamColour)
				return a.first;
	}();

	// looks through its moves and if one of them is at opponent's king square then it checks the king
	if ((!s_checkedKing.first || s_checkedKing.second != m_teamColour) && std::find(m_possibleMoves.begin(), m_possibleMoves.end(), kingPos) != m_possibleMoves.end())
	{
		s_checkedKing.first = true;
		s_checkedKing.second = s_positionInfo.at(kingPos)->getColour();
		s_dangerousPieces.push_back(m_coordinates);
	}
}

void PieceType::tryToPreventCheck()
{
	// if the king of this piece's team is checked
	if (s_checkedKing.first && s_checkedKing.second == m_teamColour && typeid(*this) != typeid(King))
	{
		// if it's a double check then erases all moves, else tries preventing the check
		(s_dangerousPieces.size() > 1) ? m_possibleMoves.clear()
			: s_positionInfo.at(s_dangerousPieces[0])->eraseDangerousCells(m_possibleMoves, findMyKingPosition(), false);
	}
}

void PieceType::checkIfIsPinned(const std::vector<std::string>& ml)
{
	// to check if the piece is pinned it deletes itself from the common map (positionInfo), lets the other pieces recalculate their moves according to new condotions,
	// and at last if the pieces can check the king, it gets pinned and removes all the illegal moves. After that it puts itself back into the common map.
	// The moves are eventually updated in the end of the turn in the Piece::update()
	if (!m_possibleMoves.empty())
	{
		PieceType::Coordinates kingPos = findMyKingPosition();

		// no need to emplace the piece back because it's done in update()
		s_positionInfo.erase(m_coordinates);

		for (auto&& positionInfo : s_positionInfo) // looks through the pieces
			if (positionInfo.second->getColour() != m_teamColour) // finds the opposite team's
			{
				positionInfo.second->calculatePossibleMoves(ml); // calculates the new set of moves without our concrete piece

				if (kingPos != m_coordinates) // not a king
				{
					// if the king is checked these operations are useless, so it quits; else checks if king is reachable without our piece
					if (s_checkedKing.first) break;
					else if (std::find(positionInfo.second->m_possibleMoves.begin(), positionInfo.second->m_possibleMoves.end(), kingPos) != positionInfo.second->m_possibleMoves.end())
						return positionInfo.second->eraseDangerousCells(m_possibleMoves, kingPos, false);
				}
				else // a king (unlike the others) needs to look through all the opponent's pieces (regardless of being checked)
					positionInfo.second->eraseDangerousCells(m_possibleMoves, m_coordinates, true);
			}

		// removes all the moves containing pieces of the same colour.
		// I put it here because eraseDangerousCells() needs these moves to be present
		// in order not to let the king attack protected enemy piececs.
		std::erase_if(m_possibleMoves, [this](const Coordinates& move) 
			{
				auto it = s_positionInfo.find(move);
				return (it != s_positionInfo.end() && it->second->getColour() == m_teamColour);
			});
	}
}

PieceType::Coordinates PieceType::findMyKingPosition()
{
	for (auto&& a : s_positionInfo)
		if (typeid(*(a.second)) == typeid(King) && a.second->getColour() == m_teamColour)
			return a.first;
}

void PieceType::Coordinates::updateBoard(const sf::Vector2i& pos)
{
	x = pos.x / SQUARE_WIDTH;
	y = 7 - pos.y / SQUARE_HEIGHT;
}

const sf::Vector2i PieceType::Coordinates::toPosition(const PieceType::Coordinates & coor)
{
	return { coor.x * SQUARE_WIDTH, (7 - coor.y) * SQUARE_HEIGHT };
}

const sf::Vector2i PieceType::Coordinates::toBoard(const sf::Vector2i& pos)
{
	return { pos.x / SQUARE_WIDTH, 7 - pos.y / SQUARE_HEIGHT };
}

bool PieceType::Coordinates::operator<(const Coordinates& c) const
{
	return (y < c.y || (y == c.y && x < c.x));
}

bool PieceType::Coordinates::operator==(const Coordinates& c) const
{
	return (x == c.x && y == c.y);
}

bool PieceType::Coordinates::operator!=(const Coordinates& c) const
{
	return (x != c.x || y != c.y);
}


////////PAWN////////////

Pawn::Pawn(int x, int y, TeamColour tc) : PieceType(x, y, tc, 1)
{
	if (tc == TeamColour::WHITE)
		m_textureDirectory = "Images/Pieces/wP.png";
	else if (tc == TeamColour::BLACK)
		m_textureDirectory = "Images/Pieces/bP.png";
}

void Pawn::calculatePossibleMoves(const std::vector<std::string>& ml)
{
	m_possibleMoves.clear();

	int ahead = m_coordinates.y + 2 * static_cast<int>(m_teamColour) - 1; // y ahead of the pawn

	if (ahead >= 0 && ahead <= 7)
	{
		if (!s_positionInfo.contains({ m_coordinates.x, ahead })) // checks if there are no pieces straight ahead of it
		{
			m_possibleMoves.push_back({ m_coordinates.x, ahead });

			// if that's the first move it can walk two squares ahead (if the square is empty per se)
			if (m_movesFirstTime && !s_positionInfo.contains({ m_coordinates.x, ahead + 2 * static_cast<int>(m_teamColour) - 1 }))
				m_possibleMoves.push_back({ m_coordinates.x, ahead + 2 * static_cast<int>(m_teamColour) - 1 });
		}

		addMoves(m_coordinates.x - 1, ahead); // move on the left
		addMoves(m_coordinates.x + 1, ahead); // move on the right
	}

	//////////////////EN PASSANT /////////////////

	if (ahead == 2 || ahead == 5)
	{   /////////////refactor in cass I add complex tockens into the list
		auto enPassant = [ml, this, ahead](int x){
			if (auto it = s_positionInfo.find(PieceType::Coordinates(x, m_coordinates.y));
				!ml.empty() && !s_positionInfo.contains({ x, ahead })
				&& it != s_positionInfo.end() && it->second->getColour() != m_teamColour)
			{
				const std::string sLastMove = ml.back().substr(static_cast<size_t>(m_teamColour) * 5, 4);
				std::string move = "Pa";
				move.push_back('A' + x);
				move.push_back('1' + m_coordinates.y);

				if (move == sLastMove)
					m_possibleMoves.push_back({ x, ahead });
			}
		};
		enPassant(m_coordinates.x + 1);
		enPassant(m_coordinates.x - 1);
	}
}

void Pawn::eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)
{
	// the method is called when the king is checked by this pawn
	std::erase_if(possibleMoves, [this, isKing](const PieceType::Coordinates& move) {
		// if it's the king, it removes only the moves where the pawn attacks; if it's not the king, it removes everything except for the position of the pawn
		return ((std::abs(move.x - m_coordinates.x) == 1 && move.y == (m_coordinates.y + 2 * static_cast<int>(m_teamColour) - 1) && isKing) || move != m_coordinates && !isKing);
		});
}

bool Pawn::addMoves(int x, int y)
{
	// only if there are pieces of its opponent's colour
	if (auto it = s_positionInfo.find(PieceType::Coordinates(x, y));
		it != s_positionInfo.end() && it->second->getColour() != m_teamColour)
		m_possibleMoves.push_back({ x, y });
	return true;
}



//////////ROOK/////////////////

Rook::Rook(int x, int y, TeamColour tc) : PieceType(x, y, tc, 5)
{
	if (tc == TeamColour::WHITE)
		m_textureDirectory = "Images/Pieces/wR.png";
	else if (tc == TeamColour::BLACK)
		m_textureDirectory = "Images/Pieces/bR.png";
}

void Rook::calculatePossibleMoves(const std::vector<std::string>& ml)
{
	m_possibleMoves.clear();

	for (int i = m_coordinates.x - 1; i >= 0 && addMoves(i, m_coordinates.y); --i) {}
	for (int i = m_coordinates.x + 1; i <= 7 && addMoves(i, m_coordinates.y); ++i) {}
	for (int i = m_coordinates.y - 1; i >= 0 && addMoves(m_coordinates.x, i); --i) {}
	for (int i = m_coordinates.y + 1; i <= 7 && addMoves(m_coordinates.x, i); ++i) {}
}

void Rook::eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)
{	 // a piece is pinned or tries to prevent a check
	if (!isKing)
	{
		std::erase_if(possibleMoves, [this, kingPos](const PieceType::Coordinates& move) {
			return !((move.x == kingPos.x && m_coordinates.x == kingPos.x && (move.y < kingPos.y&& move.y >= m_coordinates.y || move.y > kingPos.y && move.y <= m_coordinates.y))
				|| (move.y == kingPos.y && m_coordinates.y == kingPos.y && (move.x < kingPos.x&& move.x >= m_coordinates.x || move.x > kingPos.x && move.x <= m_coordinates.x)));
		});
	} // the king removes illegal cells (both when checked and not)
	else if (std::abs(kingPos.x - m_coordinates.x) <= 1 || std::abs(kingPos.y - m_coordinates.y) <= 1)
	{
		// removes intercepting moves
		std::erase_if(possibleMoves, [this](const Coordinates& move) {
			return std::find(m_possibleMoves.begin(), m_possibleMoves.end(), move) != m_possibleMoves.end();
		});
	}
}

bool Rook::addMoves(int x, int y)
{
	m_possibleMoves.push_back({ x, y }); // adds regardless of the colour (important when it comes to protected pieces, so that the king could walk legally)

	return (!s_positionInfo.contains({ x, y })); // if it stumbles upon a piece
}



////////KNIGHT////////////

Knight::Knight(int x, int y, TeamColour tc) : PieceType(x, y, tc, 3)
{
	if (tc == TeamColour::WHITE)
		m_textureDirectory = "Images/Pieces/wN.png";
	else if (tc == TeamColour::BLACK)
		m_textureDirectory = "Images/Pieces/bN.png";
}
void Knight::calculatePossibleMoves(const std::vector<std::string>& ml)
{
	m_possibleMoves.clear();

	addMoves(m_coordinates.x - 2, m_coordinates.y + 1);
	addMoves(m_coordinates.x - 2, m_coordinates.y - 1);
	addMoves(m_coordinates.x - 1, m_coordinates.y + 2);
	addMoves(m_coordinates.x - 1, m_coordinates.y - 2);
	addMoves(m_coordinates.x + 1, m_coordinates.y + 2);
	addMoves(m_coordinates.x + 1, m_coordinates.y - 2);
	addMoves(m_coordinates.x + 2, m_coordinates.y + 1);
	addMoves(m_coordinates.x + 2, m_coordinates.y - 1);
}

void Knight::eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)
{
	// the only condition when the knight can check the king/attack nearby squares
	if (std::abs(kingPos.x - m_coordinates.x) <= 3 && std::abs(kingPos.y - m_coordinates.y) <= 3)
		std::erase_if(possibleMoves, [this, isKing](const Coordinates& move) {
			if (isKing) // removes the intercepting squares (including the protected pieces) for the king
				return std::find(m_possibleMoves.begin(), m_possibleMoves.end(), move) != m_possibleMoves.end();

			//if its not the king it just leaves its own position
			return (move != m_coordinates);
		});
}

bool Knight::addMoves(int x, int y)
{
	// adds a move only if the square is empty or there's an opponent's piece; and if it's not out of the bounds
	if (x >= 0 && y >= 0 && x <= 7 && y <= 7)
		m_possibleMoves.push_back({ x, y });
	return true;
}



/////////BISHOP//////////////

Bishop::Bishop(int x, int y, TeamColour tc) : PieceType(x, y, tc, 3)
{
	if (tc == TeamColour::WHITE)
		m_textureDirectory = "Images/Pieces/wB.png";
	else if (tc == TeamColour::BLACK)
		m_textureDirectory = "Images/Pieces/bB.png";
}

void Bishop::calculatePossibleMoves(const std::vector<std::string>& ml)
{
	m_possibleMoves.clear();

	for (int i = m_coordinates.x - 1, j = m_coordinates.y + 1; i >= 0 && j <= 7 && addMoves(i, j); i--, j++) {}
	for (int i = m_coordinates.x + 1, j = m_coordinates.y + 1; i <= 7 && j <= 7 && addMoves(i, j); i++, j++) {}
	for (int i = m_coordinates.x - 1, j = m_coordinates.y - 1; i >= 0 && j >= 0 && addMoves(i, j); i--, j--) {}
	for (int i = m_coordinates.x + 1, j = m_coordinates.y - 1; i <= 7 && j >= 0 && addMoves(i, j); i++, j--) {}
}

void Bishop::eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)
{
	if (!isKing) // if a piece is pinned or tries to prevent a check
	{
		std::erase_if(possibleMoves, [this, kingPos](const PieceType::Coordinates& move) {
			return !(((m_coordinates.x + m_coordinates.y == move.x + move.y) && (kingPos.x + kingPos.y == move.x + move.y)
				|| (m_coordinates.x - m_coordinates.y == move.x - move.y) && (kingPos.x - kingPos.y == move.x - move.y))
				&& (move.x >= m_coordinates.x && move.x < kingPos.x || move.x > kingPos.x && move.x <= m_coordinates.x));
			});
	}
	else // if the king is checked (or not) and deletes illegal moves
	{
		std::erase_if(possibleMoves, [this](const Coordinates& move) {
			return std::find(m_possibleMoves.begin(), m_possibleMoves.end(), move) != m_possibleMoves.end();
		});
	}
}

bool Bishop::addMoves(int x, int y)
{
	m_possibleMoves.push_back({ x, y });// adds regardless of the colour (important when it comes to protected pieces, so that the king could walk legally)

	return (!s_positionInfo.contains({ x, y })); // if it stumbles upon a piece
}



/////////QUEEN///////////

Queen::Queen(int x, int y, TeamColour tc) : PieceType(x, y, tc, 9)
{
	m_possibleMoves.clear();

	if (tc == TeamColour::WHITE)
		m_textureDirectory = "Images/Pieces/wQ.png";
	else if (tc == TeamColour::BLACK)
		m_textureDirectory = "Images/Pieces/bQ.png";
}
void Queen::calculatePossibleMoves(const std::vector<std::string>& ml)
{
	m_possibleMoves.clear();

	for (int i = m_coordinates.x - 1, j = m_coordinates.y + 1; i >= 0 && j <= 7 && addMoves(i, j); i--, j++) {}
	for (int i = m_coordinates.x + 1, j = m_coordinates.y + 1; i <= 7 && j <= 7 && addMoves(i, j); i++, j++) {}
	for (int i = m_coordinates.x - 1, j = m_coordinates.y - 1; i >= 0 && j >= 0 && addMoves(i, j); i--, j--) {}
	for (int i = m_coordinates.x + 1, j = m_coordinates.y - 1; i <= 7 && j >= 0 && addMoves(i, j); i++, j--) {}
	for (int i = m_coordinates.x - 1; i >= 0 && addMoves(i, m_coordinates.y); --i) {}
	for (int i = m_coordinates.x + 1; i <= 7 && addMoves(i, m_coordinates.y); ++i) {}
	for (int i = m_coordinates.y - 1; i >= 0 && addMoves(m_coordinates.x, i); --i) {}
	for (int i = m_coordinates.y + 1; i <= 7 && addMoves(m_coordinates.x, i); ++i) {}
}

void Queen::eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)
{
	if (!isKing) // if a piece is pinned or tries to prevent a check
	{
		std::erase_if(possibleMoves, [this, kingPos](const PieceType::Coordinates& move) {
			if (kingPos.x == m_coordinates.x || kingPos.y == m_coordinates.y)
				return !((move.x == kingPos.x && m_coordinates.x == kingPos.x && (move.y < kingPos.y&& move.y >= m_coordinates.y || move.y > kingPos.y && move.y <= m_coordinates.y))
					|| (move.y == kingPos.y && m_coordinates.y == kingPos.y && (move.x < kingPos.x&& move.x >= m_coordinates.x || move.x > kingPos.x && move.x <= m_coordinates.x)));
			else
				return !(((m_coordinates.x + m_coordinates.y == move.x + move.y) && (kingPos.x + kingPos.y == move.x + move.y)
					|| (m_coordinates.x - m_coordinates.y == move.x - move.y) && (kingPos.x - kingPos.y == move.x - move.y))
					&& (move.x >= m_coordinates.x && move.x < kingPos.x || move.x > kingPos.x && move.x <= m_coordinates.x));
		});
	}
	else  // if the king is checked (or not) and deletes illegal moves
	{
		std::erase_if(possibleMoves, [this](const Coordinates& move) {
			return std::find(m_possibleMoves.begin(), m_possibleMoves.end(), move) != m_possibleMoves.end();
		});
	}
}

bool Queen::addMoves(int x, int y)
{
	// adds regardless of the colour (important when it comes to protected pieces, so that the king could walk legally)
	m_possibleMoves.push_back({ x, y });
	return (!s_positionInfo.contains({ x, y }));
}



//////KING////////////

King::King(int x, int y, TeamColour tc) : PieceType(x, y, tc, 0)
{
	if (tc == TeamColour::WHITE)
		m_textureDirectory = "Images/Pieces/wK.png";
	else if (tc == TeamColour::BLACK)
		m_textureDirectory = "Images/Pieces/bk.png";
}

void King::calculatePossibleMoves(const std::vector<std::string>& ml)
{
	m_possibleMoves.clear();

	for (int i = -1; i <= 1; ++i)
	{
		addMoves(m_coordinates.x + i, m_coordinates.y + 2 * static_cast<int>(m_teamColour) - 1); // cells ahead
		if (i) addMoves(m_coordinates.x + i, m_coordinates.y); // cells at its y position
		addMoves(m_coordinates.x + i, m_coordinates.y + 1 - 2 * static_cast<int>(m_teamColour)); // cells behind
	}

	///////// Castling
	if (m_movesFirstTime && (!s_checkedKing.first || s_checkedKing.second != m_teamColour))
	{
		auto checkForStrikeThrough = [this](bool isLeftRook)
		{
			for (auto&& a : s_positionInfo)
				if (a.second->getColour() != getColour())
					if ((isLeftRook 
						&& (std::find(a.second->m_possibleMoves.begin(), a.second->m_possibleMoves.end(), PieceType::Coordinates(1, m_coordinates.y)) != a.second->m_possibleMoves.end()
						|| (std::find(a.second->m_possibleMoves.begin(), a.second->m_possibleMoves.end(), PieceType::Coordinates(3, m_coordinates.y)) != a.second->m_possibleMoves.end())))
						|| (!isLeftRook && (std::find(a.second->m_possibleMoves.begin(), a.second->m_possibleMoves.end(), PieceType::Coordinates(5, m_coordinates.y)) != a.second->m_possibleMoves.end())))
					{
						return false; // conditions for right and left rooks
					}
			return true;
		};

		// check the left rook
		if (auto it = s_positionInfo.find(PieceType::Coordinates(0, m_coordinates.y));
			it != s_positionInfo.end() && it->second->isFirstTime() &&
			!s_positionInfo.contains({ 1,  m_coordinates.y }) && !s_positionInfo.contains({ 2,  m_coordinates.y }) && !s_positionInfo.contains({ 3,  m_coordinates.y }) && checkForStrikeThrough(true))
		{
			m_possibleMoves.push_back({ 2,  m_coordinates.y });
		}
		// check the right rook
		if (auto it = s_positionInfo.find(PieceType::Coordinates(7, m_coordinates.y));
			it != s_positionInfo.end() && it->second->isFirstTime() &&
			!s_positionInfo.contains({ 5,  m_coordinates.y }) && !s_positionInfo.contains({ 6,  m_coordinates.y }) && checkForStrikeThrough(false))
		{
			m_possibleMoves.push_back({ 6,  m_coordinates.y });
		}
	}
}

void King::eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)
{
	// needs to be executed only if two kings stand one next to another
	if (std::abs(kingPos.x - m_coordinates.x) <= 2 && std::abs(kingPos.y - m_coordinates.y) <= 2)
	{
		std::erase_if(possibleMoves, [this](const PieceType::Coordinates& move) {
			return (std::abs(move.x - m_coordinates.x) <= 1 && std::abs(move.y - m_coordinates.y) <= 1);
		});
	}
}

bool King::addMoves(int x, int y)
{
	if (auto it = s_positionInfo.find(PieceType::Coordinates(x, y));
		(it == s_positionInfo.end() || it->second->getColour() != m_teamColour) && x >= 0 && x <= 7 && y >= 0 && y <= 7)
		m_possibleMoves.push_back({ x, y });
	return true;
}
