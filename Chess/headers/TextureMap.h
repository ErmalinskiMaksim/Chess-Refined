#ifndef TEXTURE_MAP_H
#define TEXTURE_MAP_H

#include "RTWgui/LibraryDependent/Texture.h"
#include "Piece.h"

static constexpr std::string_view IMAGE_PIECE_MAP_PATH = "Images/ChessPiecesArray.png";

class TextureMap {
public:
    TextureMap() = default;
    TextureMap(Texture);
    Rect getTile(PieceType, PieceColor) const noexcept;
    const Texture& get() const noexcept;
private:
    Texture m_texture; 
    float m_pieceWidth;
    float m_pieceHeight;
};

#endif
