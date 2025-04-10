#ifndef PIECE_H
#define PIECE_H

#include "PieceType.h"
#include "Window.h"
#include <memory>

class Piece
{
public:
	Piece(std::unique_ptr<PieceType>);
	Piece(const Piece&) = delete;
	Piece(Piece&&) = default;
	void operator=(const Piece&) = delete;
	Piece& operator=(Piece&&) = default;
	~Piece() = default;

	void processEvents(const PieceType::Coordinates& boardPos, Window& window, std::vector<std::string> & ml);
	void update(const std::vector<std::string>& ml);
	void display(Window& window, sf::Sprite s, sf::Texture t);

	// getters
	const PieceType* const getType() const;
	const sf::Vector2i getPosition() const;
	int getValue() const;

	// setters
	void setPosition(const sf::Vector2i& pos);
public:
	static bool s_isTurnOver;		 // for an arbitrary piece to know if the turn is over
	static bool s_anyPiecesSelected; // for an arbitrary piece to know if there are any selected pieces
private:
	void checkForEnPassant(const PieceType::Coordinates& current, const PieceType::Coordinates& next);
	void checkForCastling();
	void checkForPromotion(Window& window);
private:
	std::unique_ptr<PieceType> m_type; // type of a piece
	bool m_selected;   // is it selected by the player
	bool m_hiden;      // hidden pieces don't participate in the game
};

#endif
