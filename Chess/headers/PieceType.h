#ifndef PIECETYPE_H
#define PIECETYPE_H

#include <map>
#include <vector>
#include <string>
#include <typeinfo>
#include "Constants.h"
#include "../include/SFML/System/Vector2.hpp"

enum class TeamColour { BLACK = 0, WHITE = 1 };

class PieceType
{
public:
	PieceType(int x, int y, TeamColour tc, int val);
	virtual ~PieceType();

	// getters
	virtual TeamColour getColour() const;
	virtual const char* getDirectory() const;
	virtual bool isFirstTime() const;
	virtual sf::Vector2i getPosition() const;
	virtual int getValue() const;

	//setters
	virtual void setFirstTime(bool b);
	virtual void setPosition(const sf::Vector2i& pos);

	// updtes movelist
	virtual void updateMoveList(std::vector<std::string>& ml);

	// the name says it all
	virtual void checkTheKing();

	// checks if can prevent a check
	virtual void tryToPreventCheck();

	// if it's pinned it removes the illegal moves
	virtual void checkIfIsPinned(const std::vector<std::string>& ml);

	// calculates all moves (including illegal)
	virtual void calculatePossibleMoves(const std::vector<std::string>& ml) = 0;

public:
	struct Coordinates
	{
		Coordinates(int x, int y) : x(x), y(y) {}
		Coordinates(const sf::Vector2i& v) : x(v.x), y(v.y){}
		void updateBoard(const sf::Vector2i&);
		static const sf::Vector2i toPosition(const PieceType::Coordinates&);
		static const sf::Vector2i toBoard(const sf::Vector2i&);
		bool operator<(const Coordinates&)const;
		bool operator==(const Coordinates&)const;
		bool operator!=(const Coordinates&)const;
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

	// subroutine that simplifies the addition of moves
	virtual bool addMoves(int x, int y) = 0;

	// finds the king position
	virtual PieceType::Coordinates findMyKingPosition();

protected:
	const char* m_textureDirectory;
	TeamColour m_teamColour;
	bool m_movesFirstTime;
	sf::Vector2i m_position;      // not board
	int m_value;
};



class Pawn : public PieceType
{
public:
	Pawn(int x, int y, TeamColour tc);
	void calculatePossibleMoves(const std::vector<std::string>& ml)override;
private:
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
	bool addMoves(int x, int y)override;
};

class Rook : public PieceType
{
public:
	Rook(int x, int y, TeamColour tc);
	void calculatePossibleMoves(const std::vector<std::string>& ml)override;
private:
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
	bool addMoves(int x, int y)override;
};

class Knight : public PieceType
{
public:
	Knight(int x, int y, TeamColour tc);
	void calculatePossibleMoves(const std::vector<std::string>& ml)override;
private:
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
	bool addMoves(int x, int y)override;
};

class Bishop : public PieceType
{
public:
	Bishop(int x, int y, TeamColour tc);
	void calculatePossibleMoves(const std::vector<std::string>& ml)override;
private:
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
	bool addMoves(int x, int y)override;
};

class Queen : public PieceType
{
public:
	Queen(int x, int y, TeamColour tc);
	void calculatePossibleMoves(const std::vector<std::string>& ml)override;
private:
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
	bool addMoves(int x, int y)override;
};

class King : public PieceType
{
public:
	King(int x, int y, TeamColour tc);
	void calculatePossibleMoves(const std::vector<std::string>& ml)override;
private:
	void eraseDangerousCells(std::vector<PieceType::Coordinates>& possibleMoves, const PieceType::Coordinates& kingPos, bool isKing)override;
	bool addMoves(int x, int y)override;
};

#endif
