#ifndef PIECE_H
#define PIECE_H

#include "PieceType.h"
#include "Window.h"

class Piece
{
public:
	Piece() : m_hiden(false), m_selected(false), m_type(nullptr) {}
	~Piece();

	void processEvents(const PieceType::Coordinates& boardPos, Window& window, std::list<std::string> & ml);
	void update(const std::list<std::string>& ml);
	void display(Window& window, sf::Sprite s, sf::Texture t);

	// getters
	PieceType* getType() const;
	const sf::Vector2i getPosition() const;
	int getValue() const;
	static bool isTurnOver();

	// setters
	void setType(PieceType* type);
	void setPosition(const sf::Vector2i& pos);
	static void setTurnOver(bool isOver);

private:
	void checkForEnPassant(const PieceType::Coordinates& current, const PieceType::Coordinates& next);
	void checkForCastling();
	void checkForPromotion(Window& window);
private:

	PieceType* m_type; // type of a piece
	bool m_selected;   // is it selected by the player
	bool m_hiden;      // hidden pieces don't participate in the game

	static bool s_isTurnOver;		 // for an arbitrary piece to know if the turn is over
	static bool s_anyPiecesSelected; // for an arbitrary piece to know if there are any selected pieces
};

#endif
