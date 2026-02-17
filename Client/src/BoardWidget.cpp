#include "BoardWidget.h"

BoardWidget::BoardWidget(Widget&& widget)
    : Widget(std::move(widget))
    , m_squareSpaceHitBox{m_hitBox}
    , m_squareSize{m_squareSpaceHitBox.w / 8}
{}

Pos BoardWidget::guiPosToLogical(float x, float y) const noexcept {
    return Pos(static_cast<int8_t>(x / m_squareSize), 7 - static_cast<int8_t>(y / m_squareSize));
}

Rect BoardWidget::rectFromPos(Pos pos) const noexcept {
    return Rect{static_cast<float>(pos.x)*m_squareSize, static_cast<float>(7-pos.y)*m_squareSize
            , m_squareSize, m_squareSize};
}

void BoardWidget::render(const Renderer& renderer, const Font&) const {
    if (m_texture.empty()) [[unlikely]] {
        Rect textureRect = { 0.0f, 0.0f, m_hitBox.w, m_hitBox.h };
        Texture upToDateTexture{renderer.get(), textureRect.w, textureRect.h};

        renderer.setTarget(upToDateTexture.get());
        renderer.clear(Color4{});

        for (auto row = 0; row < 8; ++row)
            for (auto column = 0; column < 8; ++column)
            {
                auto square = Rect{m_squareSize * static_cast<float>(column)
                             , m_squareSize * static_cast<float>(row)
                             , m_squareSize, m_squareSize};
                auto col = (row + column) % 2 ? Color{0x70, 0x70, 0x70, 0xFF} : Color {0xA5, 0xA5, 0xA5, 0xFF};
                renderer.renderFillRect(&square, col);
            }

        renderer.setTarget();

        std::swap(m_texture, upToDateTexture);
    }
    renderer.renderTexture(m_texture.get(), nullptr, &m_hitBox);
}
