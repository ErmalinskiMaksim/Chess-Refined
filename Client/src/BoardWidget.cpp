#include "BoardWidget.h"

BoardWidget::BoardWidget(Widget&& widget)
    : Widget(std::move(widget))
    , m_squareSize{m_hitBox.w / 8}
{}

// Responsibility: convert gui coordiantes to logical position
Pos BoardWidget::guiPosToLogical(float x, float y) const noexcept {
    return Pos(static_cast<int8_t>(x / m_squareSize), 7 - static_cast<int8_t>(y / m_squareSize));
}

// Responsibility: get a rectangle that encompasses a logical position
Rect BoardWidget::rectFromPos(Pos pos) const noexcept {
    return Rect{static_cast<float>(pos.x)*m_squareSize, static_cast<float>(7-pos.y)*m_squareSize
            , m_squareSize, m_squareSize};
}

// Responsibility: main rendering logic
void BoardWidget::render(const Renderer& renderer, const Font&) const {
    if (m_texture.empty()) [[unlikely]] { // if the texture is not initialized
        // Generate a new texture
        Rect textureRect = { 0.0f, 0.0f, m_hitBox.w, m_hitBox.h };
        Texture upToDateTexture{renderer.get(), textureRect.w, textureRect.h};

        // change rendering target from the window to the texture
        renderer.setTarget(upToDateTexture.get());
        renderer.clear(Color4{});

        // render the board texture
        for (auto row = 0; row < 8; ++row)
            for (auto column = 0; column < 8; ++column)
            {
                auto square = Rect{m_squareSize * static_cast<float>(column)
                             , m_squareSize * static_cast<float>(row)
                             , m_squareSize, m_squareSize};
                auto col = (row + column) % 2 ? Color{0x70, 0x70, 0x70, 0xFF} : Color {0xA5, 0xA5, 0xA5, 0xFF};
                renderer.renderFillRect(&square, col);
            }

        // reset the target back to the window
        renderer.setTarget();

        // update the texture
        std::swap(m_texture, upToDateTexture);
    }
    // render the texture
    renderer.renderTexture(m_texture.get(), nullptr, &m_hitBox);
}
