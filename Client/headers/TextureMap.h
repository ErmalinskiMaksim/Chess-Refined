#ifndef TEXTURE_MAP_H
#define TEXTURE_MAP_H

#include "RTWgui/LibraryDependent/Texture.h"
#include "Piece.h"

static constexpr std::string_view IMAGE_PIECE_MAP_PATH = "Client/Res/Images/ChessPiecesArray.png";

// This class represents a texture map containing an image of all chess pieces
class TextureMap {
public:
    TextureMap() = default;
    TextureMap(Texture);

    // get a tile boundary
    Rect getTile(Piece) const noexcept;

    // get a full texture view
    TexturePtrType get() const noexcept;

    // check if a texture is loaded
    bool empty() const noexcept;
private:
    // main texture
    Texture m_texture; 
    // dimensions of a single piece tile
    float m_pieceWidth;
    float m_pieceHeight;
};

#endif
