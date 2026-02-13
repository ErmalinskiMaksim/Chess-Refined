#include "TextureMap.h"

TextureMap::TextureMap(Texture texture) 
    : m_texture(std::move(texture))
    , m_pieceWidth{static_cast<float>(m_texture.get()->w) / 6}
    , m_pieceHeight{static_cast<float>(m_texture.get()->h) / 2}
{}

Rect TextureMap::getTile(PieceType type, PieceColor color) const noexcept {
    float y = (color == PieceColor::WHITE) ? m_pieceHeight : 0.0f;
    switch(type) {
        case PieceType::QUEEN:  return Rect{0.0f          , y, m_pieceWidth, m_pieceHeight}; 
        case PieceType::KING:   return Rect{m_pieceWidth  , y, m_pieceWidth, m_pieceHeight};
        case PieceType::ROOK:   return Rect{m_pieceWidth*2, y, m_pieceWidth, m_pieceHeight};
        case PieceType::KNIGHT: return Rect{m_pieceWidth*3, y, m_pieceWidth, m_pieceHeight};
        case PieceType::BISHOP: return Rect{m_pieceWidth*4, y, m_pieceWidth, m_pieceHeight};
        case PieceType::PAWN:   return Rect{m_pieceWidth*5, y, m_pieceWidth, m_pieceHeight};
        default: return Rect{};
    }
}

const Texture& TextureMap::get() const noexcept {
    return m_texture;
}
