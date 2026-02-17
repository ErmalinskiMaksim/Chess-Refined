#include "TextureMap.h"

TextureMap::TextureMap(Texture texture) 
    : m_texture(std::move(texture))
    , m_pieceWidth{static_cast<float>(m_texture.get()->w) / 6}
    , m_pieceHeight{static_cast<float>(m_texture.get()->h) / 2}
{}

Rect TextureMap::getTile(Piece piece) const noexcept {
    float y = (piece.col == Piece::Color::WHITE) ? m_pieceHeight : 0.0f;
    switch(piece.type) {
        case Piece::Type::QUEEN:  return Rect{0.0f          , y, m_pieceWidth, m_pieceHeight}; 
        case Piece::Type::KING:   return Rect{m_pieceWidth  , y, m_pieceWidth, m_pieceHeight};
        case Piece::Type::ROOK:   return Rect{m_pieceWidth*2, y, m_pieceWidth, m_pieceHeight};
        case Piece::Type::KNIGHT: return Rect{m_pieceWidth*3, y, m_pieceWidth, m_pieceHeight};
        case Piece::Type::BISHOP: return Rect{m_pieceWidth*4, y, m_pieceWidth, m_pieceHeight};
        case Piece::Type::PAWN:   return Rect{m_pieceWidth*5, y, m_pieceWidth, m_pieceHeight};
        default: return Rect{};
    }
}

TexturePtrType TextureMap::get() const noexcept {
    return m_texture.get();
}

bool TextureMap::empty() const noexcept {
    return m_texture.empty();
}
