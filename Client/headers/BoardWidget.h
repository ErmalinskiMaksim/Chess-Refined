#ifndef BOARD_WIDGET_H
#define BOARD_WIDGET_H

#include "RTWgui/Widgets/Widget.h"
#include "Pos.h"

// A widget class that represents a board
class BoardWidget : public Widget {
public:
    BoardWidget(Widget&&);
    // converts gui coordiantes to logical position
    Pos guiPosToLogical(float, float) const noexcept;
    // gets a rectangle that encompasses a logical position
    Rect rectFromPos(Pos) const noexcept;
    // Responsibility: main rendering logic
    void render(const Renderer&, const Font&) const;
private:
    float m_squareSize;
};

#endif
