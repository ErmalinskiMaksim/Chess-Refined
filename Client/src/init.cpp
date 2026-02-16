#include "RTWgui/Init.h"

// Custom widgets

// Custom handlers for layers

// Custom Interactors
#include "BoardInteractor.h"

// Window
const std::string_view WINDOW_TITLE = "Chess";
constexpr unsigned TILE_SIZE = 60;
const unsigned WINDOW_WIDTH  = TILE_SIZE*8;
const unsigned WINDOW_HEIGHT = TILE_SIZE*8;

// Font
const std::string_view MAIN_FONT_PATH = "Client/Res/Fonts/Monoid-Regular.ttf";
const float MAIN_FONT_SZ = 32;

const size_t MAIN_LAYER_COUNT = 1;

void initializeLayers(LayerArray& layers, float mainFontCharWidth, float mainFontCharHeight) {
    constexpr int BOARD_HEIGHT = TILE_SIZE*8;
    constexpr int BOARD_WIDTH = TILE_SIZE*8; 

    layers[0] = std::make_unique
        <Layer<BoardWidget, HandlerContext, NonModalLayerCreateRequest, BoardInteractor
            , BoardPromotionHandler<HandlerContext>, GameOverHandler<HandlerContext>>>(
            NonModalLayerCreateRequest{
                Widget {
                    Rect{0.0f, 0.0f, BOARD_WIDTH, BOARD_HEIGHT}
                    , Color{0xEE, 0xEE, 0xEE, 0xFF}
                    , Color{0xEE, 0xEE, 0xEE, 0xFF}
                    , mainFontCharWidth
                    , mainFontCharHeight
                }, NonModalLayerCreateRequest::Payload{}}
            , BoardPromotionHandler<HandlerContext>{}
            , GameOverHandler<HandlerContext>{});
}
