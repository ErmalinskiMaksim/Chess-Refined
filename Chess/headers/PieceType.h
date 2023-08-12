#ifndef PIECETYPE_H
#define PIECETYPE_H

#include <map>
#include <vector>
#include <list>
#include <string>
#include "Constants.h"
#include "../include/SFML/System/Vector2.hpp"

enum class TeamColour { BLACK = 0, WHITE = 1 };

class PieceType
{
public:
	PieceType() : m_movesFirstTime(true), m_possibleMoves(0) {}
	virtual ~PieceType() { m_possibleMoves.clear(); }

	// getters
	virtual TeamColour getColour() const;
	virtual const char* getDirectory() const;
	virtual const char* getName()const;
	virtual bool isFirstTime() const;
	virtual sf::Vector2i getPosition() const;
	virtual int getValue() const;

	//setters
	virtual void setFirstTime(bool b);
	virtual void setPosition(const sf::Vector2i& pos);

	// updtes movelist
	virtual void updateMoveList(std::list<std::string>& ml);

	// the name says it all
	virtual void checkTheKing();

	// checks if can prevent a check
	virtual void tryToPreventCheck();

	// if it's pinned it removes the illegal moves
	virtual void checkIfIsPinned(const std::list<std::string>& ml);

	// calculates all moves (including illegal)
	virtual void calculatePossibleMoves(const std::list<std::string>& ml) = 0;

public:
	struct Coordinates
	{
		Coordinates() {}
		Coordinates(int x, int y) : x(x), y(y) {}
		Coordinates(const sf::Vector2i& v) : x(v.x), y(v.y){}
		void updateBoard(const sf::Vector2i& pos);
		static const sf::Vector2i toPosition(const PieceType::Coordinates & pos);
		static const sf::Vector2i toBoard(const sf::Vector2i& pos);
		bool operator<(const Coordinates c)const;
		bool operator==(const Coordinates c)const;
		bool operator!=(const Coordinates c)const;
		int x, y;
	}m_coordinates; // coordinates on the board

	// piece's avilable moves
	std::vector<PieceType::Coordinates> m_possibleMoves;

	// a map which contains all pieces on the board
	static std::map<PieceType::Coordinates, PieceType*> s_positionInfo;

	// pieces that check the king
	static std::vector<PieceType::Coordinates> s_dangerousPieces;

	// the currently checked king
	static std::pair<bool, TeamColour> s_checkedKing;

protected:

	// erases illegal moves (both when the king is checked and when safe)
	virtual void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing) = 0;

	// finds the king position
	virtual PieceType::Coordinates findMyKingPosition();

protected:
	const char* m_name;
	const char* m_textureDirectory;
	TeamColour m_teamColour;
	bool m_movesFirstTime;
	sf::Vector2i m_position;      // not board
	int m_value;
};



class Pawn : public PieceType
{
public:
	Pawn(TeamColour tc);
	void calculatePossibleMoves(const std::list<std::string>& ml)override;
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
};

class Rook : public PieceType
{
public:
	Rook(TeamColour tc);
	void calculatePossibleMoves(const std::list<std::string>& ml)override;
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
};

class Knight : public PieceType
{
public:
	Knight(TeamColour tc);
	void calculatePossibleMoves(const std::list<std::string>& ml)override;
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
};

class Bishop : public PieceType
{
public:
	Bishop(TeamColour tc);
	void calculatePossibleMoves(const std::list<std::string>& ml)override;
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
};

class Queen : public PieceType
{
public:
	Queen(TeamColour tc);
	void calculatePossibleMoves(const std::list<std::string>& ml)override;
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
};

class King : public PieceType
{
public:
	King(TeamColour tc);
	void calculatePossibleMoves(const std::list<std::string>& ml)override;
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
};

#endif
