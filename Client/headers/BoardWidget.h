#ifndef BOARD_WIDGET_H
#define BOARD_WIDGET_H

#include "RTWgui/Widgets/Widget.h"
#include "Pos.h"

class BoardWidget : public Widget {
public:
    BoardWidget(Widget&&);
    Pos guiPosToLogical(float, float) const noexcept;
    Point logicalToGuiPos(Pos) const noexcept;
    Rect rectFromPos(Pos) const noexcept;
    void render(const Renderer&, const Font&) const;
private:
    Rect m_squareSpaceHitBox;
    float m_squareSize;
};

#endif
